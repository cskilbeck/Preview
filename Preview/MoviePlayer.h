//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

namespace Movie
{
	class Sampler;
	struct Player;

	//////////////////////////////////////////////////////////////////////

	struct Frame
	{
		Size2D dimensions;		// size in pixels
		int frame;				// frame number
		byte *mem;				// 32bpp xRGB data (alpha channel will be garbage, not all 00 or FF)
		list_node<Frame> node;
		bool operator == (int frameNumber) const;

	private:

		Frame(Size2D const &size, int frameNumber);
		~Frame();

		friend struct Player;
		friend class Sampler;
	};

	//////////////////////////////////////////////////////////////////////

	struct Player
	{
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
		HRESULT Seek(int64 frame);

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
