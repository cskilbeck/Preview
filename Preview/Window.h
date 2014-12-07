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
	virtual void OnMouseMove(Point pos, uintptr flags);
	virtual void OnMouseWheel(Point pos, int delta, uintptr flags);
	virtual void OnLeftButtonDown(Point pos, uintptr flags);
	virtual void OnLeftButtonUp(Point pos, uintptr flags);
	virtual void OnRightButtonDown(Point pos, uintptr flags);
	virtual void OnRightButtonUp(Point pos, uintptr flags);
	virtual void OnChar(int key, uintptr flags);
	virtual void OnKeyDown(int key, uintptr flags);
	virtual void OnKeyUp(int key, uintptr flags);

	void Show();
	void Hide();
	void Close();
	void Center();
	bool Update();
	int Width() const;
	int Height() const;
	float FWidth() const;
	float FHeight() const;
	Vec2 FSize() const;
	Rect ClientRect() const;
	bool SetMessageWait(bool wait);
	bool GetMessageWait() const;
	Size GetSize();
	void ChangeSize(int newWidth, int newHeight);

protected:

	void DoResize();
	bool Init(int width, int height);
	LRESULT HandleMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	friend LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

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

inline Rect Window::ClientRect() const
{
	Rect r;
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
