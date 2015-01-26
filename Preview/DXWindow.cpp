//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////

DXWindow::DXWindow(int width, int height, tchar const *caption)
	: Window(width, height, caption, WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_OVERLAPPEDWINDOW|WS_SIZEBOX|WS_BORDER)
	, mFrame(0)
	, mDXWindow(null)
{
	mMessageWait = false;
}

//////////////////////////////////////////////////////////////////////

bool DXWindow::OnCreate()
{
	return Window::OnCreate() && OpenD3D();
}

//////////////////////////////////////////////////////////////////////

void DXWindow::OnDestroy()
{
	CloseD3D();
}

//////////////////////////////////////////////////////////////////////

DXWindow::~DXWindow()
{
}

//////////////////////////////////////////////////////////////////////

void DXWindow::OnPaint(PAINTSTRUCT &ps)
{
	// Don't call Window::OnPaint(ps);
}

//////////////////////////////////////////////////////////////////////

void DXWindow::OnDraw()
{
	// override this...
	Clear(Color::Black);
}

//////////////////////////////////////////////////////////////////////

void DXWindow::OnResized()
{
	if(ResizeD3D())
	{
		OnDraw();
		Present();
	}
}

//////////////////////////////////////////////////////////////////////

bool DXWindow::OnUpdate()
{
	OnDraw();
	Present();
	++mFrame;
	return true;
}

//////////////////////////////////////////////////////////////////////

void DXWindow::Present()
{
	mSwapChain->Present(1, 0);
}

//////////////////////////////////////////////////////////////////////

bool DXWindow::OpenD3D()
{
	CoInitializeEx(null, 0);

	UINT createDeviceFlags = 0;

#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE
		//D3D_DRIVER_TYPE_WARP,
		//D3D_DRIVER_TYPE_REFERENCE
	};

	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_10_0
		//D3D_FEATURE_LEVEL_9_1	// CRASHES!? In texture loader...
		//D3D_FEATURE_LEVEL_11_0
		//D3D_FEATURE_LEVEL_10_1
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);



	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = mDXWindow != null ? mDXWindow : mHWND;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	HRESULT hr;
	for(UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		mDriverType = driverTypes[driverTypeIndex];

		hr = DXB(D3D11CreateDeviceAndSwapChain(null, mDriverType, null, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &mSwapChain, &gDevice, &mFeatureLevel, &mContext));
		if(SUCCEEDED(hr))
		{
			break;
		}
	}
	if(FAILED(hr))
	{
		ErrorMessageBox(TEXT("Failed to initialize Direct3D!\nExiting..."));
		CloseD3D();
		return false;
	}

	GetBackBuffer();

	return true;
}

//////////////////////////////////////////////////////////////////////

bool DXWindow::ResizeD3D()
{
	if(mContext != null)
	{
		mContext->ClearState();
		mContext->Flush();
	}

	ReleaseBackBuffer();
	if(mSwapChain != null)
	{
		mSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
		return GetBackBuffer();
	}
	return false;
}

//////////////////////////////////////////////////////////////////////

void DXWindow::CloseD3D()
{
	ReleaseBackBuffer();

	mSwapChain.Release();

	if(mContext != null)
	{
		mContext->ClearState();
		mContext->Flush();
	}

	mContext.Release();

#if defined(DEBUG)
	if(gDevice != null)
	{
		DXPtr<ID3D11Debug> D3DDebug;
		gDevice->QueryInterface(__uuidof(ID3D11Debug), (void **)&D3DDebug);
		D3DDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL);
		D3DDebug.Release();
	}
#endif

	gDevice.Release();
}

//////////////////////////////////////////////////////////////////////

bool DXWindow::GetBackBuffer()
{
	DXPtr<ID3D11Texture2D> pBackBuffer;
	DXB(mSwapChain->GetBuffer(0, __uuidof(pBackBuffer), (LPVOID*)&pBackBuffer));
	DXB(gDevice->CreateRenderTargetView(pBackBuffer, null, &mRenderTargetView));

	D3D11_VIEWPORT vp = { 0 };
	vp.Width = FWidth();
	vp.Height = FHeight();

	mContext->RSSetViewports(1, &vp);
	mContext->OMSetRenderTargets(1, &mRenderTargetView, null);
	return true;
}

//////////////////////////////////////////////////////////////////////

void DXWindow::ReleaseBackBuffer()
{
	if(mContext != null)
	{
		mContext->OMSetRenderTargets(0, null, null);
	}
	mRenderTargetView.Release();
}

//////////////////////////////////////////////////////////////////////

void DXWindow::Clear(Color color)
{
	float rgba[4];
	mContext->ClearRenderTargetView(mRenderTargetView, color.GetFloatsRGBA(rgba));
}
