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

#include "ZL_Input.h"
#include "ZL_Platform.h"
#include "ZL_Display_Impl.h"
#include "ZL_Display.h"

enum { ZLI_KEYMAP_SIZE = (ZLK_LAST+7)/8 };
typedef unsigned int zlipointer_t; //use 30 bits = 5 buttons, 6 mice/touches
static unsigned char ZLI_Inited, ZLI_MaxPointers, ZLI_Lock, ZLI_Unlock, ZLI_KeyHeld[ZLI_KEYMAP_SIZE], ZLI_Helds;
struct sZLI_Data { unsigned char KeyDown[ZLI_KEYMAP_SIZE], KeyUp[ZLI_KEYMAP_SIZE], KeyDowns, KeyUps, PointerGotMove; zlipointer_t PointerDown, PointerUp; scalar RelX, RelY, WheelY; };
static sZLI_Data ZLI_Incoming, ZLI_Now;
static ZL_Vector ZLI_PointerNow[6], ZLI_PointerOld[6], ZLI_PointerClicked[6];
static zlipointer_t ZLI_PointerHeld;

static void ZLI_OnKeyDownUp(ZL_KeyboardEvent& e)
{
	int index = e.key>>3, mask = (1<<(e.key&7));
	if (e.is_repeat) { }
	else if (e.is_down) { ZLI_Incoming.KeyDown[index] |= mask; ZLI_Incoming.KeyDowns++; ZLI_KeyHeld[index] |=  mask; ZLI_Helds++; }
	else                { ZLI_Incoming.KeyUp[index]   |= mask; ZLI_Incoming.KeyUps++;   ZLI_KeyHeld[index] &= ~mask; ZLI_Helds--; }
}

static void ZLI_OnPointerDownUp(ZL_PointerPressEvent& e)
{
	if (e.which >= ZLI_MaxPointers) { if (e.which >= 6) return; ZLI_MaxPointers = e.which+1; }
	if (!e.button || e.button >= 6) return;
	zlipointer_t mask = 1 << (e.which * 5 + e.button - 1);
	if (e.is_down) { ZLI_Incoming.PointerDown |= mask; ZLI_PointerHeld |=  mask; ZLI_PointerClicked[e.which] = e; }
	else           { ZLI_Incoming.PointerUp   |= mask; ZLI_PointerHeld &= ~mask; }
	ZLI_PointerNow[e.which] = e;
}

static void ZLI_OnPointerMove(ZL_PointerMoveEvent& e)
{
	if (e.which >= ZLI_MaxPointers) { if (e.which >= 6) return; ZLI_MaxPointers = e.which+1; }
	if (!(ZLI_Incoming.PointerGotMove & (1 << e.which))) { ZLI_Incoming.PointerGotMove |= 1<<e.which; ZLI_PointerOld[e.which] = ZLI_PointerNow[e.which]; }
	ZLI_PointerNow[e.which] = e;
	ZLI_Incoming.RelX += e.xrel;
	ZLI_Incoming.RelY += e.yrel;
}

static void ZLI_OnMouseWheel(ZL_MouseWheelEvent& e)
{
	ZLI_Incoming.WheelY += e.y;
}

static void ZL_Input_KeepAlive()
{
	ZLI_Now = ZLI_Incoming;
	memset(&ZLI_Incoming, 0, sizeof(ZLI_Incoming));
}

void ZL_Input::Init()
{
	if (ZLI_Inited) return;
	ZLI_Inited = 1;
	ZL_Display::sigKeyDown.connect(&ZLI_OnKeyDownUp);
	ZL_Display::sigKeyUp.connect(&ZLI_OnKeyDownUp);
	ZL_Display::sigPointerDown.connect(&ZLI_OnPointerDownUp);
	ZL_Display::sigPointerUp.connect(&ZLI_OnPointerDownUp);
	ZL_Display::sigPointerMove.connect(&ZLI_OnPointerMove);
	ZL_Display::sigMouseWheel.connect(&ZLI_OnMouseWheel);
	ZL_Application::sigKeepAlive.connect(&ZL_Input_KeepAlive);
}

bool ZL_Input::IsLock(unsigned char lock_code) { return ZLI_Lock == lock_code; }
void ZL_Input::SetLock(unsigned char lock_code) { ZLI_Lock = lock_code; }
void ZL_Input::UnlockBegin(unsigned char lock_code) { ZLI_Unlock = lock_code; }

bool ZL_Input::Down(ZL_Key key, bool consume)
{
	if (ZLI_Lock != ZLI_Unlock || !(ZLI_Now.KeyDown[key>>3] & (1<<(key&7)))) return false;
	if (consume) ZLI_Now.KeyDown[key>>3] &= ~(1<<(key&7));
	return true;
}

bool ZL_Input::Up(ZL_Key key, bool consume)
{
	if (ZLI_Lock != ZLI_Unlock || !(ZLI_Now.KeyUp[key>>3] & (1<<(key&7)))) return false;
	if (consume) ZLI_Now.KeyUp[key>>3] &= ~(1<<(key&7));
	return true;
}

bool ZL_Input::Held(ZL_Key key, bool consume)
{
	if (ZLI_Lock != ZLI_Unlock || !(ZLI_KeyHeld[key>>3] & (1<<(key&7)))) return false;
	if (consume) ZLI_KeyHeld[key>>3] &= ~(1<<(key&7));
	return true;
}

void ZL_Input::Consume(ZL_Key key)
{
	ZLI_Now.KeyDown[key>>3] &= ~(1<<(key&7));
	ZLI_Now.KeyUp[key>>3] &= ~(1<<(key&7));
	ZLI_KeyHeld[key>>3] &= ~(1<<(key&7));
}

int ZL_Input::KeyDownCount() { return ZLI_Now.KeyDowns; }
int ZL_Input::KeyUpCount() { return ZLI_Now.KeyUps; }
int ZL_Input::KeyHeldCount() { return ZLI_Helds; }

#define ZLI_CONSUME_POINTERDOWN(i) if (consume) ZLI_Now.PointerDown &= ~(1 << ((i) * 5 + btn - 1));
#define ZLI_CONSUME_POINTERUP(i)   if (consume) ZLI_Now.PointerUp   &= ~(1 << ((i) * 5 + btn - 1));
#define ZLI_CONSUME_POINTERHELD(i) if (consume) ZLI_PointerHeld     &= ~(1 << ((i) * 5 + btn - 1));

int ZL_Input::Down(int btn, bool consume)
{
	if (ZLI_Lock == ZLI_Unlock)
		for (unsigned char i = 0; i < ZLI_MaxPointers; i++)
			if (ZLI_Now.PointerDown & (1 << (i * 5 + btn - 1)))
				{ ZLI_CONSUME_POINTERDOWN(i) return i+1; }
	return 0;
}

int ZL_Input::Up(int btn, bool consume)
{
	if (ZLI_Lock == ZLI_Unlock)
		for (unsigned char i = 0; i < ZLI_MaxPointers; i++)
			if (ZLI_Now.PointerUp & (1 << (i * 5 + btn - 1)))
				{ ZLI_CONSUME_POINTERUP(i) return i+1; }
	return 0;
}

int ZL_Input::Held(int btn, bool consume)
{
	if (ZLI_Lock == ZLI_Unlock)
		for (unsigned char i = 0; i < ZLI_MaxPointers; i++)
			if (ZLI_PointerHeld & (1 << (i * 5 + btn - 1)))
				{ ZLI_CONSUME_POINTERHELD(i) return i+1; }
	return 0;
}

int ZL_Input::Clicked(int btn, bool consume)
{
	if (ZLI_Lock == ZLI_Unlock)
		for (unsigned char i = 0; i < ZLI_MaxPointers; i++)
			if (ZLI_Now.PointerUp & (1 << (i * 5 + btn - 1)))
				{ ZLI_CONSUME_POINTERUP(i) return i+1; }
	return 0;
}

int ZL_Input::Down(const ZL_Rectf& rec, int btn, bool consume)
{
	if (ZLI_Lock == ZLI_Unlock)
		for (unsigned char i = 0; i < ZLI_MaxPointers; i++)
			if (ZLI_Now.PointerDown & (1 << (i * 5 + btn - 1)) && rec.Contains(ZLI_PointerNow[i]))
				{ ZLI_CONSUME_POINTERDOWN(i) return i+1; }
	return 0;
}

int ZL_Input::Up(const ZL_Rectf& rec, int btn, bool consume)
{
	if (ZLI_Lock == ZLI_Unlock)
		for (unsigned char i = 0; i < ZLI_MaxPointers; i++)
			if (ZLI_Now.PointerUp & (1 << (i * 5 + btn - 1)) && rec.Contains(ZLI_PointerNow[i]))
				{ ZLI_CONSUME_POINTERUP(i) return i+1; }
	return 0;
}

int ZL_Input::Held(const ZL_Rectf& rec, int btn, bool consume)
{
	if (ZLI_Lock == ZLI_Unlock)
		for (unsigned char i = 0; i < ZLI_MaxPointers; i++)
			if (ZLI_PointerHeld & (1 << (i * 5 + btn - 1)) && rec.Contains(ZLI_PointerNow[i]))
				{ ZLI_CONSUME_POINTERHELD(i) return i+1; }
	return 0;
}

int ZL_Input::Clicked(const ZL_Rectf& rec, int btn, bool consume)
{
	if (ZLI_Lock == ZLI_Unlock)
		for (unsigned char i = 0; i < ZLI_MaxPointers; i++)
			if (ZLI_Now.PointerUp & (1 << (i * 5 + btn - 1)) && rec.Contains(ZLI_PointerNow[i]) && rec.Contains(ZLI_PointerClicked[i]))
				{ ZLI_CONSUME_POINTERUP(i) return i+1; }
	return 0;
}

int ZL_Input::DownOutside(const ZL_Rectf& rec, int btn, bool consume)
{
	if (ZLI_Lock == ZLI_Unlock)
		for (unsigned char i = 0; i < ZLI_MaxPointers; i++)
			if (ZLI_Now.PointerDown & (1 << (i * 5 + btn - 1)) && !rec.Contains(ZLI_PointerNow[i]))
				{ ZLI_CONSUME_POINTERDOWN(i) return i+1; }
	return 0;
}

int ZL_Input::UpOutside(const ZL_Rectf& rec, int btn, bool consume)
{
	if (ZLI_Lock == ZLI_Unlock)
		for (unsigned char i = 0; i < ZLI_MaxPointers; i++)
			if (ZLI_Now.PointerUp & (1 << (i * 5 + btn - 1)) && !rec.Contains(ZLI_PointerNow[i]))
				{ ZLI_CONSUME_POINTERUP(i) return i+1; }
	return 0;
}

int ZL_Input::HeldOutside(const ZL_Rectf& rec, int btn, bool consume)
{
	if (ZLI_Lock == ZLI_Unlock)
		for (unsigned char i = 0; i < ZLI_MaxPointers; i++)
			if (ZLI_PointerHeld & (1 << (i * 5 + btn - 1)) && !rec.Contains(ZLI_PointerNow[i]))
				{ ZLI_CONSUME_POINTERHELD(i) return i+1; }
	return 0;
}

int ZL_Input::ClickedOutside(const ZL_Rectf& rec, int btn, bool consume)
{
	if (ZLI_Lock == ZLI_Unlock)
		for (unsigned char i = 0; i < ZLI_MaxPointers; i++)
			if (ZLI_Now.PointerUp & (1 << (i * 5 + btn - 1)) && !rec.Contains(ZLI_PointerNow[i]) && !rec.Contains(ZLI_PointerClicked[i]))
				{ ZLI_CONSUME_POINTERUP(i) return i+1; }
	return 0;
}

int ZL_Input::Drag(const ZL_Rectf& rec, int btn, bool consume)
{
	if (ZLI_Lock == ZLI_Unlock)
		for (unsigned char i = 0; i < ZLI_MaxPointers; i++)
			if (ZLI_PointerHeld & (1 << (i * 5 + btn - 1)) && rec.Contains(ZLI_PointerClicked[i]) && rec.Contains(ZLI_PointerNow[i]))
				{ ZLI_CONSUME_POINTERHELD(i) return i+1; }
	return 0;
}

int ZL_Input::DragOutside(const ZL_Rectf& rec, int btn, bool consume)
{
	if (ZLI_Lock == ZLI_Unlock)
		for (unsigned char i = 0; i < ZLI_MaxPointers; i++)
			if (ZLI_PointerHeld & (1 << (i * 5 + btn - 1)) && !rec.Contains(ZLI_PointerClicked[i]) && !rec.Contains(ZLI_PointerNow[i]))
				{ ZLI_CONSUME_POINTERHELD(i) return i+1; }
	return 0;
}

int ZL_Input::DragTo(const ZL_Rectf& rec, int btn, bool consume)
{
	if (ZLI_Lock == ZLI_Unlock)
		for (unsigned char i = 0; i < ZLI_MaxPointers; i++)
			if (ZLI_PointerHeld & (1 << (i * 5 + btn - 1)) && rec.Contains(ZLI_PointerNow[i]))
				{ ZLI_CONSUME_POINTERHELD(i) return i+1; }
	return 0;
}

int ZL_Input::DragFrom(const ZL_Rectf& rec, int btn, bool consume)
{
	if (ZLI_Lock == ZLI_Unlock)
		for (unsigned char i = 0; i < ZLI_MaxPointers; i++)
			if (ZLI_PointerHeld & (1 << (i * 5 + btn - 1)) && rec.Contains(ZLI_PointerClicked[i]))
				{ ZLI_CONSUME_POINTERHELD(i) return i+1; }
	return 0;
}

int ZL_Input::Hover(const ZL_Rectf& rec)
{
	if (ZLI_Lock == ZLI_Unlock)
		for (unsigned char i = 0; i < ZLI_MaxPointers; i++)
			if (rec.Contains(ZLI_PointerNow[i])) return i+1;
	return 0;
}

void ZL_Input::Consume(int pointernum, int btn)
{
	zlipointer_t mask = ~(1 << ((pointernum - 1) * 5 + btn - 1));
	ZLI_Now.PointerDown &= ~mask;
	ZLI_Now.PointerUp &= mask;
	ZLI_PointerHeld &= ~mask;
}

bool ZL_Input::PointerUp(int pointernum, int btn, bool consume)
{
	if (ZLI_Lock == ZLI_Unlock)
		if (ZLI_Now.PointerUp & (1 << ((pointernum-1) * 5 + btn - 1)))
			{ ZLI_CONSUME_POINTERUP(pointernum-1) return true; }
	return false;
}

ZL_Vector ZL_Input::Pointer(int pointernum)
{
	return (ZLI_Lock == ZLI_Unlock && pointernum && pointernum <= 6 ? ZLI_PointerNow[pointernum-1] : ZL_Vector::Zero);
}

ZL_Vector ZL_Input::PointerDelta(int pointernum)
{
	return (ZLI_Lock == ZLI_Unlock && pointernum && pointernum <= 6 && (ZLI_Now.PointerGotMove & (1 << (pointernum-1))) ? ZLI_PointerNow[pointernum-1] - ZLI_PointerOld[pointernum-1] : ZL_Vector::Zero);
}

ZL_Vector ZL_Input::MouseDelta() { return ZL_Vector(ZLI_Now.RelX, ZLI_Now.RelY); }

scalar ZL_Input::MouseWheel() { return (ZLI_Lock == ZLI_Unlock ? ZLI_Now.WheelY : 0); }

//---------------------------------------------------------------------------------------------------------------------------------

ZL_Signal_v1<ZL_JoyAxisEvent&> ZL_Joystick::sigJoyAxis;
ZL_Signal_v1<ZL_JoyBallEvent&> ZL_Joystick::sigJoyBall;
ZL_Signal_v1<ZL_JoyHatEvent&> ZL_Joystick::sigJoyHat;
ZL_Signal_v1<ZL_JoyButtonEvent&> ZL_Joystick::sigJoyButtonDown, ZL_Joystick::sigJoyButtonUp;

static bool ZL_Joystick_Process_Event(ZL_Event& event)
{
	switch (event.type)
	{
		case ZL_EVENT_JOYAXISMOTION: // Joystick axis motion
			//if (event.jaxis.axis == 1) ZL_LOG3("ZillaLib", "Joystick::Motion - Id: %d - Axis: %d - Val: %d", event.jaxis.which, event.jaxis.axis, event.jaxis.value);
			ZL_Joystick::sigJoyAxis.call(event.jaxis);
			break;
		case ZL_EVENT_JOYBALLMOTION: // Joystick trackball motion
			ZL_Joystick::sigJoyBall.call(event.jball);
			break;
		case ZL_EVENT_JOYHATMOTION: // Joystick hat position change
			ZL_Joystick::sigJoyHat.call(event.jhat);
			break;
		case ZL_EVENT_JOYBUTTONDOWN: //Joystick button pressed
			ZL_Joystick::sigJoyButtonDown.call(event.jbutton);
			break;
		case ZL_EVENT_JOYBUTTONUP: //Joystick button released
			ZL_Joystick::sigJoyButtonUp.call(event.jbutton);
			break;
		default:
			return false;
	}
	return true;
}

bool ZL_Joystick::Init()
{
	funcProcessEventsJoystick = &ZL_Joystick_Process_Event;
	#ifdef ZL_USE_JOYSTICKINIT
	return ZL_InitJoystickSubSystem();
	#else
	return true;
	#endif
}

int ZL_Joystick::NumJoysticks()
{
	return ZL_NumJoysticks();
}

ZL_JoystickData* ZL_Joystick::JoystickOpen(int device_index)
{
	return ZL_JoystickHandleOpen(device_index);
}

void ZL_Joystick::JoystickClose(ZL_JoystickData *JoyStick)
{
	ZL_JoystickHandleClose(JoyStick);
}

void ZL_Joystick::AllSigDisconnect(void *callback_class_inst)
{
	sigJoyAxis.disconnect_class(callback_class_inst);
	sigJoyBall.disconnect_class(callback_class_inst);
	sigJoyHat.disconnect_class(callback_class_inst);
	sigJoyButtonDown.disconnect_class(callback_class_inst);
	sigJoyButtonUp.disconnect_class(callback_class_inst);
}
