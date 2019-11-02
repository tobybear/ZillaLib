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

#ifndef __ZL_INPUT__
#define __ZL_INPUT__

#include "ZL_Signal.h"
#include "ZL_Events.h"

//This is a different interface for input events which is based on queries instead of the event based ZL_Display interface.
struct ZL_Input
{
	static void Init();

	//Keyboard queries
	static bool Down(ZL_Key key, bool consume = false);
	static bool Up(ZL_Key key, bool consume = false);
	static bool Held(ZL_Key key, bool consume = false);
	static void Consume(ZL_Key key);
	static int KeyDownCount();
	static int KeyUpCount();
	static int KeyHeldCount();

	//Mouse/Touch queries
	static int Down(int btn = ZL_BUTTON_LEFT, bool consume = false);
	static int Up(int btn = ZL_BUTTON_LEFT, bool consume = false);
	static int Held(int btn = ZL_BUTTON_LEFT, bool consume = false);
	static int Clicked(int btn = ZL_BUTTON_LEFT, bool consume = true);
	static int Down(const ZL_Rectf& rec, int btn = ZL_BUTTON_LEFT, bool consume = false);
	static int Up(const ZL_Rectf& rec, int btn = ZL_BUTTON_LEFT, bool consume = false);
	static int Held(const ZL_Rectf& rec, int btn = ZL_BUTTON_LEFT, bool consume = false);
	static int Clicked(const ZL_Rectf& rec, int btn = ZL_BUTTON_LEFT, bool consume = true);
	static int DownOutside(const ZL_Rectf& rec, int btn = ZL_BUTTON_LEFT, bool consume = false);
	static int UpOutside(const ZL_Rectf& rec, int btn = ZL_BUTTON_LEFT, bool consume = false);
	static int HeldOutside(const ZL_Rectf& rec, int btn = ZL_BUTTON_LEFT, bool consume = false);
	static int ClickedOutside(const ZL_Rectf& rec, int btn = ZL_BUTTON_LEFT, bool consume = true);
	static int Drag(const ZL_Rectf& rec, int btn = ZL_BUTTON_LEFT, bool consume = false);
	static int DragOutside(const ZL_Rectf& rec, int btn = ZL_BUTTON_LEFT, bool consume = false);
	static int DragTo(const ZL_Rectf& rec, int btn = ZL_BUTTON_LEFT, bool consume = false);
	static int DragFrom(const ZL_Rectf& rec, int btn = ZL_BUTTON_LEFT, bool consume = false);
	static int Hover(const ZL_Rectf& rec);
	static void Consume(int pointernum = 1, int btn = ZL_BUTTON_LEFT);
	static bool PointerUp(int pointernum, int btn = ZL_BUTTON_LEFT, bool consume = false);
	static ZL_Vector Pointer(int pointernum = 1); //touch or mouse screen position
	static ZL_Vector PointerDelta(int pointernum = 1); //touch or mouse move delta this frame
	static ZL_Vector MouseDelta(); //accumulated movement this frame, works for locked mouse pointer
	static scalar MouseWheel();

	//input locking (i.e. for modal ui)
	static void SetLock(unsigned char lock_code);
	static inline void RemoveLock() { SetLock(0); }
	static void UnlockBegin(unsigned char lock_code);
	static inline void UnlockEnd() { UnlockBegin(0); }
	static bool IsLock(unsigned char lock_code);
	static inline bool IsLocked() { return !IsLock(0); }
};

//Interfaces with joysticks and gamepads and other input types with analog data (like mobile device motion sensors)
struct ZL_Joystick
{
	static bool Init();

	static int NumJoysticks();
	static ZL_JoystickData* JoystickOpen(int device_index);
	static void JoystickClose(ZL_JoystickData *JoyStick);

	static ZL_Signal_v1<ZL_JoyAxisEvent&> sigJoyAxis;
	static ZL_Signal_v1<ZL_JoyBallEvent&> sigJoyBall;
	static ZL_Signal_v1<ZL_JoyHatEvent&> sigJoyHat;
	static ZL_Signal_v1<ZL_JoyButtonEvent&> sigJoyButtonDown, sigJoyButtonUp;

	static void AllSigDisconnect(void *callback_class_inst);
};

#endif //__ZL_INPUT__
