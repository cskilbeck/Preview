//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <ShellApi.h>

using namespace Gdiplus;

//////////////////////////////////////////////////////////////////////

Preview::Preview()
	: Window(100, 100)
	, bitmap(null)
{
	int argc;
	LPWSTR *argv = CommandLineToArgvW(GetCommandLine(), &argc);
	if(argc > 0)
	{
		wstring s(argv[1]);
		LoadGDIPlusBitmap(s.c_str(), 0, bitmap);
	}
	LocalFree(argv);
	gridBrush[0] = new SolidBrush(Gdiplus::Color(255, 128, 128, 128));
	gridBrush[1] = new SolidBrush(Gdiplus::Color(255, 192, 192, 192));
	gridSize = 32;
	Resize(bitmap->GetWidth(), bitmap->GetHeight());
	Center();
	CreateBackbuffer();
	DrawGrid();
	RenderBitmap();
}

//////////////////////////////////////////////////////////////////////
// create backbuffer same size as bitmap 

void Preview::CreateBackbuffer()
{
	HDC dc = GetDC(null);
	Graphics *g = Graphics::FromHDC(dc);
	backbuffer = new Bitmap(bitmap->GetWidth(), bitmap->GetHeight(), g);
	delete g;
	ReleaseDC(null, dc);
}

//////////////////////////////////////////////////////////////////////
// overlay image onto backbuffer grid

void Preview::RenderBitmap()
{
	Graphics *g = Graphics::FromImage(backbuffer);
	g->DrawImage(bitmap, 0, 0, bitmap->GetWidth(), bitmap->GetHeight());
	delete g;
}

//////////////////////////////////////////////////////////////////////
// draw grid into backbuffer

void Preview::DrawGrid()
{
	Graphics *g = Graphics::FromImage(backbuffer);
	int b = 0;
	for(uint y=0; y<backbuffer->GetHeight(); y += gridSize)
	{
		int c = b;
		b = 1-b;
		for(uint x=0; x<backbuffer->GetWidth(); x += gridSize)
		{
			g->FillRectangle(gridBrush[c], x, y, gridSize, gridSize);
			c = 1-c;
		}
	}
	delete g;
}

//////////////////////////////////////////////////////////////////////

void Preview::OnClosing()
{
	Delete(bitmap);
}

//////////////////////////////////////////////////////////////////////

void Preview::OnPaint(HDC dc, PAINTSTRUCT &ps)
{
	Graphics *g = Graphics::FromHDC(dc);
	int x = (Width() - backbuffer->GetWidth()) / 2;
	int y = (Height() - backbuffer->GetHeight()) / 2;
	g->DrawImage(backbuffer, x, y, backbuffer->GetWidth(), backbuffer->GetHeight());
	delete g;
}

//////////////////////////////////////////////////////////////////////

void Preview::OnResize()
{
	CreateBackbuffer();
	DrawGrid();
	RenderBitmap();
}

//////////////////////////////////////////////////////////////////////

int Preview::OnPaintBackground(HDC dc)
{
	RECT r;
	GetClientRect(mHWND, &r);
	FillRect(dc, &r, (HBRUSH)GetStockObject(WHITE_BRUSH));
	return 1;
}

//////////////////////////////////////////////////////////////////////

Preview::~Preview()
{
	Delete(gridBrush[0]);
	Delete(gridBrush[1]);
	Delete(backbuffer);
	Delete(bitmap);
}

