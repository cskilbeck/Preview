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

int Video::Frames() const
{
	return frames;
}

//////////////////////////////////////////////////////////////////////
// This always returns 3 (RGB24)

uint Video::Frame::BytesPerPixel() const
{
	assert(bmi->biBitCount == 24);
	return 3;
}

//////////////////////////////////////////////////////////////////////
// This rounds up

uint Video::Frame::RowPitch() const
{
	assert(bmi->biBitCount == 24);
	return (bmi->biWidth * 3 + 3) & -4;
}

//////////////////////////////////////////////////////////////////////

Video::Video() : buffer(null)
{
}

//////////////////////////////////////////////////////////////////////

void Video::Init()
{
	if(pMediaDet == null)
	{
		DXT(CoCreateInstance(CLSID_MediaDet, NULL, CLSCTX_INPROC, IID_IMediaDet, (void **)&pMediaDet));
	}
}

//////////////////////////////////////////////////////////////////////

Video::~Video()
{
	FreeFrameBuffer();
}

//////////////////////////////////////////////////////////////////////

void Video::Open(PWSTR filename)
{
	Init();
	FreeFrameBuffer();

	DXT(pMediaDet->put_Filename(filename));

	long streamID;
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
		frames = (int)(length * fps);
		TRACE("Video file is %dx%d,has %d frames at %.2ffps, is %.2f seconds long\n", Width(), Height(), frames, fps, length);
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

bool Video::GetFrame(int nFrame, Video::Frame &frame)
{
	frame.bmi = null;
	frame.frame = -1;
	if(nFrame >= frames)
	{
		return false;
	}
	try
	{
		DXT(pMediaDet->GetBitmapBits(nFrame / fps, &bufferSize, (char *)buffer, Width(), Height()));
	}
	catch(HRException e)
	{
		return false;
	}
	frame.bmi = (BITMAPINFOHEADER *)buffer;
	frame.frame = nFrame;
	return true;
}

