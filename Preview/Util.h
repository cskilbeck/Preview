//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

#if !defined(ARRAYSIZE)
#define ARRAYSIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

//////////////////////////////////////////////////////////////////////

void Trace(wchar const *strMsg, ...);
void Trace(char const *strMsg, ...);

#if defined(_DEBUG)
#define TRACE Trace
#else
#define TRACE(x, ...) if (false) {} else (x);
#endif

HRESULT LoadFile(tchar const *filename, void **data, size_t *size = null);
HRESULT LoadResource(uint32 resourceID, void **data, size_t *size = null);

WString WideStringFromTString(TString const &str);
WString WideStringFromString(String const &str);

TString TStringFromWideString(WString const &str);
TString TStringFromString(String const &str);

String StringFromTString(TString const &str);
String StringFromWideString(WString const &str);

WString Format(wchar const *fmt, ...);
String Format(char const *fmt, ...);
TString GetCurrentFolder();
TString GetDrive(tchar const *path);
TString GetPath(tchar const *path);
TString GetFilename(tchar const *path);
TString GetExtension(tchar const *path);
TString SetExtension(tchar const *path, tchar const *ext);

#ifndef PI
#define PI 3.14159265f
#define PI_2 (PI/2)
#endif

//////////////////////////////////////////////////////////////////////
// UTF8 decoding

bool GetUTF8Length(uint8 *bytes, uint32 *length);
bool DecodeUTF8(uint8 *bytes, wchar *buffer, uint32 bufferSize);

//////////////////////////////////////////////////////////////////////

template <typename T> inline int64 asciiToInt64(T const *p)
{
	assert(p != 0);
	int64 result = 0;
	bool negate = false;
	if(*p == '-')
	{
		negate = true;
		++p;
	}
	while(*p)
	{
		result *= 10;
		T c = *p++;
		if(c < '0' || c > '9')
		{
			break;
		}
		result += c - '0';
	}
	return negate ? -result : result;
}

//////////////////////////////////////////////////////////////////////

template <typename T> void Delete(T * &arr)
{
	if(arr != null)
	{
		delete arr;
		arr = null;
	}
}

//////////////////////////////////////////////////////////////////////

template <typename T> void Release(T &ptr)
{
	if(ptr != null)
	{
		ptr->Release();
		ptr = null;
	}
}

//////////////////////////////////////////////////////////////////////
// !! BY VALUE

template <typename T> T Max(T a, T b)
{
	return a > b ? a : b;
}

//////////////////////////////////////////////////////////////////////

template <typename T> T Min(T a, T b)
{
	return a < b ? a : b;
}

//////////////////////////////////////////////////////////////////////

template <typename T> T Constrain(T a, T lower, T upper)
{
	return Min(Max(a, lower), upper);
}

//////////////////////////////////////////////////////////////////////

template <typename T> int sgn(T val)	// where T is int, short, char etc. caveat caller
{
	return (T(0) < val) - (val < T(0)); 
}

//////////////////////////////////////////////////////////////////////

template <typename T> void Swap(T &a, T &b)
{
	std::swap(a, b);
}

//////////////////////////////////////////////////////////////////////

inline float Ease(float d)
{
	float d2 = d * d;
	float d3 = d2 * d;
	return 3 * d2 - 2 * d3;
}

//////////////////////////////////////////////////////////////////////

inline String ToLower(String const &s)
{
	String r(s);
	std::transform(r.begin(), r.end(), r.begin(), ::tolower);
	return r;
}

//////////////////////////////////////////////////////////////////////

inline WString ToLower(WString const &s)
{
	WString r(s);
	std::transform(r.begin(), r.end(), r.begin(), ::tolower);
	return r;
}

//////////////////////////////////////////////////////////////////////

inline int CountBits(uint32 i)
{
	i = i - ((i >> 1) & 0x55555555);
	i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
	return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

//////////////////////////////////////////////////////////////////////

inline int HighBit(uint32 x)
{
	unsigned long index;
	byte isNonzero = _BitScanReverse(&index, x);
	return isNonzero ? index + 1 : 0;
}

//////////////////////////////////////////////////////////////////////

#if defined(_WIN64)
inline int HighBit64(uint64 x)
{
	unsigned long index;
	byte isNonzero = _BitScanReverse64(&index, x);
	return isNonzero ? index + 1 : 0;
}
#endif

//////////////////////////////////////////////////////////////////////

inline int NextPowerOf2(int v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

//////////////////////////////////////////////////////////////////////

template<class T> struct in_reverse
{
	T &l;
	in_reverse(T &list) : l(list) {}

	auto begin() ->         decltype(l.rbegin())   { return l.rbegin(); } 
	auto begin() const ->   decltype(l.crbegin())  { return l.crbegin(); } 
	auto end() ->           decltype(l.rend())     { return l.rend(); } 
	auto end() const ->     decltype(l.crend())    { return l.crend(); } 
};

template<class T> in_reverse<T> reverse(T &l)
{
	return in_reverse<T>(l);
}

//////////////////////////////////////////////////////////////////////

