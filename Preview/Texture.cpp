//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Win32.h"
#include "ImageLoader.h"

//////////////////////////////////////////////////////////////////////

static uint BPPFromTextureFormat(DXGI_FORMAT format)
{
	switch(format)
	{
		case DXGI_FORMAT_UNKNOWN:
			return 0;

		case DXGI_FORMAT_R32G32B32A32_TYPELESS:
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
		case DXGI_FORMAT_R32G32B32A32_UINT:
		case DXGI_FORMAT_R32G32B32A32_SINT:
			return 128;

		case DXGI_FORMAT_R32G32B32_TYPELESS:
		case DXGI_FORMAT_R32G32B32_FLOAT:
		case DXGI_FORMAT_R32G32B32_UINT:
		case DXGI_FORMAT_R32G32B32_SINT:
			return 96;

		case DXGI_FORMAT_R16G16B16A16_TYPELESS:
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
		case DXGI_FORMAT_R16G16B16A16_UNORM:
		case DXGI_FORMAT_R16G16B16A16_UINT:
		case DXGI_FORMAT_R16G16B16A16_SNORM:
		case DXGI_FORMAT_R16G16B16A16_SINT:
		case DXGI_FORMAT_R32G32_TYPELESS:
		case DXGI_FORMAT_R32G32_FLOAT:
		case DXGI_FORMAT_R32G32_UINT:
		case DXGI_FORMAT_R32G32_SINT:
		case DXGI_FORMAT_R32G8X24_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
			return 64;

		case DXGI_FORMAT_R10G10B10A2_TYPELESS:
		case DXGI_FORMAT_R10G10B10A2_UNORM:
		case DXGI_FORMAT_R10G10B10A2_UINT:
		case DXGI_FORMAT_R11G11B10_FLOAT:
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_R8G8B8A8_UINT:
		case DXGI_FORMAT_R8G8B8A8_SNORM:
		case DXGI_FORMAT_R8G8B8A8_SINT:
		case DXGI_FORMAT_R16G16_TYPELESS:
		case DXGI_FORMAT_R16G16_FLOAT:
		case DXGI_FORMAT_R16G16_UNORM:
		case DXGI_FORMAT_R16G16_UINT:
		case DXGI_FORMAT_R16G16_SNORM:
		case DXGI_FORMAT_R16G16_SINT:
		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_R32_FLOAT:
		case DXGI_FORMAT_R32_UINT:
		case DXGI_FORMAT_R32_SINT:
		case DXGI_FORMAT_R24G8_TYPELESS:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8X8_UNORM:
			return 32;

		case DXGI_FORMAT_R8G8_TYPELESS:
		case DXGI_FORMAT_R8G8_UNORM:
		case DXGI_FORMAT_R8G8_UINT:
		case DXGI_FORMAT_R8G8_SNORM:
		case DXGI_FORMAT_R8G8_SINT:
		case DXGI_FORMAT_R16_TYPELESS:
		case DXGI_FORMAT_R16_FLOAT:
		case DXGI_FORMAT_D16_UNORM:
		case DXGI_FORMAT_R16_UNORM:
		case DXGI_FORMAT_R16_UINT:
		case DXGI_FORMAT_R16_SNORM:
		case DXGI_FORMAT_R16_SINT:
		case DXGI_FORMAT_B5G6R5_UNORM:
		case DXGI_FORMAT_B5G5R5A1_UNORM:
			return 16;

		case DXGI_FORMAT_R8_TYPELESS:
		case DXGI_FORMAT_R8_UNORM:
		case DXGI_FORMAT_R8_UINT:
		case DXGI_FORMAT_R8_SNORM:
		case DXGI_FORMAT_R8_SINT:
		case DXGI_FORMAT_A8_UNORM:
			return 8;

		// Compressed format; http://msdn2.microsoft.com/en-us/library/bb694531(VS.85).aspx
		case DXGI_FORMAT_BC2_TYPELESS:
		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC2_UNORM_SRGB:
		case DXGI_FORMAT_BC3_TYPELESS:
		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC3_UNORM_SRGB:
		case DXGI_FORMAT_BC5_TYPELESS:
		case DXGI_FORMAT_BC5_UNORM:
		case DXGI_FORMAT_BC5_SNORM:
			return 128;

		// Compressed format; http://msdn2.microsoft.com/en-us/library/bb694531(VS.85).aspx
		case DXGI_FORMAT_R1_UNORM:
		case DXGI_FORMAT_BC1_TYPELESS:
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC1_UNORM_SRGB:
		case DXGI_FORMAT_BC4_TYPELESS:
		case DXGI_FORMAT_BC4_UNORM:
		case DXGI_FORMAT_BC4_SNORM:
			return 64;

		// Compressed format; http://msdn2.microsoft.com/en-us/library/bb694531(VS.85).aspx
		case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
			return 32;

		// These are compressed, but bit-size information is unclear.
		case DXGI_FORMAT_R8G8_B8G8_UNORM:
		case DXGI_FORMAT_G8R8_G8B8_UNORM:
			return 32;

		default:
			return 0;
	}
}


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

void Texture::InitFromPixelBuffer(byte *buffer, DXGI_FORMAT pixelFormat, int width, int height)
{
	uint bpp = BPPFromTextureFormat(pixelFormat);
	D3D11_SUBRESOURCE_DATA data[1];
	data[0].pSysMem = (void *)buffer;
	data[0].SysMemPitch = (width * bpp + 7) / 8;
	data[0].SysMemSlicePitch = 0;

	CD3D11_TEXTURE2D_DESC desc(pixelFormat, width, height, 1, 1);
	if(!FAILED(sDevice->CreateTexture2D(&desc, data, &mTexture2D)))
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = pixelFormat;
		srvDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = desc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = desc.MipLevels - 1;
		if(FAILED(sDevice->CreateShaderResourceView(mTexture2D, &srvDesc, &mShaderResourceView)))
		{
			mTexture2D.Release();
		}
		else
		{
			mTexture2D->GetDesc(&mTextureDesc);
		}
	}
}

//////////////////////////////////////////////////////////////////////

uint Texture::BitsPerPixel() const
{
	return BPPFromTextureFormat(mTextureDesc.Format);
}

//////////////////////////////////////////////////////////////////////

void Texture::Update(byte *pixels)
{
	uint rowPitch = (Width() * BitsPerPixel() + 7) / 8;
	sContext->UpdateSubresource(mTexture2D, 0, null, pixels, rowPitch, rowPitch * Height());
}

//////////////////////////////////////////////////////////////////////

Texture::Texture(tchar const *name)
	: mName(name)
{
	if(!FAILED(CreateWICTextureFromFile(sDevice, sContext, WideStringFromTString(mName).c_str(), (ID3D11Resource **)&mTexture2D, &mShaderResourceView)))
	{
		mTexture2D->GetDesc(&mTextureDesc);
	}
}

//////////////////////////////////////////////////////////////////////

Texture::Texture(int w, int h, DXGI_FORMAT format, byte *pixels)
{
	InitFromPixelBuffer(pixels, format, w, h);
}

//////////////////////////////////////////////////////////////////////

Texture::Texture(int width, int height, Color color)
{
	Ptr<byte> pPixels(new byte[width * height * 4]);
	for(int y = 0; y < height; ++y)
	{
		Color *row = (Color *)(pPixels.get() + y * width * 4);
		for(int x = 0; x < width; ++x)
		{
			row[x] = color;
		}
	}
	InitFromPixelBuffer(pPixels.get(), DXGI_FORMAT_B8G8R8A8_UNORM, width, height);
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
