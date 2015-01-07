//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

class FrameGrabber
{
public:

	struct Frame
	{
		byte const *Buffer() const;
		uint RowPitch() const;
		uint BitsPerPixel() const;
		int Width() const;
		int Height() const;

		BITMAPINFOHEADER *bmi;
	};

	FrameGrabber();
	FrameGrabber(PWSTR filename);
	~FrameGrabber();

	void Open(PWSTR filename);
	void Close();
	void FreeFrameBuffer();

	int Width() const;
	int Height() const;

	Frame GetFrame(int nFrame);

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
