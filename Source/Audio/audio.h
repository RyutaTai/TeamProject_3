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

	//	唯一のインスタンス取得
	static Audio& Instance()
	{
		static Audio instance_;
		return instance_;
	}

	void Initialize();

	void Play(int loopCount = 0/*255 : XAUDIO2_LOOP_INFINITE*/);			//	再生
	void Stop(bool playTails = true, bool waitForBufferToUnqueue = true);	//	停止
	void Volume(float volume);												//	ボリューム設定
	bool Queuing();

	IXAudio2* GetXAudio2() { return xaudio2_.Get(); }

private:
	WAVEFORMATEXTENSIBLE wfx_ = { 0 };
	XAUDIO2_BUFFER buffer_ = { 0 };

	//	ソースボイス
	//	各サウンドの波形を受け取って再生する機能を持ったオブジェクト
	//	各SEやBGMごとにソースボイスを作成して管理するため、最低でも同時に鳴らすサウンド数分のボイスが必要
	IXAudio2SourceVoice* sourceVoice_;

	Microsoft::WRL::ComPtr<IXAudio2> xaudio2_;
	IXAudio2MasteringVoice* masterVoice_= nullptr;

};

HRESULT FindChunk(HANDLE hfile, DWORD fourcc, DWORD& chunkSize, DWORD& chunkDataPosition);
HRESULT ReadChunkData(HANDLE hFile, LPVOID buffer, DWORD bufferSize, DWORD bufferOffset);

