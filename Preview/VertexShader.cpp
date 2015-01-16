//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////

HRESULT VertexShader::Create(Resource const &blob, field_definition const inputDesc[], int numElements)
{
	DX(Destroy());
	DX(Shader::Create(blob, blob.Size()));
	DX(gDevice->CreateVertexShader(blob, blob.Size(), null, &vertexShader));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

void VertexShader::Activate(ID3D11DeviceContext *context)
{
	context->VSSetConstantBuffers(0, (uint)mBuffers.size(), &mBuffers[0]);
	context->VSSetShader(vertexShader, null, 0);
}

//////////////////////////////////////////////////////////////////////

HRESULT VertexShader::Destroy()
{
	vertexShader.Release();
	return Shader::Destroy();
}

//////////////////////////////////////////////////////////////////////

VertexShader::~VertexShader()
{
	Destroy();
}
