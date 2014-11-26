//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Window
{
	Window(int clientWidth, int clientHeight, TCHAR *caption = null);
	virtual ~Window();

	virtual bool OnUpdate() { return false; }
	virtual void OnClosing() {}
	virtual void OnResize() {}
	virtual void OnPaint(HDC dc, PAINTSTRUCT &ps) {}
	virtual int OnPaintBackground(HDC dc) { return 0; }
	virtual void OnMouseMove(Point2D pos) {}
	virtual void OnLeftButtonDown(Point2D pos) {}
	virtual void OnLeftButtonUp(Point2D pos) {}
	virtual void OnRightButtonDown(Point2D pos) {}
	virtual void OnRightButtonUp(Point2D pos) {}

	bool Update(bool waitForMessages = true);

	void Show();
	void Hide();
	void Center();
	int Width() const;
	int Height() const;
	Size2D GetSize();
	void SetPosition(int x, int y, int width, int height);
	void Resize(int clientWidth, int clientHeight);
	void Move(int x, int y);

protected:

	friend LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void Create(int w, int h, TCHAR *caption);
	void Sized();

	int						mWidth;
	int						mHeight;
	HWND					mHWND;
	HINSTANCE				mHINST;
};
