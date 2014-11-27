#include "stdafx.h"

#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;

GDI::GDI()
{
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}

GDI::~GDI()
{
	GdiplusShutdown(gdiplusToken);
}

GDI GDI::initializer;