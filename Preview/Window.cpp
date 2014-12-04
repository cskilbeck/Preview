//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////

Window::Window(int width, int height, tchar const *caption)
	: mHWND(null)
	, mHINST(null)
	, mMenu(null)
	, mMouseDown(false)
	, mMessageWait(false)
	, mCaption(caption)
{
	if(!Init(width, height))
	{
		Release();
		DestroyWindow(mHWND);
	}
}

//////////////////////////////////////////////////////////////////////

static void CenterRectInMonitor(Rect &rc, HMONITOR hMonitor)
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

static void CentreRectInMonitorWithMouseInIt(Rect &rc)
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
	Rect rc;
	GetClientRect(mHWND, &rc);
	CenterRectInMonitor(rc, MonitorFromWindow(mHWND, MONITOR_DEFAULTTOPRIMARY));
	AdjustWindowRectEx(&rc, WS_OVERLAPPEDWINDOW, true, 0);
	SetWindowPos(mHWND, null, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOSIZE|SWP_NOZORDER);
}

//////////////////////////////////////////////////////////////////////

bool Window::Init(int width, int height)
{
	mHINST = GetModuleHandle(null);
	mMenu = LoadMenu(mHINST, MAKEINTRESOURCE(IDC_PREVIEW));

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = sizeof(Window *);
	wcex.hInstance = 0;
	wcex.hIcon = null;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = NULL;
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = TEXT("WindowClass");
	wcex.hIconSm = null;
	RegisterClassEx(&wcex);

	mWidth = width;
	mHeight = height;

	Rect rect(100, 100, width, height);
	AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, true, 0);

	CentreRectInMonitorWithMouseInIt(rect);

	mHWND = CreateWindowEx(0, TEXT("WindowClass"), mCaption.c_str(), WS_OVERLAPPEDWINDOW,
						   rect.left, rect.top, rect.Width(), rect.Height(),
						   NULL, mMenu, mHINST, this);
	if(mHWND == null)
	{
		DWORD err = GetLastError();
		ErrorMessageBox(TEXT("Failed to create window (%08x) - %s"), err, Win32ErrorMessage(err).c_str());
		return false;
	}
	return OpenD3D();
}

//////////////////////////////////////////////////////////////////////

void Window::ChangeSize(int newWidth, int newHeight)
{
	mWidth = newWidth;
	mHeight = newHeight;
	Rect rc(0, 0, mWidth, mHeight);
	AdjustWindowRectEx(&rc, WS_OVERLAPPEDWINDOW, true, 0);
	SetWindowPos(mHWND, null, 0, 0, rc.Width(), rc.Height(), SWP_NOMOVE|SWP_NOZORDER);
}

//////////////////////////////////////////////////////////////////////

bool Window::Update()
{
	if(mHWND == null)
	{
		return false;
	}

	MSG msg;

	if(mMessageWait && WaitMessage() == 0)
	{
		return false;
	}
	while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	OnUpdate();
	return msg.message != WM_QUIT;
}

//////////////////////////////////////////////////////////////////////

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Window *window;
	LRESULT rc;

	switch(message)
	{
	case WM_NCCREATE:
		window = (Window *)((CREATESTRUCT *)lParam)->lpCreateParams;
		SetWindowLongPtr(hWnd, 0, (LONG_PTR)window);
		window->mHWND = hWnd;
		rc = true;
		break;

	case WM_GETMINMAXINFO:
		rc = DefWindowProc(hWnd, message, wParam, lParam);
		break;

	default:
		rc = ((Window *)GetWindowLongPtr(hWnd, 0))->HandleMessage(hWnd, message, wParam, lParam);
	}
	//TRACE(TEXT("%s: %08x,%08x (%d,%d) returns %d\n"), GetMessageName(message).c_str(), wParam, lParam, wParam, lParam, rc);
	return rc;
}

//////////////////////////////////////////////////////////////////////

LRESULT Window::HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_ERASEBKGND:
			return true;

		case WM_SIZING:
			DoResize();
			OnDraw();
			Present();
			break;

		case WM_MOUSEMOVE:
			break;

		case WM_LBUTTONDOWN:
			SetCapture(hWnd);
			break;

		case WM_LBUTTONUP:
			ReleaseCapture();
			break;

		case WM_RBUTTONDOWN:
			SetCapture(hWnd);
			break;

		case WM_RBUTTONUP:
			ReleaseCapture();
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////

void Window::DoResize()
{
	Rect rc;
	GetClientRect(mHWND, &rc);
	if(rc.Width() != mWidth || rc.Height() != mHeight)
	{
		mWidth = rc.Width();
		mHeight = rc.Height();
		ResizeD3D();
	}
}

//////////////////////////////////////////////////////////////////////

void Window::ResizeD3D()
{
	if(mContext != null)
	{
		mContext->ClearState();
		mContext->OMSetRenderTargets(0, null, null);
		mContext->Flush();
	}

	ReleaseBackBuffer();
	mSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
	GetBackBuffer();
}

//////////////////////////////////////////////////////////////////////

Window::~Window()
{
	Release();
}

//////////////////////////////////////////////////////////////////////

void Window::Release()
{
	CloseD3D();
}

//////////////////////////////////////////////////////////////////////

bool Window::OpenD3D()
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

		hr = D3D11CreateDeviceAndSwapChain(NULL, mDriverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
										   D3D11_SDK_VERSION, &sd, &mSwapChain, &mDevice, &mFeatureLevel, &mContext);
		if(SUCCEEDED(hr))
		{
			break;
		}
	}
	if(FAILED(hr))
	{
		ErrorMessageBox(TEXT("Failed to initialize Direct3D!\nExiting..."));
		Release();
		return false;
	}
	GetBackBuffer();
	return true;
}

//////////////////////////////////////////////////////////////////////

void Window::CloseD3D()
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

bool Window::GetBackBuffer()
{
	DXPtr<ID3D11Texture2D> pBackBuffer;
	DXB(mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer));
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

void Window::ReleaseBackBuffer()
{
	if(mContext != null)
	{
		mContext->OMSetRenderTargets(0, NULL, NULL);
	}
	mRenderTargetView.Release();
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