//////////////////////////////////////////////////////////////////////
// Load from command line, pop file choose dialog if empty?
// Options
//  - Scaling modes for minification and magnification
// Load/Save options
// Don't churn messageloop when not necessary
// Window sizing/stretching/squeezing etc on startup
// Selection/Copy/Save As
// ?? Use GDI+ to load the textures?
// Support GIFs
// Full screen mode
// ?? Cycle through images in a folder
//////////////////////////////////////////////////////////////////////
// - Pan/Zoom (ditch that crazy matrix function)
//////////////////////////////////////////////////////////////////////
// -- Split Window object into plain and DirectX versions
// -- Timer
// -- Make MENU optional in Window
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

//////////////////////////////////////////////////////////////////////

Preview::Vertex Preview::vert[6] =
{
	{ { 0, 0 }, { 0, 0 } },
	{ { 1, 0 }, { 1, 0 } },
	{ { 1, 1 }, { 1, 1 } },
	{ { 1, 1 }, { 1, 1 } },
	{ { 0, 1 }, { 0, 1 } },
	{ { 0, 0 }, { 0, 0 } },
};

void Preview::SetQuad()
{
	Vec2 topRight(mCurrentDrawRect.BottomRight.x, mCurrentDrawRect.TopLeft.y);
	Vec2 bottomLeft(mCurrentDrawRect.TopLeft.x, mCurrentDrawRect.BottomRight.y);
	vert[0].mPos = mCurrentDrawRect.TopLeft;
	vert[1].mPos = topRight;
	vert[2].mPos = mCurrentDrawRect.BottomRight;
	vert[3].mPos = mCurrentDrawRect.BottomRight;
	vert[4].mPos = bottomLeft;
	vert[5].mPos = mCurrentDrawRect.TopLeft;
}

//////////////////////////////////////////////////////////////////////

HRESULT Preview::LoadShaders()
{
	vertexShader.Release();
	pixelShader.Release();
	Resource vertData(IDR_VERTEXSHADER);
	Resource pixelData(IDR_PIXELSHADER);
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
	CD3D11_BUFFER_DESC bd(sizeof(VertexShaderConstants), D3D11_BIND_CONSTANT_BUFFER);
	DX(mDevice->CreateBuffer(&bd, NULL, &vertexShaderConstants));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT Preview::CreatePixelShaderConstants()
{
	pixelShaderConstants.Release();
	CD3D11_BUFFER_DESC bd(sizeof(PixelShaderConstants), D3D11_BIND_CONSTANT_BUFFER);
	DX(mDevice->CreateBuffer(&bd, NULL, &pixelShaderConstants));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT Preview::CreateSampler()
{
	sampler.Release();
	CD3D11_SAMPLER_DESC sampDesc(D3D11_DEFAULT);
	sampDesc.Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
	DX(mDevice->CreateSamplerState(&sampDesc, &sampler));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT Preview::CreateVertexBuffer()
{
	vertexBuffer.Release();
	CD3D11_BUFFER_DESC bd(sizeof(vert), D3D11_BIND_VERTEX_BUFFER);
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
	DX(mDevice->CreateBlendState(&blendDesc, &blendState));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

Preview::Preview(int width, int height)
	: DXWindow(width, height)
	, mBackgroundColor(255, 0, 255)
	, mScale(1)
	, mLastZoomTime(0)
	, mMenu(null)
	, mDrag(false)
	, mHandCursor(NULL)
{
}

//////////////////////////////////////////////////////////////////////

bool Preview::OnCreate()
{
	mMenu = LoadMenu(mHINST, MAKEINTRESOURCE(IDC_PREVIEW));
	SetMenu(mHWND, mMenu);

	if(DXWindow::OnCreate())
	{
		DXB(LoadShaders());
		DXB(CreateSampler());
		DXB(CreateVertexBuffer());
		DXB(CreateRasterizerState());
		DXB(CreateBlendState());
		DXB(CreateVertexShaderConstants());
		DXB(CreatePixelShaderConstants());
		DXB(CreateDepthStencilState());

		mHandCursor = LoadCursor(mHINST, MAKEINTRESOURCE(IDC_DRAG));

		mTexture.reset(new Texture(TEXT("D:\\test.png")));
		ChangeSize(mTexture->Width(), mTexture->Height());
		Center();
		mOldClientRect = ClientRect();
		mDrawRect.Set(Vec2(0, 0), mTexture->FSize());
		mCurrentDrawRect = mDrawRect;
		mTimer.Reset();
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////

void Preview::OnDestroy()
{
	DXWindow::OnDestroy();
	DestroyCursor(mHandCursor);
	DestroyMenu(mMenu);
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
	Vec2 mousePos(pos);

	if(mDrawRect.Contains(mousePos))
	{
		double time = mTimer.Elapsed();
		double deltaZ = time - mLastZoomTime;
		mLastZoomTime = time;

		float d = 1.0f + delta / 600.0f;
		float newScale = Constrain(mScale * d, 0.1f, 32.0f);
		if(mScale < 1 && newScale >= 1 || mScale > 1 && newScale <= 1)
		{
			if(deltaZ < 0.25f)
			{
				newScale = 1.0f;
				mDrawRect = mCurrentDrawRect;
			}
		}
		mScale = newScale;
		Vec2 frac = (mousePos - mDrawRect.TopLeft) / mDrawRect.Size();
		Vec2 sz = mTexture->FSize() * mScale;
		mDrawRect.Resize(sz);
		mDrawRect.MoveTo(mousePos - sz * frac);
	}
}

//////////////////////////////////////////////////////////////////////

void Preview::OnResize()
{
	DXWindow::OnResize();

	Vec2 midPoint = mDrawRect.MidPoint() * ClientRect().FSize() / mOldClientRect.FSize();
	Vec2 hs = mDrawRect.Size() / 2;
	mDrawRect.Set(midPoint - hs, midPoint + hs);
	mCurrentDrawRect = mDrawRect;
	mOldClientRect = ClientRect();
}

//////////////////////////////////////////////////////////////////////

void Preview::CalcDrawRect()
{
	if(mTexture != null)
	{
		Vec2 sz = mTexture->FSize() * mScale;
		Vec2 tl = (FSize() - sz) * 0.5f;
		mDrawRect.Set(tl, tl + sz);
	}
}

//////////////////////////////////////////////////////////////////////

void Preview::OnRightButtonDown(Point pos, uintptr flags)
{
	Vec2 mousePos(pos);
	if(mCurrentDrawRect.Contains(mousePos))
	{
		mDrawRect = mCurrentDrawRect;
		mDrag = true;
		mDragPos = pos;
		SetCursor(mHandCursor);
	}
}

//////////////////////////////////////////////////////////////////////

void Preview::OnRightButtonUp(Point pos, uintptr flags)
{
	mDrag = false;
	SetCursor(null);
}

//////////////////////////////////////////////////////////////////////

void Preview::OnMouseMove(Point pos, uintptr flags)
{
	if(mDrag)
	{
		mCurrentDrawRect.Translate(Vec2(pos - mDragPos));
		mDrawRect = mCurrentDrawRect;
		mDragPos = pos;
	}
}

//////////////////////////////////////////////////////////////////////

bool Preview::OnUpdate()
{
	DXWindow::OnUpdate();

	mDeltaTime = mTimer.Delta();

	Vec2 tld = mDrawRect.TopLeft - mCurrentDrawRect.TopLeft;
	Vec2 brd = mDrawRect.BottomRight - mCurrentDrawRect.BottomRight;

	if(tld.Length() > 0 || brd.Length() > 0)
	{
		mCurrentDrawRect.TopLeft += tld * 10.0f * (float)mDeltaTime;
		mCurrentDrawRect.BottomRight += brd * 10.0f * (float)mDeltaTime;

		tld = mDrawRect.TopLeft - mCurrentDrawRect.TopLeft;
		brd = mDrawRect.BottomRight - mCurrentDrawRect.BottomRight;

		if(tld.Length() < 1 && brd.Length() < 1)
		{
			mCurrentDrawRect = mDrawRect;
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

	UINT strides[] = { sizeof(Vertex) };
	UINT offsets[] = { 0 };

	float gridSize = 16;

	SetQuad();

	vsConstants.matrix = XMMatrixTranspose(matrix);
	vsConstants.textureSize = XMFLOAT2(mTexture->FWidth(), mTexture->FHeight());

	psConstants.channelMask = XMVectorSet(1, 1, 1, 1);
	psConstants.channelOffset = XMVectorSet(0, 0, 0, 0);
	psConstants.gridColor0 = XMVectorSet(0.8f, 0.8f, 0.8f, 1);
	psConstants.gridColor1 = XMVectorSet(0.6f, 0.6f, 0.6f, 1);
	psConstants.gridSize = XMFLOAT2(gridSize, gridSize);
	psConstants.gridSize2 = XMFLOAT2(gridSize * 2, gridSize * 2);

	mContext->UpdateSubresource(pixelShaderConstants, 0, NULL, &psConstants, 0, 0);
	mContext->UpdateSubresource(vertexShaderConstants, 0, NULL, &vsConstants, 0, 0);
	mContext->UpdateSubresource(vertexBuffer, 0, NULL, vert, 0, 0);

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
