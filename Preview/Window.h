//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Window
{
	Window(int width = 640, int height = 480);
	~Window();

	virtual bool OnUpdate();
	virtual void OnDraw();
	virtual void OnClosing();
	virtual void OnResize();
	virtual void OnPaint(HDC dc, PAINTSTRUCT &ps);
	virtual LRESULT OnPaintBackground(HDC dc);
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
	void SetScissorRectangle(Rect2D const &rect);

protected:

	void DoResize();
	bool Init(int width, int height);
	bool InitD3D();
	bool GetBackBuffer();
	void ReleaseBackBuffer();
	LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);

	friend LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	friend struct Texture;

	int								mWidth;
	int								mHeight;
	HWND							mHWND;
	HINSTANCE						mHINST;
	bool							mInResizingLoop;
	bool							mMouseDown;

	DXPtr<ID3D11Device>				mDevice;
	DXPtr<ID3D11DeviceContext>		mContext;
	DXPtr<ID3D11RenderTargetView>	mRenderTargetView;
	DXPtr<IDXGISwapChain>			mSwapChain;

	D3D_DRIVER_TYPE					mDriverType;
	D3D_FEATURE_LEVEL				mFeatureLevel;
};
