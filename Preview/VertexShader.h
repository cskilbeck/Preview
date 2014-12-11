//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct VertexShader: Shader
{
	DXPtr<ID3D11VertexShader>	vertexShader;
	DXPtr<ID3D11InputLayout>	vertexLayout;

	HRESULT CreateInputLayoutDesc(void const *blob, size_t size, std::vector<D3D11_INPUT_ELEMENT_DESC> &ied);
	HRESULT Create(void const *blob, size_t size);
	void Activate(DXPtr<ID3D11DeviceContext> context);
};
