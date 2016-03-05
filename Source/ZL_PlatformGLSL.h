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

#ifndef __ZL_PLATFORM_GLSL__
#define __ZL_PLATFORM_GLSL__
#ifdef __cplusplus

namespace ZLGLSL
{
	typedef float GLSLscalar;

	enum eActiveProgram { NONE, COLOR, TEXTURE, CUSTOM };
	extern eActiveProgram ActiveProgram;

	extern GLuint UNI_MVP;
	extern GLuint UNI_TEXTURE;
	extern GLuint ATTR_POSITION;
	extern GLuint ATTR_COLOR;
	extern GLuint ATTR_TEXCOORD;

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

	void Project(GLSLscalar& x, GLSLscalar& y);
	void Unproject(GLSLscalar& x, GLSLscalar& y);

	#ifndef ZL_VIDEO_DIRECT3D
	bool CreateShaders();
	#endif
}

#endif //__cplusplus
#endif //__ZL_PLATFORM_GLSL__
