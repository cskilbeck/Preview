//////////////////////////////////////////////////////////////////////

#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <tchar.h>
#include <malloc.h>
#include <memory.h>
#include <assert.h>

#include <vector>
#include <string>
#include <algorithm>
#include <memory>
#include <unordered_map>
#include <exception>

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <dshow.h>
#include <objbase.h>

#include "Resource.h"

#include "Types.h"
#include "Util.h"
#include "WinResource.h"
#include "WinMessages.h"
#include "Timer.h"
#include "Vec2.h"
#include "Point.h"
#include "Size.h"
#include "Rect.h"
#include "Win32.h"

#include "Color.h"
#include "D3D.h"
#include "Texture.h"
#include "CBuffer.h"
#include "Shader.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "Material.h"
#include "Window.h"
#include "DXWindow.h"
#include "qedit.h"
#include "FrameGrabber.h"
#include "AVIPlayer.h"
