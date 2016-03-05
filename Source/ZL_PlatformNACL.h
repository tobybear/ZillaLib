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

#ifndef __ZL_PLATFORM_NACL__
#define __ZL_PLATFORM_NACL__

#include "ZL_PlatformPosix.h"
#include <GLES2/gl2.h>

#ifdef __cplusplus

//GLSL namespace
#include "ZL_PlatformGLSL.h"

//Display
void ZL_SetFullscreen(bool toFullscreen);
void ZL_SetPointerLock(bool doLockPointer);

//platform specific
#define vsnprintf __builtin_vsnprintf
void __nacl_log(const char* logtag, const char* logtext);
#define ZL_LOG_PRINT(LOGTAG, LOGTEXT) __nacl_log(LOGTAG, LOGTEXT)

#endif //__cplusplus
#endif //__ZL_PLATFORM_NACL__
