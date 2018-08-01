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

#include "ZL_Surface.h"
#include "ZL_Texture_Impl.h"
#include "ZL_Application.h"
#include <vector>
#include <string.h>
#include <assert.h>

struct ZL_Surface_BatchRenderContext
{
	std::vector<GLscalar> data;
	GLscalar *vertices_start, *vertices_cur, *texcoordbox_start, *texcoordbox_cur, *colors_start, *colors_cur;
	const ZL_Surface_Impl *srf;
	static std::vector<ZL_Surface_BatchRenderContext*> *ActiveContexts;

	ZL_Surface_BatchRenderContext(const ZL_Surface_Impl *srf) : vertices_start(NULL), srf(srf)
	{
		if (!ActiveContexts) ActiveContexts = new std::vector<ZL_Surface_BatchRenderContext*>();
	}

	void Begin(bool SeperateColors, size_t BufferSize)
	{
		assert(!vertices_start);
		BufferSize *= 6 * 2; //6 vertices (2 triangles) of 2 (verts/texcoords) or 2*2 (color) values
		data.resize(BufferSize * (SeperateColors ? 4 : 2));
		vertices_cur = vertices_start = &data[0];
		texcoordbox_cur = texcoordbox_start = vertices_start + BufferSize;
		colors_cur = colors_start = (SeperateColors ? texcoordbox_start + BufferSize : NULL);
		ActiveContexts->push_back(this);
	}

	void End(bool DrawAndClear)
	{
		assert(vertices_start);
		for (std::vector<ZL_Surface_BatchRenderContext*>::iterator it = ActiveContexts->begin(); it != ActiveContexts->end(); ++it)
			if (*it == this) { ActiveContexts->erase(it); break; }
		if (!DrawAndClear) return;
		if (vertices_cur != vertices_start) { Draw(); }
		vertices_start = NULL;
	}

	void Draw()
	{
		assert(vertices_start);
		glBindTexture(GL_TEXTURE_2D, srf->tex->gltexid);
		ZLGL_ENABLE_TEXTURE();
		if (colors_start) ZLGL_COLORARRAY_ENABLE();
		else ZLGL_COLORA(srf->color, srf->fOpacity);
		ZLGL_VERTEXTPOINTER(2, GL_SCALAR, 0, vertices_start);
		if (colors_start) ZLGL_COLORARRAY_POINTER(4, GL_SCALAR, 0, colors_start);
		ZLGL_TEXCOORDPOINTER(2, GL_SCALAR, 0, texcoordbox_start);
		glDrawArraysUnbuffered(GL_TRIANGLES, 0, (GLsizei)((vertices_cur-vertices_start)/2));
		if (colors_start) ZLGL_COLORARRAY_DISABLE();
	}

	void Add(const GLscalar *VerticesBox, const GLscalar* TexCoordBox, const ZL_Color* Color)
	{
		if (vertices_cur == texcoordbox_start)
		{
			for (std::vector<ZL_Surface_BatchRenderContext*>::iterator it = ActiveContexts->begin(); it != ActiveContexts->end(); ++it)
				(*it)->Draw();
			vertices_cur = vertices_start;
			texcoordbox_cur = texcoordbox_start;
			colors_cur = colors_start;
		}
		memcpy(&vertices_cur[0], VerticesBox+0, sizeof(GLscalar)*6);
		memcpy(&vertices_cur[6], VerticesBox+2, sizeof(GLscalar)*6);
		vertices_cur += 12;
		memcpy(&texcoordbox_cur[0], TexCoordBox+0, sizeof(GLscalar)*6);
		memcpy(&texcoordbox_cur[6], TexCoordBox+2, sizeof(GLscalar)*6);
		texcoordbox_cur += 12;
		if (colors_start)
		{
			assert(colors_cur < vertices_start+data.size());
			memcpy(&colors_cur[0], Color, sizeof(scalar)*4);
			colors_cur[3] *= srf->fOpacity;
			memcpy(&colors_cur[4], &colors_cur[0], sizeof(scalar)*4);
			memcpy(&colors_cur[8], &colors_cur[0], sizeof(scalar)*4);
			memcpy(&colors_cur[12], &colors_cur[0], sizeof(scalar)*12);
			colors_cur += 24;
		}
	}
};

std::vector<ZL_Surface_BatchRenderContext*> *ZL_Surface_BatchRenderContext::ActiveContexts = NULL;

ZL_Surface_Impl::ZL_Surface_Impl(const ZL_FileLink& file)
 : pBatchRender(NULL), hasClipping(false), fRotate(0), fScaleW(1), fScaleH(1), fOpacity(1), orDraw(ZL_Origin::BottomLeft), orRotate(ZL_Origin::Center), color(ZL_Color::White)
{
	tex = ZL_Texture_Impl::LoadTextureRef(file);
	if (!tex->gltexid) { tex->DelRef(); tex = NULL; return; }
	CalcUnclippedTexCoordBoxAndContentSize();
}

ZL_Surface_Impl::ZL_Surface_Impl(int width, int height, bool use_alpha)
 : pBatchRender(NULL), hasClipping(false), fRotate(0), fScaleW(1), fScaleH(1), fOpacity(1), orDraw(ZL_Origin::BottomLeft), orRotate(ZL_Origin::Center), color(ZL_Color::White)
{
	tex = new ZL_Texture_Impl(width, height, use_alpha);
	if (!tex->gltexid) { tex->DelRef(); tex = NULL; return; }
	CalcUnclippedTexCoordBoxAndContentSize();
}
ZL_Surface_Impl::ZL_Surface_Impl(ZL_Texture_Impl* tex)
 : tex(tex), pBatchRender(NULL), hasClipping(false), fRotate(0), fScaleW(1), fScaleH(1), fOpacity(1), orDraw(ZL_Origin::BottomLeft), orRotate(ZL_Origin::Center), color(ZL_Color::White)
{
	tex->AddRef();
	CalcUnclippedTexCoordBoxAndContentSize();
}
ZL_Surface_Impl::ZL_Surface_Impl(const ZL_Surface_Impl* src)
{
	memcpy((void*)this, (void*)src, sizeof(ZL_Surface_Impl));
	if (pBatchRender) pBatchRender = NULL;
	if (tex) tex->AddRef();
}

ZL_Surface_Impl::~ZL_Surface_Impl()
{
	if (tex) tex->DelRef();
	if (pBatchRender) delete pBatchRender;
}

void ZL_Surface_Impl::CalcContentSizes(const scalar scalew, const scalar scaleh)
{
	if (fScaleW == scalew && fScaleH == scaleh) return;
	fHCW = GetHCW(scalew);
	fHCH = GetHCH(scaleh);
	fScaleW = scalew;
	fScaleH = scaleh;
}

void ZL_Surface_Impl::CalcUnclippedTexCoordBoxAndContentSize()
{
	TexCoordBox[0] = TexCoordBox[4] = TexCoordBox[1] = TexCoordBox[3] = 0;
	TexCoordBox[5] = TexCoordBox[7] = (tex->hTex > tex->h ? tex->h / s(tex->hTex) : s(1));
	TexCoordBox[2] = TexCoordBox[6] = (tex->wTex > tex->w ? tex->w / s(tex->wTex) : s(1));
	fHCW = tex->w * fScaleW / 2;
	fHCH = tex->h * fScaleH / 2;
}

void ZL_Surface_Impl::CalcRotation(const scalar rotate)
{
	if (fRotate == rotate) return;
	if (rotate) { fRSin = ssin(rotate); fRCos = scos(rotate); }
	fRotate = rotate;
}

inline void ZL_Surface_Impl::DrawOrBatch(const ZL_Color &color, const GLscalar v1x, const GLscalar v1y, const GLscalar v2x, const GLscalar v2y, const GLscalar v3x, const GLscalar v3y, const GLscalar v4x, const GLscalar v4y, const GLscalar* texcoordbox)
{
	const GLscalar VerticesBox[8] = { v1x,v1y,v2x,v2y,v3x,v3y,v4x,v4y };
	if (pBatchRender && pBatchRender->vertices_start) pBatchRender->Add(VerticesBox, texcoordbox, &color);
	else
	{
		glBindTexture(GL_TEXTURE_2D, tex->gltexid);
		ZLGL_ENABLE_TEXTURE();
		ZLGL_COLORA(color, fOpacity);
		ZLGL_TEXCOORDPOINTER(2, GL_SCALAR, 0, texcoordbox);
		ZLGL_VERTEXTPOINTER(2, GL_SCALAR, 0, VerticesBox);
		glDrawArraysUnbuffered(GL_TRIANGLE_STRIP, 0, 4);
	}
}

void ZL_Surface_Impl::Draw(scalar x, scalar y, const scalar rotate, const scalar hcw, const scalar hch, scalar rsin, scalar rcos, const ZL_Color &color)
{
	switch (orDraw)
	{
		case ZL_Origin::Center: break;
		case ZL_Origin::TopLeft:      x+=hcw, y-=hch; break;
		case ZL_Origin::TopCenter:            y-=hch; break;
		case ZL_Origin::TopRight:     x-=hcw; y-=hch; break;
		case ZL_Origin::CenterLeft:   x+=hcw;         break;
		case ZL_Origin::CenterRight:  x-=hcw;         break;
		case ZL_Origin::BottomLeft:   x+=hcw; y+=hch; break;
		case ZL_Origin::BottomCenter:         y+=hch; break;
		case ZL_Origin::BottomRight:  x-=hcw, y+=hch; break;
		default:
			x -= hcw * (ZL_Origin::FromCustomGetX(orDraw)*s(2)-s(1));
			y += hcw * (ZL_Origin::FromCustomGetY(orDraw)*s(2)-s(1));
	}
	if (rotate == 0)
	{
		DrawOrBatch(color, x-hcw , y-hch , x+hcw , y-hch ,  x-hcw , y+hch, x+hcw , y+hch, TexCoordBox);
	}
	else
	{
		scalar cosW = rcos*hcw, sinW = rsin*hcw, sinH = rsin*hch, cosH = rcos*hch;
		switch (orRotate)
		{
			case ZL_Origin::Center: break;
			case ZL_Origin::TopLeft:      x+=-hcw+cosW+sinH; y+=+hch+sinW-cosH; break;
			case ZL_Origin::TopCenter:    x+=         +sinH; y+=+hch     -cosH; break;
			case ZL_Origin::TopRight:     x+=+hcw-cosW+sinH; y+=+hch-sinW-cosH; break;
			case ZL_Origin::CenterLeft:   x+=-hcw+cosW     ; y+=    +sinW     ; break;
			case ZL_Origin::CenterRight:  x+=+hcw-cosW     ; y+=    -sinW     ; break;
			case ZL_Origin::BottomLeft:   x+=-hcw+cosW-sinH, y+=-hch+sinW+cosH; break;
			case ZL_Origin::BottomCenter: x+=         -sinH; y+=-hch     +cosH; break;
			case ZL_Origin::BottomRight:  x+=+hcw-cosW-sinH; y+=-hch-sinW+cosH; break;
			default:
				rcos=(ZL_Origin::FromCustomGetX(orRotate)*s(2)-s(1));
				rsin=(ZL_Origin::FromCustomGetY(orRotate)*s(2)-s(1));
				x+=(+hcw-cosW)*rcos-sinH*rsin;
				y+=(-hch+cosH)*rsin-sinW*rcos;
		}
		DrawOrBatch(color, x-cosW+sinH, y-sinW-cosH, x+cosW+sinH, y+sinW-cosH, x-cosW-sinH, y-sinW+cosH, x+cosW-sinH, y+sinW+cosH, TexCoordBox);
	}
}

void ZL_Surface_Impl::DrawTo(const scalar x1, const scalar y1, const scalar x2, const scalar y2, scalar scalew, scalar scaleh, const ZL_Color &color)
{
	if (tex->wraps == GL_REPEAT/* || tex->wraps == GL_MIRRORED_REPEAT (not available on GLES)*/)
	{
		scalar orX, orY;
		if (orDraw >= ZL_Origin::_CUSTOM_START)
		{
			orX = ZL_Origin::FromCustomGetX(orDraw);
			orY = ZL_Origin::FromCustomGetY(orDraw);
		}
		else
		{
			orX = ((orDraw & ZL_Origin::_MASK_LEFT) ? s(0) : ((orDraw & ZL_Origin::_MASK_RIGHT)  ? s(1) : s(.5)));
			orY = ((orDraw & ZL_Origin::_MASK_TOP)  ? s(0) : ((orDraw & ZL_Origin::_MASK_BOTTOM) ? s(1) : s(.5)));
		}
		scalar w = (x1 - x2)/s(tex->wRep)/scalew, h = (y1 - y2)/s(tex->hRep)/scaleh;
		GLscalar RepeatTexCoordBox[8];
		RepeatTexCoordBox[0] = RepeatTexCoordBox[4] =  w * orX;
		RepeatTexCoordBox[2] = RepeatTexCoordBox[6] = -w * (s(1) - orX);
		RepeatTexCoordBox[5] = RepeatTexCoordBox[7] = -h * orY;
		RepeatTexCoordBox[1] = RepeatTexCoordBox[3] =  h * (s(1) - orY);
		DrawOrBatch(color, x1 , y1 , x2 , y1 , x1 , y2, x2 , y2, RepeatTexCoordBox);
	}
	else DrawOrBatch(color, x1 , y1 , x2 , y1 , x1 , y2, x2 , y2, TexCoordBox);
}

ZL_IMPL_OWNER_DEFAULT_IMPLEMENTATIONS(ZL_Surface)

ZL_Surface::ZL_Surface(const ZL_FileLink& file) : impl(new ZL_Surface_Impl(file))
{
	if (!impl->tex) { delete impl; impl = NULL; }
}

ZL_Surface::ZL_Surface(int width, int height, bool use_alpha) : impl(new ZL_Surface_Impl(width, height, use_alpha))
{
	if (!impl->tex) { delete impl; impl = NULL; }
}

ZL_Surface ZL_Surface::Clone() const
{
	ZL_Surface ret;
	if (impl) ret.impl = new ZL_Surface_Impl(impl);
	return ret;
}

int ZL_Surface::GetWidth() const { return impl ? impl->tex->wRep : 0; }
int ZL_Surface::GetHeight() const { return impl ? impl->tex->hRep : 0; }
ZL_Vector ZL_Surface::GetSize() const { return impl ? ZL_Vector(s(impl->tex->wRep), s(impl->tex->hRep)) : ZL_Vector(); }
ZL_Color& ZL_Surface::GetColor() const { return impl->color; }
scalar ZL_Surface::GetScaleW() const { return impl ? impl->fScaleW : 0; }
scalar ZL_Surface::GetScaleH() const { return impl ? impl->fScaleH : 0; }
ZL_Vector ZL_Surface::GetScale() const { return impl ? ZL_Vector(impl->fScaleW, impl->fScaleH) : ZL_Vector(); }
ZL_Origin::Type ZL_Surface::GetDrawOrigin() const { return impl ? impl->orDraw : ZL_Origin::BottomLeft; }
ZL_Origin::Type ZL_Surface::GetRotateOrigin() const { return impl ? impl->orRotate : ZL_Origin::Center; }
scalar ZL_Surface::GetRotate() const { return impl ? impl->fRotate : 0; }
scalar ZL_Surface::GetRotateDeg() const { return impl ? impl->fRotate*PIUNDER180 : 0; }

ZL_Surface& ZL_Surface::SetColor(const ZL_Color &color)                  { if (impl) impl->color = color; return *this; }
ZL_Surface& ZL_Surface::SetAlpha(scalar opacity)                         { if (impl) impl->fOpacity = opacity; return *this; }
ZL_Surface& ZL_Surface::AddAlpha(scalar opacity)                         { if (impl) impl->fOpacity += opacity; return *this; }
ZL_Surface& ZL_Surface::SetOrigin(ZL_Origin::Type Origin)                { if (impl) impl->orDraw = impl->orRotate = Origin; return *this; }
ZL_Surface& ZL_Surface::SetDrawOrigin(ZL_Origin::Type DrawOrigin)        { if (impl) impl->orDraw = DrawOrigin; return *this; }
ZL_Surface& ZL_Surface::SetRotateOrigin(ZL_Origin::Type RotateOrigin)    { if (impl) impl->orRotate = RotateOrigin; return *this; }
ZL_Surface& ZL_Surface::SetRotate(scalar rotate_rad)                     { if (impl) impl->CalcRotation(rotate_rad); return *this; }
ZL_Surface& ZL_Surface::AddRotate(scalar rotate_rad)                     { if (impl) impl->CalcRotation(impl->fRotate + rotate_rad); return *this; }
ZL_Surface& ZL_Surface::SetRotateDeg(scalar rotate_deg)                  { if (impl) impl->CalcRotation(rotate_deg*PIOVER180); return *this; }
ZL_Surface& ZL_Surface::AddRotateDeg(scalar rotate_deg)                  { if (impl) impl->CalcRotation(impl->fRotate + rotate_deg*PIOVER180); return *this; }
ZL_Surface& ZL_Surface::SetScale(scalar scale)                           { if (impl) impl->CalcContentSizes(scale, scale); return *this; }
ZL_Surface& ZL_Surface::AddScale(scalar scale)                           { if (impl) impl->CalcContentSizes(impl->fScaleW+scale, impl->fScaleH+scale); return *this; }
ZL_Surface& ZL_Surface::SetScale(scalar scalew, scalar scaleh)           { if (impl) impl->CalcContentSizes(scalew, scaleh); return *this; }
ZL_Surface& ZL_Surface::AddScale(scalar scalew, scalar scaleh)           { if (impl) impl->CalcContentSizes(impl->fScaleW+scalew, impl->fScaleH+scaleh); return *this; }
ZL_Surface& ZL_Surface::SetScaleTo(scalar target)                        { if (impl) impl->CalcContentSizes(target/impl->tex->w, target/impl->tex->h); return *this; }
ZL_Surface& ZL_Surface::SetScaleTo(scalar targetw, scalar targeth)       { if (impl) impl->CalcContentSizes(targetw/impl->tex->w, targeth/impl->tex->h); return *this; }
ZL_Surface& ZL_Surface::FlipH()                                          { if (impl) impl->CalcContentSizes(impl->fScaleW*-1, impl->fScaleH); return *this; }
ZL_Surface& ZL_Surface::FlipV()                                          { if (impl) impl->CalcContentSizes(impl->fScaleW, impl->fScaleH*-1); return *this; }

ZL_Vector ZL_Surface::GetTileSize() const
{
	if (!impl || !impl->hasClipping) return ZL_Vector(0, 0);
	int w = (int)((impl->TexCoordBox[2] > impl->TexCoordBox[0] ? (impl->TexCoordBox[2] - impl->TexCoordBox[0]) : (impl->TexCoordBox[0] - impl->TexCoordBox[2])) * impl->tex->w + 1.1f); if (!w) w = 1;
	int h = (int)((impl->TexCoordBox[1] > impl->TexCoordBox[5] ? (impl->TexCoordBox[1] - impl->TexCoordBox[5]) : (impl->TexCoordBox[5] - impl->TexCoordBox[1])) * impl->tex->h + 1.1f); if (!h) h = 1;
	return ZL_Vector(s(w), s(h));
}

ZL_Surface& ZL_Surface::SetClipping(ZL_Rect clip)
{
	if (!impl) return *this;
	impl->hasClipping = true;
	const int divisor = (impl->tex->filtermag == GL_NEAREST ? 12 : 2);
	impl->TexCoordBox[0] = impl->TexCoordBox[4] =        s(clip.left  *divisor+1) / (impl->tex->wTex*divisor);
	impl->TexCoordBox[1] = impl->TexCoordBox[3] = s(1) - s(clip.bottom*divisor-1) / (impl->tex->hTex*divisor);
	impl->TexCoordBox[2] = impl->TexCoordBox[6] =        s(clip.right *divisor-1) / (impl->tex->wTex*divisor);
	impl->TexCoordBox[5] = impl->TexCoordBox[7] = s(1) - s(clip.top   *divisor+1) / (impl->tex->hTex*divisor);
	impl->fHCW = impl->fScaleW * sabs(s(clip.Width()))  * s(.5);
	impl->fHCH = impl->fScaleH * sabs(s(clip.Height())) * s(.5);
	return *this;
}

ZL_Surface& ZL_Surface::SetClipping(ZL_Rectf clip)
{
	if (!impl) return *this;
	if (IsTextureRepeatMode()) SetTextureRepeatMode(false);
	impl->hasClipping = true;
	impl->TexCoordBox[0] = impl->TexCoordBox[4] = (impl->tex->wTex > impl->tex->w ? clip.left  * impl->tex->w / impl->tex->wTex : clip.left);
	impl->TexCoordBox[1] = impl->TexCoordBox[3] = (impl->tex->hTex > impl->tex->h ? clip.low   * impl->tex->h / impl->tex->hTex : clip.low);
	impl->TexCoordBox[2] = impl->TexCoordBox[6] = (impl->tex->wTex > impl->tex->w ? clip.right * impl->tex->w / impl->tex->wTex : clip.right);
	impl->TexCoordBox[5] = impl->TexCoordBox[7] = (impl->tex->hTex > impl->tex->h ? clip.high  * impl->tex->h / impl->tex->hTex : clip.high);
	impl->fHCW = impl->fScaleW * impl->tex->w * sabs(clip.Width())  * s(.5);
	impl->fHCH = impl->fScaleH * impl->tex->w * sabs(clip.Height()) * s(.5);
	return *this;
}

ZL_Surface& ZL_Surface::SetTilesetClipping(int ImageNumTileCols, int ImageNumTileRows)
{
	if (!impl || !ImageNumTileCols || !ImageNumTileRows) return *this;
	SetClipping(ZL_Rect(0, 0, impl->tex->w / ImageNumTileCols, impl->tex->h / ImageNumTileRows));
	return *this;
}

ZL_Surface& ZL_Surface::SetTilesetIndex(int Index)
{
	if (!impl || !impl->hasClipping) return *this;
	int w = (int)((impl->TexCoordBox[2] > impl->TexCoordBox[0] ? (impl->TexCoordBox[2] - impl->TexCoordBox[0]) : (impl->TexCoordBox[0] - impl->TexCoordBox[2])) * impl->tex->w + 1.1f); if (!w) w = 1;
	int h = (int)((impl->TexCoordBox[1] > impl->TexCoordBox[5] ? (impl->TexCoordBox[1] - impl->TexCoordBox[5]) : (impl->TexCoordBox[5] - impl->TexCoordBox[1])) * impl->tex->h + 1.1f); if (!h) h = 1;
	int cols = (w > impl->tex->w ? 1 : impl->tex->w / w);
	const int divisor = (impl->tex->filtermag == GL_NEAREST ? 12 : 2);
	GLscalar* tcb = impl->TexCoordBox;
	scalar left =        s(w * (Index % cols) *divisor+1) / (impl->tex->wTex*divisor); if (left != tcb[0]) { scalar tcw = tcb[2] - tcb[0]; tcb[2] = tcb[6] = (tcb[0] = tcb[4] = left) + tcw; }
	scalar top  = s(1) - s(h * (Index / cols) *divisor+1) / (impl->tex->hTex*divisor); if (top  != tcb[5]) { scalar tch = tcb[5] - tcb[1]; tcb[1] = tcb[3] = (tcb[5] = tcb[7] = top ) - tch; }
	return *this;
}

ZL_Surface& ZL_Surface::SetTilesetIndex(int IndexCol, int IndexRow)
{
	if (!impl || !impl->hasClipping) return *this;
	int w = (int)((impl->TexCoordBox[2] > impl->TexCoordBox[0] ? (impl->TexCoordBox[2] - impl->TexCoordBox[0]) : (impl->TexCoordBox[0] - impl->TexCoordBox[2])) * impl->tex->w + 1.1f); if (!w) w = 1;
	int h = (int)((impl->TexCoordBox[1] > impl->TexCoordBox[5] ? (impl->TexCoordBox[1] - impl->TexCoordBox[5]) : (impl->TexCoordBox[5] - impl->TexCoordBox[1])) * impl->tex->h + 1.1f); if (!h) h = 1;
	const int divisor = (impl->tex->filtermag == GL_NEAREST ? 12 : 2);
	GLscalar* tcb = impl->TexCoordBox;
	scalar left =        s(w * IndexCol *divisor+1) / (impl->tex->wTex*divisor); if (left != tcb[0]) { scalar tcw = tcb[2] - tcb[0]; tcb[2] = tcb[6] = (tcb[0] = tcb[4] = left) + tcw; }
	scalar top  = s(1) - s(h * IndexRow *divisor+1) / (impl->tex->hTex*divisor); if (top  != tcb[5]) { scalar tch = tcb[5] - tcb[1]; tcb[1] = tcb[3] = (tcb[5] = tcb[7] = top ) - tch; }
	return *this;
}

ZL_Surface& ZL_Surface::ClearClipping()
{
	if (!impl || !impl->hasClipping) return *this;
	impl->hasClipping = false;
	impl->CalcUnclippedTexCoordBoxAndContentSize();
	return *this;
}

bool ZL_Surface::HasClipping() const
{
	return (impl && impl->hasClipping);
}

ZL_Surface& ZL_Surface::SetTextureRepeatMode(bool TextureRepeat)
{
	if (impl && !impl->hasClipping) impl->tex->SetTextureWrap(TextureRepeat ? GL_REPEAT : GL_CLAMP_TO_EDGE, TextureRepeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
	return *this;
}

ZL_Surface& ZL_Surface::SetTextureFilterMode(bool FilterLinearMin, bool FilterLinearMag)
{
	if (impl) impl->tex->SetTextureFilter(FilterLinearMin ? GL_LINEAR : GL_NEAREST, FilterLinearMag ? GL_LINEAR : GL_NEAREST);
	return *this;
}

bool ZL_Surface::IsTextureRepeatMode() const
{
	return (impl && !impl->hasClipping && impl->tex->wraps == GL_REPEAT && impl->tex->wrapt == GL_REPEAT);
}

void ZL_Surface::BatchRenderBegin(bool SeperateColors, size_t BufferSize)
{
	if (!impl) return;
	if (!impl->pBatchRender) impl->pBatchRender = new ZL_Surface_BatchRenderContext(impl);
	impl->pBatchRender->Begin(SeperateColors, BufferSize);
}

void ZL_Surface::BatchRenderEnd(bool DrawAndClear)
{
	if (impl) impl->pBatchRender->End(DrawAndClear);
}

bool ZL_Surface::BatchRenderActive()
{
	return (impl && impl->pBatchRender && impl->pBatchRender->vertices_start);
}

void ZL_Surface::BatchRenderDraw()
{
	if (impl) impl->pBatchRender->Draw();
}

void ZL_Surface::RenderToBegin(bool clear, bool set2DOrtho)
{
	if (!impl || !impl->tex || !impl->tex->pFrameBuffer) return;
	impl->tex->FrameBufferBegin(clear);
	if (set2DOrtho) ZL_Display::PushOrtho(0, s(impl->tex->wRep), 0, s(impl->tex->hRep));
	impl->tex->pFrameBuffer->flags = (set2DOrtho ? 1 : 0);
}

void ZL_Surface::RenderToEnd()
{
	if (!impl || !impl->tex || !impl->tex->pFrameBuffer) return;
	impl->tex->FrameBufferEnd();
	if (impl->tex->pFrameBuffer->flags & 1) ZL_Display::PopOrtho(); //set2DOrtho
}

bool ZL_Surface::RenderToActive()
{
	return (impl && impl->tex && impl->tex->pFrameBuffer && (GLuint)active_framebuffer == impl->tex->pFrameBuffer->glFB);
}

void ZL_Surface::Draw(scalar x, scalar y)
{
	if (impl) impl->Draw(x, y, impl->fRotate, impl->fHCW, impl->fHCH, impl->fRSin, impl->fRCos, impl->color);
}
void ZL_Surface::Draw(const ZL_Vector &pos)
{
	if (impl) impl->Draw(pos.x, pos.y, impl->fRotate, impl->fHCW, impl->fHCH, impl->fRSin, impl->fRCos, impl->color);
}
void ZL_Surface::Draw(scalar x, scalar y, const ZL_Color &color)
{
	if (impl) impl->Draw(x, y, impl->fRotate, impl->fHCW, impl->fHCH, impl->fRSin, impl->fRCos, color);
}
void ZL_Surface::Draw(const ZL_Vector &pos, const ZL_Color &color)
{
	if (impl) impl->Draw(pos.x, pos.y, impl->fRotate, impl->fHCW, impl->fHCH, impl->fRSin, impl->fRCos, color);
}
void ZL_Surface::Draw(scalar x, scalar y, scalar scalew, scalar scaleh)
{
	if (impl) impl->Draw(x, y, impl->fRotate, impl->GetHCW(scalew), impl->GetHCH(scaleh), impl->fRSin, impl->fRCos, impl->color);
}
void ZL_Surface::Draw(const ZL_Vector &pos, scalar scalew, scalar scaleh)
{
	if (impl) impl->Draw(pos.x, pos.y, impl->fRotate, impl->GetHCW(scalew), impl->GetHCH(scaleh), impl->fRSin, impl->fRCos, impl->color);
}
void ZL_Surface::Draw(scalar x, scalar y, scalar scalew, scalar scaleh, const ZL_Color &color)
{
	if (impl) impl->Draw(x, y, impl->fRotate, impl->GetHCW(scalew), impl->GetHCH(scaleh), impl->fRSin, impl->fRCos, color);
}
void ZL_Surface::Draw(const ZL_Vector &pos, scalar scalew, scalar scaleh, const ZL_Color &color)
{
	if (impl) impl->Draw(pos.x, pos.y, impl->fRotate, impl->GetHCW(scalew), impl->GetHCH(scaleh), impl->fRSin, impl->fRCos, color);
}
void ZL_Surface::Draw(scalar x, scalar y, scalar rotate_rad)
{
	if (impl) impl->Draw(x, y, rotate_rad, impl->fHCW, impl->fHCH, ssin(rotate_rad), scos(rotate_rad), impl->color);
}
void ZL_Surface::Draw(const ZL_Vector &pos, scalar rotate_rad)
{
	if (impl) impl->Draw(pos.x, pos.y, rotate_rad, impl->fHCW, impl->fHCH, ssin(rotate_rad), scos(rotate_rad), impl->color);
}
void ZL_Surface::Draw(scalar x, scalar y, scalar rotate_rad, const ZL_Color &color)
{
	if (impl) impl->Draw(x, y, rotate_rad, impl->fHCW, impl->fHCH, ssin(rotate_rad), scos(rotate_rad), color);
}
void ZL_Surface::Draw(const ZL_Vector &pos, scalar rotate_rad, const ZL_Color &color)
{
	if (impl) impl->Draw(pos.x, pos.y, rotate_rad, impl->fHCW, impl->fHCH, ssin(rotate_rad), scos(rotate_rad), color);
}
void ZL_Surface::Draw(scalar x, scalar y, scalar rotate_rad, scalar scalew, scalar scaleh)
{
	if (impl) impl->Draw(x, y, rotate_rad, impl->GetHCW(scalew), impl->GetHCH(scaleh), ssin(rotate_rad), scos(rotate_rad), impl->color);
}
void ZL_Surface::Draw(const ZL_Vector &pos, scalar rotate_rad, scalar scalew, scalar scaleh)
{
	if (impl) impl->Draw(pos.x, pos.y, rotate_rad, impl->GetHCW(scalew), impl->GetHCH(scaleh), ssin(rotate_rad), scos(rotate_rad), impl->color);
}
void ZL_Surface::Draw(scalar x, scalar y, scalar rotate_rad, scalar scalew, scalar scaleh, const ZL_Color &color)
{
	if (impl) impl->Draw(x, y, rotate_rad, impl->GetHCW(scalew), impl->GetHCH(scaleh), ssin(rotate_rad), scos(rotate_rad), color);
}
void ZL_Surface::Draw(const ZL_Vector &pos, scalar rotate_rad, scalar scalew, scalar scaleh, const ZL_Color &color)
{
	if (impl) impl->Draw(pos.x, pos.y, rotate_rad, impl->GetHCW(scalew), impl->GetHCH(scaleh), ssin(rotate_rad), scos(rotate_rad), color);
}

void ZL_Surface::DrawTo(scalar x1, scalar y1, scalar x2, scalar y2)
{
	if (impl) impl->DrawTo(x1, y1, x2, y2, impl->fScaleW, impl->fScaleH, impl->color);
}
void ZL_Surface::DrawTo(scalar x1, scalar y1, scalar x2, scalar y2, const ZL_Color &color)
{
	if (impl) impl->DrawTo(x1, y1, x2, y2, impl->fScaleW, impl->fScaleH, color);
}
void ZL_Surface::DrawTo(scalar x1, scalar y1, scalar x2, scalar y2, scalar scalew, scalar scaleh)
{
	if (impl) impl->DrawTo(x1, y1, x2, y2, scalew, scaleh, impl->color);
}
void ZL_Surface::DrawTo(scalar x1, scalar y1, scalar x2, scalar y2, scalar scalew, scalar scaleh, const ZL_Color &color)
{
	if (impl) impl->DrawTo(x1, y1, x2, y2, scalew, scaleh, color);
}
void ZL_Surface::DrawTo(const ZL_Vector &pos, scalar width, scalar height)
{
	if (impl) impl->DrawTo(pos.x, pos.y, pos.x+width, pos.y+height, impl->fScaleW, impl->fScaleH, impl->color);
}
void ZL_Surface::DrawTo(const ZL_Vector &pos, scalar width, scalar height, const ZL_Color &color)
{
	if (impl) impl->DrawTo(pos.x, pos.y, pos.x+width, pos.y+height, impl->fScaleW, impl->fScaleH, color);
}
void ZL_Surface::DrawTo(const ZL_Vector &pos, scalar width, scalar height, scalar scalew, scalar scaleh)
{
	if (impl) impl->DrawTo(pos.x, pos.y, pos.x+width, pos.y+height, scalew, scaleh, impl->color);
}
void ZL_Surface::DrawTo(const ZL_Vector &pos, scalar width, scalar height, scalar scalew, scalar scaleh, const ZL_Color &color)
{
	if (impl) impl->DrawTo(pos.x, pos.y, pos.x+width, pos.y+height, scalew, scaleh, color);
}
void ZL_Surface::DrawTo(const ZL_Rectf &rec)
{
	if (impl) impl->DrawTo(rec.left, rec.low, rec.right, rec.high, impl->fScaleW, impl->fScaleH, impl->color);
}
void ZL_Surface::DrawTo(const ZL_Rectf &rec, const ZL_Color &color)
{
	if (impl) impl->DrawTo(rec.left, rec.low, rec.right, rec.high, impl->fScaleW, impl->fScaleH, color);
}
void ZL_Surface::DrawTo(const ZL_Rectf &rec, scalar scalew, scalar scaleh)
{
	if (impl) impl->DrawTo(rec.left, rec.low, rec.right, rec.high, scalew, scaleh, impl->color);
}
void ZL_Surface::DrawTo(const ZL_Rectf &rec, scalar scalew, scalar scaleh, const ZL_Color &color)
{
	if (impl) impl->DrawTo(rec.left, rec.low, rec.right, rec.high, scalew, scaleh, color);
}

void ZL_Surface::DrawQuad(const ZL_Vector &p1, const ZL_Vector &p2, const ZL_Vector &p3, const ZL_Vector &p4)
{
	if (impl) impl->DrawOrBatch(impl->color, p1.x, p1.y, p2.x, p2.y, p4.x, p4.y, p3.x, p3.y, impl->TexCoordBox);
}
void ZL_Surface::DrawQuad(const ZL_Vector &p1, const ZL_Vector &p2, const ZL_Vector &p3, const ZL_Vector &p4, const ZL_Color &color)
{
	if (impl) impl->DrawOrBatch(color, p1.x, p1.y, p2.x, p2.y, p4.x, p4.y, p3.x, p3.y, impl->TexCoordBox);
}
void ZL_Surface::DrawQuad(scalar x1, scalar y1, scalar x2, scalar y2, scalar x3, scalar y3, scalar x4, scalar y4)
{
	if (impl) impl->DrawOrBatch(impl->color, x1, y1, x2, y2, x4, y4, x3, y3, impl->TexCoordBox);
}
void ZL_Surface::DrawQuad(scalar x1, scalar y1, scalar x2, scalar y2, scalar x3, scalar y3, scalar x4, scalar y4, const ZL_Color &color)
{
	if (impl) impl->DrawOrBatch(color, x1, y1, x2, y2, x4, y4, x3, y3, impl->TexCoordBox);
}

void ZL_Surface::DrawBox(const scalar* VerticesBox, const scalar* TexCoordBox, const ZL_Color &color) const
{
	if (!impl) return;
	if (impl->pBatchRender && impl->pBatchRender->vertices_start) { impl->pBatchRender->Add(VerticesBox, TexCoordBox, &color); return; }
	glBindTexture(GL_TEXTURE_2D, impl->tex->gltexid);
	ZLGL_ENABLE_TEXTURE();
	ZLGL_COLOR(color);
	ZLGL_TEXCOORDPOINTER(2, GL_SCALAR, 0, TexCoordBox);
	ZLGL_VERTEXTPOINTER(2, GL_SCALAR, 0, VerticesBox);
	glDrawArraysUnbuffered(GL_TRIANGLE_STRIP, 0, 4);
}
