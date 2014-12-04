//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Win32.h"

//////////////////////////////////////////////////////////////////////

Preview w(640, 480);

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
	int frames = 0;
	while(w.Update())
	{
		w.OnDraw();
		w.Present();
		if(frames == 0)
		{
			w.Show();
		}
		++frames;
	}
	return 0;
}
