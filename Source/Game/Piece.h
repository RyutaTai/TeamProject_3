#pragma once

#include "GameObject.h"

//	�����̋�N���X
class Piece :public GameObject
{
private:	
	//	�\���̂Ƃ�
	enum class PIECE_TYPE		//	��̎�ނ�錾����
	{
		HUHYOU,					//	����
		TOKIN,					//	�Ƌ��i�����������j
		KYOUSHA,				//	����
		NARIKYOU,				//	�����i���������ԁj
		KEIMA,					//	�j�n
		NARIKEI,				//	���j�i�������j�n�j
		GINSHOU,				//	�⏫
		NARIGIN,				//	����i�������⏫�j
		HISHA,					//	���
		RYUOU,					//	�����i��������ԁj
		KAKUGYOU,				//	�p�s
		RYUMA,					//	���n�i�������p�s�j
		KINSHOU,				//	����
		OUSHOU,					//	���� �ʏ��͒�`���ĂȂ�
	};

	enum class DIRECTION		//	��̐i�s����
	{

	};

	struct PieceInfo			//	��̏��
	{
	public:
		int posX_;						//	���x���W
		int posY_;						//	���y���W
		PIECE_TYPE pieceType_;			//	��̎��
		bool isPromote_ = false;		//	����񂩂��H ������Ă��邩	�p�ꌗ�ł͏����̐�����upromote:���i����v���g���ĕ\������
		bool isEnemy_;					//	�G�̋�A�����̋ �G�Ȃ�true
	};

public:
	static const int PIECE_MAX = 40;	//	��̍ő吔
	//	��̏������W
	PieceInfo pieceInfo_[PIECE_MAX] =
	{
		//	�G
		{1,1,PIECE_TYPE::KYOUSHA,	false,	true},
		{2,1,PIECE_TYPE::KEIMA,		false,	true},
		{3,1,PIECE_TYPE::GINSHOU,	false,	true},
		{4,1,PIECE_TYPE::KINSHOU,	false,	true},
		{5,1,PIECE_TYPE::OUSHOU,	false,	true},
		{6,1,PIECE_TYPE::KINSHOU,	false,	true},
		{7,1,PIECE_TYPE::GINSHOU,	false,	true},
		{8,1,PIECE_TYPE::KEIMA,		false,	true},
		{9,1,PIECE_TYPE::KYOUSHA,	false,	true},

		{2,2,PIECE_TYPE::HISHA,		false,	true},	//	10
		{8,2,PIECE_TYPE::KAKUGYOU,	false,	true},

		{1,1,PIECE_TYPE::HUHYOU,	false,	true},
		{2,1,PIECE_TYPE::HUHYOU,	false,	true},
		{3,1,PIECE_TYPE::HUHYOU,	false,	true},
		{4,1,PIECE_TYPE::HUHYOU,	false,	true},
		{5,1,PIECE_TYPE::HUHYOU,	false,	true},
		{6,1,PIECE_TYPE::HUHYOU,	false,	true},
		{7,1,PIECE_TYPE::HUHYOU,	false,	true},
		{8,1,PIECE_TYPE::HUHYOU,	false,	true},
		{9,1,PIECE_TYPE::HUHYOU,	false,	true},	//	20

		//	����
		{1,7,PIECE_TYPE::KYOUSHA,	false,	false},
		{2,7,PIECE_TYPE::KEIMA,		false,	false},
		{3,7,PIECE_TYPE::GINSHOU,	false,	false},
		{4,7,PIECE_TYPE::KINSHOU,	false,	false},
		{5,7,PIECE_TYPE::OUSHOU,	false,	false},
		{6,7,PIECE_TYPE::KINSHOU,	false,	false},
		{7,7,PIECE_TYPE::GINSHOU,	false,	false},
		{8,7,PIECE_TYPE::KEIMA,		false,	false},
		{9,7,PIECE_TYPE::KYOUSHA,	false,	false},

		{2,8,PIECE_TYPE::KAKUGYOU,	false,	false},	//	30
		{8,8,PIECE_TYPE::HISHA,		false,	false},

		{1,9,PIECE_TYPE::HUHYOU,	false,	false},
		{2,9,PIECE_TYPE::HUHYOU,	false,	false},
		{3,9,PIECE_TYPE::HUHYOU,	false,	false},
		{4,9,PIECE_TYPE::HUHYOU,	false,	false},
		{5,9,PIECE_TYPE::HUHYOU,	false,	false},
		{6,9,PIECE_TYPE::HUHYOU,	false,	false},
		{7,9,PIECE_TYPE::HUHYOU,	false,	false},
		{8,9,PIECE_TYPE::HUHYOU,	false,	false},
		{9,9,PIECE_TYPE::HUHYOU,	false,	false},	//	40
	};

public:
	Piece(const char* filename, bool triangulate = false);
	~Piece() {}

	void Initialize(int index);							//	�������@�ʒu�ݒ�Ƃ�
	void Update(float elapsedTime);						//	�X�V����
	void Render();										//	�`�揈��
	void DrawDebug();									//	�f�o�b�O�`��

	void Destroy();										//	�j������
	PieceInfo GetPieceInfo(int index) { return this->pieceInfo_[index]; }	//	�����̋�f�[�^�擾	

private:
	static const int boradX_ = 9;						//	�}�X��x��
	static const int boardY_ = 9;						//	�}�X��y��
	static const int board_[boradX_][boardY_];			//	�����Ղ̃f�[�^ 9x9
	DirectX::XMFLOAT3 pieceOffset_ = { 10,10,10 };		//	����ŏ��ɕ`�悷��Ƃ��̃I�t�Z�b�g�l

private:
	static int num;	//	ImGui�f�o�b�O�p
	int myNum_;		//	ImGui�f�o�b�O�p
};

