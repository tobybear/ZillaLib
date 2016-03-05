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

#ifndef __ZL_PLATFORM_WP__
#define __ZL_PLATFORM_WP__

#include <windows.h>
#include <synchapi.h>

//glwrapper
#define GL_COLOR_BUFFER_BIT               0x00004000
#define GL_VERSION                        0x1F02
#define GL_SRC_ALPHA                      0x0302
#define GL_ONE_MINUS_SRC_ALPHA            0x0303
#define GL_LEQUAL                         0x0203
#define GL_FLOAT                          0x1406
#define GL_UNSIGNED_SHORT                 0x1403
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_FALSE                          0
#define GL_TRUE                           1
#define GL_UNPACK_ALIGNMENT               0x0CF5
/* Enables */
#define GL_TEXTURE_2D                     0x0DE1
#define GL_SCISSOR_TEST                   0x0C11
#define GL_BLEND                          0x0BE2
#define GL_DEPTH_TEST                     0x0B71
#define GL_CULL_FACE                      0x0B44
/* DrawMode */
#define GL_POINTS                         0x0000
#define GL_LINES                          0x0001
#define GL_LINE_LOOP                      0x0002
#define GL_LINE_STRIP                     0x0003
#define GL_TRIANGLES                      0x0004
#define GL_TRIANGLE_STRIP                 0x0005
#define GL_TRIANGLE_FAN                   0x0006
/* TextureWrapMode */
#define GL_REPEAT                         0x2901
#define GL_CLAMP_TO_EDGE                  0x812F
/* PixelFormat */
#define GL_RGB                            0x1907
#define GL_RGBA                           0x1908
#define GL_LUMINANCE                      0x1909
#define GL_LUMINANCE_ALPHA                0x190A
/* glGets */
#define GL_MAX_TEXTURE_SIZE               0x0D33
#define GL_FRAMEBUFFER_BINDING            0x8CA6
/* TextureParameterName */
#define GL_TEXTURE_MAG_FILTER             0x2800
#define GL_TEXTURE_MIN_FILTER             0x2801
#define GL_TEXTURE_WRAP_S                 0x2802
#define GL_TEXTURE_WRAP_T                 0x2803
/* TextureMagFilter */
#define GL_NEAREST                        0x2600
#define GL_LINEAR                         0x2601

typedef unsigned int GLuint;
typedef int GLint;
typedef size_t GLsizei;
typedef int GLenum;
typedef float GLfloat;
typedef unsigned char GLubyte;
typedef int GLfixed;
typedef unsigned char GLboolean;
typedef void GLvoid;
typedef unsigned short GLushort;

void glClear(int mask);
void glClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
const GLubyte* glGetString(GLenum name);
void glGetIntegerv(GLenum pname, GLint* params);
void glScissor(int x, int y, int width, int height);
void glEnable(GLenum what);
void glDisable(GLenum what);
void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
void glDepthFunc(GLenum which);
void glDepthMask(GLenum which);
void glBlendFunc(GLenum sfactor, GLenum dfactor);
void glLineWidth(GLfloat width);
void glVertexAttrib4f(GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void glVertexAttrib4fv(GLuint indx, const GLfloat* values);
void glVertexAttribPointer(GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr);
void glEnableVertexAttribArray(GLuint index);
void glDisableVertexAttribArray(GLuint index);
void glDrawArrays(GLenum mode, GLint first, GLsizei count);
void glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices);
void glBindTexture(GLenum target, GLuint texture);
void glGenTextures(GLsizei n, GLuint* textures);
void glTexParameteri(GLenum target, GLenum pname, GLint param);
void glPixelStorei(GLenum pname, GLint param);
void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels);
void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
void glDeleteTextures(GLsizei n, const GLuint* textures);

#ifdef __cplusplus

//GLSL namespace
#include "ZL_PlatformGLSL.h"

//windows phone specific
void ZL_SoftKeyboardToggle();
void ZL_SoftKeyboardShow();
void ZL_SoftKeyboardHide();
bool ZL_SoftKeyboardIsShown();
void __wp8_log(const char* logtag, const char* logtext);
#define ZL_LOG_PRINT(LOGTAG, LOGTEXT) __wp8_log(LOGTAG, LOGTEXT)

//Thread
int ZL_CreateThread(void *(*start_routine) (void *p), void *arg);
void ZL_WaitThread(int hthread, int *pstatus);
typedef int ZL_ThreadHandle;
typedef HANDLE ZL_MutexHandle;
#define ZL_MutexLock(m) WaitForSingleObjectEx(m, INFINITE, FALSE)
#define ZL_MutexUnlock(m) ReleaseMutex(m)
#define ZL_MutexTryLock(m) (WaitForSingleObjectEx(m, 0, FALSE)==0)
#define ZL_MutexInit(m) m = CreateMutexEx(NULL, NULL, 0, SYNCHRONIZE)
#define ZL_MutexDestroy CloseHandle
#define ZL_MutexNone NULL
#define ZL_MutexIsNone(m) (m == NULL)

#endif //__cplusplus
#endif //__ZL_PLATFORM_WP__
