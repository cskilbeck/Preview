//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Window;

struct Texture
{
	static DXPtr<ID3D11Device> sDevice;
	static DXPtr<ID3D11DeviceContext> sContext;

	static void SetDeviceAndContext(DXPtr<ID3D11Device> device, DXPtr<ID3D11DeviceContext> context);
	static void ReleaseDeviceAndContext();

	Texture();
	Texture(tchar const *name);
	Texture(int w, int h, Color color);
	Texture(int w, int h, byte *pixels);
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

	tstring							mName;
	DXPtr<ID3D11Texture2D>			mTexture2D;
	DXPtr<ID3D11ShaderResourceView>	mShaderResourceView;
	D3D11_TEXTURE2D_DESC			mTextureDesc;
};

//////////////////////////////////////////////////////////////////////
