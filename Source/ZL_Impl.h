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

#ifndef __ZL_IMPL__
#define __ZL_IMPL__

struct ZL_Impl
{
	inline ZL_Impl() : RefCount(1) { }
	inline virtual ~ZL_Impl() {}
	inline void AddRef() { RefCount++; }
	inline void DelRef() { if (!(--RefCount)) delete this; }
	inline int GetRefCount() { return RefCount; }
	static void CopyRef(ZL_Impl* source, ZL_Impl*& target)
	{
		if (source == target) return;
		if (target) target->DelRef();
		if (source) source->AddRef();
		target = source;
	}
private:
	unsigned int RefCount;
};

#define ZL_IMPL_OWNER_NONULLCON_IMPLEMENTATIONS(ZL_CLASS) \
	ZL_CLASS::~ZL_CLASS() { if (impl) impl->DelRef(); } \
	ZL_CLASS::ZL_CLASS(const ZL_CLASS &source) : impl(0) { ZL_Impl::CopyRef(source.impl, (ZL_Impl*&)impl); } \
	ZL_CLASS & ZL_CLASS::operator=(const ZL_CLASS &source) { ZL_Impl::CopyRef(source.impl, (ZL_Impl*&)impl); return *this; }

#define ZL_IMPL_OWNER_NOASSIGNMENT_IMPLEMENTATIONS(ZL_CLASS) \
	ZL_CLASS::ZL_CLASS() : impl(NULL) { } \
	ZL_CLASS::~ZL_CLASS() { if (impl) impl->DelRef(); } \
	ZL_CLASS::ZL_CLASS(const ZL_CLASS &source) : impl(0) { ZL_Impl::CopyRef(source.impl, (ZL_Impl*&)impl); } \
	
#define ZL_IMPL_OWNER_DEFAULT_IMPLEMENTATIONS(ZL_CLASS) \
	ZL_CLASS::ZL_CLASS() : impl(NULL) { } \
	ZL_IMPL_OWNER_NONULLCON_IMPLEMENTATIONS(ZL_CLASS)

#define ZL_IMPL_OWNER_INHERITED_IMPLEMENTATIONS(ZL_CLASS) \
	ZL_CLASS::ZL_CLASS() {} \
	ZL_CLASS::~ZL_CLASS() {} \
	ZL_CLASS::ZL_CLASS(const ZL_CLASS &source) { ZL_Impl::CopyRef(source.impl, (ZL_Impl*&)impl); } \
	ZL_CLASS & ZL_CLASS::operator=(const ZL_CLASS &source) { ZL_Impl::CopyRef(source.impl, (ZL_Impl*&)impl); return *this; }

#define ZL_STATIC_ASSERT(COND,MSG) typedef char static_assertion_##MSG[(COND)?1:-1]

#if defined(_MSC_VER) && (_MSC_VER <= 1200)
#define ZL_STATIC_ISSAME(TYPE1,TYPE2) true
#else
#define ZL_STATIC_ISSAME(TYPE1,TYPE2) (ZL_STATIC_ISSAME_TEMPLATE<const TYPE1, const TYPE2>::value)
template<typename T, typename U> struct ZL_STATIC_ISSAME_TEMPLATE { static const bool value = false; };
template<typename T> struct ZL_STATIC_ISSAME_TEMPLATE<T,T> { static const bool value = true; };
#endif

#ifdef ZILLALOG
#define ZL_DEBUG_ASSERT(COND,MSG) (COND)?0:(*(int*)0)++
#else
#define ZL_DEBUG_ASSERT(COND,MSG)
#endif

template<class ImplType, class OwnerType> static inline ImplType* ZL_ImplFromOwner(OwnerType* owner); //no implementation, use the one below

template<class ImplType, class OwnerType> static inline ImplType* ZL_ImplFromOwner(OwnerType& owner)
{
	ZL_STATIC_ASSERT(sizeof(OwnerType) == sizeof(ImplType*), OwnerType_Is_Bad);
	return (*(ImplType**)&owner);
}

template<class OwnerType, class ImplType> static inline OwnerType ZL_ImplMakeOwner(ImplType* impl, bool countNewReference)
{
	ZL_STATIC_ASSERT(sizeof(OwnerType) == sizeof(ImplType*), OwnerType_Is_Bad);
	OwnerType ret;
	ZL_DEBUG_ASSERT(!*(ImplType**)&ret, Owner_Default_Constructor_Needs_Null_Impl);
	if (impl) { *(ImplType**)&ret = impl; if (countNewReference) impl->AddRef(); }
	return ret;
}

#endif //__ZL_IMPL__
