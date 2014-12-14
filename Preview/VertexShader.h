//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct VertexShader: Shader
{
	DXPtr<ID3D11VertexShader>	vertexShader;
	DXPtr<ID3D11InputLayout>	vertexLayout;

	virtual ~VertexShader() override;
	virtual HRESULT Destroy() override;

	HRESULT CreateInputLayout(void const *blob, size_t size);
	HRESULT Create(void const *blob, size_t size);
	void Activate(ID3D11DeviceContext *context);
	void LoadConstants(ID3D11DeviceContext *context);
};
