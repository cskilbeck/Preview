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
	float v1 = upsideDown ? 1.0f : 0.0f;
	float v2 = 1.0f - v1;

	Vec2 p[4] =
	{
		mDrawRect.TopLeft(),
		mDrawRect.TopRight(),
		mDrawRect.BottomRight(),
		mDrawRect.BottomLeft()
	};

	Vec2 uv[4] =
	{
		Vec2(0, v1),
		Vec2(1, v1),
		Vec2(1, v2),
		Vec2(0, v2)
	};

	int id[6] = { 0, 1, 2, 2, 3, 0 };

	Vertex *v = (Vertex *)alphaVertexBuffer.GetBuffer();
	for(int i = 0; i < 6; ++i)
	{
		v[i].mPosition = p[id[i]];
		v[i].mTexCoord = uv[id[i]];
	}
	alphaVertexBuffer.Commit(mContext);
}
//////////////////////////////////////////////////////////////////////

AVIPlayer::AVIPlayer(int width, int height)
	: DXWindow(width, height)
	, mMenu(null)
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
	colorMaterial.Destroy();
	alphaMaterial.Destroy();
	colorVertexBuffer.Destroy();
	alphaVertexBuffer.Destroy();
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
			movie1.TogglePause();
			movie2.TogglePause();
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

void AVIPlayer::CalcDrawRect()
{
	if(movie1.movie.IsOpen())
	{
		Vec2 border(2, 4);
		Vec2 dx = Vec2(Width() / 2.0f, Height() / 2.0f) - border;
		float ar = (float)movie1.width / movie1.height;
		Vec2 m(dx.x, dx.x / ar);
		if(m.y > dx.y)
		{
			m.y = dx.y;
			m.x = dx.y * ar;
		}
		Vec2 tl = (dx - m) / 2;
		mDrawRect.Set(tl + border, tl + m);
	}
}

//////////////////////////////////////////////////////////////////////

bool AVIPlayer::OnCreate()
{
	mMenu = LoadMenu(mHINST, MAKEINTRESOURCE(IDC_PREVIEW));
	SetMenu(mHWND, mMenu);

	mDXWindow = CreateWindow(TEXT("STATIC"), null, WS_VISIBLE | WS_CHILD | SS_OWNERDRAW, 0, 0, Width(), Height() / 2, mHWND, null, null, null);

	if(DXWindow::OnCreate())
	{
		MoveToMiddleOfMonitor();

		{
			WinResource vs(IDR_COLORVERTEXSHADER);
			WinResource ps(IDR_COLORPIXELSHADER);
			DXB(colorMaterial.Create<ColorVertex>(vs, ps));
			DXB(colorVertexBuffer.Create<ColorVertex>(6, vs));
		}

		{
			WinResource vs(IDR_ALPHAVERTEXSHADER);
			WinResource ps(IDR_ALPHAPIXELSHADER);
			DXB(alphaMaterial.Create<Vertex>(vs, ps));
			DXB(alphaVertexBuffer.Create<Vertex>(6, vs));
		}

		using DirectX::XMVectorSet;

		ConstantBuffer &cb = *alphaMaterial.pixelShader.GetCB("PixelShaderConstants");
		cb.Set("ChannelMask", XMVectorSet(1, 1, 1, 0));
		cb.Set("ColorOffset", XMVectorSet(0, 0, 0, 1));
		cb.Set("GridColor0", XMVectorSet(0.8f, 0.8f, 0.8f, 1));
		cb.Set("GridColor1", XMVectorSet(0.6f, 0.6f, 0.6f, 1));
		cb.Set("GridSize", Vec2(16, 16));
		cb.Set("GridSize2", Vec2(32, 32));
		cb.Commit(mContext);

		frameToPlay = 0;

		DXB(movie1.Open(L"D:\\AVCaptures\\XB1_intro.avi"));
		DXB(movie2.Open(L"D:\\AVCaptures\\XB1_intro.avi"));

		movie1.movie.Play();
		movie2.movie.Play();

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

Matrix ScreenSpaceMatrix(float width, float height)
{
	float hlfw = 2.0f / width;
	float hlfh = -2.0f / height;
	return Matrix(hlfw, 0.0f, 0.0f, 0.0f,
				  0.0f, hlfh, 0.0f, 0.0f,
				  0.0f, 0.0f, 1.0f, 0.0f,
				  -1.0f, 1.0f, 0.0f, 1.0f);

}

//////////////////////////////////////////////////////////////////////

void AVIPlayer::OnDraw()
{
	using namespace DirectX;
	Clear(Color::DarkOliveGreen);

	CD3D11_VIEWPORT vp(0.0f, 0.0f, Width() / 2.0f, Height() / 2.0f);

	Matrix matrix(XMMatrixTranspose(ScreenSpaceMatrix(vp.Width, vp.Height)));

	CalcDrawRect();
	SetQuad(true);
	alphaVertexBuffer.Activate(mContext);

	mContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ConstantBuffer &cb = *alphaMaterial.vertexShader.GetCB("VertConstants");
	cb.Set("ProjectionMatrix", matrix);
	cb.Set("TextureSize", movie1.texture->FSize());
	cb.Commit(mContext);

	alphaMaterial.pixelShader.SetSampler("samplerState", movie1.texture->mSampler);
	alphaMaterial.pixelShader.SetTexture("picture", movie1.texture->mShaderResourceView);
	alphaMaterial.Activate(mContext);

	vp.TopLeftX = 0;
	mContext->RSSetViewports(1, &vp);
	mContext->Draw(alphaVertexBuffer.VertexCount(), 0);

	alphaMaterial.pixelShader.SetSampler("samplerState", movie2.texture->mSampler);
	alphaMaterial.pixelShader.SetTexture("picture", movie2.texture->mShaderResourceView);
	alphaMaterial.Activate(mContext);

	vp.TopLeftX = Width() / 2.0f;;
	mContext->RSSetViewports(1, &vp);
	mContext->Draw(alphaVertexBuffer.VertexCount(), 0);

	{
		static uint32 tot[3] = { 0, 0 };
		static Random bfoo;
		tot[bfoo.Ranged(_countof(tot))]++;
		Trace("%9d,%9d,%9d\n", tot[0], tot[1], tot[2]);

	}

	if(frameDropped != 0)
	{
		ColorVertex *d = (ColorVertex *)colorVertexBuffer.GetBuffer();
		Vertex *s = (Vertex *)alphaVertexBuffer.GetBuffer();
		for(uint i = 0; i < colorVertexBuffer.VertexCount(); ++i)
		{
			d[i].Pos = s[i].mPosition;
			d[i].Color = 0x8080ff00;
		}
		colorVertexBuffer.Commit(mContext);
		colorVertexBuffer.Activate(mContext);

		ConstantBuffer &b = *colorMaterial.vertexShader.GetCB("VertConstants");
		b.Set("ProjectionMatrix", matrix);
		b.Commit(mContext);

		colorMaterial.Activate(mContext);

		if(frameDropped & 1)
		{
			vp.TopLeftX = 0;
			mContext->RSSetViewports(1, &vp);
			mContext->Draw(colorVertexBuffer.VertexCount(), 0);
		}
		if(frameDropped & 2)
		{
			vp.TopLeftX = Width() / 2.0f;
			mContext->RSSetViewports(1, &vp);
			mContext->Draw(colorVertexBuffer.VertexCount(), 0);
		}
	}
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
