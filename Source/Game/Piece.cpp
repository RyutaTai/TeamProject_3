#include "Piece.h"

#include "PieceManager.h"
#include "Stage.h"

int Piece::num = 0;	//	�f�o�b�O�p

//	�R���X�g���N�^
Piece::Piece(const char* filename, bool triangulate)
	:GameObject(filename, triangulate)
{
	myNum_ = num++;	//	�f�o�b�O�p
}

//	������
void Piece::Initialize(int index)
{
	GetTransform()->SetPositionX(Stage::Instance().GetTransform()->GetPosition().x);
	GetTransform()->SetPositionY(Stage::Instance().GetTransform()->GetPosition().y);
	GetTransform()->SetPositionZ(Stage::Instance().GetTransform()->GetPosition().z);
}

//	�X�V����
void Piece::Update(float elapsedTime)
{

}

//	�j��
void Piece::Destroy()
{
	PieceManager::Instance().Remove(this);
}

//	�`�揈��
void Piece::Render()
{
	GameObject::Render(1000.0f);
}

//	�f�o�b�O�`��
void Piece::DrawDebug()
{
	std::string n = "piace" + std::to_string(myNum_);
	if (ImGui::TreeNode(n.c_str()))
	{
		GetTransform()->DrawDebug();
		ImGui::TreePop();
	}
}
