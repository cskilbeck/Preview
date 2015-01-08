//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////

TaskManager::TaskManager(int maxResults)
	: resultLimit(maxResults)
	, context(null)
{
	mRequestAddedEvent = CreateEvent(null, false, false, null);
	mResultRemovedEvent = CreateEvent(null, false, false, null);
}

//////////////////////////////////////////////////////////////////////

TaskManager::~TaskManager()
{
	thread.Stop();

	CloseHandle(mRequestAddedEvent);
	mRequestAddedEvent = INVALID_HANDLE_VALUE;

	CloseHandle(mResultRemovedEvent);
	mResultRemovedEvent = INVALID_HANDLE_VALUE;
}

//////////////////////////////////////////////////////////////////////
// Thread Side
//////////////////////////////////////////////////////////////////////

uint32 TaskManager::Main()
{
	TRACE("TaskManager starting...\n");
	while(true)
	{
		while(mRequests.IsEmpty())
		{
			TRACE("Waiting for a task\n");
			WaitForSingleObject(mRequestAddedEvent, INFINITE);
		}
		Task *t = mRequests.Pop();
		if(t != null)
		{
			TRACE("Executing a task\n");
			t->Execute(context);
			TRACE("Task complete\n");
			while(mResults.Length() >= resultLimit)
			{
				TRACE("Result set full, waiting for the client to take one away\n");
				WaitForSingleObject(mResultRemovedEvent, INFINITE);
			}
			mResults.Push(t);
		}
	}
	return 1;
}

//////////////////////////////////////////////////////////////////////
// Client Side
//////////////////////////////////////////////////////////////////////

void TaskManager::SetContext(void *ctx)
{
	context = ctx;
}

//////////////////////////////////////////////////////////////////////

void TaskManager::Start()
{
	thread.Start([this]
	{
		return Main();
	});
}

//////////////////////////////////////////////////////////////////////

void TaskManager::Suspend()
{
	thread.Suspend();
}

//////////////////////////////////////////////////////////////////////

void TaskManager::Resume()
{
	thread.Resume();
}

//////////////////////////////////////////////////////////////////////

void TaskManager::Terminate()
{
	thread.Stop();
}

//////////////////////////////////////////////////////////////////////

void TaskManager::AddRequest(Task *t)
{
	mRequests.Push(t);
	MemoryBarrier();	// need this? probly not but it can't hurt...
	SetEvent(mRequestAddedEvent);
}

//////////////////////////////////////////////////////////////////////

template<typename T> Task *TaskManager::FindResult(T const &c)
{
	return mResults.Find(c);
}

//////////////////////////////////////////////////////////////////////

Task *TaskManager::RemoveResult(Task *result)
{
	Task *r = mResults.Remove(result);
	MemoryBarrier();
	SetEvent(mResultRemovedEvent);
	return r;
}
