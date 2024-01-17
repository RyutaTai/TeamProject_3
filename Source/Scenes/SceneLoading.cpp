#include "SceneLoading.h"

#include "SceneManager.h"
#include "../Graphics/Graphics.h"
#include "../Core/Framework.h"
#include "../Others/Transform.h"
#include "../Others/MathHelper.h"

//	������
void SceneLoading::Initialize()
{
	//	�X�v���C�g������
	sprite_ = std::make_unique<Sprite>(Graphics::Instance().GetDevice(), L"./Resources/Image/NowLoading.png");

	//	�X���b�h�J�n
	thread_ = new std::thread(LoadingThread, this);	//	LoadingThread�֐���this��n��

}

//	�I����
void SceneLoading::Finalize()
{
	//	�X���b�h�I����
	if (thread_ != nullptr)
	{
		thread_->join();	//	�������I���܂ő҂�
		delete thread_;
		thread_ = nullptr;
	}

	//	�X�v���C�g�I����
	sprite_ = nullptr;
}

//	�X�V����
void SceneLoading::Update(const float& elapsedTime)
{
	//	���̃V�[���̏���������������V�[����؂�ւ���
	if (nextScene_->IsReady()) {
		SceneManager::Instance().ChangeScene(nextScene_);
		nextScene_ = nullptr;
	}
}

//	�`�揈��
void SceneLoading::Render()
{
	//	Sprite
	{
		//	���[�f�B���O��ʂ�`��
		sprite_->GetTransform()->SetSize({ 1280,720 });
		//sprite_->GetTransform()->SetSize({ 5120,720 });//5120�ł͖���?
		sprite_->GetTransform()->SetTexSize({ 1280,720 });

		animationTimer++;	//	elapsedTime���֐��Ŏ擾���đ������ق��������H �����Ȃ�����x���Ȃ����肷��
		//animationTimer += this->timeAddend + this->GetElapsedTime();
		if (animationTimer > animationFrame)
		{
			float x = animationNumber * 1280;
			sprite_->GetTransform()->SetTexPosX(x);					//	���̃R�}�Ɉړ�
			animationTimer = 0;										//	�A�j���[�V�����^�C�}�[�����Z�b�g
			animationNumber++;										//	�A�j���[�V�������𑝂₷
			if (animationNumber > animationMAX)animationNumber = 0;	//	�A�j���[�V�����������Z�b�g
		}
		sprite_->Render();
	}
}

//	���[�f�B���O�X���b�h
void SceneLoading::LoadingThread(SceneLoading* scene)
{
	//	COM�֘A�̏������ŃX���b�h���ɌĂԕK�v������
	CoInitialize(nullptr);

	//	���̃V�[���̏��������s��
	scene->nextScene_->Initialize();

	//	�X���b�h���I���O��COM�֘A�̏I����
	CoUninitialize();

	//	���̃V�[���̏��������ݒ�
	scene->nextScene_->SetReady();

}

//	�f�o�b�O�`��
void SceneLoading::DrawDebug()
{
	sprite_->DrawDebug();
}