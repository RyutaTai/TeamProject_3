#include "SceneTitle.h"

#include "../Graphics/Graphics.h"
#include "../Resources/Texture.h"
#include "../Input/GamePad.h"
#include "../Scenes/SceneManager.h"
#include "../Scenes/SceneLoading.h"
#include "../Scenes/SceneGame.h"

//	初期化
void SceneTitle::Initialize()
{
	sprite_[static_cast<int>(SPRITE_TITLE::BACK)]		= std::make_unique<Sprite>(Graphics::Instance().GetDevice(), L"./Resources/Image/Title.png");
	sprite_[static_cast<int>(SPRITE_TITLE::TEXT_KEY)]	= std::make_unique<Sprite>(Graphics::Instance().GetDevice(), L"./Resources/Fonts/font4.png");

	// オーディオ初期化
	//Audio audioInstance_ = Audio::Instance();
	//audioInstance_.Initialize();
	//bgm_[0] = std::make_unique<Audio>(audioInstance_.GetXAudio2(), L"./Resources/Audio/BGM/009.wav");
	//se_[0] = std::make_unique<Audio>(audioInstance_.GetXAudio2(), L"./Resources/Audio/SE/0footsteps-of-a-runner-on-gravel.wav");
	//se_[1] = std::make_unique<Audio>(audioInstance_.GetXAudio2(), L"./Resources/Audio/SE/0footsteps-dry-leaves-g.wav");
	//se_[2] = std::make_unique<Audio>(audioInstance_.GetXAudio2(), L"./Resources/Audio/SE/0explosion-8-bit.wav");

}

//	終了化
void SceneTitle::Finalize()
{
	for (int i = 0; i < static_cast<int>(SPRITE_TITLE::MAX); i++)
	{
		if (sprite_[i] != nullptr)
		{
			sprite_[i] = nullptr;
		}
	}
}
	
//	更新処理
void SceneTitle::Update(const float& elapsedTime)
{
	GamePad gamePad;
	gamePad.Acquire();
	static const int BUTTON_MAX = 4;

	GamePad::Button anyButton[BUTTON_MAX] =
	{ 
		GamePad::Button::A,		//	Z
		GamePad::Button::B,		//	X
		GamePad::Button::X,		//	C
		GamePad::Button::Y,		//	V
	};

	//	なにかボタンを押したらゲームシーンへ切り替え
	for (int i = 0; i < BUTTON_MAX; i++)
	{
		if (gamePad.ButtonState(anyButton[i], TriggerMode::NONE))
		{
			SceneManager::Instance().ChangeScene(new SceneLoading(new SceneGame));
			break;
		}
	}
	//	BGM再生
	//bgm_[0]->Play();
}

//	描画処理
void SceneTitle::Render()
{	
	// タイトルスプライト描画
	sprite_[static_cast<int>(SPRITE_TITLE::BACK)]->Render();
	sprite_[static_cast<int>(SPRITE_TITLE::TEXT_KEY)]->Textout("Push to Enter Key.", 350, 600, 30, 25);
}

//	デバッグ描画
void SceneTitle::DrawDebug()
{
	sprite_[static_cast<int>(SPRITE_TITLE::BACK)]->DrawDebug();
}
