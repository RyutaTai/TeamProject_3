#pragma once

#include <memory>

#include "Scene.h"
#include "../Resources/Sprite.h"
#include "../Audio/audio.h"

class SceneTitle : public Scene
{
public:
	SceneTitle(){}
	~SceneTitle()override{}

	void Initialize()						override;
	void Finalize()							override;

	void Update(const float& elapsedTime)	override;
	void Render()							override;
	void DrawDebug()						override;

private:
	enum class SPRITE_TITLE
	{
		BACK,		//	背景画像
		TEXT_KEY,	//	Push to ○○Key の表示
		MAX,		//	スプライトの上限数
	};
	std::unique_ptr<Sprite> sprite_[static_cast<int>(SPRITE_TITLE::MAX)];
	

//private:	//	オーディオ
//	static const int SE_MAX_ = 8;
//	static const int BGM_MAX_ = 4;
//	std::unique_ptr<Audio> bgm_[BGM_MAX_];
//	std::unique_ptr<Audio> se_[SE_MAX_];
};

