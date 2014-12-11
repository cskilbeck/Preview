
#pragma once

//////////////////////////////////////////////////////////////////////

struct Material
{
	Material();
	~Material();

	HRESULT Create();
	void Activate(DXPtr<ID3D11DeviceContext> context);

	HRESULT SetPixelShader(PixelShader *p);
	HRESULT SetVertexShader(VertexShader *v);

	HRESULT CreateSampler();
	HRESULT CreateRasterizerState();
	HRESULT CreateBlendState();
	HRESULT CreateDepthStencilState();

	Ptr<PixelShader>				mPixelShader;
	Ptr<VertexShader>				mVertexShader;

	DXPtr<ID3D11RasterizerState>	rasterizerState;
	DXPtr<ID3D11BlendState>			blendState;
	DXPtr<ID3D11DepthStencilState>	mDepthStencilState;

	// this is probably not in the right place...
	// and anyway, there might be more than 1
	DXPtr<ID3D11SamplerState>		sampler;
};
