//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Preview : Window
{
	Preview();
	~Preview();

	void OnClosing() override;

	void OnResize() override;
	void OnPaint(HDC dc, PAINTSTRUCT &ps) override;
	int OnPaintBackground(HDC dc) override;

	ptr<Gdiplus::Bitmap> bitmap;
	ptr<Gdiplus::Bitmap> backbuffer;
	ptr<Gdiplus::SolidBrush> gridBrush[2];
	ptr<Gdiplus::SolidBrush> backBrush;
	int gridSize;

	void CreateBackbuffer();
	void RenderBitmap();
	void DrawGrid();
};
