//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////

namespace
{
	struct WindowImpl
	{
		static WindowImpl *g_WindowImpl;

		//////////////////////////////////////////////////////////////////////

		WindowImpl()
			: mHWND(null)
			, mHINST(null)
			, OnResize(null)
			, mInResizingLoop(false)
			, mMouseDown(false)
		{
			g_WindowImpl = this;
		}

		//////////////////////////////////////////////////////////////////////

		static bool IsWindowImpl(WindowImpl *i)
		{
			return i == g_WindowImpl;
		}

		//////////////////////////////////////////////////////////////////////

		int						mWidth;
		int						mHeight;
		HWND					mHWND;
		HINSTANCE				mHINST;
		bool					mInResizingLoop;
		bool					mMouseDown;

		std::function<void ()>	OnResize;

		//////////////////////////////////////////////////////////////////////

		void EnableScissoring(bool enable)
		{
			Graphics::EnableScissoring(enable);
		}

		//////////////////////////////////////////////////////////////////////

		void SetScissorRectangle(Rect2D const &rect)
		{
			Graphics::SetScissorRectangle(rect);
		}

		//////////////////////////////////////////////////////////////////////

		void ClearBackBuffer(Color color)
		{
			Graphics::ClearBackBuffer(color);
		}

		//////////////////////////////////////////////////////////////////////

		void Present()
		{
			Graphics::Present();
		}

		//////////////////////////////////////////////////////////////////////

		void CenterRectInMonitor(RECT &rc, HMONITOR hMonitor)
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

		void Center()
		{
			RECT rc;
			GetClientRect(mHWND, &rc);
			CenterRectInMonitor(rc, MonitorFromWindow(mHWND, MONITOR_DEFAULTTOPRIMARY));
			AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, true);
			SetWindowPos(mHWND, null, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOSIZE|SWP_NOZORDER);
		}

		//////////////////////////////////////////////////////////////////////

		void CentreRectInMonitorWithMouseInIt(RECT &rc)
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

		void InitWindow(int width, int height)
		{
			mHINST = GetModuleHandle(null);

			WNDCLASSEX wcex;
			wcex.cbSize = sizeof(WNDCLASSEX);
			wcex.style = CS_HREDRAW | CS_VREDRAW;
			wcex.lpfnWndProc = WndProc;
			wcex.cbClsExtra = 0;
			wcex.cbWndExtra = sizeof(WindowImpl *);
			wcex.hInstance = 0;
			wcex.hIcon = null;
			wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
			wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
			wcex.lpszMenuName = MAKEINTRESOURCE(IDC_PREVIEW);
			wcex.lpszClassName = L"WindowClass";
			wcex.hIconSm = null;
			if(!RegisterClassEx(&wcex))
			{
				return;
			}

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
				return;
			}

			SetWindowLongPtr(mHWND, GWLP_USERDATA, (LONG_PTR)this);

			GetClientRect(mHWND, &rc);
		}
	
		//////////////////////////////////////////////////////////////////////

		void ChangeSize(int newWidth, int newHeight)
		{
			mWidth = newWidth;
			mHeight = newHeight;
			RECT rc = { 0, 0, mWidth, mHeight };
			AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, true);
			SetWindowPos(mHWND, null, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE|SWP_NOZORDER);
		}

		//////////////////////////////////////////////////////////////////////

		bool Update()
		{
			MSG msg;

			while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			return msg.message != WM_QUIT;
		}

		//////////////////////////////////////////////////////////////////////

		static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
		{
			switch(message)
			{
			case WM_NCCREATE:
				SetWindowLongPtr(hWnd, 0, (LONG_PTR)(((CREATESTRUCT *)lParam)->lpCreateParams));

			case WM_GETMINMAXINFO:
				return DefWindowProc(hWnd, message, wParam, lParam);

			default:
				{
					WindowImpl *ths = (WindowImpl *)GetWindowLongPtr(hWnd, 0);
					if(WindowImpl::IsWindowImpl(ths))
					{
						return ths->HandleMessage(hWnd, message, wParam, lParam);
					}
					else
					{
						return DefWindowProc(hWnd, message, wParam, lParam);
					}
				}
			}
		}

		//////////////////////////////////////////////////////////////////////

		LRESULT HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
		{
			PAINTSTRUCT ps;
			HDC hdc;

			switch(message)
			{
				case WM_PAINT:
					hdc = BeginPaint(hWnd, &ps);
					EndPaint(hWnd, &ps);
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
						SetCapture(hWnd);
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
					return DefWindowProc(hWnd, message, wParam, lParam);
			}

			return 0;
		}

		//////////////////////////////////////////////////////////////////////

		void DoResize()
		{
			RECT rc;
			GetClientRect(mHWND, &rc);
			if(rc.right != mWidth || rc.bottom != mHeight)
			{
				mWidth = rc.right;
				mHeight = rc.bottom;

				Graphics::Resize(mWidth, mHeight);

				if(OnResize != null)
				{
					OnResize();
				}
			}
		}

		//////////////////////////////////////////////////////////////////////

		void Init(int width, int height)
		{
			InitWindow(width, height);
			Graphics::Init(mHWND);
		}

		//////////////////////////////////////////////////////////////////////

		~WindowImpl()
		{
			Release();
			g_WindowImpl = null;
		}

		//////////////////////////////////////////////////////////////////////

		void Release()
		{
			Graphics::Release();
		}
	};

	WindowImpl *WindowImpl::g_WindowImpl = null;
	WindowImpl g_Window;
}

//////////////////////////////////////////////////////////////////////

namespace Window
{
	std::function<void ()> OnResize;

	void Release()
	{
		g_Window.Release();
	}

	//////////////////////////////////////////////////////////////////////

	int Width()
	{
		return g_Window.mWidth;
	}

	//////////////////////////////////////////////////////////////////////

	int Height()
	{
		return g_Window.mHeight;
	}

	//////////////////////////////////////////////////////////////////////

	// Show, Hide, Maximize, Minimize, Close

	void Show()
	{
		ShowWindow(WindowImpl::g_WindowImpl->mHWND, SW_SHOW);
	}

	//////////////////////////////////////////////////////////////////////

	void Hide()
	{
		ShowWindow(WindowImpl::g_WindowImpl->mHWND, SW_HIDE);
	}

	//////////////////////////////////////////////////////////////////////

	void Recenter()
	{
		WindowImpl::g_WindowImpl->Center();
	}

	//////////////////////////////////////////////////////////////////////

	void EnableScissoring(bool enable)
	{
		g_Window.EnableScissoring(enable);
	}

	//////////////////////////////////////////////////////////////////////

	void SetScissorRectangle(Rect2D const &rect)
	{
		g_Window.SetScissorRectangle(rect);
	}

	//////////////////////////////////////////////////////////////////////

	Size2D GetSize()
	{
		return Size2D(Width(), Height());
	}

	//////////////////////////////////////////////////////////////////////

	void Init(int width, int height)
	{
		g_Window.Init(width, height);
		g_Window.OnResize = []
		{
			if(OnResize != null)
			{
				OnResize();
			}
		};
	}

	//////////////////////////////////////////////////////////////////////

	void ChangeSize(int newWidth, int newHeight)
	{
		g_Window.ChangeSize(newWidth, newHeight);
	}

	//////////////////////////////////////////////////////////////////////

	bool Update()
	{
		return g_Window.Update();
	}

	//////////////////////////////////////////////////////////////////////

	void Clear(Color color)
	{
		g_Window.ClearBackBuffer(color);
	}

	//////////////////////////////////////////////////////////////////////

	void Present()
	{
		g_Window.Present();
	}
}
