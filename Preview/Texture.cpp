//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Win32.h"
#include "ImageLoader.h"

//////////////////////////////////////////////////////////////////////

DXPtr<ID3D11Device> Texture::sDevice;
DXPtr<ID3D11DeviceContext> Texture::sContext;

//////////////////////////////////////////////////////////////////////

void Texture::SetDeviceAndContext(DXPtr<ID3D11Device> device, DXPtr<ID3D11DeviceContext> context)
{
	sDevice = device;
	sContext = context;
}

//////////////////////////////////////////////////////////////////////

void Texture::ReleaseDeviceAndContext()
{
	sDevice.Release();
	sContext.Release();
}

//////////////////////////////////////////////////////////////////////

Texture::Texture(tchar const *name)
	: mTexture2D(null)
	, mShaderResourceView(null)
	, mName(name)
{
	mTextureDesc.MipLevels = 0;
	mTextureDesc.Width = 0;
	mTextureDesc.Height = 0;
	wstring w(WideStringFromTString(name));
	if(!FAILED(CreateWICTextureFromFile(sDevice, sContext, w.c_str(), (ID3D11Resource **)&mTexture2D, &mShaderResourceView)))
	{
		mTexture2D->GetDesc(&mTextureDesc);
	}
}

//////////////////////////////////////////////////////////////////////

Texture::Texture(int width, int height, byte *pixels)
	: mTexture2D(null)
	, mShaderResourceView(null)
{
	mTextureDesc.MipLevels = 0;
	mTextureDesc.Width = 0;
	mTextureDesc.Height = 0;
}

//////////////////////////////////////////////////////////////////////

Texture::Texture(int width, int height, Color color)
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
	HRESULT hr = sDevice->CreateTexture2D(&desc, data, &mTexture2D);

	Delete(pPixels);

	if(!FAILED(hr))
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = desc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = desc.MipLevels -1;	
		HRESULT hr = sDevice->CreateShaderResourceView(mTexture2D, &srvDesc, &mShaderResourceView);

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
	sContext->PSSetShaderResources(channel, 1, &mShaderResourceView);
}
