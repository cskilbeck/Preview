//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct __declspec(uuid("{71771540-2017-11cf-ae26-0020afd79767}")) CLSID_Sampler;

//////////////////////////////////////////////////////////////////////

HRESULT AddToRot(IUnknown *pUnkGraph, DWORD *pdwRegister);

//////////////////////////////////////////////////////////////////////

struct Sampler: public CBaseVideoRenderer
{
	using callback_t = std::function<void(BITMAPINFOHEADER *, byte *)>;

	BITMAPINFOHEADER	bmih;
	callback_t			callback;

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
			bmih = vi->bmiHeader;
			return S_OK;
		}
		return E_FAIL;
	}

	//////////////////////////////////////////////////////////////////////

	HRESULT DoRenderSample(IMediaSample *sample)
	{
		byte *data;
		DX(sample->GetPointer(&data));
		if(callback != null)
		{
			callback(&bmih, data);
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

	MoviePlayer() : sampler(null)
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
		DWORD rotRegister;
		DX(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&graph));
		AddToRot(graph, &rotRegister);

		DX(graph->QueryInterface(IID_IMediaControl, (void **)&mediaControl));
		DX(graph->QueryInterface(IID_IMediaSeeking, (void **)&seeker));

		HRESULT hr = S_OK;
		sampler = new Sampler(0, &hr);
		if(SUCCEEDED(hr))
		{
			DX(graph->AddFilter((IBaseFilter *)sampler, L"Sampler"));
		}
		return hr;
	}

	//////////////////////////////////////////////////////////////////////

	HRESULT PlayMovie(wchar const *filename)
	{
		long evCode = 0;
		DXPtr<IMediaEvent> pEvent;
		Init();
		DX(graph->QueryInterface(IID_IMediaEvent, (void **)&pEvent));
		DX(graph->RenderFile(filename, null));
		DX(mediaControl->Run());
		pEvent->WaitForCompletion(INFINITE, &evCode);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////

	HRESULT Open(wchar *filename, Sampler::callback_t callback)
	{
		sampler->callback = callback;
		DX(graph->AddSourceFilter(filename, L"File Source", &videoSource));
		DX(videoSource->FindPin(L"Output", &videoOutputPin));
		DX(sampler->FindPin(L"In", &rendererPin));
		DX(graph->Disconnect(videoOutputPin));
		DX(graph->Connect(videoOutputPin, rendererPin));
		DX(seeker->GetCapabilities(&seekingCapabilites));
		DX(seeker->SetTimeFormat(&TIME_FORMAT_FRAME));
		HRESULT hr = graph->Render(videoOutputPin);
		if(FAILED(hr) || hr != VFW_E_ALREADY_CONNECTED)	// Huh? Still works...
		{
			// Problem...
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////

	HRESULT Pause()
	{
		DX(mediaControl->Pause());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////

	HRESULT Play()
	{
		DX(mediaControl->Run());
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
};
