//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

typedef uint8_t			uint8;
typedef uint16_t		uint16;
typedef uint32_t		uint32;
typedef uint64_t		uint64;
typedef int8_t			int8;
typedef int16_t			int16;
typedef int32_t			int32;
typedef int64_t			int64;
typedef unsigned int	uint;
typedef wchar_t			wchar;
typedef TCHAR			tchar;
typedef uint8			byte;
typedef LONG_PTR		intptr;
typedef ULONG_PTR		uintptr;

static const nullptr_t null = nullptr;

using std::vector;
using std::string;
using std::wstring;

using tstring = std::basic_string<tchar>;

template <typename T> using Ptr = std::unique_ptr<T>;
