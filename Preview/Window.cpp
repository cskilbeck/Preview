//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////

static void CenterRectInMonitor(Rect2D &rc, HMONITOR hMonitor)
{
	if(hMonitor != INVALID_HANDLE_VALUE)
	{
		MONITORINFO monitorInfo = { 0 };
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
}

//////////////////////////////////////////////////////////////////////

static void CentreRectInMonitorWithMouseInIt(Rect2D &rc)
{
	Point2D ptCursorPos;
	GetCursorPos(&ptCursorPos);
	CenterRectInMonitor(rc, MonitorFromPoint(ptCursorPos, MONITOR_DEFAULTTOPRIMARY));
}

//////////////////////////////////////////////////////////////////////

static void CentreRectInDefaultMonitor(Rect2D &rc)
{
	CenterRectInMonitor(rc, MonitorFromPoint(Point2D(0, 0), MONITOR_DEFAULTTOPRIMARY));
}

//////////////////////////////////////////////////////////////////////

Window::Window(int width, int height, tchar const *caption)
	: mHWND(null)
	, mHINST(null)
	, mLeftMouseDown(false)
	, mRightMouseDown(false)
	, mMessageWait(true)
	, mCaption(caption)
{
	if(!Init(width, height))
	{
		Close();
	}
}

//////////////////////////////////////////////////////////////////////

void Window::MoveToMiddleOfMonitor()
{
	Rect2D rc = ClientRect();
	CenterRectInMonitor(rc, MonitorFromWindow(mHWND, MONITOR_DEFAULTTOPRIMARY));
	AdjustWindowRectEx(&rc, WS_OVERLAPPEDWINDOW, GetMenu(mHWND) != null, 0);
	SetWindowPos(mHWND, null, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOSIZE|SWP_NOZORDER);
}

//////////////////////////////////////////////////////////////////////

bool Window::Init(int width, int height)
{
	mHINST = GetModuleHandle(null);

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
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

	Rect2D rect(0, 0, width, height);

	mHWND = CreateWindowEx(0, TEXT("WindowClass"), mCaption.c_str(), WS_OVERLAPPEDWINDOW,
						   rect.left, rect.top, rect.Width(), rect.Height(),
						   NULL, null, mHINST, this);
	if(mHWND == null)
	{
		DWORD err = GetLastError();
		ErrorMessageBox(TEXT("Failed to create window (%08x) - %s"), err, Win32ErrorMessage(err).c_str());
		return false;
	}
	PostMessage(mHWND, WM_USER, 0, 0);				// this won't get processed until message handler picks it up
	return true;
}

//////////////////////////////////////////////////////////////////////

bool Window::OnCreate()
{
	MoveToMiddleOfMonitor();
	return true;
}

//////////////////////////////////////////////////////////////////////

void Window::SetWindowRect(Rect2D const &r)
{
	SetWindowPos(mHWND, null, r.left, r.top, r.Width(), r.Height(), SWP_NOZORDER);
}

//////////////////////////////////////////////////////////////////////

long Window::GetStyle() const
{
	return GetWindowLong(mHWND, GWL_STYLE);
}

//////////////////////////////////////////////////////////////////////

long Window::GetExStyle() const
{
	return GetWindowLong(mHWND, GWL_EXSTYLE);
}

//////////////////////////////////////////////////////////////////////

bool Window::HasMenu() const
{
	return GetMenu(mHWND) != null;
}

//////////////////////////////////////////////////////////////////////

bool Window::IsResizable() const
{
	return (GetStyle() & WS_THICKFRAME) != 0;
}

//////////////////////////////////////////////////////////////////////

Rect2D Window::GetBorders() const
{
	Rect2D rc(0, 0, 0, 0);
	AdjustWindowRectEx(&rc, GetStyle(), HasMenu(), GetExStyle());
	return rc;
}

//////////////////////////////////////////////////////////////////////

Rect2D Window::GetWindowRectFromClientRect(Rect2D const &clientRect)
{
	Rect2D rc = GetBorders();
	int minSize = IsResizable() ? GetSystemMetrics(SM_CXMINTRACK) : 0;
	rc.left += clientRect.left;
	rc.top += clientRect.top;
	rc.right += clientRect.right;
	rc.bottom += clientRect.bottom;
	rc.right = rc.left + Max(rc.Width(), minSize);
	return rc;
}

//////////////////////////////////////////////////////////////////////

Rect2D Window::GetClientRectFromWindowRect(Rect2D const &windowRect)
{
	Rect2D rc = GetBorders();
	Point2D tl(windowRect.left - rc.left, windowRect.top - rc.top);
	Point2D br(windowRect.right - rc.right, windowRect.bottom - rc.bottom);
	return Rect2D(tl, Size2D(br - tl));
}

//////////////////////////////////////////////////////////////////////

void Window::ResizeWindow(int newWidth, int newHeight)
{
	Rect2D oldRect;
	Rect2D rc = GetWindowRectFromClientRect(Rect2D(0, 0, newWidth, newHeight));
	GetWindowRect(mHWND, &oldRect);
	rc.MoveTo(oldRect.MidPoint() - rc.HalfSize());
	rc.left = Max(0L, rc.left);
	rc.top = Max(0L, rc.top);
	SetWindowRect(rc);
}

//////////////////////////////////////////////////////////////////////

void Window::ChangeSize(int newWidth, int newHeight)
{
	Rect2D rc(0, 0, newWidth, newHeight);
	AdjustWindowRectEx(&rc, GetStyle(), HasMenu(), GetExStyle());
	SetWindowPos(mHWND, null, 0, 0, rc.Width(), rc.Height(), SWP_NOMOVE | SWP_NOZORDER);
}

//////////////////////////////////////////////////////////////////////

bool Window::Update()
{
	MSG msg;

	if(mHWND == null || mMessageWait && WaitMessage() == 0)
	{
		return false;
	}
	while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.message != WM_QUIT ? OnUpdate() : false;
}

//////////////////////////////////////////////////////////////////////
// The 1st WM_GETMINMAXINFO message will not get routed

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Window *w;

	if(msg == WM_NCCREATE)
	{
		w = (Window *)((CREATESTRUCT *)lParam)->lpCreateParams;
		SetWindowLongPtr(hWnd, 0, (LONG_PTR)w);
		w->mHWND = hWnd;
	}
	else
	{
		w = (Window *)GetWindowLongPtr(hWnd, 0);
	}

	return w ? w->HandleMessage(hWnd, msg, wParam, lParam) : DefWindowProc(hWnd, msg, wParam, lParam);
}

//////////////////////////////////////////////////////////////////////

LRESULT Window::HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MINMAXINFO *mminfo;
	PAINTSTRUCT ps;
	Point2D mousePos;

	switch(msg)
	{
		case WM_GETMINMAXINFO:
			mminfo = (MINMAXINFO *)lParam;
			mminfo->ptMinTrackSize.y = 16 + GetSystemMetrics(SM_CYMINTRACK) + (HasMenu() ? GetSystemMetrics(SM_CYMENU) : 0);
			break;

		case WM_USER:
			if(!OnCreate())
			{
				Close();
			}
			else
			{
				Show();
			}
			break;

		case WM_DESTROY:
			OnDestroy();
			PostQuitMessage(0);
			break;

		case WM_ERASEBKGND:
			return true;

		case WM_SIZE:
			DoResize();
			break;

		case WM_SIZING:
			DoResize();
			break;

		case WM_PAINT:
			BeginPaint(mHWND, &ps);
			OnPaint(ps);
			EndPaint(mHWND, &ps);
			break;

		case WM_CHAR:
			OnChar((int)wParam, lParam);
			break;

		case WM_KEYDOWN:
			OnKeyDown((int)wParam, lParam);
			break;

		case WM_KEYUP:
			OnKeyUp((int)wParam, lParam);
			break;

		case WM_MOUSEWHEEL:
			mousePos = GetPointFromParam(lParam);
			ScreenToClient(hWnd, &mousePos);
			OnMouseWheel(mousePos, (int16)HIWORD(wParam), wParam);
			break;

		case WM_MOUSEMOVE:
			OnMouseMove(GetPointFromParam(lParam), wParam);
			break;

		case WM_LBUTTONDBLCLK:
			OnLeftMouseDoubleClick(GetPointFromParam(lParam));
			break;

		case WM_RBUTTONDBLCLK:
			OnRightMouseDoubleClick(GetPointFromParam(lParam));
			break;

		case WM_LBUTTONDOWN:
			SetCapture(hWnd);
			mLeftMouseDown = true;
			OnLeftButtonDown(GetPointFromParam(lParam), wParam);
			break;

		case WM_LBUTTONUP:
			ReleaseCapture();
			mLeftMouseDown = false;
			OnLeftButtonUp(GetPointFromParam(lParam), wParam);
			break;

		case WM_RBUTTONDOWN:
			SetCapture(hWnd);
			mRightMouseDown = true;
			OnRightButtonDown(GetPointFromParam(lParam), wParam);
			break;

		case WM_RBUTTONUP:
			ReleaseCapture();
			mRightMouseDown = false;
			OnRightButtonUp(GetPointFromParam(lParam), wParam);
			break;

		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////

void Window::DoResize()
{
	Rect2D rc;
	GetClientRect(mHWND, &rc);
	mWidth = rc.Width();
	mHeight = rc.Height();
	OnResize();
}

//////////////////////////////////////////////////////////////////////

Window::~Window()
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

void Window::OnPaint(PAINTSTRUCT &ps)
{
	FillRect(ps.hdc, &ClientRect(), (HBRUSH)GetStockObject(WHITE_BRUSH));
	tchar hello[] = TEXT("Override your OnPaint function, fool!");
	TextOut(ps.hdc, 0, 0, hello, ARRAYSIZE(hello) - 1);
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

void Window::OnDestroy()
{
}

//////////////////////////////////////////////////////////////////////

void Window::OnMouseMove(Point2D pos, uintptr flags)
{
}

//////////////////////////////////////////////////////////////////////

void Window::OnLeftMouseDoubleClick(Point2D pos)
{
}

//////////////////////////////////////////////////////////////////////

void Window::OnRightMouseDoubleClick(Point2D pos)
{
}

//////////////////////////////////////////////////////////////////////

void Window::OnLeftButtonDown(Point2D pos, uintptr flags)
{
}

//////////////////////////////////////////////////////////////////////

void Window::OnLeftButtonUp(Point2D pos, uintptr flags)
{
}

//////////////////////////////////////////////////////////////////////

void Window::OnRightButtonDown(Point2D pos, uintptr flags)
{
}

//////////////////////////////////////////////////////////////////////

void Window::OnRightButtonUp(Point2D pos, uintptr flags)
{
}

//////////////////////////////////////////////////////////////////////

void Window::OnChar(int key, uintptr flags)
{
}

//////////////////////////////////////////////////////////////////////

void Window::OnKeyDown(int key, uintptr flags)
{
}

//////////////////////////////////////////////////////////////////////

void Window::OnKeyUp(int key, uintptr flags)
{
}

//////////////////////////////////////////////////////////////////////

void Window::OnMouseWheel(Point2D pos, int delta, uintptr flags)
{
}

//////////////////////////////////////////////////////////////////////

void Window::Close()
{
	DestroyWindow(mHWND);
}
