#include "texture.h"

#include <filesystem>
#include <fstream>
#include <DDSTextureLoader.h>

static std::map<std::pmr::wstring, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>resources;

HRESULT LoadTextureFromFile(ID3D11Device* device, const wchar_t* filename,
	ID3D11ShaderResourceView** shaderResourceView, D3D11_TEXTURE2D_DESC* texture2dDesc)
{
	HRESULT hr{ S_OK };
	Microsoft::WRL::ComPtr<ID3D11Resource>resource;
	std::filesystem::path ddsFilename(filename);
	ddsFilename.replace_extension("dds");
	if (std::filesystem::exists(ddsFilename.c_str()))
	{
		hr = DirectX::CreateDDSTextureFromFile(device, ddsFilename.c_str(), resource.GetAddressOf(), shaderResourceView);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}
	else
	{
		hr = DirectX::CreateWICTextureFromFile(device, filename, resource.GetAddressOf(), shaderResourceView);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}
	resources.insert(std::make_pair(filename, *shaderResourceView));
	
	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2D;
	hr = resource.Get()->QueryInterface<ID3D11Texture2D>(texture2D.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	texture2D->GetDesc(texture2dDesc);

	return hr;
}

void ReleaseAllTextures()
{
	resources.clear();
}