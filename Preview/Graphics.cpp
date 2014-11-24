//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Win32.h"
#include "d3d.h"

//////////////////////////////////////////////////////////////////////

struct GraphicsImpl
{
	//////////////////////////////////////////////////////////////////////

	GraphicsImpl()
		: mDriverType(D3D_DRIVER_TYPE_NULL)
		, mFeatureLevel(D3D_FEATURE_LEVEL_9_1)
		, mHWND(null)
	{
	}

	//////////////////////////////////////////////////////////////////////

	void Release()
	{
		mRenderTargetView.Release();
		mSwapChain.Release();

		if(Graphics::Context != null)
		{
			Graphics::Context->ClearState();
			Graphics::Context->Flush();
		}

		Graphics::Context.Release();

		if(Graphics::Device != null)
		{
			#if defined(DEBUG)
				DXPtr<ID3D11Debug> D3DDebug;
				Graphics::Device->QueryInterface(__uuidof(ID3D11Debug), (void **)&D3DDebug);
				D3DDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
				D3DDebug.Release();
			#endif
			Graphics::Device.Release();
		}

		mHWND = null;
	}

	//////////////////////////////////////////////////////////////////////

	bool Init(HWND hWnd)
	{
		mHWND = hWnd;

		CoInitializeEx(null, 0);

		RECT rc;
		GetClientRect(hWnd, &rc);

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
		sd.BufferDesc.Width = Window::Width();
		sd.BufferDesc.Height = Window::Height();
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hWnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;

		HRESULT hr;

		for(UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
		{
			mDriverType = driverTypes[driverTypeIndex];

			hr = D3D11CreateDeviceAndSwapChain(NULL, mDriverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
				D3D11_SDK_VERSION, &sd, &mSwapChain, &Graphics::Device, &mFeatureLevel, &Graphics::Context);
			if(SUCCEEDED(hr))
			{
				break;
			}
		}
		if(FAILED(hr))
		{
			MessageBox(null, L"Failed to initialize D3D!\nExiting...", L"Fatal Error", MB_ICONEXCLAMATION);
			Release();
			exit(0);
			return false;
		}

		GetBackBuffer();

		return true;
	}

	//////////////////////////////////////////////////////////////////////

	void Resize(int width, int height)
	{
		if(Graphics::Context != null)
		{
			Graphics::Context->ClearState();
		}

		ReleaseBackBuffer();

		DXGI_MODE_DESC d;
		d.Width = width;
		d.Height = height;
		d.RefreshRate.Numerator = 60;
		d.RefreshRate.Denominator = 1;
		d.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		d.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		d.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		mSwapChain->ResizeTarget(&d);
		mSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

		GetBackBuffer();
	}

	//////////////////////////////////////////////////////////////////////

	void ClearBackBuffer(Color color)
	{
		float r = color.GetRed() / 255.0f;
		float g = color.GetGreen() / 255.0f;
		float b = color.GetBlue() / 255.0f;
		float a = 1.0f;
		float ClearColor[4] = { r, g, b, a };
		Graphics::Context->ClearRenderTargetView(mRenderTargetView, ClearColor);
	}

	//////////////////////////////////////////////////////////////////////

	void ReleaseBackBuffer()
	{
		mRenderTargetView.Release();
	}

	//////////////////////////////////////////////////////////////////////

	void GetBackBuffer()
	{
		// Create a render target view
		ID3D11Texture2D* pBackBuffer = NULL;
		if(FAILED(mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer)))
		{
			Release();
			return;
		}

		HRESULT hr = Graphics::Device->CreateRenderTargetView(pBackBuffer, NULL, &mRenderTargetView);
		pBackBuffer->Release();
		if(FAILED(hr))
		{
			Release();
			return;
		}

		D3D11_VIEWPORT vp;
		vp.Width = (FLOAT)Window::Width();
		vp.Height = (FLOAT)Window::Height();
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;

		Graphics::Context->RSSetViewports(1, &vp);
		Graphics::Context->OMSetRenderTargets(1, &mRenderTargetView, NULL);
	}

	//////////////////////////////////////////////////////////////////////

	HWND							mHWND;
	DXPtr<ID3D11RenderTargetView>	mRenderTargetView;
	DXPtr<IDXGISwapChain>			mSwapChain;
	D3D_DRIVER_TYPE					mDriverType;
	D3D_FEATURE_LEVEL				mFeatureLevel;
};

//////////////////////////////////////////////////////////////////////

static GraphicsImpl impl;

//////////////////////////////////////////////////////////////////////

namespace Graphics
{
	DXPtr<ID3D11Device>			Device;
	DXPtr<ID3D11DeviceContext>	Context;

	bool Init(HWND hWnd)
	{
		bool rc = impl.Init(hWnd);
		return rc;
	}

	//////////////////////////////////////////////////////////////////////

	void Release()
	{
		impl.Release();
	}

	//////////////////////////////////////////////////////////////////////

	void Resize(int width, int height)
	{
		impl.Resize(width, height);
	}

	//////////////////////////////////////////////////////////////////////

	void ClearBackBuffer(Color color)
	{
		impl.ClearBackBuffer(color);
	}

	//////////////////////////////////////////////////////////////////////

	void Present()
	{
		impl.mSwapChain->Present(1, 0);
	}

	//////////////////////////////////////////////////////////////////////

	void SetScissorRectangle(Rect2D const &rect)
	{
		assert(false);
	}

	//////////////////////////////////////////////////////////////////////

	void EnableScissoring(bool enable)
	{
		assert(false);
	}

}