//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <ShellApi.h>

//////////////////////////////////////////////////////////////////////

Preview::Preview()
	: Window(100, 100)
	, bitmap(null)
{
	backBrush = ptr<SolidBrush>(new SolidBrush(Gdiplus::Color(255, 255, 0, 255)));
	gridBrush[0] = ptr<SolidBrush>(new SolidBrush(Gdiplus::Color(255, 128, 128, 128)));
	gridBrush[1] = ptr<SolidBrush>(new SolidBrush(Gdiplus::Color(255, 192, 192, 192)));
	gridSize = 32;

	int argc;
	LPWSTR *argv = CommandLineToArgvW(GetCommandLine(), &argc);
	if(argc > 1)
	{
		wstring s(argv[1]);
		LoadGDIPlusBitmap(s.c_str(), 0, bitmap);
		if(bitmap != null)
		{
			Resize(bitmap->GetWidth(), bitmap->GetHeight());
			Center();
			CreateBackbuffer();
			DrawGrid();
			RenderBitmap();
		}
		else
		{
			Center();
			CreateBackbuffer();
			DrawGrid();
		}
	}
	LocalFree(argv);
}

//////////////////////////////////////////////////////////////////////
// create backbuffer same size as bitmap 

void Preview::CreateBackbuffer()
{
	// if we already have one, and it's big enough, keep it
	if(backbuffer == null || backbuffer->GetWidth() < Width() || backbuffer->GetHeight() < Height())
	{
		backbuffer.reset(new Bitmap(Width(), Height(), PixelFormat32bppARGB));
	}
	Gfx(backbuffer)->FillRectangle(backBrush.get(), 0, 0, Width(), Height());
}

//////////////////////////////////////////////////////////////////////
// overlay image onto backbuffer grid

void Preview::RenderBitmap()
{
	auto gdi(Gfx(backbuffer));
	float x = ((float)Width() - bitmap->GetWidth()) / 2;
	float y = ((float)Height() - bitmap->GetHeight()) / 2;
	gdi->DrawImage(bitmap.get(), x, y, (float)bitmap->GetWidth(), (float)bitmap->GetHeight());
}

//////////////////////////////////////////////////////////////////////
// draw grid into backbuffer

void Preview::DrawGrid()
{
	auto gdi = Gfx(backbuffer);
	int b = 0;
	float w = (float)bitmap->GetWidth();
	float h = (float)bitmap->GetHeight();
	float xo = (Width() - w) / 2;
	float yo = (Height() - h) / 2;
	float right = xo + w;
	float bottom = yo + h;
	for(float y = 0; y < w; y += gridSize)
	{
		int c = b;
		b = 1 - b;
		for(float x = 0; x < h; x += gridSize)
		{
			float xp = x + xo;
			float yp = y + yo;
			float r1 = Min(xp + gridSize, right);
			float b1 = Min(yp + gridSize, bottom);
			float w1 = r1 - xp;
			float h1 = b1 - yp;
			gdi->FillRectangle(gridBrush[c].get(), xp, yp, w1, h1);
			c = 1-c;
		}
	}
}

//////////////////////////////////////////////////////////////////////

void Preview::OnClosing()
{
	bitmap.reset();
}

//////////////////////////////////////////////////////////////////////

void Preview::OnPaint(HDC dc, PAINTSTRUCT &ps)
{
}

//////////////////////////////////////////////////////////////////////

Preview::~Preview()
{
}

//////////////////////////////////////////////////////////////////////

void Preview::OnResize()
{
	CreateBackbuffer();
	DrawGrid();
	RenderBitmap();
}

//////////////////////////////////////////////////////////////////////

void Preview::OnChar(int charCode, uint32 flags)
{
	switch(charCode)
	{
		case VK_ESCAPE:
			Close();
			break;
	}
}

//////////////////////////////////////////////////////////////////////

int Preview::OnPaintBackground(HDC dc)
{
	auto gdi = Gfx(dc);
	gdi->DrawImage(backbuffer.get(), 0, 0, backbuffer->GetWidth(), backbuffer->GetHeight());
	return 1;
}


