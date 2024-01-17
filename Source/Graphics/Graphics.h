#pragma once

#include <wrl.h>
#include <memory>
#include <d3d11.h>
#include <DirectXMath.h>

#include "../Graphics/Framebuffer.h"
#include "../Graphics/FullScreenQuad.h"
#include "../Graphics/Shader.h"

class Graphics
{
public://ç\ë¢ëÃ
	struct SceneConstants
	{
		DirectX::XMFLOAT4X4 viewProjection_;
		DirectX::XMFLOAT4   lightDirection_;
		DirectX::XMFLOAT4   cameraPosition_;
	};

public:
	Graphics(HWND hwnd);
	~Graphics();

	static Graphics& Instance()
	{
		return *instance_;
	}

	ID3D11Device*			GetDevice()			  { return device_.Get(); }
	ID3D11DeviceContext*	GetDeviceContext()	  { return deviceContext_.Get(); }
	IDXGISwapChain*			GetSwapChain()		  { return swapChain_.Get(); }
	ID3D11RenderTargetView* GetRenderTargetView() { return renderTargetView_.Get(); }
	ID3D11DepthStencilView* GetDepthStencilView() { return depthStencilView_.Get(); }
	Shader*					GetShader()			  { return shader_.get(); }

private:
	std::unique_ptr<Shader> shader_ = nullptr;
	Microsoft::WRL::ComPtr <ID3D11Device>				device_;
	Microsoft::WRL::ComPtr <ID3D11DeviceContext>		deviceContext_;
	Microsoft::WRL::ComPtr <IDXGISwapChain>				swapChain_;
	Microsoft::WRL::ComPtr <ID3D11RenderTargetView>		renderTargetView_;
	Microsoft::WRL::ComPtr <ID3D11DepthStencilView>		depthStencilView_;

	//ConstantBuffer
	Microsoft::WRL::ComPtr <ID3D11Buffer>				constantBuffers_[8];

	std::unique_ptr <FrameBuffer>						frameBuffers_[8];
	std::unique_ptr <FullScreenQuad>					bitBlockTransfer_;

	static Graphics* instance_;

};

