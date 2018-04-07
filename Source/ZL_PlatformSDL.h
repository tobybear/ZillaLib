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

#ifndef __ZL_PLATFORM_SDL__
#define __ZL_PLATFORM_SDL__

#include <SDL_opengl.h>

#ifdef __cplusplus

//GLSL namespace
#include "ZL_PlatformGLSL.h"

//Double precision
#include "ZL_Math.h"

//Display
void ZL_SetFullscreen(bool toFullscreen);
void ZL_SetPointerLock(bool doLockPointer);
bool ZL_InitJoystickSubSystem();
bool ZL_LoadReleaseDesktopDataBundle(const char* DataBundleFileName = NULL);
void ZL_UpdateTPFLimit();

//Thread
typedef void* ZL_ThreadHandle;
ZL_ThreadHandle ZL_CreateThread(void *(*start_routine) (void *p), void *arg);
void ZL_WaitThread(ZL_ThreadHandle thread, int *status);
typedef void* ZL_MutexHandle;
int ZL_MutexLock(ZL_MutexHandle mutex);
int ZL_MutexUnlock(ZL_MutexHandle mutex);
bool ZL_MutexTryLock(ZL_MutexHandle mutex);
ZL_MutexHandle ZL_CreateMutex();
void ZL_MutexDestroy(ZL_MutexHandle mutex);
#define ZL_MutexInit(m) (m = ZL_CreateMutex())
#define ZL_MutexNone NULL
#define ZL_MutexIsNone(m) (m == NULL)

//GL stuff
#ifndef __MACOSX__
extern PFNGLCREATESHADERPROC             glCreateShader;
extern PFNGLSHADERSOURCEPROC             glShaderSource;
extern PFNGLCOMPILESHADERPROC            glCompileShader;
extern PFNGLCREATEPROGRAMPROC            glCreateProgram;
extern PFNGLATTACHSHADERPROC             glAttachShader;
extern PFNGLDETACHSHADERPROC             glDetachShader;
extern PFNGLLINKPROGRAMPROC              glLinkProgram;
extern PFNGLUSEPROGRAMPROC               glUseProgram;
extern PFNGLGETSHADERIVPROC              glGetShaderiv;
extern PFNGLGETPROGRAMIVPROC             glGetProgramiv;
#ifdef ZILLALOG
extern PFNGLGETSHADERINFOLOGPROC         glGetShaderInfoLog;
extern PFNGLGETPROGRAMINFOLOGPROC        glGetProgramInfoLog;
#endif
extern PFNGLGETATTRIBLOCATIONPROC        glGetAttribLocation;
extern PFNGLBINDATTRIBLOCATIONPROC       glBindAttribLocation;
extern PFNGLVERTEXATTRIBPOINTERPROC      glVertexAttribPointer;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC  glEnableVertexAttribArray;
extern PFNGLGETUNIFORMLOCATIONPROC       glGetUniformLocation;
#if !defined(ZL_DOUBLE_PRECISCION)
extern PFNGLUNIFORMMATRIX4FVPROC         glUniformMatrix4fv;
extern PFNGLVERTEXATTRIB4FVPROC          glVertexAttrib4fv;
extern PFNGLVERTEXATTRIB4FPROC           glVertexAttrib4f;
extern PFNGLUNIFORM1FPROC                glUniform1f;
#else
extern PFNGLUNIFORMMATRIX4DVPROC         glUniformMatrix4dv;
extern PFNGLVERTEXATTRIB4DVPROC          glVertexAttrib4dv;
extern PFNGLVERTEXATTRIB4DPROC           glVertexAttrib4d;
extern PFNGLUNIFORM1DPROC                glUniform1d;
#endif
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
extern PFNGLDELETESHADERPROC             glDeleteShader;
extern PFNGLDELETEPROGRAMPROC            glDeleteProgram;
extern PFNGLISPROGRAMPROC                glIsProgram;
extern PFNGLBLENDEQUATIONSEPARATEPROC    glBlendEquationSeparate;
extern PFNGLBLENDFUNCSEPARATEPROC        glBlendFuncSeparate;
#ifndef GL_ATI_blend_equation_separate //On linux some OpenGL 2 functions are already defined in the OS GL.h, avoid redefinition
extern PFNGLBLENDCOLORPROC               glBlendColor;
extern PFNGLBLENDEQUATIONPROC            glBlendEquation;
#endif
#ifndef ZL_DISABLE_DISPLAY3D
#define ZL_REQUIRE_INIT3DGLEXTENSIONENTRIES
void ZL_Init3DGLExtensionEntries();
extern PFNGLGETACTIVEUNIFORMPROC         glGetActiveUniform;
#ifndef GL_ATI_blend_equation_separate //On linux some OpenGL 2 functions are already defined in the OS GL.h, avoid redefinition
extern PFNGLACTIVETEXTUREPROC            glActiveTexture;
#endif
extern PFNGLGENBUFFERSPROC               glGenBuffers;
extern PFNGLDELETEBUFFERSPROC            glDeleteBuffers;
extern PFNGLBINDBUFFERPROC               glBindBuffer;
extern PFNGLBUFFERDATAPROC               glBufferData;
#ifdef ZILLALOG
extern PFNGLGETBUFFERPARAMETERIVPROC     glGetBufferParameteriv;
extern PFNGLMAPBUFFERPROC                glMapBuffer;
extern PFNGLUNMAPBUFFERPROC              glUnmapBuffer;
#endif
extern PFNGLUNIFORM1IPROC                glUniform1i;
#if !defined(ZL_DOUBLE_PRECISCION)
extern PFNGLUNIFORM2FPROC                glUniform2f;
extern PFNGLUNIFORM3FPROC                glUniform3f;
extern PFNGLUNIFORM3FVPROC               glUniform3fv;
extern PFNGLUNIFORM4FPROC                glUniform4f;
#else
extern PFNGLUNIFORM2DPROC                glUniform2d;
extern PFNGLUNIFORM3DPROC                glUniform3d;
extern PFNGLUNIFORM3DVPROC               glUniform3dv;
extern PFNGLUNIFORM4DPROC                glUniform4d;
#endif
#endif
#endif
extern PFNGLGENFRAMEBUFFERSPROC          glGenFramebuffers;
extern PFNGLDELETEFRAMEBUFFERSPROC       glDeleteFramebuffers;
extern PFNGLBINDFRAMEBUFFERPROC          glBindFramebuffer;
extern PFNGLFRAMEBUFFERTEXTURE2DPROC     glFramebufferTexture2D;

#endif //__cplusplus
#endif //__ZL_PLATFORM_SDL__
