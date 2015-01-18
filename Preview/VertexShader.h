//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct VertexShader: Shader
{
	DXPtr<ID3D11VertexShader>	vertexShader;

	virtual ~VertexShader() override;
	virtual HRESULT Destroy() override;

	HRESULT Create(Resource const &blob, field_definition const inputDesc[], int numElements);
	void Activate(ID3D11DeviceContext *context);
	void LoadConstants(ID3D11DeviceContext *context);
};

