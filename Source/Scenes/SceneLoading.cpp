#include "SceneLoading.h"

#include "SceneManager.h"
#include "../Graphics/Graphics.h"
#include "../Core/Framework.h"
#include "../Others/Transform.h"
#include "../Others/MathHelper.h"

//	初期化
void SceneLoading::Initialize()
{
	//	スプライト初期化
	sprite_ = std::make_unique<Sprite>(Graphics::Instance().GetDevice(), L"./Resources/Image/NowLoading.png");

	//	スレッド開始
	thread_ = new std::thread(LoadingThread, this);	//	LoadingThread関数にthisを渡す

}

//	終了化
void SceneLoading::Finalize()
{
	//	スレッド終了化
	if (thread_ != nullptr)
	{
		thread_->join();	//	処理が終わるまで待つ
		delete thread_;
		thread_ = nullptr;
	}

	//	スプライト終了化
	sprite_ = nullptr;
}

//	更新処理
void SceneLoading::Update(const float& elapsedTime)
{
	//	次のシーンの準備が完了したらシーンを切り替える
	if (nextScene_->IsReady()) {
		SceneManager::Instance().ChangeScene(nextScene_);
		nextScene_ = nullptr;
	}
}

//	描画処理
void SceneLoading::Render()
{
	//	Sprite
	{
		//	ローディング画面を描画
		sprite_->GetTransform()->SetSize({ 1280,720 });
		//sprite_->GetTransform()->SetSize({ 5120,720 });//5120では無理?
		sprite_->GetTransform()->SetTexSize({ 1280,720 });

		animationTimer++;	//	elapsedTimeを関数で取得して足したほうがいい？ 速くなったり遅くなったりする
		//animationTimer += this->timeAddend + this->GetElapsedTime();
		if (animationTimer > animationFrame)
		{
			float x = animationNumber * 1280;
			sprite_->GetTransform()->SetTexPosX(x);					//	次のコマに移動
			animationTimer = 0;										//	アニメーションタイマーをリセット
			animationNumber++;										//	アニメーション数を増やす
			if (animationNumber > animationMAX)animationNumber = 0;	//	アニメーション数をリセット
		}
		sprite_->Render();
	}
}

//	ローディングスレッド
void SceneLoading::LoadingThread(SceneLoading* scene)
{
	//	COM関連の初期化でスレッド毎に呼ぶ必要がある
	CoInitialize(nullptr);

	//	次のシーンの初期化を行う
	scene->nextScene_->Initialize();

	//	スレッドが終わる前にCOM関連の終了化
	CoUninitialize();

	//	次のシーンの準備完了設定
	scene->nextScene_->SetReady();

}

//	デバッグ描画
void SceneLoading::DrawDebug()
{
	sprite_->DrawDebug();
}