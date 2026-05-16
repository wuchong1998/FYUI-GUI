#pragma once

// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 VIDEOCONVERTDLL_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// VIDEOCONVERTDLL_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef SVGDLL_EXPORTS
#define SVGDLL_API __declspec(dllexport)
#else
#define SVGDLL_API __declspec(dllimport)
#endif
#include <windows.h>
#include <string>


extern "C" _declspec(dllexport) void* GetSvgHandle(unsigned char*, DWORD,int, int&, int&,bool&, HBITMAP&);
extern "C" _declspec(dllexport) void GetSvgWidthHeight(void* , int& , int& );
extern "C" _declspec(dllexport) void GetSvgHbitmap(void*, int, int, bool&, HBITMAP&);
extern "C" _declspec(dllexport) void FreeSVGBitmap(HBITMAP&);
extern "C" _declspec(dllexport) void FreeSVGHandle(void*);

SVGDLL_API void* GetSvgHandle(unsigned char* svg_data, DWORD svg_size, int nScale,int &svg_width,int &svg_height,
	bool & alpha_channel,HBITMAP& bitmap);

SVGDLL_API void GetSvgWidthHeight(void*handle, int& svg_width, int& svg_height);

SVGDLL_API void GetSvgHbitmap(void* handle, int svg_width, int svg_height, bool& alpha_channel, HBITMAP& bitmap);

SVGDLL_API void FreeSVGBitmap(HBITMAP&bitmap);

SVGDLL_API void FreeSVGHandle(void* handle);
