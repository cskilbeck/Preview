//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <time.h>
#include <time.inl>

//////////////////////////////////////////////////////////////////////

void TRACE(tchar const *strMsg, ...)
{
	tchar strBuffer[512];
	va_list args;
	va_start(args, strMsg);
	_vstprintf_s(strBuffer, strMsg, args);
	va_end(args);
	OutputDebugString(strBuffer);
}

//////////////////////////////////////////////////////////////////////

HRESULT LoadResource(uint32 resourceid, void **data, size_t *size)
{
	HRSRC myResource = ::FindResource(NULL, MAKEINTRESOURCE(resourceid), RT_RCDATA);
	if(myResource == null)
	{
		return ERROR_RESOURCE_DATA_NOT_FOUND;
	}

	HGLOBAL myResourceData = ::LoadResource(NULL, myResource);
	if(myResourceData == null)
	{
		return ERROR_RESOURCE_FAILED;
	}

	void *pMyBinaryData = ::LockResource(myResourceData);
	if(pMyBinaryData == null)
	{
		return ERROR_RESOURCE_NOT_AVAILABLE;
	}

	size_t s = (size_t)SizeofResource(GetModuleHandle(null), myResource);
	if(s == 0)
	{
		return ERROR_RESOURCE_FAILED;
	}

	if(size != null)
	{
		*size = s;
	}
	if(data != null)
	{
		*data = pMyBinaryData;
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

uint8 *LoadFile(tchar const *filename, size_t *size)
{
	uint8 *buf = null;
	FILE *f = null;

	if(_tfopen_s(&f, filename, TEXT("rb")) == 0)
	{
		fseek(f, 0, SEEK_END);
		uint32 len = ftell(f);
		fseek(f, 0, SEEK_SET);

		buf = new uint8[len + 2];

		if(buf != null)
		{
			size_t s = fread_s(buf, len, 1, len, f);

			if(s != len)
			{
				delete [] buf;
				buf = null;
			}
			else
			{
				*((tchar *)(((tchar *)buf) + len)) = (tchar)'\0';
				if(size != null)
				{
					*size = len;
				}
			}
		}

		fclose(f);
	}
	else
	{
		MessageBox(null, Format(TEXT("File not found: %s"), filename).c_str(), TEXT("LoadFile"), MB_ICONERROR);
	}
	return buf;
}

//////////////////////////////////////////////////////////////////////

tstring Format(tchar const *fmt, ...)
{
	tchar buffer[1024];
	va_list v;
	va_start(v, fmt);
	_vstprintf_s(buffer, fmt, v);
	return tstring(buffer);
}

//////////////////////////////////////////////////////////////////////

wstring WideStringFromTString(tstring const &str)
{
	#ifdef UNICODE
		return str;
	#else
		return WideStringFromString(str);
	#endif
}

//////////////////////////////////////////////////////////////////////

wstring WideStringFromString(string const &str)
{
	vector<wchar> temp;
	temp.resize(str.size() + 1);
	temp[0] = (wchar)0;
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), &temp[0], str.size());
	temp[str.size()] = 0;
	return wstring(&temp[0]);
}

//////////////////////////////////////////////////////////////////////

string StringFromWideString(wstring const &str)
{
	vector<char> temp;
	int bufSize = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), str.size() + 1, NULL, 0, NULL, FALSE);
	if(bufSize > 0)
	{
		temp.resize(bufSize);
		WideCharToMultiByte(CP_UTF8, 0, str.c_str(), str.size() + 1, &temp[0], bufSize, NULL, FALSE);
		return string(&temp[0]);
	}
	return string();
}

//////////////////////////////////////////////////////////////////////

tstring GetCurrentFolder()
{
	vector<tchar> s;
	DWORD l = GetCurrentDirectory(0, NULL);
	assert(l != 0);
	s.resize((size_t)l + 1);
	GetCurrentDirectory(l, &s[0]);
	s[l] = 0;
	return Format(L"%s", &s[0]);
}
