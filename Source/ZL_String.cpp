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

#include "ZL_Platform.h"
#include "ZL_String.h"
#include <assert.h>
#include <stdio.h>
#if defined(_MSC_VER) && !defined(vsnprintf)
#define vsnprintf _vsnprintf
#endif

ZL_String ZL_String::EmptyString = ZL_String();

ZL_String ZL_String::vformat(const char *format, va_list ap)
{
	size_t size = 1024;
	char stackbuf[1024];
	std::vector<char> dynamicbuf;
	char *buf = &stackbuf[0];
	while (1)
	{
		int needed = vsnprintf(buf, size, format, ap);
		if (needed <= (int)size && needed >= 0) return ZL_String(buf, (size_t) needed);
		size = (needed > 0) ? (needed+1) : (size*2);
		dynamicbuf.resize(size);
		buf = &dynamicbuf[0];
	}
	return ZL_String();
}

ZL_String ZL_String::format(const char *format, ...)
{
	size_t size = 1024;
	char stackbuf[1024], *buf = stackbuf;
	ZL_String dynamicbuf;
	for (va_list ap;;)
	{
		va_start(ap, format); int needed = vsnprintf(buf, size, format, ap); va_end(ap);
		if (needed >= 0 && needed <= (int)size) return (buf == stackbuf ? ZL_String(buf, needed) : dynamicbuf);
		size = (needed > 0) ? (needed+1) : (size*2);
		dynamicbuf.resize(size);
		buf = &dynamicbuf.front();
	}
}

ZL_String& ZL_String::to_upper()
{
	for (iterator it = begin(); it != end(); ++it)
		*it = toupper(*it);
	return *this;
}

ZL_String& ZL_String::to_lower()
{
	for (iterator it = begin(); it != end(); ++it)
		*it = tolower(*it);
	return *this;
}

ZL_String& ZL_String::str_replace(const ZL_String &from, const ZL_String &to)
{
	size_t fromlen = from.size(), tolen = to.size();
	for (size_type f = find(from); f != npos; f = find(from, f + tolen))
		replace(f, fromlen, to);
	return *this;
}

std::vector<ZL_String> ZL_String::split(const ZL_String &delimiter) const
{
	assert(delimiter.size());
	std::vector<ZL_String> splits;
	for (size_t start = 0, end = 0; end != std::string::npos;)
	{
		end = find(delimiter, start);
		splits.push_back(substr(start, (end == std::string::npos ? std::string::npos : end - start)));
		start = ((end > (std::string::npos - delimiter.size())) ? std::string::npos : end + delimiter.size());
	}
	return splits;
}
