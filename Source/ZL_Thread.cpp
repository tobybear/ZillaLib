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

#include "ZL_Thread.h"
#include "ZL_Platform.h"
#include "ZL_Impl.h"

struct ZL_Thread_Impl : ZL_Impl
{
	void (*fn)();
	void* (*runthread)(void *);
	void *data;
	ZL_Slot *pslot;
	ZL_ThreadHandle hthread;

	ZL_Thread_Impl(void (*fn)(), void* (*runthread)(void *)) : fn(fn), runthread(runthread), data(NULL), pslot(NULL), hthread((ZL_ThreadHandle)NULL) { }
	ZL_Thread_Impl(ZL_Slot *pslot, void* (*runthread)(void *)) : fn(NULL),  runthread(runthread), data(NULL), pslot(pslot), hthread((ZL_ThreadHandle)NULL) { }

	~ZL_Thread_Impl()
	{
		if (pslot) delete pslot;
	}
};

#define TI ((ZL_Thread_Impl*)p)
static void* runthreada(void *p) { TI->fn(); TI->DelRef(); return (void*)1; }
static void* runthreadb(void *p) { ((void(*)(void*))TI->fn)(TI->data); TI->DelRef(); return (void*)1; }
static void* runthreadc(void *p) { int ret = ((int(*)())TI->fn)(); TI->DelRef(); return (void*)(size_t)ret; }
static void* runthreadd(void *p) { int ret = ((int(*)(void*))TI->fn)(TI->data); TI->DelRef(); return (void*)(size_t)ret; }
static void* runthread1(void *p) { ((ZL_Slot_v0*)TI->pslot)->call(); TI->DelRef(); return (void*)1; }
static void* runthread2(void *p) { ((ZL_Slot_v1<void*>*)TI->pslot)->call(TI->data); TI->DelRef(); return (void*)1; }
static void* runthread3(void *p) { int ret = 1; ((ZL_Slot_v1<int*>*)TI->pslot)->call(&ret); TI->DelRef(); return (void*)(size_t)ret; }
static void* runthread4(void *p) { int ret = 1; ((ZL_Slot_v2<void*, int*>*)TI->pslot)->call(TI->data, &ret); TI->DelRef(); return (void*)(size_t)ret; }

ZL_IMPL_OWNER_DEFAULT_IMPLEMENTATIONS(ZL_Thread)

ZL_Thread::ZL_Thread(void (*fn)())       : impl(new ZL_Thread_Impl(           fn, &runthreada)) { }
ZL_Thread::ZL_Thread(void (*fn)(void *)) : impl(new ZL_Thread_Impl((void(*)())fn, &runthreadb)) { }
ZL_Thread::ZL_Thread(int (*fn)())        : impl(new ZL_Thread_Impl((void(*)())fn, &runthreadc)) { }
ZL_Thread::ZL_Thread(int (*fn)(void *))  : impl(new ZL_Thread_Impl((void(*)())fn, &runthreadd)) { }
ZL_Thread ZL_Thread::implslot1(ZL_Slot *slot) { ZL_Thread t; t.impl = new ZL_Thread_Impl(slot, &runthread1); return t; }
ZL_Thread ZL_Thread::implslot2(ZL_Slot *slot) { ZL_Thread t; t.impl = new ZL_Thread_Impl(slot, &runthread2); return t; }
ZL_Thread ZL_Thread::implslot3(ZL_Slot *slot) { ZL_Thread t; t.impl = new ZL_Thread_Impl(slot, &runthread3); return t; }
ZL_Thread ZL_Thread::implslot4(ZL_Slot *slot) { ZL_Thread t; t.impl = new ZL_Thread_Impl(slot, &runthread4); return t; }

void ZL_Thread::Run(void *data)
{
	if (!impl || impl->hthread) return;
	impl->AddRef();
	impl->data = data;
	impl->hthread = ZL_CreateThread(impl->runthread, (void*)impl);
}

int ZL_Thread::Wait()
{
	if (!impl || !impl->hthread) return -1;
	int ret = 0;
	ZL_WaitThread(impl->hthread, &ret);
	return ret;
}

void ZL_Thread::Sleep(int ms)
{
	ZL_Delay(ms);
}
