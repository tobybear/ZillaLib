/*
  ZillaLib
  Copyright (C) 2010-2020 Bernhard Schelling

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

#ifndef __ZL_STRING__
#define __ZL_STRING__

#if defined(_MSC_VER) && (_MSC_VER <= 1200)
#pragma warning(disable:4786) //'Some STL template class' : identifier was truncated to '255' characters in the debug information
#pragma warning(disable:4530) //C++ exception handler used, but unwind semantics are not enabled
#endif

#ifdef s
#pragma push_macro("s")
#undef s
#define _HAS_PUSHED_S_MACRO
#endif

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <string>
#include <vector>

#ifdef _HAS_PUSHED_S_MACRO
#pragma pop_macro("s")
#undef _HAS_PUSHED_S_MACRO
#endif

#if defined(_MSC_VER)
typedef signed __int64 i64;
typedef unsigned __int64 u64;
#else
typedef long long i64;
typedef unsigned long long u64;
#endif
typedef int i32;
typedef unsigned int u32;

class ZL_String : public std::string
{
	typedef std::string str;
	typedef unsigned char uchr;
	typedef signed char schr;

public:
	static ZL_String EmptyString;

	typedef char chr;

	ZL_String() { }

	static ZL_String format(const char *format, ...);
	static ZL_String vformat(const char *format, va_list ap);
	static void format(ZL_String& target, const char *format, ...);
	static void vformat(ZL_String& target, const char *format, va_list ap);
	static ZL_String to_upper(const ZL_String &s) { ZL_String c = s; c.to_upper(); return c; }
	static ZL_String to_lower(const ZL_String &s) { ZL_String c = s; c.to_lower(); return c; }
	static ZL_String str_replace(const ZL_String &s, const ZL_String &from, const ZL_String &to) { ZL_String c = s; c.str_replace(from, to); return c; }
	ZL_String& to_upper();
	ZL_String& to_lower();
	ZL_String& str_replace(const ZL_String &from, const ZL_String &to);
	std::vector<ZL_String> split(const ZL_String &delimiter) const;

	ZL_String(size_t n, chr c)         : str(n, c)   { }
	ZL_String(const ZL_String &source) : str(source) { }
	ZL_String(const str &source)       : str(source) { }
	ZL_String(const chr *source)       : str(source) { }
	ZL_String(const uchr *source)      : str((chr*)source) { }
	ZL_String(const schr *source)      : str((chr*)source) { }
	ZL_String(const chr *source, size_t size)  : str(source, size) { }
	ZL_String(const uchr *source, size_t size) : str((chr*)source, size) { }
	ZL_String(const schr *source, size_t size) : str((chr*)source, size) { }
	ZL_String(const chr source)  { assign(1, source); }
	ZL_String(const uchr source) { assign(1, (chr)source); }
	ZL_String(const schr source) { assign(1, (chr)source); }
	ZL_String(const unsigned int source)   { operator<<(source); }
	ZL_String(const signed int source)     { operator<<(source); }
	ZL_String(const unsigned short source) { operator<<(source); }
	ZL_String(const signed short source)   { operator<<(source); }
	ZL_String(const unsigned long source)  { operator<<(source); }
	ZL_String(const signed long source)    { operator<<(source); }
	ZL_String(const float source)          { operator<<(source); }
	ZL_String(const double source)         { operator<<(source); }

	#if (defined(__cplusplus) && (__cplusplus > 199711 || _MSC_VER >= 1800) && (!defined(__GNUC__) || defined(__clang__) || __GNUC__ > 4))
	inline ZL_String(ZL_String&& source) : str((str&&)source) { }
	inline ZL_String& operator=(const ZL_String& source) { assign(source); return *this; }
	#endif

	ZL_String& operator +=(const ZL_String &source) { append(source); return *this; }
	ZL_String& operator +=(const str &source)       { append(source); return *this; }
	ZL_String& operator +=(const chr *source)       { append(source); return *this; }
	ZL_String& operator +=(const uchr *source)      { append((chr*)source); return *this; }
	ZL_String& operator +=(const schr *source)      { append((chr*)source); return *this; }
	ZL_String& operator +=(const chr source)        { append(1, source); return *this; }
	ZL_String& operator +=(const uchr source)       { append(1, (chr)source); return *this; }
	ZL_String& operator +=(const schr source)       { append(1, (chr)source); return *this; }
	ZL_String& operator +=(const unsigned int source)   { return operator<<(source); }
	ZL_String& operator +=(const signed int source)     { return operator<<(source); }
	ZL_String& operator +=(const unsigned short source) { return operator<<(source); }
	ZL_String& operator +=(const signed short source)   { return operator<<(source); }
	ZL_String& operator +=(const unsigned long source)  { return operator<<(source); }
	ZL_String& operator +=(const signed long source)    { return operator<<(source); }
	ZL_String& operator +=(const double source)         { return operator<<(source); }
	ZL_String& operator +=(const float source)          { return operator<<(source); }
	ZL_String& operator <<(const ZL_String &source) { append(source); return *this; }
	ZL_String& operator <<(const str &source)       { append(source); return *this; }
	ZL_String& operator <<(const chr *source)       { append(source); return *this; }
	ZL_String& operator <<(const uchr *source)      { append((chr*)source); return *this; }
	ZL_String& operator <<(const schr *source)      { append((chr*)source); return *this; }
	ZL_String& operator <<(const chr source)        { append(1, source); return *this; }
	ZL_String& operator <<(const uchr source)       { append(1, (chr)source); return *this; }
	ZL_String& operator <<(const schr source)       { append(1, (chr)source); return *this; }
	ZL_String& operator <<(const unsigned int source);
	ZL_String& operator <<(const signed int source);
	ZL_String& operator <<(const unsigned short source);
	ZL_String& operator <<(const signed short source);
	ZL_String& operator <<(const unsigned long source);
	ZL_String& operator <<(const signed long source);
	ZL_String& operator <<(const double source);
	ZL_String& operator <<(const float source);
	ZL_String operator +(const ZL_String &source) const { ZL_String r(*this); r.append(source); return r; }
	ZL_String operator +(const str &source) const       { ZL_String r(*this); r.append(source); return r; }
	ZL_String operator +(const chr *source) const       { ZL_String r(*this); r.append(source); return r; }
	ZL_String operator +(const uchr *source) const      { ZL_String r(*this); r.append((chr*)source); return r; }
	ZL_String operator +(const schr *source) const      { ZL_String r(*this); r.append((chr*)source); return r; }
	ZL_String operator +(const chr source) const        { ZL_String r(*this); r.append(1, source); return r; }
	ZL_String operator +(const uchr source) const       { ZL_String r(*this); r.append(1, (chr)source); return r; }
	ZL_String operator +(const schr source) const       { ZL_String r(*this); r.append(1, (chr)source); return r; }
	ZL_String operator +(const unsigned int source) const   { return *this + ZL_String(source); }
	ZL_String operator +(const signed int source) const     { return *this + ZL_String(source); }
	ZL_String operator +(const unsigned short source) const { return *this + ZL_String(source); }
	ZL_String operator +(const signed short source) const   { return *this + ZL_String(source); }
	ZL_String operator +(const unsigned long source) const  { return *this + ZL_String(source); }
	ZL_String operator +(const signed long source) const    { return *this + ZL_String(source); }
	ZL_String operator +(const double source) const         { return *this + ZL_String(source); }
	ZL_String operator +(const float source)  const         { return *this + ZL_String(source); }

	void operator *=(int level) { if (level <= 0) erase(); else { size_t len = size(); while (--level) append(c_str(), len); } }

	ZL_String operator *(int level) { if (level <= 0) return ZL_String( ); ZL_String r(*this); while (--level) r.append(*this); return r; }

	const chr * operator*() const { return c_str(); }
	operator const chr *() const { return c_str(); }
	operator bool () const { if (!size()) return false; char c = this[0]; if (c=='y'||c=='Y'||c=='t'||c=='T'||c=='1') return true; if (c=='n'||c=='N'||c=='f'||c=='F') return false; return atoi(c_str()) != 0; }

	operator chr () const { return (chr)atoi(c_str()); }
	operator uchr () const { return (uchr)atoi(c_str()); }
	operator schr () const { return (schr)atoi(c_str()); }
	operator unsigned int () const { return (unsigned int)atoi(c_str()); }
	operator signed int () const { return (signed int)atoi(c_str()); }
	operator double () const { return atof(c_str()); }
	operator float () const { return (float)atof(c_str()); }

	//Older Visual C++ aren't able to handle 64bit numbers being put into std::ostringstream
	#if (!defined(_MSC_VER) || _MSC_VER > 1200)
	ZL_String(const u64 source)                  { operator<<(source); }
	ZL_String(const i64 source)                  { operator<<(source); }
	ZL_String& operator +=(const u64 source)     { return operator<<(source); }
	ZL_String& operator +=(const i64 source)     { return operator<<(source); }
	ZL_String& operator <<(const u64 source);
	ZL_String& operator <<(const i64 source);
	ZL_String operator +(const u64 source) const { return *this + ZL_String(source); }
	ZL_String operator +(const i64 source) const { return *this + ZL_String(source); }
	#endif
};

#endif //__ZL_STRING__
