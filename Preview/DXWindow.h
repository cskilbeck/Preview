//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct DXWindow: Window
{
	DXWindow(int width, int height, tchar const *caption = TEXT("DXWindow"));
	virtual ~DXWindow();

protected:

	virtual void OnDraw();

	void OnPaint(PAINTSTRUCT &ps) override;
	bool OnUpdate() override;
	bool OnCreate() override;
	void OnResize() override;
	void OnDestroy() override;

	bool OpenD3D();
	bool ResizeD3D();
	void CloseD3D();
	bool GetBackBuffer();
	void ReleaseBackBuffer();
	void Clear(Color color);
	void Present();

	HWND							mDXWindow;

	int								mFrame;

	DXPtr<ID3D11DeviceContext>		mContext;
	DXPtr<ID3D11RenderTargetView>	mRenderTargetView;
	DXPtr<IDXGISwapChain>			mSwapChain;

	D3D_DRIVER_TYPE					mDriverType;
	D3D_FEATURE_LEVEL				mFeatureLevel;
};

