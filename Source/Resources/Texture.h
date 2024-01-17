#pragma once

#include <WICTextureLoader.h>
#include <wrl.h>
#include <string>
#include <map>

#include "../Others/Misc.h"

HRESULT LoadTextureFromFile(ID3D11Device* device, const wchar_t* filename,
	ID3D11ShaderResourceView** shaderResourceView, D3D11_TEXTURE2D_DESC* texture2dDesc);

void ReleaseAllTextures();