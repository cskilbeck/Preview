//////////////////////////////////////////////////////////////////////

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3dcommon.h>
#include <DirectXMath.h>

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
#include <sstream>
#include <iostream>
#include <iterator>
#include <list>
#include <unordered_map>
#include <map>
#include <functional>
#include <algorithm>
#include <array>
#include <memory>

template <typename T> using ptr = std::unique_ptr<T>;

#include "linked_list.h"

using chs::linked_list;
using chs::list_node;

#include "Types.h"
#include "Util.h"
#include "Vec2.h"
#include "Point.h"
#include "Size.h"
#include "Rect.h"

#include "Color.h"
#include "D3D.h"
#include "Ptr.h"
#include "WinMessages.h"
#include "Window.h"
#include "App.h"
#include "ImageLoader.h"
#include "Preview.h"

#include "resource.h"

