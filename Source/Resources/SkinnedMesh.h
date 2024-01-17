#pragma once

#include <d3d11.h>
#include <directxmath.h>
#include <wrl.h>
#include <vector>
#include <string>
#include <fbxsdk.h>
#include <unordered_map>
#include <cereal/archives/binary.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/unordered_map.hpp>

namespace DirectX
{
	template<class T>
	void serialize(T& archive, DirectX::XMFLOAT2& v)
	{
		archive(
			cereal::make_nvp("x", v.x),
			cereal::make_nvp("y", v.y)
		);
	}

	template<class T>
	void serialize(T& archive, DirectX::XMFLOAT3& v)
	{
		archive(
			cereal::make_nvp("x", v.x),
			cereal::make_nvp("y", v.y),
			cereal::make_nvp("z", v.z)
		);
	}

	template<class T>
	void serialize(T& archive, DirectX::XMFLOAT4& v)
	{
		archive(
			cereal::make_nvp("x", v.x),
			cereal::make_nvp("y", v.y),
			cereal::make_nvp("z", v.z),
			cereal::make_nvp("w", v.w)
		);
	}

	template<class T>
	void serialize(T& archive, DirectX::XMFLOAT4X4& m)
	{
		archive(
			cereal::make_nvp("_11", m._11), cereal::make_nvp("_12", m._12),
			cereal::make_nvp("_13", m._13), cereal::make_nvp("_14", m._14),
			cereal::make_nvp("_21", m._21), cereal::make_nvp("_22", m._22),
			cereal::make_nvp("_23", m._23), cereal::make_nvp("_24", m._24),
			cereal::make_nvp("_31", m._31), cereal::make_nvp("_32", m._32),
			cereal::make_nvp("_33", m._33), cereal::make_nvp("_34", m._34),
			cereal::make_nvp("_41", m._41), cereal::make_nvp("_42", m._42),
			cereal::make_nvp("_43", m._43), cereal::make_nvp("_44", m._44)
		);
	}
}

struct Skeleton
{
	struct Bone
	{
		template<class T>
		void serialize(T& archive)
		{
			archive(uniqueID_, name_, parentIndex_,nodeIndex_, offsetTransform_);
		}

		uint64_t uniqueID_{ 0 };
		std::string name_;
		// 'parent_index' is index that refers to the parent bone's position in the array that contains itself.
		int64_t parentIndex_{ -1 };//-1 : the bone is orphan
		// 'node_index' is an index that refers to the node array of the scene.
		int64_t nodeIndex_{ 0 };

		// 'offset_transform' is used to convert from model(mesh) space to bone(node) scene.
		DirectX::XMFLOAT4X4 offsetTransform_{ 1, 0, 0, 0,
												0, 1, 0, 0,
												0, 0, 1, 0,
												0, 0, 0, 1 };

		bool isOrphan()const { return parentIndex_ < 0; }
	};

	template<class T>
	void serialize(T& archive)
	{
		archive(bones_);
	}

	std::vector<Bone>bones_;
	int64_t indexof(uint64_t uniqueID)const
	{
		int64_t index{ 0 };
		for (const Bone& bone : bones_)
		{
			if (bone.uniqueID_ == uniqueID)
			{
				return index;
			}
			++index;
		}
		return -1;
	}
};

struct Animation
{
	std::string name_;
	float samplingRate_{ 0 };

	struct Keyframe
	{
		struct Node
		{
			template <class T>
			void serialize(T& archive)
			{
				archive(globalTransform_, scaling_, rotation_, translation_);
			}

			// 'global_transform' is used to convert from local space of node to global space of scene.
			DirectX::XMFLOAT4X4 globalTransform_ { 1, 0, 0, 0, 
													0, 1, 0, 0,
													0, 0, 1, 0,
													0, 0, 0, 1 };

			//The transform data of a node includes its translation,rotation and scaling vectors
			//with respect to its parent
			DirectX::XMFLOAT3 scaling_{ 1,1,1 };
			DirectX::XMFLOAT4 rotation_{ 0,0,0,1 };//Rotaion quaternion
			DirectX::XMFLOAT3 translation_{ 0,0,0 };
		};
		std::vector<Node>nodes_;

		template<class T>
		void serialize(T& archive)
		{
			archive(nodes_);
		}
	};
	std::vector<Keyframe>sequence_;

	template<class T>
	void serialize(T& archive)
	{
		archive(name_, samplingRate_, sequence_);
	}
};

struct SceneStruct
{
	struct Node
	{
		template<class T>
		void serialize(T& archive)
		{
			archive(uniqueID_, name_, attribute_, parentIndex_);
		}

		uint64_t uniqueID_{ 0 };
		std::string name_;
		FbxNodeAttribute::EType attribute_{ FbxNodeAttribute::EType::eUnknown };
		int64_t parentIndex_{ -1 };
	};
	std::vector<Node>nodes_;

	template<class T>
	void serialize(T& archive)
	{
		archive(nodes_);
	}

	int64_t indexof(uint64_t unique_id)const
	{
		int64_t index{ 0 };
		for (const Node& node : nodes_)
		{
			if (node.uniqueID_ == unique_id)
			{
				return index;
			}
			++index;
		}
		return -1;
	}
};

class SkinnedMesh
{
public:
	static const int MAX_BONE_INFLUENCES{ 4 };
	struct Vertex
	{
		template<class T>
		void serialize(T& archive)
		{
			archive(position_, normal_, tangent_, texcoord_,boneWeights_, boneIndices_);
		}

		DirectX::XMFLOAT3 position_;
		DirectX::XMFLOAT3 normal_;
		DirectX::XMFLOAT4 tangent_;
		DirectX::XMFLOAT2 texcoord_;
		float boneWeights_[MAX_BONE_INFLUENCES]{ 1,0,0,0 };
		uint32_t boneIndices_[MAX_BONE_INFLUENCES]{};
	};
	static const int MAX_BONES{ 256 };

	struct Constants
	{
		DirectX::XMFLOAT4X4 world_;
		DirectX::XMFLOAT4   materialColor_;
		DirectX::XMFLOAT4X4 boneTransforms_[MAX_BONES]{ 
			{ 1, 0, 0, 0,
			  0, 1, 0, 0,
			  0, 0, 1, 0,
			  0, 0, 0, 1 } };
	};

	struct Mesh
	{
		uint64_t uniqueID_{ 0 };
		std::string name_;
		// 'node_index'is an index that refers to the node array of the scene.
		int64_t nodeIndex_{ 0 };
		std::vector<Vertex>vertices_;
		std::vector<uint32_t>indices_;
		DirectX::XMFLOAT4X4 defaultGlobalTransform_{ 
			1,0,0,0,
			0,1,0,0,
			0,0,1,0,
			0,0,0,1 };
		struct Subset
		{
			template<class T>
			void serialize(T& archive)
			{
				archive(materialUniqueID_, materialName_, startIndexLocation_, indexCount_);
			}

			uint64_t materialUniqueID_{ 0 };
			std::string materialName_;

			uint32_t startIndexLocation_{ 0 };
			uint32_t indexCount_{ 0 };
		}; std::vector<Subset>subsets_;

		template<class T>
		void serialize(T& archive)
		{
			archive(uniqueID_, name_, nodeIndex_, subsets_, defaultGlobalTransform_,
				bindPose_, boundingBox_, vertices_, indices_);
		}
		Skeleton bindPose_;

	private:
		Microsoft::WRL::ComPtr <ID3D11Buffer> vertexBuffer_;
		Microsoft::WRL::ComPtr <ID3D11Buffer> indexBuffer_;
		friend class SkinnedMesh;
		DirectX::XMFLOAT3 boundingBox_[2]
		{
			{+D3D11_FLOAT32_MAX,+D3D11_FLOAT32_MAX,+D3D11_FLOAT32_MAX},
			{-D3D11_FLOAT32_MAX,-D3D11_FLOAT32_MAX,-D3D11_FLOAT32_MAX}
		};
	};
	std::vector<Mesh>meshes_;

	struct Material
	{
		template<class T>
		void serialize(T& archive)
		{
			archive(uniqueID_, name_, Ka_, Kd_, Ks_, textureFilenames_);
		}

		uint64_t uniqueID_{ 0 };
		std::string name_;

		DirectX::XMFLOAT4 Ka_{ 0.2f,0.2f,0.2f,1.0f };	//	アンビエント(環境光)
		DirectX::XMFLOAT4 Kd_{ 0.8f,0.8f,0.8f,1.0f };	//	ディフューズ(拡散反射光)
		DirectX::XMFLOAT4 Ks_{ 1.0f,1.0f,1.0f,1.0f };	//	スペキュラ(鏡面反射光)

		std::string textureFilenames_[4];
		Microsoft::WRL::ComPtr <ID3D11ShaderResourceView> shaderResourceViews_[4];
	};
	std::unordered_map<uint64_t, Material>materials_;

	std::vector<Animation>animationClips_;

public:
	SkinnedMesh(ID3D11Device* device, const char* fbxFilename, bool triangulate = false, float SamplingRate = 0);
	virtual ~SkinnedMesh() = default;

	void Render(const DirectX::XMMATRIX& world, const DirectX::XMFLOAT4& materialColor, const Animation::Keyframe* keyframe);

	void UpdateAnimation(Animation::Keyframe& keyframe);
	bool AppendAnimations(const char* animationFilename, float samplingRate);
	void BlendAnimations(const Animation::Keyframe* keyframes[2], float factor, Animation::Keyframe& keyframe);

private:
	void FetchMeshes(FbxScene* fbxScene, std::vector<Mesh>& meshes);
	void CreateComObjects(ID3D11Device* device, const char* fbxFileName);
	void FetchMaterials(FbxScene* fbxScene, std::unordered_map<uint64_t, Material>& materials);
	void FetchSkeleton(FbxMesh* fbxMesh, Skeleton& bindPose);
	void FetchAnimations(FbxScene* fbxScene, std::vector<Animation>& animationClips,
		float samplingRate /*If this value is 0, the animation data will be sampled at the default frame rate.*/);

private:
	Microsoft::WRL::ComPtr <ID3D11VertexShader>	vertexShader_;
	Microsoft::WRL::ComPtr <ID3D11PixelShader>	pixelShader_;
	Microsoft::WRL::ComPtr <ID3D11InputLayout>	inputLayout_;
	Microsoft::WRL::ComPtr <ID3D11Buffer>		constantBufer_;

protected:
	SceneStruct sceneView_;

};

struct BoneInfluence
{
	uint32_t boneIndex_;
	float boneWeight_;
};
using BoneInfluencesPerControlPoint = std::vector<BoneInfluence>;

inline DirectX::XMFLOAT3   ToXmfloat3(const FbxDouble3& fbxdouble3);
inline DirectX::XMFLOAT4   ToXmfloat4(const FbxDouble4& fbxdouble4);
inline DirectX::XMFLOAT4X4 ToXmfloat4x4(const FbxAMatrix& fbxamatrix);
void FetchBoneInfluences(const FbxMesh* fbxMesh,
	std::vector<BoneInfluencesPerControlPoint>& boneInfluences);