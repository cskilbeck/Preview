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

	Gdiplus::Bitmap *bitmap;
	Gdiplus::Bitmap *backbuffer;

	Gdiplus::SolidBrush *gridBrush[2];
	int gridSize;

	void CreateBackbuffer();
	void RenderBitmap();
	void DrawGrid();
};
