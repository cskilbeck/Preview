//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Win32.h"
#include "ImageLoader.h"

//////////////////////////////////////////////////////////////////////

struct Texture::TextureImpl
{
	Texture::TextureImpl(char const *name)
		: mTexture2D(null)
		, mShaderResourceView(null)
	{
		mTextureDesc.MipLevels = 0;
		mTextureDesc.Width = 0;
		mTextureDesc.Height = 0;
		wstring w(WideStringFromString(name));
		if(!FAILED(CreateWICTextureFromFile(Graphics::Device, Graphics::Context, w.c_str(), (ID3D11Resource **)&mTexture2D, &mShaderResourceView)))
		{
			mTexture2D->GetDesc(&mTextureDesc);
		}
	}

	TextureImpl(int width, int height, Color color)
		: mTexture2D(null)
		, mShaderResourceView(null)
	{
		mTextureDesc.MipLevels = 0;
		mTextureDesc.Width = 0;
		mTextureDesc.Height = 0;

		UINT8 *pPixels = new UINT8[width * height * 4];
		for(int y = 0; y < height; ++y)
		{
			UINT32 *row = (UINT32 *)(pPixels + y * width * 4);
			for(int x = 0; x < width; ++x)
			{
				row[x] = color.mColor;
			}
		}
		D3D11_SUBRESOURCE_DATA data[1];
		data[0].pSysMem = (void *)pPixels;
		data[0].SysMemPitch = width * 4;
		data[0].SysMemSlicePitch = 0;

		CD3D11_TEXTURE2D_DESC desc(DXGI_FORMAT_R8G8B8A8_UNORM, width, height, 1, 1);
		HRESULT hr = Graphics::Device->CreateTexture2D(&desc, data, &mTexture2D);

		Delete(pPixels);

		if(!FAILED(hr))
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
			srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			srvDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = desc.MipLevels;
			srvDesc.Texture2D.MostDetailedMip = desc.MipLevels -1;	
			HRESULT hr = Graphics::Device->CreateShaderResourceView(mTexture2D, &srvDesc, &mShaderResourceView);

			if(FAILED(hr))
			{
				mTexture2D.Release();
			}
		}
	}

	~TextureImpl()
	{
		mShaderResourceView.Release();
		mTexture2D.Release();
	}

	DXPtr<ID3D11Texture2D>			mTexture2D;
	DXPtr<ID3D11ShaderResourceView>	mShaderResourceView;
	D3D11_TEXTURE2D_DESC			mTextureDesc;
};

//////////////////////////////////////////////////////////////////////

Texture::Texture(char const *name)
	: mName(name)
	, impl(new TextureImpl(name))
{
	mSize = Size2D(impl->mTextureDesc.Width, impl->mTextureDesc.Height);
	TRACE("LoadTexture %s %d,%d [%d,%d] : {%f,%f}\n", mName.c_str(), mSize.w, mSize.h, mSize.w, mSize.h);
}

//////////////////////////////////////////////////////////////////////

Texture::Texture(int width, int height, Color color)
	: mName(Format("%dX%dX%08x", width, height, color))
	, impl(new TextureImpl(width, height, color))
	, mSize(width, height)
{
	TRACE("CreateTexture %d,%d:%08x\n", width, height, color);
}

//////////////////////////////////////////////////////////////////////

Texture::~Texture()
{
	TRACE("Delete Texture %s\n", mName.c_str());
	mName.clear();
	Delete(impl);
}

//////////////////////////////////////////////////////////////////////

void Texture::Activate()
{
	if(impl != null)
	{
		Graphics::Context->PSSetShaderResources(0, 1, &impl->mShaderResourceView);
	}
}

//////////////////////////////////////////////////////////////////////

Texture *Texture::Load(char const *name)
{
	return new Texture(name);
}

//////////////////////////////////////////////////////////////////////

Texture *Texture::Create(int width, int height, Color color)
{
	return new Texture(width, height, color);
}

