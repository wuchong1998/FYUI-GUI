#pragma once

// If FYUI is linked as a static library, define FYUI_STATIC in both the FYUI
// project and the consuming EXE project. DLL builds should define FYUI_EXPORTS
// only while building FYUI itself.
#ifdef FYUI_STATIC
#define FYUI_API 
#else
#if defined(FYUI_EXPORTS)
#	if defined(_MSC_VER)
#		define FYUI_API __declspec(dllexport)
#	else
#		define FYUI_API
#	endif
#else
#	if defined(_MSC_VER)
#		define FYUI_API __declspec(dllimport)
#	else
#		define FYUI_API 
#	endif
#endif
#endif
#define FYUI_COMDAT __declspec(selectany)

#pragma warning(disable:4505)
#pragma warning(disable:4251)
#pragma warning(disable:4189)
#pragma warning(disable:4121)
#pragma warning(disable:4100)



#include "Utils/Utils.h"
#include "Utils/unzip.h"
#include "Core/UIMakup.h"
#include "Utils/observer_impl_base.h"
#include "Utils/UIShadow.h"
#include "Utils/UIDelegate.h"
#include "Utils/DragDropImpl.h"
#include "Utils/TrayIcon.h"
#include "Utils/DPI.h"
#include "Utils/sdk_util.h"

#include "Core/UIDefine.h"
#include "Core/UIResourceManager.h"
#include "Core/UIManager.h"
#include "Core/UIBase.h"
#include "Core/ControlFactory.h"
#include "Core/UIControl.h"
#include "Core/UIContainer.h"

#include "Core/UIDlgBuilder.h"
#include "Core/UIRender.h"
#include "Utils/WinImplBase.h"

#include "Layout/UIVerticalLayout.h"
#include "Layout/UIHorizontalLayout.h"
#include "Layout/UITileLayout.h"
#include "Layout/UITabLayout.h"
#include "Layout/UIChildLayout.h"

#include "Control/UIList.h"
#include "Control/VirtualListUI.h"
#include "Control/UICombo.h"
#include "Control/UIScrollBar.h"
#include "Control/UITreeView.h"
#include "Control/UILabel.h"
#include "Control/UIGifAnim.h"
#include "Control/UIAnimation.h"
#include "Layout/UIAnimationTabLayout.h"
#include "Control/UIButton.h"
#include "Control/UIFrameTest.h"
#include "Control/UIRichEdit.h"
#include "Control/UIOption.h"
#include "Control/UIProgress.h"
#include "Control/UISlider.h"
#include "Control/UIComboBox.h"
#include "Control/UIMenu.h"
#include "Control/UIColorPalette.h"
#include "Control/UIListEx.h"
#include "Control/UIFadeButton.h"
#include "Control/UIRing.h"
#include "Control/UIButtonEx.h"
#include "Control/IVirtualDataProvider.h"
#include "Control/UICountdown.h"

//

#ifdef SVG

typedef void* (*GetSvgHandleFunc)(unsigned char* svg_data, DWORD svg_size, int nScale, int& svg_width, int& svg_height,
	bool& alpha_channel, HBITMAP& bitmap);

typedef void(*GetSvgWidthHeightFunc)(void* handle, int& svg_width, int& svg_height);

typedef void* (*GetSvgHbitmapFunc)(void* handle, int svg_width, int svg_height, bool& alpha_channel, HBITMAP& bitmap);

typedef void* (*FreeSVGBitmap)(HBITMAP& bitmap);
typedef void* (*FreeSVGHandle)(void* handle);
#endif




