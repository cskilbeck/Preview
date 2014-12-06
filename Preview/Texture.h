//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Window;

struct Texture
{
	Texture();
	Texture(tchar const *name, Window *parent);
	Texture(int w, int h, Color color, Window *parent);
	Texture(int w, int h, byte *pixels, Window *parent);
	~Texture();

	int Width() const
	{
		return mTextureDesc.Width;
	}

	int Height() const
	{
		return mTextureDesc.Height;
	}

	float FWidth() const
	{
		return (float)mTextureDesc.Width;
	}

	float FHeight() const
	{
		return (float)mTextureDesc.Height;
	}

	Vec2 FSize() const
	{
		return Vec2((float)Width(), (float)Height());
	}

	bool IsValid() const
	{
		return mTexture2D != null;
	}

	tstring const &GetName() const
	{
		return mName;
	}

	void Activate(int channel = 0);

private:

	Size							mSize;
	tstring							mName;
	DXPtr<ID3D11Device>				mDevice;
	DXPtr<ID3D11DeviceContext>		mContext;
	DXPtr<ID3D11Texture2D>			mTexture2D;
	DXPtr<ID3D11ShaderResourceView>	mShaderResourceView;
	D3D11_TEXTURE2D_DESC			mTextureDesc;
};

//////////////////////////////////////////////////////////////////////
