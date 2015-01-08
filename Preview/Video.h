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

		bool operator == (int f) const
		{
			return frame == f;
		}

		BITMAPINFOHEADER *bmi;
		int frame;
	};

	Video();
	~Video();

	void Open(PWSTR filename);
	void Close();
	void FreeFrameBuffer();

	int Width() const;
	int Height() const;
	int Frames() const;

	bool GetFrame(int nFrame, Frame &frame);

private:

	void Init();

	DXPtr<IMediaDet>	pMediaDet;
	double				fps;
	double				length;
	int					frames;
	long				bufferSize;
	byte *				buffer;
	VIDEOINFOHEADER		videoInfoHeader;

};

//////////////////////////////////////////////////////////////////////
