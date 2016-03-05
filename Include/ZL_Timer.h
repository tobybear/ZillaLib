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

#ifndef __ZL_TIMER__
#define __ZL_TIMER__

#include "ZL_Display.h"
#include "ZL_Signal.h"
#include <list>

struct ZL_Timer
{
	static bool Init();

	//callback timers (durations are given in ms)
	static ZL_Timer* AddSingleTimer(ticks_t TimeDuration); //calls sigDone after duration
	static struct ZL_RepeatingTimer* AddMultiTimer(ticks_t TimeDelay, size_t Repeats); //called once each delay for fixed number of repeats
	static struct ZL_RepeatingTimer* AddEndlessTimer(ticks_t TimeDelay); //called once each delay, must be manually ended
	static struct ZL_RepeatingTimer* AddLimitedTicker(ticks_t TimeDuration); //called once a frame until duration
	static struct ZL_RepeatingTimer* AddEndlessTicker(); //called every frame, must be manually ended
	static ZL_Timer* AddCustom(ZL_Timer *pCustomTimer);

	//change a variable over time linearly or following a curve (easing/tweening)
	typedef scalar (*easingfunc_t)(scalar);
	static ZL_Timer* AddTransitionRect(ZL_Rect *pRect, ZL_Origin::Type Direction, bool bReverse = false, ticks_t TimeDuration = 800, ticks_t TimeOffset = 0, easingfunc_t Easing = &ZL_Easing::OutQuad);
	static ZL_Timer* AddTransitionMargin(ZL_Rect *pRectMargin, ZL_Origin::Type Direction, bool bReverse = false, ticks_t TimeDuration = 800, ticks_t TimeOffset = 0, easingfunc_t Easing = &ZL_Easing::OutQuad);
	static ZL_Timer* AddTransitionFloat(scalar *pFloat, scalar fTargetValue, ticks_t TimeDuration = 800, ticks_t TimeOffset = 0, easingfunc_t Easing = &ZL_Easing::OutQuad);
	static ZL_Timer* AddTransitionScalar(scalar *pScalar, scalar TargetValue, ticks_t TimeDuration = 800, ticks_t TimeOffset = 0, easingfunc_t Easing = &ZL_Easing::OutQuad);
	static ZL_Timer* AddTransitionRectf(ZL_Rectf *pRectf, ZL_Origin::Type Direction, bool bReverse = false, ticks_t TimeDuration = 800, ticks_t TimeOffset = 0, easingfunc_t Easing = &ZL_Easing::OutQuad);

	//managing active timers
	static void ReinitAllActiveTimers();
	static void EndTimers();
	static void EndTransitionFor(void *p);
	static bool HasActiveTimer();
	static ZL_Timer* GetActiveTransitionFor(void *p);

	//regular functions on timers
	void End();
	inline ticks_t Elapsed() { return ZLSINCE(TimeStart); }
	inline scalar ElapsedSeconds() { return ZLSINCESECONDS(TimeStart); }

	ticks_t TimeStart, TimeEnd;
	ZL_Signal_v0 sigDone;

	//use this as easing function if you want linear change not following a curve
	static inline scalar NoEasing(scalar t) { return t; }

protected:
	//instances are created and managed by static functions, only pointers to them are used outside
	ZL_Timer(ticks_t TimeDuration = 800, ticks_t TimeOffset = 0, bool HasTransition = false);
	virtual ~ZL_Timer() {}
	bool HasBegun, HasTransition, IsEndless, RemoveFlag;
	virtual void Calculate() { }
	virtual void Reinit() { }

private:
	ZL_Timer(const ZL_Timer&);
	ZL_Timer& operator=(const ZL_Timer&);
	static void KeepAlive();
};

struct ZL_RepeatingTimer : public ZL_Timer
{
	ZL_Signal_v1<ZL_RepeatingTimer*> sigCall;
};

#endif //__ZL_TIMER__
