//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <time.h>
#include <time.inl>

//////////////////////////////////////////////////////////////////////

void TRACE(char const *strMsg, ...)
{
	char strBuffer[512];
	va_list args;
	va_start(args, strMsg);
	vsprintf_s(strBuffer, 512, strMsg, args);
	va_end(args);
	OutputDebugStringA(strBuffer);
}

//////////////////////////////////////////////////////////////////////

void TRACE(wchar const *strMsg, ...)
{
	wchar strBuffer[512];
	va_list args;
	va_start(args, strMsg);
	_vsnwprintf_s(strBuffer, 512, strMsg, args);
	va_end(args);
	OutputDebugStringW(strBuffer);
}

//////////////////////////////////////////////////////////////////////

#if defined(DEBUG)

void DBG(int x, int y, char const *strMsg, ...)
{
	char strBuffer[512];
	va_list args;
	va_start(args, strMsg);
	_vsnprintf_s(strBuffer, 512, strMsg, args);
	va_end(args);
	extern Font *g_DebugFont;
	extern SpriteList *g_DebugSpriteList;
	g_DebugSpriteList->ResetTransform();
	g_DebugFont->DrawString(g_DebugSpriteList, strBuffer, Vec2((float)x, (float)y));
}

void DBG(int x, int y, wchar const *strMsg, ...)
{
	wchar strBuffer[512];
	va_list args;
	va_start(args, strMsg);
	_vsnwprintf_s(strBuffer, 512, strMsg, args);
	va_end(args);
	string s(StringFromWideString(wstring(strBuffer)));
	extern Font *g_DebugFont;
	extern SpriteList *g_DebugSpriteList;
	g_DebugSpriteList->ResetTransform();
	g_DebugFont->DrawString(g_DebugSpriteList, s.c_str(), Vec2((float)x, (float)y));
}

#endif

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

uint8 *LoadFile(char const *filename, size_t *size)
{
	uint8 *buf = null;
	FILE *f = null;

	if(fopen_s(&f, filename, "rb") == 0)
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
				*((wchar *)(((char *)buf) + len)) = L'\0';
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
		MessageBoxA(null, Format("File not found: %s", filename).c_str(), "LoadFile", MB_ICONERROR);
	}
	return buf;
}

//////////////////////////////////////////////////////////////////////

wstring Format(wchar const *fmt, ...)
{
	wchar buffer[512];

	va_list v;
	va_start(v, fmt);
	_vsnwprintf_s(buffer, 512, fmt, v);
	return wstring(buffer);
}

//////////////////////////////////////////////////////////////////////

string Format(char const *fmt, ...)
{
	char buffer[512];

	va_list v;
	va_start(v, fmt);
	_vsnprintf_s(buffer, 512, fmt, v);
	return string(buffer);
}

//////////////////////////////////////////////////////////////////////

tstring TFormat(tchar const *fmt, ...)
{
	tchar buffer[512];

	va_list v;
	va_start(v, fmt);
	#ifdef UNICODE
		_vsnwprintf_s(buffer, 512, fmt, v);
	#else
		_vsnprintf_s(buffer, 512, fmt, v);
	#endif
	return tstring(buffer);
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

static char const *sDateTimeFormat = "%Y-%m-%d %H:%M:%S";

string FormatTime(time_t t)
{
	char buffer[512];
	struct tm stm;
	buffer[0] = 0;
	if(gmtime_s(&stm, &t) == 0)
	{
		strftime(buffer, 512, sDateTimeFormat, &stm);
	}
	return string(buffer);
}

//////////////////////////////////////////////////////////////////////

time_t ParseTime(char const *t)
{
	struct tm stm = { 0 };
	extern char * strptime(const char *buf, const char *fmt, struct tm *tm);
	char *p = strptime(t, sDateTimeFormat, &stm);
	time_t time(0);
	if(p != null)
	{
		time = mktime(&stm);
	}
	return time;
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
	return Format(TEXT("%s"), &s[0]);
}

//////////////////////////////////////////////////////////////////////

void SetCurrentFolder(tchar const *relative)
{
	vector<tchar> s;
	s.resize(GetCurrentDirectory(0, null));
	GetCurrentDirectory(s.size(), &s[0]);
	s.resize(s.size() - 1);
	s.push_back(L'\\');
	for(; *relative; ++relative)
	{
		s.push_back(*relative);
	}
	s.push_back(0);
	tchar *p = &s[0];
	SetCurrentDirectory(p);
}
