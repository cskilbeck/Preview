//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

class CriticalSection
{
public:
	CriticalSection()
	{
		InitializeCriticalSection(&cs);
	}

	~CriticalSection()
	{
		DeleteCriticalSection(&cs);
	}

	void Enter()
	{
		EnterCriticalSection(&cs);
	}

	void Leave()
	{
		LeaveCriticalSection(&cs);
	}

	operator CRITICAL_SECTION *()
	{
		return &cs;
	}

private:
	CRITICAL_SECTION cs;
	friend class Lock;
};

//////////////////////////////////////////////////////////////////////

class Lock
{
public:
	Lock(CRITICAL_SECTION &section)
		: cs(section)
	{
		EnterCriticalSection(&cs);
	}

	Lock(CriticalSection &section)
		: cs(section.cs)
	{
		EnterCriticalSection(&cs);
	}

	~Lock()
	{
		LeaveCriticalSection(&cs);
	}

private:
	CRITICAL_SECTION &cs;
};
