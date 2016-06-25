#ifndef __ZL_DEMOUI__
#define __ZL_DEMOUI__

#include "ZL_Display.h"
#include "ZL_Signal.h"
#include "ZL_Surface.h"
#include "ZL_Font.h"

struct ZL_DemoUIItem;
struct ZL_DemoUIButton;
struct ZL_DemoUI
{
	static bool Init(const ZL_Font& fntUIFont);
	static const ZL_DemoUIItem& Add(const ZL_DemoUIItem& item);
	static void Draw();
	static void Clear(int duration = -1);
	static void DrawDebug();
	static void Push();
	static void Pop(int duration = -1);
	static bool HandlesEvent(const ZL_PointerPressEvent& e);
	static const ZL_DemoUIItem& GetById(int id);
	static const ZL_DemoUIButton& GetButtonById(int id);
};

struct ZL_DemoUIMessage;
struct ZL_DemoUIItem_Impl;
struct ZL_DemoUIItem
{
	ZL_DemoUIItem();
	~ZL_DemoUIItem();
	ZL_DemoUIItem(const ZL_DemoUIItem &source);
	ZL_DemoUIItem& operator=(const ZL_DemoUIItem &source);
	operator bool () const { return (impl!=NULL); }
	bool operator==(const ZL_DemoUIItem &b) const { return (impl==b.impl); }
	bool operator!=(const ZL_DemoUIItem &b) const { return (impl!=b.impl); }

	const ZL_DemoUIItem& Animate(int duration = 500, bool stay = true) const;
	const ZL_DemoUIItem& GoAway(int duration = -1) const;

	const ZL_DemoUIItem& SetColor(const ZL_Color& col = ZL_Color::Red) const;
	const ZL_DemoUIItem& SetRotate(scalar rotate = s(5)) const;
	const ZL_DemoUIItem& SetScale(scalar scale = s(2)) const;

	scalar& x() const;
	scalar& y() const;
	scalar& width() const;
	scalar& height() const;
	scalar& scale() const;
	scalar& rotate() const;

	inline const ZL_DemoUIButton& AsButton() const { return *(ZL_DemoUIButton*)this; }

protected:
	friend struct ZL_DemoUI;
	ZL_DemoUIItem_Impl *impl;
};

struct ZL_DemoUIMessage : public ZL_DemoUIItem
{
	ZL_DemoUIMessage();
	~ZL_DemoUIMessage();
	ZL_DemoUIMessage(const char* txt, scalar x = ZLHALFW, scalar y = ZLHALFH);
	ZL_DemoUIMessage(const ZL_Surface& srf, scalar x = ZLHALFW, scalar y = ZLHALFH);
	ZL_DemoUIMessage(const ZL_Surface& srf, const char* txt, scalar x = ZLHALFW, scalar y = ZLHALFH, bool useSurfaceWidth = true);
	ZL_DemoUIMessage(const ZL_DemoUIMessage &source);
	ZL_DemoUIMessage& operator=(const ZL_DemoUIMessage &source);

	const ZL_DemoUIMessage& setText(const char* txt, bool resizeTo = false) const;
	const ZL_DemoUIMessage& setSurface(const ZL_Surface& srf, bool resizeTo = false) const;
};

struct ZL_DemoUIButton : public ZL_DemoUIMessage
{
	ZL_DemoUIButton();
	~ZL_DemoUIButton();
	ZL_DemoUIButton(const char* txt, scalar x = ZLHALFW, scalar y = ZLHALFH);
	ZL_DemoUIButton(const ZL_Surface& srf, scalar x = ZLHALFW, scalar y = ZLHALFH);
	ZL_DemoUIButton(const ZL_Surface& srf, const char* txt, scalar x = ZLHALFW, scalar y = ZLHALFH, bool useSurfaceWidth = true);
	ZL_DemoUIButton(const ZL_DemoUIButton &source);
	ZL_DemoUIButton& operator=(const ZL_DemoUIButton &source);

	const ZL_DemoUIButton& setText(const char* txt, bool resizeTo = false) const;
	const ZL_DemoUIButton& setSurface(const ZL_Surface& srf, bool resizeTo = false) const;
	const ZL_DemoUIButton& setId(int id) const;
	ZL_Signal_v0& sigTouch() const;
	ZL_Signal_v1<int>& sigIdTouch() const;
};

#endif //__ZL_DEMOUI__

#ifdef ZL_OPT_DO_IMPLEMENTATION

#include "ZL_Timer.h"
#include "../Source/ZL_Impl.h"
#include <list>
#include <assert.h>

enum ZL_DemoUIType
{
	UITYPE_MESSAGE,
	UITYPE_BUTTON
};

struct ZL_DemoUIAnimation;

struct ZL_DemoUIItem_Impl : ZL_Impl
{
	ZL_DemoUIType type;
	ZL_DemoUIAnimation *anim_enter, *anim_leave;
	scalar x, y, rotate, scale, width, height;
	bool stay, leaving;
	ZL_Color color;
	int stack;
	ZL_DemoUIItem_Impl(scalar x, scalar y, ZL_DemoUIType type) : type(type), anim_enter(NULL), anim_leave(NULL), x(x), y(y), rotate(0), scale(1), stay(true), leaving(false), color(ZL_Color::White) { }
	virtual ~ZL_DemoUIItem_Impl();
	virtual bool Draw() = 0;
	void GoAway(int duration = -1);
};

struct ZL_DemoUIMessage_Impl : ZL_DemoUIItem_Impl
{
	ZL_TextBuffer txt;
	ZL_Surface srf;
	ZL_DemoUIMessage_Impl(scalar x, scalar y) : ZL_DemoUIItem_Impl(x, y, UITYPE_MESSAGE) { }
	virtual bool Draw();
};

struct ZL_DemoUIButton_Impl : ZL_DemoUIMessage_Impl
{
	int id, touchRadius;
	ZL_Signal_v0 sigTouch;
	ZL_Signal_v1<int> sigIdTouch;
	ZL_DemoUIButton_Impl(scalar x, scalar y) : ZL_DemoUIMessage_Impl(x, y) { type = UITYPE_BUTTON; }
	bool Touched(scalar touchx, scalar touchy);
	void DrawDebug();
};

struct ZL_DemoUIAnimation
{
	unsigned int start, end;
	ZL_DemoUIAnimation(int duration, bool leaving) : start(ZLTICKS), end(ZLTICKS + duration), f(leaving ? s(0) : s(1)) { }
	bool Update(ZL_DemoUIItem_Impl *item)
	{
		if (ZLTICKS >= end) { return false; }
		//f = s(ZLTICKS - start) / s(end - start);
		f = s(0.5) * ((ZLTICKS - start)*(ZLTICKS - start)) * (s(2) / ((end-start) * (end-start)));
		if (!item->leaving) f = 1 - f;
		return true;
	}
	virtual scalar GetScale() { return 1; }
	virtual scalar GetMoveX() { return 0; }
	virtual inline ~ZL_DemoUIAnimation() { }
	protected: scalar f;
};

struct ZL_DemoUIAnimationScale : public ZL_DemoUIAnimation
{
	scalar scale;
	ZL_DemoUIAnimationScale(int duration, bool leaving) :  ZL_DemoUIAnimation(duration, leaving), scale(3) { }
	virtual scalar GetScale()
	{
		return 1 + (f*(scale-1));
	}
};

struct ZL_DemoUIAnimationMove : public ZL_DemoUIAnimation
{
	ZL_Vector move;
	ZL_DemoUIAnimationMove(ZL_DemoUIItem_Impl *item, int duration, bool leaving) :  ZL_DemoUIAnimation(duration, leaving), move(ZL_Display::Width + 20.0f - item->x + (item->width*item->scale/2.0f*(1+item->rotate/25.0f)),0) { }
	virtual scalar GetMoveX() { return f*move.x; }
};


void ZL_DemoUIItem_Impl::GoAway(int duration)
{
	if (leaving) return;
	stay = false;
	if (anim_leave) { anim_leave->end = ZLTICKS + (duration >= 0 ? duration : (anim_leave->end - anim_leave->start)); anim_leave->start = ZLTICKS; }
	if (!anim_enter) leaving = true;
}

ZL_DemoUIItem_Impl::~ZL_DemoUIItem_Impl()
{
	if (anim_enter) delete anim_enter;
	if (anim_leave) delete anim_leave;
}

// --------------------------------------------------------------------------------------------

static struct ZL_DemoUIData
{
	ZL_Font fntUIFont;
	std::list<ZL_DemoUIItem_Impl*> items;
	int stack;
	bool handledmouseup;
	ZL_DemoUIItem getbyiditem;
	void OnPointerUp(ZL_PointerPressEvent& e)
	{
		for (std::list<ZL_DemoUIItem_Impl*>::iterator it = items.begin(); it != items.end();++it)
			if ((*it)->type == UITYPE_BUTTON)
				if (((ZL_DemoUIButton_Impl*)(*it))->Touched(e.x, e.y))
				{ handledmouseup = true; return; }
		handledmouseup = false;
	}
	void OnResized(ZL_WindowResizeEvent& e)
	{
		for (std::list<ZL_DemoUIItem_Impl*>::iterator it = items.begin(); it != items.end();++it)
		{
			(*it)->x = (*it)->x * ZLWIDTH / e.old_width;
			(*it)->y = (*it)->y * ZLHEIGHT / e.old_height;
		}
	}
} *data = NULL;

// --------------------------------------------------------------------------------------------

bool ZL_DemoUI::Init(const ZL_Font& fntUIFont)
{
	if (data)
	{
		data->fntUIFont = fntUIFont;
		return true;
	}
	data = new ZL_DemoUIData();
	data->fntUIFont = fntUIFont;
	data->stack = 0;
	data->handledmouseup = false;
	ZL_Display::sigPointerUp.connect(data, &ZL_DemoUIData::OnPointerUp);
	ZL_Display::sigResized.connect(data, &ZL_DemoUIData::OnResized);
	return true;
}

void ZL_DemoUI::Push() { data->stack++; }

void ZL_DemoUI::Pop(int duration)
{
	if (!data->stack) return;
	for (std::list<ZL_DemoUIItem_Impl*>::iterator it = data->items.begin(); it != data->items.end();++it)
		if (data->stack == (*it)->stack) (*it)->GoAway(duration);
	data->stack--;
}

const ZL_DemoUIItem& ZL_DemoUI::Add(const ZL_DemoUIItem& item)
{
	item.impl->stack = data->stack;
	item.impl->AddRef();
	data->items.push_back(item.impl);
	return item;
}

void ZL_DemoUI::Draw()
{
	for (std::list<ZL_DemoUIItem_Impl*>::iterator it = data->items.begin(); it != data->items.end();)
	{
		if ((*it)->Draw()) ++it;
		else { (*it)->DelRef(); it = data->items.erase(it); }
	}
}

void ZL_DemoUI::Clear(int duration)
{
	for (std::list<ZL_DemoUIItem_Impl*>::iterator it = data->items.begin(); it != data->items.end();++it)
		(*it)->GoAway(duration);
}

void ZL_DemoUI::DrawDebug()
{
	for (std::list<ZL_DemoUIItem_Impl*>::iterator it = data->items.begin(); it != data->items.end();++it)
		if ((*it)->type == UITYPE_BUTTON)
			((ZL_DemoUIButton_Impl*)(*it))->DrawDebug();
}

bool ZL_DemoUI::HandlesEvent(const ZL_PointerPressEvent& e)
{
	assert(data);
	if (!e.is_down) return data->handledmouseup;
	for (std::list<ZL_DemoUIItem_Impl*>::iterator it = data->items.begin(); it != data->items.end();++it)
		if ((*it)->type == UITYPE_BUTTON)
			if (((ZL_DemoUIButton_Impl*)(*it))->Touched(e.x, e.y))
				return true;
	return false;
}

const ZL_DemoUIItem& ZL_DemoUI::GetById(int id)
{
	data->getbyiditem.impl = NULL;
	for (std::list<ZL_DemoUIItem_Impl*>::iterator it = data->items.begin(); it != data->items.end();++it)
		if ((*it)->type == UITYPE_BUTTON && ((ZL_DemoUIButton_Impl*)(*it))->id == id)
			{ data->getbyiditem.impl = *it; break; }
	return data->getbyiditem;
}

const ZL_DemoUIButton& ZL_DemoUI::GetButtonById(int id)
{
	data->getbyiditem.impl = NULL;
	for (std::list<ZL_DemoUIItem_Impl*>::reverse_iterator it = data->items.rbegin(); it != data->items.rend();++it)
		if ((*it)->type == UITYPE_BUTTON && (*it)->stay && ((ZL_DemoUIButton_Impl*)(*it))->id == id)
			{ data->getbyiditem.impl = *it; break; }
	return *(ZL_DemoUIButton*)&data->getbyiditem;
}

// --------------------------------------------------------------------------------------------

ZL_IMPL_OWNER_DEFAULT_IMPLEMENTATIONS(ZL_DemoUIItem)

const ZL_DemoUIItem& ZL_DemoUIItem::Animate(int duration, bool stay) const
{
	if (!impl) return *this;
	impl->leaving = false;
	impl->stay = stay;
	if (impl->anim_enter) delete(impl->anim_enter);
	if (impl->anim_leave) delete(impl->anim_leave);
	impl->anim_enter = new ZL_DemoUIAnimationScale((stay ? duration : (duration * 2 / 3)), false);
	impl->anim_leave = new ZL_DemoUIAnimationMove(impl, (stay ? duration : (duration / 3)), true);
	return *this;
}

const ZL_DemoUIItem& ZL_DemoUIItem::GoAway(int duration) const
{
	if (impl && impl->stay) impl->GoAway(duration);
	return *this;
}

const ZL_DemoUIItem& ZL_DemoUIItem::SetColor(const ZL_Color& color) const
{
	impl->color = color;
	return *this;
}

const ZL_DemoUIItem& ZL_DemoUIItem::SetRotate(scalar rotate) const
{
	impl->rotate = rotate;
	return *this;
}

const ZL_DemoUIItem& ZL_DemoUIItem::SetScale(scalar scale) const
{
	impl->scale = scale;
	return *this;
}

scalar& ZL_DemoUIItem::x() const      { assert(impl); return impl->x; }
scalar& ZL_DemoUIItem::y() const      { assert(impl); return impl->y; }
scalar& ZL_DemoUIItem::width() const  { assert(impl); return impl->width; }
scalar& ZL_DemoUIItem::height() const { assert(impl); return impl->height; }
scalar& ZL_DemoUIItem::scale() const  { assert(impl); return impl->scale; }
scalar& ZL_DemoUIItem::rotate() const { assert(impl); return impl->rotate; }

// --------------------------------------------------------------------------------------------

ZL_IMPL_OWNER_INHERITED_IMPLEMENTATIONS(ZL_DemoUIMessage)
ZL_DemoUIMessage::ZL_DemoUIMessage(const char* txt, scalar x, scalar y)  { impl = new ZL_DemoUIMessage_Impl(x, y); setText(txt, true); }
ZL_DemoUIMessage::ZL_DemoUIMessage(const ZL_Surface& srf, scalar x, scalar y) { impl = new ZL_DemoUIMessage_Impl(x, y); setSurface(srf, true); }
ZL_DemoUIMessage::ZL_DemoUIMessage(const ZL_Surface& srf, const char* txt, scalar x, scalar y, bool useSurfaceWidth)
{
	impl = new ZL_DemoUIMessage_Impl(x, y);
	setText(txt, !useSurfaceWidth);
	setSurface(srf, useSurfaceWidth);
}

const ZL_DemoUIMessage& ZL_DemoUIMessage::setText(const char* txt, bool resizeTo) const
{
	assert(impl);
	((ZL_DemoUIMessage_Impl*)impl)->txt = ZL_TextBuffer(data->fntUIFont, txt);
	if (resizeTo) ((ZL_DemoUIMessage_Impl*)impl)->txt.GetDimensions(&impl->width, &impl->height);
	return *this;
}

const ZL_DemoUIMessage& ZL_DemoUIMessage::setSurface(const ZL_Surface& srf, bool resizeTo) const
{
	assert(impl);
	if (resizeTo)
	{
		((ZL_DemoUIMessage_Impl*)impl)->height = (scalar)(srf.GetHeight());
		((ZL_DemoUIMessage_Impl*)impl)->width = (scalar)(srf.GetWidth());
	}
	((ZL_DemoUIMessage_Impl*)impl)->srf = srf;
	((ZL_DemoUIMessage_Impl*)impl)->srf.SetDrawOrigin(ZL_Origin::Center);
	((ZL_DemoUIMessage_Impl*)impl)->srf.SetRotateDeg(((ZL_DemoUIMessage_Impl*)impl)->rotate);
	return *this;
}

bool ZL_DemoUIMessage_Impl::Draw()
{
	if ((!anim_enter && !anim_leave && !stay) || (leaving && !anim_leave)) return false;
	ZL_DemoUIAnimation *anim = (leaving ? anim_leave : anim_enter);
	if (anim && !anim->Update(this)) { if (leaving) return false; if (stay) { anim = NULL; } else { GoAway(); leaving = true; } }
	scalar amovex = (anim ? anim->GetMoveX() : 0);
	scalar ascale = (anim ? anim->GetScale() : 1)*scale;
	if (srf)
	{
		scalar w = ascale * width / srf.GetWidth(), h = ascale * height / srf.GetHeight();
		//srf.Draw(x+movex+5, y-5, rot, w, h, ZL_Color::Black);
		srf.Draw(x+amovex+0, y+0, w, h, color);
	}
	if (txt)
	{
		scalar shadowdepth = 4*ascale;
		if (rotate) { ZL_Display::PushMatrix(); ZL_Display::RotateDeg(rotate); }
		txt.Draw(x+amovex+shadowdepth, y-shadowdepth, ascale, ZL_Color::Black, ZL_Origin::Center);
		txt.Draw(x+amovex, y+0, ascale, color, ZL_Origin::Center);
		if (rotate) { ZL_Display::PopMatrix(); }
	}
	return true;
}

ZL_IMPL_OWNER_INHERITED_IMPLEMENTATIONS(ZL_DemoUIButton)
ZL_DemoUIButton::ZL_DemoUIButton(const char* txt, scalar x, scalar y)  { impl = new ZL_DemoUIButton_Impl(x, y); setText(txt, true); }
ZL_DemoUIButton::ZL_DemoUIButton(const ZL_Surface& srf, scalar x, scalar y) { impl = new ZL_DemoUIButton_Impl(x, y); setSurface(srf, true); }
ZL_DemoUIButton::ZL_DemoUIButton(const ZL_Surface& srf, const char* txt, scalar x, scalar y, bool useSurfaceWidth)
{
	impl = new ZL_DemoUIButton_Impl(x, y);
	impl->height = 10;
	setText(txt, !useSurfaceWidth);
	setSurface(srf, useSurfaceWidth);
}

const ZL_DemoUIButton& ZL_DemoUIButton::setId(int id) const
{
	assert(impl);
	((ZL_DemoUIButton_Impl*)impl)->id = id;
	return *this;
}

ZL_Signal_v0& ZL_DemoUIButton::sigTouch() const
{
	assert(impl);
	return ((ZL_DemoUIButton_Impl*)impl)->sigTouch;
}

ZL_Signal_v1<int>& ZL_DemoUIButton::sigIdTouch() const
{
	assert(impl);
	return ((ZL_DemoUIButton_Impl*)impl)->sigIdTouch;
}

const ZL_DemoUIButton& ZL_DemoUIButton::setText(const char* txt, bool resizeTo) const
{
	assert(impl);
	ZL_DemoUIMessage::setText(txt, resizeTo);
	((ZL_DemoUIButton_Impl*)impl)->touchRadius = (int)(impl->height/5);
	return *this;
}

const ZL_DemoUIButton& ZL_DemoUIButton::setSurface(const ZL_Surface& srf, bool resizeTo) const
{
	assert(impl);
	ZL_DemoUIMessage::setSurface(srf, resizeTo);
	((ZL_DemoUIButton_Impl*)impl)->touchRadius = (int)(impl->height/5);
	return *this;
}

bool ZL_DemoUIButton_Impl::Touched(scalar touchx, scalar touchy)
{
	if (!stay || (!sigTouch.HasConnections() && !sigIdTouch.HasConnections())) return false;
	ZL_Vector p(x, y), t(touchx, touchy);
	if (rotate) t.Rotate(p, rotate);
	scalar rad = height*scale/2 + touchRadius;
	if (ZL_Rectf(p.x-width/2*scale, p.y - rad, p.x+width/2*scale, p.y + rad).Contains(t)) { sigTouch.call(); sigIdTouch.call(id); return true; }
	p.x += width*scale/2 - touchRadius;
	if ((t-p).GetLength() < rad) { sigTouch.call(); sigIdTouch.call(id); return true; }
	p.x -= width*scale - touchRadius*2;
	if ((t-p).GetLength() < rad) { sigTouch.call(); sigIdTouch.call(id); return true; }
	return false;
}

void ZL_DemoUIButton_Impl::DrawDebug()
{
	if (!stay || (!sigTouch.HasConnections() && !sigIdTouch.HasConnections())) return;
	//rot not represented
	ZL_Vector p(x, y);
	scalar rad = height*scale/2 + touchRadius;
	ZL_Display::DrawRect(p.x-width/2*scale, p.y - rad, p.x+width/2*scale, p.y + rad, ZL_Color::Red);
	p.x += width*scale/2 - touchRadius;
	ZL_Display::DrawCircle(p.x, p.y, rad, ZL_Color::Red);
	p.x -= width*scale - touchRadius*2;
	ZL_Display::DrawCircle(p.x, p.y, rad, ZL_Color::Red);
}

#endif //ZL_OPT_DO_IMPLEMENTATION
