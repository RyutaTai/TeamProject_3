#pragma once

#include <thread>

#include "Scene.h"
#include "../Resources/Sprite.h"

class SceneLoading :public Scene
{
public:
	SceneLoading(Scene* nextScene) :nextScene_(nextScene) {}
	SceneLoading() {}
	~SceneLoading()override {}

	void Initialize()						override;
	void Finalize()							override;

	void Update(const float& elapsedTime)	override;
	void Render()							override;
	void DrawDebug()						override;

private:
	//	ローディングスレッド
	static void LoadingThread(SceneLoading* scene);

private:
	Scene*						nextScene_	= nullptr;
	std::thread*				thread_		= nullptr;
	std::unique_ptr<Sprite>		sprite_		= nullptr;
	float angle_ = 0.0f;

	int animationNumber		= 0;		//	現在のアニメーション数(何枚目のアニメーションか)
	int animationMAX		= 5;		//	画像のアニメーション枚数
	float animationFrame	= 120.0f;	//	次のコマに行くまでのフレーム数
	float animationTimer	= 0;		//	現在のフレーム数(アニメーションタイマー)
};

