//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

namespace Movie
{
	//////////////////////////////////////////////////////////////////////

	class Player
	{
	public:

		//////////////////////////////////////////////////////////////////////
		// a frame of the movie

		struct Frame
		{
			Size2D dimensions;		// size in pixels
			int frame;				// frame number
			byte *mem;				// 32bpp xRGB data (alpha channel will be garbage, not all 00 or FF)

			list_node<Frame> node;

			bool operator == (int frameNumber) const
			{
				return frame == frameNumber;
			}

		private:

			// for putting in Queue<>
			// only friends can construct
			Frame(Size2D const &size, int frameNumber)
			{
				dimensions = size;
				frame = frameNumber;
				size_t bufferLength = size.cx * sizeof(uint32) * size.cy;
				mem = new byte[bufferLength];
			}

			~Frame()
			{
				delete[] mem;
			}

			// allow Player::Sampler to access ctor
			friend class Player;
			friend class Sampler;
		};

		using FrameQueue = Queue < Frame, &Frame::node >;
		using FrameList = linked_list < Frame, &Frame::node >;

		//////////////////////////////////////////////////////////////////////

	private:

		struct __declspec(uuid("{71771540-2017-11cf-ae26-0020afd79767}")) CLSID_Sampler;

		//////////////////////////////////////////////////////////////////////
		// class Sampler - private to Player
		// ???? - Merge this with Player!?

		class Sampler: public CBaseVideoRenderer
		{
			BITMAPINFOHEADER	bmih;
			double				frameReferenceTime;
			HANDLE				frameReleased;
			int					maxFrameQueueSize;
			FrameQueue			frameCache;
			FrameQueue			readyFrames;
			volatile bool		aborted;

			//////////////////////////////////////////////////////////////////////

			Frame *ProcessFrame(int frame, byte const *data)
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

			//////////////////////////////////////////////////////////////////////

		public:

			Sampler(int maxFramesToBuffer, IUnknown* unk, HRESULT *hr)
				: CBaseVideoRenderer(__uuidof(CLSID_Sampler), NAME("Frame Sampler"), unk, hr)
				, maxFrameQueueSize(maxFramesToBuffer)
				, aborted(false)
			{
				bmih = { 0 };
				ZeroMemory(&bmih, sizeof(bmih));
				frameReleased = CreateEvent(NULL, false, false, TEXT("Frame Released Event"));
			};

			//////////////////////////////////////////////////////////////////////

			~Sampler()
			{
				FlushCache();
				FlushFrames();
			}

			//////////////////////////////////////////////////////////////////////

			void FlushFrames()
			{
				while(!readyFrames.IsEmpty())
				{
					delete readyFrames.Pop();
				}
			}

			//////////////////////////////////////////////////////////////////////

			void Abort()
			{
				aborted = true;
			}

			//////////////////////////////////////////////////////////////////////

			void FlushCache()
			{
				while(!frameCache.IsEmpty())
				{
					delete frameCache.Pop();
				}
			}

			//////////////////////////////////////////////////////////////////////

			int Width() const
			{
				return bmih.biWidth;
			}

			//////////////////////////////////////////////////////////////////////

			int Height() const
			{
				return bmih.biHeight;
			}

			//////////////////////////////////////////////////////////////////////
			// How many frames are queued up

			int FramesWaiting()
			{
				return (int)(readyFrames.Length());
			}

			//////////////////////////////////////////////////////////////////////
			// Client wants to eat a frame

			Frame *GetFrame()
			{
				return readyFrames.Pop();
				SetEvent(frameReleased);
			}

			//////////////////////////////////////////////////////////////////////
			// Client wants a specific frame

			Frame *GetFrame(int frameNumber)
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

			void ReleaseFrame(Frame *f)
			{
				frameCache.Push(f);
			}

			//////////////////////////////////////////////////////////////////////

			void Flush()
			{
				while(!readyFrames.IsEmpty())
				{
					readyFrames.Pop();
				}
				SetEvent(frameReleased);
			}

			//////////////////////////////////////////////////////////////////////
			// Get the movie details (fps & dimensions)

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
			// Add the frame to the readyFrames queue

			HRESULT DoRenderSample(IMediaSample *sample)
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

			HRESULT WaitForRenderTime()
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

			HRESULT ShouldDrawSampleNow(IMediaSample *sample, REFERENCE_TIME *start, REFERENCE_TIME *stop)
			{
				return S_OK;		// Sync happens by throttling in WaitForRenderTime() & clientside
				//return S_FALSE;
			}
		};

		//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
		static HRESULT AddToRot(IUnknown *pUnkGraph, DWORD *pdwRegister)
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

	public:

		//////////////////////////////////////////////////////////////////////

		Player()
		{
		}

		//////////////////////////////////////////////////////////////////////

		~Player()
		{
			Close();
		}

		//////////////////////////////////////////////////////////////////////

		HRESULT Open(wchar const *filename, int maxFrameBuffer = 3)
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

		void FlushBuffers()
		{
			sampler->FlushCache();
			sampler->FlushFrames();
		}

		//////////////////////////////////////////////////////////////////////

		HRESULT Close()
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

		int FramesWaiting() const
		{
			return sampler->FramesWaiting();
		}

		//////////////////////////////////////////////////////////////////////

		Frame *GetFrame()
		{
			return sampler->GetFrame();
		}

		//////////////////////////////////////////////////////////////////////

		Frame *GetFrame(int frameNumber)
		{
			return sampler->GetFrame(frameNumber);
		}

		//////////////////////////////////////////////////////////////////////

		void Flush()
		{
			sampler->Flush();
		}

		//////////////////////////////////////////////////////////////////////

		void ReleaseFrame(Frame *f)
		{
			if(f != null)
			{
				sampler->ReleaseFrame(f);
			}
		}

		//////////////////////////////////////////////////////////////////////

		bool IsOpen() const
		{
			return graph != null && GetState() != -1;
		}

		//////////////////////////////////////////////////////////////////////

		long GetState() const
		{
			OAFilterState fs = -1;
			if(SUCCEEDED(mediaControl->GetState(0, &fs)))
			{
				return fs;
			}
			return -1;
		}

		//////////////////////////////////////////////////////////////////////

		bool IsPaused() const
		{
			return GetState() == State_Paused;
		}

		//////////////////////////////////////////////////////////////////////

		bool IsStopped() const
		{
			return GetState() == State_Stopped;
		}

		//////////////////////////////////////////////////////////////////////

		bool IsRunning() const
		{
			return GetState() == State_Running;
		}

		//////////////////////////////////////////////////////////////////////

		int Width() const
		{
			return sampler->Width();
		}

		//////////////////////////////////////////////////////////////////////

		int Height() const
		{
			return sampler->Height();
		}

		//////////////////////////////////////////////////////////////////////

		HRESULT Pause()
		{
			if(mediaControl != null)
			{
				DX(mediaControl->Pause());
				return S_OK;
			}
			return E_NOT_VALID_STATE;
		}

		//////////////////////////////////////////////////////////////////////

		HRESULT Play()
		{
			if(mediaControl != null)
			{
				DX(mediaControl->Run());
				return S_OK;
			}
			return E_NOT_VALID_STATE;
		}

		//////////////////////////////////////////////////////////////////////

		HRESULT Seek(int64 frame)
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

		//////////////////////////////////////////////////////////////////////

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

