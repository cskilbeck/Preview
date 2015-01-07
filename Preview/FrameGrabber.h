//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

class FrameGrabber
{
public:

	struct Frame
	{
		BITMAPINFOHEADER *bmi;
		byte const *Buffer() const;
		uint RowPitch() const;
		uint BitsPerPixel() const;
		int Width() const;
		int Height() const;
	};

	FrameGrabber();
	FrameGrabber(PWSTR filename);
	void Open(PWSTR filename);
	void Close();
	int Width() const;
	int Height() const;
	void FreeFrameBuffer();
	Frame GetFrame(int nFrame);
	~FrameGrabber();

private:

	void Init();

	DXPtr<IMediaDet>	pMediaDet;
	long				streamID;
	BITMAPINFOHEADER *	pbih;
	double				fps;
	double				length;
	long				bufferSize;
	byte *				buffer;
	VIDEOINFOHEADER		videoInfoHeader;

};

//////////////////////////////////////////////////////////////////////
