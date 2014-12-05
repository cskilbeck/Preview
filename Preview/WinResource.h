//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct WinResource
{
	WinResource(DWORD resourceid);
	~WinResource();

	bool IsValid() const
	{
		return data != null;
	}

	operator void const *() const
	{
		return data;
	}

	size_t Size() const
	{
		return size;
	}

private:

	void *		data;
	size_t		size;
};