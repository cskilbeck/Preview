//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Task
{
	list_node<Task> node;
	virtual bool Execute() = 0;
};

//////////////////////////////////////////////////////////////////////

template <typename task_t> class TaskManager
{
public:

	TaskManager(int maxResults)
		: resultLimit(maxResults)
	{
		mRequestAddedEvent = CreateEvent(null, false, false, null);
		mResultRemovedEvent = CreateEvent(null, false, false, null);
	}

	~TaskManager()
	{
		thread.Stop();

		CloseHandle(mRequestAddedEvent);
		mRequestAddedEvent = INVALID_HANDLE_VALUE;

		CloseHandle(mResultRemovedEvent);
		mResultRemovedEvent = INVALID_HANDLE_VALUE;
	}

	void Start()
	{
		thread.Start([this]
		{
			return Main();
		});
	}

	//////////////////////////////////////////////////////////////////////

	void Suspend()
	{
		thread.Suspend();
	}

	//////////////////////////////////////////////////////////////////////

	void Resume()
	{
		thread.Resume();
	}

	//////////////////////////////////////////////////////////////////////

	void Terminate()
	{
		thread.Stop();
	}

	//////////////////////////////////////////////////////////////////////

	void AddRequest(task_t *t)
	{
		mRequests.Push(t);
		SetEvent(mRequestAddedEvent);
	}

	//////////////////////////////////////////////////////////////////////

	template<typename findable_t> inline task_t *FindResult(findable_t const &c)
	{
		return (task_t *)mResults.Find<findable_t, task_t>(c);
	}

	//////////////////////////////////////////////////////////////////////

	task_t *RemoveResult(task_t *result)
	{
		task_t *r = (task_t *)mResults.Remove(result);
		SetEvent(mResultRemovedEvent);
		return r;
	}

private:

	uint32 Main()
	{
		TRACE("TaskManager starting...\n");
		while(true)
		{
			while(mRequests.IsEmpty())
			{
				TRACE("Waiting for a task\n");
				WaitForSingleObject(mRequestAddedEvent, INFINITE);
			}
			task_t *t = (task_t *)mRequests.Pop();
			if(t != null)
			{
				TRACE("Executing a task\n");
				t->Execute();
				TRACE("Task complete\n");
				while(mResults.Length() >= (size_t)resultLimit)
				{
					TRACE("Result set full, waiting for the client to take one away\n");
					WaitForSingleObject(mResultRemovedEvent, INFINITE);
				}
				mResults.Push(t);
			}
		}
		return 1;
	}
	
	Queue<Task, &Task::node>	mRequests;
	Queue<Task, &Task::node>	mResults;
	HANDLE						mRequestAddedEvent;
	HANDLE						mResultRemovedEvent;
	Thread						thread;
	int							resultLimit;
};
