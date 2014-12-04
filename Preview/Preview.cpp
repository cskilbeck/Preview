//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "D3D.h"

//////////////////////////////////////////////////////////////////////


__declspec (align(4)) struct Vertex
{
	Vec2 mPos;
	Vec2 mTexCoord;

	void Set(Vec2 const &pos, Vec2 const &texCoord)
	{
		mPos = pos;
		mTexCoord = texCoord;
	}
};

static const float s = 5;

static Vertex vert[6] = 
{
	{ { 0, 0 }, { 0, 0 } },
	{ { s, 0 }, { s, 0 } },
	{ { s, s }, { s, s } },

	{ { s, s }, { s, s } },
	{ { 0, s }, { 0, s } },
	{ { 0, 0 }, { 0, 0 } },
};

//////////////////////////////////////////////////////////////////////

HRESULT Preview::LoadShaders()
{
	size_t size;
	void *buffer;
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);
	DX(LoadResource(IDR_VERTEXSHADER, &buffer, &size));
	DX(mDevice->CreateVertexShader(buffer, size, NULL, &vertexShader));
	DX(mDevice->CreateInputLayout(layout, numElements, buffer, size, &vertexLayout));
	DX(LoadResource(IDR_PIXELSHADER, &buffer, &size));
	DX(mDevice->CreatePixelShader(buffer, size, NULL, &pixelShader));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT Preview::CreateSampler()
{
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	DX(mDevice->CreateSamplerState(&sampDesc, &sampler));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT Preview::CreateVertexBuffer()
{
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(vert);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vert;
	DX(mDevice->CreateBuffer(&bd, &InitData, &vertexBuffer));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT Preview::CreateRasterizerState()
{
	CD3D11_RASTERIZER_DESC rasterizerDesc(D3D11_DEFAULT);
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.AntialiasedLineEnable = TRUE;
	DX(mDevice->CreateRasterizerState(&rasterizerDesc, &rasterizerState));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT Preview::CreateBlendState()
{
	CD3D11_BLEND_DESC blendDesc(D3D11_DEFAULT);
	D3D11_RENDER_TARGET_BLEND_DESC rtBlendDesc;
	rtBlendDesc.BlendEnable = true;
	rtBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
	rtBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rtBlendDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	rtBlendDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	rtBlendDesc.SrcBlendAlpha = D3D11_BLEND_ZERO;
	rtBlendDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
	rtBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0] = rtBlendDesc;
	DX(mDevice->CreateBlendState(&blendDesc, &blendState));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

Preview::Preview(int width, int height) : Window(width, height)
{
	DXV(LoadShaders());
	DXV(CreateSampler());
	DXV(CreateVertexBuffer());
	DXV(CreateRasterizerState());
	DXV(CreateBlendState());

	mTexture.reset(new Texture(TEXT("D:\\test.png"), this));

	ChangeSize(mTexture->Width(), mTexture->Height());
	Center();

	OnDraw();
	Present();

	Show();
}

//////////////////////////////////////////////////////////////////////

Preview::~Preview()
{
}

//////////////////////////////////////////////////////////////////////

bool Preview::OnUpdate()
{
	return true;
}

//////////////////////////////////////////////////////////////////////

void Preview::OnDraw()
{
	static byte r = 16, g = 64, b = 32;
	Clear(Color(r, g, b));
	UINT strides[] = { sizeof(Vertex) };
	UINT offsets[] = { 0 };
	mContext->PSSetSamplers(0, 1, &sampler);
	mTexture->Activate();
	mContext->OMSetBlendState(blendState, 0, 0xffffffff);
	mContext->RSSetState(rasterizerState);
	mContext->IASetInputLayout(vertexLayout);
	mContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mContext->IASetVertexBuffers(0, 1, &vertexBuffer, strides, offsets);
	mContext->PSSetShader(pixelShader, NULL, 0);
	mContext->VSSetShader(vertexShader, NULL, 0);
	mContext->Draw(6, 0);
}
