//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Win32.h"
#include "ImageLoader.h"

//////////////////////////////////////////////////////////////////////

Texture::Texture(tchar const *name, Window *parent)
	: mTexture2D(null)
	, mShaderResourceView(null)
	, mContext(parent->mContext)
	, mDevice(parent->mDevice)
	, mName(name)
{
	mTextureDesc.MipLevels = 0;
	mTextureDesc.Width = 0;
	mTextureDesc.Height = 0;
	wstring w(WideStringFromTString(name));
	if(!FAILED(CreateWICTextureFromFile(mDevice, mContext, w.c_str(), (ID3D11Resource **)&mTexture2D, &mShaderResourceView)))
	{
		mTexture2D->GetDesc(&mTextureDesc);
	}
}

//////////////////////////////////////////////////////////////////////

Texture::Texture(int width, int height, byte *pixels, Window *parent)
	: mTexture2D(null)
	, mShaderResourceView(null)
	, mContext(parent->mContext)
	, mDevice(parent->mDevice)
{
	mTextureDesc.MipLevels = 0;
	mTextureDesc.Width = 0;
	mTextureDesc.Height = 0;
}

//////////////////////////////////////////////////////////////////////

Texture::Texture(int width, int height, Color color, Window *parent)
	: mTexture2D(null)
	, mShaderResourceView(null)
	, mContext(parent->mContext)
	, mDevice(parent->mDevice)
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
	HRESULT hr = mDevice->CreateTexture2D(&desc, data, &mTexture2D);

	Delete(pPixels);

	if(!FAILED(hr))
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = desc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = desc.MipLevels -1;	
		HRESULT hr = mDevice->CreateShaderResourceView(mTexture2D, &srvDesc, &mShaderResourceView);

		if(FAILED(hr))
		{
			mTexture2D.Release();
		}
	}
}

//////////////////////////////////////////////////////////////////////

Texture::~Texture()
{
}

//////////////////////////////////////////////////////////////////////

void Texture::Activate(int channel /* = 0 */)
{
	mContext->PSSetShaderResources(channel, 1, &mShaderResourceView);
}
