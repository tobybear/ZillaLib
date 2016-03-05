#include "ZL_AdDisplay.h"
#include "../../Source/ZL_Platform.h"

static int native_adwidth;
static ZL_Rectf recAdArea;

void calcRecAdArea(ZL_Origin::Type pos)
{
	int native_adheight = native_adwidth*50/320; //320x50 = Standard banner
	recAdArea.left = s(pos % 3 == 1 ? 0 : (pos % 3 == 2 ? (native_width-native_adwidth)/2   : native_width-native_adwidth));
	recAdArea.low  = s(pos <= 3     ? 0 : (pos <= 6     ? (native_height-native_adheight)/2 : native_height-native_adheight));
 	if (native_width != ZLWIDTH || native_height != ZLHEIGHT)
	{
		recAdArea.left -= window_viewport[0]; recAdArea.left = recAdArea.left*ZLWIDTH/window_viewport[2];
		recAdArea.low -= window_viewport[1]; recAdArea.low = recAdArea.low*ZLHEIGHT/window_viewport[3];
		recAdArea.right = recAdArea.left + native_adwidth*ZLWIDTH/window_viewport[2];
		recAdArea.high = recAdArea.low + native_adheight*ZLHEIGHT/window_viewport[3];
	}
	else
	{
		recAdArea.right = recAdArea.left + native_adwidth;
		recAdArea.high = recAdArea.low + native_adheight;
	}
}

void calcRecAdArea(short x, short y)
{
	int native_adheight = native_adwidth*50/320; //320x50 = Standard banner
	recAdArea.left = x;
	recAdArea.high  = y;
	recAdArea.right = s(x + native_adwidth*ZLWIDTH/window_viewport[2]);
	recAdArea.low = s(y - native_adheight*ZLHEIGHT/window_viewport[3]);
}

#if defined(__ANDROID__)
#include <jni.h>
#include <android/log.h>
#include <assert.h>
static jclass JavaAdDisplay = NULL;
static jmethodID JavaAdDisplayShow = NULL;
int JavaCallAdDisplayInit(const ZL_String& admob_unit_id, bool show, short pos, short x, short y)
{
	jclass cls = jniEnv->FindClass("org/zillalib/AdDisplay");
	assert(cls);
	jmethodID JavaAdDisplayInit = jniEnv->GetStaticMethodID(cls, "Init", "(Lorg/zillalib/ZillaActivity;Ljava/lang/String;ZSSS)V");
	assert(JavaAdDisplayInit);
	jniEnv->CallStaticVoidMethod(cls, JavaAdDisplayInit, JavaZillaActivity, jniEnv->NewStringUTF(admob_unit_id.c_str()), (jboolean)show, (jshort)pos, (jshort)x, (jshort)y);
	jfieldID fadwidth = jniEnv->GetStaticFieldID(cls, "adWidth", "I");
	assert(fadwidth);
	return jniEnv->GetStaticIntField(cls, fadwidth);
}

void JavaCallAdDisplayShow(bool show, short pos, short x, short y)
{
	if (JavaAdDisplayShow == NULL)
	{
		JavaAdDisplay = jniEnv->FindClass("org/zillalib/AdDisplay");
		assert(JavaAdDisplay);
		JavaAdDisplayShow = jniEnv->GetStaticMethodID(JavaAdDisplay, "Show", "(Lorg/zillalib/ZillaActivity;ZSSS)V");
		assert(JavaAdDisplayShow);
	}
	jniEnv->CallStaticVoidMethod(JavaAdDisplay, JavaAdDisplayShow, JavaZillaActivity, (jboolean)show, (jshort)pos, (jshort)x, (jshort)y);
}
#define PlatformAdDisplayInit(id,show,pos,x,y) JavaCallAdDisplayInit(id,show,pos,x,y)
#define PlatformAdDisplayShow(show,pos,x,y) JavaCallAdDisplayShow(show,pos,x,y)
#elif defined(__IPHONEOS__)
int ZL_IOSAdDisplayInit(const ZL_String& admob_unit_id, bool show, short pos, short x, short y);
void ZL_IOSAdDisplayShow(bool show, short pos, short x, short y);
#define PlatformAdDisplayInit(id,show,pos,x,y) ZL_IOSAdDisplayInit(id,show,pos,x,y)
#define PlatformAdDisplayShow(show,pos,x,y) ZL_IOSAdDisplayShow(show,pos,x,y)
#else
int PlatformAdDisplayInit(const ZL_String&, bool, short, short, short) { return MIN(native_width, native_height); }
void PlatformAdDisplayShow(bool, short, short, short) { }
#endif

bool ZL_AdDisplay::Init(const ZL_String& admob_unit_id, bool show, ZL_Origin::Type pos)
{
	native_adwidth = PlatformAdDisplayInit(admob_unit_id, show, (short)pos, 0, 0);
	calcRecAdArea(pos);
	return true;
}

bool ZL_AdDisplay::Init(const ZL_String& admob_unit_id, short x, short y, bool show)
{
	int native_x = x, native_y = ZLHEIGHT - y;
	if (native_width != ZLWIDTH || native_height != ZLHEIGHT)
	{
		native_x = native_x*window_viewport[2]/ZLWIDTH; native_x += window_viewport[0];
		native_y = native_y*window_viewport[3]/ZLHEIGHT; native_y += window_viewport[1];
	}
	native_adwidth = PlatformAdDisplayInit(admob_unit_id, show, (short)0, native_x, native_y);
	calcRecAdArea(x, y);
	return true;
}

void ZL_AdDisplay::Show(bool show)
{
	PlatformAdDisplayShow(show, (short)-1, 0, 0);
}

void ZL_AdDisplay::Show(bool show, ZL_Origin::Type pos)
{
	PlatformAdDisplayShow(show, (short)pos, 0, 0);
	calcRecAdArea(pos);
}

void ZL_AdDisplay::Show(short x, short y, bool show)
{
	int native_x = x, native_y = ZLHEIGHT - y;
	if (native_width != ZLWIDTH || native_height != ZLHEIGHT)
	{
		native_x = native_x*window_viewport[2]/ZLWIDTH; native_x += window_viewport[0];
		native_y = native_y*window_viewport[3]/ZLHEIGHT; native_y += window_viewport[1];
	}
	PlatformAdDisplayShow(show, (short)0, native_x, native_y);
	calcRecAdArea(x, y);
}

const ZL_Rectf& ZL_AdDisplay::GetAdArea()
{
	return recAdArea;
}
