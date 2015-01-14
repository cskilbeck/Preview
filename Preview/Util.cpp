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
// Crappy RAII HANDLE wrapper

struct Handle
{
	Handle(HANDLE handle = INVALID_HANDLE_VALUE)
		: h(handle)
	{
	}

	~Handle()
	{
		Close();
	}

	bool Close()
	{
		bool rc = Valid() ? (CloseHandle(h) != 0) : true;
		h = INVALID_HANDLE_VALUE;
		return rc;
	}

	bool Valid() const
	{
		return h != INVALID_HANDLE_VALUE;
	}

	bool operator == (HANDLE handle) const
	{
		return h == handle;
	}

	bool operator != (HANDLE handle) const
	{
		return h != handle;
	}

	operator HANDLE * ()
	{
		return &h;
	}

	operator HANDLE & ()
	{
		return h;
	}

	HANDLE h;
};

//////////////////////////////////////////////////////////////////////

struct File
{
	HRESULT Open(tchar const *filename)
	{
		name = filename;
		h = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		if(h == INVALID_HANDLE_VALUE)
		{
			error = ErrorMsgBox(Format(TEXT("Error opening %s"), filename).c_str());
		}
	}

	bool Close()
	{
		return h.Close();
	}

	bool IsOpen() const
	{
		return h.Valid();
	}

	int64 Size()
	{
		if(!IsOpen())
		{
			error = ERROR_FILE_OFFLINE;
			return -1;
		}
		int64 fileSize = 0;
		if(GetFileSizeEx(h, (LARGE_INTEGER *)&fileSize))
		{
			error = ERROR_SUCCESS;
			return fileSize;
		}
		error = ErrorMsgBox(Format(TEXT("Error getting file size of %s"), name.c_str()).c_str());
		return -1;
	}

	DWORD Read(DWORD bytes, void *buffer)
	{
		DWORD got;
		error = ERROR_SUCCESS;
		if(ReadFile(h, buffer, bytes, &got, null))
		{
			return got;
		}
		error = GetLastError();
		return 0;
	}

	DWORD Write(DWORD bytes, void *buffer)
	{
		DWORD wrote;
		error = ERROR_SUCCESS;
		if(WriteFile(h, buffer, bytes, &wrote, null))
		{
			return wrote;
		}
		error = GetLastError();
		return 0;
	}

	tstring name;
	Handle h;
	long error;
};

//////////////////////////////////////////////////////////////////////
// Simple file loader

HRESULT LoadFile(tchar const *filename, void **data, size_t *size)
{
	if(filename == null || data == null || size == null)
	{
		return E_POINTER;
	}

	Handle h = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if(h == INVALID_HANDLE_VALUE)
	{
		return ErrorMsgBox(Format(TEXT("Error opening %s"), filename).c_str());
	}

	int64 fileSize = 0;
	if(!GetFileSizeEx(h, (LARGE_INTEGER *)&fileSize))
	{
		return ErrorMsgBox(Format(TEXT("Error getting file size of %s"), filename).c_str());
	}

	if(fileSize > 0x7ffffff - sizeof(tchar))
	{
		MessageBox(null, Format(TEXT("Can't load %s (2GB limit, sorry)"), filename).c_str(), TEXT("Error"), MB_ICONEXCLAMATION);
		return ERROR_FILE_TOO_LARGE;
	}

	Ptr<uint8> buf(new uint8[fileSize + sizeof(tchar)]);
	if(buf == null)
	{
		MessageBox(null, Format(TEXT("Can't load %s - file > 2GB!"), filename).c_str(), TEXT("Error"), MB_ICONEXCLAMATION);
		return ERROR_NOT_ENOUGH_MEMORY;
	}

	DWORD got;
	if(!ReadFile(h, buf.get(), (DWORD)fileSize, &got, null) || got != fileSize)
	{
		return ERROR_FILE_CORRUPT;
	}
	*(tchar *)(buf.get() + fileSize) = tchar(0);

	*data = buf.release();
	*size = fileSize;

	return S_OK;
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
		Ptr<tchar> s(new tchar[l]);
		GetCurrentDirectory(l, s.get());
		return tstring(s.get());
	}
	TRACE(TEXT("Error getting current folder: %s"), GetLastErrorText());
	return tstring();
}

//////////////////////////////////////////////////////////////////////

static struct PathComponents
{
	tchar drive[_MAX_DRIVE];
	tchar dir[_MAX_DIR];
	tchar name[_MAX_FNAME];
	tchar ext[_MAX_EXT];
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
