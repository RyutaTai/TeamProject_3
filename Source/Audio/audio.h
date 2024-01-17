#pragma once

#include <xaudio2.h>
#include <mmreg.h>
#include <wrl.h>

#include "../Others/Misc.h"

class Audio
{
public:
	Audio(IXAudio2* xaudio2, const wchar_t* filename);
	Audio() {}
	virtual ~Audio();

	//	�B��̃C���X�^���X�擾
	static Audio& Instance()
	{
		static Audio instance_;
		return instance_;
	}

	void Initialize();

	void Play(int loopCount = 0/*255 : XAUDIO2_LOOP_INFINITE*/);			//	�Đ�
	void Stop(bool playTails = true, bool waitForBufferToUnqueue = true);	//	��~
	void Volume(float volume);												//	�{�����[���ݒ�
	bool Queuing();

	IXAudio2* GetXAudio2() { return xaudio2_.Get(); }

private:
	WAVEFORMATEXTENSIBLE wfx_ = { 0 };
	XAUDIO2_BUFFER buffer_ = { 0 };

	//	�\�[�X�{�C�X
	//	�e�T�E���h�̔g�`���󂯎���čĐ�����@�\���������I�u�W�F�N�g
	//	�eSE��BGM���ƂɃ\�[�X�{�C�X���쐬���ĊǗ����邽�߁A�Œ�ł������ɖ炷�T�E���h�����̃{�C�X���K�v
	IXAudio2SourceVoice* sourceVoice_;

	Microsoft::WRL::ComPtr<IXAudio2> xaudio2_;
	IXAudio2MasteringVoice* masterVoice_= nullptr;

};

HRESULT FindChunk(HANDLE hfile, DWORD fourcc, DWORD& chunkSize, DWORD& chunkDataPosition);
HRESULT ReadChunkData(HANDLE hFile, LPVOID buffer, DWORD bufferSize, DWORD bufferOffset);

