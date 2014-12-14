//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////

static DXGI_FORMAT formats[4][4] =
{
	{
		DXGI_FORMAT_R32_TYPELESS,
		DXGI_FORMAT_R32_UINT,
		DXGI_FORMAT_R32_SINT,
		DXGI_FORMAT_R32_FLOAT
	},
	{
		DXGI_FORMAT_R32G32_TYPELESS,
		DXGI_FORMAT_R32G32_UINT,
		DXGI_FORMAT_R32G32_SINT,
		DXGI_FORMAT_R32G32_FLOAT
	},
	{
		DXGI_FORMAT_R32G32B32_TYPELESS,
		DXGI_FORMAT_R32G32B32_UINT,
		DXGI_FORMAT_R32G32B32_SINT,
		DXGI_FORMAT_R32G32B32_FLOAT
	},
	{
		DXGI_FORMAT_R32G32B32A32_TYPELESS,
		DXGI_FORMAT_R32G32B32A32_UINT,
		DXGI_FORMAT_R32G32B32A32_SINT,
		DXGI_FORMAT_R32G32B32A32_FLOAT
	}
};

//////////////////////////////////////////////////////////////////////

HRESULT VertexShader::CreateInputLayout(void const *blob, size_t size)
{
	vector<D3D11_INPUT_ELEMENT_DESC> ied;
	ied.resize(mShaderDesc.InputParameters);
	for(uint i = 0; i < mShaderDesc.InputParameters; ++i)
	{
		D3D11_INPUT_ELEMENT_DESC &d = ied[i];
		D3D11_SIGNATURE_PARAMETER_DESC pd;
		mReflector->GetInputParameterDesc(i, &pd);
		ZeroMemory(&d, sizeof(d));
		d.SemanticName = pd.SemanticName;
		d.SemanticIndex = pd.SemanticIndex;
		d.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		d.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		d.Format = formats[HighBit(pd.Mask) - 1][pd.ComponentType];
	}
	vertexLayout.Release();
	DX(gDevice->CreateInputLayout(&ied[0], (uint)ied.size(), blob, size, &vertexLayout));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT VertexShader::Create(void const *blob, size_t size)
{
	TRACE(TEXT("VertexShader:\n"));
	DX(Shader::Create(blob, size));
	vertexShader.Release();
	DX(gDevice->CreateVertexShader(blob, size, null, &vertexShader));
	DX(CreateInputLayout(blob, size));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

void VertexShader::LoadConstants(ID3D11DeviceContext *context)
{
	context->VSSetConstantBuffers(0, (uint)mBuffers.size(), &mBuffers[0]);
}

//////////////////////////////////////////////////////////////////////

void VertexShader::Activate(ID3D11DeviceContext *context)
{
	context->VSSetShader(vertexShader, NULL, 0);
	LoadConstants(context);
}

HRESULT VertexShader::Destroy()
{
	vertexShader.Release();
	vertexLayout.Release();
	return Shader::Destroy();
}

VertexShader::~VertexShader()
{
	Destroy();
}