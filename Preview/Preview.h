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
	void OnChar(int charCode, uint32 flags) override;

	ptr<Image> bitmap;
	ptr<Image> backbuffer;
	ptr<SolidBrush> gridBrush[2];
	ptr<SolidBrush> backBrush;
	int gridSize;

	void CreateBackbuffer();
	void RenderBitmap();
	void DrawGrid();
};
