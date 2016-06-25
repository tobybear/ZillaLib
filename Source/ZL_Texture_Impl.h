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

#ifndef __ZL_TEXTURE_IMPL__
#define __ZL_TEXTURE_IMPL__

#include "ZL_File.h"
#include "ZL_Impl.h"
#include "ZL_Display.h"
#include "ZL_Display_Impl.h"
#include "ZL_Platform.h"
#include "ZL_File_Impl.h"

struct ZL_BitmapSurface
{
	int BytesPerPixel, w, h;
	unsigned char* pixels;
};

struct ZL_TextureFrameBuffer
{
	GLuint glFB;
	int viewport[4];
	unsigned char flags;
	ZL_TextureFrameBuffer* pPrevFrameBuffer;
	#ifdef ZL_VIDEO_WEAKCONTEXT
	void *pStorePixelData;
	#endif
};

struct ZL_Texture_Impl : ZL_Impl
{
	GLuint gltexid; // OpenGL texture name (texture id)
	GLenum format;  // The color format of the texture
	int w, h;       // The width and height of the original surface
	int wTex, hTex; // The actual size of the OpenGL texture (it might differ, power of two etc.)
	int wRep, hRep; // The size for GL_REPEAT wrap mode before the texture was resized to conform to power of two textures
	GLint wraps, wrapt, filtermin, filtermag;
	ZL_TextureFrameBuffer *pFrameBuffer;

	static ZL_Texture_Impl* LoadTextureRef(const ZL_FileLink& file, ZL_BitmapSurface* out_surface = NULL);

	void SetTextureFilter(GLint newfiltermin, GLint newfiltermag);
	void SetTextureWrap(GLint newwraps, GLint newwrapt);

	void FrameBufferBegin(bool clear);
	void FrameBufferEnd();

	bool LoadSurfaceAndTexture(const ZL_File& file, ZL_BitmapSurface* out_surface = NULL);
	void SetupFrameBuffer(int width, int height);

	ZL_Texture_Impl(int width, int height, bool use_alpha);

private:
	ZL_Texture_Impl(const ZL_File& file, ZL_BitmapSurface* out_surface);
	unsigned char* LoadSurfaceData(const ZL_File& file, ZL_BitmapSurface* out_surface);
	~ZL_Texture_Impl();
};

struct ZL_Surface_BatchRenderContext;

struct ZL_Surface_Impl : ZL_Impl
{
	ZL_Texture_Impl *tex;
	ZL_Surface_BatchRenderContext *pBatchRender;
	bool hasClipping;
	scalar fRotate, fScaleW, fScaleH, fOpacity;
	scalar fHCW, fHCH, fRSin, fRCos;
	ZL_Origin::Type orDraw, orRotate;
	ZL_Color color;
	GLscalar TexCoordBox[8];
	ZL_Surface_Impl(const ZL_FileLink& file);
	ZL_Surface_Impl(int width, int height, bool use_alpha);
	ZL_Surface_Impl(const ZL_Surface_Impl* src);
	~ZL_Surface_Impl();
	inline scalar GetHCW(const scalar scalew) { return (fScaleW == scalew ? fHCW : (fScaleW == -scalew ? -fHCW : (hasClipping ? fHCW*scalew/fScaleW : tex->w*scalew*s(.5)))); }
	inline scalar GetHCH(const scalar scaleh) { return (fScaleH == scaleh ? fHCH : (fScaleH == -scaleh ? -fHCH : (hasClipping ? fHCH*scaleh/fScaleH : tex->h*scaleh*s(.5)))); }
	void CalcContentSizes(const scalar scalew, const scalar scaleh);
	void CalcUnclippedTexCoordBoxAndContentSize();
	void CalcRotation(const scalar rotate);
	void Draw(scalar x, scalar y, const scalar rotate, const scalar hcw, const scalar hch, const scalar rsin, const scalar rcos, const ZL_Color &color);
	void DrawTo(const scalar x1, const scalar y1, const scalar x2, const scalar y2, scalar scalew, scalar scaleh, const ZL_Color &color);
	inline void DrawOrBatch(const ZL_Color &color, const GLscalar v1x, const GLscalar v1y, const GLscalar v2x, const GLscalar v2y, const GLscalar v3x, const GLscalar v3y, const GLscalar v4x, const GLscalar v4y, const GLscalar* texcoordbox);
};

#endif //__ZL_TEXTURE_IMPL__
