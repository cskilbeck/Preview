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

	bool IsValid() const
	{
		return mTexture2D != null;
	}

	tstring const &GetName() const
	{
		return mName;
	}

	void Activate();

private:

	Size2D							mSize;
	tstring							mName;
	DXPtr<ID3D11Device>				mDevice;
	DXPtr<ID3D11DeviceContext>		mContext;
	DXPtr<ID3D11Texture2D>			mTexture2D;
	DXPtr<ID3D11ShaderResourceView>	mShaderResourceView;
	D3D11_TEXTURE2D_DESC			mTextureDesc;
};

//////////////////////////////////////////////////////////////////////
