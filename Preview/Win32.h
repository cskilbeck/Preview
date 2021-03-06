//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

void ChangeDirectory(tchar const *relative);
tstring Win32ErrorMessage(DWORD err = 0);
void ErrorMessageBox(tchar const *format, ...);

//////////////////////////////////////////////////////////////////////

inline Point2D GetPointFromParam(uintptr param)
{
	return Point2D((int16)LOWORD(param), (int16)HIWORD(param));
}

