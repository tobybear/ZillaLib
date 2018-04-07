/*
  ZillaLib
  Copyright (C) 2010-2018 Bernhard Schelling

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include "ZL_Application.h"
#include "ZL_String.h"
#include "ZL_Platform.h"
#include <time.h>
#include <assert.h>
#ifndef ZL_LOG_PRINT
#include <stdio.h>
#endif

ZL_Application* ZL_MainApplication = 0;
short ZL_Requested_FPS = 0;
unsigned int ZL_LastFPSTicks = 0, ZL_TPF_Limit = 0, ZL_MainApplicationFlags = 0;
int ZL_DoneReturn;
void (*funcSceneManagerCalculate)() = NULL;
void (*funcSceneManagerDraw)() = NULL;

ZL_Signal_v0 ZL_Application::sigKeepAlive;
static int FPS_Frame_Count = 0;

unsigned short ZL_Application::FPS = 60;
ticks_t ZL_Application::Ticks = 0;
ticks_t ZL_Application::ElapsedTicks = 0;
unsigned int ZL_Application::FrameCount = 0;
scalar ZL_Application::Elapsed = s(0);

//set to some readable pointer because audio thread might access this on display error or before window is inited
#define WINDOWFLAGS_INVALID ((unsigned int*)&native_width)
unsigned int *pZL_WindowFlags = WINDOWFLAGS_INVALID;

static void _ZL_ApplicationUpdateTimingFps()
{
	ticks_t last = ZL_Application::Ticks;
	ZL_Application::Ticks = ZL_GetTicks();
	//Elapsed = s(ElapsedTicks = Ticks - last) / s(1000);return; //disable any frame limiting

	FPS_Frame_Count++;
	if (ZL_Application::Ticks - ZL_LastFPSTicks >= 1000)
	{
		ZL_Application::FPS = (ZL_Application::Ticks - ZL_LastFPSTicks < 2000 ? (short)(FPS_Frame_Count * ( 2.0f - (float)(ZL_Application::Ticks - ZL_LastFPSTicks)*0.001f ) + 0.495f) : 0);
		if (ZL_Application::FPS < 1) ZL_Application::FPS = 1; //<= 0 is bad for code like: x / FPS
		FPS_Frame_Count = 0;
		ZL_LastFPSTicks += 1000;
		if (ZL_TPF_Limit && (ZL_Application::FPS > (1000/ZL_TPF_Limit)+5 || ZL_Application::FPS < (1000/(ZL_TPF_Limit+1))-5)) ZL_MainApplicationFlags |= ZL_APPLICATION_VSYNCFAILED;
		else ZL_MainApplicationFlags &= ~ZL_APPLICATION_VSYNCFAILED;
		//ZL_LOG3("FPS", "FPS: %d - AVGFPS: %d - VSYNC: %d", ZL_Application::FPS, ZL_Application::FrameCount / (ZL_Application::Ticks/1000), !(ZL_MainApplicationFlags & (ZL_APPLICATION_NOVSYNC|ZL_APPLICATION_VSYNCFAILED)));
	}

	if (!ZL_TPF_Limit || (ZL_MainApplicationFlags & (ZL_APPLICATION_NOVSYNC|ZL_APPLICATION_VSYNCFAILED)))
	{
		ZL_Application::ElapsedTicks = ZL_Application::Ticks - last;
		if (ZL_Application::ElapsedTicks < ZL_TPF_Limit)
		{
			ZL_Delay(ZL_TPF_Limit - ZL_Application::ElapsedTicks);
			ZL_Application::Ticks = ZL_GetTicks();
			ZL_Application::ElapsedTicks = ZL_Application::Ticks - last;
		}
		ZL_Application::Elapsed = s(ZL_Application::ElapsedTicks) / s(1000);
		if (ZL_Application::Elapsed >= 1) ZL_Application::Elapsed = s(0.999996); //>= 1 is bad for code like: x / (1 - elapsed)
		if (ZL_Application::Elapsed <= 0) ZL_Application::Elapsed = s(0.000004); //<= 0 is bad for code like: x / Elapsed
	}
	else if (ZL_Requested_FPS == 60)
	{
		ZL_Application::Elapsed = s(1.0/60.0);
		ZL_Application::ElapsedTicks = (ZL_Application::FrameCount%3 ? 17 : 16);
	}
	else
	{
		ZL_Application::Elapsed = s(1) / s(ZL_Requested_FPS);
		ZL_Application::ElapsedTicks = ZL_Application::FrameCount * 1000;
		ZL_Application::ElapsedTicks = ((ZL_Application::ElapsedTicks+1000) / ZL_Requested_FPS) - (ZL_Application::ElapsedTicks / ZL_Requested_FPS);
		if (ZL_Application::ElapsedTicks < (unsigned int)ZL_TPF_Limit-1 || ZL_Application::ElapsedTicks > (unsigned int)ZL_TPF_Limit+1) ZL_Application::ElapsedTicks = ZL_TPF_Limit;
	}
	//ZL_LOG2("ELAPSED", "F: %f - T: %d", ZL_Application::Elapsed, ZL_Application::ElapsedTicks);

	ZL_Application::FrameCount++;
}

ZL_Application::ZL_Application(short fpslimit)
{
	assert(!ZL_MainApplication);
	ZL_MainApplication = this;
	SetFpsLimit(fpslimit);
	FPS = (ZL_Requested_FPS > 0 ? (unsigned short)ZL_Requested_FPS : 0);
}

void ZL_Application::SetFpsLimit(short fps)
{
	ZL_Requested_FPS = fps;
	if (fps < 1) ZL_TPF_Limit = 0;
	else ZL_TPF_Limit = (unsigned short)((1000.0f / (float)fps)-0.0495f);
	#if !defined(__SMARTPHONE__) && !defined(__WEBAPP__)
	ZL_UpdateTPFLimit();
	#endif
}

void ZL_Application::Frame()
{
	_ZL_ApplicationUpdateTimingFps();
	sigKeepAlive.call();
	if (funcSceneManagerCalculate)
	{
		funcSceneManagerCalculate();
		funcSceneManagerDraw();
	}
	else
	{
		if (native_aspectcorrection) { glClearColor(0.0f, 0.0f, 0.0f, 1.0f); glClear(GL_COLOR_BUFFER_BIT); }
		BeforeFrame();
		AfterFrame();
	}
}

void ZL_Application::Quit(int Return)
{
	ZL_DoneReturn = Return;
	ZL_MainApplicationFlags |= ZL_APPLICATION_DONE;
}

ZL_Application& ZL_Application::GetApplication()
{
	return *ZL_MainApplication;
}

int ZL_Application::Log(const char *logtag, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	ZL_String logtext = ZL_String::vformat(format, ap);
	va_end(ap);
	#ifdef ZL_LOG_PRINT
	ZL_LOG_PRINT(logtag, logtext.c_str());
	#else
	printf("[%4u.%03u] [%10s] %s\n", Ticks/1000, Ticks%1000, logtag, logtext.c_str());
	//printf("[%5u] [%10s] %s\n", FrameCount, logtag, logtext.c_str());
	#endif
	return (int)logtext.size();
}

ZL_String ZL_Application::DeviceUniqueID()
{
	#ifdef ZL_HAS_DEVICEUNIQUEID
	return ZL_DeviceUniqueID();
	#else
	return ZL_String::EmptyString;
	#endif
}

void ZL_Application::SettingsInit(const char* FallbackConfigFilePrefix)
{
	ZL_SettingsInit(FallbackConfigFilePrefix);
}
ZL_String ZL_Application::SettingsGet(const char* Key)
{
	return ZL_SettingsGet(Key);
}
void ZL_Application::SettingsSet(const char* Key, const ZL_String& Value)
{
	ZL_SettingsSet(Key, Value);
}
void ZL_Application::SettingsDel(const char* Key)
{
	ZL_SettingsDel(Key);
}
bool ZL_Application::SettingsHas(const char* Key)
{
	return ZL_SettingsHas(Key);
}
void ZL_Application::SettingsSynchronize()
{
	ZL_SettingsSynchronize();
}

void ZL_Application::OpenExternalUrl(const char* url)
{
	ZL_OpenExternalUrl(url);
}

#if defined(NDEBUG) && !defined(__SMARTPHONE__) && !defined(__WEBAPP__)
bool ZL_Application::LoadReleaseDesktopDataBundle(const char* DataBundleFileName)
{
	return ZL_LoadReleaseDesktopDataBundle(DataBundleFileName);
}
#endif

static double ZL_TickDuration, ZL_TickSum, ZL_TickExcess;

ZL_ApplicationConstantTicks::ZL_ApplicationConstantTicks(short fps, unsigned short tps) : ZL_Application(fps)
{
	Elapsed = s(1)/s(tps > 0 ? tps : 60);
	ZL_TickDuration = (tps > 0 ? (1000.0/(double)tps) : 0.0);
	ZL_TickSum = 0.0;
	ZL_TickExcess = 0.0;
	Ticks = 0;
}

void ZL_ApplicationConstantTicks::SetFpsTps(short fps, unsigned short tps)
{
	SetFpsLimit(fps);
	Elapsed = s(1)/s(tps > 0 ? tps : 60);
	ZL_TickDuration = (tps > 0 ? (1000.0/(double)tps) : 0.0);
}

void ZL_ApplicationConstantTicks::Frame()
{
	assert(funcSceneManagerCalculate);
	_ZL_ApplicationUpdateTimingFps();
	ticks_t now = Ticks;
	ZL_TickExcess += (double)ElapsedTicks;
	if (ZL_TickDuration == 0)
	{
		sigKeepAlive.call();
		funcSceneManagerCalculate();
		AfterCalculate();
		ZL_TickSum = (double)now;
		ZL_TickExcess = 0;
	}
	else if (ZL_TickExcess >= -ZL_TickDuration)
	{
		scalar DrawElapsed = Elapsed;
		unsigned int DrawElapsedTicks = ElapsedTicks;
		Elapsed = s(ZL_TickDuration/1000.0);
		unsigned int maxcalcs = 1000/ZL_TPF_Limit;
		for (double MinExcess = -ZL_TickDuration; ZL_TickExcess >= MinExcess && maxcalcs--; MinExcess = ZL_TickDuration)
		{
			ZL_TickExcess -= ZL_TickDuration;
			ticks_t OldTicks = (ticks_t)ZL_TickSum;
			ZL_TickSum += ZL_TickDuration;
			Ticks = (ticks_t)ZL_TickSum;
			ElapsedTicks = Ticks - OldTicks;
			sigKeepAlive.call();
			funcSceneManagerCalculate();
			AfterCalculate();
			//calcticks++;
			if (MinExcess < 0 && ZL_TickExcess < ZL_TickDuration*2) break;
		}
		//if (calcticks != 1) { ZL_LOG3("FRAME", "Skipped %d frames (%f -> %f)!", calcticks-1, NOWEXCESS, ZL_TickExcess); }
		Elapsed = DrawElapsed;
		ElapsedTicks = DrawElapsedTicks;
	}
	else Ticks = (ticks_t)ZL_TickSum;
	funcSceneManagerDraw();
	Ticks = now;
}

void ZillaLibInit(int argc, char *argv[])
{
	srand((unsigned int)time(NULL));rand();
	assert(ZL_MainApplication);
	ZL_MainApplication->Load(argc, argv);
	if (pZL_WindowFlags == WINDOWFLAGS_INVALID) { ZL_MainApplication->Quit(1); }
	ZL_StartTicks();
	ZL_LastFPSTicks = ZL_Application::Ticks = ZL_GetTicks();
}
