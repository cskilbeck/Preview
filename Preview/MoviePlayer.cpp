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
		BITMAPINFOHEADER	bmih;
		double				frameReferenceTime;
		HANDLE				frameReleased;
		int					maxFrameQueueSize;
		FrameQueue			frameCache;
		FrameQueue			readyFrames;
		volatile bool		aborted;

		Frame *ProcessFrame(int frame, byte const *data);

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
	};

	//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
	HRESULT AddToRot(IUnknown *pUnkGraph, DWORD *pdwRegister)
	{
		IMoniker * pMoniker = NULL;
		IRunningObjectTable *pROT = NULL;

		if(FAILED(GetRunningObjectTable(0, &pROT)))
		{
			return E_FAIL;
		}

		const size_t STRING_LENGTH = 256;

		WCHAR wsz[STRING_LENGTH];
		StringCchPrintfW(wsz, STRING_LENGTH, L"FilterGraph %08x pid %08x", (DWORD_PTR)pUnkGraph, GetCurrentProcessId());

		HRESULT hr = CreateItemMoniker(L"!", wsz, &pMoniker);
		if(SUCCEEDED(hr))
		{
			hr = pROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, pUnkGraph,
								pMoniker, pdwRegister);
			pMoniker->Release();
		}
		pROT->Release();

		return hr;
	}
#endif

	bool Frame::operator == (int frameNumber) const
	{
		return frame == frameNumber;
	}

	Frame::Frame(Size2D const &size, int frameNumber)
	{
		dimensions = size;
		frame = frameNumber;
		size_t bufferLength = size.cx * sizeof(uint32) * size.cy;
		mem = new byte[bufferLength];
	}

	Frame::~Frame()
	{
		delete[] mem;
	}

	Frame *Sampler::ProcessFrame(int frame, byte const *data)
	{
		Size2D size(bmih.biWidth, bmih.biHeight);

		// get a frame from the cache
		Frame *f = frameCache.Pop();
		if(f == null)
		{
			// or make a new one
			f = new Frame(size, frame);
		}
		else
		{
			f->frame = frame;
		}

		// convert from 24bpp to 32bpp
		int rowPitch = DIBWIDTHBYTES(bmih) / sizeof(uint32);
		uint32 *dstRow = (uint32 *)f->mem;
		uint32 const *srcRow = (uint32 *)data;
		for(int y = 0; y < bmih.biHeight; ++y)
		{
			uint32 *dst = dstRow;
			uint32 const *src = (uint32 *)srcRow;
			for(int x = 0; x < bmih.biWidth; x += 4, src += 3)
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
			dstRow += bmih.biWidth;
		}
		return f;
	}

	Sampler::Sampler(int maxFramesToBuffer, IUnknown* unk, HRESULT *hr)
		: CBaseVideoRenderer(__uuidof(CLSID_Sampler), NAME("Frame Sampler"), unk, hr)
		, maxFrameQueueSize(maxFramesToBuffer)
		, aborted(false)
	{
		bmih = { 0 };
		ZeroMemory(&bmih, sizeof(bmih));
		frameReleased = CreateEvent(NULL, false, false, TEXT("Frame Released Event"));
	};

	//////////////////////////////////////////////////////////////////////

	Sampler::~Sampler()
	{
		FlushCache();
		FlushFrames();
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

	void Sampler::Abort()
	{
		aborted = true;
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
		while(!readyFrames.IsEmpty())
		{
			readyFrames.Pop();
		}
		SetEvent(frameReleased);
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

		readyFrames.Push(ProcessFrame((int)(frameStart / frameReferenceTime), data));

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////
	// Block until the readyFrames Queue hasn't got too many in it

	HRESULT Sampler::WaitForRenderTime()
	{
		DX(CBaseRenderer::WaitForRenderTime());
		while(readyFrames.Length() >= (size_t)maxFrameQueueSize && !aborted)
		{
			WaitForSingleObject(frameReleased, 100);
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
			CoUninitialize();
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

	bool Player::IsOpen() const
	{
		return graph != null && GetState() != -1;
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
		if(mediaControl != null)
		{
			DX(mediaControl->Pause());
			return S_OK;
		}
		return E_NOT_VALID_STATE;
	}

	//////////////////////////////////////////////////////////////////////

	HRESULT Player::Play()
	{
		if(mediaControl != null)
		{
			DX(mediaControl->Run());
			return S_OK;
		}
		return E_NOT_VALID_STATE;
	}

	//////////////////////////////////////////////////////////////////////

	HRESULT Player::Seek(int64 frame)
	{
		if(seekingCapabilites & AM_SEEKING_AbsolutePositioning)
		{
			if(seeker != null)
			{
				sampler->FlushCache();
				sampler->FlushFrames();
				DX(seeker->SetPositions(&frame, AM_SEEKING_AbsolutePositioning, null, AM_SEEKING_NoPositioning));
				return S_OK;
			}
			return E_NOT_VALID_STATE;
		}
		return E_FAIL;
	}

} // namespace Movie
