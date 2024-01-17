#include "Audio.h"

#include <windows.h>
#include <winerror.h>

//	コンストラクタ
Audio::Audio(IXAudio2* xaudio2, const wchar_t* filename)
{
	HRESULT hr;

	// Open the file
	HANDLE hfile = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (INVALID_HANDLE_VALUE == hfile)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	if (INVALID_SET_FILE_POINTER == SetFilePointer(hfile, 0, NULL, FILE_BEGIN))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	DWORD chunkSize;
	DWORD chunkPosition;
	//	check the file type, should be 'WAVE' or 'XWMA'
	FindChunk(hfile, 'FFIR'/*RIFF*/, chunkSize, chunkPosition);
	DWORD filetype;
	ReadChunkData(hfile, &filetype, sizeof(DWORD), chunkPosition);
	_ASSERT_EXPR(filetype == 'EVAW'/*WAVE*/, L"Only support 'WAVE'");

	FindChunk(hfile, ' tmf'/*FMT*/, chunkSize, chunkPosition);
	ReadChunkData(hfile, &wfx_, chunkSize, chunkPosition);

	//	fill out the audio data buffer with the contents of the fourccDATA chunk
	FindChunk(hfile, 'atad'/*DATA*/, chunkSize, chunkPosition);
	BYTE* data = new BYTE[chunkSize];
	ReadChunkData(hfile, data, chunkSize, chunkPosition);

	buffer_.AudioBytes = chunkSize;			//	size of the audio buffer in bytes
	buffer_.pAudioData = data;				//	buffer containing audio data
	buffer_.Flags = XAUDIO2_END_OF_STREAM;	//	tell the source voice not to expect any data after this buffer

	hr = xaudio2->CreateSourceVoice(&sourceVoice_, (WAVEFORMATEX*)&wfx_);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
}

//	デストラクタ
Audio::~Audio()
{
	sourceVoice_->DestroyVoice();
	delete[] buffer_.pAudioData;
}

//	初期化
void Audio::Initialize()
{
	HRESULT hr{ S_OK };
	hr = XAudio2Create(xaudio2_.GetAddressOf(), 0, XAUDIO2_DEFAULT_PROCESSOR);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	hr = xaudio2_->CreateMasteringVoice(&masterVoice_);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
}

//	再生
void Audio::Play(int loopCount)
{
	HRESULT hr;

	XAUDIO2_VOICE_STATE voice_state = {};
	sourceVoice_->GetState(&voice_state);

	if (voice_state.BuffersQueued)
	{

		//Stop(false, 0);
		return;
	}

	buffer_.LoopCount = loopCount;
	hr = sourceVoice_->SubmitSourceBuffer(&buffer_);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	hr = sourceVoice_->Start(0);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
}

//	停止
void Audio::Stop(bool playTails/*Continue emitting effect output after the voice is stopped. */, bool waitForBufferToUnqueue)
{
	XAUDIO2_VOICE_STATE voiceState = {};
	sourceVoice_->GetState(&voiceState);
	if (!voiceState.BuffersQueued)
	{
		return;
	}

	HRESULT hr;
	hr = sourceVoice_->Stop(playTails ? XAUDIO2_PLAY_TAILS : 0);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	hr = sourceVoice_->FlushSourceBuffers();
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	while (waitForBufferToUnqueue && voiceState.BuffersQueued)
	{
		sourceVoice_->GetState(&voiceState);
	}
}

//	ボリューム設定
void Audio::Volume(float volume)
{
	HRESULT hr = sourceVoice_->SetVolume(volume);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
}

bool Audio::Queuing()
{
	XAUDIO2_VOICE_STATE voiceState = {};
	sourceVoice_->GetState(&voiceState);
	return voiceState.BuffersQueued;
}

HRESULT FindChunk(HANDLE hfile, DWORD fourcc, DWORD& chunkSize, DWORD& chunkDataPosition)
{
	HRESULT hr = S_OK;

	if (INVALID_SET_FILE_POINTER == SetFilePointer(hfile, 0, NULL, FILE_BEGIN))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}
	//	DWORD : Double WORD 32bit(4バイト)整数型
	DWORD chunkType;
	DWORD chunkDataSize;
	DWORD riffDataSize = 0;
	DWORD fileType;
	DWORD bytesRead = 0;
	DWORD offset = 0;

	while (hr == S_OK)
	{
		DWORD numberOfBytesRead;
		if (0 == ReadFile(hfile, &chunkType, sizeof(DWORD), &numberOfBytesRead, NULL))
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
		}

		if (0 == ReadFile(hfile, &chunkDataSize, sizeof(DWORD), &numberOfBytesRead, NULL))
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
		}

		switch (chunkType)
		{
		case 'FFIR'/*RIFF*/:
			riffDataSize = chunkDataSize;
			chunkDataSize = 4;
			if (0 == ReadFile(hfile, &fileType, sizeof(DWORD), &numberOfBytesRead, NULL))
			{
				hr = HRESULT_FROM_WIN32(GetLastError());
			}
			break;

		default:
			if (INVALID_SET_FILE_POINTER == SetFilePointer(hfile, chunkDataSize, NULL, FILE_CURRENT))
			{
				return HRESULT_FROM_WIN32(GetLastError());
			}
		}

		offset += sizeof(DWORD) * 2;

		if (chunkType == fourcc)
		{
			chunkSize = chunkDataSize;
			chunkDataPosition = offset;
			return S_OK;
		}

		offset += chunkDataSize;

		if (bytesRead >= riffDataSize)
		{
			return S_FALSE;
		}
	}

	return S_OK;

}

HRESULT ReadChunkData(HANDLE hFile, LPVOID buffer, DWORD bufferSize, DWORD bufferOffset)
{
	HRESULT hr = S_OK;
	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferOffset, NULL, FILE_BEGIN))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}
	DWORD numberOfBytesRead;
	if (0 == ReadFile(hFile, buffer, bufferSize, &numberOfBytesRead, NULL))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
	}
	return hr;
}
