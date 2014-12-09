//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Window
{
	Window(int width = 640, int height = 480, tchar const *caption = TEXT("Window"));
	~Window();

	virtual bool OnCreate();
	virtual bool OnUpdate();
	virtual void OnPaint(PAINTSTRUCT &ps);
	virtual void OnDestroy();
	virtual void OnResize();
	virtual void OnMouseMove(Point2D pos, uintptr flags);
	virtual void OnMouseWheel(Point2D pos, int delta, uintptr flags);
	virtual void OnLeftMouseDoubleClick(Point2D pos);
	virtual void OnRightMouseDoubleClick(Point2D pos);
	virtual void OnLeftButtonDown(Point2D pos, uintptr flags);
	virtual void OnLeftButtonUp(Point2D pos, uintptr flags);
	virtual void OnRightButtonDown(Point2D pos, uintptr flags);
	virtual void OnRightButtonUp(Point2D pos, uintptr flags);
	virtual void OnChar(int key, uintptr flags);
	virtual void OnKeyDown(int key, uintptr flags);
	virtual void OnKeyUp(int key, uintptr flags);

	void Show();
	void Hide();
	void Close();
	void MoveToMiddleOfMonitor();
	bool Update();
	int Width() const;
	int Height() const;
	float FWidth() const;
	float FHeight() const;
	Vec2 FSize() const;
	Rect2D ClientRect() const;
	bool SetMessageWait(bool wait);
	bool GetMessageWait() const;
	void SetWindowRect(Rect2D const &r);
	Size2D GetSize();
	void ChangeSize(int newWidth, int newHeight);
	void ResizeWindow(int newWidth, int newHeight);
	Rect2D GetWindowRectFromClientRect(Rect2D const &clientRect);
	Rect2D GetClientRectFromWindowRect(Rect2D const &windowRect);

protected:

	void DoResize();
	bool Init(int width, int height);
	long GetStyle() const;
	long GetExStyle() const;
	bool HasMenu() const;
	bool IsResizable() const;
	Rect2D GetBorders() const;
	LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	friend LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	int								mWidth;
	int								mHeight;
	HWND							mHWND;
	HINSTANCE						mHINST;
	bool							mLeftMouseDown;
	bool							mRightMouseDown;
	bool							mMessageWait;
	tstring							mCaption;

};

//////////////////////////////////////////////////////////////////////

inline int Window::Width() const
{
	return mWidth;
}

//////////////////////////////////////////////////////////////////////

inline int Window::Height() const
{
	return mHeight;
}

//////////////////////////////////////////////////////////////////////

inline float Window::FWidth() const
{
	return (float)mWidth;
}

//////////////////////////////////////////////////////////////////////

inline float Window::FHeight() const
{
	return (float)mHeight;
}

//////////////////////////////////////////////////////////////////////

inline Vec2 Window::FSize() const
{
	return Vec2(FWidth(), FHeight());
}

//////////////////////////////////////////////////////////////////////

inline Rect2D Window::ClientRect() const
{
	Rect2D r;
	GetClientRect(mHWND, &r);
	return r;
}

//////////////////////////////////////////////////////////////////////

inline bool Window::SetMessageWait(bool wait)
{
	mMessageWait = wait;
}

//////////////////////////////////////////////////////////////////////

inline bool Window::GetMessageWait() const
{
	return mMessageWait;
}
