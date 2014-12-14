//////////////////////////////////////////////////////////////////////
// PixelShader & PSConstants
// VertexShader & VSConstants
// VertexLayout
// DepthStencilState
// BlendState
// RasterizerState
// SamplerState
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////

Material::Material()
{
}

//////////////////////////////////////////////////////////////////////

HRESULT Material::CreateDepthStencilState()
{
	mDepthStencilState.Release();
	CD3D11_DEPTH_STENCIL_DESC depthstencilDesc(D3D11_DEFAULT);
	depthstencilDesc.DepthEnable = false;
	depthstencilDesc.StencilEnable = false;
	DX(gDevice->CreateDepthStencilState(&depthstencilDesc, &mDepthStencilState));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT Material::CreateRasterizerState()
{
	rasterizerState.Release();
	CD3D11_RASTERIZER_DESC rasterizerDesc(D3D11_DEFAULT);
	rasterizerDesc.DepthClipEnable = false;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	DX(gDevice->CreateRasterizerState(&rasterizerDesc, &rasterizerState));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT Material::CreateBlendState()
{
	blendState.Release();
	CD3D11_BLEND_DESC blendDesc(D3D11_DEFAULT);
	DX(gDevice->CreateBlendState(&blendDesc, &blendState));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT Material::Create()
{
	DX(CreateRasterizerState());
	DX(CreateBlendState());
	DX(CreateDepthStencilState());

	return S_OK;
}

//////////////////////////////////////////////////////////////////////

void Material::Activate(DXPtr<ID3D11DeviceContext> context)
{
	context->OMSetBlendState(blendState, 0, 0xffffffff);
	context->OMSetDepthStencilState(mDepthStencilState, 0);
	context->RSSetState(rasterizerState);
}

//////////////////////////////////////////////////////////////////////

HRESULT Material::Destroy()
{
	blendState.Release();
	mDepthStencilState.Release();
	rasterizerState.Release();
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

Material::~Material()
{
	Destroy();
}
