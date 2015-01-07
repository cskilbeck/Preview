//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <time.h>
#include <time.inl>

//////////////////////////////////////////////////////////////////////

#if defined(_DEBUG)
void TRACE(wchar const *strMsg, ...)
{
	wchar strBuffer[512];
	va_list args;
	va_start(args, strMsg);
	_vsnwprintf_s(strBuffer, ARRAYSIZE(strBuffer), strMsg, args);
	va_end(args);
	OutputDebugStringW(strBuffer);
}

//////////////////////////////////////////////////////////////////////

void TRACE(char const *strMsg, ...)
{
	char strBuffer[512];
	va_list args;
	va_start(args, strMsg);
	_vsnprintf_s(strBuffer, ARRAYSIZE(strBuffer), strMsg, args);
	va_end(args);
	OutputDebugStringA(strBuffer);
}
#endif

//////////////////////////////////////////////////////////////////////

HRESULT LoadResource(uint32 resourceid, void **data, size_t *size)
{
	if(size != null) *size = 0;
	if(data != null) *data = null;
	HRSRC myResource = ::FindResource(NULL, MAKEINTRESOURCE(resourceid), RT_RCDATA);
	if(myResource == null) return ERROR_RESOURCE_DATA_NOT_FOUND;

	HGLOBAL myResourceData = ::LoadResource(NULL, myResource);
	if(myResourceData == null) return ERROR_RESOURCE_FAILED;

	void *pMyBinaryData = ::LockResource(myResourceData);
	if(pMyBinaryData == null)
	{
		FreeResource(myResourceData);
		return ERROR_RESOURCE_NOT_AVAILABLE;
	}

	size_t s = (size_t)SizeofResource(GetModuleHandle(null), myResource);
	if(s == 0)
	{
		FreeResource(myResourceData);
		return ERROR_RESOURCE_FAILED;
	}
	if(size != null) *size = s;
	if(data != null) *data = pMyBinaryData;
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

wstring Format(wchar const *fmt, ...)
{
	wchar buffer[1024];
	va_list v;
	va_start(v, fmt);
	_vsnwprintf_s(buffer, ARRAYSIZE(buffer), fmt, v);
	return wstring(buffer);
}

//////////////////////////////////////////////////////////////////////

string Format(char const *fmt, ...)
{
	char buffer[1024];
	va_list v;
	va_start(v, fmt);
	_vsnprintf_s(buffer, ARRAYSIZE(buffer), fmt, v);
	return string(buffer);
}

//////////////////////////////////////////////////////////////////////

tstring GetLastErrorText()
{
	tchar *buf;
	uint32 retSize = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
									FORMAT_MESSAGE_FROM_SYSTEM,
									NULL,
									GetLastError(),
									LANG_NEUTRAL,
									(LPTSTR)&buf,
									0,
									NULL);

	tstring rc;
	if(retSize == 0 || buf == null)
	{
		rc = tstring(TEXT("Unknown error"));
	}
	else
	{
		rc = tstring(buf);
		LocalFree((HLOCAL)buf);
	}
	return rc;
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

tstring TStringFromWideString(wstring const &str)
{
#ifdef UNICODE
	return str;
#else
	return StringFromWideString(str);
#endif
}

//////////////////////////////////////////////////////////////////////

string StringFromTString(tstring const &str)
{
#ifdef UNICODE
	return StringFromWideString(str);
#else
	return str;
#endif
}

//////////////////////////////////////////////////////////////////////

tstring TStringFromString(string const &str)
{
#ifdef UNICODE
	return WideStringFromString(str);
#else
	return str;
#endif
}

//////////////////////////////////////////////////////////////////////

wstring WideStringFromString(string const &str)
{
	vector<wchar> temp;
	temp.resize(str.size() + 1);
	temp[0] = (wchar)0;
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), &temp[0], (int)str.size());
	temp[str.size()] = 0;
	return wstring(&temp[0]);
}

//////////////////////////////////////////////////////////////////////

string StringFromWideString(wstring const &str)
{
	vector<char> temp;
	int bufSize = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), (int)str.size() + 1, NULL, 0, NULL, FALSE);
	if(bufSize > 0)
	{
		temp.resize(bufSize);
		WideCharToMultiByte(CP_UTF8, 0, str.c_str(), (int)str.size() + 1, &temp[0], bufSize, NULL, FALSE);
		return string(&temp[0]);
	}
	return string();
}

//////////////////////////////////////////////////////////////////////

tstring GetCurrentFolder()
{
	uint32 l = GetCurrentDirectory(0, null);
	if(l != 0)
	{
		std::unique_ptr<tchar> s(new tchar[l]);
		GetCurrentDirectory(l, s.get());
		return tstring(s.get());
	}
	TRACE(TEXT("Error getting current folder: %s"), GetLastErrorText());
	return tstring();
}

//////////////////////////////////////////////////////////////////////

static struct PathComponents
{
	tchar drive[MAX_PATH];
	tchar dir[MAX_PATH];
	tchar name[MAX_PATH];
	tchar ext[MAX_PATH];
} pc;

//////////////////////////////////////////////////////////////////////

PathComponents SplitPath(tchar const *path, PathComponents &pc)
{
	_tsplitpath_s(path, pc.drive, pc.dir, pc.name, pc.ext);
	return pc;
}

//////////////////////////////////////////////////////////////////////

tstring GetDrive(tchar const *path)
{
	return SplitPath(path, pc).drive;
}

//////////////////////////////////////////////////////////////////////

tstring GetPath(tchar const *path)
{
	return SplitPath(path, pc).dir;
}

//////////////////////////////////////////////////////////////////////

tstring GetFilename(tchar const *path)
{
	return SplitPath(path, pc).name;
}

//////////////////////////////////////////////////////////////////////

tstring GetExtension(tchar const *path)
{
	return SplitPath(path, pc).ext;
}

//////////////////////////////////////////////////////////////////////

tstring SetExtension(tchar const *path, tchar const *ext)
{
	SplitPath(path, pc);
	return tstring(pc.drive) + pc.dir + pc.name + ext;
}
