//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

template<typename T> class Queue
{
public:

	//////////////////////////////////////////////////////////////////////

	Queue()
	{
	}

	//////////////////////////////////////////////////////////////////////

	~Queue()
	{
	}

	//////////////////////////////////////////////////////////////////////

	bool IsEmpty()
	{
		Lock _(critSec);
		return items.empty();
	}

	//////////////////////////////////////////////////////////////////////

	void Push(T *item)
	{
		Lock _(critSec);
		items.push_back(item);
	}

	//////////////////////////////////////////////////////////////////////

	size_t Length()
	{
		Lock _(critSec);
		return items.size();
	}

	//////////////////////////////////////////////////////////////////////

	T *Remove(T *p)
	{
		Lock _(critSec);
		items.remove(p);
		return p;
	}

	//////////////////////////////////////////////////////////////////////

	T *Pop()
	{
		Lock _(critSec);
		if(items.empty())
		{ 
			return null;
		}
		T *f = items.front();
		items.pop_front();
		return f;
	}

	//////////////////////////////////////////////////////////////////////

	T *Find(std::function<bool(T *)> comp)
	{
		Lock _(critSec);
		auto p = std::find_if(items.begin(), items.end(), comp);
		return (p == items.end()) ? null : *p;
	}

	//////////////////////////////////////////////////////////////////////

protected:

	CriticalSection		critSec;
	std::list<T *>		items;

};
