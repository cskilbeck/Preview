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

	HRESULT Close()
	{
		DX(movie.Close());
		texture.reset();
		return S_OK;
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
		if(currentFrame < frameToPlay)
		{
			Movie::Frame *frame = movie.GetFrame(frameToPlay);
			if(frame != null)
			{
				texture->Update(context, (byte *)frame->mem.get());
				currentFrame = frame->frame;
				movie.ReleaseFrame(frame);
			}
		}
	}

	void TogglePause()
	{
		if(movie.IsPaused())
		{
			movie.Play();
		}
		else
		{
			movie.Pause();
		}
	}

	//////////////////////////////////////////////////////////////////////

	int CurrentFrame() const
	{
		return currentFrame;
	}
};

//////////////////////////////////////////////////////////////////////

#define VertexDef(def_vert, element, end_def)								\
	def_vert(Vertex)														\
		element(Float, 2, mPosition, POSITION, 0, DXGI_FORMAT_R32G32_FLOAT)	\
		element(Float, 2, mTexCoord, TEXCOORD, 0, DXGI_FORMAT_R32G32_FLOAT)	\
	end_def()

vert_GenerateStruct(VertexDef)
vert_GenerateFields(VertexDef)

#define ColorVertexDef(def_vert, element, end_def)							\
	def_vert(ColorVertex)													\
		element(Float, 2, mPos,   POSITION, 0, DXGI_FORMAT_R32G32_FLOAT)	\
		element(Byte,  4, mColor, COLOR,    0, DXGI_FORMAT_R8G8B8A8_UNORM)	\
	end_def()

vert_GenerateStruct(ColorVertexDef)
vert_GenerateFields(ColorVertexDef)

//////////////////////////////////////////////////////////////////////

struct AVIPlayer: DXWindow
{
	AVIPlayer(int width = 640, int height = 480);
	~AVIPlayer();

	bool OnCreate() override;
	bool OnUpdate() override;
	void OnDraw() override;
	void OnResize() override;
	void OnChar(int key, uintptr flags) override;
	void OnDestroy() override;

	//////////////////////////////////////////////////////////////////////

	void CalcDrawRect();
	void SetQuad(bool upsideDown);

	//////////////////////////////////////////////////////////////////////

	HMENU mMenu;
	RectF mDrawRect;
	Timer mTimer;
	double mDeltaTime;
	MyPlayer movie1;
	MyPlayer movie2;
	int frameToPlay;
	int frameDropped;
	Material colorMaterial;
	Material alphaMaterial;
	VertexBuffer colorVertexBuffer;
	VertexBuffer alphaVertexBuffer;

	//////////////////////////////////////////////////////////////////////

};
