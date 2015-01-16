
#pragma once

//////////////////////////////////////////////////////////////////////

struct Material
{
	VertexShader					vertexShader;
	PixelShader						pixelShader;
	DXPtr<ID3D11RasterizerState>	rasterizerState;
	DXPtr<ID3D11BlendState>			blendState;
	DXPtr<ID3D11DepthStencilState>	depthStencilState;

	template<typename T> HRESULT Create(Resource const &vsResource, Resource const &psResource)
	{
		DX(vertexShader.Create(vsResource, T::fields, _countof(T::fields)));
		DX(pixelShader.Create(psResource));
		CreateBlendState();
		CreateDepthStencilState();
		CreateRasterizerState();
		return S_OK;
	}

	HRESULT Destroy()
	{
		blendState.Release();
		depthStencilState.Release();
		rasterizerState.Release();
		DX(vertexShader.Destroy());
		DX(pixelShader.Destroy());
		return S_OK;
	}

	void Activate(ID3D11DeviceContext *context)
	{
		context->OMSetBlendState(blendState, 0, 0xffffffff);
		context->OMSetDepthStencilState(depthStencilState, 0);
		context->RSSetState(rasterizerState);
		pixelShader.Activate(context);
		vertexShader.Activate(context);
	}

	HRESULT CreateBlendState()
	{
		blendState.Release();
		CD3D11_BLEND_DESC blendDesc(D3D11_DEFAULT);
		DX(gDevice->CreateBlendState(&blendDesc, &blendState));
		return S_OK;
	}

	HRESULT CreateDepthStencilState()
	{
		depthStencilState.Release();
		CD3D11_DEPTH_STENCIL_DESC depthstencilDesc(D3D11_DEFAULT);
		depthstencilDesc.DepthEnable = false;
		depthstencilDesc.StencilEnable = false;
		DX(gDevice->CreateDepthStencilState(&depthstencilDesc, &depthStencilState));
		return S_OK;
	}

	HRESULT CreateRasterizerState()
	{
		rasterizerState.Release();
		CD3D11_RASTERIZER_DESC rasterizerDesc(D3D11_DEFAULT);
		rasterizerDesc.DepthClipEnable = false;
		rasterizerDesc.CullMode = D3D11_CULL_NONE;
		DX(gDevice->CreateRasterizerState(&rasterizerDesc, &rasterizerState));
		return S_OK;
	}

};

