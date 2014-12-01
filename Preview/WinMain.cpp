//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Win32.h"

//////////////////////////////////////////////////////////////////////

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
<<<<<<< HEAD
	ShowCurrentFolder();

	Preview w(640, 480);
=======
	Window::Init(480, 320);
>>>>>>> origin/master

	w.Show();

	while(w.Update())
	{
		w.OnDraw();
		w.Present();
	}

	return 0;
}
