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

#ifndef __ZL_EVENTS__
#define __ZL_EVENTS__

#include "ZL_Math.h"

//List of input keys
enum ZL_Key
{
	ZLK_UNKNOWN = 0,

	ZLK_A = 4,
	ZLK_B = 5,
	ZLK_C = 6,
	ZLK_D = 7,
	ZLK_E = 8,
	ZLK_F = 9,
	ZLK_G = 10,
	ZLK_H = 11,
	ZLK_I = 12,
	ZLK_J = 13,
	ZLK_K = 14,
	ZLK_L = 15,
	ZLK_M = 16,
	ZLK_N = 17,
	ZLK_O = 18,
	ZLK_P = 19,
	ZLK_Q = 20,
	ZLK_R = 21,
	ZLK_S = 22,
	ZLK_T = 23,
	ZLK_U = 24,
	ZLK_V = 25,
	ZLK_W = 26,
	ZLK_X = 27,
	ZLK_Y = 28,
	ZLK_Z = 29,

	ZLK_1 = 30,
	ZLK_2 = 31,
	ZLK_3 = 32,
	ZLK_4 = 33,
	ZLK_5 = 34,
	ZLK_6 = 35,
	ZLK_7 = 36,
	ZLK_8 = 37,
	ZLK_9 = 38,
	ZLK_0 = 39,

	ZLK_RETURN = 40,
	ZLK_ESCAPE = 41,
	ZLK_BACKSPACE = 42,
	ZLK_TAB = 43,
	ZLK_SPACE = 44,

	ZLK_MINUS = 45,
	ZLK_EQUALS = 46,
	ZLK_LEFTBRACKET = 47,
	ZLK_RIGHTBRACKET = 48,
	ZLK_BACKSLASH = 49,
	ZLK_SHARP = 50,
	ZLK_SEMICOLON = 51,
	ZLK_APOSTROPHE = 52,
	ZLK_GRAVE = 53,
	ZLK_COMMA = 54,
	ZLK_PERIOD = 55,
	ZLK_SLASH = 56,

	ZLK_CAPSLOCK = 57,

	ZLK_F1 = 58,
	ZLK_F2 = 59,
	ZLK_F3 = 60,
	ZLK_F4 = 61,
	ZLK_F5 = 62,
	ZLK_F6 = 63,
	ZLK_F7 = 64,
	ZLK_F8 = 65,
	ZLK_F9 = 66,
	ZLK_F10 = 67,
	ZLK_F11 = 68,
	ZLK_F12 = 69,

	ZLK_PRINTSCREEN = 70,
	ZLK_SCROLLLOCK = 71,
	ZLK_PAUSE = 72,
	ZLK_INSERT = 73,
	ZLK_HOME = 74,
	ZLK_PAGEUP = 75,
	ZLK_DELETE = 76,
	ZLK_END = 77,
	ZLK_PAGEDOWN = 78,
	ZLK_RIGHT = 79,
	ZLK_LEFT = 80,
	ZLK_DOWN = 81,
	ZLK_UP = 82,

	ZLK_NUMLOCKCLEAR = 83,

	ZLK_KP_DIVIDE = 84,
	ZLK_KP_MULTIPLY = 85,
	ZLK_KP_MINUS = 86,
	ZLK_KP_PLUS = 87,
	ZLK_KP_ENTER = 88,
	ZLK_KP_1 = 89,
	ZLK_KP_2 = 90,
	ZLK_KP_3 = 91,
	ZLK_KP_4 = 92,
	ZLK_KP_5 = 93,
	ZLK_KP_6 = 94,
	ZLK_KP_7 = 95,
	ZLK_KP_8 = 96,
	ZLK_KP_9 = 97,
	ZLK_KP_0 = 98,
	ZLK_KP_PERIOD = 99,

	ZLK_NONUSBACKSLASH = 100,
	ZLK_APPLICATION = 101,
	ZLK_POWER = 102,
	ZLK_KP_EQUALS = 103,
	ZLK_F13 = 104,
	ZLK_F14 = 105,
	ZLK_F15 = 106,
	ZLK_F16 = 107,
	ZLK_F17 = 108,
	ZLK_F18 = 109,
	ZLK_F19 = 110,
	ZLK_F20 = 111,
	ZLK_F21 = 112,
	ZLK_F22 = 113,
	ZLK_F23 = 114,
	ZLK_F24 = 115,
	ZLK_EXECUTE = 116,
	ZLK_HELP = 117,
	ZLK_MENU = 118,
	ZLK_SELECT = 119,
	ZLK_STOP = 120,
	ZLK_AGAIN = 121,   // redo
	ZLK_UNDO = 122,
	ZLK_CUT = 123,
	ZLK_COPY = 124,
	ZLK_PASTE = 125,
	ZLK_FIND = 126,
	ZLK_MUTE = 127,
	ZLK_VOLUMEUP = 128,
	ZLK_VOLUMEDOWN = 129,

	ZLK_KP_COMMA = 133,
	ZLK_KP_EQUALSAS400 = 134,

	ZLK_INTERNATIONAL1 = 135,
	ZLK_INTERNATIONAL2 = 136,
	ZLK_INTERNATIONAL3 = 137, // Yen
	ZLK_INTERNATIONAL4 = 138,
	ZLK_INTERNATIONAL5 = 139,
	ZLK_INTERNATIONAL6 = 140,
	ZLK_INTERNATIONAL7 = 141,
	ZLK_INTERNATIONAL8 = 142,
	ZLK_INTERNATIONAL9 = 143,
	ZLK_LANG1 = 144, // IME/Alphabet toggle
	ZLK_LANG2 = 145, // IME conversion
	ZLK_LANG3 = 146, // Katakana
	ZLK_LANG4 = 147, // Hiragana
	ZLK_LANG5 = 148, // Zenkaku/Hankaku
	ZLK_LANG6 = 149, // reserved
	ZLK_LANG7 = 150, // reserved
	ZLK_LANG8 = 151, // reserved
	ZLK_LANG9 = 152, // reserved

	ZLK_ALTERASE = 153, // Erase-Eaze
	ZLK_SYSREQ = 154,
	ZLK_CANCEL = 155,
	ZLK_CLEAR = 156,
	ZLK_PRIOR = 157,
	ZLK_RETURN2 = 158,
	ZLK_SEPARATOR = 159,
	ZLK_OUT = 160,
	ZLK_OPER = 161,
	ZLK_CLEARAGAIN = 162,
	ZLK_CRSEL = 163,
	ZLK_EXSEL = 164,

	ZLK_KP_00 = 176,
	ZLK_KP_000 = 177,
	ZLK_THOUSANDSSEPARATOR = 178,
	ZLK_DECIMALSEPARATOR = 179,
	ZLK_CURRENCYUNIT = 180,
	ZLK_CURRENCYSUBUNIT = 181,
	ZLK_KP_LEFTPAREN = 182,
	ZLK_KP_RIGHTPAREN = 183,
	ZLK_KP_LEFTBRACE = 184,
	ZLK_KP_RIGHTBRACE = 185,
	ZLK_KP_TAB = 186,
	ZLK_KP_BACKSPACE = 187,
	ZLK_KP_A = 188,
	ZLK_KP_B = 189,
	ZLK_KP_C = 190,
	ZLK_KP_D = 191,
	ZLK_KP_E = 192,
	ZLK_KP_F = 193,
	ZLK_KP_XOR = 194,
	ZLK_KP_POWER = 195,
	ZLK_KP_PERCENT = 196,
	ZLK_KP_LESS = 197,
	ZLK_KP_GREATER = 198,
	ZLK_KP_AMPERSAND = 199,
	ZLK_KP_DBLAMPERSAND = 200,
	ZLK_KP_VERTICALBAR = 201,
	ZLK_KP_DBLVERTICALBAR = 202,
	ZLK_KP_COLON = 203,
	ZLK_KP_HASH = 204,
	ZLK_KP_SPACE = 205,
	ZLK_KP_AT = 206,
	ZLK_KP_EXCLAM = 207,
	ZLK_KP_MEMSTORE = 208,
	ZLK_KP_MEMRECALL = 209,
	ZLK_KP_MEMCLEAR = 210,
	ZLK_KP_MEMADD = 211,
	ZLK_KP_MEMSUBTRACT = 212,
	ZLK_KP_MEMMULTIPLY = 213,
	ZLK_KP_MEMDIVIDE = 214,
	ZLK_KP_PLUSMINUS = 215,
	ZLK_KP_CLEAR = 216,
	ZLK_KP_CLEARENTRY = 217,
	ZLK_KP_BINARY = 218,
	ZLK_KP_OCTAL = 219,
	ZLK_KP_DECIMAL = 220,
	ZLK_KP_HEXADECIMAL = 221,

	ZLK_LCTRL = 224,
	ZLK_LSHIFT = 225,
	ZLK_LALT = 226, // alt, option
	ZLK_LGUI = 227, // windows, command (apple), meta
	ZLK_RCTRL = 228,
	ZLK_RSHIFT = 229,
	ZLK_RALT = 230, // alt gr, option
	ZLK_RGUI = 231, // windows, command (apple), meta

	ZLK_MODE = 257,

	ZLK_AUDIONEXT = 258,
	ZLK_AUDIOPREV = 259,
	ZLK_AUDIOSTOP = 260,
	ZLK_AUDIOPLAY = 261,
	ZLK_AUDIOMUTE = 262,
	ZLK_MEDIASELECT = 263,
	ZLK_WWW = 264,
	ZLK_MAIL = 265,
	ZLK_CALCULATOR = 266,
	ZLK_COMPUTER = 267,
	ZLK_AC_SEARCH = 268,
	ZLK_AC_HOME = 269,
	ZLK_AC_BACK = 270,
	ZLK_AC_FORWARD = 271,
	ZLK_AC_STOP = 272,
	ZLK_AC_REFRESH = 273,
	ZLK_AC_BOOKMARKS = 274,

	ZLK_BRIGHTNESSDOWN = 275,
	ZLK_BRIGHTNESSUP = 276,
	ZLK_DISPLAYSWITCH = 277,
	ZLK_KBDILLUMTOGGLE = 278,
	ZLK_KBDILLUMDOWN = 279,
	ZLK_KBDILLUMUP = 280,
	ZLK_EJECT = 281,
	ZLK_SLEEP = 282,

	ZLK_LAST
};

//Key modifiers reported in key related events
enum ZL_KeyMod
{
	ZLKMOD_NONE  = 0x0000,
	ZLKMOD_LSHIFT= 0x0001,
	ZLKMOD_RSHIFT= 0x0002,
	ZLKMOD_LCTRL = 0x0040,
	ZLKMOD_RCTRL = 0x0080,
	ZLKMOD_LALT  = 0x0100,
	ZLKMOD_RALT  = 0x0200,
	ZLKMOD_LMETA = 0x0400,
	ZLKMOD_RMETA = 0x0800,
	ZLKMOD_NUM   = 0x1000,
	ZLKMOD_CAPS  = 0x2000,
	ZLKMOD_MODE  = 0x4000,
	ZLKMOD_RESERVED = 0x8000
};

#define ZLKMOD_CTRL  (ZLKMOD_LCTRL|ZLKMOD_RCTRL)
#define ZLKMOD_SHIFT (ZLKMOD_LSHIFT|ZLKMOD_RSHIFT)
#define ZLKMOD_ALT   (ZLKMOD_LALT|ZLKMOD_RALT)
#define ZLKMOD_META  (ZLKMOD_LMETA|ZLKMOD_RMETA)

//Mouse buttons
#define ZL_BUTTON_LEFT   1
#define ZL_BUTTON_MIDDLE 2
#define ZL_BUTTON_RIGHT  3

//Joystick related definitions
#define ZL_HAT_CENTERED  0x00
#define ZL_HAT_UP        0x01
#define ZL_HAT_RIGHT     0x02
#define ZL_HAT_DOWN      0x04
#define ZL_HAT_LEFT      0x08
#define ZL_HAT_RIGHTUP   (ZL_HAT_RIGHT|ZL_HAT_UP)
#define ZL_HAT_RIGHTDOWN (ZL_HAT_RIGHT|ZL_HAT_DOWN)
#define ZL_HAT_LEFTUP    (ZL_HAT_LEFT|ZL_HAT_UP)
#define ZL_HAT_LEFTDOWN  (ZL_HAT_LEFT|ZL_HAT_DOWN)

//Event structures passed to event signals

struct ZL_KeyboardEvent
{
	bool is_down;       // key is down
	bool is_repeat;     // some platforms report repeated keydowns on long press
	ZL_Key key;         // Key ID
	unsigned short mod; // current key modifiers (ZLKMOD)
};

class ZL_PointerMoveEvent
{
	public:
	unsigned char which; // The mouse device index
	unsigned char state; // The mouse button state (bit register)
	scalar x, y;         // The X/Y coordinates of the mouse
	scalar xrel, yrel;   // The relative motion in the X/Y direction
	inline operator ZL_Vector() { return ZL_Vector(x, y); }
	inline ZL_Vector rel() { return ZL_Vector(xrel, yrel); }
};

class ZL_PointerPressEvent
{
	public:
	unsigned char which;    // The mouse device index
	unsigned char button;   // The mouse button index
	bool is_down;           // mouse button is down
	scalar x, y;	        // The X/Y coordinates of the mouse at press time
	inline operator ZL_Vector() { return ZL_Vector(x, y); }
};

class ZL_MouseWheelEvent
{
	public:
	unsigned char which;	  // The mouse device index
	scalar x, y;	          // The scrolled motion values horizontally and vertically
	operator ZL_Vector() { return ZL_Vector(x, y); }
};

struct ZL_WindowResizeEvent
{
	scalar old_width;  // Old width
	scalar old_height; // Old height
	bool window_fullscreen, window_maximized; // Desktop platforms only
};

struct ZL_WindowActivateEvent
{
	bool key_focus, mouse_focus, minimized, maximized;
};

struct ZL_JoyAxisEvent
{
	unsigned char which; // The joystick device index
	unsigned char axis;  // The joystick axis index
	int value;           // The axis value (range: -32768 to 32767)
};

struct ZL_JoyBallEvent
{
	unsigned char which; //The joystick device index
	unsigned char ball;  // The joystick trackball index
	int xrel, yrel;      // The relative motion in the X/Y direction
};

struct ZL_JoyHatEvent
{
	unsigned char which; // The joystick device index
	unsigned char hat;   // The joystick hat index
	unsigned char value; // The hat position value. ZL_HAT_*
};

struct ZL_JoyButtonEvent
{
	unsigned char which;  // The joystick device index
	unsigned char button; // The joystick button index
	bool is_down;         // The joystick button is down
};

struct ZL_JoystickData
{
	unsigned char device_index;              // Device index
	const char *name;                        // Joystick name - system dependent
	int naxes;                               // Number of axis controls on the joystick
	signed short *axes;                      // Current axis states
	int nhats;                               // Number of hats on the joystick
	unsigned char *hats;                     // Current hat states
	int nballs;                              // Number of trackballs on the joystick
	struct balldelta { int dx, dy; } *balls; // Current ball motion deltas
	int nbuttons;                            // Number of buttons on the joystick
	unsigned char *buttons;                  // Current button states
};

#endif
