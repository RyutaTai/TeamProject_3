#include "GameObject.h"

#include "../Graphics/Graphics.h"

//	�R���X�g���N�^
GameObject::GameObject(const char* fbxFilename,bool triangulate)
{
	model_ = std::make_unique<SkinnedMesh>(Graphics::Instance().GetDevice(), fbxFilename,triangulate);
}

//	�`�揈��
void GameObject::Render(const float& scale)
{
	model_->Render(GetTransform()->CalcWorldMatrix(scale), {1,1,1,1},nullptr);
}