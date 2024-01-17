#pragma once

#include <d3d11.h>
#include <windows.h>
#include <tchar.h>
#include <sstream>
#include <DirectXMath.h>
#include <wrl.h>

#include "../Others/Misc.h"
#include "../Core/HighResolutionTimer.h"
#include "../Graphics/Graphics.h"
#include "../../External/Imgui/ImGuiCtrl.h"


CONST LONG SCREEN_WIDTH{ 1280 };
CONST LONG SCREEN_HEIGHT{ 720 };
CONST BOOL FULLSCREEN{ FALSE };
CONST LPCWSTR APPLICATION_NAME{ L"TAI" };

class Framework
{
public:
	Framework(HWND hwnd);
	~Framework();
	Framework(const Framework&) = delete;
	Framework& operator=(const Framework&) = delete;
	Framework(Framework&&) noexcept = delete;
	Framework& operator=(Framework&&) noexcept = delete;

	int run()
	{
		MSG msg{};

		if (!Initialize())
		{
			return 0;
		}

		IMGUI_CTRL_INITIALIZE(hwnd_, Graphics::Instance().GetDevice(), Graphics::Instance().GetDeviceContext());

		while (WM_QUIT != msg.message)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				tictoc_.Tick();
				CalculateFrameStats();
				Update(tictoc_.TimeInterval());
				Render();
			}
		}

		IMGUI_CTRL_UNINITIALIZE();

#if 1
		BOOL fullscreen_ = 0;
		graphics_.GetSwapChain()->GetFullscreenState(&fullscreen_, 0);
		if (fullscreen_)
		{
			graphics_.GetSwapChain()->SetFullscreenState(FALSE, 0);
		}
#endif

		return Uninitialize() ? static_cast<int>(msg.wParam) : 0;
	}

	LRESULT CALLBACK HandleMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
#ifdef USE_IMGUI
		IMGUI_CTRL_WND_PRC_HANDLER(hwnd, msg, wparam, lparam);
#endif
		switch (msg)
		{
		case WM_PAINT:
		{
			PAINTSTRUCT ps{};
			BeginPaint(hwnd, &ps);

			EndPaint(hwnd, &ps);
		}
		break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_CREATE:
			break;
		case WM_KEYDOWN:
			if (wparam == VK_ESCAPE)
			{
				PostMessage(hwnd, WM_CLOSE, 0, 0);
			}
			break;
		case WM_ENTERSIZEMOVE:
			tictoc_.Stop();
			break;
		case WM_EXITSIZEMOVE:
			tictoc_.Start();
			break;
		default:
			return DefWindowProc(hwnd, msg, wparam, lparam);
		}
		return 0;
	}

private:
	bool Initialize();
	void Update(float elapsedTime/*Elapsed seconds from last frame*/);
	void Render();
	bool Uninitialize();
	void CalculateFrameStats()
	{
		if (++frames_, (tictoc_.TimeStamp() - elapsedTime_) >= 1.0f)
		{
			float fps = static_cast<float>(frames_);
			std::wostringstream outs;
			outs.precision(6);
			outs << APPLICATION_NAME << L" : FPS : " << fps << L" / " << L"Frame Time : " << 1000.0f / fps << L" (ms)";
			SetWindowTextW(hwnd_, outs.str().c_str());

			frames_ = 0;
			elapsedTime_ += 1.0f;
		}
	}

public:
	CONST HWND hwnd_;
	Graphics graphics_;

private:
	HighResolutionTimer tictoc_;
	uint32_t frames_{ 0 };
	float elapsedTime_{ 0.0f };


};

