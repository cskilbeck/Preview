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

void PixelShader::Activate(ID3D11DeviceContext *context)
{
	context->PSSetShader(pixelShader, null, 0);
	context->PSSetConstantBuffers(0, (uint)mBuffers.size(), mBuffers.empty() ? null : &mBuffers[0]);
	context->PSSetShaderResources(0, (uint)mTextures.size(), mTextures.empty() ? null : &mTextures[0]);
	context->PSSetSamplers(0, (uint)mSamplers.size(), mSamplers.empty() ? null : &mSamplers[0]);
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