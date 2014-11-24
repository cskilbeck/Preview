//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Win32.h"

//////////////////////////////////////////////////////////////////////

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
	Window::Init(480, 320);

	Preview preview;
	preview.Init();

	Window::Show();

	while(Window::Update())
	{
		preview.Update();
		preview.Draw();

		Window::Present();
	}

	preview.Release();
	return 0;
}
