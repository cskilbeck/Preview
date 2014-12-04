//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

inline void CD(tchar const *relative)
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
	SetCurrentDirectory(&s[0]);
}

//////////////////////////////////////////////////////////////////////

tstring Win32ErrorMessage(DWORD err = 0);

void ErrorMessageBox(tchar const *format, ...);