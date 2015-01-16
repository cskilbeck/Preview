//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct PixelShader: Shader
{
	virtual ~PixelShader() override;
	virtual HRESULT Destroy() override;

	DXPtr<ID3D11PixelShader>	pixelShader;

	HRESULT Create(Resource const &blob);
	void Activate(ID3D11DeviceContext *context);
};
