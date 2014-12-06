//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Window
{
	Window(int width = 640, int height = 480, tchar const *caption = TEXT("Window"));
	~Window();

	virtual bool OnUpdate();
	virtual void OnDraw();
	virtual void OnClosing();
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

	virtual void Release();

	void Show();
	void Hide();
	void Close();
	void Center();
	bool Update();
	void Clear(Color color);
	void Present();

	int Width() const
	{
		return mWidth;
	}

	int Height() const
	{
		return mHeight;
	}

	float FWidth() const
	{
		return (float)mWidth;
	}

	float FHeight() const
	{
		return (float)mHeight;
	}

	Vec2 FSize() const
	{
		return Vec2(FWidth(), FHeight());
	}

	Size GetSize();
	void ChangeSize(int newWidth, int newHeight);

	void EnableScissoring(bool enable);
	void SetScissorRectangle(Rect const &rect);

	bool SetMessageWait(bool wait)
	{
		mMessageWait = wait;
	}

//protected:

	void DoResize();
	bool Init(int width, int height);
	bool OpenD3D();
	void ResizeD3D();
	void CloseD3D();
	bool GetBackBuffer();
	void ReleaseBackBuffer();
	LRESULT HandleMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	friend LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	friend struct Texture;

	int								mFrame;
	int								mWidth;
	int								mHeight;
	HWND							mHWND;
	HINSTANCE						mHINST;
	HMENU							mMenu;
	bool							mLeftMouseDown;
	bool							mRightMouseDown;
	bool							mMessageWait;
	tstring							mCaption;

	DXPtr<ID3D11Device>				mDevice;
	DXPtr<ID3D11DeviceContext>		mContext;
	DXPtr<ID3D11RenderTargetView>	mRenderTargetView;
	DXPtr<IDXGISwapChain>			mSwapChain;

	D3D_DRIVER_TYPE					mDriverType;
	D3D_FEATURE_LEVEL				mFeatureLevel;
};
