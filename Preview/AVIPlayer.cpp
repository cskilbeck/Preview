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

	vert[0].Set(p[tl], Vec2(0, v1));		// upside down UVs
	vert[1].Set(p[tr], Vec2(1, v1));
	vert[2].Set(p[br], Vec2(1, v2));
	vert[3].Set(p[br], Vec2(1, v2));
	vert[4].Set(p[bl], Vec2(0, v2));
	vert[5].Set(p[tl], Vec2(0, v1));
}

//////////////////////////////////////////////////////////////////////

HRESULT AVIPlayer::LoadShaders()
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
		DX(gDevice->CreateVertexShader(vertData, vertData.Size(), NULL, &vertexShader));
		DX(gDevice->CreateInputLayout(layout, ARRAYSIZE(layout), vertData, vertData.Size(), &vertexLayout));
		DX(gDevice->CreatePixelShader(pixelData, pixelData.Size(), NULL, &pixelShader));
		return S_OK;
	}
	else
	{
		return ERROR_RESOURCE_DATA_NOT_FOUND;
	}
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

HRESULT AVIPlayer::CreateVertexShaderConstants()
{
	vertexShaderConstants.Release();
	CD3D11_BUFFER_DESC bd(sizeof(VertexShaderConstants), D3D11_BIND_CONSTANT_BUFFER);
	DX(gDevice->CreateBuffer(&bd, NULL, &vertexShaderConstants));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT AVIPlayer::CreatePixelShaderConstants()
{
	pixelShaderConstants.Release();
	CD3D11_BUFFER_DESC bd(sizeof(PixelShaderConstants), D3D11_BIND_CONSTANT_BUFFER);
	DX(gDevice->CreateBuffer(&bd, NULL, &pixelShaderConstants));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT AVIPlayer::CreateSampler()
{
	sampler.Release();
	CD3D11_SAMPLER_DESC sampDesc(D3D11_DEFAULT);
	sampDesc.Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
	DX(gDevice->CreateSamplerState(&sampDesc, &sampler));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT AVIPlayer::CreateVertexBuffer()
{
	vertexBuffer.Release();
	CD3D11_BUFFER_DESC bd(sizeof(vert), D3D11_BIND_VERTEX_BUFFER);
	D3D11_SUBRESOURCE_DATA InitData = { vert, 0, 0 };
	DX(gDevice->CreateBuffer(&bd, &InitData, &vertexBuffer));
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

template <typename T> HRESULT LoadShader(T &shader, DWORD resourceID)
{
	WinResource r1(resourceID);
	return r1.IsValid() ? shader.Create(r1, r1.Size()) : r1.Error();
}

//////////////////////////////////////////////////////////////////////

bool AVIPlayer::OnCreate()
{
	mMenu = LoadMenu(mHINST, MAKEINTRESOURCE(IDC_PREVIEW));
	SetMenu(mHWND, mMenu);

	mDXWindow = CreateWindow(TEXT("STATIC"), null, WS_VISIBLE | WS_CHILD | SS_OWNERDRAW, 0, 0, Width(), Height() / 2, mHWND, null, null, null);

	if(DXWindow::OnCreate())
	{
		DXB(LoadShader(colorVertexShader, IDR_COLORVERTEXSHADER));
		DXB(colorVertexBuffer.Create(2, colorVertexShader));

		DXB(LoadShader(colorPixelShader, IDR_COLORPIXELSHADER));

		colorVertexBuffer[0] = { Vec2(0, 0), Color::Cyan };
		colorVertexBuffer[1] = { Vec2(100, 100), Color::Magenta };
		colorVertexBuffer.Update(mContext);

		DXB(LoadShaders());
		DXB(CreateSampler());
		DXB(CreateVertexBuffer());
		DXB(CreateRasterizerState());
		DXB(CreateBlendState());
		DXB(CreateVertexShaderConstants());
		DXB(CreatePixelShaderConstants());
		DXB(CreateDepthStencilState());

		mHandCursor = LoadCursor(mHINST, MAKEINTRESOURCE(IDC_DRAG));

		//mTexture.reset(new Texture(960, 540, Color::BrightBlue));

		//ConstantBuffer *b = mVertexShader.GetCB("VertConstants");
		//Matrix *m = (Matrix *)b->AddressOf("ProjectionMatrix");
		//Vec2 *v = (Vec2 *)b->AddressOf("TextureSize");

		//ConstantBuffer *pb = mPixelShader.GetCB("PixelShaderConstants");
		//PixelShaderConstants *p = (PixelShaderConstants *)(pb->GetBuffer());

		frameToPlay = -8;

		DXB(movie1.Open(L"D:\\AVCaptures\\XB1_intro.avi"));
		DXB(movie2.Open(L"D:\\AVCaptures\\XB1_intro.avi"));

		MoveToMiddleOfMonitor();

		// scale the movie into the DXWindow
		CalcDrawRect();

		mTimer.Reset();

		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////

void AVIPlayer::OnDestroy()
{
	movie1.movie.Close();
	movie2.movie.Close();

//	mMaterial.Destroy();
	colorPixelShader.Destroy();
	colorVertexShader.Destroy();

	pixelShader.Release();
	pixelShaderConstants.Release();

	vertexShader.Release();
	vertexLayout.Release();
	vertexShaderConstants.Release();

	rasterizerState.Release();
	blendState.Release();
	mDepthStencilState.Release();

	sampler.Release();

	vertexBuffer.Release();

	DestroyCursor(mHandCursor);
	DestroyMenu(mMenu);

	DXWindow::OnDestroy();
}

//////////////////////////////////////////////////////////////////////

AVIPlayer::~AVIPlayer()
{
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
			frameToPlay = 1900;
			movie1.movie.Seek(frameToPlay);
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

bool AVIPlayer::OnUpdate()
{
	DXWindow::OnUpdate();

	mDeltaTime = mTimer.Delta();

	CalcDrawRect();

	if(frameToPlay < 0)
	{
		if(movie1.movie.IsPaused())
		{
			DXB(movie1.movie.Play());
		}
		if(movie2.movie.IsPaused())
		{
			DXB(movie2.movie.Play());
		}
		++frameToPlay;
	}
	else
	{
		movie1.Update(mContext, frameToPlay);
		movie2.Update(mContext, frameToPlay);

		if(movie1.CurrentFrame() == frameToPlay && movie2.CurrentFrame() == frameToPlay)
		{
			++frameToPlay;
			frameDropped = false;
		}
		else
		{
			frameDropped = true;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////

static void Text(HDC dc, int x, int y, tchar const *fmt, ...)
{
	tchar buffer[1024];
	va_list v;
	va_start(v, fmt);
	int l = _vstprintf_s(buffer, fmt, v);
	TextOut(dc, x, y, buffer, l);
}

void AVIPlayer::OnDraw()
{
	using namespace DirectX;

	Clear(mBackgroundColor);
	if(movie1.texture != null)
	{
		CD3D11_VIEWPORT vp(0.0f, 0.0f, Width() / 2.0f, Height() / 2.0f);

		float hlfw = 2.0f / vp.Width;
		float hlfh = -2.0f / vp.Height;

		Matrix matrix(hlfw, 0.0f, 0.0f, 0.0f,
					  0.0f, hlfh, 0.0f, 0.0f,
					  0.0f, 0.0f, 1.0f, 0.0f,
					  -1.0f, 1.0f, 0.0f, 1.0f);

		UINT strides[] = { sizeof(Vertex) };
		UINT offsets[] = { 0 };

		float gridSize = 16;

		VertexShaderConstants vsConstants;
		PixelShaderConstants psConstants;

		SetQuad(true);

		vsConstants.matrix = XMMatrixTranspose(matrix);
		vsConstants.textureSize = movie1.texture->FSize();

		psConstants.channelMask = XMVectorSet(1, 1, 1, 0);
		psConstants.channelOffset = XMVectorSet(0, 0, 0, 1);
		psConstants.gridColor0 = XMVectorSet(0.8f, 0.8f, 0.8f, 1);
		psConstants.gridColor1 = XMVectorSet(0.6f, 0.6f, 0.6f, 1);
		psConstants.gridSize = Vec2(gridSize, gridSize);
		psConstants.gridSize2 = Vec2(gridSize * 2, gridSize * 2);

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

		mContext->RSSetViewports(1, &vp);

		movie1.texture->Activate(mContext);
		mContext->Draw(ARRAYSIZE(vert), 0);

		if(movie2.texture != null)
		{
			vp.TopLeftX = Width() / 2.0f;;
			mContext->RSSetViewports(1, &vp);
			movie1.texture->Activate(mContext);
			mContext->Draw(ARRAYSIZE(vert), 0);
		}

		colorPixelShader.Activate(mContext);
		colorVertexBuffer.Update(mContext);
		colorVertexBuffer.Activate(mContext);
		colorVertexShader.Activate(mContext);
		mContext->Draw(2, 0);
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
