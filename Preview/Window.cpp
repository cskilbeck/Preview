//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////

BOOL UnadjustWindowRectEx(LPRECT prc, DWORD dwStyle, BOOL fMenu, DWORD dwExStyle)
{
	RECT rc;
	SetRectEmpty(&rc);
	BOOL fRc = AdjustWindowRectEx(&rc, dwStyle, fMenu, dwExStyle);
	if (fRc) {
		prc->left -= rc.left;
		prc->top -= rc.top;
		prc->right -= rc.right;
		prc->bottom -= rc.bottom;
	}
	return fRc;
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

Window::Window(int clientWidth, int clientHeight, TCHAR *caption)
	: mHWND(null)
	, mHINST(null)
{
	Create(clientWidth, clientHeight, caption);
}

//////////////////////////////////////////////////////////////////////

Window::~Window()
{
}

//////////////////////////////////////////////////////////////////////

void Window::Close()
{
	OnClosing();
	DestroyWindow(mHWND);
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

void Window::Center()
{
	RECT rc;
	GetClientRect(mHWND, &rc);
	CenterRectInMonitor(rc, MonitorFromWindow(mHWND, MONITOR_DEFAULTTOPRIMARY));
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, true);
	SetWindowPos(mHWND, null, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOSIZE|SWP_NOZORDER);
}

//////////////////////////////////////////////////////////////////////

void Window::Create(int width, int height, TCHAR *caption)
{
	mHINST = GetModuleHandle(null);

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
							caption,
							WS_OVERLAPPEDWINDOW,
							rc.left,
							rc.top,
							rc.right - rc.left, rc.bottom - rc.top,
							NULL,
							NULL,
							mHINST,
							this);
	assert(mHWND != null);
}
	
//////////////////////////////////////////////////////////////////////

void Window::Resize(int clientWidth, int clientHeight)
{
	mWidth = clientWidth;
	mHeight = clientHeight;
	RECT rc = { 0, 0, mWidth, mHeight };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, true);
	SetWindowPos(mHWND, null, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE|SWP_NOZORDER);
}

//////////////////////////////////////////////////////////////////////

Size2D GetSizeForClientSize(Size2D clientSize)
{
	RECT rc = { 0, 0, clientSize.w, clientSize.h };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, true);
	return Size2D(rc.right - rc.left, rc.bottom - rc.top);
}

//////////////////////////////////////////////////////////////////////

uint Window::Width() const
{
	return mWidth;
}

//////////////////////////////////////////////////////////////////////

uint Window::Height() const
{
	return mHeight;
}

//////////////////////////////////////////////////////////////////////

bool Window::Update(bool waitForMessage)
{
	MSG msg;

	if(waitForMessage)
	{
		BOOL r;
		while((r = GetMessage( &msg, NULL, 0, 0 )) != 0)
		{ 
			if (r == -1)
			{
				return false;
			}
			else
			{
				TranslateMessage(&msg); 
				DispatchMessage(&msg); 
			}
		}
	}
	else
	{
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return msg.message != WM_QUIT;
}

//////////////////////////////////////////////////////////////////////

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	TRACE(TEXT("%s [%08x,%08x] (%d,%d)\n"), GetMessageName(message).c_str(), wParam, lParam, wParam, lParam);

	switch(message)
	{
	case WM_NCCREATE:
		SetWindowLongPtr(hWnd, 0, (LONG_PTR)(((CREATESTRUCT *)lParam)->lpCreateParams));
		return DefWindowProc(hWnd, message, wParam, lParam);

	case WM_GETMINMAXINFO:
		return DefWindowProc(hWnd, message, wParam, lParam);

	default:
		{
			Window *window = (Window *)GetWindowLongPtr(hWnd, 0);
			if(window != null)
			{
				return window->HandleMessage(hWnd, message, wParam, lParam);
			}
			else
			{
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////

LRESULT Window::HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	POINTS p;
	WINDOWPOS *windowPos;
	RECT rc;

	switch(message)
	{
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			OnPaint(hdc, ps);
			EndPaint(hWnd, &ps);
			break;

		case WM_ERASEBKGND:
			return OnPaintBackground((HDC)wParam);

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_WINDOWPOSCHANGING:
			windowPos = (WINDOWPOS *)lParam;
			SetRect(&rc, windowPos->x, windowPos->y, windowPos->cx + windowPos->x, windowPos->cy + windowPos->y);
			UnadjustWindowRectEx(&rc, WS_OVERLAPPED, TRUE, 0);
			mWidth = rc.right - rc.left;
			mHeight = rc.bottom - rc.top;
			OnResize();
			break;

		case WM_SIZE:
			Sized();
			break;

		case WM_CHAR:
			OnChar((int)wParam, lParam);
			break;

		case WM_MOUSEMOVE:
			p = MAKEPOINTS(lParam);
			OnMouseMove(Point2D(p.x, p.y));
			break;

		case WM_LBUTTONDOWN:
			p = MAKEPOINTS(lParam);
			SetCapture(hWnd);
			OnLeftButtonDown(Point2D(p.x, p.y));
			break;

		case WM_LBUTTONUP:
			p = MAKEPOINTS(lParam);
			ReleaseCapture();
			OnLeftButtonUp(Point2D(p.x, p.y));
			break;

		case WM_RBUTTONDOWN:
			p = MAKEPOINTS(lParam);
			SetCapture(hWnd);
			OnRightButtonDown(Point2D(p.x, p.y));
			break;

		case WM_RBUTTONUP:
			p = MAKEPOINTS(lParam);
			ReleaseCapture();
			OnRightButtonUp(Point2D(p.x, p.y));
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////

void Window::Sized()
{
	RECT rc;
	GetClientRect(mHWND, &rc);
	if(rc.right != mWidth || rc.bottom != mHeight)
	{
		mWidth = rc.right;
		mHeight = rc.bottom;
	}
}

