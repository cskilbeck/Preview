//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Preview : Window
{
	Preview(int width, int height);
	~Preview();

	bool OnUpdate() override;
	void OnDraw() override;

	HRESULT LoadShaders();
	HRESULT CreateSampler();
	HRESULT CreateVertexBuffer();
	HRESULT CreateRasterizerState();
	HRESULT CreateBlendState();

	Ptr<Texture> mTexture;

	DXPtr<ID3D11InputLayout>		vertexLayout;
	DXPtr<ID3D11PixelShader>		pixelShader;
	DXPtr<ID3D11VertexShader>		vertexShader;
	DXPtr<ID3D11Buffer>				vertexBuffer;
	DXPtr<ID3D11RasterizerState>	rasterizerState;
	DXPtr<ID3D11BlendState>			blendState;
	DXPtr<ID3D11SamplerState>		sampler;

};
