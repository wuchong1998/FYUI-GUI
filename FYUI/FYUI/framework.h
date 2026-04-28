#pragma once

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
#include <tchar.h>
#include <windows.h>
#include <windowsx.h>

#include <stddef.h>
#include <richedit.h>
#include <assert.h>
#include <crtdbg.h>
#include <malloc.h>

#include <gdiplus.h>
#include <string>
#include <map>
#include <deque>
#include <vector>
#include <stdio.h>

#include <atlbase.h>



#pragma comment( lib, "comctl32.lib" )
#pragma comment( lib, "GdiPlus.lib" )
#pragma comment( lib, "Imm32.lib" )


#include "FYUILib.h"
#include <olectl.h>

#define lengthof(x) (sizeof(x)/sizeof(*x))
#define MAX max
#define MIN min
#define CLAMP(x,a,b) (MIN(b,MAX(a,x)))
