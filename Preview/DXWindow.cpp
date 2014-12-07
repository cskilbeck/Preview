//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////

DXWindow::DXWindow(int width, int height, tchar const *caption)
	: Window(width, height, caption)
	, mFrame(0)
{
	mMessageWait = false;
}

//////////////////////////////////////////////////////////////////////

bool DXWindow::OnCreate()
{
	return OpenD3D();
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
}

//////////////////////////////////////////////////////////////////////

void DXWindow::OnDraw()
{
	Clear(Color(16, 64, 32));
}

//////////////////////////////////////////////////////////////////////

void DXWindow::OnResize()
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
	if(mFrame == 1)
	{
		Show();
	}
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
	sd.BufferDesc.Width = Width();
	sd.BufferDesc.Height = Height();
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = mHWND;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	HRESULT hr;
	for(UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		mDriverType = driverTypes[driverTypeIndex];

		hr = DXB(D3D11CreateDeviceAndSwapChain(NULL, mDriverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &mSwapChain, &mDevice, &mFeatureLevel, &mContext));
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

	Texture::SetDeviceAndContext(mDevice, mContext);

	return true;
}

//////////////////////////////////////////////////////////////////////

bool DXWindow::ResizeD3D()
{
	if(mContext != null)
	{
		mContext->ClearState();
		mContext->OMSetRenderTargets(0, null, null);
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
	Texture::ReleaseDeviceAndContext();

	ReleaseBackBuffer();

	mSwapChain.Release();

	if(mContext != null)
	{
		mContext->ClearState();
		mContext->Flush();
	}

	mContext.Release();

#if defined(DEBUG)
	if(mDevice != null)
	{
		DXPtr<ID3D11Debug> D3DDebug;
		mDevice->QueryInterface(__uuidof(ID3D11Debug), (void **)&D3DDebug);
		D3DDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL);
		D3DDebug.Release();
	}
#endif

	mDevice.Release();
}

//////////////////////////////////////////////////////////////////////

bool DXWindow::GetBackBuffer()
{
	DXPtr<ID3D11Texture2D> pBackBuffer;
	DXB(mSwapChain->GetBuffer(0, __uuidof(pBackBuffer), (LPVOID*)&pBackBuffer));
	DXB(mDevice->CreateRenderTargetView(pBackBuffer, NULL, &mRenderTargetView));

	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = (FLOAT)Width();
	vp.Height = (FLOAT)Height();
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;

	mContext->RSSetViewports(1, &vp);
	mContext->OMSetRenderTargets(1, &mRenderTargetView, NULL);
	return true;
}

//////////////////////////////////////////////////////////////////////

void DXWindow::ReleaseBackBuffer()
{
	if(mContext != null)
	{
		mContext->OMSetRenderTargets(0, NULL, NULL);
	}
	mRenderTargetView.Release();
}

//////////////////////////////////////////////////////////////////////

void DXWindow::Clear(Color color)
{
	float f[4];
	mContext->ClearRenderTargetView(mRenderTargetView, color.GetFloats(f));
}
