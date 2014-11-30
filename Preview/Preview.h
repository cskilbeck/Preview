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
	void OnChar(int charCode, uint32 flags) override;

	void CreateBackbuffer();
	void RenderBitmap();
	void DrawGrid();

	ptr<Image> bitmap;
	ptr<Image> backbuffer;
	ptr<SolidBrush> gridBrush[2];
	ptr<SolidBrush> backBrush;
	int gridSize;
	Rect drawRect;
};
