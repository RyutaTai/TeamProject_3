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
	//	���[�f�B���O�X���b�h
	static void LoadingThread(SceneLoading* scene);

private:
	Scene*						nextScene_	= nullptr;
	std::thread*				thread_		= nullptr;
	std::unique_ptr<Sprite>		sprite_		= nullptr;
	float angle_ = 0.0f;

	int animationNumber		= 0;		//	���݂̃A�j���[�V������(�����ڂ̃A�j���[�V������)
	int animationMAX		= 5;		//	�摜�̃A�j���[�V��������
	float animationFrame	= 120.0f;	//	���̃R�}�ɍs���܂ł̃t���[����
	float animationTimer	= 0;		//	���݂̃t���[����(�A�j���[�V�����^�C�}�[)
};

