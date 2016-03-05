#ifndef __ZL_TOUCHINPUT__
#define __ZL_TOUCHINPUT__

#include <ZL_Display.h>

struct ZL_TouchInput
{
	static scalar DragStartDistance; //distance pressed needs to move until scrolling/dragging starts - defaults to 5
	static enum eScrollMode { SCROLL_SINGULAR, SCROLL_DUAL_OR_RIGHTMB } ScrollMode;
	enum eClickResult { START_DRAG, CANCEL_DRAG, START_SCROLL, UNUSED };
	//ClickResult is unused in SCROLL_SINGULAR mode, START_DRAG is the default in SCROLL_DUAL mode
	static ZL_Signal_v2<const ZL_Vector& /*click point*/, eClickResult& /*ClickResult*/> sigClick;
	static ZL_Signal_v2<const ZL_Vector& /*init point*/, const ZL_Vector& /*scroll amount*/> sigScroll;
	static ZL_Signal_v3<const ZL_Vector& /*init point*/, const ZL_Vector& /*current point*/, const ZL_Vector& /*move amount*/> sigDrag;
	static ZL_Signal_v3<const ZL_Vector& /*init point*/, scalar /*rezoom factor*/, const ZL_Vector& /*two finger zoom center*/> sigZoom;
	static ZL_Signal_v1<const ZL_Vector& /*init point*/> sigDragStart;
	static ZL_Signal_v2<const ZL_Vector& /*init point*/, const ZL_Vector& /*end point*/> sigDragEnd;
	static size_t ActiveTouches();
	static bool Init(bool useMouseWheelZoom = true);
	static void AllSigDisconnect(void *callback_class_inst);
};

#endif //__ZL_TOUCHINPUT__

#ifdef ZL_OPT_DO_IMPLEMENTATION

enum ZL_TouchInput::eScrollMode ZL_TouchInput::ScrollMode;
scalar ZL_TouchInput::DragStartDistance;
ZL_Signal_v2<const ZL_Vector& /*click point*/, ZL_TouchInput::eClickResult& /*ClickResult*/> ZL_TouchInput::sigClick;
ZL_Signal_v2<const ZL_Vector& /*init point*/, const ZL_Vector& /*scroll amount*/> ZL_TouchInput::sigScroll;
ZL_Signal_v3<const ZL_Vector& /*init point*/, const ZL_Vector& /*current point*/, const ZL_Vector& /*move amount*/> ZL_TouchInput::sigDrag;
ZL_Signal_v3<const ZL_Vector& /*init point*/, scalar /*rezoom factor*/, const ZL_Vector& /*two finger zoom center*/> ZL_TouchInput::sigZoom;
ZL_Signal_v1<const ZL_Vector& /*init point*/> ZL_TouchInput::sigDragStart;
ZL_Signal_v2<const ZL_Vector& /*init point*/, const ZL_Vector& /*end point*/> ZL_TouchInput::sigDragEnd;

struct sTouch
{
	ZL_Vector start, last10p[10];
	ticks_t last10t[10];
	ZL_Vector total;
	bool isdown, isdrag, isscroll;
	int lastcount, lastindex;
};
static sTouch touches[3];
static ZL_Vector scroll_inertia_start;
static ticks_t scroll_inertia_ticks;
static sTouch *click_at_touch = NULL;
static unsigned int click_at = 0;

static void _ZL_TouchInput_OnPointerDown(ZL_PointerPressEvent& e)
{
	if (ZL_TouchInput::sigScroll.HasConnections())
	{
		scroll_inertia_ticks = 0;
	}
	if (e.which > 2) return;
	sTouch& t = touches[e.which];
	t.isdown = true;
	t.isdrag = t.isscroll = false;
	t.start = e;
	t.total = ZL_Vector(0, 0);
	t.lastcount = t.lastindex = 1;
	t.last10t[0] = ZLTICKS;
	t.last10p[0] = t.start;
	if (ZL_TouchInput::sigClick.HasConnections() && ZL_TouchInput::ScrollMode == ZL_TouchInput::SCROLL_DUAL_OR_RIGHTMB && e.button == 1)
	{
		sTouch& ta = touches[(e.which == 0 ? 1 : (e.which == 1 ? 2 : 0))];
		sTouch& tb = touches[(e.which == 0 ? 2 : (e.which == 1 ? 0 : 1))];
		click_at = (!ta.isdown && !tb.isdown ? ZLTICKS+100 : 0);
		click_at_touch = &t;
	}
}

static void _ZL_TouchInput_OnPointerUp(ZL_PointerPressEvent& e)
{
	if (e.which > 2) return;
	touches[e.which].isdown = false;
	if (touches[0].isdown || touches[1].isdown || touches[2].isdown) return;
	if (touches[e.which].isdrag)
	{
		sTouch& t = touches[e.which];
		if (ZL_TouchInput::sigScroll.HasConnections() && t.isscroll && t.lastcount > 3 && (ZLTICKS - t.last10t[t.lastindex == 0 ? 9 : t.lastindex-1]) < 100)
		{
			ZL_Vector sum; int sumnum = 0;
			for (int i = t.lastindex+10-1; i >= t.lastindex && sumnum < t.lastcount; i--, sumnum++)
				if (t.last10t[i%10] - t.last10t[(i-1)%10] >= 100) break;
				else sum += t.last10p[i%10] - t.last10p[(i-1)%10];
			if (sumnum)
			{
				scroll_inertia_ticks = ZLTICKS;
				scroll_inertia_start.x = sum.x/s(sumnum)/s(2);
				scroll_inertia_start.y = sum.y/s(sumnum)/s(2);
			}
		}
		else if (!t.isscroll && t.isdrag && !touches[(e.which == 0 ? 1 : (e.which == 1 ? 2 : 0))].isdown && !touches[(e.which == 0 ? 2 : (e.which == 1 ? 0 : 1))].isdown)
		{
			ZL_TouchInput::sigDragEnd.call(t.start, e);
		}
	}
	else if (ZL_TouchInput::ScrollMode == ZL_TouchInput::SCROLL_SINGULAR && e.button == 1 && !touches[e.which].isscroll)
	{
		ZL_TouchInput::eClickResult ClickResult = ZL_TouchInput::UNUSED;
		ZL_TouchInput::sigClick.call(e, ClickResult);
	}
}

static void _ZL_TouchInput_OnPointerMove(ZL_PointerMoveEvent& e)
{
	if (!e.state || e.which > 2 || (e.xrel == 0 && e.yrel == 0)) return;
	sTouch& t = touches[e.which];
	if (!t.isdown) return; //cancelled drag
	t.last10t[t.lastindex] = ZLTICKS;
	t.last10p[t.lastindex] = e;
	int previndex = ((t.lastindex+9)%10);
	ZL_Vector rel(e.x-t.last10p[previndex].x, e.y-t.last10p[previndex].y);
	if (t.lastindex == 9) t.lastindex = 0; else t.lastindex++;
	t.lastcount++;
	bool dragstarted = false;
	if (!t.isscroll && !t.isdrag) { t.total += rel.VecAbs(); if (t.total > ZL_TouchInput::DragStartDistance) { t.isdrag = true; dragstarted = true; } }
	if (touches[(e.which == 0 ? 1 : (e.which == 1 ? 2 : 0))].isdown || touches[(e.which == 0 ? 2 : (e.which == 1 ? 0 : 1))].isdown)
		return; //Multitouch is handled during KeepAlive
	if (!t.isscroll && e.state == 1 && t.isdrag)
	{
		if (ZL_TouchInput::ScrollMode == ZL_TouchInput::SCROLL_SINGULAR)
			t.isscroll = true;
		else if (click_at)
		{
			ZL_TouchInput::eClickResult ClickResult = ZL_TouchInput::START_DRAG;
			ZL_TouchInput::sigClick.call(click_at_touch->start, ClickResult);
			if (ClickResult == ZL_TouchInput::CANCEL_DRAG) t.isdown = t.isdrag = false;
			if (ClickResult == ZL_TouchInput::START_SCROLL) t.isscroll = true;
			click_at = 0;
		}
		if (t.isdown && !t.isscroll)
		{
			if (dragstarted) ZL_TouchInput::sigDragStart.call(t.start);
			ZL_TouchInput::sigDrag.call(t.start, e, (dragstarted ? (-t.start + e) : rel));
		}
	}
	if (t.isscroll /*|| ZL_TouchInput::ScrollMode == ZL_TouchInput::SCROLL_SINGULAR*/ || !(e.state & 1))
	{
		t.isscroll = true;
		ZL_TouchInput::sigScroll.call(t.start, rel);
	}
	//if (e.which == 0) { e.which = 1; e.y -= 200; _ZL_TouchInput_OnPointerMove(e); }
}

static void _ZL_TouchInput_OnMouseWheel(ZL_MouseWheelEvent& e)
{
	if (!e.y) return;
	ZL_Vector pos(ZL_Display::PointerX, ZL_Display::PointerY);
	ZL_TouchInput::sigZoom.call(pos, (e.y > 0 ? s(0.8) : s(1.25)), pos);
}

static void _ZL_TouchInput_KeepAlive()
{
	if (scroll_inertia_ticks)
	{
		scalar f = ZLSINCESECONDS(scroll_inertia_ticks);
		if (f >= s(1)) scroll_inertia_ticks = 0;
		else ZL_TouchInput::sigScroll.call(touches[0].start, scroll_inertia_start * ZL_Easing::InQuad(s(1) - f));
	}
	int num_touch_down = (touches[0].isdown ? 1 : 0) + (touches[1].isdown ? 1 : 0)  + (touches[2].isdown ? 1 : 0);
	if (num_touch_down <= 1)
	{
		if (click_at && click_at <= ZLTICKS)
		{
			ZL_TouchInput::eClickResult ClickResult = ZL_TouchInput::START_DRAG;
			ZL_TouchInput::sigClick.call(click_at_touch->start, ClickResult);
			if (ClickResult == ZL_TouchInput::CANCEL_DRAG) touches[0].isdown = touches[1].isdown = touches[2].isdown = false;
			if (ClickResult == ZL_TouchInput::START_SCROLL) { touches[0].isscroll = touches[0].isdown; touches[1].isscroll = touches[1].isdown; touches[2].isscroll = touches[2].isdown; }
			click_at = 0;
		}
	}
	else if (num_touch_down == 2)
	{
		bool hasScroll = ZL_TouchInput::sigScroll.HasConnections(), hasZoom = ZL_TouchInput::sigZoom.HasConnections();
		if (hasScroll || hasZoom)
		{
			sTouch& ta = (touches[0].isdown ? touches[0] : touches[1]);
			sTouch& tb = (touches[0].isdown && touches[1].isdown ? touches[1] : touches[2]);
			if ((ta.lastcount > 1 || tb.lastcount > 1) && (ta.last10t[(ta.lastindex == 0 ? 9 : ta.lastindex-1)] == ZLTICKS || tb.last10t[(tb.lastindex == 0 ? 9 : tb.lastindex-1)] == ZLTICKS))
			{
				ZL_Vector &ta0 = ta.last10p[(ta.lastindex == 0 ? 9 : ta.lastindex-1)];
				ZL_Vector &tb0 = tb.last10p[(tb.lastindex == 0 ? 9 : tb.lastindex-1)];
				ZL_Vector &ta1 = (ta.lastcount == 1 ? ta0 : ta.last10p[(ta.lastindex < 2 ? 10-2+ta.lastindex : ta.lastindex-2)]);
				ZL_Vector &tb1 = (tb.lastcount == 1 ? tb0 : tb.last10p[(tb.lastindex < 2 ? 10-2+tb.lastindex : tb.lastindex-2)]);
				if (hasScroll && ta.lastcount >= 3 && tb.lastcount >= 3)
				{
					ZL_Vector &ta2 = ta.last10p[(ta.lastindex < 3 ? 10-3+ta.lastindex : ta.lastindex-3)];
					ZL_Vector &tb2 = tb.last10p[(tb.lastindex < 3 ? 10-3+tb.lastindex : tb.lastindex-3)];
					ZL_Vector tarel = ZL_Vector(ta1.x - ta2.x + ta0.x - ta1.x, ta1.y - ta2.y + ta0.y - ta1.y);
					ZL_Vector tbrel = ZL_Vector(tb1.x - tb2.x + tb0.x - tb1.x, tb1.y - tb2.y + tb0.y - tb1.y);
					ZL_Vector scroll_amount((tarel.x < 0 ? 1 : (tarel.x > 0 ? 2 : 0)) == (tbrel.x < 0 ? 1 : (tbrel.x > 0 ? 2 : 0)) ? (tarel.x + tbrel.x) / s(4) : 0,
											(tarel.y < 0 ? 1 : (tarel.y > 0 ? 2 : 0)) == (tbrel.y < 0 ? 1 : (tbrel.y > 0 ? 2 : 0)) ? (tarel.y + tbrel.y) / s(4) : 0);
					if (scroll_amount.x || scroll_amount.y)
					{
						ZL_TouchInput::sigScroll.call(ta.start, scroll_amount);
						ta.isscroll = tb.isscroll = true;
					}
				}
				if (hasZoom)
				{
					ZL_Vector diff = (ta0 - tb0), difflast = (ta1 - tb1);
					if ((diff.x != difflast.x || diff.y != difflast.y) && diff > 50 && difflast > 50)
						ZL_TouchInput::sigZoom.call(ta.start, difflast.GetLengthSq() / diff.GetLengthSq(), (ta0==ta1 ? ta0 : (tb0==tb1 ? tb0 : (ta0+tb0)/s(2))));
				}
			}
		}
	}
}

size_t ZL_TouchInput::ActiveTouches()
{
	return (touches[0].isdown ? 1 : 0) + (touches[1].isdown ? 1 : 0)  + (touches[2].isdown ? 1 : 0);
}

bool ZL_TouchInput::Init(bool useMouseWheelZoom)
{
	ZL_Display::sigPointerDown.connect(&_ZL_TouchInput_OnPointerDown);
	ZL_Display::sigPointerUp.connect(&_ZL_TouchInput_OnPointerUp);
	ZL_Display::sigPointerMove.connect(&_ZL_TouchInput_OnPointerMove);
	if (useMouseWheelZoom)
		ZL_Display::sigMouseWheel.connect(&_ZL_TouchInput_OnMouseWheel);
	memset(touches, 0, sizeof(touches));
	ZL_Application::sigKeepAlive.connect(&_ZL_TouchInput_KeepAlive);
	DragStartDistance = 5;
	ScrollMode = SCROLL_SINGULAR;
	//DEBUG: sTouch& t = touches[1]; t.start = ZL_Vector(1280, 720); t.isdown = true; t.isdrag = t.isscroll = false; t.total = ZL_Vector(0, 0); t.lastcount = t.lastindex = 1; t.last10t[0] = ZLTICKS; t.last10p[0] = t.start;
	return true;
}

void ZL_TouchInput::AllSigDisconnect(void *callback_class_inst)
{
	sigScroll.disconnect_class(callback_class_inst);
	sigClick.disconnect_class(callback_class_inst);
	sigDrag.disconnect_class(callback_class_inst);
	sigZoom.disconnect_class(callback_class_inst);
}

#endif //ZL_OPT_DO_IMPLEMENTATION
