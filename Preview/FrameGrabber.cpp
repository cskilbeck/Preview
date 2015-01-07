//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#pragma comment(lib,"strmiids")

//////////////////////////////////////////////////////////////////////

namespace
{
	class AM_MEDIA_TYPE_: public AM_MEDIA_TYPE
	{
	public:

		AM_MEDIA_TYPE_()
		{
			ZeroMemory(this, sizeof(*this));
		}

		AM_MEDIA_TYPE_(GUID const &major, GUID const &sub)
		{
			ZeroMemory(this, sizeof(*this));
			majortype = major;
			subtype = sub;
		}

		~AM_MEDIA_TYPE_()
		{
			if(cbFormat != 0)
			{
				CoTaskMemFree((PVOID)pbFormat);
				cbFormat = 0;
				pbFormat = null;
			}
			if(pUnk != null)
			{
				pUnk->Release(); // pUnk should not be used.
				pUnk = null;
			}
		}
	};
}

//////////////////////////////////////////////////////////////////////

byte const *Video::Frame::Buffer() const
{
	return (byte *)bmi + bmi->biSize;
}

//////////////////////////////////////////////////////////////////////

int Video::Frame::Width() const
{
	return bmi->biWidth;
}

//////////////////////////////////////////////////////////////////////

int Video::Frame::Height() const
{
	return bmi->biHeight;
}

//////////////////////////////////////////////////////////////////////

uint Video::Frame::BytesPerPixel() const
{
	return (bmi->biBitCount + 7) / 8;
}

//////////////////////////////////////////////////////////////////////

uint Video::Frame::RowPitch() const
{
	return bmi->biWidth * BytesPerPixel();
}

//////////////////////////////////////////////////////////////////////

Video::Video() : buffer(null)
{
	Init();
}

//////////////////////////////////////////////////////////////////////

Video::Video(PWSTR filename) : buffer(null)
{
	Init();
	Open(filename);
}

//////////////////////////////////////////////////////////////////////

void Video::Open(PWSTR filename)
{
	FreeFrameBuffer();

	DXT(pMediaDet->put_Filename(filename));

	long numStreams;
	DXT(pMediaDet->get_OutputStreams(&numStreams));

	for(long stream = 0; stream < numStreams; stream++)
	{
		AM_MEDIA_TYPE_ amMediaType;
		DXT(pMediaDet->put_CurrentStream(stream));
		DXT(pMediaDet->get_StreamMediaType(&amMediaType));

		if(amMediaType.majortype == MEDIATYPE_Video && amMediaType.formattype == FORMAT_VideoInfo)
		{
			streamID = stream;
			memcpy(&videoInfoHeader, (VIDEOINFOHEADER *)(amMediaType.pbFormat), sizeof(VIDEOINFOHEADER));
			DXT(pMediaDet->GetBitmapBits(0, &bufferSize, NULL, Width(), Height()));
			buffer = new byte[bufferSize];
			break;
		}
	}
	if(streamID != -1)
	{
		DXT(pMediaDet->get_FrameRate(&fps));
		DXT(pMediaDet->get_StreamLength(&length));
		TRACE("Video file is %dx%d,has %d frames at %.2ffps, is %.2f seconds long\n", Width(), Height(), (uint)(length * fps), fps, length);
	}
}

//////////////////////////////////////////////////////////////////////

int Video::Width() const
{
	return videoInfoHeader.bmiHeader.biWidth;
}

//////////////////////////////////////////////////////////////////////

int Video::Height() const
{
	return videoInfoHeader.bmiHeader.biHeight;
}

//////////////////////////////////////////////////////////////////////

void Video::Close()
{
	pMediaDet.Release();
	FreeFrameBuffer();
}

//////////////////////////////////////////////////////////////////////

void Video::FreeFrameBuffer()
{
	if(buffer != null)
	{
		delete[] buffer;
		buffer = null;
	}
}

//////////////////////////////////////////////////////////////////////

Video::Frame Video::GetFrame(int nFrame)
{
	Frame f;
	DXT(pMediaDet->GetBitmapBits(nFrame / fps, &bufferSize, (char *)buffer, Width(), Height()));
	f.bmi = (BITMAPINFOHEADER *)buffer;
	return f;
}

//////////////////////////////////////////////////////////////////////

Video::~Video()
{
	FreeFrameBuffer();
}

//////////////////////////////////////////////////////////////////////

void Video::Init()
{
	streamID = -1;
	pbih = null;
	DXT(CoCreateInstance(CLSID_MediaDet, NULL, CLSCTX_INPROC, IID_IMediaDet, (void **)&pMediaDet));
}
