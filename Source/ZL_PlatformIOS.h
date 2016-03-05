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

#ifndef __ZL_PLATFORM_IOS__
#define __ZL_PLATFORM_IOS__

#include "ZL_PlatformPosix.h"

#ifdef ZL_VIDEO_OPENGL_ES1
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#endif

#ifdef ZL_VIDEO_OPENGL_ES2
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#endif

#ifdef __cplusplus
#include "ZL_String.h"

//GLSL namespace
#ifdef ZL_VIDEO_USE_GLSL
#include "ZL_PlatformGLSL.h"
#endif

//device specific
ZL_String ZL_DeviceUniqueID();
void ZL_SoftKeyboardToggle();
void ZL_SoftKeyboardShow();
void ZL_SoftKeyboardHide();
bool ZL_SoftKeyboardIsShown();
void ZL_DeviceVibrate(int duration);

//Audio
void* ZL_AudioPlayerOpen(ZL_String filename);
void ZL_AudioPlayerPlay(void* audioPlayer, bool looped);
void ZL_AudioPlayerPause(void* audioPlayer);
void ZL_AudioPlayerStop(void* audioPlayer);
void ZL_AudioPlayerResume(void* audioPlayer);
void ZL_AudioPlayerRelease(void* audioPlayer);
void ZL_AudioPlayerRate(void* audioPlayer, float rate);
void ZL_AudioPlayerVolume(void* audioPlayer, float vol);

#endif //__cplusplus
#endif //__ZL_PLATFORM_IOS__
