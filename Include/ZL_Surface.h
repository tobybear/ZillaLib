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

#ifndef __ZL_SURFACE__
#define __ZL_SURFACE__

#if (defined(_MSC_VER) && !defined(WINAPI_FAMILY))
#pragma comment (lib, "opengl32.lib")
#endif

#include "ZL_File.h"
#include "ZL_Display.h"

struct ZL_Surface
{
	ZL_Surface();
	ZL_Surface(const ZL_FileLink& file);
	ZL_Surface(int width, int height, bool use_alpha = false); //can fail when framebuffers not supported by gpu
	~ZL_Surface();
	ZL_Surface(const ZL_Surface &source);
	ZL_Surface &operator =(const ZL_Surface &source);
	operator bool () const { return (impl!=NULL); }
	bool operator==(const ZL_Surface &b) const { return (impl==b.impl); }
	bool operator!=(const ZL_Surface &b) const { return (impl!=b.impl); }
	ZL_Surface Clone() const;

	int GetWidth() const;
	int GetHeight() const;
	ZL_Vector GetSize() const;
	ZL_Vector GetTileSize() const;
	ZL_Color& GetColor() const;
	scalar GetScaleW() const;
	scalar GetScaleH() const;
	ZL_Vector GetScale() const;
	ZL_Origin::Type GetDrawOrigin() const;
	ZL_Origin::Type GetRotateOrigin() const;
	scalar GetRotate() const;
	scalar GetRotateDeg() const;

	ZL_Surface& SetOrigin(ZL_Origin::Type Origin);
	ZL_Surface& SetDrawOrigin(ZL_Origin::Type DrawOrigin);
	ZL_Surface& SetRotateOrigin(ZL_Origin::Type RotateOrigin);
	ZL_Surface& SetRotate(scalar rotate_rad);
	ZL_Surface& AddRotate(scalar rotate_rad);
	ZL_Surface& SetRotateDeg(scalar rotate_deg);
	ZL_Surface& AddRotateDeg(scalar rotate_deg);
	ZL_Surface& SetScale(scalar scale);
	ZL_Surface& AddScale(scalar scale);
	ZL_Surface& SetScale(scalar scalew, scalar scaleh);
	ZL_Surface& AddScale(scalar scalew, scalar scaleh);
	ZL_Surface& SetScaleTo(scalar target_size);
	ZL_Surface& SetScaleTo(scalar target_width, scalar target_height);
	ZL_Surface& FlipH();
	ZL_Surface& FlipV();
	ZL_Surface& SetColor(const ZL_Color &color);
	ZL_Surface& SetAlpha(scalar opacity);
	ZL_Surface& AddAlpha(scalar opacity);

	ZL_Surface& SetClipping(ZL_Rect clip);
	ZL_Surface& SetClipping(ZL_Rectf clip);
	ZL_Surface& SetTilesetClipping(int ImageNumTileCols, int ImageNumTileRows);
	ZL_Surface& SetTilesetIndex(int Index);
	ZL_Surface& SetTilesetIndex(int IndexCol, int IndexRow);
	ZL_Surface& ClearClipping();
	bool HasClipping() const;
	ZL_Surface& SetTextureRepeatMode(bool TextureRepeat = true);
	ZL_Surface& SetTextureFilterMode(bool FilterLinearMin = false, bool FilterLinearMag = false);
	bool IsTextureRepeatMode() const;

	void BatchRenderBegin(bool SeperateColors = false, size_t BufferSize = 64);
	void BatchRenderEnd(bool DrawAndClear = true);
	bool BatchRenderActive();
	void BatchRenderDraw();

	void RenderToBegin(bool clear = false, bool set2DOrtho = true);
	void RenderToEnd();
	bool RenderToActive();

	void Draw(scalar x, scalar y);
	void Draw(const ZL_Vector &pos);
	void Draw(scalar x, scalar y, const ZL_Color &color);
	void Draw(const ZL_Vector &pos, const ZL_Color &color);
	void Draw(scalar x, scalar y, scalar scalew, scalar scaleh);
	void Draw(const ZL_Vector &pos, scalar scalew, scalar scaleh);
	void Draw(scalar x, scalar y, scalar scalew, scalar scaleh, const ZL_Color &color);
	void Draw(const ZL_Vector &pos, scalar scalew, scalar scaleh, const ZL_Color &color);
	void Draw(scalar x, scalar y, scalar rotate_rad);
	void Draw(const ZL_Vector &pos, scalar rotate_rad);
	void Draw(scalar x, scalar y, scalar rotate_rad, const ZL_Color &color);
	void Draw(const ZL_Vector &pos, scalar rotate_rad, const ZL_Color &color);
	void Draw(scalar x, scalar y, scalar rotate_rad, scalar scalew, scalar scaleh);
	void Draw(const ZL_Vector &pos, scalar rotate_rad, scalar scalew, scalar scaleh);
	void Draw(scalar x, scalar y, scalar rotate_rad, scalar scalew, scalar scaleh, const ZL_Color &color);
	void Draw(const ZL_Vector &pos, scalar rotate_rad, scalar scalew, scalar scaleh, const ZL_Color &color);

	void DrawTo(scalar x1, scalar y1, scalar x2, scalar y2);
	void DrawTo(scalar x1, scalar y1, scalar x2, scalar y2, const ZL_Color &color);
	void DrawTo(scalar x1, scalar y1, scalar x2, scalar y2, scalar scalew, scalar scaleh);
	void DrawTo(scalar x1, scalar y1, scalar x2, scalar y2, scalar scalew, scalar scaleh, const ZL_Color &color);
	void DrawTo(const ZL_Vector &pos, scalar width, scalar height);
	void DrawTo(const ZL_Vector &pos, scalar width, scalar height, const ZL_Color &color);
	void DrawTo(const ZL_Vector &pos, scalar width, scalar height, scalar scalew, scalar scaleh);
	void DrawTo(const ZL_Vector &pos, scalar width, scalar height, scalar scalew, scalar scaleh, const ZL_Color &color);
	void DrawTo(const ZL_Rectf &rec);
	void DrawTo(const ZL_Rectf &rec, const ZL_Color &color);
	void DrawTo(const ZL_Rectf &rec, scalar scalew, scalar scaleh);
	void DrawTo(const ZL_Rectf &rec, scalar scalew, scalar scaleh, const ZL_Color &color);

	void DrawQuad(const ZL_Vector &p1, const ZL_Vector &p2, const ZL_Vector &p3, const ZL_Vector &p4);
	void DrawQuad(const ZL_Vector &p1, const ZL_Vector &p2, const ZL_Vector &p3, const ZL_Vector &p4, const ZL_Color &color);
	void DrawQuad(scalar x1, scalar y1, scalar x2, scalar y2, scalar x3, scalar y3, scalar x4, scalar y4);
	void DrawQuad(scalar x1, scalar y1, scalar x2, scalar y2, scalar x3, scalar y3, scalar x4, scalar y4, const ZL_Color &color);

	void DrawBox(const scalar* VerticesBox, const scalar* TexCoordBox, const ZL_Color &color) const;

	private: struct ZL_Surface_Impl* impl;
};

#endif //__ZL_SURFACE__
