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

#ifndef __ZL_DISPLAY_IMPL__
#define __ZL_DISPLAY_IMPL__

#include "ZL_Math.h"
#include "ZL_Platform.h"

#if defined(ZL_VIDEO_USE_GLSL)
	#define GLTRANSLATE(x,y) ZLGLSL::MatrixTranslate(x,y)
	#define GLROTATEDEG(a) ZLGLSL::MatrixRotate(a/PIUNDER180)
	#define GLROTATERAD(a) ZLGLSL::MatrixRotate(a)
	#define GLROTATEXY(rx,ry) ZLGLSL::MatrixRotate(rx, ry)
	#define GLTRANSFORMXYROTXY(x,y,rx,ry) ZLGLSL::MatrixTransform(x,y,rx,ry)
	#define GLTRANSFORMREVXYROTXY(x,y,rx,ry) ZLGLSL::MatrixTransformReverse(x,y,rx,ry)
	#define GLSCALE(x,y) ZLGLSL::MatrixScale(x,y)
#else
	#if !defined(ZL_DOUBLE_PRECISCION)
		#define GLTRANSLATE(x,y) glTranslatef(x,y,0)
		#define GLROTATEDEG(a) glRotatef(a, 0, 0, 1)
		#define GLROTATERAD(a) glRotatef(a*PIUNDER180, 0, 0, 1)
		#define GLSCALE(x,y) glScalef(x,y,0)
	#else
		#define GLTRANSLATE(x,y) glTranslated(x,y,0)
		#define GLROTATEDEG(a) glRotated(a, 0, 0, 1)
		#define GLROTATERAD(a) glRotated(a*PIUNDER180, 0, 0, 1)
		#define GLSCALE(x,y) glScaled(x,y,0)
	#endif
	#define GLROTATEXY(rx,ry) GLROTATERAD(satan2(ry,rx))
	#define GLTRANSFORMXYROTXY(x,y,rx,ry) { GLTRANSLATE(x,y); GLROTATERAD(satan2(ry,rx)); }
	#define GLTRANSFORMREVXYROTXY(x,y,rx,ry) { GLROTATERAD(satan2(-ry,-rx)); GLTRANSLATE(-x,-y); }
#endif

#if !defined(ZL_DOUBLE_PRECISCION)
	typedef GLfloat GLscalar;
	#define GL_SCALAR GL_FLOAT
	#if defined(ZL_VIDEO_USE_GLSL)
		#define glVertexAttrib4 glVertexAttrib4f
		#define glVertexAttrib4v glVertexAttrib4fv
		#define glUniformMatrix4v glUniformMatrix4fv
		#define glUniform1 glUniform1f
	#else
		#define glVertex2v glVertex2fv
		#define glVertex2 glVertex2f
		#define glTexCoord2v glTexCoord2fv
		#define glTexCoord2 glTexCoord2f
		#define glColor4 glColor4f
	#endif
#else
	typedef GLdouble GLscalar;
	#define GL_SCALAR GL_DOUBLE
	#if defined(ZL_VIDEO_USE_GLSL)
		#define glVertexAttrib4 glVertexAttrib4d
		#define glVertexAttrib4v glVertexAttrib4dv
		#define glUniformMatrix4v glUniformMatrix4dv
		#define glUniform1 glUniform1d
	#else
		#define glVertex2v glVertex2dv
		#define glVertex2 glVertex2d
		#define glTexCoord2v glTexCoord2dv
		#define glTexCoord2 glTexCoord2d
		#define glColor4 glColor4d
	#endif
#endif

#if defined(ZL_VIDEO_USE_GLSL)
	#define ZLGL_DISABLE_PROGRAM() ZLGLSL::DisableProgram() //disables automatic propagation of matrix updates onto GPU until next program is selected
	#define ZLGL_DISABLE_TEXTURE() ZLGLSL::SelectColorProgram()
	#define ZLGL_ENABLE_TEXTURE() ZLGLSL::SelectTextureProgram()
	#define ZLGL_COLOR(x)       glVertexAttrib4v(ZLGLSL::ATTR_COLOR, (GLscalar*)&x)
	#define ZLGL_COLORA(x,al)   glVertexAttrib4(ZLGLSL::ATTR_COLOR, x.r, x.g, x.b, x.a * al)
	#define ZLGL_VERTEXTPOINTER(size, type, stride, ptr)     glVertexAttribPointer(ZLGLSL::ATTR_POSITION, size, type, GL_FALSE, stride, ptr)
	#define ZLGL_TEXCOORDPOINTER(size, type, stride, ptr)    glVertexAttribPointer(ZLGLSL::ATTR_TEXCOORD, size, type, GL_FALSE, stride, ptr)
	#define ZLGL_COLORARRAY_POINTER(size, type, stride, ptr) glVertexAttribPointer(ZLGLSL::ATTR_COLOR, size, type, GL_FALSE, stride, ptr)
	#define ZLGL_COLORARRAY_ENABLE()  glEnableVertexAttribArray(ZLGLSL::ATTR_COLOR)
	#define ZLGL_COLORARRAY_DISABLE() glDisableVertexAttribArray(ZLGLSL::ATTR_COLOR)
	#define GLPUSHMATRIX() ZLGLSL::MatrixPush()
	#define GLPOPMATRIX() ZLGLSL::MatrixPop()
	#define GLORTHO(r,l,b,t) ZLGLSL::MatrixOrtho(r,l,b,t)
	#define GLLOADIDENTITY() ZLGLSL::MatrixIdentity()
#else
	#define ZLGL_DISABLE_PROGRAM()
	#define ZLGL_DISABLE_TEXTURE() glDisable(GL_TEXTURE_2D)
	#define ZLGL_ENABLE_TEXTURE() glEnable(GL_TEXTURE_2D)
	#define ZLGL_COLOR(x)     glColor4(x.r, x.g, x.b, x.a)
	#define ZLGL_COLORA(x,al) glColor4(x.r, x.g, x.b, x.a * al)
	#define ZLGL_VERTEXTPOINTER(size, type, stride, ptr)     glVertexPointer(size, type, stride, ptr)
	#define ZLGL_TEXCOORDPOINTER(size, type, stride, ptr)    glTexCoordPointer(size, type, stride, ptr)
	#define ZLGL_COLORARRAY_POINTER(size, type, stride, ptr) glColorPointer(size, type, stride, ptr)
	#define ZLGL_COLORARRAY_ENABLE()   glEnableClientState(GL_COLOR_ARRAY)
	#define ZLGL_COLORARRAY_DISABLE()  glDisableClientState(GL_COLOR_ARRAY)
	#define GLPUSHMATRIX() glPushMatrix()
	#define GLPOPMATRIX() glPopMatrix()
	#define GLORTHO(r,l,b,t) glOrtho(r,l,b,t, -1.0f, 1.0f)
	#define GLLOADIDENTITY() glLoadIdentity()
#endif

#ifdef ZL_VIDEO_OPENGL_ES1
	#define glOrtho glOrthof
	#define glFrustum glFrustumf
	#define GL_FRAMEBUFFER_BINDING GL_FRAMEBUFFER_BINDING_OES
	#define glGenFramebuffers glGenFramebuffersOES
	#define glBindFramebuffer glBindFramebufferOES
	#define GL_FRAMEBUFFER GL_FRAMEBUFFER_OES
	#define glFramebufferTexture2D glFramebufferTexture2DOES
	#define GL_COLOR_ATTACHMENT0 GL_COLOR_ATTACHMENT0_OES
	#define glDeleteFramebuffers glDeleteFramebuffersOES
#endif

#ifdef __EMSCRIPTEN__ //because IEWebGL does not like glVertexAttrib4fv
	#undef ZLGL_COLOR
	#define ZLGL_COLOR(x) glVertexAttrib4f(ZLGLSL::ATTR_COLOR, x.r, x.g, x.b, x.a)
#endif

#ifdef __cplusplus

#include "ZL_Events.h"

#define ZL_WINDOWFLAGS_HAS(flg) ((*pZL_WindowFlags) & (flg))

struct ZL_Event
{
	static inline ZL_Event Make(unsigned int type) { ZL_Event e; e.type = type; return e; }
	struct ZL_TextInputEvent { char text[32]; };
	struct ZL_WindowEvent { unsigned char event; int data1; int data2; };
	unsigned int type;
	union
	{
		ZL_PointerMoveEvent motion;
		ZL_PointerPressEvent button;
		ZL_MouseWheelEvent wheel;
		ZL_KeyboardEvent key;
		ZL_TextInputEvent text;
		ZL_JoyAxisEvent jaxis;
		ZL_JoyBallEvent jball;
		ZL_JoyHatEvent jhat;
		ZL_JoyButtonEvent jbutton;
		ZL_WindowEvent window;
	};
};

enum ZL_WindowFlags
{
	ZL_WINDOW_FULLSCREEN            = 0x00000001,
	ZL_WINDOW_RESIZABLE             = 0x00000020,
	ZL_WINDOW_MINIMIZED             = 0x00000040,
	ZL_WINDOW_INPUT_FOCUS           = 0x00000200,
	ZL_WINDOW_MOUSE_FOCUS           = 0x00000400,
	ZL_WINDOW_ALLOWRESIZEHORIZONTAL = 0x00001000,
	ZL_WINDOW_ALLOWRESIZEVERTICAL   = 0x00002000,
	ZL_WINDOW_ALLOWANYORIENTATION   = 0x00004000,
	ZL_WINDOW_PREVENTALTENTER       = 0x00008000,
	ZL_WINDOW_PREVENTALTF4          = 0x00010000,
	ZL_WINDOW_POINTERLOCK           = 0x00020000,
	ZL_WINDOW_DEPTHBUFFER           = 0x00040000
};

enum ZL_EventNumber
{
	ZL_EVENT_NONE = 0,
	ZL_EVENT_MOUSEMOTION ,
	ZL_EVENT_MOUSEBUTTONDOWN,
	ZL_EVENT_MOUSEBUTTONUP,
	ZL_EVENT_MOUSEWHEEL,
	ZL_EVENT_KEYDOWN,
	ZL_EVENT_KEYUP,
	ZL_EVENT_TEXTINPUT,
	ZL_EVENT_JOYAXISMOTION,
	ZL_EVENT_JOYBALLMOTION,
	ZL_EVENT_JOYHATMOTION,
	ZL_EVENT_JOYBUTTONDOWN,
	ZL_EVENT_JOYBUTTONUP,
	ZL_EVENT_QUIT,
	ZL_EVENT_WINDOW,
	ZL_WINDOWEVENT_RESIZED,
	ZL_WINDOWEVENT_MINIMIZED,
	ZL_WINDOWEVENT_RESTORED,
	ZL_WINDOWEVENT_MOVED,
	ZL_WINDOWEVENT_CLOSE,
	ZL_WINDOWEVENT_FOCUS,
	_ZL_EVENT_MAX
};

#ifdef ZL_VIDEO_WEAKCONTEXT
#ifdef ZL_VIDEO_USE_GLSL
bool CheckProgramsIfContextLost();
#else
bool CheckTexturesIfContextLost();
bool CheckFontTexturesIfContextLost();
#endif
#ifdef ZL_VIDEO_USE_GLSL
void RecreateAllProgramsOnContextLost();
#endif
void RecreateAllTexturesOnContextLost();
void RecreateAllFontTexturesOnContextLost();
void StoreAllFrameBufferTexturesOnDeactivate();
#endif

#endif //__cplusplus

#endif //__ZL_DISPLAY_IMPL__
