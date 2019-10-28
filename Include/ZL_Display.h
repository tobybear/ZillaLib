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

#ifndef __ZL_DISPLAY__
#define __ZL_DISPLAY__

#if (defined(_MSC_VER) && !defined(WINAPI_FAMILY))
#pragma comment (lib, "opengl32.lib")
#endif

#include "ZL_String.h"
#include "ZL_Signal.h"
#include "ZL_Math.h"
#include "ZL_Application.h"
#include "ZL_Events.h"
#include <vector>

//Display size related helper macros
#define ZLWIDTH    ZL_Display::Width
#define ZLHALFW    (ZL_Display::Width*s(.5))
#define ZLFROMW(x) (ZL_Display::Width-s(x))
#define ZLHEIGHT   ZL_Display::Height
#define ZLHALFH    (ZL_Display::Height*s(.5))
#define ZLFROMH(y) (ZL_Display::Height-s(y))
#define ZLASPECTR  (ZL_Display::Width/ZL_Display::Height)
#define ZLCENTER   ZL_Vector(ZLHALFW, ZLHALFH)

//Color related helper macros
#define ZLTRANSPARENT         ZL_Color::Transparent
#define ZLBLACK               ZL_Color::Black
#define ZLWHITE               ZL_Color::White
#define ZLOPAQUE              ZL_Color::White
#define ZLRGB(r,g,b)          ZL_Color(s(r), s(g), s(b))
#define ZLRGBA(r,g,b,a)       ZL_Color(s(r), s(g), s(b), s(a))
#define ZLRGBX(x)             ZL_Color(s((x>>16)&0xFF)/s(255), s((x>>8)&0xFF)/s(255), s(x&0xFF)/s(255))
#define ZLRGBAX(x)            ZL_Color(s((x>>24)&0xFF)/s(255), s((x>>16)&0xFF)/s(255), s((x>>8)&0xFF)/s(255), s(x&0xFF)/s(255))
#define ZLRGBFF(r,g,b)        ZL_Color(s(r)/s(255), s(g)/s(255), s(b)/s(255))
#define ZLRGBAFF(r,g,b,a)     ZL_Color(s(r)/s(255), s(g)/s(255), s(b)/s(255), s(a)/s(255))
#define ZLALPHA(a)            ZL_Color(1, 1, 1, s(a))
#define ZLLUM(lum)            ZL_Color(s(lum), s(lum), s(lum))
#define ZLLUMA(lum,a)         ZL_Color(s(lum), s(lum), s(lum), s(a))
#define ZLCOLA(col,a)         ZL_Color(col.r, col.g, col.b, s(a))
#define ZLHSV(hue,sat,val)    ZL_Color::HSVA(s(hue), s(sat), s(val))
#define ZLHSVA(hue,sat,val,a) ZL_Color::HSVA(s(hue), s(sat), s(val), s(a))

namespace ZL_Origin
{
	enum Type
	{
		_MASK_CENTER = 1, _MASK_LEFT = 2, _MASK_RIGHT = 4, _MASK_TOP = 8, _MASK_BOTTOM = 16,
		TopLeft      = _MASK_TOP    | _MASK_LEFT,
		TopCenter    = _MASK_TOP    | _MASK_CENTER,
		TopRight     = _MASK_TOP    | _MASK_RIGHT,
		CenterLeft   = _MASK_CENTER | _MASK_LEFT,
		Center       = _MASK_CENTER | _MASK_CENTER,
		CenterRight  = _MASK_CENTER | _MASK_RIGHT,
		BottomLeft   = _MASK_BOTTOM | _MASK_LEFT,
		BottomCenter = _MASK_BOTTOM | _MASK_CENTER,
		BottomRight  = _MASK_BOTTOM | _MASK_RIGHT,
		_CUSTOM_START = 0x20, _CUSTOM_END = _CUSTOM_START+32767*32767-1
	};
	//To make a custom origin offset, enter x and y values from 0.0 (fully top/left aligned) to 1.0 (fully bottom/right aligned)
	static inline Type Custom(scalar x, scalar y) { return (Type)(_CUSTOM_START + (int)(x*32766) + 32767*(int)(y*32766)); }
	static inline Type Custom(ZL_Vector vec) { return Custom(vec.x, vec.y); }
	static inline scalar FromCustomGetX(Type custom) { return s(((int)custom-_CUSTOM_START)%32767)/s(32766); }
	static inline scalar FromCustomGetY(Type custom) { return s(((int)custom-_CUSTOM_START)/32767)/s(32766); }
	static inline ZL_Vector FromCustom(Type custom) { return ZL_Vector(FromCustomGetX(custom), FromCustomGetY(custom)); }
};

//Integer based rectangle (uses top/bottom, used for image file based coordinates where position 0,0 is top-left)
struct ZL_Rect
{
	ZL_Rect() : left(0), top(0), right(0), bottom(0) { }
	ZL_Rect(int left, int top, int right, int bottom) : left(left), top(top), right(right), bottom(bottom) { }
	ZL_Rect(const ZL_Point &p, int width, int height) : left(p.x), top(p.y), right(p.x+width), bottom(p.y+height) { }
	ZL_Rect(const ZL_Rect &rect) : left(rect.left), top(rect.top), right(rect.right), bottom(rect.bottom) { }
	ZL_Rect &operator+=(const ZL_Rect &r) { left += r.left; top += r.top; right += r.right; bottom += r.bottom; return *this; }
	ZL_Rect &operator-=(const ZL_Rect &r) { left -= r.left; top -= r.top; right -= r.right; bottom -= r.bottom; return *this; }
	ZL_Rect &operator+=(const ZL_Point &p) { left += p.x; top += p.y; right += p.x; bottom += p.y; return *this; }
	ZL_Rect &operator-=(const ZL_Point &p) { left -= p.x; top -= p.y; right -= p.x; bottom -= p.y; return *this; }
	ZL_Rect operator+(const ZL_Rect &r) const { return ZL_Rect(left + r.left, top + r.top, right + r.right, bottom + r.bottom); }
	ZL_Rect operator-(const ZL_Rect &r) const { return ZL_Rect(left - r.left, top - r.top, right - r.right, bottom - r.bottom); }
	ZL_Rect operator+(const ZL_Point &p) const { return ZL_Rect(left + p.x, top + p.y, right + p.x, bottom + p.y); }
	ZL_Rect operator-(const ZL_Point &p) const { return ZL_Rect(left - p.x, top - p.y, right - p.x, bottom - p.y); }
	bool operator==(const ZL_Rect &r) const { return (left == r.left && top == r.top && right == r.right && bottom == r.bottom); }
	bool operator!=(const ZL_Rect &r) const { return (left != r.left || top != r.top || right != r.right || bottom != r.bottom); }
	bool operator!() const { return (!left && !top && !right && !bottom); }

	int left,top,right,bottom;
	void SetSize(int width, int height) { right = left + width; bottom = top + height; }
	inline int Width() const { return right - left; }
	inline int Height() const { return bottom - top; }
	bool Overlaps(const ZL_Rect &r) const { return (r.left <= right && r.right >= left && r.top <= bottom && r.bottom >= top); }
	bool Contains(const ZL_Point &p) const { return (p.x >= left && p.y >= top && p.x <= right && p.y <= bottom); }
	bool Contains(const ZL_Rect &r) const { return (r.right <= right && r.left >= left && r.bottom <= bottom && r.top >= top); }
};

//Float based rectangle (uses low/high for vertical, used for screen based coordinates where position 0,0 is bottom-left)
struct ZL_Rectf
{
	static ZL_Rectf BySize(scalar x, scalar y, scalar width, scalar height) { return ZL_Rectf(x, y, x+width, y+height); }
	static ZL_Rectf BySize(const ZL_Vector &pos, const ZL_Vector &size) { return ZL_Rectf(pos.x, pos.y, pos.x+size.x, pos.y+size.y); }
	static ZL_Rectf ByCorners(const ZL_Vector &left_low, const ZL_Vector &right_high) { return ZL_Rectf(left_low.x, left_low.y, right_high.x, right_high.y); }
	static ZL_Rectf FromCenter(scalar x, scalar y, scalar extend_x, scalar extend_y) { return ZL_Rectf(x-extend_x, y-extend_y, x+extend_x, y+extend_y); }
	ZL_Rectf() : left(0), low(0), right(0), high(0) { }
	ZL_Rectf(scalar left, scalar low, scalar right, scalar high) : left(left), low(low), right(right), high(high) { }
	ZL_Rectf(const ZL_Vector &p, scalar width, scalar height) : left(p.x), low(p.y), right(p.x+width), high(p.y+height) { }
	ZL_Rectf(const ZL_Vector &p, const ZL_Vector &extends) : left(p.x-extends.x), low(p.y-extends.y), right(p.x+extends.x), high(p.y+extends.y) { }
	ZL_Rectf(const ZL_Vector &p, scalar extendrad) : left(p.x-extendrad), low(p.y-extendrad), right(p.x+extendrad), high(p.y+extendrad) { }
	ZL_Rectf(scalar x, scalar y, const ZL_Vector &extends) : left(x-extends.x), low(y-extends.y), right(x+extends.x), high(y+extends.y) { }
	ZL_Rectf(scalar x, scalar y, scalar extendrad) : left(x-extendrad), low(y-extendrad), right(x+extendrad), high(y+extendrad) { }
	ZL_Rectf(const ZL_Rectf &rect) : left(rect.left), low(rect.low), right(rect.right), high(rect.high) { }
	ZL_Rectf(const ZL_Rect &rect) : left(s(rect.left)), low(s(rect.top)), right(s(rect.right)), high(s(rect.bottom)) { }
	ZL_Rectf &operator+=(const ZL_Rectf &r) { left += r.left; low += r.low; right += r.right; high += r.high; return *this; }
	ZL_Rectf &operator-=(const ZL_Rectf &r) { left -= r.left; low -= r.low; right -= r.right; high -= r.high; return *this; }
	ZL_Rectf &operator+=(const ZL_Vector &p) { left += p.x; low += p.y; right += p.x; high += p.y; return *this; }
	ZL_Rectf &operator-=(const ZL_Vector &p) { left -= p.x; low -= p.y; right -= p.x; high -= p.y; return *this; }
	ZL_Rectf &operator+=(const scalar &f) { left -= f; low -= f; right += f; high += f; return *this; }
	ZL_Rectf &operator-=(const scalar &f) { left += f; low += f; right -= f; high -= f; return *this; }
	ZL_Rectf operator+(const ZL_Rectf &r) const { return ZL_Rectf(left + r.left, low + r.low, right + r.right, high + r.high); }
	ZL_Rectf operator-(const ZL_Rectf &r) const { return ZL_Rectf(left - r.left, low - r.low, right - r.right, high - r.high); }
	ZL_Rectf operator+(const ZL_Vector &p) const { return ZL_Rectf(left + p.x, low + p.y, right + p.x, high + p.y); }
	ZL_Rectf operator-(const ZL_Vector &p) const { return ZL_Rectf(left - p.x, low - p.y, right - p.x, high - p.y); }
	ZL_Rectf operator+(const scalar &f) const { return ZL_Rectf(left - f, low - f, right + f, high + f); }
	ZL_Rectf operator-(const scalar &f) const { return ZL_Rectf(left + f, low + f, right - f, high - f); }
	bool operator==(const ZL_Rectf &r) const { return (left == r.left && low == r.low && right == r.right && high == r.high); }
	bool operator!=(const ZL_Rectf &r) const { return (left != r.left || low != r.low || right != r.right || high != r.high); }
	bool operator!() const { return (!left && !high && !right && !low); }
	operator ZL_Rect () { return ZL_Rect((int)left, (int)low, (int)right, (int)high); }

	scalar left,low,right,high;
	void SetSize(scalar width, scalar height) { right = left + width; high = low + height; }
	inline scalar Width() const { return right - left; }
	inline scalar Height() const { return high - low; }
	bool Overlaps(const ZL_Rectf &r) const { return (r.left < right && r.right > left && r.low < high && r.high > low); }
	bool Contains(const ZL_Vector &p) const { return (p.x >= left && p.y >= low && p.x < right && p.y < high); }
	bool Contains(const ZL_Rectf &r) const { return (r.right <= right && r.left >= left && r.high <= high && r.low >= low); }
	bool Overlaps(const ZL_Vector& c, scalar ex, scalar ey) const { return ((c.x-ex) < right && (c.x+ex) > left && (c.y-ey) < high && (c.y+ey) > low); }
	bool Overlaps(const ZL_Vector& c, scalar r) const;
	ZL_Vector LowLeft() const { return ZL_Vector(left, low); }
	ZL_Vector LowRight() const { return ZL_Vector(right, low); }
	ZL_Vector HighLeft() const { return ZL_Vector(left, high); }
	ZL_Vector HighRight() const { return ZL_Vector(right, high); }
	ZL_Vector Center() const { return ZL_Vector((left+right)*s(.5), (low+high)*s(.5)); }
	scalar MidX() const { return (right+left)*s(.5); }
	scalar MidY() const { return (low+high)*s(.5); }
	ZL_Vector Clamp(const ZL_Vector& p) const { return ZL_Vector(ZL_Math::Clamp(p.x, left, right), ZL_Math::Clamp(p.y, low, high)); }
	ZL_Vector Extents() const { return ZL_Vector((right-left)*s(.5), (high-low)*s(.5)); }
	void Expand(const ZL_Vector& p) { if (p.x < left) left = p.x; else if (p.x > right) right = p.x; if (p.y < low) low = p.y; else if (p.y > high) high = p.y; }
	ZL_Vector Distance(const ZL_Vector& p) const { return ZL_Vector((p.x < left ? p.x - left : (p.x > right ? p.x - right : 0)), (p.y < low ? p.y - low : (p.y > high ? p.y - high : 0))); }
	ZL_Vector LerpPos(const ZL_Vector& pos0to1) const { return ZL_Vector(left+(right-left)*pos0to1.x, high-(high-low)*pos0to1.y); }
	ZL_Vector InverseLerpPos(const ZL_Vector& pos) const { return ZL_Vector((pos.x-left)/(right-left), (pos.y-low)/(high-low)); }
	ZL_Vector GetCorner(ZL_Origin::Type orCorner) const;
	ZL_Vector ClosestPointOnBorder(const ZL_Vector& p) const;

	//Linear interpolate between two rectangles
	static ZL_Rectf Lerp(const ZL_Rectf& recFrom, const ZL_Rectf& recTo, const scalar f) { return ZL_Rectf(ZL_Math::Lerp(recFrom.left, recTo.left, f), ZL_Math::Lerp(recFrom.low, recTo.low, f), ZL_Math::Lerp(recFrom.right, recTo.right, f), ZL_Math::Lerp(recFrom.high, recTo.high, f)); }

	//Map a point on one rectangle onto another rectangle
	static ZL_Vector Map(const ZL_Vector& p, const ZL_Rectf& src, const ZL_Rectf& dst) { return ZL_Vector(ZL_Math::Map(p.x, src.left, src.right, dst.left, dst.right), ZL_Math::Map(p.y, src.low, src.high, dst.low, dst.high)); }
};

//A color consisting of red, green, blue and alpha values in floats
struct ZL_Color
{
	scalar r,g,b,a;
	static const ZL_Color Transparent, White, Black, Silver, Gray, DarkGray;
	static const ZL_Color Red, Green, Blue, Yellow, Magenta, Cyan, Orange, Brown, Pink;
	static const ZL_Color DarkRed, DarkGreen, DarkBlue, DarkYellow, DarkMagenta, DarkCyan;
	bool operator==(const ZL_Color &c) const { return (r==c.r&&g==c.g&&b==c.b&&a==c.a); }
	bool operator!=(const ZL_Color &c) const { return (r!=c.r||g!=c.g||b!=c.b||a!=c.a); }
	bool operator!() const { return (!r && !g && !b && !a); }
	ZL_Color &operator+=(const ZL_Color &p) { r+=p.r; g+=p.g; b+=p.b; a+=p.a; return *this; }
	ZL_Color &operator-=(const ZL_Color &p) { r-=p.r; g-=p.g; b-=p.b; a-=p.a; return *this; }
	ZL_Color &operator*=(const ZL_Color &p) { r*=p.r; g*=p.g; b*=p.b; a*=p.a; return *this; }
	ZL_Color &operator/=(const ZL_Color &p) { r/=p.r; g/=p.g; b/=p.b; a/=p.a; return *this; }
	ZL_Color &operator+=(const scalar &f)   { r+=f; g+=f; b+=f; a+=f; return *this; }
	ZL_Color &operator-=(const scalar &f)   { r-=f; g-=f; b-=f; a-=f; return *this; }
	ZL_Color &operator*=(const scalar &f)   { r*=f; g*=f; b*=f; a*=f; return *this; }
	ZL_Color &operator/=(const scalar &f)   { r/=f; g/=f; b/=f; a/=f; return *this; }
	ZL_Color operator+(const ZL_Color &p) const { return ZL_Color(r+p.r, g+p.g, b+p.b, a+p.a); }
	ZL_Color operator-(const ZL_Color &p) const { return ZL_Color(r-p.r, g-p.g, b-p.b, a-p.a); }
	ZL_Color operator*(const ZL_Color &p) const { return ZL_Color(r*p.r, g*p.g, b*p.b, a*p.a); }
	ZL_Color operator/(const ZL_Color &p) const { return ZL_Color(r/p.r, g/p.g, b/p.b, a/p.a); }
	ZL_Color operator+(const scalar &f)   const { return ZL_Color(r+f, g+f, b+f, a); }
	ZL_Color operator-(const scalar &f)   const { return ZL_Color(r-f, g-f, b-f, a); }
	ZL_Color operator*(const scalar &f)   const { return ZL_Color(r*f, g*f, b*f, a); }
	ZL_Color operator/(const scalar &f)   const { return ZL_Color(r/f, g/f, b/f, a); }
	ZL_Color() : r(1), g(1), b(1), a(1) { }
	ZL_Color(const scalar r, const scalar g, const scalar b, const scalar a = 1) : r(r), g(g), b(b), a(a) { }
	ZL_Color(const scalar a) : r(1), g(1), b(1), a(a) { }
	ZL_Color& Norm() { r=(r<0?0:(r>1?1:r)); g=(g<0?0:(g>1?1:g)); b=(b<0?0:(b>1?1:b)); a=(a<0?0:(a>1?1:a)); return *this; }
	static inline ZL_Color LUM(const scalar LUM) { return ZL_Color(LUM, LUM, LUM); }
	static inline ZL_Color LUMA(const scalar LUM, const scalar A) { return ZL_Color(LUM, LUM, LUM, A); }
	static inline ZL_Color HSVA(const scalar H, const scalar S, const scalar V, const scalar A = s(1)) { return ZL_Color(V<=0?0:S<=0?V:H<s(1/6.)?V:H<s(2/6.)?V*(1-S*(H*s(6)-s(1))):H<s(4/6.)?V*(1-S):H<s(5/6.)?V*(1-S*(1-(H*s(6)-s(4)))):V,V<=0?0:S<=0?V:H<s(1/6.)?V*(1-S*(1-(H*s(6)))):H<s(3/6.)?V:H<s(4/6.)?V*(1-S*(H*s(6)-s(3))):V*(1-S),V<=0?0:S<=0?V:H<s(2/6.)?V*(1-S):H<s(3/6.)?V*(1-S*(1-(H*s(6)-s(2)))):H<s(5/6.)?V:V*(1-S*(H*s(6)-s(5))),A); }
	static ZL_Color Lerp(const ZL_Color& From, const ZL_Color& To, const scalar f) { return ZL_Color(ZL_Math::Lerp(From.r, To.r, f), ZL_Math::Lerp(From.g, To.g, f), ZL_Math::Lerp(From.b, To.b, f), ZL_Math::Lerp(From.a, To.a, f)); }
};

enum ZL_DisplayInitFlags
{
	ZL_DISPLAY_DEFAULT                    = 0x000, //no special flag
	ZL_DISPLAY_FULLSCREEN                 = 0x001, //window will be fullscreen (only related to desktop platforms)
	ZL_DISPLAY_RESIZABLE                  = 0x006, //freely resizable viewport, application must accompany for variable width & height, incompatible with following two settings
	ZL_DISPLAY_ALLOWRESIZEHORIZONTAL      = 0x002, //allow widening of viewport to accompany various native screen aspect ratio and scale vertical to native aspect ratio
	ZL_DISPLAY_ALLOWRESIZEVERTICAL        = 0x004, //allow vertical resize of viewport to accompany various native screen aspect ratio and scale horizontal to native aspect ratio
	ZL_DISPLAY_ALLOWANYORIENTATION        = 0x008, //allow both portrait and landscape screen orientation, otherwise fixed based on window aspect ratio (currently only on mobile)
	ZL_DISPLAY_PREVENTALTENTER            = 0x010, //when set ALT+ENTER won't toggle fullscreen (on platforms that have full screen switching)
	ZL_DISPLAY_PREVENTALTF4               = 0x020, //when set ALT+F4 won't quit
	ZL_DISPLAY_DEPTHBUFFER                = 0x040, //use for 3d rendering with depth buffer
	ZL_DISPLAY_ANDROID_OVERRIDEVOLUMEKEYS = 0x100, //[Android only] when set the volume cannot be changed with the hardware buttons but are available to sigKeyDown/sigKeyUp
	ZL_DISPLAY_ANDROID_IMMERSIVEMODE      = 0x200, //[Android only] will hide the navigation bar and get more screen space available to the application
};

//Polygon consisting of arbitrary number of points, can be filled or outlined with color or texture. Handles intersections and multiple contours.
struct ZL_Polygon
{
	enum ColoredMode { BORDER = 1, FILL = 2, BORDER_FILL = BORDER|FILL };
	enum IntersectMode { ODD = 0, NONZERO = 1, POSITIVE = 2, NEGATIVE = 3, ABS_GEQ_TWO = 4 };
	typedef std::vector<ZL_Vector> PointList;
	ZL_Polygon();
	ZL_Polygon(ColoredMode mode); //colored polygon with outline border and/or filled color
	ZL_Polygon(const struct ZL_Surface& surface, bool withBorder = false); //polygon filled with a texture and optionally also generate outline borders
	~ZL_Polygon();
	ZL_Polygon(const ZL_Polygon& source);
	ZL_Polygon& operator=(const ZL_Polygon& source);
	operator bool () const { return (impl!=NULL); }
	bool operator==(const ZL_Polygon& b) const { return (impl==b.impl); }
	bool operator!=(const ZL_Polygon& b) const { return (impl!=b.impl); }

	ZL_Polygon& Add(const ZL_Vector *p, int pnum, IntersectMode selfintersect = NONZERO); //append contour to this polygon set
	ZL_Polygon& Add(const PointList& contour, IntersectMode selfintersect = NONZERO); //append contour to this polygon set
	ZL_Polygon& Add(const std::vector<PointList>& contours, IntersectMode intersect = NONZERO); //append multiple contours to this polygon set
	ZL_Polygon& Add(const PointList*const* contours, int cnum, IntersectMode intersect = NONZERO); //append multiple contours to this polygon set
	ZL_Polygon& Extrude(const ZL_Vector *p, int pnum, scalar offsetout, scalar offsetin = 0, bool offsetjoints = false, bool loop = true, bool ccw = false, scalar capscale = 1); //append extruded outline
	ZL_Polygon& Extrude(const PointList& contour, scalar offsetout, scalar offsetin = 0, bool offsetjoints = false, bool loop = true, bool ccw = false, scalar capscale = 1); //append extruded outline
	ZL_Polygon& ExtrudeFromBorder(const ZL_Polygon& source, scalar offsetout, scalar offsetin = 0, bool offsetjoints = false, bool loop = true, bool ccw = false, scalar capscale = 1); //append extruded outline from other polygon border

	ZL_Polygon& SetSurfaceColor(const ZL_Color& color);
	struct ZL_Surface GetSurface() const;
	const ZL_Rectf& GetBoundingBox() const;
	void Clear(); //To start over with Add()/Extrude() calls, this keeps memory allocated.
	void RemoveBorder(); //Remove the calculated border data if not needed anymore, keeps fill data.

	void Draw(const ZL_Color& color_border, const ZL_Color& color_fill = ZL_Color::Transparent) const;
	void Fill(const ZL_Color& color_fill) const;
	void Draw() const; //draw textured
	void Draw(const struct ZL_Surface& surface) const; //draw with another texture which should have the same dimensions as the one it was initialized with

	//Functions that return points of the calculated border (or multiple borders) in clockwise order (usable for pushing data into collision handling or physics system)
	size_t GetBorders(std::vector<PointList>& out) const;
	bool GetBorder(std::vector<ZL_Vector>& out) const;

	static size_t GetBorders(const std::vector<PointList>& contours, std::vector<PointList>& out, IntersectMode intersect = NONZERO);
	static bool GetBorder(const std::vector<PointList>& contours, std::vector<ZL_Vector>& out, IntersectMode intersect = NONZERO);
	static size_t GetBorders(const PointList*const* contours, int cnum, std::vector<PointList>& out, IntersectMode intersect = NONZERO);
	static bool GetBorder(const PointList*const* contours, int cnum, std::vector<ZL_Vector>& out, IntersectMode intersect = NONZERO);

	private: struct ZL_Polygon_Impl* impl;
};

//Open GL shader language helper macros
#define ZL_GLES_PRECISION_HIGH   "highp"   //16-bit, floating point range: -2^62 to 2^62, integer range: -2^16 to 2^16
#define ZL_GLES_PRECISION_MEDIUM "mediump" //10 bit, floating point range: -2^14 to 2^14, integer range: -2^10 to 2^10
#define ZL_GLES_PRECISION_LOW    "lowp"    // 8 bit, floating point range: -2 to 2, integer range: -2^8 to 2^8
#define ZL_SHADER_SOURCE_HEADER(GLES_PRECISION) "#ifdef GL_ES\nprecision " GLES_PRECISION " float;\n#endif\n"
#if (!defined(_MSC_VER) || (_MSC_VER >= 1400))
#define ZL_SHADER_SOURCE_QUOTE(GLES_PRECISION,...) ZL_SHADER_SOURCE_HEADER(GLES_PRECISION) #__VA_ARGS__
#endif

//Surface shaders that are applied to surface drawing. Both fragment and vertex shaders can be customized. Very simple, only up to two float uniforms are supported.
struct ZL_Shader
{
	ZL_Shader();
	ZL_Shader(const char* code_fragment, const char *code_vertex = NULL, const char* name_uniform_float_1 = NULL, const char* name_uniform_float_2 = NULL);
	~ZL_Shader();
	ZL_Shader(const ZL_Shader &source);
	ZL_Shader &operator=(const ZL_Shader &source);
	operator bool () const { return (impl!=NULL); }
	bool operator==(const ZL_Shader &b) const { return (impl==b.impl); }
	bool operator!=(const ZL_Shader &b) const { return (impl!=b.impl); }

	void Activate();
	void SetUniform(scalar uni1);
	void SetUniform(scalar uni1, scalar uni2);
	void Deactivate();

	private: struct ZL_Shader_Impl* impl;
};

//Post process shaders that render everything between Start and Apply to a framebuffer and perform the shader once over the whole render display (can fail when framebuffers not supported by gpu)
struct ZL_PostProcess
{
	ZL_PostProcess();
	ZL_PostProcess(const char* code_fragment, bool use_alpha = false, const char* name_uniform_float_1 = NULL, const char* name_uniform_float_2 = NULL);
	~ZL_PostProcess();
	ZL_PostProcess(const ZL_PostProcess &source);
	ZL_PostProcess &operator=(const ZL_PostProcess &source);
	operator bool () const { return (impl!=NULL); }
	bool operator==(const ZL_PostProcess &b) const { return (impl==b.impl); }
	bool operator!=(const ZL_PostProcess &b) const { return (impl!=b.impl); }

	void Start(bool clear = true);
	void Apply();
	void Apply(scalar uni1);
	void Apply(scalar uni1, scalar uni2);

	private: struct ZL_PostProcess_Impl* impl;
};

//Global display related functions (any application need to at least call ZL_Display::Init to get a display)
struct ZL_Display
{
	//Initialize rendering on a window
	static bool Init(const char* title, int width = 640, int height = 480, int displayflags = ZL_DISPLAY_DEFAULT);

	//Clear the entire screen
	static void ClearFill(ZL_Color col = ZL_Color::Black);

	//Set a clipping rectangle for clipped rendering
	static void SetClip(const ZL_Rectf &clip);
	static void SetClip(int x, int y, int width, int height);
	static void ResetClip();

	//set alpha blending handling
	enum BlendFunc { BLEND_SRCALPHA = 0x0302, BLEND_INVSRCALPHA = 0x0303, BLEND_SRCCOLOR = 0x0300, BLEND_INVSRCCOLOR = 0x0301, BLEND_DESTCOLOR = 0x0306, BLEND_INVDESTCOLOR = 0x0307, BLEND_ZERO = 0, BLEND_ONE = 1, BLEND_DESTALPHA = 0x0304, BLEND_INVDESTALPHA = 0x0305, BLEND_CONSTCOLOR = 0x8001, BLEND_INVCONSTCOLOR = 0x8002, BLEND_CONSTALPHA = 0x8003, BLEND_INVCONSTALPHA = 0x8004, BLEND_SRCALPHASATURATE = 0x0308 };
	static void SetBlendFunc(BlendFunc mode_src, BlendFunc mode_dest);
	static void SetBlendModeSeparate(BlendFunc mode_rgb_src, BlendFunc mode_rgb_dest, BlendFunc mode_alpha_src, BlendFunc mode_alpha_dest);
	inline static void ResetBlendFunc() { SetBlendFunc(BLEND_SRCALPHA, BLEND_INVSRCALPHA); }
	enum BlendEquation { BLENDEQUATION_ADD = 0x8006, BLENDEQUATION_MIN = 0x8007, BLENDEQUATION_MAX = 0x8008, BLENDEQUATION_SUBTRACT = 0x800A, BLENDEQUATION_REVERSE_SUBTRACT = 0x800B };
	static void SetBlendEquation(BlendEquation func);
	static void SetBlendEquationSeparate(BlendEquation func_rgb, BlendEquation func_alpha);
	inline static void ResetBlendEquation() { SetBlendEquation(BLENDEQUATION_ADD); }
	static void SetBlendConstantColor(const ZL_Color& constant_color);

	//Control rendering matrix (global translation, rotation, scaling)
	static void PushMatrix();
	static void PopMatrix();
	static void Translate(const ZL_Vector& v);
	static void Translate(scalar x, scalar y);
	static void Rotate(scalar angle_rad); //rotate in radians
	static void RotateDeg(scalar angle_deg); //rotate in degree
	static void Rotate(scalar rotx, scalar roty); //fast rotation (precalculated cos/sin)
	static void Transform(scalar x, scalar y, scalar rotx, scalar roty); //fast (precalculated cos/sin)
	static void TransformReverse(scalar x, scalar y, scalar rotx, scalar roty); //fast (precalculated cos/sin)
	static void Scale(scalar scale);
	static void Scale(scalar scalex, scalar scaley);
	static void SetMatrix(const struct ZL_Matrix& mtx);

	//Reset ortho matrix (define what left/right/up/down borders on the screen confirm to)
	inline static void PushOrtho(const ZL_Rectf &o) { PushOrtho(o.left, o.right, o.low, o.high); }
	static void PushOrtho(scalar left, scalar right, scalar bottom, scalar top);
	static void PopOrtho();

	//Convert positions from current matrix transformed to screen coordinates and back
	inline static ZL_Vector WorldToScreen(const ZL_Vector& pos) { return WorldToScreen(pos.x, pos.y); }
	static ZL_Vector WorldToScreen(scalar x, scalar y);
	inline static void ConvertWorldToScreen(ZL_Vector& pos) { pos = WorldToScreen(pos.x, pos.y); }
	inline static ZL_Vector ScreenToWorld(const ZL_Vector& pos) { return ScreenToWorld(pos.x, pos.y); }
	static ZL_Vector ScreenToWorld(scalar x, scalar y);
	inline static void ConvertScreenToWorld(ZL_Vector& pos) { pos = ScreenToWorld(pos.x, pos.y); }

	//2D geometry drawing functions
	inline static void DrawLine(const ZL_Vector& p1, const ZL_Vector& p2, const ZL_Color &color) { DrawLine(p1.x, p1.y, p2.x, p2.y, color); }
	static void DrawLine(scalar x1, scalar y1, scalar x2, scalar y2, const ZL_Color &color);
	inline static void DrawWideLine(const ZL_Vector& p1, const ZL_Vector& p2, scalar width, const ZL_Color &color_border, const ZL_Color &color_fill = ZL_Color::Transparent) { DrawWideLine(p1.x, p1.y, p2.x, p2.y, width, color_border, color_fill); }
	static void DrawWideLine(scalar x1, scalar y1, scalar x2, scalar y2, scalar width, const ZL_Color &color_border, const ZL_Color &color_fill = ZL_Color::Transparent);
	inline static void DrawRect(const ZL_Rectf& rec, const ZL_Color &color_border, const ZL_Color &color_fill = ZL_Color::Transparent) { DrawRect(rec.left, rec.low, rec.right, rec.high, color_border, color_fill); }
	static void DrawRect(const scalar& x1, const scalar& y1, const scalar& x2, const scalar& y2, const ZL_Color &color_border, const ZL_Color &color_fill = ZL_Color::Transparent);
	inline static void DrawCircle(scalar cx, scalar cy, scalar r, const ZL_Color &color_border, const ZL_Color &color_fill = ZL_Color::Transparent) { DrawEllipse(cx, cy, r, r, color_border, color_fill); }
	inline static void DrawCircle(const ZL_Vector& p, scalar r, const ZL_Color &color_border, const ZL_Color &color_fill = ZL_Color::Transparent) { DrawEllipse(p.x, p.y, r, r, color_border, color_fill); }
	inline static void DrawEllipse(const ZL_Vector& p, scalar rx, scalar ry, const ZL_Color &color_border, const ZL_Color &color_fill = ZL_Color::Transparent) { DrawEllipse(p.x, p.y, rx, ry, color_border, color_fill); }
	static void DrawEllipse(scalar cx, scalar cy, scalar rx, scalar ry, const ZL_Color &color_border, const ZL_Color &color_fill = ZL_Color::Transparent);
	inline static void DrawTriangle(const ZL_Vector &p1, const ZL_Vector &p2, const ZL_Vector &p3, const ZL_Color &color_border, const ZL_Color &color_fill = ZL_Color::Transparent) { DrawTriangle(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, color_border, color_fill); }
	static void DrawTriangle(scalar x1, scalar y1, scalar x2, scalar y2, scalar x3, scalar y3, const ZL_Color &color_border, const ZL_Color &color_fill = ZL_Color::Transparent);
	inline static void DrawQuad(const ZL_Vector &p1, const ZL_Vector &p2, const ZL_Vector &p3, const ZL_Vector &p4, const ZL_Color &color_border, const ZL_Color &color_fill = ZL_Color::Transparent) { DrawQuad(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y, color_border, color_fill); }
	static void DrawQuad(scalar x1, scalar y1, scalar x2, scalar y2, scalar x3, scalar y3, scalar x4, scalar y4, const ZL_Color &color_border, const ZL_Color &color_fill = ZL_Color::Transparent);
	static void DrawBezier(scalar x1, scalar y1, scalar x2, scalar y2, scalar x3, scalar y3, scalar x4, scalar y4, const ZL_Color &color);

	//Filled 2D geometry shape drawing functions
	inline static void FillWideLine(const ZL_Vector& p1, const ZL_Vector& p2, scalar width, const ZL_Color &color_fill = ZL_Color::Transparent) { DrawWideLine(p1.x, p1.y, p2.x, p2.y, width, ZL_Color::Transparent, color_fill); }
	inline static void FillWideLine(scalar x1, scalar y1, scalar x2, scalar y2, scalar width, const ZL_Color &color_fill = ZL_Color::Transparent) { DrawWideLine(x1, y1, x2, y2, width, ZL_Color::Transparent, color_fill); }
	inline static void FillRect(const ZL_Rectf& rec, const ZL_Color &color_fill) { DrawRect(rec.left, rec.low, rec.right, rec.high, ZL_Color::Transparent, color_fill); }
	inline static void FillRect(const scalar& x1, const scalar& y1, const scalar& x2, const scalar& y2, const ZL_Color &color_fill) { DrawRect(x1, y1, x2, y2, ZL_Color::Transparent, color_fill); }
	inline static void FillCircle(scalar cx, scalar cy, scalar r, const ZL_Color &color_fill) { DrawEllipse(cx, cy, r, r, ZL_Color::Transparent, color_fill); }
	inline static void FillCircle(const ZL_Vector& p, scalar r, const ZL_Color &color_fill) { DrawEllipse(p.x, p.y, r, r, ZL_Color::Transparent, color_fill); }
	inline static void FillEllipse(const ZL_Vector& p, scalar rx, scalar ry, const ZL_Color &color_fill) { DrawEllipse(p.x, p.y, rx, ry, ZL_Color::Transparent, color_fill); }
	inline static void FillEllipse(scalar cx, scalar cy, scalar rx, scalar ry, const ZL_Color &color_fill) {  { DrawEllipse(cx, cy, rx, ry, ZL_Color::Transparent, color_fill); } }
	inline static void FillTriangle(const ZL_Vector &p1, const ZL_Vector &p2, const ZL_Vector &p3, const ZL_Color &color_fill) { DrawTriangle(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, ZL_Color::Transparent, color_fill); }
	inline static void FillTriangle(scalar x1, scalar y1, scalar x2, scalar y2, scalar x3, scalar y3, const ZL_Color &color_fill) { DrawTriangle(x1, y1, x2, y2, x3, y3, ZL_Color::Transparent, color_fill); }
	inline static void FillQuad(const ZL_Vector &p1, const ZL_Vector &p2, const ZL_Vector &p3, const ZL_Vector &p4, const ZL_Color &color_fill) { DrawQuad(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y, ZL_Color::Transparent, color_fill); }
	inline static void FillQuad(scalar x1, scalar y1, scalar x2, scalar y2, scalar x3, scalar y3, scalar x4, scalar y4, const ZL_Color &color_fill) { DrawQuad(x1, y1, x2, y2, x3, y3, x4, y4, ZL_Color::Transparent, color_fill); }
	inline static void FillGradient(const ZL_Rectf& rec, const ZL_Color &col1, const ZL_Color &col2, const ZL_Color &col3, const ZL_Color &col4) { FillGradient(rec.left, rec.low, rec.right, rec.high, col1, col2, col3, col4); }
	static void FillGradient(const scalar& x1, const scalar& y1, const scalar& x2, const scalar& y2, const ZL_Color &col1, const ZL_Color &col2, const ZL_Color &col3, const ZL_Color &col4);

	//Anti-aliasing and line thickness only properly works on desktop OpenGL (and even then just on some platforms/drivers)
	static void SetAA(bool aa);
	static void SetThickness(scalar thickness = 1.0f);

	//Window properties
	static scalar Width, Height;
	inline static ZL_Vector Size() { return ZL_Vector(Width, Height); }
	inline static ZL_Vector Center() { return ZL_Vector(Width*s(.5), Height*s(.5)); }

	//Events
	static ZL_Signal_v1<ZL_KeyboardEvent&> sigKeyDown, sigKeyUp;
	static ZL_Signal_v1<const ZL_String&> sigTextInput;
	static ZL_Signal_v1<ZL_PointerMoveEvent&> sigPointerMove;
	static ZL_Signal_v1<ZL_PointerPressEvent&> sigPointerDown, sigPointerUp;
	static ZL_Signal_v1<ZL_MouseWheelEvent&> sigMouseWheel;
	static ZL_Signal_v1<ZL_WindowActivateEvent&> sigActivated; //input focus, mouse movement, iconified
	static ZL_Signal_v1<ZL_WindowResizeEvent&> sigResized;
	static void AllSigDisconnect(void *callback_class_inst);

	//Data from the last input events (pointer position, key/button states)
	static scalar PointerX, PointerY;
	static bool KeyDown[ZLK_LAST]; //ZL_Key bool map
	static bool MouseDown[8]; //Bool map of pressed mouse buttons, for touch panels only [ZL_BUTTON_LEFT] is used
	inline static ZL_Vector PointerPos() { return ZL_Vector(PointerX, PointerY); }

	//Get the name of a key as a string (in english)
	static ZL_String KeyScancodeName(ZL_Key key);

	//Query the current window state
	static bool IsFullscreen();
	static bool IsMinimized();
	static bool HasInputFocus();
	static bool HasMouseFocus();

	//desktop/web only (does nothing otherwise)
	static void ToggleFullscreen();
	static void SetFullscreen(bool toFullscreen);
	static void TogglePointerLock();
	static void SetPointerLock(bool doLockPointer);

	//touch input (mobile) devices only (does nothing otherwise)
	static void SoftKeyboardToggle();
	static void SoftKeyboardShow();
	static void SoftKeyboardHide();
	static bool SoftKeyboardIsShown();
	static void DeviceVibrate(int duration); //iOS doesn't support duration
};

#endif //__ZL_DISPLAY__
