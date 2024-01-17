#pragma once

#include "Scene.h"

class SceneManager
{
private:
	SceneManager() {}
	~SceneManager() {}

public: 
	static SceneManager& Instance()
	{
		static SceneManager instance;
		return instance;
	}
	void Update(const float& elapsedTime);
	void Render();
	void Clear();
	void ChangeScene(Scene* scene);

private:
	Scene* currentScene_ = nullptr;		//	現在のシーン
	Scene* nextScene_ = nullptr;		//	次のシーン

};

