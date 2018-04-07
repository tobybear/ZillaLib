/*
  ZillaLib
  Copyright (C) 2010-2018 Bernhard Schelling

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

#if defined(ZILLALOG)
#define ZL_IMPL_ASSERT(cond,msg) ((cond) ? ((int)0) : *(volatile int*)0 = 0xbad|printf("[FAILED ASSERT] " #cond "\n[MESSAGE] " msg "\n"))
#else
#define ZL_IMPL_ASSERT(cond,msg) ((void)0)
#endif

struct ZL_Impl
{
	inline ZL_Impl() : RefCount(1) { }
	inline virtual ~ZL_Impl() {}
	inline void AddRef() { ZL_IMPL_ASSERT(RefCount > 0 && RefCount < 0xFFFFFF, "Broken reference"); RefCount++; }
	inline void DelRef() { ZL_IMPL_ASSERT(RefCount > 0 && RefCount < 0xFFFFFF, "Broken reference"); if (!(--RefCount)) delete this; }
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

#if defined(_MSC_VER) && (_MSC_VER <= 1200)
#define ZL_STATIC_ISSAME(TYPE1,TYPE2) true
#else
#define ZL_STATIC_ISSAME(TYPE1,TYPE2) (ZL_STATIC_ISSAME_TEMPLATE<const TYPE1, const TYPE2>::value)
template<typename T, typename U> struct ZL_STATIC_ISSAME_TEMPLATE { static const bool value = false; };
template<typename T> struct ZL_STATIC_ISSAME_TEMPLATE<T,T> { static const bool value = true; };
#endif
#define ZL_IMPL_STATIC_ASSERT(cond,msg) typedef char static_assertion_##msg[(cond)?1:-1]

template<class ImplType, class OwnerType> static inline ImplType* ZL_ImplFromOwner(OwnerType* owner); //no implementation, use the one below

template<class ImplType, class OwnerType> static inline ImplType* ZL_ImplFromOwner(OwnerType& owner)
{
	ZL_IMPL_STATIC_ASSERT(sizeof(OwnerType) == sizeof(ImplType*), OwnerType_Is_Bad);
	return (*(ImplType**)&owner);
}

template<class OwnerType, class ImplType> static inline OwnerType ZL_ImplMakeOwner(ImplType* impl, bool countNewReference)
{
	ZL_IMPL_STATIC_ASSERT(sizeof(OwnerType) == sizeof(ImplType*), OwnerType_Is_Bad);
	OwnerType ret;
	ZL_IMPL_ASSERT(!*(ImplType**)&ret, "Owner default constructor needs to init impl with NULL");
	if (impl) { *(ImplType**)&ret = impl; if (countNewReference) impl->AddRef(); }
	return ret;
}

#endif //__ZL_IMPL__
