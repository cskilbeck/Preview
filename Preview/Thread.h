//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

class Thread
{
public:

	//////////////////////////////////////////////////////////////////////

	Thread()
		: threadProc(nullptr)
		, handle(INVALID_HANDLE_VALUE)
	{
	}

	//////////////////////////////////////////////////////////////////////

	bool IsValid() const
	{
		return handle != INVALID_HANDLE_VALUE;
	}

	//////////////////////////////////////////////////////////////////////

	bool IsComplete()
	{
		return (IsValid() && GetExitCodeThread(handle, &retVal)) ? retVal != STILL_ACTIVE : false;
	}

	//////////////////////////////////////////////////////////////////////

	void Start(std::function<DWORD()> func)
	{
		Stop();
		threadProc = func;
		handle = CreateThread(NULL, 0, ThreadKickShim, this, CREATE_SUSPENDED, NULL);
		if(IsValid())
		{
			Resume();
		}
	}

	//////////////////////////////////////////////////////////////////////

	void Suspend()
	{
		SuspendThread(handle);
	}

	//////////////////////////////////////////////////////////////////////

	void Resume()
	{
		ResumeThread(handle);
	}

	//////////////////////////////////////////////////////////////////////

	void Stop()
	{
		if(IsValid())
		{
			TerminateThread(handle, 0);
			CloseHandle(handle);
			handle = INVALID_HANDLE_VALUE;
		}
	}

	//////////////////////////////////////////////////////////////////////

	DWORD GetReturnValue() const
	{
		return retVal;
	}

	//////////////////////////////////////////////////////////////////////

private:

	static DWORD __stdcall ThreadKickShim(LPVOID t)
	{
		return reinterpret_cast<Thread *>(t)->threadProc();
	}

	//////////////////////////////////////////////////////////////////////

	std::function<DWORD()>	threadProc;
	HANDLE					handle;
	DWORD					retVal;
};