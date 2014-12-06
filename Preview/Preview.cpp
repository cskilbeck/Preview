//////////////////////////////////////////////////////////////////////
// Split Window object into plain and DirectX versions
// Timer
// Load from command line, pop file choose dialog if empty
// Options Menu
// Load/Save options
// Don't churn messageloop when not necessary
// Window sizing/stretching/squeezing etc
// Pan/Zoom
// Selection/Copy/Save As
// ?? Use GDI+ to load the textures?
// Support GIFs
// Full screen mode
// ?? Cycle through images in a folder
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

//////////////////////////////////////////////////////////////////////

static Vertex vert[6] = 
{
	{ { 0, 0 }, { 0, 0 } },
	{ { 1, 0 }, { 1, 0 } },
	{ { 1, 1 }, { 1, 1 } },
	{ { 1, 1 }, { 1, 1 } },
	{ { 0, 1 }, { 0, 1 } },
	{ { 0, 0 }, { 0, 0 } },
};

//////////////////////////////////////////////////////////////////////

__declspec (align(16)) struct PixelShaderConstants
{
	DirectX::XMVECTOR channelMask;
	DirectX::XMVECTOR channelOffset;
	DirectX::XMVECTOR gridColor0;
	DirectX::XMVECTOR gridColor1;
	DirectX::XMFLOAT2 gridSize;
	DirectX::XMFLOAT2 gridSize2;
};

//////////////////////////////////////////////////////////////////////

__declspec (align(16)) struct VertexShaderConstants
{
	Matrix matrix;
	DirectX::XMFLOAT2 textureSize;
};

//////////////////////////////////////////////////////////////////////

HRESULT Preview::LoadShaders()
{
	vertexShader.Release();
	pixelShader.Release();
	WinResource vertData(IDR_VERTEXSHADER);
	WinResource pixelData(IDR_PIXELSHADER);
	if(vertData.IsValid() && pixelData.IsValid())
	{
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		UINT numElements = ARRAYSIZE(layout);
		DX(mDevice->CreateVertexShader(vertData, vertData.Size(), NULL, &vertexShader));
		DX(mDevice->CreateInputLayout(layout, numElements, vertData, vertData.Size(), &vertexLayout));
		DX(mDevice->CreatePixelShader(pixelData, pixelData.Size(), NULL, &pixelShader));
		return S_OK;
	}
	else
	{
		return ERROR_RESOURCE_DATA_NOT_FOUND;
	}
}

//////////////////////////////////////////////////////////////////////

HRESULT Preview::CreateDepthStencilState()
{
	mDepthStencilState.Release();
	CD3D11_DEPTH_STENCIL_DESC depthstencilDesc(D3D11_DEFAULT);
	depthstencilDesc.DepthEnable = FALSE;
	depthstencilDesc.StencilEnable = FALSE;
	DX(mDevice->CreateDepthStencilState(&depthstencilDesc, &mDepthStencilState));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT Preview::CreateVertexShaderConstants()
{
	vertexShaderConstants.Release();
	CD3D11_BUFFER_DESC bd(sizeof(VertexShaderConstants), D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DEFAULT, 0);
	DX(mDevice->CreateBuffer(&bd, NULL, &vertexShaderConstants));
	return S_OK;
}

HRESULT Preview::CreatePixelShaderConstants()
{
	pixelShaderConstants.Release();
	CD3D11_BUFFER_DESC bd(sizeof(PixelShaderConstants), D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DEFAULT, 0);
	DX(mDevice->CreateBuffer(&bd, NULL, &pixelShaderConstants));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT Preview::CreateSampler()
{
	sampler.Release();
	CD3D11_SAMPLER_DESC sampDesc(D3D11_DEFAULT);
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	DX(mDevice->CreateSamplerState(&sampDesc, &sampler));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT Preview::CreateVertexBuffer()
{
	vertexBuffer.Release();
	CD3D11_BUFFER_DESC bd(sizeof(vert), D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
	D3D11_SUBRESOURCE_DATA InitData = { vert, 0, 0 };
	DX(mDevice->CreateBuffer(&bd, &InitData, &vertexBuffer));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT Preview::CreateRasterizerState()
{
	rasterizerState.Release();
	CD3D11_RASTERIZER_DESC rasterizerDesc(D3D11_DEFAULT);
	rasterizerDesc.DepthClipEnable = false;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	DX(mDevice->CreateRasterizerState(&rasterizerDesc, &rasterizerState));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT Preview::CreateBlendState()
{
	blendState.Release();
	CD3D11_BLEND_DESC blendDesc(D3D11_DEFAULT);
	D3D11_RENDER_TARGET_BLEND_DESC &bd = blendDesc.RenderTarget[0];
	bd.BlendEnable = true;
	bd.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	bd.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	bd.SrcBlendAlpha = D3D11_BLEND_ZERO;
	bd.DestBlendAlpha = D3D11_BLEND_ZERO;
	DX(mDevice->CreateBlendState(&blendDesc, &blendState));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

Preview::Preview()
	: Window(100, 100)
	, mBackgroundColor(255, 0, 255)
	, mScale(1)
	, mCurrentScale(1)
	, mLastZoomTime(0)
	, mOffset(0, 0)
	, mDrag(false)
{
	DXV(LoadShaders());
	DXV(CreateSampler());
	DXV(CreateVertexBuffer());
	DXV(CreateRasterizerState());
	DXV(CreateBlendState());
	DXV(CreateVertexShaderConstants());
	DXV(CreateDepthStencilState());
	DXV(CreatePixelShaderConstants());

	mTexture.reset(new Texture(TEXT("D:\\test.png"), this));
	ChangeSize(mTexture->Width(), mTexture->Height());
	Center();
	mTranslation = FSize() / 2;
	mTimer.Reset();
}

//////////////////////////////////////////////////////////////////////

Preview::~Preview()
{
}

//////////////////////////////////////////////////////////////////////

void Preview::OnChar(int key, uintptr flags)
{
	switch(key)
	{
		case 27:
			Close();
			break;
	}
}

//////////////////////////////////////////////////////////////////////
// make it stick at 1.0

void Preview::OnMouseWheel(Point pos, int delta, uintptr flags)
{
	double time = mTimer.Elapsed();
	double deltaZ = time - mLastZoomTime;
	mLastZoomTime = time;

	float d = 1.0f + delta / 600.0f;
	float newScale = Constrain(mScale * d, 0.1f, 32.0f);
	if(mScale < 1 && newScale >= 1 || mScale > 1 && newScale <= 1 || mScale == 1 && mCurrentScale == 1)
	{
		if(deltaZ < 0.25f)
		{
			newScale = 1.0f;
			mCurrentScale = 1.0f;
		}
	}
	mScale = newScale;
}

//////////////////////////////////////////////////////////////////////

void Preview::OnResize()
{
	mTranslation = FSize() / 2;
}

//////////////////////////////////////////////////////////////////////

void Preview::OnRightButtonDown(Point pos, uintptr flags)
{
	mDrag = true;
	mDragPos = pos;
	SetCursor(LoadCursor(mHINST, MAKEINTRESOURCE(IDC_DRAG)));
}

//////////////////////////////////////////////////////////////////////

void Preview::OnRightButtonUp(Point pos, uintptr flags)
{
	mDrag = false;
	mTranslation += mOffset;
	mOffset = Vec2::zero;
	SetCursor(null);
}

//////////////////////////////////////////////////////////////////////

void Preview::OnMouseMove(Point pos, uintptr flags)
{
	if(mDrag)
	{
		mOffset = Vec2(pos - mDragPos);
	}
}

//////////////////////////////////////////////////////////////////////

bool Preview::OnUpdate()
{
	mDeltaTime = mTimer.Delta();

	Window::OnUpdate();
	if(mFrame == 1)
	{
		Show();
	}

	if(mCurrentScale != mScale)
	{
		float diff = mScale - mCurrentScale;
		mCurrentScale += diff * 10.0f * (float)mDeltaTime;
		if(fabsf(mCurrentScale - mScale) < 0.01f)
		{
			mCurrentScale = mScale;
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////

static DirectX::XMVECTOR XMVec2(Vec2 const &src)
{
	return DirectX::XMLoadFloat2((CONST DirectX::XMFLOAT2*)&src);
}

void Preview::OnDraw()
{
	using namespace DirectX;

	VertexShaderConstants vsConstants;
	PixelShaderConstants psConstants;

	float halfWidth = 2.0f / Width();
	float halfHeight = -2.0f / Height();

	Matrix matrix(halfWidth, 0.0f, 0.0f, 0.0f,
				  0.0f, halfHeight, 0.0f, 0.0f,
				  0.0f, 0.0f, 1.0f, 0.0f,
				  -1.0f, 1.0f, 0.0f, 1.0f);

	float rotation = 0;
	//scale = sinf(mFrame * 0.0125f) * 3.25f + 4;
	//rotation = mFrame * 0.025f + (sinf(mFrame * 0.033f) * 0.1f + 0.2f);

	XMVECTOR scalingOrigin = XMVec2(Vec2(0.5f, 0.5f));
	XMVECTOR rotationOrigin = XMVec2(Vec2::zero);
	XMVECTOR xscale = XMVec2(mTexture->FSize() * mCurrentScale);
	XMVECTOR translate = XMVec2(mTranslation + mOffset);
	XMMATRIX m2d = XMMatrixTransformation2D(scalingOrigin, 0, xscale, rotationOrigin, rotation, translate);

	UINT strides[] = { sizeof(Vertex) };
	UINT offsets[] = { 0 };

	float gridSize = 16;

	vsConstants.matrix = XMMatrixTranspose(m2d * matrix);
	vsConstants.textureSize = XMFLOAT2(mTexture->FWidth(), mTexture->FHeight());

	psConstants.channelMask = XMVectorSet(1, 1, 1, 1);
	psConstants.channelOffset = XMVectorSet(0, 0, 0, 0);
	psConstants.gridColor0 = XMVectorSet(0.8f, 0.8f, 0.8f, 1);
	psConstants.gridColor1 = XMVectorSet(0.6f, 0.6f, 0.6f, 1);
	psConstants.gridSize = XMFLOAT2(gridSize, gridSize);
	psConstants.gridSize2 = XMFLOAT2(gridSize * 2, gridSize * 2);

	mContext->UpdateSubresource(pixelShaderConstants, 0, NULL, &psConstants, 0, 0);
	mContext->UpdateSubresource(vertexShaderConstants, 0, NULL, &vsConstants, 0, 0);

	mContext->IASetInputLayout(vertexLayout);
	mContext->IASetVertexBuffers(0, 1, &vertexBuffer, strides, offsets);
	mContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	mContext->VSSetShader(vertexShader, NULL, 0);
	mContext->VSSetConstantBuffers(0, 1, &vertexShaderConstants);

	mContext->PSSetShader(pixelShader, NULL, 0);
	mContext->PSSetConstantBuffers(0, 1, &pixelShaderConstants);
	mContext->PSSetSamplers(0, 1, &sampler);

	mContext->OMSetBlendState(blendState, 0, 0xffffffff);
	mContext->OMSetDepthStencilState(mDepthStencilState, 0);

	mContext->RSSetState(rasterizerState);

	mTexture->Activate();

	Clear(mBackgroundColor);
	mContext->Draw(ARRAYSIZE(vert), 0);
}
