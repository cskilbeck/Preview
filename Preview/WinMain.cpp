//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
	AVIPlayer preview(640, 480);

	while(preview.Update())
	{
	}
	return 0;
}
