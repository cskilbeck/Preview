//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Player
{
	//////////////////////////////////////////////////////////////////////

	Movie::Player movie;
	Ptr<Texture> texture;
	int currentFrame;

	//////////////////////////////////////////////////////////////////////

	Player()
		: currentFrame(-1)
	{
	}

	//////////////////////////////////////////////////////////////////////

	~Player()
	{
		texture.reset();
	}

	//////////////////////////////////////////////////////////////////////

	HRESULT Open(wchar const *filename)
	{
		DX(movie.Open(filename, 16));
		texture.reset(new Texture(movie.Width(), movie.Height(), Color::Black));
		movie.Play();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////

	void Update(ID3D11DeviceContext *context, int frameToPlay)
	{
		if(currentFrame != frameToPlay)
		{
			Movie::Player::Frame *frame = movie.GetFrame(frameToPlay);
			if(frame != null)
			{
				texture->Update(context, (byte *)frame->mem);
				currentFrame = frame->frame;
				movie.ReleaseFrame(frame);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////

	int CurrentFrame() const
	{
		return currentFrame;
	}
};

//////////////////////////////////////////////////////////////////////

struct AVIPlayer: DXWindow
{
	//////////////////////////////////////////////////////////////////////

	__declspec (align(4)) struct Vertex
	{
		Vec2 mPos;
		Vec2 mTexCoord;

		void Set(Vec2 const &pos, Vec2 const &texCoord)
		{
			mPos = pos;
			mTexCoord = texCoord;
		}
	};

	//////////////////////////////////////////////////////////////////////

	struct PixelShaderConstants
	{
		DirectX::XMVECTOR channelMask;
		DirectX::XMVECTOR channelOffset;
		DirectX::XMVECTOR gridColor0;
		DirectX::XMVECTOR gridColor1;
		DirectX::XMFLOAT2 gridSize;
		DirectX::XMFLOAT2 gridSize2;
	};

	//////////////////////////////////////////////////////////////////////

	struct VertexShaderConstants
	{
		Matrix matrix;
		DirectX::XMFLOAT2 textureSize;
	};

	//////////////////////////////////////////////////////////////////////

	AVIPlayer(int width = 640, int height = 480);
	~AVIPlayer();

	bool OnCreate() override;
	bool OnUpdate() override;
	void OnDraw() override;
	void OnResize() override;
	void OnChar(int key, uintptr flags) override;
	void OnMouseWheel(Point2D pos, int delta, uintptr flags) override;
	void OnLeftMouseDoubleClick(Point2D pos);
	void OnRightButtonDown(Point2D pos, uintptr flags) override;
	void OnRightButtonUp(Point2D pos, uintptr flags) override;
	void OnMouseMove(Point2D pos, uintptr flags) override;
	void OnDestroy() override;

	HRESULT LoadShaders();
	HRESULT CreateSampler();
	HRESULT CreateVertexBuffer();
	HRESULT CreateRasterizerState();
	HRESULT CreateBlendState();
	HRESULT CreateDepthStencilState();
	HRESULT CreateVertexShaderConstants();
	HRESULT CreatePixelShaderConstants();

	//////////////////////////////////////////////////////////////////////

	void CalcDrawRect();
	void SetQuad(bool upsideDown);
	void CenterImageInWindow();
	void CenterImageInWindowAndResetZoom();

	//////////////////////////////////////////////////////////////////////

	static Vertex vert[6];

	//////////////////////////////////////////////////////////////////////

	HMENU mMenu;
	Color mBackgroundColor;
	HCURSOR mHandCursor;
	Rect2D mOldClientRect;
	RectF mDrawRect;
	RectF mCurrentDrawRect;
	float mScale;
	Timer mTimer;
	double mDeltaTime;
	double mLastZoomTime;
	bool mDrag;
	Point2D mDragPos;
	bool mMaintainImagePosition;
	Material mMaterial;
	PixelShader mPixelShader;
	VertexShader mVertexShader;

	Player movie1;
	Player movie2;

	Ptr<Texture> mTexture1;

	int frameToPlay;
	bool frameDropped;

	//////////////////////////////////////////////////////////////////////

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

	//////////////////////////////////////////////////////////////////////

};
