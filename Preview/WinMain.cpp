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

	Preview w(640, 480);

	w.Show();

	while(w.Update())
	{
		w.OnDraw();
		w.Present();
	}

	return 0;
}
