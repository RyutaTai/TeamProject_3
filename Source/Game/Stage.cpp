#include "Stage.h"

#include "../Graphics/Graphics.h"

//	�R���X�g���N�^
Stage::Stage(const char* filename, bool triangulate)
	: GameObject(filename, triangulate)
{
	GetTransform()->SetPosition(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));
	GetTransform()->SetScale(DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f));
	GetTransform()->SetRotation(DirectX::XMFLOAT4(0.0f, DirectX::XMConvertToRadians(0), 0.0f, 0.0f));

}

//�f�X�g���N�^
Stage::~Stage()
{

}

//	�`�揈��
void Stage::Render()
{
	GameObject::Render(1.0f);
}

//	�f�o�b�O�`��
void Stage::DrawDebug()
{
	GetTransform()->DrawDebug();
}
