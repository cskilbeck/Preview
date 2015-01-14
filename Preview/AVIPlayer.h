//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct MyPlayer
{
	//////////////////////////////////////////////////////////////////////

	Movie::Player movie;
	Ptr<Texture> texture;
	int currentFrame;
	int width;
	int height;

	//////////////////////////////////////////////////////////////////////

	MyPlayer()
		: currentFrame(-1)
	{
	}

	//////////////////////////////////////////////////////////////////////

	~MyPlayer()
	{
		texture.reset();
	}

	//////////////////////////////////////////////////////////////////////

	HRESULT Open(wchar const *filename)
	{
		DX(movie.Open(filename, 4));
		texture.reset(new Texture(movie.Width(), movie.Height(), Color::Black));
		width = movie.Width();
		height = movie.Height();
		movie.Play();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////

	void Update(ID3D11DeviceContext *context, int frameToPlay)
	{
		if(currentFrame != frameToPlay)
		{
			Movie::Frame *frame = movie.GetFrame(frameToPlay);
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

	__declspec (align(4)) struct ColorVertex
	{
		Vec2 mPos;
		Color mColor;
	};

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

	Vec2 MovieBorder() const;
	Vec2 DXSize() const;
	void CalcDrawRect();
	void SetQuad(bool upsideDown);

	//////////////////////////////////////////////////////////////////////

	HMENU mMenu;
	Color mBackgroundColor;
	HCURSOR mHandCursor;
	RectF mDrawRect;
	float mScale;
	Timer mTimer;
	double mDeltaTime;
	Vertex vert[6];

	MyPlayer movie1;
	MyPlayer movie2;

	int frameToPlay;
	bool frameDropped;

	PixelShader						colorPixelShader;
	VertexShader					colorVertexShader;
	VertexBuffer<ColorVertex>		colorVertexBuffer;

	DXPtr<ID3D11InputLayout>		vertexLayout;
	DXPtr<ID3D11PixelShader>		pixelShader;
	DXPtr<ID3D11VertexShader>		vertexShader;
	DXPtr<ID3D11Buffer>				vertexBuffer;
	DXPtr<ID3D11Buffer>				vertexShaderConstants;
	DXPtr<ID3D11Buffer>				pixelShaderConstants;

	DXPtr<ID3D11InputLayout>		colorVertexLayout;

	DXPtr<ID3D11RasterizerState>	rasterizerState;
	DXPtr<ID3D11BlendState>			blendState;
	DXPtr<ID3D11SamplerState>		sampler;
	DXPtr<ID3D11DepthStencilState>	mDepthStencilState;

	//////////////////////////////////////////////////////////////////////

};
