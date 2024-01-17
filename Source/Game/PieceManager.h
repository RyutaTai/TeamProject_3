#pragma once

#include <set>

#include "../Game/Piece.h"

//	将棋の駒管理クラス
class PieceManager
{
private:
	PieceManager() {}
	~PieceManager() {}
public:
	//	唯一のインスタンス取得
	static PieceManager& Instance()
	{
		static PieceManager instance_;
		return instance_;
	}

	void Initialize(int index);		//	初期化
	void Update(float elapsedTime);	//	更新処理
	void Render();					//	描画処理
	void DrawDebug();				//	デバッグ描画

	void Register(Piece* piece);	//	駒を登録
	void Clear();					//	駒を全て削除
	void Remove(Piece* piece);		//	駒を削除

	int GetPieceCount() const { return static_cast<int>(pieces_.size()); }	//	現在の駒の数取得
	Piece* GetPiece(int index) { return pieces_.at(index); }				//	駒を取得

private:
	std::vector<Piece*> pieces_;	//	駒の配列
	std::set<Piece*> removes_;		//	破棄リスト

	int pieceCount_;				//	現在の駒の数

};

