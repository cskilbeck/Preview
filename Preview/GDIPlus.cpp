#include "stdafx.h"

#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;

GDIPlus::GDIPlus()
{
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}

GDIPlus::~GDIPlus()
{
	GdiplusShutdown(gdiplusToken);
}

GDIPlus GDIPlus::gdiplusInitializer;