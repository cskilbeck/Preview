//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////

WinResource::WinResource(DWORD resourceid)
	: data(null)
	, size(0)
{
	HRSRC myResource = FindResource(NULL, MAKEINTRESOURCE(resourceid), RT_RCDATA);
	if(myResource != null)
	{
		HGLOBAL myResourceData = LoadResource(NULL, myResource);
		if(myResourceData != null)
		{
			data = LockResource(myResourceData);
			if(data != null)
			{
				size = (size_t)SizeofResource(GetModuleHandle(null), myResource);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////

WinResource::~WinResource()
{
	if(data != null)
	{
		FreeResource(data);
		data = null;
		size = 0;
	}
}
