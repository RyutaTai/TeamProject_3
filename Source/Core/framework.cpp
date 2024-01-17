#include "Framework.h"

#include <dxgi.h>

#include "../Graphics/Graphics.h"
#include "../Graphics/Shader.h"
#include "../Scenes/SceneManager.h"
#include "../Scenes/SceneTitle.h"

//	コンストラクタ
Framework::Framework(HWND hwnd) 
	: hwnd_(hwnd),
	graphics_(hwnd)
{
}

//	デストラクタ
Framework::~Framework()
{

}

//	初期化
bool Framework::Initialize()
{
	HRESULT hr{ S_OK };

	// ADAPTER
	Microsoft::WRL::ComPtr <IDXGIFactory> factory;
	CreateDXGIFactory(IID_PPV_ARGS(factory.GetAddressOf()));
	Microsoft::WRL::ComPtr <IDXGIAdapter> adapter;
	for (UINT adapterIndex = 0; S_OK == factory->EnumAdapters(adapterIndex, adapter.GetAddressOf()); ++adapterIndex) {
		DXGI_ADAPTER_DESC adapterDesc;
		adapter->GetDesc(&adapterDesc);
		if (adapterDesc.VendorId == 0x1002/*AMD*/ || adapterDesc.VendorId == 0x10DE/*NVIDIA*/)
		{
			break;
		}
	}

	//	シーン初期化
	SceneManager::Instance().ChangeScene(new SceneTitle);

	return true;
}

//	更新処理
void Framework::Update(float elapsedTime/*Elapsed seconds from last frame*/)
{
	IMGUI_CTRL_CLEAR_FRAME();

	//	シーンの更新
	SceneManager::Instance().Update(elapsedTime);

}

//	描画処理
void Framework::Render()
{
	FLOAT color[]{ 0, 1, 1, 1 };

	ID3D11RenderTargetView* renderTargetView = Graphics::Instance().GetRenderTargetView();
	graphics_.GetDeviceContext()->ClearRenderTargetView(Graphics::Instance().GetRenderTargetView(), color);
#if 0
	graphics_.GetDeviceContext()->ClearDepthStencilView(Graphics::Instance().GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
#endif
	graphics_.GetDeviceContext()->OMSetRenderTargets(1, &renderTargetView, Graphics::Instance().GetDepthStencilView());

	// サンプラーステートセット呼び出しここだけでいい
	graphics_.GetShader()->SetSamplerState(graphics_.GetDeviceContext());

	//	シーンの描画
	SceneManager::Instance().Render();

	IMGUI_CTRL_DISPLAY();

	UINT syncInterval{ 0 };

	//	Present()で表示するバッファをスワップさせ、画面に表示する
	//	(FrontバッファとBackバッファを入れ替えて画面に表示)
	Graphics::Instance().GetSwapChain()->Present(syncInterval, 0);

}

//	終了化
bool Framework::Uninitialize()
{
	return true;
}
