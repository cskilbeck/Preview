//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Task : list_node<Task>
{
	virtual bool Execute(void *context) = 0;
};

//////////////////////////////////////////////////////////////////////

class TaskManager
{
public:

	TaskManager(int maxResults);
	~TaskManager();

	void SetContext(void *ctx);
	void Start();
	void Suspend();
	void Resume();
	void Terminate();
		
	void AddRequest(Task *task);
	template<typename T> Task *FindResult(T const &c);
	Task *RemoveResult(Task *result);

//private:

	uint32 Main();

	Queue<Task>	mRequests;
	Queue<Task>	mResults;
	HANDLE		mRequestAddedEvent;
	HANDLE		mResultRemovedEvent;
	Thread		thread;
	int			resultLimit;
	void *		context;
};
