//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

namespace Window
{
	enum Orientation
	{
		Portrait,
		Landscape
	};

	extern function<void ()> OnResize;

	void Init(int width, int height);
	void Show();
	void Hide();
	void Recenter();
	bool Update();
	void Clear(Color color);
	void Present();

	Orientation CurrentOrientation();

	int Width();
	int Height();
	Size2D GetSize();
	void ChangeSize(int newWidth, int newHeight);

	void Release();

	void EnableScissoring(bool enable);
	void SetScissorRectangle(Rect2D const &rect);
};
