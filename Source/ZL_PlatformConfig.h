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

// set this define to enable support for network communications
// #define ZL_USE_NETWORK

// set this define to enable usage of ENET library
// #define ZL_USE_ENET

// set this define to enable tesselation (used for 2D polygons and 3D meshes)
#define ZL_USE_TESSELATE

// set this define to enable the SynthIMC synthesizer
#define ZL_USE_SYNTHIMC

// set this define to enable support for OGG Vorbis files
#define ZL_USE_VORBIS

// set this define to enable support for TTF fonts
#define ZL_USE_TTF

// set this define to include support for stbimage loader
#define ZL_USE_STBIMAGE

// set this define to include support for JSON, XML, Checksum, Base64, Compression
#define ZL_USE_DATA

// set this define to include support for QOI image files
// #define ZL_USE_QOI

// set this to include support for 3D scene rendering
#define ZL_USE_DISPLAY3D

#ifndef ZL_USE_DISPLAY3D
#define ZL_DISABLE_DISPLAY3D
#else
#undef ZL_DISABLE_DISPLAY3D
#endif

#if defined(__ANDROID__)
	#define ZL_VIDEO_WEAKCONTEXT
	#define ZL_USE_PTHREAD
	#define ZL_USE_POSIXTIME
	#ifndef ZL_USE_ENET
		#define ZL_USE_ENET
	#endif
	#define ZL_HAS_SOFTKEYBOARD
	#define ZL_HAS_DEVICEVIBRATE
	#define ZL_HAS_DEVICEUNIQUEID
	#define ZL_HAS_NATIVEZLIB
#elif defined(__IPHONEOS__)
	#define ZL_USE_PTHREAD
	#define ZL_USE_POSIXTIME
	#ifndef ZL_USE_ENET
		#define ZL_USE_ENET
	#endif
	#define ZL_HAS_SOFTKEYBOARD
	#define ZL_HAS_DEVICEVIBRATE
	#define ZL_HAS_DEVICEUNIQUEID
	#define ZL_HAS_NATIVEZLIB
#elif defined(__WINDOWSPHONE__)
	#define ZL_VIDEO_DIRECT3D
	#ifndef ZL_USE_ENET
		#define ZL_USE_ENET
	#endif
	#define ZL_HAS_SOFTKEYBOARD
	#define _CRT_SECURE_NO_WARNINGS
#elif defined(__NACL__)
	#undef ZL_USE_ENET
	#define ZL_USE_PTHREAD
	#define ZL_HAS_FULLSCREEN
	#define ZL_HAS_POINTERLOCK
#elif defined(__WEBAPP__)
	#undef ZL_USE_ENET
	#define ZL_HAS_FULLSCREEN
	#define ZL_HAS_POINTERLOCK
#else
	#define ZL_USE_SDL
	#ifndef ZL_USE_ENET
		#define ZL_USE_ENET
	#endif
	#define ZL_USE_JOYSTICKINIT
	#define ZL_HAS_FULLSCREEN
	#define ZL_HAS_POINTERLOCK
#endif

#endif
