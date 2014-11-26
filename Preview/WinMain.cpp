//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Win32.h"
#include "D3D.h"

//////////////////////////////////////////////////////////////////////

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
	ptr<Preview> preview(new Preview());
	preview->Show();

	while(preview->Update())
	{
	}

	return 0;
}
