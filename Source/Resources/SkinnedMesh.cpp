#include "SkinnedMesh.h"

#include <sstream>
#include <functional>
#include <filesystem>
#include <fstream>

#include "StaticMesh.h"
#include "../Graphics/Shader.h"
#include "../Graphics/Graphics.h"
#include "../Others/Misc.h"
#include "../Resources/Texture.h"

//	コンストラクタ
SkinnedMesh::SkinnedMesh(ID3D11Device* device, const char* fbxFilename, bool triangulate/*三角化*/, float samplingRate)
{
	std::filesystem::path cerealFilename(fbxFilename);

	cerealFilename.replace_extension("cereal");
	if (std::filesystem::exists(cerealFilename.c_str()))	//	シリアライズされたファイルがあれば
	{
		std::ifstream ifs(cerealFilename.c_str(), std::ios::binary);
		cereal::BinaryInputArchive deserialization(ifs);
		deserialization(sceneView_, meshes_, materials_, animationClips_);
	}
	else //	シリアライズされたファイルがなければFBXからロードする
	{
		FbxManager* fbxManager{ FbxManager::Create() };
		FbxScene* fbxScene{ FbxScene::Create(fbxManager,"") };

		FbxImporter* fbxImporter{ FbxImporter::Create(fbxManager,"") };
		bool importStatus{ false };
		importStatus = fbxImporter->Initialize(fbxFilename);
		_ASSERT_EXPR_A(importStatus, fbxImporter->GetStatus().GetErrorString());

		importStatus = fbxImporter->Import(fbxScene);
		_ASSERT_EXPR_A(importStatus, fbxImporter->GetStatus().GetErrorString());

		FbxGeometryConverter fbx_converter(fbxManager);
		if (triangulate)
		{
			fbx_converter.Triangulate(fbxScene, true/*replace*/, false/*legacy*/);
			fbx_converter.RemoveBadPolygonsFromMeshes(fbxScene);
		}

		std::function<void(FbxNode*)>traverse{ [&](FbxNode* fbxNode) {
			SceneStruct::Node& node{sceneView_.nodes_.emplace_back()};
			node.attribute_ = fbxNode->GetNodeAttribute() ?
				fbxNode->GetNodeAttribute()->GetAttributeType() : FbxNodeAttribute::EType::eUnknown;
			node.name_ = fbxNode->GetName();
			node.uniqueID_ = fbxNode->GetUniqueID();
			node.parentIndex_ = sceneView_.indexof(fbxNode->GetParent() ?
				fbxNode->GetParent()->GetUniqueID() : 0);
			for (int childIndex = 0; childIndex < fbxNode->GetChildCount(); ++childIndex)
			{
				traverse(fbxNode->GetChild(childIndex));
			}
		} };
		traverse(fbxScene->GetRootNode());

		FetchMeshes(fbxScene, meshes_);
		FetchMaterials(fbxScene, materials_);
		FetchAnimations(fbxScene, animationClips_, samplingRate);

#if 0
		for (const Scene::Node& node : sceneView_.nodes_)
		{
			FbxNode* fbxNode{ fbxScene->FindNodeByName(node.name_.c_str()) };
			//Display node data in the output window as debug
			std::string nodeName = fbxNode->GetName();
			uint64_t uid = fbxNode->GetUniqueID();
			uint64_t parentUid = fbxNode->GetParent() ? fbxNode->GetParent()->GetUniqueID() : 0;
			int32_t type = fbxNode->GetNodeAttribute() ? fbxNode->GetNodeAttribute()->GetAttributeType() : 0;

			std::stringstream debugString;
			debugString << nodeName << ":" << uid << ":" << parentUid << ":" << type << "\n";
			OutputDebugStringA(debugString.str().c_str());
		}
#endif
		fbxManager->Destroy();

		std::ofstream ofs(cerealFilename.c_str(), std::ios::binary);
		cereal::BinaryOutputArchive serialization(ofs);
		serialization(sceneView_, meshes_, materials_, animationClips_);
	}

	CreateComObjects(device, fbxFilename);
}

void SkinnedMesh::FetchMeshes(FbxScene* fbxScene, std::vector<Mesh>& meshes)
{
	for (const SceneStruct::Node& node : sceneView_.nodes_)
	{
		if (node.attribute_ != FbxNodeAttribute::EType::eMesh)
		{
			continue;
		}

		FbxNode* fbxNode{ fbxScene->FindNodeByName(node.name_.c_str()) };
		FbxMesh* fbxMesh{ fbxNode->GetMesh() };

		Mesh& mesh{ meshes.emplace_back() };
		mesh.uniqueID_ = fbxMesh->GetNode()->GetUniqueID();
		mesh.name_ = fbxMesh->GetNode()->GetName();
		mesh.nodeIndex_ = sceneView_.indexof(mesh.uniqueID_);
		mesh.defaultGlobalTransform_ = ToXmfloat4x4(fbxMesh->GetNode()->EvaluateGlobalTransform());

		std::vector<BoneInfluencesPerControlPoint>boneInfluences;
		FetchBoneInfluences(fbxMesh, boneInfluences);
		FetchSkeleton(fbxMesh, mesh.bindPose_);
		
		std::vector<Mesh::Subset>& subsets{ mesh.subsets_ };
		const int materialCount{ fbxMesh->GetNode()->GetMaterialCount() };
		subsets.resize(materialCount > 0 ? materialCount : 1);
		for (int materialIndex = 0; materialIndex < materialCount; ++materialIndex)
		{
			const FbxSurfaceMaterial* fbxMaterial{ fbxMesh->GetNode()->GetMaterial(materialIndex) };
			subsets.at(materialIndex).materialName_ = fbxMaterial->GetName();
			subsets.at(materialIndex).materialUniqueID_ = fbxMaterial->GetUniqueID();
		}
		if (materialCount > 0)
		{
			const int polygonCount{ fbxMesh->GetPolygonCount() };
			for (int polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex)
			{
				const int materialIndex{ fbxMesh->GetElementMaterial()->GetIndexArray().GetAt(polygonIndex) };
				subsets.at(materialIndex).indexCount_ += 3;
			}
			uint32_t offset{ 0 };
			for (Mesh::Subset& subset : subsets)
			{
				subset.startIndexLocation_ = offset;
				offset += subset.indexCount_;
				//This will be as counter in the following procedures,reset to zero
				subset.indexCount_ = 0;
			}
		}

		const int polygonCount{ fbxMesh->GetPolygonCount() };
		mesh.vertices_.resize(polygonCount * 3LL);
		mesh.indices_.resize(polygonCount * 3LL);

		FbxStringList uvNames;
		fbxMesh->GetUVSetNames(uvNames);
		const FbxVector4* controlPoints{ fbxMesh->GetControlPoints() };
		for (int polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex)
		{
			const int materialIndex{ materialCount > 0 ?
			fbxMesh->GetElementMaterial()->GetIndexArray().GetAt(polygonIndex) : 0 };
			Mesh::Subset& subset{ subsets.at(materialIndex) };
			const uint32_t offset{ subset.startIndexLocation_ + subset.indexCount_ };

			for (int positionInPolygon = 0; positionInPolygon < 3; ++positionInPolygon)
			{
				const int vertexIndex{ polygonIndex * 3 + positionInPolygon };

				Vertex vertex;
				const int polygonVertex{ fbxMesh->GetPolygonVertex(polygonIndex,positionInPolygon) };
				vertex.position_.x = static_cast<float>(controlPoints[polygonVertex][0]);
				vertex.position_.y = static_cast<float>(controlPoints[polygonVertex][1]);
				vertex.position_.z = static_cast<float>(controlPoints[polygonVertex][2]);

				const BoneInfluencesPerControlPoint& influencesPerControlPoint
				{ boneInfluences.at(polygonVertex) };
				for (size_t influenceIndex = 0; influenceIndex < influencesPerControlPoint.size();
					++influenceIndex)
				{
					if (influenceIndex < MAX_BONE_INFLUENCES)
					{
						vertex.boneWeights_[influenceIndex] =
							influencesPerControlPoint.at(influenceIndex).boneWeight_;
						vertex.boneIndices_[influenceIndex] =
							influencesPerControlPoint.at(influenceIndex).boneIndex_;
					}
					else//影響を受けるボーンの数がMAX_BONE_INFLUENCES以上だったらどうするか考える
					{
						_ASSERT_EXPR(FALSE, L"'influence_index' > MAX_BONE_INFLUENCES");//アサートで落とす

						//４つのweighthのトータルは必ず1.0にならないとだめ

					}
				}

				if (fbxMesh->GetElementNormalCount() > 0)
				{
					FbxVector4 normal;
					fbxMesh->GetPolygonVertexNormal(polygonIndex, positionInPolygon, normal);
					vertex.normal_.x = static_cast<float>(normal[0]);
					vertex.normal_.y = static_cast<float>(normal[1]);
					vertex.normal_.z = static_cast<float>(normal[2]);
				}
				if (fbxMesh->GetElementUVCount() > 0)
				{
					FbxVector2 uv;
					bool unmappedUv;
					fbxMesh->GetPolygonVertexUV(polygonIndex, positionInPolygon,
						uvNames[0], uv, unmappedUv);
					vertex.texcoord_.x = static_cast<float>(uv[0]);
					vertex.texcoord_.y = 1.0f - static_cast<float>(uv[1]);
				}
				//法線ベクトルの値を取得
				if (fbxMesh->GenerateTangentsData(0, false))
				{
					const FbxGeometryElementTangent* tangent = fbxMesh->GetElementTangent(0);
					vertex.tangent_.x = static_cast<float>(tangent->GetDirectArray().GetAt(vertexIndex)[0]);
					vertex.tangent_.y = static_cast<float>(tangent->GetDirectArray().GetAt(vertexIndex)[1]);
					vertex.tangent_.z = static_cast<float>(tangent->GetDirectArray().GetAt(vertexIndex)[2]);
					vertex.tangent_.w = static_cast<float>(tangent->GetDirectArray().GetAt(vertexIndex)[3]);
				}

				mesh.vertices_.at(vertexIndex) = std::move(vertex);
				mesh.indices_.at(static_cast<size_t>(offset) + positionInPolygon) = vertexIndex;
				subset.indexCount_++;
			}

		}

		for (const Vertex& v : mesh.vertices_)
		{
			mesh.boundingBox_[0].x = std::min<float>(mesh.boundingBox_[0].x, v.position_.x);
			mesh.boundingBox_[0].y = std::min<float>(mesh.boundingBox_[0].y, v.position_.y);
			mesh.boundingBox_[0].z = std::min<float>(mesh.boundingBox_[0].z, v.position_.z);
			mesh.boundingBox_[1].x = std::min<float>(mesh.boundingBox_[1].x, v.position_.x);
			mesh.boundingBox_[1].y = std::min<float>(mesh.boundingBox_[1].y, v.position_.y);
			mesh.boundingBox_[1].z = std::min<float>(mesh.boundingBox_[1].z, v.position_.z);
		}
	}
}

void SkinnedMesh::CreateComObjects(ID3D11Device* device, const char* fbxFilename)
{
	for (Mesh& mesh : meshes_)
	{
		HRESULT hr{ S_OK };
		D3D11_BUFFER_DESC bufferDesc{};
		D3D11_SUBRESOURCE_DATA subresourceData{};
		bufferDesc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * mesh.vertices_.size());
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;
		subresourceData.pSysMem = mesh.vertices_.data();
		subresourceData.SysMemPitch = 0;
		subresourceData.SysMemSlicePitch = 0;
		hr = device->CreateBuffer(&bufferDesc, &subresourceData, mesh.vertexBuffer_.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		bufferDesc.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * mesh.indices_.size());
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		subresourceData.pSysMem = mesh.indices_.data();
		hr = device->CreateBuffer(&bufferDesc, &subresourceData, mesh.indexBuffer_.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

#if 1
		mesh.vertices_.clear();
		mesh.indices_.clear();
#endif
	}

	for (std::unordered_map<uint64_t, Material>::iterator iterator = materials_.begin();
		iterator != materials_.end(); ++iterator)
	{
		for (size_t textureIndex = 0; textureIndex < 2; ++textureIndex)
		{
			if (iterator->second.textureFilenames_[textureIndex].size() > 0)
			{
				std::filesystem::path path(fbxFilename);
				path.replace_filename(iterator->second.textureFilenames_[textureIndex]);
				D3D11_TEXTURE2D_DESC texture2dDesc;
				LoadTextureFromFile(device, path.c_str(),
					iterator->second.shaderResourceViews_[textureIndex].GetAddressOf(), &texture2dDesc);
			}
			else
			{
				MakeDummyTexture(device, iterator->second.shaderResourceViews_[textureIndex].GetAddressOf(),
					textureIndex == 1 ? 0xFFFF7F7F : 0xFFFFFFFF, 16);
			}
		}
	}

	HRESULT hr = S_OK;
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[]
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT},
		{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT},
		{"TANGENT",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT},
		{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT},
		{"WEIGHTS",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT},
		{"BONES",0,DXGI_FORMAT_R32G32B32A32_UINT,0,D3D11_APPEND_ALIGNED_ELEMENT},
	};
	Graphics::Instance().GetShader()->CreateVsFromCso(device, "./Resources/Shader/SkinnedMeshVs.cso", vertexShader_.ReleaseAndGetAddressOf(),
		inputLayout_.ReleaseAndGetAddressOf(), inputElementDesc, ARRAYSIZE(inputElementDesc));
	Graphics::Instance().GetShader()->CreatePsFromCso(device, "./Resources/Shader/SkinnedMeshPs.cso", pixelShader_.ReleaseAndGetAddressOf());

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = sizeof(Constants);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hr = device->CreateBuffer(&bufferDesc, nullptr, constantBufer_.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
}

void SkinnedMesh::FetchMaterials(FbxScene* fbxScene, std::unordered_map<uint64_t, Material>& materials)
{
	const size_t nodeCount{ sceneView_.nodes_.size() };
	for (size_t nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
	{
		const SceneStruct::Node& node{ sceneView_.nodes_.at(nodeIndex) };
		const FbxNode* fbxNode{ fbxScene->FindNodeByName(node.name_.c_str()) };
		const int materialCount{ fbxNode->GetMaterialCount() };

		for (int materialIndex = 0; materialIndex < materialCount; ++materialIndex)
		{
			const FbxSurfaceMaterial* fbxSurfaceMaterial{ fbxNode->GetMaterial(materialIndex) };

			Material material;
			material.name_ = fbxSurfaceMaterial->GetName();
			material.uniqueID_ = fbxSurfaceMaterial->GetUniqueID();
			FbxProperty fbxProperty;
			fbxProperty = fbxSurfaceMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);
			if (fbxProperty.IsValid())
			{
				const FbxDouble3 color{ fbxProperty.Get<FbxDouble3>() };
				material.Kd_.x = static_cast<float>(color[0]);
				material.Kd_.y = static_cast<float>(color[1]);
				material.Kd_.z = static_cast<float>(color[2]);
				material.Kd_.w = 1.0f;

				const FbxFileTexture* fbxTexture{ fbxProperty.GetSrcObject<FbxFileTexture>() };
				material.textureFilenames_[0] = fbxTexture ? fbxTexture->GetRelativeFileName() : "";
			}
			fbxProperty = fbxSurfaceMaterial->FindProperty(FbxSurfaceMaterial::sNormalMap);
			if (fbxProperty.IsValid())
			{
				const FbxFileTexture* fileTexture{ fbxProperty.GetSrcObject<FbxFileTexture>() };
				material.textureFilenames_[1] = fileTexture ? fileTexture->GetRelativeFileName() : "";
			}
			materials.emplace(material.uniqueID_, std::move(material));

		}
	}
	materials.emplace();	//	最初のマテリアルが読み込まれる
}

void SkinnedMesh::FetchSkeleton(FbxMesh* fbxMesh, Skeleton& bindPose)
{
	const int deformerCount = fbxMesh->GetDeformerCount(FbxDeformer::eSkin);
	for (int deformerIndex = 0; deformerIndex < deformerCount; ++deformerIndex)
	{
		FbxSkin* skin = static_cast<FbxSkin*>(fbxMesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));
		const int clusterCount = skin->GetClusterCount();
		bindPose.bones_.resize(clusterCount);
		for (int clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex)
		{
			FbxCluster* cluster = skin->GetCluster(clusterIndex);

			Skeleton::Bone& bone{ bindPose.bones_.at(clusterIndex) };
			bone.name_ = cluster->GetLink()->GetName();
			bone.uniqueID_ = cluster->GetLink()->GetUniqueID();
			bone.parentIndex_ = bindPose.indexof(cluster->GetLink()->GetParent()->GetUniqueID());
			bone.nodeIndex_ = sceneView_.indexof(bone.uniqueID_);

			//'reference_global_init_position' is used to convert from local space of model(mesh) to
		    // global space of scene.
			FbxAMatrix referenceGlobalInitPosition;
			cluster->GetTransformMatrix(referenceGlobalInitPosition);

			// 'cluster_global_init_position' is used to convert from local space of bone to
			// global space of scene.
			FbxAMatrix clusterGlobalInitPosition;
			cluster->GetTransformLinkMatrix(clusterGlobalInitPosition);

			// Matrices are defined using the Column Major scheme. When a FbxAMatrix represents a transformation
			// (translation, rotation and scale), the last row of the matrix represents the translation part of
			//the transformation.
			// Compose 'bone.offset_transform' matrix that trnasforms position from mesh space to bone space.
			// This matrix is called the offset matrix.
			bone.offsetTransform_ = ToXmfloat4x4(clusterGlobalInitPosition.Inverse() * referenceGlobalInitPosition);

		}
	}
}

void SkinnedMesh::FetchAnimations(FbxScene* fbxScene, std::vector<Animation>& animationClips,
	float samplingRate /*If this value is 0, the animation data will be sampled at the default frame rate.*/)
{
	FbxArray<FbxString*>animationStackNames;
	fbxScene->FillAnimStackNameArray(animationStackNames);
	const int animationStackCount{ animationStackNames.GetCount() };
	for (int animationStackIndex = 0; animationStackIndex < animationStackCount; ++animationStackIndex)
	{
		Animation& animationClip{ animationClips.emplace_back() };
		animationClip.name_ = animationStackNames[animationStackIndex]->Buffer();

		FbxAnimStack* animationStack{ fbxScene->FindMember<FbxAnimStack>(animationClip.name_.c_str()) };
		fbxScene->SetCurrentAnimationStack(animationStack);

		const FbxTime::EMode timeMode{ fbxScene->GetGlobalSettings().GetTimeMode() };
		FbxTime oneSecond;
		oneSecond.SetTime(0, 0, 1, 0, 0, timeMode);
		animationClip.samplingRate_ = samplingRate > 0 ?
			samplingRate : static_cast<float>(oneSecond.GetFrameRate(timeMode));
		const FbxTime samplingInterval
			{ static_cast<FbxLongLong>(oneSecond.Get() / animationClip.samplingRate_) };
		const FbxTakeInfo* takeInfo{ fbxScene->GetTakeInfo(animationClip.name_.c_str()) };
		const FbxTime startTime{ takeInfo->mLocalTimeSpan.GetStart() };
		const FbxTime stopTime{ takeInfo->mLocalTimeSpan.GetStop() };
		for (FbxTime time = startTime; time < stopTime; time += samplingInterval)
		{
			Animation::Keyframe& keyframe{ animationClip.sequence_.emplace_back() };

			const size_t nodeCount{ sceneView_.nodes_.size() };
			keyframe.nodes_.resize(nodeCount);
			for (size_t nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
			{
				FbxNode* fbxNode{ fbxScene->FindNodeByName(sceneView_.nodes_.at(nodeIndex).name_.c_str()) };
				if (fbxNode)
				{
					Animation::Keyframe::Node& node{ keyframe.nodes_.at(nodeIndex) };
					// 'global_transform' is a transformation matrix of a node with respect to
					// the scene's global coordinate system.
					node.globalTransform_ = ToXmfloat4x4(fbxNode->EvaluateGlobalTransform(time));
					
					//'local_transform' is a transformation matrix of a node with respect to
					//its parent's local coordinate system.
					const FbxAMatrix& localTransform{ fbxNode->EvaluateLocalTransform(time) };
					node.scaling_ = ToXmfloat3(localTransform.GetS());
					node.rotation_ = ToXmfloat4(localTransform.GetQ());
					node.translation_ = ToXmfloat3(localTransform.GetT());
				}
			}
		}
	}
	for (int animationStackIndex = 0; animationStackIndex < animationStackCount; ++animationStackIndex)
	{
		delete animationStackNames[animationStackIndex];
	}
}

void SkinnedMesh::Render(const DirectX::XMMATRIX& world, const DirectX::XMFLOAT4& materialColor, const Animation::Keyframe* keyframe)
{
	Graphics& graphics = Graphics::Instance();

	D3D11_VIEWPORT viewport{};
	UINT numViewports{ 1 };

	graphics.GetDeviceContext()->RSGetViewports(&numViewports, &viewport);

	for (const Mesh& mesh : meshes_)
	{
		uint32_t stride{ sizeof(Vertex) };
		uint32_t offset{ 0 };
		graphics.GetDeviceContext()->IASetVertexBuffers(0, 1, mesh.vertexBuffer_.GetAddressOf(), &stride, &offset);
		graphics.GetDeviceContext()->IASetIndexBuffer(mesh.indexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);
		graphics.GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		graphics.GetDeviceContext()->IASetInputLayout(inputLayout_.Get());
		graphics.GetDeviceContext()->VSSetShader(vertexShader_.Get(), nullptr, 0);
		graphics.GetDeviceContext()->PSSetShader(pixelShader_.Get(), nullptr, 0);

		Constants data;
		if (keyframe && keyframe->nodes_.size() > 0)
		{
			const Animation::Keyframe::Node& meshNode{ keyframe->nodes_.at(mesh.nodeIndex_) };
			XMStoreFloat4x4(&data.world_, XMLoadFloat4x4(&meshNode.globalTransform_) * world);

			const size_t boneCount{ mesh.bindPose_.bones_.size() };
			_ASSERT_EXPR(boneCount < MAX_BONES, L"The value of the 'bone_count' has exceeded MAX_BONES.");

			for (size_t boneIndex = 0; boneIndex < boneCount; ++boneIndex)
			{
				const Skeleton::Bone& bone{ mesh.bindPose_.bones_.at(boneIndex) };
				const Animation::Keyframe::Node& boneNode{ keyframe->nodes_.at(bone.nodeIndex_) };
				XMStoreFloat4x4(&data.boneTransforms_[boneIndex],
					XMLoadFloat4x4(&bone.offsetTransform_) *
					XMLoadFloat4x4(&boneNode.globalTransform_) *
					XMMatrixInverse(nullptr, XMLoadFloat4x4(&mesh.defaultGlobalTransform_))
				);
			}
		}
		else
		{
			DirectX::XMStoreFloat4x4(&data.world_,
				XMLoadFloat4x4(&mesh.defaultGlobalTransform_) * world);
			for (size_t boneIndex = 0; boneIndex < MAX_BONES; ++boneIndex)
			{
				data.boneTransforms_[boneIndex] = { 1,0,0,0,
													0,1,0,0,
													0,0,1,0,
													0,0,0,1 };
			}
		}
		for (const Mesh::Subset& subset : mesh.subsets_)
		{
			const Material& material{ materials_.at(subset.materialUniqueID_) };
			XMStoreFloat4(&data.materialColor_, DirectX::XMVectorMultiply(XMLoadFloat4(&materialColor), XMLoadFloat4(&material.Kd_)));
			graphics.GetDeviceContext()->UpdateSubresource(constantBufer_.Get(), 0, 0, &data, 0, 0);
			graphics.GetDeviceContext()->VSSetConstantBuffers(0, 1, constantBufer_.GetAddressOf());
			graphics.GetDeviceContext()->PSSetShaderResources(0, 1, material.shaderResourceViews_[0].GetAddressOf());
			graphics.GetDeviceContext()->PSSetShaderResources(1, 1, material.shaderResourceViews_[1].GetAddressOf());

			graphics.GetDeviceContext()->DrawIndexed(subset.indexCount_, subset.startIndexLocation_, 0);
		}
	}
}

inline DirectX::XMFLOAT4X4 ToXmfloat4x4(const FbxAMatrix& fbxamatrix)
{
	DirectX::XMFLOAT4X4 xmfloat4x4;
	for (int row = 0; row < 4; ++row)
	{
		for (int colmun = 0; colmun < 4; ++colmun)
		{
			xmfloat4x4.m[row][colmun] = static_cast<float>(fbxamatrix[row][colmun]);
		}
	}
	return xmfloat4x4;
}

inline DirectX::XMFLOAT3 ToXmfloat3(const FbxDouble3& fbxdouble3)
{
	DirectX::XMFLOAT3 xmfloat3;
	xmfloat3.x = static_cast<float>(fbxdouble3[0]);
	xmfloat3.y = static_cast<float>(fbxdouble3[1]);
	xmfloat3.z = static_cast<float>(fbxdouble3[2]);
	return xmfloat3;
}

inline DirectX::XMFLOAT4 ToXmfloat4(const FbxDouble4& fbxdouble4)
{
	DirectX::XMFLOAT4 xmfloat4;
	xmfloat4.x = static_cast<float>(fbxdouble4[0]);
	xmfloat4.y = static_cast<float>(fbxdouble4[1]);
	xmfloat4.z = static_cast<float>(fbxdouble4[2]);
	xmfloat4.w = static_cast<float>(fbxdouble4[3]);
	return xmfloat4;
}

void FetchBoneInfluences(const FbxMesh* fbxMesh,
	std::vector<BoneInfluencesPerControlPoint>& boneInfluences)
{
	const int controlPointsCount{ fbxMesh->GetControlPointsCount() };
	boneInfluences.resize(controlPointsCount);

	const int skinCount{ fbxMesh->GetDeformerCount(FbxDeformer::eSkin) };
	for (int skinIndex = 0; skinIndex < skinCount; ++skinIndex)
	{
		const FbxSkin* fbxSkin
		{ static_cast<FbxSkin*>(fbxMesh->GetDeformer(skinIndex,FbxDeformer::eSkin)) };
		
		const int clusterCount{ fbxSkin->GetClusterCount() };
		for (int clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex)
		{
			const FbxCluster* fbxCluster{ fbxSkin->GetCluster(clusterIndex) };

			const int controlPointIndicesCount{ fbxCluster->GetControlPointIndicesCount() };
			for (int controlPointIndicesIndex = 0; controlPointIndicesIndex < controlPointIndicesCount;
				++controlPointIndicesIndex)
			{
				int controlPointIndex{ fbxCluster->GetControlPointIndices() [controlPointIndicesIndex ] };
				double controlPointWeight
				{ fbxCluster->GetControlPointWeights()[controlPointIndicesIndex] };
				BoneInfluence& boneInfluence{ boneInfluences.at(controlPointIndex).emplace_back() };
				boneInfluence.boneIndex_ = static_cast<uint32_t>(clusterIndex);
				boneInfluence.boneWeight_ = static_cast<float>(controlPointWeight);
			}
		}
	
	}
}

void SkinnedMesh::UpdateAnimation(Animation::Keyframe& keyframe)
{
	size_t nodeCount{ keyframe.nodes_.size() };
	for (size_t nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
	{
		Animation::Keyframe::Node& node{ keyframe.nodes_.at(nodeIndex) };
		DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(node.scaling_.x,node.scaling_.y,node.scaling_.z) };
		DirectX::XMMATRIX R{ DirectX::XMMatrixRotationQuaternion(XMLoadFloat4(&node.rotation_)) };
		DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(node.translation_.x,node.translation_.y,node.translation_.z) };

		int64_t parentIndex{ sceneView_.nodes_.at(nodeIndex).parentIndex_ };
		DirectX::XMMATRIX P{ parentIndex < 0 ? DirectX::XMMatrixIdentity() : 
		DirectX::XMLoadFloat4x4(&keyframe.nodes_.at(parentIndex).globalTransform_) };

		XMStoreFloat4x4(&node.globalTransform_, S * R * T * P);
	}
}

bool SkinnedMesh::AppendAnimations(const char* animationFilename, float samplingRate)
{
	FbxManager* fbxManager{ FbxManager::Create() };
	FbxScene*	fbxScene{ FbxScene::Create(fbxManager,"") };

	FbxImporter* fbxImporter{ FbxImporter::Create(fbxManager,"") };
	bool importStatus{ false };
	importStatus = fbxImporter->Initialize(animationFilename);
	_ASSERT_EXPR_A(importStatus, fbxImporter->GetStatus().GetErrorString());
	importStatus = fbxImporter->Import(fbxScene);
	_ASSERT_EXPR_A(importStatus, fbxImporter->GetStatus().GetErrorString());

	FetchAnimations(fbxScene, animationClips_, samplingRate);

	fbxManager->Destroy();

		
	return true;
}

void SkinnedMesh::BlendAnimations(const Animation::Keyframe* keyframes[2], float factor, Animation::Keyframe& keyframe)
{
	size_t nodeCount{ keyframes[0]->nodes_.size() };
	keyframe.nodes_.resize(nodeCount);
	for (size_t nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
	{
		DirectX::XMVECTOR S[2]{
			XMLoadFloat3(&keyframes[0]->nodes_.at(nodeIndex).scaling_),
			XMLoadFloat3(&keyframes[1]->nodes_.at(nodeIndex).scaling_) };
		XMStoreFloat3(&keyframe.nodes_.at(nodeIndex).scaling_, DirectX::XMVectorLerp(S[0], S[1], factor));

		DirectX::XMVECTOR R[2]{
			XMLoadFloat4(&keyframes[0]->nodes_.at(nodeIndex).rotation_),
			XMLoadFloat4(&keyframes[1]->nodes_.at(nodeIndex).rotation_) };
		XMStoreFloat4(&keyframe.nodes_.at(nodeIndex).rotation_, DirectX::XMQuaternionSlerp(R[0], R[1], factor));

		DirectX::XMVECTOR T[2]{
			XMLoadFloat3(&keyframes[0]->nodes_.at(nodeIndex).translation_),
			XMLoadFloat3(&keyframes[1]->nodes_.at(nodeIndex).translation_) };
		XMStoreFloat3(&keyframe.nodes_.at(nodeIndex).translation_, DirectX::XMVectorLerp(T[0], T[1], factor));
	}
}