
#pragma once

//////////////////////////////////////////////////////////////////////

struct Material
{
	Material();
	~Material();

	HRESULT Create();
	HRESULT Destroy();
	void Activate(DXPtr<ID3D11DeviceContext> context);

	HRESULT SetPixelShader(PixelShader *p);
	HRESULT SetVertexShader(VertexShader *v);

	HRESULT CreateSampler();
	HRESULT CreateRasterizerState();
	HRESULT CreateBlendState();
	HRESULT CreateDepthStencilState();

	Vector<Texture *>				mTextures;		// pointers to all the textures that this material needs...

	DXPtr<ID3D11RasterizerState>	rasterizerState;
	DXPtr<ID3D11BlendState>			blendState;
	DXPtr<ID3D11DepthStencilState>	mDepthStencilState;
};
