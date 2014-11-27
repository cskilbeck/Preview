#pragma once

using Graphics = Gdiplus::Graphics;
using Image = Gdiplus::Image;
using Bitmap = Gdiplus::Bitmap;
using SolidBrush = Gdiplus::SolidBrush;
using Status = Gdiplus::Status;

class GDIPlus
{
public:
	
	static inline ptr<Graphics> GraphicsFromImage(ptr<Image> &image)
	{
		return ptr<Graphics>(Graphics::FromImage(image.get()));
	}

	static inline ptr<Graphics> GraphicsFromHDC(HDC dc)
	{
		return ptr<Graphics>(Graphics::FromHDC(dc));
	}

private:

	GDIPlus();
	~GDIPlus();

	static GDIPlus gdiplusInitializer;

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
};
