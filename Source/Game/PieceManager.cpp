#include "PieceManager.h"

//	������
void PieceManager::Initialize(int index)
{
	//��̏�����
	for (Piece* piece : pieces_)
	{
		piece->Initialize(index);
	}
}

//	�X�V����
void PieceManager::Update(float elapsedTime)
{
	//	��̍X�V����
	for (Piece* piece : pieces_)
	{
		piece->Update(elapsedTime);
	}

	//	�j������
	for (Piece* piece : removes_)
	{
		std::vector<Piece*>::iterator it =
			std::find(pieces_.begin(), pieces_.end(), piece);
		if (it != pieces_.end())
		{
			pieces_.erase(it);
		}
		delete piece;
	}
	//	�j�����X�g���N���A
	removes_.clear();
}

//	���o�^
void PieceManager::Register(Piece* piece)
{
	pieces_.emplace_back(piece);
}

//	���S�č폜
void PieceManager::Clear()
{
	for (Piece* piece : pieces_)
	{
		delete piece;
	}
	pieces_.clear();
}

//	����폜
void PieceManager::Remove(Piece* piece)
{
	removes_.insert(piece);	//	�j�����X�g�ɒǉ�
}

//	�`�揈��
void PieceManager::Render()
{
	for (Piece* piece : pieces_)
	{
		piece->Render();
	}
}

//	�f�o�b�O�`��
void PieceManager::DrawDebug()
{
	for (Piece* piece : pieces_)
	{
		piece->DrawDebug();
	}
}


