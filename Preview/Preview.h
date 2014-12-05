//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Preview : Window
{
	Preview();
	~Preview();

	bool OnUpdate() override;
	void OnDraw() override;

	void OnChar(int key, uint32 flags) override;

	HRESULT LoadShaders();
	HRESULT CreateSampler();
	HRESULT CreateVertexBuffer();
	HRESULT CreateRasterizerState();
	HRESULT CreateBlendState();
	HRESULT CreateDepthStencilState();
	HRESULT CreateVertexShaderConstants();
	HRESULT CreatePixelShaderConstants();

	Ptr<Texture> mTexture;

	DXPtr<ID3D11InputLayout>		vertexLayout;
	DXPtr<ID3D11PixelShader>		pixelShader;
	DXPtr<ID3D11VertexShader>		vertexShader;
	DXPtr<ID3D11Buffer>				vertexBuffer;
	DXPtr<ID3D11RasterizerState>	rasterizerState;
	DXPtr<ID3D11BlendState>			blendState;
	DXPtr<ID3D11SamplerState>		sampler;
	DXPtr<ID3D11Buffer>				vertexShaderConstants;
	DXPtr<ID3D11Buffer>				pixelShaderConstants;
	DXPtr<ID3D11DepthStencilState>	mDepthStencilState;

};
