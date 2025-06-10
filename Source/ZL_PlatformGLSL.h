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

#ifndef __ZL_PLATFORM_GLSL__
#define __ZL_PLATFORM_GLSL__
#ifdef __cplusplus

#ifndef ZL_VIDEO_USE_GLSL
#error ZL_VIDEO_USE_GLSL needs to be defined
#endif

#ifdef ZL_VIDEO_OPENGL_ES2
#define ZLGLSL_LIST_HIGH_PRECISION_HEADER "precision highp float;",
#define ZLGLSL_LIST_LOW_PRECISION_HEADER "precision lowp float;",
#else
#define ZLGLSL_LIST_HIGH_PRECISION_HEADER
#define ZLGLSL_LIST_LOW_PRECISION_HEADER
#endif

#ifdef ZL_VIDEO_OPENGL_CORE
#define ZLGLSL_LIST_VS_HEADER "#version 140\n#define attribute in\n#define varying out\n",
#define ZLGLSL_LIST_FS_HEADER "#version 140\n#define varying in\n#define texture2D texture\n#define gl_FragColor fragColor\nout vec4 fragColor;\n",
#else
#define ZLGLSL_LIST_VS_HEADER
#define ZLGLSL_LIST_FS_HEADER
#endif

#if !defined(ZL_VIDEO_OPENGL_ES2) && !defined(ZL_VIDEO_OPENGL_CORE)
#define ZLGLSL_LIST_NULL_HEADER NULL,
#else
#define ZLGLSL_LIST_NULL_HEADER
#endif

namespace ZLGLSL
{
	typedef float GLSLscalar;

	enum eActiveProgram { NONE, COLOR, TEXTURE, CUSTOM, DISPLAY3D };
	extern eActiveProgram ActiveProgram;

	enum eAttributes { ATTR_POSITION, ATTR_COLOR, ATTR_TEXCOORD, _ATTR_MAX };

	extern GLuint UNI_MVP;

	void _COLOR_PROGRAM_ACTIVATE();
	void _TEXTURE_PROGRAM_ACTIVATE();
	inline void DisableProgram() { ActiveProgram = NONE; }
	inline void SelectColorProgram() { if (ActiveProgram != COLOR && ActiveProgram != CUSTOM) _COLOR_PROGRAM_ACTIVATE(); }
	inline void SelectTextureProgram() { if (ActiveProgram != TEXTURE && ActiveProgram != CUSTOM) _TEXTURE_PROGRAM_ACTIVATE(); }

	void MatrixPush();
	void MatrixPop();
	void MatrixIdentity();
	void MatrixTranslate(GLSLscalar x, GLSLscalar y);
	void MatrixScale(GLSLscalar x, GLSLscalar y);
	void MatrixRotate(GLSLscalar angle_rad);
	void MatrixRotate(GLSLscalar rcos, GLSLscalar rsin);
	void MatrixTransform(GLSLscalar tx, GLSLscalar ty, GLSLscalar rcos, GLSLscalar rsin);
	void MatrixTransformReverse(GLSLscalar tx, GLSLscalar ty, GLSLscalar rcos, GLSLscalar rsin);
	void MatrixOrtho(GLSLscalar left, GLSLscalar right, GLSLscalar bottom, GLSLscalar top);
	void LoadMatrix(const GLSLscalar* mtx);

	void Project(GLSLscalar& x, GLSLscalar& y);
	void Unproject(GLSLscalar& x, GLSLscalar& y);

	#ifndef ZL_VIDEO_DIRECT3D
	GLuint CreateProgramFromVertexAndFragmentShaders(GLsizei vertex_shader_srcs_count, const char*const* vertex_shader_srcs, GLsizei fragment_shader_srcs_count, const char*const* fragment_shader_srcs, GLsizei bind_attribs_count, const char*const* bind_attribs);
	bool CreateShaders();
	#endif
}

#endif //__cplusplus
#endif //__ZL_PLATFORM_GLSL__
