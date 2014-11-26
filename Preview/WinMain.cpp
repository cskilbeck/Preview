//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Win32.h"
#include "D3D.h"

#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;

//////////////////////////////////////////////////////////////////////

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	Preview preview;
	preview.Show();

	while(preview.Update())
	{
	}

	preview.~Preview();

	GdiplusShutdown(gdiplusToken);
	return 0;
}
