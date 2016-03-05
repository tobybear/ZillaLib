/*
  ZillaLib
  Copyright (C) 2010-2016 Bernhard Schelling

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

#ifndef __ZL_PLATFORM_ANDROID__
#define __ZL_PLATFORM_ANDROID__

#define ZL_USE_CLOCKGETTIME
#include "ZL_PlatformPosix.h"

#ifdef ZL_VIDEO_OPENGL_ES1
#define GL_GLEXT_PROTOTYPES
#include <GLES/gl.h>
#include <GLES/glext.h>
#endif

#ifdef ZL_VIDEO_OPENGL_ES2
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

#ifdef __cplusplus
#include "ZL_Math.h"
#include "ZL_String.h"

//GLSL namespace
#ifdef ZL_VIDEO_USE_GLSL
#include "ZL_PlatformGLSL.h"
#endif

//android specific
ZL_String ZL_DeviceUniqueID();
void ZL_SoftKeyboardToggle();
void ZL_SoftKeyboardShow();
void ZL_SoftKeyboardHide();
bool ZL_SoftKeyboardIsShown();
void ZL_DeviceVibrate(int duration);
#if defined(ZILLALOG)
void __android_log(const char* logtag, const char* logtext);
#define ZL_LOG_PRINT(LOGTAG, LOGTEXT) __android_log(LOGTAG, LOGTEXT)
#else
#define ZL_LOG_PRINT(LOGTAG, LOGTEXT)
#endif

//Audio
void JavaCallStartAudioThread();
void* ZL_AudioAndroidLoad(struct ZL_File_Impl* file_impl);
void ZL_AudioAndroidPlay(void* audioPlayer, bool looped);
void ZL_AudioAndroidPause(void* audioPlayer);
void ZL_AudioAndroidStop(void* audioPlayer);
void ZL_AudioAndroidResume(void* audioPlayer);
void ZL_AudioAndroidRelease(void* audioPlayer);

#endif //__cplusplus
#endif //__ZL_PLATFORM_ANDROID__
