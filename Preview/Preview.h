//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Preview : DXWindow
{
	Preview(int width = 640, int height = 480);
	~Preview();

	bool OnCreate() override;
	bool OnUpdate() override;
	void OnDraw() override;
	void OnResize() override;
	void OnChar(int key, uintptr flags) override;
	void OnMouseWheel(Point pos, int delta, uintptr flags) override;
	void OnRightButtonDown(Point pos, uintptr flags) override;
	void OnRightButtonUp(Point pos, uintptr flags) override;
	void OnMouseMove(Point pos, uintptr flags) override;
	void OnDestroy() override;

	HRESULT LoadShaders();
	HRESULT CreateSampler();
	HRESULT CreateVertexBuffer();
	HRESULT CreateRasterizerState();
	HRESULT CreateBlendState();
	HRESULT CreateDepthStencilState();
	HRESULT CreateVertexShaderConstants();
	HRESULT CreatePixelShaderConstants();

	HMENU mMenu;
	Ptr<Texture> mTexture;
	Color mBackgroundColor;
	HCURSOR mHandCursor;
	Vec2 mTranslation;
	Vec2 mOffset;
	float mScale;
	float mCurrentScale;
	Vec2 mScaleOrg;
	Timer mTimer;
	double mDeltaTime;
	double mLastZoomTime;
	bool mDrag;
	Point mDragPos;

	DXPtr<ID3D11InputLayout>		vertexLayout;
	DXPtr<ID3D11PixelShader>		pixelShader;
	DXPtr<ID3D11VertexShader>		vertexShader;
	DXPtr<ID3D11Buffer>				vertexBuffer;
	DXPtr<ID3D11RasterizerState>	rasterizerState;
	DXPtr<ID3D11BlendState>			blendState;
	DXPtr<ID3D11SamplerState>		sampler;
	DXPtr<ID3D11Buffer>				vertexShaderConstants;
	DXPtr<ID3D11Buffer>				pixelShaderConstants;
	DXPtr<ID3D11DepthStencilState>	mDepthStencilState;

};
