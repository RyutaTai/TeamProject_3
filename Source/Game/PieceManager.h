#pragma once

#include <set>

#include "../Game/Piece.h"

//	�����̋�Ǘ��N���X
class PieceManager
{
private:
	PieceManager() {}
	~PieceManager() {}
public:
	//	�B��̃C���X�^���X�擾
	static PieceManager& Instance()
	{
		static PieceManager instance_;
		return instance_;
	}

	void Initialize(int index);		//	������
	void Update(float elapsedTime);	//	�X�V����
	void Render();					//	�`�揈��
	void DrawDebug();				//	�f�o�b�O�`��

	void Register(Piece* piece);	//	���o�^
	void Clear();					//	���S�č폜
	void Remove(Piece* piece);		//	����폜

	int GetPieceCount() const { return static_cast<int>(pieces_.size()); }	//	���݂̋�̐��擾
	Piece* GetPiece(int index) { return pieces_.at(index); }				//	����擾

private:
	std::vector<Piece*> pieces_;	//	��̔z��
	std::set<Piece*> removes_;		//	�j�����X�g

	int pieceCount_;				//	���݂̋�̐�

};

