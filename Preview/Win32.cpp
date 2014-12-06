//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <strsafe.h>

//////////////////////////////////////////////////////////////////////

tstring Win32ErrorMessage(DWORD err)
{
	LPTSTR lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				  NULL,
				  err ? err : GetLastError(),
				  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				  (LPTSTR)&lpMsgBuf,
				  0, NULL);

	tstring r(lpMsgBuf);
	LocalFree(lpMsgBuf);
	return r;
}

//////////////////////////////////////////////////////////////////////

void ErrorMessageBox(tchar const *format, ...)
{
	tchar buffer[1024];
	va_list v;
	va_start(v, format);
	_vstprintf_s(buffer, format, v);
	MessageBox(NULL, buffer, TEXT("Error"), MB_ICONEXCLAMATION);
	TRACE(Format(TEXT("%s\n"), buffer).c_str());
	//assert(false);
}

//////////////////////////////////////////////////////////////////////

void ChangeDirectory(tchar const *relative)
{
	vector<tchar> s;
	s.resize(GetCurrentDirectory(0, null));
	GetCurrentDirectory((DWORD)s.size(), &s[0]);
	s.resize(s.size() - 1);
	s.push_back(L'\\');
	for(; *relative; ++relative)
	{
		s.push_back(*relative);
	}
	s.push_back(0);
	SetCurrentDirectory(&s[0]);
}
