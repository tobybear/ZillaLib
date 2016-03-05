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

#ifndef __ZL_SIGNAL__
#define __ZL_SIGNAL__

#if defined(_MSC_VER) && (_MSC_VER <= 1200)
#pragma warning(disable:4530) //C++ exception handler used, but unwind semantics are not enabled
#endif
#if (!defined(_MSC_VER) || (_MSC_VER > 1600))
#define ZL_LAMBDA_SUPPORT
#endif

#include <vector>
#include <stddef.h>
#include <string.h>

#define ZL_FUNCTIONSLOT_BODY(N, PARAMDEF, PARAMLIST) { \
		typedef void (*callback) PARAMDEF; callback cb_func; \
		inline ZL_FunctionSlot_v##N(callback func) : cb_func(func) { } \
		virtual void call PARAMDEF { (*(callback)cb_func)PARAMLIST; } \
	};

#define ZL_FUNCDATASLOT_BODY(N, PARAMDEF, PARAMDEFWITHDATA, PARAMLISTWITHDATA) { \
		typedef void (*callback) PARAMDEFWITHDATA; callback cb_func; void* cb_data; \
		inline ZL_FuncDataSlot_v##N(callback func, void* data) : cb_func(func), cb_data(data) { } \
		virtual void call PARAMDEF { (*(callback)cb_func)PARAMLISTWITHDATA; } \
	};

#define ZL_METHODSLOT_BODY(N, PARAMDEF, PARAMLIST) { \
		typedef void (CallbackClass::*callback) PARAMDEF; callback cb_func; void *cb_inst;  \
		inline ZL_MethodSlot_v##N(CallbackClass *inst, callback func) : cb_func(func), cb_inst(inst) { } \
		virtual void call PARAMDEF { ((CallbackClass*)cb_inst->*(callback)cb_func)PARAMLIST; } \
	};

#define ZL_PREDICATESLOT_BODY(N, PARAMDEF, PARAMLIST) { \
		size_t sz_pred; Predicate cb_pred; \
		inline ZL_PredicateSlot_v##N(const Predicate& pred) : sz_pred(sizeof(pred)), cb_pred(pred) { } \
		virtual void call PARAMDEF { cb_pred.operator() PARAMLIST; } \
	};

struct ZL_Slot { virtual ~ZL_Slot() { } };

struct ZL_Slot_v0 : public ZL_Slot { virtual void call()=0; };
struct ZL_FunctionSlot_v0 : public ZL_Slot_v0 ZL_FUNCTIONSLOT_BODY(0, (), ())
struct ZL_FuncDataSlot_v0 : public ZL_Slot_v0 ZL_FUNCDATASLOT_BODY(0, (), (void* data), (cb_data))
template <class CallbackClass> struct ZL_MethodSlot_v0 : public ZL_Slot_v0 ZL_METHODSLOT_BODY(0, (), ())
template <typename Predicate> struct ZL_PredicateSlot_v0 : public ZL_Slot_v0 ZL_PREDICATESLOT_BODY(0, (), ())

template <class Param1> struct ZL_Slot_v1 : public ZL_Slot { virtual void call(Param1 p1)=0; };
template <class Param1> struct ZL_FunctionSlot_v1 : public ZL_Slot_v1<Param1> ZL_FUNCTIONSLOT_BODY(1, (Param1 p1), (p1))
template <class Param1> struct ZL_FuncDataSlot_v1 : public ZL_Slot_v1<Param1> ZL_FUNCDATASLOT_BODY(1, (Param1 p1), (Param1 p1, void* data), (p1, cb_data))
template <class CallbackClass, class Param1> struct ZL_MethodSlot_v1 : public ZL_Slot_v1<Param1> ZL_METHODSLOT_BODY(1, (Param1 p1), (p1))
template <typename Predicate, class Param1> struct ZL_PredicateSlot_v1 : public ZL_Slot_v1<Param1> ZL_PREDICATESLOT_BODY(1, (Param1 p1), (p1))

template <class Param1, class Param2> struct ZL_Slot_v2 : public ZL_Slot { virtual void call(Param1 p1, Param2 p2)=0; };
template <class Param1, class Param2> struct ZL_FunctionSlot_v2 : public ZL_Slot_v2<Param1, Param2> ZL_FUNCTIONSLOT_BODY(2, (Param1 p1, Param2 p2), (p1, p2))
template <class Param1, class Param2> struct ZL_FuncDataSlot_v2 : public ZL_Slot_v2<Param1, Param2> ZL_FUNCDATASLOT_BODY(2, (Param1 p1, Param2 p2), (Param1 p1, Param2 p2, void* data), (p1, p2, cb_data))
template <class CallbackClass, class Param1, class Param2> struct ZL_MethodSlot_v2 : public ZL_Slot_v2<Param1, Param2> ZL_METHODSLOT_BODY(2, (Param1 p1, Param2 p2), (p1, p2))
template <typename Predicate, class Param1, class Param2> struct ZL_PredicateSlot_v2 : public ZL_Slot_v2<Param1, Param2> ZL_PREDICATESLOT_BODY(2, (Param1 p1, Param2 p2), (p1, p2))

template <class Param1, class Param2, class Param3> struct ZL_Slot_v3 : public ZL_Slot { virtual void call(Param1 p1, Param2 p2, Param3 p3)=0; };
template <class Param1, class Param2, class Param3> struct ZL_FunctionSlot_v3 : public ZL_Slot_v3<Param1, Param2, Param3> ZL_FUNCTIONSLOT_BODY(3, (Param1 p1, Param2 p2, Param3 p3), (p1, p2, p3))
template <class Param1, class Param2, class Param3> struct ZL_FuncDataSlot_v3 : public ZL_Slot_v3<Param1, Param2, Param3> ZL_FUNCDATASLOT_BODY(3, (Param1 p1, Param2 p2, Param3 p3), (Param1 p1, Param2 p2, Param3 p3, void* data), (p1, p2, p3, cb_data))
template <class CallbackClass, class Param1, class Param2, class Param3> struct ZL_MethodSlot_v3 : public ZL_Slot_v3<Param1, Param2, Param3> ZL_METHODSLOT_BODY(3, (Param1 p1, Param2 p2, Param3 p3), (p1, p2, p3))
template <typename Predicate, class Param1, class Param2, class Param3> struct ZL_PredicateSlot_v3 : public ZL_Slot_v3<Param1, Param2, Param3> ZL_PREDICATESLOT_BODY(3, (Param1 p1, Param2 p2, Param3 p3), (p1, p2, p3))

struct ZL_Signal
{
	~ZL_Signal() { disconnect_all(); }
	void disconnect_class(void *cb_inst);
	void disconnect_all();
	bool HasConnections() { return !slots.empty(); }
protected:
	std::vector<ZL_Slot*> slots;
};

#define ZL_SIGNAL_DISCONNECT_FUNCTION_BODY(fstype)  for (std::vector<ZL_Slot*>::iterator it = slots.begin(); it != slots.end();) if ((fstype(*it))->cb_func == cb_func) { delete (*it); it = slots.erase(it); } else ++it;
#define ZL_SIGNAL_DISCONNECT_PREDICATE_BODY(pstype) for (std::vector<ZL_Slot*>::iterator it = slots.begin(); it != slots.end();) if ((pstype(*it))->sz_pred == sizeof(Predicate) && !memcmp((void*)pstype(*it), (void*)&ps, sizeof(Predicate))) { delete (*it); it = slots.erase(it); } else ++it;
#define ZL_SIGNAL_DISCONNECT_METHOD_BODY(mstype)    for (std::vector<ZL_Slot*>::iterator it = slots.begin(); it != slots.end();) if ((mstype(*it))->cb_inst == cb_inst && (mstype(*it))->cb_func == cb_func) { delete (*it); it = slots.erase(it); } else ++it;
#define ZL_SIGNAL_CALL_BODY(stype, sargs)           for (std::vector<ZL_Slot*>::iterator it = slots.begin(); it != slots.end(); ++it) (stype(*it))->call sargs;

struct ZL_Signal_v0 : public ZL_Signal
{
	void connect(void(*cb_func)()) { slots.push_back(new ZL_FunctionSlot_v0(cb_func)); }
	void disconnect(void(*cb_func)()) { ZL_SIGNAL_DISCONNECT_FUNCTION_BODY((ZL_FunctionSlot_v0*)) }
	void connect(void(*cb_func)(void*), void* cb_data) { slots.push_back(new ZL_FuncDataSlot_v0(cb_func, cb_data)); }
	void disconnect(void(*cb_func)(void*)) { ZL_SIGNAL_DISCONNECT_FUNCTION_BODY((ZL_FuncDataSlot_v0*)) }
	template<class CallbackClass> void connect(CallbackClass *cb_inst, void(CallbackClass::*cb_func)()) { slots.push_back(new ZL_MethodSlot_v0<CallbackClass>(cb_inst, cb_func)); }
	template<class CallbackClass> void disconnect(CallbackClass *cb_inst, void(CallbackClass::*cb_func)()) { ZL_SIGNAL_DISCONNECT_METHOD_BODY((ZL_MethodSlot_v0<CallbackClass>*)) }
	template<class Predicate> void connect_lambda(const Predicate& Pred) { slots.push_back(new ZL_PredicateSlot_v0<Predicate>(Pred)); }
	template<class Predicate> void disconnect_lambda(const Predicate& Pred) { ZL_PredicateSlot_v0<Predicate> ps(Pred); ZL_SIGNAL_DISCONNECT_PREDICATE_BODY((ZL_PredicateSlot_v0<Predicate>*)) }
	void call() { ZL_SIGNAL_CALL_BODY((ZL_Slot_v0*), ()) }
};

template <class Param1> struct ZL_Signal_v1 : public ZL_Signal
{
	void connect(void(*cb_func)(Param1)) { slots.push_back(new ZL_FunctionSlot_v1<Param1>(cb_func)); }
	void disconnect(void(*cb_func)(Param1)) { ZL_SIGNAL_DISCONNECT_FUNCTION_BODY((ZL_FunctionSlot_v1<Param1>*)) }
	void connect(void(*cb_func)(Param1, void*), void* cb_data) { slots.push_back(new ZL_FuncDataSlot_v1<Param1>(cb_func, cb_data)); }
	void disconnect(void(*cb_func)(Param1, void*)) { ZL_SIGNAL_DISCONNECT_FUNCTION_BODY((ZL_FuncDataSlot_v1<Param1>*)) }
	template<class CallbackClass> void connect(CallbackClass *cb_inst, void(CallbackClass::*cb_func)(Param1)) { slots.push_back(new ZL_MethodSlot_v1<CallbackClass, Param1>(cb_inst, cb_func)); }
	template<class CallbackClass> void disconnect(CallbackClass *cb_inst, void(CallbackClass::*cb_func)(Param1)) { ZL_SIGNAL_DISCONNECT_METHOD_BODY((ZL_MethodSlot_v1<CallbackClass, Param1>*)) }
	template<typename Predicate> void connect_lambda(const Predicate& Pred) { slots.push_back(new ZL_PredicateSlot_v1<Predicate, Param1>(Pred)); }
	template<typename Predicate> void disconnect_lambda(const Predicate& Pred) { ZL_PredicateSlot_v1<Predicate, Param1> ps(Pred); ZL_SIGNAL_DISCONNECT_PREDICATE_BODY((ZL_PredicateSlot_v1<Predicate, Param1>*)) }
	void call(Param1 p1) { ZL_SIGNAL_CALL_BODY((ZL_Slot_v1<Param1>*), (p1)) }
};

template <class Param1, class Param2> struct ZL_Signal_v2 : public ZL_Signal
{
	void connect(void(*cb_func)(Param1, Param2)) { slots.push_back(new ZL_FunctionSlot_v2<Param1, Param2>(cb_func)); }
	void disconnect(void(*cb_func)(Param1, Param2)) { ZL_SIGNAL_DISCONNECT_FUNCTION_BODY((ZL_FunctionSlot_v2<Param1, Param2>*)) }
	void connect(void(*cb_func)(Param1, Param2, void*), void* cb_data) { slots.push_back(new ZL_FuncDataSlot_v2<Param1, Param2>(cb_func, cb_data)); }
	void disconnect(void(*cb_func)(Param1, Param2, void*)) { ZL_SIGNAL_DISCONNECT_FUNCTION_BODY((ZL_FuncDataSlot_v2<Param1, Param2>*)) }
	template<class CallbackClass> void connect(CallbackClass *cb_inst, void(CallbackClass::*cb_func)(Param1, Param2)) { slots.push_back(new ZL_MethodSlot_v2<CallbackClass, Param1, Param2>(cb_inst, cb_func)); }
	template<class CallbackClass> void disconnect(CallbackClass *cb_inst, void(CallbackClass::*cb_func)(Param1, Param2)) { ZL_SIGNAL_DISCONNECT_METHOD_BODY((ZL_MethodSlot_v2<CallbackClass, Param1, Param2>*)) }
	template<typename Predicate> void connect_lambda(const Predicate& Pred) { slots.push_back(new ZL_PredicateSlot_v2<Predicate, Param1, Param2>(Pred)); }
	template<typename Predicate> void disconnect_lambda(const Predicate& Pred) { ZL_PredicateSlot_v2<Predicate, Param1, Param2> ps(Pred); ZL_SIGNAL_DISCONNECT_PREDICATE_BODY((ZL_PredicateSlot_v2<Predicate, Param1, Param2>*)) }
	void call(Param1 p1, Param2 p2) { ZL_SIGNAL_CALL_BODY((ZL_Slot_v2<Param1, Param2>*), (p1, p2)) }
};

template <class Param1, class Param2, class Param3> struct ZL_Signal_v3 : public ZL_Signal
{
	void connect(void(*cb_func)(Param1, Param2, Param3)) { slots.push_back(new ZL_FunctionSlot_v3<Param1, Param2, Param3>(cb_func)); }
	void disconnect(void(*cb_func)(Param1, Param2, Param3)) { ZL_SIGNAL_DISCONNECT_FUNCTION_BODY((ZL_FunctionSlot_v3<Param1, Param2, Param3>*)) }
	void connect(void(*cb_func)(Param1, Param2, Param3, void*), void* cb_data) { slots.push_back(new ZL_FuncDataSlot_v3<Param1, Param2, Param3>(cb_func, cb_data)); }
	void disconnect(void(*cb_func)(Param1, Param2, Param3, void*)) { ZL_SIGNAL_DISCONNECT_FUNCTION_BODY((ZL_FuncDataSlot_v3<Param1, Param2, Param3>*)) }
	template<class CallbackClass> void connect(CallbackClass *cb_inst, void(CallbackClass::*cb_func)(Param1, Param2, Param3)) { slots.push_back(new ZL_MethodSlot_v3<CallbackClass, Param1, Param2, Param3>(cb_inst, cb_func)); }
	template<class CallbackClass> void disconnect(CallbackClass *cb_inst, void(CallbackClass::*cb_func)(Param1, Param2, Param3)) { ZL_SIGNAL_DISCONNECT_METHOD_BODY((ZL_MethodSlot_v3<CallbackClass, Param1, Param2, Param3>*)) }
	template<typename Predicate> void connect_lambda(const Predicate& Pred) { slots.push_back(new ZL_PredicateSlot_v3<Predicate, Param1, Param2, Param3>(Pred)); }
	template<typename Predicate> void disconnect_lambda(const Predicate& Pred) { ZL_PredicateSlot_v3<Predicate, Param1, Param2, Param3> ps(Pred); ZL_SIGNAL_DISCONNECT_PREDICATE_BODY((ZL_PredicateSlot_v3<Predicate, Param1, Param2, Param3>*)) }
	void call(Param1 p1, Param2 p2, Param3 p3) { ZL_SIGNAL_CALL_BODY((ZL_Slot_v3<Param1, Param2, Param3>*), (p1, p2, p3)) }
};

#endif //__ZL_SIGNAL__
