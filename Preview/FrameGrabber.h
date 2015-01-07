//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

class Video
{
public:

	struct Frame
	{
		byte const *Buffer() const;
		uint RowPitch() const;
		uint BytesPerPixel() const;
		int Width() const;
		int Height() const;

		BITMAPINFOHEADER *bmi;
	};

	Video();
	Video(PWSTR filename);
	~Video();

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
