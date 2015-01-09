//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

template<typename T, list_node<T> T::*NODE = null> class Queue
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
		return items.pop_front();
	}

	//////////////////////////////////////////////////////////////////////

	template <typename findable_t, typename task_t> task_t *Find(findable_t const &f)
	{
		Lock _(critSec);
		for(auto p = items.head(); p != items.done(); p = items.next(p))
		{
			task_t *t = (task_t *)p;
			if(*t == f)
			{
				return t;
			}
		}
		return nullptr;
	}

	//////////////////////////////////////////////////////////////////////

protected:

	CriticalSection			critSec;
	linked_list<T, NODE>	items;

};
