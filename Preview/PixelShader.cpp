//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////

HRESULT PixelShader::Create(void const *blob, size_t size)
{
	TRACE(TEXT("PixelShader:\n"));
	DX(Shader::Create(blob, size));

	pixelShader.Release();
	DX(gDevice->CreatePixelShader(blob, size, null, &pixelShader));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

void PixelShader::LoadConstants(ID3D11DeviceContext *context)
{
	context->PSSetConstantBuffers(0, (uint)mBuffers.size(), &mBuffers[0]);
}

//////////////////////////////////////////////////////////////////////

void PixelShader::Activate(ID3D11DeviceContext *context)
{
	context->PSSetShader(pixelShader, null, 0);
	LoadConstants(context);
}

//////////////////////////////////////////////////////////////////////

HRESULT PixelShader::Destroy()
{
	pixelShader.Release();
	return Shader::Destroy();
}

PixelShader::~PixelShader()
{
	Destroy();
}