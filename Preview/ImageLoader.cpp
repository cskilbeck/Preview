
#include "stdafx.h"

HRESULT LoadGDIPlusBitmap(TCHAR const *filename, size_t maxSize, ptr<Image> &bitmap)
{
	if(!filename)
	{
		return E_INVALIDARG;
	}
	bitmap.reset(Bitmap::FromFile(filename));
	if(bitmap.get() == null)
	{
		return E_FAIL;
	}
	if(bitmap->GetLastStatus() != Status::Ok)
	{
		return E_INVALIDARG;
	}
	return S_OK;
}
