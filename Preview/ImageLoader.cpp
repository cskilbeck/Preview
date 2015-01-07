//////////////////////////////////////////////////////////////////////
// File: WICTextureLoader.cpp
//
// Function for loading a WIC image and creating a Direct3D 11 runtime texture for it
// (auto-generating mipmaps if possible)
//
// Note: Assumes application has already called CoInitializeEx
//
// Warning: CreateWICTexture* functions are not thread-safe if given a d3dContext instance for
//          auto-gen mipmap support.
//
// Note these functions are useful for images created as simple 2D textures. For
// more complex resources, DDSTextureLoader is an excellent light-weight runtime loader.
// For a full-featured DDS file reader, writer, and texture processing pipeline see
// the 'Texconv' sample and the 'DirectXTex' library.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// http://go.microsoft.com/fwlink/?LinkId=248926
// http://go.microsoft.com/fwlink/?LinkId=248929
//
// 07/01/2015 - chs, snaffled it
//////////////////////////////////////////////////////////////////////

// We could load multi-frame images (TIFF/GIF) into a texture array.
// For now, we just load the first frame (note: DirectXTex supports multi-frame images)

#include "stdafx.h"

#include <D2D1.h>
#include <dxgiformat.h>
#include <assert.h>

#pragma warning(push)
#pragma warning(disable : 4005)
#include <wincodec.h>
#pragma warning(pop)

#include <memory>
#include <string>

#include "Util.h"
#include "ImageLoader.h"

#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/) && !defined(DXGI_1_2_FORMATS)
#define DXGI_1_2_FORMATS
#endif

extern const GUID DECLSPEC_SELECTANY DBGGUID = { 0x429b8c22,0x9188,0x4b0c,0x87,0x42,0xac,0xb0,0xbf,0x85,0xc2,0x00 };

//////////////////////////////////////////////////////////////////////
// WIC Pixel Format Translation Data
//////////////////////////////////////////////////////////////////////

struct WICTranslate
{
    GUID                wic;
    DXGI_FORMAT         format;
};

static WICTranslate g_WICFormats[] = 
{
    { GUID_WICPixelFormat128bppRGBAFloat,       DXGI_FORMAT_R32G32B32A32_FLOAT },

    { GUID_WICPixelFormat64bppRGBAHalf,         DXGI_FORMAT_R16G16B16A16_FLOAT },
    { GUID_WICPixelFormat64bppRGBA,             DXGI_FORMAT_R16G16B16A16_UNORM },

    { GUID_WICPixelFormat32bppRGBA,             DXGI_FORMAT_R8G8B8A8_UNORM },
    { GUID_WICPixelFormat32bppBGRA,             DXGI_FORMAT_B8G8R8A8_UNORM }, // DXGI 1.1
    { GUID_WICPixelFormat32bppBGR,              DXGI_FORMAT_B8G8R8X8_UNORM }, // DXGI 1.1

    { GUID_WICPixelFormat32bppRGBA1010102XR,    DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM }, // DXGI 1.1
    { GUID_WICPixelFormat32bppRGBA1010102,      DXGI_FORMAT_R10G10B10A2_UNORM },
    { GUID_WICPixelFormat32bppRGBE,             DXGI_FORMAT_R9G9B9E5_SHAREDEXP },

#ifdef DXGI_1_2_FORMATS

    { GUID_WICPixelFormat16bppBGRA5551,         DXGI_FORMAT_B5G5R5A1_UNORM },
    { GUID_WICPixelFormat16bppBGR565,           DXGI_FORMAT_B5G6R5_UNORM },

#endif // DXGI_1_2_FORMATS

    { GUID_WICPixelFormat32bppGrayFloat,        DXGI_FORMAT_R32_FLOAT },
    { GUID_WICPixelFormat16bppGrayHalf,         DXGI_FORMAT_R16_FLOAT },
    { GUID_WICPixelFormat16bppGray,             DXGI_FORMAT_R16_UNORM },
    { GUID_WICPixelFormat8bppGray,              DXGI_FORMAT_R8_UNORM },

    { GUID_WICPixelFormat8bppAlpha,             DXGI_FORMAT_A8_UNORM },

#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
    { GUID_WICPixelFormat96bppRGBFloat,         DXGI_FORMAT_R32G32B32_FLOAT },
#endif
};

//////////////////////////////////////////////////////////////////////
// WIC Pixel Format nearest conversion table
//////////////////////////////////////////////////////////////////////

struct WICConvert
{
    GUID        source;
    GUID        target;
};

static WICConvert g_WICConvert[] = 
{
    // Note target GUID in this conversion table must be one of those directly supported formats (above).

    { GUID_WICPixelFormatBlackWhite,            GUID_WICPixelFormat8bppGray }, // DXGI_FORMAT_R8_UNORM

    { GUID_WICPixelFormat1bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
    { GUID_WICPixelFormat2bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
    { GUID_WICPixelFormat4bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
    { GUID_WICPixelFormat8bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 

    { GUID_WICPixelFormat2bppGray,              GUID_WICPixelFormat8bppGray }, // DXGI_FORMAT_R8_UNORM 
    { GUID_WICPixelFormat4bppGray,              GUID_WICPixelFormat8bppGray }, // DXGI_FORMAT_R8_UNORM 

    { GUID_WICPixelFormat16bppGrayFixedPoint,   GUID_WICPixelFormat16bppGrayHalf }, // DXGI_FORMAT_R16_FLOAT 
    { GUID_WICPixelFormat32bppGrayFixedPoint,   GUID_WICPixelFormat32bppGrayFloat }, // DXGI_FORMAT_R32_FLOAT 

#ifdef DXGI_1_2_FORMATS

    { GUID_WICPixelFormat16bppBGR555,           GUID_WICPixelFormat16bppBGRA5551 }, // DXGI_FORMAT_B5G5R5A1_UNORM

#else

    { GUID_WICPixelFormat16bppBGR555,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
    { GUID_WICPixelFormat16bppBGRA5551,         GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
    { GUID_WICPixelFormat16bppBGR565,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM

#endif // DXGI_1_2_FORMATS

    { GUID_WICPixelFormat32bppBGR101010,        GUID_WICPixelFormat32bppRGBA1010102 }, // DXGI_FORMAT_R10G10B10A2_UNORM

    { GUID_WICPixelFormat24bppBGR,              GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
    { GUID_WICPixelFormat24bppRGB,              GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
    { GUID_WICPixelFormat32bppPBGRA,            GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
    { GUID_WICPixelFormat32bppPRGBA,            GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 

    { GUID_WICPixelFormat48bppRGB,              GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
    { GUID_WICPixelFormat48bppBGR,              GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
    { GUID_WICPixelFormat64bppBGRA,             GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
    { GUID_WICPixelFormat64bppPRGBA,            GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
    { GUID_WICPixelFormat64bppPBGRA,            GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM

    { GUID_WICPixelFormat48bppRGBFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
    { GUID_WICPixelFormat48bppBGRFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
    { GUID_WICPixelFormat64bppRGBAFixedPoint,   GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
    { GUID_WICPixelFormat64bppBGRAFixedPoint,   GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
    { GUID_WICPixelFormat64bppRGBFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
    { GUID_WICPixelFormat64bppRGBHalf,          GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
    { GUID_WICPixelFormat48bppRGBHalf,          GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 

    { GUID_WICPixelFormat96bppRGBFixedPoint,    GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT 
    { GUID_WICPixelFormat128bppPRGBAFloat,      GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT 
    { GUID_WICPixelFormat128bppRGBFloat,        GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT 
    { GUID_WICPixelFormat128bppRGBAFixedPoint,  GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT 
    { GUID_WICPixelFormat128bppRGBFixedPoint,   GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT 

    { GUID_WICPixelFormat32bppCMYK,             GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
    { GUID_WICPixelFormat64bppCMYK,             GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
    { GUID_WICPixelFormat40bppCMYKAlpha,        GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
    { GUID_WICPixelFormat80bppCMYKAlpha,        GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM

#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
    { GUID_WICPixelFormat32bppRGB,              GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
    { GUID_WICPixelFormat64bppRGB,              GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
    { GUID_WICPixelFormat64bppPRGBAHalf,        GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
#endif

    // We don't support n-channel formats
};

//////////////////////////////////////////////////////////////////////

static IWICImagingFactory *_GetWIC()
{
    static IWICImagingFactory *s_Factory = null;

	if(s_Factory == null)
	{
		CoInitializeEx(null, 0);
		if(FAILED(CoCreateInstance(CLSID_WICImagingFactory1,
									null,
									CLSCTX_INPROC_SERVER,
									__uuidof(IWICImagingFactory),
									(LPVOID*)&s_Factory)))
		{
			// throw something...
			s_Factory = null;
		}
	}
    return s_Factory;
}

//////////////////////////////////////////////////////////////////////

HRESULT GetWIC(IWICImagingFactory **wic)
{
	*wic = _GetWIC();
	return *wic == null ? E_NOINTERFACE : S_OK;
}

//////////////////////////////////////////////////////////////////////

static DXGI_FORMAT _WICToDXGI(GUID const &guid)
{
	for(size_t i = 0; i < _countof(g_WICFormats); ++i)
    {
		if(g_WICFormats[i].wic == guid)
		{
			return g_WICFormats[i].format;
		}
    }
    return DXGI_FORMAT_UNKNOWN;
}

//////////////////////////////////////////////////////////////////////

static size_t _WICBitsPerPixel(REFGUID targetGuid)
{
	UINT bpp = 0;

	IWICImagingFactory *pWIC;
	DXZ(GetWIC(&pWIC));
 
    DXPtr<IWICComponentInfo> cinfo;
	DXZ(pWIC->CreateComponentInfo(targetGuid, &cinfo));

    WICComponentType type;
	DXZ(cinfo->GetComponentType(&type));
	if(type == WICPixelFormat)
	{
		DXPtr<IWICPixelFormatInfo> pfinfo;
		DXZ(cinfo->QueryInterface(__uuidof(IWICPixelFormatInfo), reinterpret_cast<void**>(&pfinfo)));
		DXZ(pfinfo->GetBitsPerPixel(&bpp));
	}
	return bpp;
}

//////////////////////////////////////////////////////////////////////

static HRESULT GetFormatAndBPP(IWICBitmapFrameDecode *frame, size_t *bpp, DXGI_FORMAT *format, WICPixelFormatGUID *pixelFormat, WICPixelFormatGUID *convertGUID)
{
	if(bpp == null || format == null || pixelFormat == null || convertGUID == null)
	{
		return E_INVALIDARG;
	}

	// Determine format
	DX(frame->GetPixelFormat(pixelFormat));
	*convertGUID = *pixelFormat;
	*format = _WICToDXGI(*pixelFormat);
	if(*format == DXGI_FORMAT_UNKNOWN)
	{
		for(size_t i = 0; i < _countof(g_WICConvert); ++i)
		{
			if(g_WICConvert[i].source == *pixelFormat)
			{
				*convertGUID = g_WICConvert[i].target;
				*format = _WICToDXGI(g_WICConvert[i].target);
				assert(*format != DXGI_FORMAT_UNKNOWN);
				*bpp = _WICBitsPerPixel(*convertGUID);
				break;
			}
		}
		if(*format == DXGI_FORMAT_UNKNOWN)
		{
			return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
		}
	}
	else
	{
		*bpp = _WICBitsPerPixel(*pixelFormat);
	}
	if(*bpp == 0)
	{
		return E_FAIL;
	}

	// Verify our target format is supported by the current device
	// (handles WDDM 1.0 or WDDM 1.1 device driver cases as well as
	//  DirectX 11.0 Runtime without 16bpp format support)
	UINT support = 0;
	HRESULT hr = gDevice->CheckFormatSupport(*format, &support);
	if(FAILED(hr) || !(support & D3D11_FORMAT_SUPPORT_TEXTURE2D))
	{
		// Fallback to RGBA 32-bit format which is supported by all devices
		*convertGUID = GUID_WICPixelFormat32bppRGBA;
		*format = DXGI_FORMAT_R8G8B8A8_UNORM;
		*bpp = 32;
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////
// This is a bit conservative because the hardware could support larger textures than
// the Feature Level defined minimums, but doing it this way is much easier and more
// performant for WIC than the 'fail and retry' model used by DDSTextureLoader

static size_t GetMaxSize(size_t maxsize)
{
	if(maxsize == 0)
	{
		switch(gDevice->GetFeatureLevel())
		{
			case D3D_FEATURE_LEVEL_9_1:
			case D3D_FEATURE_LEVEL_9_2:
				return D3D_FL9_1_REQ_TEXTURE2D_U_OR_V_DIMENSION;

			case D3D_FEATURE_LEVEL_9_3:
				return D3D_FL9_3_REQ_TEXTURE2D_U_OR_V_DIMENSION;

			case D3D_FEATURE_LEVEL_10_0:
			case D3D_FEATURE_LEVEL_10_1:
				return D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION;

			default:
				return D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
		}
	}
	return maxsize;
}

//////////////////////////////////////////////////////////////////////

static HRESULT CreateTextureFromWIC(_In_opt_ ID3D11DeviceContext* d3dContext,
                                     _In_ IWICBitmapFrameDecode *frame,
                                     _Out_opt_ ID3D11Resource** texture,
                                     _Out_opt_ ID3D11ShaderResourceView** textureView,
                                     _In_ size_t maxsize,
									 _In_ bool createMipMaps)
{
	// get image dimensions
    UINT width, height;
	DX(frame->GetSize(&width, &height));
    assert(width > 0 && height > 0);

	// get how big it's allowed to be
	maxsize = GetMaxSize(maxsize);

	// work out scaled size if necessary
	UINT twidth = width;
	UINT theight = height;
	if(twidth > maxsize || theight > maxsize)
    {
        float ar = static_cast<float>(theight) / static_cast<float>(twidth);
        if(twidth > theight)
        {
            twidth = static_cast<UINT>(maxsize);
            theight = static_cast<UINT>(maxsize * ar);
        }
        else
        {
            twidth = static_cast<UINT>(maxsize / ar);
			theight = static_cast<UINT>(maxsize);
		}
    }
	assert(twidth <= maxsize && theight <= maxsize);

	// Get Format and BPP
	size_t bpp = 0;
	DXGI_FORMAT format;
	WICPixelFormatGUID pixelFormat;
	WICPixelFormatGUID convertGUID;
	DX(GetFormatAndBPP(frame, &bpp, &format, &pixelFormat, &convertGUID));

    // Allocate temporary memory for image
    size_t rowPitch = (twidth * bpp + 7) / 8;
    size_t imageSize = rowPitch * theight;
    std::unique_ptr<uint8_t[]> temp(new uint8_t[imageSize]);

	IWICImagingFactory *pWIC;
	
	// Load image data
	if(pixelFormat == convertGUID && twidth == width && theight == height)
    {
		// No conversion, no resize
		DX(frame->CopyPixels(0, static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize), temp.get()));
    }
	else if(twidth != width || theight != height)
    {
        // Resize for sure
		DX(GetWIC(&pWIC));

        DXPtr<IWICBitmapScaler> scaler;
		WICPixelFormatGUID pfScaler;

		DX(pWIC->CreateBitmapScaler(&scaler));
		DX(scaler->Initialize(frame, twidth, theight, WICBitmapInterpolationModeFant));
		DX(scaler->GetPixelFormat(&pfScaler));

		if(convertGUID == pfScaler)
        {
            // No conversion, with resize
			DX(scaler->CopyPixels(0, static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize), temp.get()));
        }
        else
        {
			// Conversion, with resize
            DXPtr<IWICFormatConverter> FC;
			DX(pWIC->CreateFormatConverter(&FC));
			DX(FC->Initialize(scaler.get(), convertGUID, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom));
			DX(FC->CopyPixels(0, static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize), temp.get()));
        }
    }
    else
    {
        // Conversion, no resize
		DX(GetWIC(&pWIC));

        DXPtr<IWICFormatConverter> FC;
		DX(pWIC->CreateFormatConverter(&FC));
		DX(FC->Initialize(frame, convertGUID, WICBitmapDitherTypeErrorDiffusion, null, 0, WICBitmapPaletteTypeCustom));
		DX(FC->CopyPixels(0, static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize), temp.get()));
    }

    // See if format is supported for auto-gen mipmaps(varies by feature level)
    bool autogen = false;
    if(d3dContext != 0 && textureView != null) // Must have context and shader-view to auto generate mipmaps
    {
        UINT fmtSupport = 0;
		HRESULT hr = gDevice->CheckFormatSupport(format, &fmtSupport);
        if(SUCCEEDED(hr) && (fmtSupport & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN))
        {
            autogen = createMipMaps;
        }
    }

    // Create texture
    D3D11_TEXTURE2D_DESC desc;
    desc.Width = twidth;
    desc.Height = theight;
    desc.MipLevels = autogen ? 0 : 1;
    desc.ArraySize = 1;
    desc.Format = format;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = autogen ? (D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET) : D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = autogen ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;

    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = temp.get();
    initData.SysMemPitch = static_cast<UINT>(rowPitch);
    initData.SysMemSlicePitch = static_cast<UINT>(imageSize);

    DXPtr<ID3D11Texture2D> tex;
	DX(gDevice->CreateTexture2D(&desc, autogen ? null : &initData, &tex));
    if(tex != null)
    {
        if(textureView != null)
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
            memset(&SRVDesc, 0, sizeof(SRVDesc));
            SRVDesc.Format = format;
            SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            SRVDesc.Texture2D.MipLevels = autogen ? -1 : 1;

			DX(gDevice->CreateShaderResourceView(tex, &SRVDesc, textureView));

            if(autogen)
            {
                assert(d3dContext != 0);
                d3dContext->UpdateSubresource(tex, 0, null, temp.get(), static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize));
                d3dContext->GenerateMips(*textureView);
            }
        }

        if(texture != null)
        {
			tex->AddRef();
            *texture = tex.get();
        }
    }
    return S_OK;
}

//////////////////////////////////////////////////////////////////////

void SetDebugInfo(ID3D11Resource **texture, ID3D11ShaderResourceView **textureView, wchar const *pstrName)
{
#if defined(_DEBUG) || defined(PROFILE)

	string s = StringFromTString(GetFilename(TStringFromWideString(wstring(pstrName)).c_str()));

	if(texture != null && *texture != null)
	{
		(*texture)->SetPrivateData(DBGGUID, static_cast<UINT>(s.size()), s.c_str());
	}
	if(textureView != null && *textureView != null)
	{
		(*textureView)->SetPrivateData(DBGGUID, static_cast<UINT>(s.size()), s.c_str());
	}

#endif
}

//////////////////////////////////////////////////////////////////////

HRESULT CreateWICTextureFromMemory(	_In_bytecount_(wicDataSize)	uint8_t const *wicData,
									_In_						size_t wicDataSize,
									_Out_opt_					ID3D11Resource **texture,
									_Out_opt_					ID3D11ShaderResourceView **textureView,
									_In_						size_t maxsize,
									_In_						bool createMipMaps
									)
{
	if(gDevice == null || wicData == null || (texture == null && textureView == null))
    {
        return E_INVALIDARG;
    }

	if(wicDataSize == 0)
    {
        return E_FAIL;
    }

#ifdef _M_AMD64
	if(wicDataSize > 0xFFFFFFFF)
	{
		return HRESULT_FROM_WIN32(ERROR_FILE_TOO_LARGE);
	}
#endif

	DXPtr<ID3D11DeviceContext> context;
	if(createMipMaps)
	{
		gDevice->GetImmediateContext(&context);
	}

	IWICImagingFactory* pWIC;
	DX(GetWIC(&pWIC));

	DXPtr<IWICStream> stream;
	DX(pWIC->CreateStream(&stream));
	DX(stream->InitializeFromMemory(const_cast<uint8_t*>(wicData), static_cast<DWORD>(wicDataSize)));

	DXPtr<IWICBitmapDecoder> decoder;
	DX(pWIC->CreateDecoderFromStream(stream.get(), 0, WICDecodeMetadataCacheOnDemand, &decoder));

	DXPtr<IWICBitmapFrameDecode> frame;
	DX(decoder->GetFrame(0, &frame));

	DX(CreateTextureFromWIC(context.get(), frame.get(), texture, textureView, maxsize, createMipMaps));

	SetDebugInfo(texture, textureView, L"WICTextureLoader");
    return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT CreateWICTextureFromFile( _In_z_	const wchar *fileName,
                                  _Out_opt_	ID3D11Resource **texture,
                                  _Out_opt_	ID3D11ShaderResourceView **textureView,
                                  _In_		size_t maxsize,
								  _In_		bool createMipMaps)
{
	assert(gDevice != null);

	if(fileName == null || (texture == null && textureView == null))
	{
		return E_INVALIDARG;
	}

	DXPtr<ID3D11DeviceContext> context;
	if(createMipMaps)
	{
		gDevice->GetImmediateContext(&context);
	}

	IWICImagingFactory *pWIC;
	DX(GetWIC(&pWIC));

	DXPtr<IWICBitmapDecoder> decoder;
	DX(pWIC->CreateDecoderFromFilename(fileName, 0, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder));

	DXPtr<IWICBitmapFrameDecode> frame;
	DX(decoder->GetFrame(0, &frame));

	DX(CreateTextureFromWIC(context.get(), frame.get(), texture, textureView, maxsize, createMipMaps));

	SetDebugInfo(texture, textureView, fileName);

	return S_OK;
}
