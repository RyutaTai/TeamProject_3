#include "SceneGame.h"

#include "../Core/Framework.h"
#include "../Graphics/Graphics.h"
#include "../Graphics/Camera.h"
#include "../Input/GamePad.h"
#include "../Game/PieceManager.h"
#include "../Game/Piece.h"

//	初期化
void SceneGame::Initialize()
{
	//	スプライト初期化
	sprite_[static_cast<int>(SPRITE_GAME::BACK)] = std::make_unique<Sprite>(Graphics::Instance().GetDevice(), L"./Resources/Image/Game.png");
	
	//	ステージ初期化
	//stage_ = std::make_unique<Stage>("./Resources/Model/cybercity-2099-v2/source/Cyber_City_2099_ANIM.fbx", true);//	シティモデル
	stage_ = std::make_unique<Stage>("./Resources/Model/Shogi/syougiban.fbx",true);//	将棋盤
	
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = (sizeof(Graphics::SceneConstants));
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	Graphics::Instance().GetDevice()->CreateBuffer(&desc, nullptr, sceneConstantBuffer_.GetAddressOf());

	//	カメラ初期設定
	Camera::Instance().Initialize();

	//	将棋の駒生成
	PieceManager& pieceManager = PieceManager::Instance();
	for (int i = 0; i < Piece::PIECE_MAX; i++)
	{
		Piece* piece = new Piece("./Resources/Model/Shogi/hu.fbx", true);
		pieceManager.Register(piece);
		pieceManager.Initialize(i);
	}

}

//	終了化
void SceneGame::Finalize()
{
	//	スプライト終了化
	for (int i = 0; i < static_cast<int>(SPRITE_GAME::MAX); i++)
	{
		if (sprite_[i] != nullptr)
		{
			sprite_[i] = nullptr;
		}
	}
	stage_ = nullptr;	//	ステージ終了化
	//	エネミー終了化
	PieceManager::Instance().Clear();
}

//	更新処理
void SceneGame::Update(const float& elapsedTime)
{
	GamePad gamePad;
	gamePad.Acquire();

	Camera::Instance().SetTarget(stage_.get()->GetTransform()->GetPosition());
	Camera::Instance().Update(elapsedTime);

}

//	描画処理
void SceneGame::Render()
{
	Graphics::SceneConstants sceneConstants;

	Camera::Instance().SetPerspectiveFov();

	DirectX::XMStoreFloat4x4(&sceneConstants.viewProjection_, Camera::Instance().GetViewMatrix() * Camera::Instance().GetProjectionMatrix());
	sceneConstants.lightDirection_ = { 0, -1, 0, 0 };
	sceneConstants.cameraPosition_ = { 0, 0, 1, 0 };
	Graphics::Instance().GetDeviceContext()->UpdateSubresource(sceneConstantBuffer_.Get(), 0, 0, &sceneConstants, 0, 0);
	Graphics::Instance().GetDeviceContext()->VSSetConstantBuffers(1, 1, sceneConstantBuffer_.GetAddressOf());
	Graphics::Instance().GetDeviceContext()->PSSetConstantBuffers(1, 1, sceneConstantBuffer_.GetAddressOf());

	//	Sprite
	{
		sprite_[static_cast<int>(SPRITE_GAME::BACK)]->GetTransform()->SetSize(SCREEN_WIDTH, SCREEN_HEIGHT);
		sprite_[static_cast<int>(SPRITE_GAME::BACK)]->Render();	//	ゲームスプライト描画
	}

	// Model
	{
		//	ステージ
		//	ステート設定
#if 0	//	シティモデル用
		Graphics::Instance().GetShader()->SetRasterizerState(Shader::RASTERIZER_STATE::CULL_NONE);	//	両面描画するため
		Graphics::Instance().GetShader()->SetDepthStencilState(Shader::DEPTH_STENCIL_STATE::ZT_ON_ZW_ON);
		Graphics::Instance().GetShader()->SetBlendState(Shader::BLEND_STATE::ALPHA);
#else	//	将棋盤用
		Graphics::Instance().GetShader()->SetRasterizerState(Shader::RASTERIZER_STATE::SOLID);
		Graphics::Instance().GetShader()->SetDepthStencilState(Shader::DEPTH_STENCIL_STATE::ZT_ON_ZW_ON);
		Graphics::Instance().GetShader()->SetBlendState(Shader::BLEND_STATE::ALPHA);
#endif
		stage_->Render();										//	ステージ描画

		//	将棋の駒描画
		Graphics::Instance().GetShader()->SetRasterizerState(Shader::RASTERIZER_STATE::SOLID);
		Graphics::Instance().GetShader()->SetDepthStencilState(Shader::DEPTH_STENCIL_STATE::ZT_OFF_ZW_OFF);
		Graphics::Instance().GetShader()->SetBlendState(Shader::BLEND_STATE::ALPHA);
		PieceManager::Instance().Render();

	}

}

//	デバッグ描画
void SceneGame::DrawDebug()
{
	Camera::Instance().DrawDebug();

	stage_->DrawDebug();
	PieceManager::Instance().DrawDebug();
}