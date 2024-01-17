#include "SceneGame.h"

#include "../Core/Framework.h"
#include "../Graphics/Graphics.h"
#include "../Graphics/Camera.h"
#include "../Input/GamePad.h"
#include "../Game/PieceManager.h"
#include "../Game/Piece.h"

//	������
void SceneGame::Initialize()
{
	//	�X�v���C�g������
	sprite_[static_cast<int>(SPRITE_GAME::BACK)] = std::make_unique<Sprite>(Graphics::Instance().GetDevice(), L"./Resources/Image/Game.png");
	
	//	�X�e�[�W������
	//stage_ = std::make_unique<Stage>("./Resources/Model/cybercity-2099-v2/source/Cyber_City_2099_ANIM.fbx", true);//	�V�e�B���f��
	stage_ = std::make_unique<Stage>("./Resources/Model/Shogi/syougiban.fbx",true);//	������
	
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = (sizeof(Graphics::SceneConstants));
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	Graphics::Instance().GetDevice()->CreateBuffer(&desc, nullptr, sceneConstantBuffer_.GetAddressOf());

	//	�J���������ݒ�
	Camera::Instance().Initialize();

	//	�����̋��
	PieceManager& pieceManager = PieceManager::Instance();
	for (int i = 0; i < Piece::PIECE_MAX; i++)
	{
		Piece* piece = new Piece("./Resources/Model/Shogi/hu.fbx", true);
		pieceManager.Register(piece);
		pieceManager.Initialize(i);
	}

}

//	�I����
void SceneGame::Finalize()
{
	//	�X�v���C�g�I����
	for (int i = 0; i < static_cast<int>(SPRITE_GAME::MAX); i++)
	{
		if (sprite_[i] != nullptr)
		{
			sprite_[i] = nullptr;
		}
	}
	stage_ = nullptr;	//	�X�e�[�W�I����
	//	�G�l�~�[�I����
	PieceManager::Instance().Clear();
}

//	�X�V����
void SceneGame::Update(const float& elapsedTime)
{
	GamePad gamePad;
	gamePad.Acquire();

	Camera::Instance().SetTarget(stage_.get()->GetTransform()->GetPosition());
	Camera::Instance().Update(elapsedTime);

}

//	�`�揈��
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
		sprite_[static_cast<int>(SPRITE_GAME::BACK)]->Render();	//	�Q�[���X�v���C�g�`��
	}

	// Model
	{
		//	�X�e�[�W
		//	�X�e�[�g�ݒ�
#if 0	//	�V�e�B���f���p
		Graphics::Instance().GetShader()->SetRasterizerState(Shader::RASTERIZER_STATE::CULL_NONE);	//	���ʕ`�悷�邽��
		Graphics::Instance().GetShader()->SetDepthStencilState(Shader::DEPTH_STENCIL_STATE::ZT_ON_ZW_ON);
		Graphics::Instance().GetShader()->SetBlendState(Shader::BLEND_STATE::ALPHA);
#else	//	�����՗p
		Graphics::Instance().GetShader()->SetRasterizerState(Shader::RASTERIZER_STATE::SOLID);
		Graphics::Instance().GetShader()->SetDepthStencilState(Shader::DEPTH_STENCIL_STATE::ZT_ON_ZW_ON);
		Graphics::Instance().GetShader()->SetBlendState(Shader::BLEND_STATE::ALPHA);
#endif
		stage_->Render();										//	�X�e�[�W�`��

		//	�����̋�`��
		Graphics::Instance().GetShader()->SetRasterizerState(Shader::RASTERIZER_STATE::SOLID);
		Graphics::Instance().GetShader()->SetDepthStencilState(Shader::DEPTH_STENCIL_STATE::ZT_OFF_ZW_OFF);
		Graphics::Instance().GetShader()->SetBlendState(Shader::BLEND_STATE::ALPHA);
		PieceManager::Instance().Render();

	}

}

//	�f�o�b�O�`��
void SceneGame::DrawDebug()
{
	Camera::Instance().DrawDebug();

	stage_->DrawDebug();
	PieceManager::Instance().DrawDebug();
}