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
	virtual void OnMouseMove(Point2D pos);
	virtual void OnLeftButtonDown(Point2D pos);
	virtual void OnLeftButtonUp(Point2D pos);
	virtual void OnRightButtonDown(Point2D pos);
	virtual void OnRightButtonUp(Point2D pos);
	virtual void OnChar(int key, uint32 flags);
	virtual void OnMouseWheel(int delta);

	virtual void Release();

	void Show();
	void Hide();
	void Center();
	bool Update();
	void Clear(Color color);
	void Present();

	int Width()
	{
		return mWidth;
	}

	int Height()
	{
		return mHeight;
	}

	Size2D GetSize();
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

	int								mWidth;
	int								mHeight;
	HWND							mHWND;
	HINSTANCE						mHINST;
	HMENU							mMenu;
	bool							mInResizingLoop;
	bool							mMouseDown;
	bool							mMessageWait;
	tstring							mCaption;

	DXPtr<ID3D11Device>				mDevice;
	DXPtr<ID3D11DeviceContext>		mContext;
	DXPtr<ID3D11RenderTargetView>	mRenderTargetView;
	DXPtr<IDXGISwapChain>			mSwapChain;

	D3D_DRIVER_TYPE					mDriverType;
	D3D_FEATURE_LEVEL				mFeatureLevel;
};
