#pragma once

struct MovieControlWindow : Window
{
	MovieControlWindow(HWND parent)
		: Window(320, 64, null, WS_CHILD|WS_CLIPSIBLINGS|WS_OVERLAPPED, null, parent)
	{
	}

	void OnPaint(PAINTSTRUCT &ps) override
	{
		static HBRUSH red = CreateSolidBrush(RGB(255, 0, 0));
		FillRect(ps.hdc, &ps.rcPaint, red);
	}
};
