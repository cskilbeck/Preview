//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////

Window::Window(int width, int height)
	: mHWND(null)
	, mHINST(null)
	, mMouseDown(false)
{
	Init(width, height);
}

//////////////////////////////////////////////////////////////////////

static void CenterRectInMonitor(RECT &rc, HMONITOR hMonitor)
{
	MONITORINFO monitorInfo = {0};
	monitorInfo.cbSize = sizeof(monitorInfo);
	if(GetMonitorInfo(hMonitor, &monitorInfo))
	{
		int iMidX = (monitorInfo.rcWork.left + monitorInfo.rcWork.right) / 2;
		int iMidY = (monitorInfo.rcWork.top + monitorInfo.rcWork.bottom) / 2;
		int iRectWidth = rc.right - rc.left;
		int iRectHeight = rc.bottom - rc.top;
		rc.left = iMidX - iRectWidth / 2;
		rc.top = iMidY - iRectHeight / 2;
		rc.right = rc.left + iRectWidth;
		rc.bottom = rc.top + iRectHeight;
	}
}

//////////////////////////////////////////////////////////////////////

static void CentreRectInMonitorWithMouseInIt(RECT &rc)
{
	POINT ptCursorPos;
	GetCursorPos(&ptCursorPos);
	HMONITOR hMonitor = MonitorFromPoint(ptCursorPos, MONITOR_DEFAULTTOPRIMARY);
	if(hMonitor != INVALID_HANDLE_VALUE)
	{
		CenterRectInMonitor(rc, hMonitor);
	}
}

//////////////////////////////////////////////////////////////////////

void Window::Center()
{
	RECT rc;
	GetClientRect(mHWND, &rc);
	CenterRectInMonitor(rc, MonitorFromWindow(mHWND, MONITOR_DEFAULTTOPRIMARY));
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, true);
	SetWindowPos(mHWND, null, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOSIZE|SWP_NOZORDER);
}

//////////////////////////////////////////////////////////////////////

bool Window::Init(int width, int height)
{
	mHINST = GetModuleHandle(null);

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = sizeof(this);
	wcex.hInstance = 0;
	wcex.hIcon = null;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = null;
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_PREVIEW);
	wcex.lpszClassName = L"WindowClass";
	wcex.hIconSm = null;
	RegisterClassEx(&wcex);

	mWidth = width;
	mHeight = height;

	RECT rc = { 0, 0, width, height };

	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, true);
	CentreRectInMonitorWithMouseInIt(rc);

	mHWND = CreateWindowEx(	0,
							L"WindowClass",
							L"Screen",
							WS_OVERLAPPEDWINDOW,
							rc.left,
							rc.top,
							rc.right - rc.left, rc.bottom - rc.top,
							NULL,
							NULL,
							mHINST,
							this);
	if(!mHWND)
	{
		TRACE("Window Create Failed: %08x\n", GetLastError());
		return false;
	}

	SetWindowLongPtr(mHWND, GWLP_USERDATA, (LONG_PTR)this);

	GetClientRect(mHWND, &rc);
	if(!InitD3D())
	{
		Release();
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////

void Window::ChangeSize(int newWidth, int newHeight)
{
	mWidth = newWidth;
	mHeight = newHeight;
	RECT rc = { 0, 0, mWidth, mHeight };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, true);
	SetWindowPos(mHWND, null, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE|SWP_NOZORDER);
}

//////////////////////////////////////////////////////////////////////

bool Window::Update()
{
	MSG msg;

	while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TRACE(TEXT("%s: %08x,%08x (%d,%d)\n"), GetMessageName(msg.message).c_str(), msg.wParam, msg.lParam, msg.wParam, msg.lParam);
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.message != WM_QUIT;
}

//////////////////////////////////////////////////////////////////////

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_NCCREATE:
		SetWindowLongPtr(hWnd, 0, (LONG_PTR)(((CREATESTRUCT *)lParam)->lpCreateParams));

	case WM_GETMINMAXINFO:
		return DefWindowProc(hWnd, message, wParam, lParam);

	default:
		{
			Window *ths = (Window *)GetWindowLongPtr(hWnd, 0);
			if(ths != null)
			{
				return ths->HandleMessage(message, wParam, lParam);
			}
			else
			{
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////

LRESULT Window::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch(message)
	{
		case WM_PAINT:
			hdc = BeginPaint(mHWND, &ps);
			EndPaint(mHWND, &ps);
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_SIZE:
			switch(wParam)
			{
			case SIZE_MINIMIZED:	// pause it
				break;

			case SIZE_MAXIMIZED:
			case SIZE_RESTORED:
				if(!mInResizingLoop)
				{
					DoResize();
				}
				break;
			default:
				break;
			}
			break;

		case WM_ENTERSIZEMOVE:
			mInResizingLoop = true;
			break;

		case WM_EXITSIZEMOVE:
			mInResizingLoop = false;
			DoResize();
			break;

		case WM_MOUSEMOVE:
			if(mMouseDown)
			{
				POINTS p = MAKEPOINTS(lParam);
			}
			break;

		case WM_LBUTTONDOWN:
			{
				POINTS p = MAKEPOINTS(lParam);
				if(!mMouseDown)
				{
				}
				mMouseDown = true;
				SetCapture(mHWND);
			}
			break;

		case WM_LBUTTONUP:
			mMouseDown = false;
			ReleaseCapture();
			break;

		case WM_RBUTTONDOWN:
			break;

		case WM_RBUTTONUP:
			break;

		default:
			return DefWindowProc(mHWND, message, wParam, lParam);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////

void Window::DoResize()
{
	RECT rc;
	GetClientRect(mHWND, &rc);
	if(rc.right != mWidth || rc.bottom != mHeight)
	{
		mWidth = rc.right;
		mHeight = rc.bottom;

		if(mContext != null)
		{
			mContext->ClearState();
			mContext->OMSetRenderTargets(0, null, null);
			mContext->Flush();
		}

		ReleaseBackBuffer();

		DXGI_MODE_DESC d;
		d.Width = mWidth;
		d.Height = mHeight;
		d.RefreshRate.Numerator = 60;
		d.RefreshRate.Denominator = 1;
		d.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		d.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
		d.Scaling = DXGI_MODE_SCALING_STRETCHED;

		mSwapChain->ResizeTarget(&d);
		mSwapChain->ResizeBuffers(0, mWidth, mHeight, DXGI_FORMAT_UNKNOWN, 0);

		GetBackBuffer();
	}
}

//////////////////////////////////////////////////////////////////////

Window::~Window()
{
	Release();
}

//////////////////////////////////////////////////////////////////////

void Window::Release()
{
	mRenderTargetView.Release();
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

bool Window::InitD3D()
{
	CoInitializeEx(null, 0);

	RECT rc;
	GetClientRect(mHWND, &rc);

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
	sd.OutputWindow = mHWND;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	HRESULT hr;

	for(UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		mDriverType = driverTypes[driverTypeIndex];

		hr = D3D11CreateDeviceAndSwapChain(NULL, mDriverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
										   D3D11_SDK_VERSION, &sd, &mSwapChain, &mDevice, &mFeatureLevel, &mContext);
		if(SUCCEEDED(hr))
		{
			break;
		}
	}
	if(FAILED(hr))
	{
		MessageBox(null, L"Failed to initialize D3D!\nExiting...", L"Fatal Error", MB_ICONEXCLAMATION);
		Release();
		return false;
	}
	GetBackBuffer();
	return true;
}

//////////////////////////////////////////////////////////////////////

bool Window::GetBackBuffer()
{
	// Create a render target view
	ID3D11Texture2D* pBackBuffer = NULL;
	DXB(mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer));

	DXB(mDevice->CreateRenderTargetView(pBackBuffer, NULL, &mRenderTargetView));
	pBackBuffer->Release();

	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)Window::Width();
	vp.Height = (FLOAT)Window::Height();
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;

	mContext->RSSetViewports(1, &vp);
	mContext->OMSetRenderTargets(1, &mRenderTargetView, NULL);
	return true;
}

//////////////////////////////////////////////////////////////////////

void Window::ReleaseBackBuffer()
{
	ULONG refCount = mRenderTargetView.Release();
	TRACE("RELEASE BACK BUFFER: %d\n", refCount);
}

//////////////////////////////////////////////////////////////////////

void Window::Clear(Color color)
{
	float r = color.GetRed() / 255.0f;
	float g = color.GetGreen() / 255.0f;
	float b = color.GetBlue() / 255.0f;
	float a = 1.0f;
	float ClearColor[4] = { r, g, b, a };
	mContext->ClearRenderTargetView(mRenderTargetView, ClearColor);
}

//////////////////////////////////////////////////////////////////////

void Window::OnPaint(HDC dc, PAINTSTRUCT &ps)
{
}

//////////////////////////////////////////////////////////////////////

void Window::OnDraw()
{
}

//////////////////////////////////////////////////////////////////////

void Window::Show()
{
	ShowWindow(mHWND, SW_SHOW);
}

//////////////////////////////////////////////////////////////////////

void Window::Hide()
{
	ShowWindow(mHWND, SW_HIDE);
}

//////////////////////////////////////////////////////////////////////

LRESULT Window::OnPaintBackground(HDC dc)
{
	return TRUE;
}

//////////////////////////////////////////////////////////////////////

bool Window::OnUpdate()
{
	return true;
}

//////////////////////////////////////////////////////////////////////

void Window::OnResize()
{
}

//////////////////////////////////////////////////////////////////////

void Window::OnClosing()
{
}

//////////////////////////////////////////////////////////////////////

void Window::OnMouseMove(Point2D pos)
{
}

//////////////////////////////////////////////////////////////////////

void Window::OnLeftButtonDown(Point2D pos)
{
}

//////////////////////////////////////////////////////////////////////

void Window::OnLeftButtonUp(Point2D pos)
{
}

//////////////////////////////////////////////////////////////////////

void Window::OnRightButtonDown(Point2D pos)
{
}

//////////////////////////////////////////////////////////////////////

void Window::OnRightButtonUp(Point2D pos)
{
}

//////////////////////////////////////////////////////////////////////

void Window::OnChar(int key, uint32 flags)
{
}

//////////////////////////////////////////////////////////////////////

void Window::OnMouseWheel(int delta)
{
}

//////////////////////////////////////////////////////////////////////

void Window::Present()
{
	mSwapChain->Present(1, 0);
}