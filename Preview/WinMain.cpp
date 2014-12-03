//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Win32.h"

//////////////////////////////////////////////////////////////////////

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
	Preview w(640, 480);

	RECT rc;
	GetClientRect(w.mHWND, &rc);
	TRACE("%d,%d,%d,%d", rc.left, rc.top, rc.right, rc.bottom);

	while(w.Update())
	{
		w.OnDraw();
		w.Present();
	}

	return 0;
}
