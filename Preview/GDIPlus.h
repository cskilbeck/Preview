#pragma once

using Graphics = Gdiplus::Graphics;
using Image = Gdiplus::Image;
using Bitmap = Gdiplus::Bitmap;
using SolidBrush = Gdiplus::SolidBrush;
using Status = Gdiplus::Status;

class GDI
{
public:
	
private:

	GDI();
	~GDI();

	static GDI initializer;

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
};

static inline ptr<Graphics> Gfx(HDC dc)
{
	return ptr<Graphics>(Graphics::FromHDC(dc));
}

static inline ptr<Graphics> Gfx(ptr<Image> &image)
{
	return ptr<Graphics>(Graphics::FromImage(image.get()));
}

