//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

namespace Movie
{
	class Sampler;
	class Player;

	//////////////////////////////////////////////////////////////////////

	class Frame
	{
	public:

		int frame;				// frame number
		byte *mem;				// 32bpp xRGB data (alpha channel will be garbage, not all 00 or FF)

		list_node<Frame> node;

		bool operator == (int frameNumber) const;	// for finding frames in a Queue<>

	private:

		Frame(int frameNumber, size_t bufferSize);
		~Frame();

		friend class Sampler;
		friend class Player;

	};

	//////////////////////////////////////////////////////////////////////

	class Player
	{
	public:

		Player();
		~Player();
		HRESULT Open(wchar const *filename, int maxFrameBuffer = 3);
		void FlushBuffers();
		HRESULT Close();
		int FramesWaiting() const;
		Frame *GetFrame();
		Frame *GetFrame(int frameNumber);
		void Flush();
		void ReleaseFrame(Frame *f);
		bool IsOpen() const;
		long GetState() const;
		bool IsPaused() const;
		bool IsStopped() const;
		bool IsRunning() const;
		int Width() const;
		int Height() const;
		HRESULT Pause();
		HRESULT Play();
		HRESULT Seek(uint frame);

	private:

		DXPtr<IGraphBuilder>	graph;
		DXPtr<IMediaControl>	mediaControl;
		DXPtr<IMediaSeeking>	seeker;
		DXPtr<IPin>				rendererPin;
		DXPtr<IBaseFilter>		videoSource;
		DXPtr<IPin>				videoOutputPin;
		DXPtr<Sampler>			sampler;
		DWORD					seekingCapabilites;

	};

} // namespace Movie
