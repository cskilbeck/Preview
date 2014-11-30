//////////////////////////////////////////////////////////////////////

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <d3d11.h>
#include <gdiplus.h>

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <memory.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <stdlib.h>

#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <memory>

template <typename T> using ptr = std::unique_ptr<T>;

#include "Types.h"
#include "Util.h"
#include "Vec2.h"
#include "Point.h"
#include "Size.h"
#include "Rect.h"
#include "GDIPlus.h"
#include "Color.h"
#include "WinMessages.h"
#include "Window.h"
#include "ImageLoader.h"
#include "Preview.h"

#include "resource.h"

