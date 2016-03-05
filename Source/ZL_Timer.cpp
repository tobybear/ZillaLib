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

#include "ZL_Timer.h"
#include "ZL_Application.h"
#include <assert.h>

static std::vector<ZL_Timer*> *pActiveTimers = NULL;
static bool ActiveTimersHaveChanged = false;

static ZL_Origin::Type FlipDir(ZL_Origin::Type dir) { return (dir == ZL_Origin::TopLeft ? ZL_Origin::BottomRight : (dir == ZL_Origin::BottomRight ? ZL_Origin::BottomLeft : (dir == ZL_Origin::TopCenter ? ZL_Origin::BottomCenter : (dir == ZL_Origin::TopRight ? ZL_Origin::BottomRight : (dir == ZL_Origin::BottomLeft ? ZL_Origin::BottomRight : (dir == ZL_Origin::CenterLeft ? ZL_Origin::CenterRight : dir)))))); }

ZL_Timer::ZL_Timer(ticks_t TimeDuration, ticks_t TimeOffset, bool HasTransition) : TimeStart(ZL_Application::Ticks + TimeOffset), TimeEnd(TimeStart + TimeDuration), HasBegun(false), HasTransition(HasTransition), IsEndless(false), RemoveFlag(false) { }

struct ZL_TimerMultiOrEndless : public ZL_RepeatingTimer
{
	ZL_TimerMultiOrEndless(bool Endless, ticks_t Delay, size_t Repeats) : Delay(MAX(1, Delay)), LastCall(TimeStart) { IsEndless = Endless; TimeEnd = TimeStart + (Delay * (ticks_t)Repeats); }
	void Calculate() { while (ZLSINCE(LastCall) >= (int)Delay) { LastCall += Delay; sigCall.call(this); } }
	ticks_t Delay, LastCall;
};

struct ZL_TimerTicker : public ZL_RepeatingTimer
{
	ZL_TimerTicker(bool Endless, ticks_t TimeDuration) { IsEndless = Endless; TimeEnd = TimeStart + TimeDuration; }
	void Calculate() { sigCall.call(this); }
};

struct ZL_Transition : public ZL_Timer
{
	ZL_Transition(ticks_t TimeDuration, ticks_t TimeOffset, void *pFor, easingfunc_t pEasingFunc) : ZL_Timer(TimeDuration, TimeOffset, true), pFor(pFor), pEasingFunc(pEasingFunc) { }
	void *pFor;
	easingfunc_t pEasingFunc;
};

struct ZL_TransitionScalar : ZL_Transition
{
	ZL_TransitionScalar(ticks_t TimeDuration, scalar *pScalar, scalar TargetValue, ticks_t TimeOffset, easingfunc_t pEasingFunc) : ZL_Transition(TimeDuration, TimeOffset, pScalar, pEasingFunc)
	{
		ValueStart = *pScalar;
		ValueAmount = TargetValue - ValueStart;
	}
	void Calculate()
	{
		if (ZLSINCE(TimeEnd) >= 0) { *(scalar*)pFor = ValueStart + ValueAmount; }
		else if (ZLSINCE(TimeStart) > 0) { *(scalar*)pFor = ValueStart + ValueAmount * pEasingFunc(s(ZLSINCE(TimeStart)) / s(TimeEnd - TimeStart)); }
	}
	scalar ValueStart, ValueAmount;
};

struct ZL_TransitionRect : ZL_Transition
{
	ZL_TransitionRect(ZL_Rect *pRect, ZL_Origin::Type Direction, bool bIsMargin, bool bReverse, ticks_t TimeDuration, ticks_t TimeOffset, easingfunc_t pEasingFunc) : ZL_Transition(TimeDuration, TimeOffset, pRect, pEasingFunc), Direction(Direction), bIsMargin(bIsMargin), bReverse(bReverse)
	{
		Setup();
		if (bReverse) Calculate();
	}
	void Setup()
	{
		ZL_Rect *pRect = (ZL_Rect*)pFor; ZL_Point offscreen;
		switch (bIsMargin ? FlipDir(Direction) : Direction)
		{
			case ZL_Origin::TopLeft:      srcx = &pRect->right; srcy = &pRect->bottom; offscreen = ZL_Point(           0,             0); break;
			case ZL_Origin::TopCenter:    srcx = &pRect->right; srcy = &pRect->bottom; offscreen = ZL_Point(       *srcx,             0); break;
			case ZL_Origin::TopRight:     srcx = &pRect->left;  srcy = &pRect->bottom; offscreen = ZL_Point((int)ZLWIDTH,             0); break;
			case ZL_Origin::CenterLeft:   srcx = &pRect->right; srcy = &pRect->bottom; offscreen = ZL_Point(           0,         *srcy); break;
			case ZL_Origin::CenterRight:  srcx = &pRect->left;  srcy = &pRect->bottom; offscreen = ZL_Point((int)ZLWIDTH,         *srcy); break;
			case ZL_Origin::BottomLeft:   srcx = &pRect->right; srcy = &pRect->top;    offscreen = ZL_Point(           0, (int)ZLHEIGHT); break;
			case ZL_Origin::BottomCenter: srcx = &pRect->right; srcy = &pRect->top;    offscreen = ZL_Point(       *srcx, (int)ZLHEIGHT); break;
			case ZL_Origin::BottomRight:  srcx = &pRect->left;  srcy = &pRect->top;    offscreen = ZL_Point((int)ZLWIDTH, (int)ZLHEIGHT); break;
			default:                      srcx = &pRect->right; srcy = &pRect->bottom; offscreen = ZL_Point(           0,             0);
		}
		if (bReverse) to = ZL_Point(*srcx, *srcy), from = offscreen;
		else          from = ZL_Point(*srcx, *srcy), to = offscreen;
	}
	void Calculate()
	{
		scalar t;
		if      (ZLSINCE(TimeStart) <= 0) t = s(0);
		else if (ZLSINCE(TimeEnd)   >= 0) t = s(1);
		else if (bReverse) t = s(1) - pEasingFunc(s(ZLUNTIL(TimeEnd)) / s(TimeEnd - TimeStart));
		else t = pEasingFunc(s(ZLSINCE(TimeStart)) / s(TimeEnd - TimeStart));
		ZL_Point move((int)(s(to.x - from.x) * t + from.x - s(*srcx)), (int)(s(to.y - from.y) * t + from.y - s(*srcy)));
		if (!bIsMargin) *(ZL_Rect*)pFor += move;
		else
		{
			ZL_Rect* pRectMargin = (ZL_Rect*)pFor;
			pRectMargin->left   += (*srcx == pRectMargin->left   ? move.x : -move.x);
			pRectMargin->top    += (*srcy == pRectMargin->top    ? move.y : -move.y);
			pRectMargin->right  += (*srcx == pRectMargin->right  ? move.x : -move.x);
			pRectMargin->bottom += (*srcy == pRectMargin->bottom ? move.y : -move.y);
		}
	}
	void Reinit()
	{
		Setup();
		Calculate();
	}
	ZL_Origin::Type Direction;
	bool bIsMargin, bReverse;
	ZL_Point from, to;
	int *srcx, *srcy;
};

struct ZL_TransitionRectf : ZL_Transition
{
	ZL_TransitionRectf(ZL_Rectf *pRectf, ZL_Origin::Type Direction, bool bReverse, ticks_t TimeDuration, ticks_t TimeOffset, easingfunc_t pEasingFunc) : ZL_Transition(TimeDuration, TimeOffset, pRectf, pEasingFunc), Direction(Direction), bReverse(bReverse)
	{
		Setup();
		if (bReverse) Calculate();
	}
	void Setup()
	{
		ZL_Rectf *pRectf = (ZL_Rectf*)pFor; ZL_Vector offscreen;
		switch (Direction)
		{
			case ZL_Origin::TopLeft:      srcx = &pRectf->right; srcy = &pRectf->low;  offscreen = ZL_Vector(      0, ZLHEIGHT); break;
			case ZL_Origin::TopCenter:    srcx = &pRectf->right; srcy = &pRectf->low;  offscreen = ZL_Vector(  *srcx, ZLHEIGHT); break;
			case ZL_Origin::TopRight:     srcx = &pRectf->left;  srcy = &pRectf->low;  offscreen = ZL_Vector(ZLWIDTH, ZLHEIGHT); break;
			case ZL_Origin::CenterLeft:   srcx = &pRectf->right; srcy = &pRectf->low;  offscreen = ZL_Vector(      0,    *srcy); break;
			case ZL_Origin::CenterRight:  srcx = &pRectf->left;  srcy = &pRectf->low;  offscreen = ZL_Vector(ZLWIDTH,    *srcy); break;
			case ZL_Origin::BottomLeft:   srcx = &pRectf->right; srcy = &pRectf->high; offscreen = ZL_Vector(      0,        0); break;
			case ZL_Origin::BottomCenter: srcx = &pRectf->right; srcy = &pRectf->high; offscreen = ZL_Vector(  *srcx,        0); break;
			case ZL_Origin::BottomRight:  srcx = &pRectf->left;  srcy = &pRectf->high; offscreen = ZL_Vector(ZLWIDTH,        0); break;
			default:            srcx = &pRectf->right; srcy = &pRectf->low;  offscreen = ZL_Vector(      0, ZLHEIGHT);
		}
		if (bReverse) to = ZL_Vector(*srcx, *srcy), from = offscreen;
		else          from = ZL_Vector(*srcx, *srcy), to = offscreen;
	}
	void Calculate()
	{
		scalar t;
		if      (ZLSINCE(TimeStart) <= 0) t = s(0);
		else if (ZLSINCE(TimeEnd)   >= 0) t = s(1);
		else if (bReverse) t = s(1) - pEasingFunc(s(ZLUNTIL(TimeEnd)) / s(TimeEnd - TimeStart));
		else t = pEasingFunc(s(ZLSINCE(TimeStart)) / s(TimeEnd - TimeStart));
		ZL_Point move((int)(s(to.x - from.x) * t + from.x - s(*srcx)), (int)(s(to.y - from.y) * t + from.y - s(*srcy)));
		*(ZL_Rectf*)pFor += move;
	}
	void Reinit()
	{
		Setup();
		Calculate();
	}
	ZL_Origin::Type Direction;
	bool bReverse;
	ZL_Vector from, to;
	scalar *srcx, *srcy;
};

void ZL_Timer::KeepAlive()
{
	RestartTimerKeepAliveLoop:
	ActiveTimersHaveChanged = false;
	for (std::vector<ZL_Timer*>::iterator it = pActiveTimers->begin(); it != pActiveTimers->end();)
	{
		ZL_Timer *pTimer = *it;
		if (!pTimer->HasBegun && ZLSINCE(pTimer->TimeStart) > 0)
		{
			pTimer->HasBegun = true;
			if (pTimer->HasTransition && ZLSINCE(pTimer->TimeStart) > 15)
			{
				//move duration forward to show the entire transition even if there was a longer delay since last frame (i.e. due to data loading)
				pTimer->TimeEnd = ZLTICKS - 15 + (pTimer->TimeEnd - pTimer->TimeStart);
				pTimer->TimeStart = ZLTICKS - 15;
			}
		}
		if (pTimer->RemoveFlag)
		{
			it = pActiveTimers->erase(it);
			delete pTimer;
		}
		else if (pTimer->HasBegun && (ZLSINCE(pTimer->TimeEnd) >= 0) && !pTimer->IsEndless)
		{
			it = pActiveTimers->erase(it);
			pTimer->Calculate();
			pTimer->sigDone.call();
			delete pTimer;
			if (ActiveTimersHaveChanged) goto RestartTimerKeepAliveLoop;
		}
		else
		{
			pTimer->Calculate();
			assert(!ActiveTimersHaveChanged); //custom calculate itself should not change active timers!
			++it;
		}
	}
}

bool ZL_Timer::Init()
{
	if (pActiveTimers) return false;
	pActiveTimers = new std::vector<ZL_Timer*>();
	ZL_Application::sigKeepAlive.connect(&KeepAlive);
	return true;
}

ZL_Timer* ZL_Timer::AddTransitionRect(ZL_Rect *pRect, ZL_Origin::Type Direction, bool bReverse, ticks_t TimeDuration, ticks_t TimeOffset, easingfunc_t Easing)
{
	return AddCustom(new ZL_TransitionRect(pRect, Direction, false, bReverse, TimeDuration, TimeOffset, Easing));
}

ZL_Timer* ZL_Timer::AddTransitionMargin(ZL_Rect *pRectMargin, ZL_Origin::Type Direction, bool bReverse, ticks_t TimeDuration, ticks_t TimeOffset, easingfunc_t Easing)
{
	return AddCustom(new ZL_TransitionRect(pRectMargin, Direction, true, bReverse, TimeDuration, TimeOffset, Easing));
}

ZL_Timer* ZL_Timer::AddTransitionFloat(scalar *pFloat, scalar fTargetValue, ticks_t TimeDuration, ticks_t TimeOffset, easingfunc_t Easing)
{
	return AddCustom(new ZL_TransitionScalar(TimeDuration, pFloat, fTargetValue, TimeOffset, Easing));
}

ZL_Timer* ZL_Timer::AddTransitionScalar(scalar *pScalar, scalar TargetValue, ticks_t TimeDuration, ticks_t TimeOffset, easingfunc_t Easing)
{
	return AddCustom(new ZL_TransitionScalar(TimeDuration, pScalar, TargetValue, TimeOffset, Easing));
}

ZL_Timer* ZL_Timer::AddTransitionRectf(ZL_Rectf *pRectf, ZL_Origin::Type Direction, bool bReverse, ticks_t TimeDuration, ticks_t TimeOffset, easingfunc_t Easing)
{
	return AddCustom(new ZL_TransitionRectf(pRectf, Direction, bReverse, TimeDuration, TimeOffset, Easing));
}

ZL_Timer* ZL_Timer::AddSingleTimer(ticks_t TimeDuration)
{
	return AddCustom(new ZL_Timer(TimeDuration));
}

ZL_RepeatingTimer* ZL_Timer::AddMultiTimer(ticks_t TimeDelay, size_t Repeats)
{
	return (ZL_RepeatingTimer*)AddCustom(new ZL_TimerMultiOrEndless(false, TimeDelay, Repeats));
}

ZL_RepeatingTimer* ZL_Timer::AddEndlessTimer(ticks_t TimeDelay)
{
	return (ZL_RepeatingTimer*)AddCustom(new ZL_TimerMultiOrEndless(true, TimeDelay, 0));
}

ZL_RepeatingTimer* ZL_Timer::AddLimitedTicker(ticks_t TimeDuration)
{
	return (ZL_RepeatingTimer*)AddCustom(new ZL_TimerTicker(false, TimeDuration));
}

ZL_RepeatingTimer* ZL_Timer::AddEndlessTicker()
{
	return (ZL_RepeatingTimer*)AddCustom(new ZL_TimerTicker(true, 0));
}


ZL_Timer* ZL_Timer::AddCustom(ZL_Timer *pCustomTimer)
{
	assert(pActiveTimers);
	pActiveTimers->push_back(pCustomTimer);
	ActiveTimersHaveChanged = true;
	return pCustomTimer;
}

void ZL_Timer::ReinitAllActiveTimers()
{
	for (std::vector<ZL_Timer*>::iterator it = pActiveTimers->begin(); it != pActiveTimers->end();++it)
		(*it)->Reinit();
}

void ZL_Timer::EndTimers()
{
	for (std::vector<ZL_Timer*>::iterator it = pActiveTimers->begin(); it != pActiveTimers->end();++it) delete (*it);
	pActiveTimers->clear();
	ActiveTimersHaveChanged = true;
}

void ZL_Timer::EndTransitionFor(void *p)
{
	for (std::vector<ZL_Timer*>::iterator it = pActiveTimers->begin(); it != pActiveTimers->end(); ++it)
		if ((*it)->HasTransition && static_cast<ZL_Transition*>(*it)->pFor == p) (*it)->RemoveFlag = true;
}

bool ZL_Timer::HasActiveTimer()
{
	return !pActiveTimers->empty();
}

ZL_Timer* ZL_Timer::GetActiveTransitionFor(void *p)
{
	for (std::vector<ZL_Timer*>::iterator it = pActiveTimers->begin(); it != pActiveTimers->end();++it)
		if ((*it)->HasTransition && static_cast<ZL_Transition*>(*it)->pFor == p) return *it;
	return NULL;
}

void ZL_Timer::End()
{
	RemoveFlag = true;
}
