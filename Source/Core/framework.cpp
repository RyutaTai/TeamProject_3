#include "Framework.h"

#include <dxgi.h>

#include "../Graphics/Graphics.h"
#include "../Graphics/Shader.h"
#include "../Scenes/SceneManager.h"
#include "../Scenes/SceneTitle.h"

//	�R���X�g���N�^
Framework::Framework(HWND hwnd) 
	: hwnd_(hwnd),
	graphics_(hwnd)
{
}

//	�f�X�g���N�^
Framework::~Framework()
{

}

//	������
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

	//	�V�[��������
	SceneManager::Instance().ChangeScene(new SceneTitle);

	return true;
}

//	�X�V����
void Framework::Update(float elapsedTime/*Elapsed seconds from last frame*/)
{
	IMGUI_CTRL_CLEAR_FRAME();

	//	�V�[���̍X�V
	SceneManager::Instance().Update(elapsedTime);

}

//	�`�揈��
void Framework::Render()
{
	FLOAT color[]{ 0, 1, 1, 1 };

	ID3D11RenderTargetView* renderTargetView = Graphics::Instance().GetRenderTargetView();
	graphics_.GetDeviceContext()->ClearRenderTargetView(Graphics::Instance().GetRenderTargetView(), color);
#if 0
	graphics_.GetDeviceContext()->ClearDepthStencilView(Graphics::Instance().GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
#endif
	graphics_.GetDeviceContext()->OMSetRenderTargets(1, &renderTargetView, Graphics::Instance().GetDepthStencilView());

	// �T���v���[�X�e�[�g�Z�b�g�Ăяo�����������ł���
	graphics_.GetShader()->SetSamplerState(graphics_.GetDeviceContext());

	//	�V�[���̕`��
	SceneManager::Instance().Render();

	IMGUI_CTRL_DISPLAY();

	UINT syncInterval{ 0 };

	//	Present()�ŕ\������o�b�t�@���X���b�v�����A��ʂɕ\������
	//	(Front�o�b�t�@��Back�o�b�t�@�����ւ��ĉ�ʂɕ\��)
	Graphics::Instance().GetSwapChain()->Present(syncInterval, 0);

}

//	�I����
bool Framework::Uninitialize()
{
	return true;
}
