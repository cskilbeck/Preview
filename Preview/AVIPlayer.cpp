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

AVIPlayer::Vertex AVIPlayer::vert[6];

void AVIPlayer::SetQuad()
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

	p[tl] = mCurrentDrawRect.TopLeft();
	p[br] = mCurrentDrawRect.BottomRight();
	p[tr] = mCurrentDrawRect.TopRight();
	p[bl] = mCurrentDrawRect.BottomLeft();

	for(int i = 0; i < np; ++i)
	{
		p[i].x = floorf(p[i].x);
		p[i].y = floorf(p[i].y);
	}

	vert[0].Set(p[tl], Vec2(0, 0));
	vert[1].Set(p[tr], Vec2(1, 0));
	vert[2].Set(p[br], Vec2(1, 1));
	vert[3].Set(p[br], Vec2(1, 1));
	vert[4].Set(p[bl], Vec2(0, 1));
	vert[5].Set(p[tl], Vec2(0, 0));
}

//////////////////////////////////////////////////////////////////////

HRESULT AVIPlayer::LoadShaders()
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
		DX(mDevice->CreateVertexShader(vertData, vertData.Size(), NULL, &vertexShader));
		DX(mDevice->CreateInputLayout(layout, ARRAYSIZE(layout), vertData, vertData.Size(), &vertexLayout));
		DX(mDevice->CreatePixelShader(pixelData, pixelData.Size(), NULL, &pixelShader));
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
	DX(mDevice->CreateDepthStencilState(&depthstencilDesc, &mDepthStencilState));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT AVIPlayer::CreateVertexShaderConstants()
{
	vertexShaderConstants.Release();
	CD3D11_BUFFER_DESC bd(sizeof(VertexShaderConstants), D3D11_BIND_CONSTANT_BUFFER);
	DX(mDevice->CreateBuffer(&bd, NULL, &vertexShaderConstants));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT AVIPlayer::CreatePixelShaderConstants()
{
	pixelShaderConstants.Release();
	CD3D11_BUFFER_DESC bd(sizeof(PixelShaderConstants), D3D11_BIND_CONSTANT_BUFFER);
	DX(mDevice->CreateBuffer(&bd, NULL, &pixelShaderConstants));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT AVIPlayer::CreateSampler()
{
	sampler.Release();
	CD3D11_SAMPLER_DESC sampDesc(D3D11_DEFAULT);
	sampDesc.Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
	DX(mDevice->CreateSamplerState(&sampDesc, &sampler));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT AVIPlayer::CreateVertexBuffer()
{
	vertexBuffer.Release();
	CD3D11_BUFFER_DESC bd(sizeof(vert), D3D11_BIND_VERTEX_BUFFER);
	D3D11_SUBRESOURCE_DATA InitData = { vert, 0, 0 };
	DX(mDevice->CreateBuffer(&bd, &InitData, &vertexBuffer));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT AVIPlayer::CreateRasterizerState()
{
	rasterizerState.Release();
	CD3D11_RASTERIZER_DESC rasterizerDesc(D3D11_DEFAULT);
	rasterizerDesc.DepthClipEnable = false;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	DX(mDevice->CreateRasterizerState(&rasterizerDesc, &rasterizerState));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT AVIPlayer::CreateBlendState()
{
	blendState.Release();
	CD3D11_BLEND_DESC blendDesc(D3D11_DEFAULT);
	DX(mDevice->CreateBlendState(&blendDesc, &blendState));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

AVIPlayer::AVIPlayer(int width, int height)
	: DXWindow(width, height)
	, mBackgroundColor(Color::DarkOliveGreen)
	, mScale(1)
	, mLastZoomTime(0)
	, mMenu(null)
	, mDrag(false)
	, mHandCursor(NULL)
	, mMaintainImagePosition(true)
	, mStream(null)
	, mStreamFrameGetter(null)
{
}

//////////////////////////////////////////////////////////////////////

bool AVIPlayer::OpenStream(tchar const *filename)
{
	CloseStream();

	if(AVIStreamOpenFromFile(&mStream, filename, streamtypeVIDEO, 0, OF_READ, NULL) == S_OK)
	{
		AVISTREAMINFO streamInfo;
		if(AVIStreamInfo(mStream, &streamInfo, sizeof(streamInfo)) == S_OK)
		{
			Rect2D const &r = (Rect2D const &)streamInfo.rcFrame;
			movieSize = r.GetSize();
			float framerate = (float)streamInfo.dwRate / streamInfo.dwScale;
			float seconds = streamInfo.dwLength / framerate;
			int minutes = (int)(seconds / 60.0f);
			TRACE(TEXT("Width: %d, Height: %d, Framerate: %f, Length: %d'%02d\"\n"), r.Width(), r.Height(), framerate, minutes, (int)seconds % 60);

			BITMAPINFOHEADER b = { 0 };
			b.biBitCount = 32;
			b.biHeight = -movieSize.Height();
			b.biWidth = movieSize.Width();
			b.biSize = sizeof(b);
			b.biPlanes = 1;
			b.biCompression = BI_RGB;

			mStreamFrameGetter = AVIStreamGetFrameOpen(mStream, &b);
			if(mStreamFrameGetter == null)
			{
				CloseStream();
			}
		}
		else
		{
			CloseStream();
		}
	}
	return mStream != null;
}

//////////////////////////////////////////////////////////////////////

void AVIPlayer::CloseStream()
{
	if(mStreamFrameGetter != null)
	{
		AVIStreamGetFrameClose(mStreamFrameGetter);
		mStreamFrameGetter = null;
	}
	if(mStream != null)
	{
		AVIStreamClose(mStream);
		mStream = null;
	}
}

//////////////////////////////////////////////////////////////////////

bool AVIPlayer::StreamIsOpen() const
{
	return mStream != null && mStreamFrameGetter != null;
}

//////////////////////////////////////////////////////////////////////

bool AVIPlayer::OnCreate()
{
	AVIFileInit();

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

		if(OpenStream(TEXT("D:\\AVCaptures\\XB1_intro.avi")))
		{
			size_t imageSize = movieSize.Width() * movieSize.Height();
			image.reset(new Color[imageSize]);
			memset(image.get(), imageSize * sizeof(Color), 0);
			mTexture.reset(new Texture(movieSize.Width(), movieSize.Height(), 0xFF00FF80));
		}

//		mTexture.reset(new Texture(TEXT("D:\\test.png")));
		mDrawRect.Set(Vec2::zero, mTexture->FSize());
		ChangeSize(mTexture->Width(), mTexture->Height());
		MoveToMiddleOfMonitor();
		mOldClientRect = ClientRect();
		CenterImageInWindowAndResetZoom();
		mCurrentDrawRect = mDrawRect;
		mTimer.Reset();
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////

void AVIPlayer::OnDestroy()
{
	image.reset();

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

	mTexture.reset();

	DestroyCursor(mHandCursor);
	DestroyMenu(mMenu);

	AVIFileExit();

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
	}
}

//////////////////////////////////////////////////////////////////////
// make it stick at 1.0

void AVIPlayer::OnMouseWheel(Point2D pos, int delta, uintptr flags)
{
	Vec2 mousePos(pos);

	if(mDrawRect.Contains(mousePos))
	{
		double time = mTimer.Elapsed();
		double deltaZ = time - mLastZoomTime;
		mLastZoomTime = time;

		float d = 1.0f + sgn(delta) * 0.1f;
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
		Vec2 frac = (mousePos - mDrawRect.TopLeft()) / mDrawRect.Size();
		Vec2 sz = mTexture->FSize() * mScale;
		mDrawRect.Resize(sz);
		mDrawRect.MoveTo(mousePos - sz * frac);
		if(ClientRect().Width() < mDrawRect.Width() || ClientRect().Height() < mDrawRect.Height())
		{
			Rect2D windowRect;
			Rect2D newRect = GetWindowRectFromClientRect(Rect2D(mDrawRect));
			GetWindowRect(mHWND, &windowRect);
			newRect.MoveTo(windowRect.MidPoint() - newRect.HalfSize());
			mDrawRect.MoveTo(Vec2(GetClientRectFromWindowRect(newRect).HalfSize()) - mDrawRect.HalfSize());
			mCurrentDrawRect = mDrawRect;
			mMaintainImagePosition = false;
			SetWindowRect(newRect);
			mMaintainImagePosition = true;
		}
	}
}

//////////////////////////////////////////////////////////////////////

void AVIPlayer::OnResize()
{
	DXWindow::OnResize();

	Vec2 midPoint = mDrawRect.MidPoint() * ClientRect().FSize() / mOldClientRect.FSize();
	Vec2 hs = mDrawRect.Size() / 2;
	if(mMaintainImagePosition)
	{
		mDrawRect.Set(midPoint - hs, midPoint + hs);
		mCurrentDrawRect = mDrawRect;
	}
	mOldClientRect = ClientRect();
}

//////////////////////////////////////////////////////////////////////

void AVIPlayer::CalcDrawRect()
{
	if(mTexture != null)
	{
		Vec2 sz = mTexture->FSize() * mScale;
		Vec2 tl = (FSize() - sz) * 0.5f;
		mDrawRect.Set(tl, tl + sz);
	}
}

//////////////////////////////////////////////////////////////////////

void AVIPlayer::OnRightButtonDown(Point2D pos, uintptr flags)
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

void AVIPlayer::OnRightButtonUp(Point2D pos, uintptr flags)
{
	mDrag = false;
	SetCursor(null);
}

//////////////////////////////////////////////////////////////////////

void AVIPlayer::CenterImageInWindow()
{
	Vec2 midPoint(ClientRect().MidPoint());
	Vec2 halfSize = mDrawRect.Size() / 2;
	mDrawRect.Set(midPoint - halfSize, midPoint + halfSize);
	mCurrentDrawRect = mDrawRect;
}

//////////////////////////////////////////////////////////////////////

void AVIPlayer::CenterImageInWindowAndResetZoom()
{
	if(mTexture != null)
	{
		mDrawRect.Resize(mTexture->FSize());
		mScale = 1;
		CenterImageInWindow();
	}
}

//////////////////////////////////////////////////////////////////////

void AVIPlayer::OnLeftMouseDoubleClick(Point2D pos)
{
	if(mTexture != null)
	{
		ChangeSize(mTexture->Width(), mTexture->Height());
		mDrawRect.Set(Vec2::zero, mTexture->FSize());
	}
	MoveToMiddleOfMonitor();
	mOldClientRect = ClientRect();
	CenterImageInWindowAndResetZoom();
}

//////////////////////////////////////////////////////////////////////

void AVIPlayer::OnMouseMove(Point2D pos, uintptr flags)
{
	if(mDrag)
	{
		mCurrentDrawRect.Translate(Vec2(pos - mDragPos));
		mDrawRect = mCurrentDrawRect;
		mDragPos = pos;
	}
}

//////////////////////////////////////////////////////////////////////

bool AVIPlayer::OnUpdate()
{
	DXWindow::OnUpdate();

	if(StreamIsOpen())
	{
		int t = (int)(mTimer.Elapsed() / 1000.0f);
		int sample = AVIStreamTimeToSample(mStream, t);
		void *p = AVIStreamGetFrame(mStreamFrameGetter, sample);
		if(p != null)
		{
			memcpy(image.get(), p, movieSize.Width() * movieSize.Height() * sizeof(Color));
		}
		mTexture->Update((byte *)image.get());
	}

	mDeltaTime = mTimer.Delta();

	Vec2 tld = mDrawRect.TopLeft() - mCurrentDrawRect.TopLeft();
	Vec2 brd = mDrawRect.BottomRight() - mCurrentDrawRect.BottomRight();

	if(tld.Length() > 0 || brd.Length() > 0)
	{
		mCurrentDrawRect.topLeft += tld * 10.0f * (float)mDeltaTime;
		mCurrentDrawRect.bottomRight += brd * 10.0f * (float)mDeltaTime;

		tld = mDrawRect.TopLeft() - mCurrentDrawRect.TopLeft();
		brd = mDrawRect.BottomRight() - mCurrentDrawRect.BottomRight();

		if(tld.Length() < 1 && brd.Length() < 1)
		{
			mCurrentDrawRect = mDrawRect;
		}
	}


	return true;
}

//////////////////////////////////////////////////////////////////////

void AVIPlayer::OnDraw()
{
	using namespace DirectX;

	if(mTexture != null)
	{
		float hlfw = 2.0f / Width();
		float hlfh = -2.0f / Height();

		Matrix matrix(hlfw, 0.0f, 0.0f, 0.0f,
					  0.0f, hlfh, 0.0f, 0.0f,
					  0.0f, 0.0f, 1.0f, 0.0f,
					  -1.0f, 1.0f, 0.0f, 1.0f);

		UINT strides[] = { sizeof(Vertex) };
		UINT offsets[] = { 0 };

		float gridSize = 16;

		VertexShaderConstants vsConstants;
		PixelShaderConstants psConstants;

		SetQuad();

		vsConstants.matrix = XMMatrixTranspose(matrix);
		vsConstants.textureSize = mTexture->FSize();

		psConstants.channelMask = XMVectorSet(1, 1, 1, 1);
		psConstants.channelOffset = XMVectorSet(0, 0, 0, 0);
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

		mTexture->Activate();

	}
	Clear(mBackgroundColor);
	mContext->Draw(ARRAYSIZE(vert), 0);
}
