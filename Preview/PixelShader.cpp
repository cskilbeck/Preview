//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////

HRESULT PixelShader::Create(void const *blob, size_t size)
{
	pixelShader.Release();

	DX(gDevice->CreatePixelShader(blob, size, null, &pixelShader));
	DX(CreateConstantBuffers(blob, size));
	return S_OK;
}

void PixelShader::Activate(DXPtr<ID3D11DeviceContext> context)
{
}