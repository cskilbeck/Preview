//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////

void Shader::DeleteConstantBuffers()
{
	for(auto i = mConstantBuffers.begin(); i != mConstantBuffers.end(); ++i)
	{
		delete i->second;
	}
	for(auto i = mBuffers.begin(); i != mBuffers.end(); ++i)
	{
		(*i)->Release();
	}
	mConstantBuffers.clear();
	mBuffers.clear();
}

//////////////////////////////////////////////////////////////////////

HRESULT Shader::CreateConstantBuffers(void const *blob, size_t size)
{
	DeleteConstantBuffers();

	DXPtr<ID3D11ShaderReflection> vsr;
	DX(D3DReflect(blob, size, IID_ID3D11ShaderReflection, (void **)&vsr));

	D3D11_SHADER_DESC desc;
	vsr->GetDesc(&desc);

	for(uint i = 0; i < desc.ConstantBuffers; ++i)
	{
		CBuffer *cb = new CBuffer();
		DX(cb->Create(vsr->GetConstantBufferByIndex(i)));
		mConstantBuffers[cb->Name] = cb;
		mBuffers.push_back(cb->mConstantBuffer);
	}
	return S_OK;
}

