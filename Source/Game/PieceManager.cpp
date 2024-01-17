#include "PieceManager.h"

//	初期化
void PieceManager::Initialize(int index)
{
	//駒の初期化
	for (Piece* piece : pieces_)
	{
		piece->Initialize(index);
	}
}

//	更新処理
void PieceManager::Update(float elapsedTime)
{
	//	駒の更新処理
	for (Piece* piece : pieces_)
	{
		piece->Update(elapsedTime);
	}

	//	破棄処理
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
	//	破棄リストをクリア
	removes_.clear();
}

//	駒を登録
void PieceManager::Register(Piece* piece)
{
	pieces_.emplace_back(piece);
}

//	駒を全て削除
void PieceManager::Clear()
{
	for (Piece* piece : pieces_)
	{
		delete piece;
	}
	pieces_.clear();
}

//	駒を削除
void PieceManager::Remove(Piece* piece)
{
	removes_.insert(piece);	//	破棄リストに追加
}

//	描画処理
void PieceManager::Render()
{
	for (Piece* piece : pieces_)
	{
		piece->Render();
	}
}

//	デバッグ描画
void PieceManager::DrawDebug()
{
	for (Piece* piece : pieces_)
	{
		piece->DrawDebug();
	}
}


