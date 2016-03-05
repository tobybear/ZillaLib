#ifndef __ZL_ADDISPLAY__
#define __ZL_ADDISPLAY__

#include <ZL_String.h>
#include <ZL_Display.h>
#include <ZL_Application.h>

//TODO: Works bad in resizable (rotatable) window environments

#if defined(__ANDROID__)
#define ZL_ADPLATFORMUNITID(android,iphone) android
#elif defined(__IPHONEOS__)
#define ZL_ADPLATFORMUNITID(android,iphone) iphone
#else
#define ZL_ADPLATFORMUNITID(android,iphone) ZL_String::EmptyString
#endif

class ZL_AdDisplay
{
public:
	static bool Init(const ZL_String& admob_unit_id, bool show = true, ZL_Origin::Type pos = ZL_Origin::BottomCenter);
	static bool Init(const ZL_String& admob_unit_id, short x, short y, bool show = true);
	static void Show(bool show = true);
	static void Show(bool show = true, ZL_Origin::Type pos = ZL_Origin::BottomCenter);
	static void Show(short x, short y, bool show = true);
	static const ZL_Rectf& GetAdArea();
};

#endif //__ZL_ADDISPLAY__
