/*
  ZillaLib
  Copyright (C) 2010-2019 Bernhard Schelling

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

#if defined(__wasm__) || defined(__EMSCRIPTEN__)
#ifndef __ZL_PLATFORM_WEB__
#define __ZL_PLATFORM_WEB__

#include "ZL_PlatformPosix.h"
#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES
#include <GL/gl.h>

#ifdef __cplusplus

#include "ZL_PlatformGLSL.h"
extern bool ZLEM_EnabledVertexAttrib[ZLGLSL::_ATTR_MAX];
inline void glEnableVertexAttribArrayUnbuffered(GLuint i) { ZLEM_EnabledVertexAttrib[i] = true; glEnableVertexAttribArray(i); }
inline void glDisableVertexAttribArrayUnbuffered(GLuint i) { ZLEM_EnabledVertexAttrib[i] = false; glDisableVertexAttribArray(i); }
void glVertexAttribPointerUnbuffered(GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr);
void glDrawArraysUnbuffered(GLenum mode, GLint first, GLsizei count);
void glDrawElementsUnbuffered(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices);
#define ZL_VIDEO_GL_SEPARATE_UNBUFFERED_CALLS

//Display
void ZL_SetFullscreen(bool toFullscreen);
void ZL_SetPointerLock(bool doLockPointer);

//Thread (todo, currently unsupprted, ignore usage)
#define ZL_ThreadHandle int
#define ZL_CreateThread(p,a) 1
#define ZL_WaitThread(t,s)
#define ZL_MutexHandle int
#define ZL_MutexLock(m)
#define ZL_MutexUnlock(m)
#define ZL_MutexTryLock(m) true
#define ZL_MutexInit(m)
#define ZL_MutexDestroy(m)
#define ZL_MutexNone 0
#define ZL_MutexIsNone(m) false

#endif //__cplusplus
#endif //__ZL_PLATFORM_WEB__
#endif //defined(__wasm__) || defined(__EMSCRIPTEN__)
