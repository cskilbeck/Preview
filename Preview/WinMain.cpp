//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Win32.h"
#include "D3D.h"

//////////////////////////////////////////////////////////////////////

void ShowCurrentFolder()
{
	WCHAR buffer[16384];
	buffer[0] = 0;
	GetCurrentDirectory(ARRAYSIZE(buffer), buffer);
	OutputDebugString(L"Current folder:");
	OutputDebugString(buffer);
	OutputDebugString(L"\n");
}

//////////////////////////////////////////////////////////////////////

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
	ShowCurrentFolder();

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
