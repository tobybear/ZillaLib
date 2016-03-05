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

#ifndef __ZL_THREAD__
#define __ZL_THREAD__

#include "ZL_Application.h"

struct ZL_Thread
{
	ZL_Thread();
	ZL_Thread(void (*fn)());
	ZL_Thread(void (*fn)(void *));
	ZL_Thread(int (*fn)());
	ZL_Thread(int (*fn)(void *));
	template<class CallbackClass> static inline ZL_Thread Create(CallbackClass *self, void(CallbackClass::*func)()) { return implslot1(new ZL_MethodSlot_v0<CallbackClass>(self, func)); }
	template<class CallbackClass> static inline ZL_Thread Create(CallbackClass *self, void(CallbackClass::*func)(void*)) { return implslot2(new ZL_MethodSlot_v1<CallbackClass, void*>(self, func)); }
	template<class CallbackClass> static inline ZL_Thread Create(CallbackClass *self, void(CallbackClass::*func)(int*)) { return implslot3(new ZL_MethodSlot_v1<CallbackClass, int*>(self, func)); }
	template<class CallbackClass> static inline ZL_Thread Create(CallbackClass *self, void(CallbackClass::*func)(void*, int*)) { return implslot4(new ZL_MethodSlot_v2<CallbackClass, void*, int*>(self, func)); }
	~ZL_Thread();
	ZL_Thread(const ZL_Thread &source);
	ZL_Thread &operator=(const ZL_Thread &source);
	operator bool () const { return (impl!=NULL); }
	bool operator==(const ZL_Thread &b) const { return (impl==b.impl); }
	bool operator!=(const ZL_Thread &b) const { return (impl!=b.impl); }

	void Run(void *data = NULL);
	int Wait();

	static void Sleep(int ms);

	private: struct ZL_Thread_Impl* impl; static ZL_Thread implslot1(ZL_Slot*); static ZL_Thread implslot2(ZL_Slot*); static ZL_Thread implslot3(ZL_Slot*); static ZL_Thread implslot4(ZL_Slot*);
};

#if !defined(__SMARTPHONE__) && !defined(__WEBAPP__)
struct ZL_Mutex
{
	ZL_Mutex();
	~ZL_Mutex();
	ZL_Mutex(const ZL_Mutex &source);
	ZL_Mutex &operator=(const ZL_Mutex &source);

	void Lock();
	void Unlock();
	bool TryLock();

	private: struct ZL_Mutex_Impl* impl;
};

struct ZL_Semaphore
{
	ZL_Semaphore();
	~ZL_Semaphore();
	ZL_Semaphore(const ZL_Semaphore &source);
	ZL_Semaphore &operator=(const ZL_Semaphore &source);

	bool Wait();
	bool TryWait();
	bool WaitTimeout(int ms);
	void Post();
	unsigned int Value();

	private: struct ZL_Semaphore_Impl* impl;
};
#endif

#endif
