/*
  ZillaLib
  Copyright (C) 2010-2025 Bernhard Schelling

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

#ifndef __ZL_PLATFORM_CONFIG__
#define __ZL_PLATFORM_CONFIG__

#include "ZL_Application.h"

#if defined(__ANDROID__)
	#define ZL_VIDEO_WEAKCONTEXT
	#define ZL_USE_PTHREAD
	#define ZL_USE_POSIXTIME
	#define ZL_USE_ENET
	#define ZL_HAS_SOFTKEYBOARD
	#define ZL_HAS_DEVICEVIBRATE
	#define ZL_HAS_DEVICEUNIQUEID
	#define ZL_HAS_NATIVEZLIB
#elif defined(__IPHONEOS__)
	#define ZL_USE_PTHREAD
	#define ZL_USE_POSIXTIME
	#define ZL_USE_ENET
	#define ZL_HAS_SOFTKEYBOARD
	#define ZL_HAS_DEVICEVIBRATE
	#define ZL_HAS_DEVICEUNIQUEID
	#define ZL_HAS_NATIVEZLIB
#elif defined(__WINDOWSPHONE__)
	#define ZL_VIDEO_DIRECT3D
	#define ZL_USE_ENET
	#define ZL_HAS_SOFTKEYBOARD
	#define _CRT_SECURE_NO_WARNINGS
#elif defined(__NACL__)
	#define ZL_USE_PTHREAD
	#define ZL_HAS_FULLSCREEN
	#define ZL_HAS_POINTERLOCK
#elif defined(__WEBAPP__)
	#define ZL_HAS_FULLSCREEN
	#define ZL_HAS_POINTERLOCK
#else
	#define ZL_USE_SDL
	#define ZL_USE_ENET
	#define ZL_USE_JOYSTICKINIT
	#define ZL_HAS_FULLSCREEN
	#define ZL_HAS_POINTERLOCK
#endif

#endif
