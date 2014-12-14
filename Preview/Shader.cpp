//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////

Shader::Shader()
	: mStartIndex(0)
{
}

//////////////////////////////////////////////////////////////////////

Shader::~Shader()
{
	Destroy();
}

//////////////////////////////////////////////////////////////////////

template <typename T> static void AddAt(vector<T> &v, uint index, T const &item)
{
	if(index >= (uint)v.size())
	{
		v.resize(index + 1);
	}
	v[index] = item;
}

//////////////////////////////////////////////////////////////////////

void Shader::DeleteConstantBuffers()
{
	mConstBufferIDs.clear();
	for(auto i = mConstantBuffers.begin(); i != mConstantBuffers.end(); ++i)
	{
		delete (*i);
	}
	mConstantBuffers.clear();
	mBuffers.clear();
}

//////////////////////////////////////////////////////////////////////

HRESULT Shader::CreateConstantBuffer(D3D11_SHADER_INPUT_BIND_DESC desc)
{
	TRACE("  ConstantBuffer: %s at %d\n", desc.Name, desc.BindPoint);
	ConstantBuffer *cb = new ConstantBuffer();
	uint i = desc.BindPoint;
	DX(cb->Create(mReflector->GetConstantBufferByIndex(i)));
	mConstBufferIDs[string(cb->Name)] = i;
	TRACE("===>mConstBufferIDs[%s] = %d\n", cb->Name, i);
	AddAt(mConstantBuffers, i, cb);
	AddAt(mBuffers, i, cb->mConstantBuffer);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT Shader::Create(void const *blob, size_t size)
{
	DX(D3DReflect(blob, size, IID_ID3D11ShaderReflection, (void **)&mReflector));
	mReflector->GetDesc(&mShaderDesc);
	DX(CreateBindings());
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT Shader::Destroy()
{
	mReflector.Release();
	DeleteConstantBuffers();
	mTextureIDs.clear();
	mSamplerIDs.clear();
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT Shader::CreateBindings()
{
	uint numBindingSlots = mShaderDesc.BoundResources;

	for(uint i = 0; i < numBindingSlots; ++i)
	{
		D3D11_SHADER_INPUT_BIND_DESC d;
		mReflector->GetResourceBindingDesc(i, &d);
		TRACE("Binding %d\n", i);
		DX(CreateBinding(d));
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT Shader::CreateTextureBinding(D3D11_SHADER_INPUT_BIND_DESC desc)
{
	TRACE("  Texture: %s at %d\n", desc.Name, desc.BindPoint);
	AddAt(mTextures, desc.BindPoint, (ID3D11ShaderResourceView *)null);
	mTextureIDs[string(desc.Name)] = desc.BindPoint;
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT Shader::CreateSamplerBinding(D3D11_SHADER_INPUT_BIND_DESC desc)
{
	TRACE("  Sampler: %s at %d\n", desc.Name, desc.BindPoint);
	AddAt(mSamplers, desc.BindPoint, (ID3D11SamplerState *)null);
	mSamplerIDs[desc.Name] = desc.BindPoint;
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT Shader::CreateBinding(D3D11_SHADER_INPUT_BIND_DESC desc)
{
	switch(desc.Type)
	{
		case D3D_SHADER_INPUT_TYPE::D3D10_SIT_TEXTURE: return CreateTextureBinding(desc);
		case D3D_SHADER_INPUT_TYPE::D3D10_SIT_SAMPLER: return CreateSamplerBinding(desc);
		case D3D_SHADER_INPUT_TYPE::D3D10_SIT_CBUFFER: return CreateConstantBuffer(desc);
		default: return ERROR_BAD_ARGUMENTS;
	}
}

