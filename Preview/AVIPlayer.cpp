//////////////////////////////////////////////////////////////////////
//
// VideoPlayer window with controls for pause, ff, rewind, scrub etc
// Fix the scaling crap to only resize when the window is resized
// Maintain aspect ratio of the movies
// 
//
//
//
//
//
//
//
//////////////////////////////////////////////////////////////////////
// Load from command line, pop file choose dialog if empty?
// Options
//  - Scaling modes for minification and magnification
//  - Stretch/Fit/Centre on resize
//  - Use fullscreen for large images (which wouldn't fit in a maximized window)
// Load/Save options
// Don't churn messageloop when not necessary
// Window sizing/stretching/squeezing etc on startup
// Selection/Copy/Save As
// ?? Use GDI+ to load the textures?
// Support GIFs
// Full screen mode
// Floating thumbnail
// ?? Cycle through images in a folder
//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
// -- Split Window object into plain and DirectX versions
// -- Timer
// -- Make MENU optional in Window
// -- Pan/Zoom (ditch that crazy matrix function)
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

//////////////////////////////////////////////////////////////////////

static void Text(HDC dc, int x, int y, tchar const *fmt, ...)
{
	tchar buffer[1024];
	va_list v;
	va_start(v, fmt);
	int l = _vstprintf_s(buffer, fmt, v);
	TextOut(dc, x, y, buffer, l);
}

//////////////////////////////////////////////////////////////////////

void AVIPlayer::SetQuad(bool upsideDown)
{
	enum
	{
		tl = 0,
		br = 1,
		tr = 2,
		bl = 3,
		np = 4
	};

	Vec2 p[np];

	p[tl] = mDrawRect.TopLeft();
	p[br] = mDrawRect.BottomRight();
	p[tr] = mDrawRect.TopRight();
	p[bl] = mDrawRect.BottomLeft();

	for(int i = 0; i < np; ++i)
	{
		p[i].x = floorf(p[i].x);
		p[i].y = floorf(p[i].y);
	}

	float v1 = upsideDown ? 1.0f : 0.0f;
	float v2 = 1.0f - v1;

	Vertex *v = alphaVertexBuffer.GetBuffer();

	v[0].Set(p[tl], Vec2(0, v1));		// upside down UVs
	v[1].Set(p[tr], Vec2(1, v1));
	v[2].Set(p[br], Vec2(1, v2));
	v[3].Set(p[br], Vec2(1, v2));
	v[4].Set(p[bl], Vec2(0, v2));
	v[5].Set(p[tl], Vec2(0, v1));

	alphaVertexBuffer.Commit(mContext);
}

//////////////////////////////////////////////////////////////////////

HRESULT AVIPlayer::CreateDepthStencilState()
{
	mDepthStencilState.Release();
	CD3D11_DEPTH_STENCIL_DESC depthstencilDesc(D3D11_DEFAULT);
	depthstencilDesc.DepthEnable = FALSE;
	depthstencilDesc.StencilEnable = FALSE;
	DX(gDevice->CreateDepthStencilState(&depthstencilDesc, &mDepthStencilState));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT AVIPlayer::CreateRasterizerState()
{
	rasterizerState.Release();
	CD3D11_RASTERIZER_DESC rasterizerDesc(D3D11_DEFAULT);
	rasterizerDesc.DepthClipEnable = false;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.ScissorEnable = false;
	DX(gDevice->CreateRasterizerState(&rasterizerDesc, &rasterizerState));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT AVIPlayer::CreateBlendState()
{
	blendState.Release();
	CD3D11_BLEND_DESC blendDesc(D3D11_DEFAULT);
	DX(gDevice->CreateBlendState(&blendDesc, &blendState));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

AVIPlayer::AVIPlayer(int width, int height)
	: DXWindow(width, height)
	, mBackgroundColor(Color::DarkOliveGreen)
	, mScale(1)
	, mMenu(null)
	, mHandCursor(NULL)
{
}

//////////////////////////////////////////////////////////////////////

AVIPlayer::~AVIPlayer()
{
}

//////////////////////////////////////////////////////////////////////

void AVIPlayer::OnDestroy()
{
	movie1.Close();
	movie2.Close();

	//	mMaterial.Destroy();
	colorPixelShader.Destroy();
	colorVertexShader.Destroy();
	colorVertexBuffer.Destroy();

	alphaPixelShader.Destroy();
	alphaVertexBuffer.Destroy();
	alphaVertexShader.Destroy();

	rasterizerState.Release();
	blendState.Release();
	mDepthStencilState.Release();

	DestroyCursor(mHandCursor);
	DestroyMenu(mMenu);

	DXWindow::OnDestroy();
}

//////////////////////////////////////////////////////////////////////

void AVIPlayer::OnChar(int key, uintptr flags)
{
	switch(key)
	{
		case 27:
			Close();
			break;
		case 'p':
			if(movie1.movie.IsPaused())
			{
				movie1.movie.Play();
			}
			else
			{
				movie1.movie.Pause();
			}
			break;
		case 32:
			frameToPlay = 0;
			movie1.movie.Seek(1900);
			movie2.movie.Seek(1900);
			break;
	}
}

//////////////////////////////////////////////////////////////////////

void AVIPlayer::OnResize()
{
	SetWindowPos(mDXWindow, null, 0, 0, Width(), Height() / 2, SWP_NOZORDER);
	CalcDrawRect();
	DXWindow::OnResize();
}

//////////////////////////////////////////////////////////////////////

Vec2 AVIPlayer::MovieBorder() const
{
	return Vec2(2, 4);
}

//////////////////////////////////////////////////////////////////////

Vec2 AVIPlayer::DXSize() const
{
	return Vec2(Width() / 2.0f, Height() / 2.0f);
}

//////////////////////////////////////////////////////////////////////

void AVIPlayer::CalcDrawRect()
{
	if(movie1.movie.IsOpen())
	{
		Vec2 dx = DXSize() - MovieBorder();
		float ar = (float)movie1.width / movie1.height;
		Vec2 m(dx.x, dx.x / ar);
		if(m.y > dx.y)
		{
			m.y = dx.y;
			m.x = dx.y * ar;
		}
		Vec2 tl = (dx - m) / 2;
		mDrawRect.Set(tl + MovieBorder(), tl + m);
	}
}

//////////////////////////////////////////////////////////////////////

HRESULT AVIPlayer::LoadUntexturedMaterial()
{
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	WinResource r1(IDR_COLORVERTEXSHADER);
	if(r1.IsValid())
	{
		DX(colorVertexShader.Create(r1, r1.Size(), layout, _countof(layout)));
	}
	DX(colorVertexBuffer.Create(6));

	r1.Release();
	r1.Load(IDR_COLORPIXELSHADER);
	if(r1.IsValid())
	{
		DX(colorPixelShader.Create(r1, r1.Size()));
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT AVIPlayer::LoadAlphaMaterial()
{
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	WinResource r1(IDR_ALPHAVERTEXSHADER);
	if(!r1.IsValid())
	{
		return ERROR_RESOURCE_DATA_NOT_FOUND;
	}
	DX(alphaVertexShader.Create(r1, r1.Size(), layout, _countof(layout)));
	DX(alphaVertexBuffer.Create(6));

	r1.Release();
	r1.Load(IDR_ALPHAPIXELSHADER);
	if(!r1.IsValid())
	{
		return ERROR_RESOURCE_DATA_NOT_FOUND;
	}
	DX(alphaPixelShader.Create(r1, r1.Size()));

	using DirectX::XMVECTOR;
	using DirectX::XMVectorSet;

	ConstantBuffer &cb = *alphaPixelShader.GetCB("PixelShaderConstants");
	cb.Set("ChannelMask", XMVectorSet(1, 1, 1, 0));
	cb.Set("ColorOffset", XMVectorSet(0, 0, 0, 1));
	cb.Set("GridColor0", XMVectorSet(0.8f, 0.8f, 0.8f, 1));
	cb.Set("GridColor1", XMVectorSet(0.6f, 0.6f, 0.6f, 1));
	cb.Set("GridSize", Vec2(16, 16));
	cb.Set("GridSize2", Vec2(32, 32));
	cb.Commit(mContext);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////

bool AVIPlayer::OnCreate()
{
	mMenu = LoadMenu(mHINST, MAKEINTRESOURCE(IDC_PREVIEW));
	SetMenu(mHWND, mMenu);

	mDXWindow = CreateWindow(TEXT("STATIC"), null, WS_VISIBLE | WS_CHILD | SS_OWNERDRAW, 0, 0, Width(), Height() / 2, mHWND, null, null, null);

	if(DXWindow::OnCreate())
	{
		DXB(LoadUntexturedMaterial());
		DXB(LoadAlphaMaterial());

		DXB(CreateRasterizerState());
		DXB(CreateBlendState());
		DXB(CreateDepthStencilState());

		mHandCursor = LoadCursor(mHINST, MAKEINTRESOURCE(IDC_DRAG));

		frameToPlay = 0;

		DXB(movie1.Open(L"D:\\AVCaptures\\XB1_intro.avi"));
		DXB(movie2.Open(L"D:\\AVCaptures\\XB1_intro.avi"));

		movie1.movie.Play();
		movie2.movie.Play();

		MoveToMiddleOfMonitor();
		mTimer.Reset();
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////

bool AVIPlayer::OnUpdate()
{
	DXWindow::OnUpdate();

	mDeltaTime = mTimer.Delta();

	CalcDrawRect();

	//TRACE("Frametoplay: %d\n", frameToPlay);

	movie1.Update(mContext, frameToPlay);
	movie2.Update(mContext, frameToPlay);

	frameDropped = 0;
	if(movie1.CurrentFrame() != frameToPlay)
	{
		frameDropped = 1;
	}
	if(movie2.CurrentFrame() != frameToPlay)
	{
		frameDropped += 2;
	}
	if(frameDropped == 0)
	{
		++frameToPlay;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////

void AVIPlayer::OnDraw()
{
	using namespace DirectX;

	CD3D11_VIEWPORT vp(0.0f, 0.0f, Width() / 2.0f, Height() / 2.0f);
	float hlfw = 2.0f / vp.Width;
	float hlfh = -2.0f / vp.Height;

	Matrix matrix(hlfw, 0.0f, 0.0f, 0.0f,
					0.0f, hlfh, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					-1.0f, 1.0f, 0.0f, 1.0f);

	SetQuad(true);

	Clear(mBackgroundColor);

	mContext->OMSetBlendState(blendState, 0, 0xffffffff);
	mContext->OMSetDepthStencilState(mDepthStencilState, 0);
	mContext->RSSetState(rasterizerState);

	alphaVertexBuffer.Activate(mContext);
	alphaVertexShader.Activate(mContext);

	mContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ConstantBuffer &cb = *alphaVertexShader.GetCB("VertConstants");
	cb.Set("ProjectionMatrix", XMMatrixTranspose(matrix));
	cb.Set("TextureSize", movie1.texture->FSize());
	cb.Commit(mContext);

	alphaPixelShader.SetSampler("samplerState", movie1.texture->mSampler);
	alphaPixelShader.SetTexture("picture", movie1.texture->mShaderResourceView);
	alphaPixelShader.Activate(mContext);

	vp.TopLeftX = 0;
	mContext->RSSetViewports(1, &vp);
	mContext->Draw(alphaVertexBuffer.VertexCount(), 0);

	alphaPixelShader.SetSampler("samplerState", movie2.texture->mSampler);
	alphaPixelShader.SetTexture("picture", movie2.texture->mShaderResourceView);
	alphaPixelShader.Activate(mContext);

	vp.TopLeftX = Width() / 2.0f;;
	mContext->RSSetViewports(1, &vp);
	mContext->Draw(alphaVertexBuffer.VertexCount(), 0);

	vp.TopLeftX = 0;// Width() / 2.0f;;
	if(frameDropped != 0)
	{
		ColorVertex *d = colorVertexBuffer.GetBuffer();
		Vertex *s = alphaVertexBuffer.GetBuffer();
		for(uint i = 0; i < colorVertexBuffer.VertexCount(); ++i)
		{
			d[i].mPos = s[i].mPos;
			d[i].mColor = 0x8080ff00;
		}
		colorVertexBuffer.Commit(mContext);
		ConstantBuffer &b = *colorVertexShader.GetCB("VertConstants");
		b.Set("ProjectionMatrix", XMMatrixTranspose(matrix));
		b.Commit(mContext);
		colorVertexBuffer.Activate(mContext);
		colorVertexShader.Activate(mContext);
		colorPixelShader.Activate(mContext);
	}

	if(frameDropped & 1)
	{
		vp.TopLeftX = Width() / 2.0f;;
		vp.TopLeftX = 0;
		mContext->RSSetViewports(1, &vp);
		mContext->Draw(colorVertexBuffer.VertexCount(), 0);
	}

	if(frameDropped & 2)
	{
		mContext->RSSetViewports(1, &vp);
		mContext->Draw(colorVertexBuffer.VertexCount(), 0);
	}



	// info panel
	HDC dc = GetDC(mHWND);
	int top = Height() / 2;
	int f1 = movie1.CurrentFrame();
	int f2 = movie2.CurrentFrame();
	int diff = f2 - f1;
	FillRect(dc, &Rect2D(0, top, Width(), Height() - top), (HBRUSH)GetStockObject(WHITE_BRUSH));
	Text(dc, 4, top + 4, TEXT("Frames queued: Left: %02d, Right: %02d"), movie1.movie.FramesWaiting(), movie2.movie.FramesWaiting());
	Text(dc, 4, top + 4 + 20, TEXT("Frame1: %5d, Frame2: %5d (Drift: %2d)"), f1, f2, diff);
	Text(dc, 4, top + 4 + 40, TEXT("Frame: %5d (%s)"), frameToPlay, frameDropped ? "DROP!" : "");
	ReleaseDC(mHWND, dc);
}
