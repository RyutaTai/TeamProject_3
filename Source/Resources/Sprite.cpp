#include "sprite.h"

#include "../Graphics/Graphics.h"
#include "../../imgui/ImGuiCtrl.h"

//	コンストラクタ
Sprite::Sprite(ID3D11Device* device, const wchar_t* filename)
{
	HRESULT hr = S_OK;

	Vertex vertices[]
	{
		{ { -1.0, +1.0, 0 }, { 1, 1, 1, 1 }, { 0, 0 } },//左上
		{ { +1.0, +1.0, 0 }, { 1, 1, 1, 1 }, { 1, 0 } },//右上
		{ { -1.0, -1.0, 0 }, { 1, 1, 1, 1 }, { 0, 1 } },//左下
		{ { +1.0, -1.0, 0 }, { 1, 1, 1, 1 }, { 1, 1 } },//右下
	};

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = sizeof(vertices);
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA subresourceData{};
	subresourceData.pSysMem = vertices;
	subresourceData.SysMemPitch = 0;
	subresourceData.SysMemSlicePitch = 0;
	
	hr = device->CreateBuffer(&bufferDesc, &subresourceData, vertexBuffer_.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[]
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,	 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",	  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,		 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	// シェーダー読み込み
	Shader* shader = Graphics::Instance().GetShader();
	hr = shader->CreateVsFromCso(device, "./Resources/Shader/SpriteVs.cso", vertexShader_.GetAddressOf(), inputLayout_.GetAddressOf(), inputElementDesc, _countof(inputElementDesc));	
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	hr = shader->CreatePsFromCso(device, "./Resources/Shader/SpritePs.cso", pixelShader_.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	
	// テクスチャ読み込み
	LoadTextureFromFile(device, filename, shaderResourceView_.GetAddressOf(), &texture2dDesc_);

	GetTransform()->SetSize(texture2dDesc_.Width, texture2dDesc_.Height);
	GetTransform()->SetTexSize(texture2dDesc_.Width, texture2dDesc_.Height);
}

//	デストラクタ
Sprite::~Sprite()
{
	
}

//	描画処理
void Sprite::Render()
{
	Graphics& graphics = Graphics::Instance();

	D3D11_VIEWPORT viewport{};
	UINT numViewports{ 1 };

	graphics.GetDeviceContext()->RSGetViewports(&numViewports, &viewport);

	//left-top
	float x0{ GetTransform()->GetPositionX() };
	float y0{ GetTransform()->GetPositionY() };
	//right-top
	float x1{ GetTransform()->GetPositionX() + GetTransform()->GetSizeX() };
	float y1{ GetTransform()->GetPositionY() };
	//left-bottom
	float x2{ GetTransform()->GetPositionX() };
	float y2{ GetTransform()->GetPositionY() + GetTransform()->GetSizeY() };
	//right-bottom
	float x3{ GetTransform()->GetPositionX() + GetTransform()->GetSizeX() };
	float y3{ GetTransform()->GetPositionY() + GetTransform()->GetSizeY() };

	auto rotate = [](float& x, float& y, float cx, float cy, float angle)
	{
		x -= cx;
		y -= cy;

		float cos{ cosf(DirectX::XMConvertToRadians(angle)) };
		float sin{ sinf(DirectX::XMConvertToRadians(angle)) };
		float tx{ x }, ty{ y };
		x = cos * tx + -sin * ty;
		y = sin * tx + cos * ty;

		x += cx;
		y += cy;
	};

#if 0	//	回転の中心を矩形の中心にする場合
	float cx = GetTransform()->GetPositionX() + GetTransform()->GetSizeX() * 0.5f;
	float cy = GetTransform()->GetPositionY() + GetTransform()->GetSizeY() * 0.5f;

#else	//	回転の中心を左上にする場合
	float cx = GetTransform()->GetPositionX();
	float cy = GetTransform()->GetPositionY();
#endif

	rotate(x0, y0, cx, cy, GetTransform()->GetAngle());
	rotate(x1, y1, cx, cy, GetTransform()->GetAngle());
	rotate(x2, y2, cx, cy, GetTransform()->GetAngle());
	rotate(x3, y3, cx, cy, GetTransform()->GetAngle());

	//	screen space to NDC
	x0 = 2.0f * x0 / viewport.Width - 1.0f;
	y0 = 1.0f - 2.0f * y0 / viewport.Height;
	x1 = 2.0f * x1 / viewport.Width - 1.0f;
	y1 = 1.0f - 2.0f * y1 / viewport.Height;
	x2 = 2.0f * x2 / viewport.Width - 1.0f;
	y2 = 1.0f - 2.0f * y2 / viewport.Height;
	x3 = 2.0f * x3 / viewport.Width - 1.0f;
	y3 = 1.0f - 2.0f * y3 / viewport.Height;

	HRESULT hr{ S_OK };
	D3D11_MAPPED_SUBRESOURCE mappedSubresource{};
	hr = graphics.GetDeviceContext()->Map(vertexBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	Vertex* vertices{ reinterpret_cast<Vertex*>(mappedSubresource.pData) };
	if (vertices != nullptr)
	{
		vertices[0].position_ = { x0,y0,0 };
		vertices[1].position_ = { x1,y1,0 };
		vertices[2].position_ = { x2,y2,0 };
		vertices[3].position_ = { x3,y3,0 };
		vertices[0].color_ = vertices[1].color_ = vertices[2].color_ = vertices[3].color_ = GetTransform()->GetColor();

		float SX = GetTransform()->GetTexPosX()  / texture2dDesc_.Width;
		float SY = GetTransform()->GetTexPosY()  / texture2dDesc_.Height;
		float SW = GetTransform()->GetTexSizeX() / texture2dDesc_.Width;
		float SH = GetTransform()->GetTexSizeY() / texture2dDesc_.Height;

		vertices[0].texcoord_ = { SX, SY };
		vertices[1].texcoord_ = { SX + SW, SY };
		vertices[2].texcoord_ = { SX, SY + SH };
		vertices[3].texcoord_ = { SX + SW, SY + SH };

	}
	graphics.GetDeviceContext()->Unmap(vertexBuffer_.Get(), 0);

	UINT stride{ sizeof(Vertex) };
	UINT offset{ 0 };

	graphics.GetDeviceContext()->IASetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(), &stride, &offset);
	graphics.GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	graphics.GetDeviceContext()->IASetInputLayout(inputLayout_.Get());
	graphics.GetDeviceContext()->VSSetShader(vertexShader_.Get(), nullptr, 0);
	graphics.GetDeviceContext()->PSSetShader(pixelShader_.Get(), nullptr, 0);

	graphics.GetDeviceContext()->PSSetShaderResources(0, 1, shaderResourceView_.GetAddressOf());

	// 各種ステートの設定
	Graphics::Instance().GetShader()->SetDepthStencilState(Shader::DEPTH_STENCIL_STATE::ZT_OFF_ZW_OFF);
	Graphics::Instance().GetShader()->SetBlendState(Shader::BLEND_STATE::ALPHA);
	Graphics::Instance().GetShader()->SetRasterizerState(Shader::RASTERIZER_STATE::CULL_NONE);

	// --- 描画 (これより下に何も書かない) ---
	graphics.GetDeviceContext()->Draw(4, 0);
}

//	テキスト描画
void Sprite::Textout(std::string s,
	float x, float y, float w, float h, float r, float g, float b, float a)
{
	float sw = static_cast<float>(texture2dDesc_.Width / 16);
	float sh = static_cast<float>(texture2dDesc_.Height / 16);
	float carriage = 0;
	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();

	for (const char c : s)
	{
		GetTransform()->SetPosition(x + carriage, y);
		GetTransform()->SetSize(w, h);
		GetTransform()->SetTexPos(sw * (c & 0x0F), sh * (c >> 4));
		GetTransform()->SetTexSize(sw, sh);
		GetTransform()->SetColor(r, g, b, a);
		Render();
		carriage += w;
	}
}

//	デバッグ描画
void Sprite::DrawDebug()
{
	GetTransform()->DrawDebug();
}

//	デバッグ描画　
void Sprite::SpriteTransform::DrawDebug()
{
#if USE_IMGUI
	if (ImGui::TreeNode("Transform"))
	{
		ImGui::DragFloat2("position", &position_.x);
		ImGui::DragFloat2("size", &size_.x);
		ImGui::DragFloat2("texPos", &texPos_.x);
		ImGui::DragFloat2("texSize", &texSize_.x);
		
		ImGui::TreePop();
	}
#endif
}