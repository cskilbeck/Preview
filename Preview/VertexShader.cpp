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

HRESULT VertexShader::CreateInputLayoutDesc(void const *blob, size_t size, vector<D3D11_INPUT_ELEMENT_DESC> &ied)
{
	DXPtr<ID3D11ShaderReflection> vsr;
	D3D11_SHADER_DESC sd;
	DX(D3DReflect(blob, size, IID_ID3D11ShaderReflection, (void **)&vsr));
	vsr->GetDesc(&sd);
	ied.clear();
	ied.resize(sd.InputParameters);
	for(uint i = 0; i < sd.InputParameters; ++i)
	{
		D3D11_INPUT_ELEMENT_DESC &d = ied[i];
		D3D11_SIGNATURE_PARAMETER_DESC pd;
		vsr->GetInputParameterDesc(i, &pd);
		ZeroMemory(&d, sizeof(d));
		d.SemanticName = pd.SemanticName;
		d.SemanticIndex = pd.SemanticIndex;
		d.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		d.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		d.Format = formats[HighBit(pd.Mask) - 1][pd.ComponentType];
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT VertexShader::Create(void const *blob, size_t size)
{
	vertexShader.Release();
	vertexLayout.Release();

	DX(gDevice->CreateVertexShader(blob, size, null, &vertexShader));
	DX(CreateConstantBuffers(blob, size));
	std::vector<D3D11_INPUT_ELEMENT_DESC> ied;
	DX(CreateInputLayoutDesc(blob, size, ied));
	DX(gDevice->CreateInputLayout(&ied[0], (uint)ied.size(), blob, size, &vertexLayout));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

void VertexShader::Activate(DXPtr<ID3D11DeviceContext> context)
{
	context->VSSetShader(vertexShader, NULL, 0);
	//context->VSSetConstantBuffers(0, NULL
}

