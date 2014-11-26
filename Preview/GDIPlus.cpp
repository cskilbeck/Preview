#include "stdafx.h"

#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;

class GDIPlus
{
public:
	
	GDIPlus()
	{
		GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	}

	~GDIPlus()
	{
		GdiplusShutdown(gdiplusToken);
	}

private:

	static GDIPlus gdiplusInitializer;

	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
};

GDIPlus GDIPlus::gdiplusInitializer;