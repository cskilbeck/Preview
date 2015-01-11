//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct __declspec(uuid("{71771540-2017-11cf-ae26-0020afd79767}")) CLSID_Sampler;

//////////////////////////////////////////////////////////////////////

HRESULT AddToRot(IUnknown *pUnkGraph, DWORD *pdwRegister);

//////////////////////////////////////////////////////////////////////

struct Sampler: public CBaseVideoRenderer
{
	using callback_t = std::function<void(BITMAPINFOHEADER *, int frame, byte *)>;

	BITMAPINFOHEADER	bmih;
	callback_t			callback;
	double				frameReferenceTime;

	//////////////////////////////////////////////////////////////////////

	Sampler(IUnknown* unk, HRESULT *hr)
		: CBaseVideoRenderer(__uuidof(CLSID_Sampler), NAME("Frame Sampler"), unk, hr)
	{
		bmih = { 0 };
		ZeroMemory(&bmih, sizeof(bmih));
	};

	//////////////////////////////////////////////////////////////////////

	HRESULT CheckMediaType(const CMediaType *media)
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

	HRESULT DoRenderSample(IMediaSample *sample)
	{
		byte *data;
		int64 frameStart;
		int64 frameEnd;
		DX(sample->GetTime(&frameStart, &frameEnd));
		DX(sample->GetPointer(&data));
		if(callback != null)
		{
			int frame = (int)(frameStart / frameReferenceTime);
			callback(&bmih, frame, data);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////

	HRESULT ShouldDrawSampleNow(IMediaSample *sample, REFERENCE_TIME *start, REFERENCE_TIME *stop)
	{
		return S_FALSE; // disable dropping of frames, enable synchronisation with *start
	}
};

//////////////////////////////////////////////////////////////////////

class MoviePlayer
{
public:

	//////////////////////////////////////////////////////////////////////

	MoviePlayer() : sampler(null), isPaused(false)
	{
		CoInitialize(null);
	}

	//////////////////////////////////////////////////////////////////////

	~MoviePlayer()
	{
		CoUninitialize();
	}

	//////////////////////////////////////////////////////////////////////

	HRESULT Init()
	{
		// create the graph manager
		DX(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&graph));

		// add it to the registered object table
		#ifdef _DEBUG
			DWORD rotRegister;
			AddToRot(graph, &rotRegister);
		#endif

		// media controller
		DX(graph->QueryInterface(IID_IMediaControl, (void **)&mediaControl));

		// seek controller
		DX(graph->QueryInterface(IID_IMediaSeeking, (void **)&seeker));

		// create the sampler
		HRESULT hr = S_OK;
		sampler = new Sampler(0, &hr);
		if(SUCCEEDED(hr))
		{
			// and add it to the graph
			DX(graph->AddFilter((IBaseFilter *)sampler, L"Sampler"));
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////

	HRESULT Open(wchar *filename, Sampler::callback_t callback)
	{
		sampler->callback = callback;

		// set the file source
		DX(graph->AddSourceFilter(filename, L"File Source", &videoSource));

		// find the output from the file source
		DX(videoSource->FindPin(L"Output", &videoOutputPin));

		// find the input to the sampler
		DX(sampler->FindPin(L"In", &rendererPin));

		// connect the file source output to the frame sampler input
		DX(graph->Connect(videoOutputPin, rendererPin));

		// get seek capabilities
		DX(seeker->GetCapabilities(&seekingCapabilites));

		// track by frames
		DX(seeker->SetTimeFormat(&TIME_FORMAT_FRAME));

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////

	bool IsPaused() const
	{
		return isPaused;
	}

	//////////////////////////////////////////////////////////////////////

	HRESULT Pause()
	{
		DX(mediaControl->Pause());
		isPaused = true;
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////

	HRESULT Play()
	{
		DX(mediaControl->Run());
		isPaused = false;
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////

	HRESULT Seek(int64 frame)
	{
		if(seekingCapabilites & AM_SEEKING_AbsolutePositioning)
		{
			DX(seeker->SetPositions(&frame, AM_SEEKING_AbsolutePositioning, null, AM_SEEKING_NoPositioning));
			return S_OK;
		}
		return E_FAIL;
	}

	//////////////////////////////////////////////////////////////////////

	DXPtr<IGraphBuilder>	graph;
	DXPtr<IMediaControl>	mediaControl;
	DXPtr<IMediaSeeking>	seeker;
	DXPtr<IPin>				rendererPin;
	DXPtr<IBaseFilter>		videoSource;
	DXPtr<IPin>				videoOutputPin;
	Sampler *				sampler;
	DWORD					seekingCapabilites;
	bool					isPaused;
};
