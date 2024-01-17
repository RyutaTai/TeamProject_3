#include "Graphics.h"

#include "../Core/Framework.h"

Graphics* Graphics::instance_ = nullptr;

//	コンストラクタ
Graphics::Graphics(HWND hwnd)
{
	_ASSERT_EXPR(instance_ == instance_, "already instance");
	instance_ = this;

	HRESULT hr{ S_OK };

	UINT createDeviceFlags{ 0 };
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	//	SwapChain
	D3D_FEATURE_LEVEL featureLevels{ D3D_FEATURE_LEVEL_11_0 };
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = SCREEN_WIDTH;
	swapChainDesc.BufferDesc.Height = SCREEN_HEIGHT;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = !FULLSCREEN;
	hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags,
		&featureLevels, 1, D3D11_SDK_VERSION, &swapChainDesc,
		swapChain_.GetAddressOf(), device_.GetAddressOf(), NULL, deviceContext_.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	//	DepthStencilView,Texture2dDesc
#if 0
	Microsoft::WRL::ComPtr <ID3D11Texture2D> depthStencilBuffer{};
	D3D11_TEXTURE2D_DESC texture2dDesc{};
	texture2dDesc.Width = SCREEN_WIDTH;
	texture2dDesc.Height = SCREEN_HEIGHT;
	texture2dDesc.MipLevels = 1;
	texture2dDesc.ArraySize = 1;
	texture2dDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	texture2dDesc.SampleDesc.Count = 1;
	texture2dDesc.SampleDesc.Quality = 0;
	texture2dDesc.Usage = D3D11_USAGE_DEFAULT;
	texture2dDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	texture2dDesc.CPUAccessFlags = 0;
	texture2dDesc.MiscFlags = 0;
	hr = device_->CreateTexture2D(&texture2dDesc, NULL, depthStencilBuffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = texture2dDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	hr = device_->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilViewDesc, depthStencilView_.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
#endif

	//	RenderTargetView
	Microsoft::WRL::ComPtr <ID3D11Texture2D> backBuffer{};
	hr = swapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(backBuffer.GetAddressOf()));
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	hr = device_->CreateRenderTargetView(backBuffer.Get(), NULL, renderTargetView_.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	//	ViewPort
	D3D11_VIEWPORT viewport{};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(SCREEN_WIDTH);
	viewport.Height = static_cast<float>(SCREEN_HEIGHT);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	deviceContext_->RSSetViewports(1, &viewport);

	//	シーン定数バッファ
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = sizeof(SceneConstants);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	hr = device_->CreateBuffer(&bufferDesc, nullptr, constantBuffers_[0].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	//	FrameBuffer
	frameBuffers_[0] = std::make_unique<FrameBuffer>(device_.Get(), 1280, 720);
	frameBuffers_[1] = std::make_unique<FrameBuffer>(device_.Get(), 1280 / 2, 720 / 2);
	bitBlockTransfer_ = std::make_unique<FullScreenQuad>(device_.Get());

	shader_ = std::make_unique<Shader>();
}

//	デストラクタ
Graphics::~Graphics()
{
}
