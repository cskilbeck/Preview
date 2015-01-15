//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////

namespace Movie
{
	//////////////////////////////////////////////////////////////////////

	struct __declspec(uuid("{71771540-2017-11cf-ae26-0020afd79767}")) CLSID_Sampler;

	//////////////////////////////////////////////////////////////////////

	using FrameQueue = Queue < Frame, &Frame::node >;
	using FrameList = linked_list < Frame, &Frame::node >;

	class Sampler : public CBaseVideoRenderer
	{
	public:

		Sampler(int maxFramesToBuffer, IUnknown* unk, HRESULT *hr);
		~Sampler();
		void FlushFrames();
		void Abort();
		void FlushCache();
		int Width() const;
		int Height() const;
		int FramesWaiting();
		Frame *GetFrame();
		Frame *GetFrame(int frameNumber);
		void ReleaseFrame(Frame *f);
		void Flush();
		HRESULT CheckMediaType(const CMediaType *media);
		HRESULT DoRenderSample(IMediaSample *sample);
		HRESULT WaitForRenderTime();
		HRESULT ShouldDrawSampleNow(IMediaSample *sample, REFERENCE_TIME *start, REFERENCE_TIME *stop);

	private:

		Frame *ProcessFrame(int frame, byte const *data);

		BITMAPINFOHEADER	bmih;
		double				frameReferenceTime;
		HANDLE				frameReleased;
		int					maxFrameQueueSize;
		FrameQueue			frameCache;
		FrameQueue			readyFrames;
		volatile bool		aborted;

	};

	//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
	HRESULT AddToRot(IUnknown *pUnkGraph, DWORD *pdwRegister)
	{
		const size_t STRING_LENGTH = 256;
		DXPtr<IMoniker> pMoniker;
		DXPtr<IRunningObjectTable> pROT;
		WCHAR wsz[STRING_LENGTH];
		DX(GetRunningObjectTable(0, &pROT));
		StringCchPrintfW(wsz, STRING_LENGTH, L"FilterGraph %08x pid %08x", (DWORD_PTR)pUnkGraph, GetCurrentProcessId());
		DX(CreateItemMoniker(L"!", wsz, &pMoniker));
		DX(pROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, pUnkGraph, pMoniker, pdwRegister));
		return S_OK;
	}
#endif

	//////////////////////////////////////////////////////////////////////

	bool Frame::operator == (int frameNumber) const
	{
		return frame == frameNumber;
	}

	//////////////////////////////////////////////////////////////////////

	Frame::Frame(int frameNumber, size_t bufferSize)
	{
		frame = frameNumber;
		mem.reset(new byte[bufferSize]);
	}

	//////////////////////////////////////////////////////////////////////

	Frame::~Frame()
	{
	}

	//////////////////////////////////////////////////////////////////////

	Frame *Sampler::ProcessFrame(int frame, byte const *data)
	{
		int w = bmih.biWidth;
		int h = bmih.biHeight;

		// get a frame from the cache
		Frame *f = frameCache.Pop();
		if(f == null)
		{
			// or make a new one
			f = new Frame(frame, w * sizeof(uint32) * h);
		}
		else
		{
			f->frame = frame;
		}

		// convert from 24bpp to 32bpp
		int rowPitch = DIBWIDTHBYTES(bmih) / sizeof(uint32);
		uint32 *dstRow = (uint32 *)f->mem.get();
		uint32 const *srcRow = (uint32 *)data;
		for(int y = 0; y < h; ++y)
		{
			uint32 *dst = dstRow;
			uint32 const *src = (uint32 *)srcRow;
			for(int x = 0; x < w; x += 4, src += 3)
			{
				uint32 a = src[0];
				uint32 b = src[1];
				uint32 c = src[2];
				dst[x + 0] = a;
				dst[x + 1] = (a >> 24) | (b << 8);
				dst[x + 2] = (b >> 16) | (c << 16);
				dst[x + 3] = (c >> 8);
			}
			srcRow += rowPitch;
			dstRow += w;
		}
		return f;
	}

	//////////////////////////////////////////////////////////////////////

	Sampler::Sampler(int maxFramesToBuffer, IUnknown* unk, HRESULT *hr)
		: CBaseVideoRenderer(__uuidof(CLSID_Sampler), NAME("Frame Sampler"), unk, hr)
		, maxFrameQueueSize(maxFramesToBuffer)
		, aborted(false)
	{
		bmih = { 0 };
		frameReleased = CreateEvent(NULL, false, false, TEXT("Frame Released Event"));
	};

	//////////////////////////////////////////////////////////////////////

	Sampler::~Sampler()
	{
		Abort();
	}

	//////////////////////////////////////////////////////////////////////

	void Sampler::Abort()
	{
		aborted = true;
		Flush();
	}

	//////////////////////////////////////////////////////////////////////

	void Sampler::FlushFrames()
	{
		while(!readyFrames.IsEmpty())
		{
			delete readyFrames.Pop();
		}
	}

	//////////////////////////////////////////////////////////////////////

	void Sampler::FlushCache()
	{
		while(!frameCache.IsEmpty())
		{
			delete frameCache.Pop();
		}
	}

	//////////////////////////////////////////////////////////////////////

	int Sampler::Width() const
	{
		return bmih.biWidth;
	}

	//////////////////////////////////////////////////////////////////////

	int Sampler::Height() const
	{
		return bmih.biHeight;
	}

	//////////////////////////////////////////////////////////////////////
	// How many frames are queued up

	int Sampler::FramesWaiting()
	{
		return (int)(readyFrames.Length());
	}

	//////////////////////////////////////////////////////////////////////
	// Client wants to eat a frame

	Frame *Sampler::GetFrame()
	{
		return readyFrames.Pop();
		SetEvent(frameReleased);
	}

	//////////////////////////////////////////////////////////////////////
	// Client wants a specific frame

	Frame *Sampler::GetFrame(int frameNumber)
	{
		Frame *f = readyFrames.Find(frameNumber);
		if(f != null)
		{
			readyFrames.Remove(f);
			SetEvent(frameReleased);
		}
		return f;
	}

	//////////////////////////////////////////////////////////////////////
	// frame has been eaten by the client, put it back in the cache

	void Sampler::ReleaseFrame(Frame *f)
	{
		frameCache.Push(f);
	}

	//////////////////////////////////////////////////////////////////////

	void Sampler::Flush()
	{
		FlushCache();
		FlushFrames();
	}

	//////////////////////////////////////////////////////////////////////
	// Get the movie details (fps & dimensions)

	HRESULT Sampler::CheckMediaType(const CMediaType *media)
	{
		VIDEOINFO *vi = (VIDEOINFO *)media->Format();
		if(vi != null && *media->Subtype() == MEDIASUBTYPE_RGB24)
		{
			frameReferenceTime = (double)vi->AvgTimePerFrame;
			bmih = vi->bmiHeader;
			return S_OK;
		}
		return E_FAIL;
	}

	//////////////////////////////////////////////////////////////////////
	// Add the frame to the readyFrames queue

	HRESULT Sampler::DoRenderSample(IMediaSample *sample)
	{
		int64 frameStart;
		int64 frameEnd;
		DX(sample->GetTime(&frameStart, &frameEnd));

		byte *data;
		DX(sample->GetPointer(&data));

		int frameNumber = (int)(frameStart / frameReferenceTime);	// these are counted up from 0 at the last seek point (so if you seek to frame 100, it will report frame 100 as 0)

		//TRACE("Got frame %d\n", frameNumber);

		readyFrames.Push(ProcessFrame(frameNumber, data));

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////
	// Block until the readyFrames Queue hasn't got too many in it

	HRESULT Sampler::WaitForRenderTime()
	{
		HRESULT hr = CBaseRenderer::WaitForRenderTime();
		if(hr == VFW_E_STATE_CHANGED)
		{
			return S_OK;
		}
		if(FAILED(hr))
		{
			return hr;
		}
		while(readyFrames.Length() >= (size_t)maxFrameQueueSize && !aborted)
		{
			WaitForSingleObject(frameReleased, 1);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////
	// Return:
	//	S_OK: no frame drop, no synchronization
	//	S_FALSE: no frame drop, enable synchronisation with *start
	//	Error code: Don't draw this frame

	HRESULT Sampler::ShouldDrawSampleNow(IMediaSample *sample, REFERENCE_TIME *start, REFERENCE_TIME *stop)
	{
		return S_OK; // Sync/throttle happens in WaitForRenderTime() & clientside
	}

	//////////////////////////////////////////////////////////////////////

	Player::Player()
	{
	}

	//////////////////////////////////////////////////////////////////////

	Player::~Player()
	{
		Close();
	}

	//////////////////////////////////////////////////////////////////////

	HRESULT Player::Open(wchar const *filename, int maxFrameBuffer)
	{
		Close();
		if(filename == null || maxFrameBuffer < 1)
		{
			return E_INVALIDARG;
		}
		CoInitialize(null);
		DX(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&graph));
#ifdef _DEBUG
		DWORD rotRegister;
		AddToRot(graph, &rotRegister);
#endif
		DX(graph->QueryInterface(IID_IMediaControl, (void **)&mediaControl));
		DX(graph->QueryInterface(IID_IMediaSeeking, (void **)&seeker));
		HRESULT hr = S_OK;
		sampler = new Sampler(maxFrameBuffer, 0, &hr);
		if(SUCCEEDED(hr))
		{
			DX(graph->AddFilter((IBaseFilter *)sampler, L"Sampler"));
		}
		DX(graph->AddSourceFilter(filename, L"File Source", &videoSource));
		DX(videoSource->FindPin(L"Output", &videoOutputPin));
		DX(sampler->FindPin(L"In", &rendererPin));
		DX(graph->Connect(videoOutputPin, rendererPin));
		DX(seeker->GetCapabilities(&seekingCapabilites));
		DX(seeker->SetTimeFormat(&TIME_FORMAT_FRAME));
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////

	void Player::FlushBuffers()
	{
		sampler->FlushCache();
		sampler->FlushFrames();
	}

	//////////////////////////////////////////////////////////////////////

	HRESULT Player::Close()
	{
		//Trace("Player::Close\n");
		if(IsOpen())
		{
			sampler->Abort();
			DX(mediaControl->Stop());
			DX(sampler->ClearPendingSample());
			while(!IsStopped())
			{
				TRACE("Waiting for movie to Stop()\n");
				Sleep(16);
			}
			DX(rendererPin->Disconnect());
			DX(videoOutputPin->Disconnect());
			DX(graph->RemoveFilter(sampler));
			DX(graph->RemoveFilter(videoSource));
			sampler.Release();
			rendererPin.Release();
			videoOutputPin.Release();
			mediaControl.Release();
			videoSource.Release();
			seeker.Release();
			graph.Release();
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////

	int Player::FramesWaiting() const
	{
		return sampler->FramesWaiting();
	}

	//////////////////////////////////////////////////////////////////////

	Frame *Player::GetFrame()
	{
		return sampler->GetFrame();
	}

	//////////////////////////////////////////////////////////////////////

	Frame *Player::GetFrame(int frameNumber)
	{
		return sampler->GetFrame(frameNumber);
	}

	//////////////////////////////////////////////////////////////////////

	void Player::Flush()
	{
		sampler->Flush();
	}

	//////////////////////////////////////////////////////////////////////

	void Player::ReleaseFrame(Frame *f)
	{
		if(f != null)
		{
			sampler->ReleaseFrame(f);
		}
	}

	//////////////////////////////////////////////////////////////////////

	long Player::GetState() const
	{
		OAFilterState fs = -1;
		if(SUCCEEDED(mediaControl->GetState(0, &fs)))
		{
			return fs;
		}
		return -1;
	}

	//////////////////////////////////////////////////////////////////////

	bool Player::IsOpen() const
	{
		return graph != null && mediaControl != null && GetState() != -1;
	}

	//////////////////////////////////////////////////////////////////////

	bool Player::IsPaused() const
	{
		return GetState() == State_Paused;
	}

	//////////////////////////////////////////////////////////////////////

	bool Player::IsStopped() const
	{
		return GetState() == State_Stopped;
	}

	//////////////////////////////////////////////////////////////////////

	bool Player::IsRunning() const
	{
		return GetState() == State_Running;
	}

	//////////////////////////////////////////////////////////////////////

	int Player::Width() const
	{
		return sampler->Width();
	}

	//////////////////////////////////////////////////////////////////////

	int Player::Height() const
	{
		return sampler->Height();
	}

	//////////////////////////////////////////////////////////////////////

	HRESULT Player::Pause()
	{
		if(mediaControl == null)
		{
			return E_NOT_VALID_STATE;
		}
		DX(mediaControl->Pause());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////

	HRESULT Player::Play()
	{
		if(mediaControl == null)
		{
			return E_NOT_VALID_STATE;
		}
		DX(mediaControl->Run());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////

	HRESULT Player::Seek(uint frame)
	{
		if((seekingCapabilites & AM_SEEKING_AbsolutePositioning) == 0)
		{
			return E_FAIL;
		}
		if(seeker == null)
		{
			return E_NOT_VALID_STATE;
		}
		int64 f = (int64)frame;
		DX(seeker->SetPositions(&f, AM_SEEKING_AbsolutePositioning, null, AM_SEEKING_NoPositioning));
		TRACE("Paused!\n");
		sampler->Flush();
		TRACE("Flushed!\n");
		return S_OK;
	}

} // namespace Movie
