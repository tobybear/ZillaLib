/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2014 Sam Lantinga <slouken@libsdl.org>

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

/**
 *  \file SDL_stdinc.h
 *
 *  This is a general header that includes C language support.
 */

#ifndef _SDL_stdinc_h_ZL
#define _SDL_stdinc_h_ZL

#include "SDL_config.h"

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#if defined(STDC_HEADERS)
# include <stdlib.h>
# include <stddef.h>
# include <stdarg.h>
#else
# if defined(HAVE_STDLIB_H)
#  include <stdlib.h>
# elif defined(HAVE_MALLOC_H)
#  include <malloc.h>
# endif
# if defined(HAVE_STDDEF_H)
#  include <stddef.h>
# endif
# if defined(HAVE_STDARG_H)
#  include <stdarg.h>
# endif
#endif
#ifdef HAVE_STRING_H
# if !defined(STDC_HEADERS) && defined(HAVE_MEMORY_H)
#  include <memory.h>
# endif
# include <string.h>
#endif
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif
#if defined(HAVE_INTTYPES_H)
# include <inttypes.h>
#elif defined(HAVE_STDINT_H)
# include <stdint.h>
#endif
#ifdef HAVE_CTYPE_H
# include <ctype.h>
#endif
#ifdef HAVE_MATH_H
# if defined(__WINRT__)
/* Defining _USE_MATH_DEFINES is required to get M_PI to be defined on
   WinRT.  See http://msdn.microsoft.com/en-us/library/4hwaceh6.aspx
   for more information.
*/
#  define _USE_MATH_DEFINES
# endif
# include <math.h>
#endif
#ifdef HAVE_FLOAT_H
# include <float.h>
#endif
#if defined(HAVE_ICONV) && defined(HAVE_ICONV_H)
# include <iconv.h>
#endif

/**
 *  The number of elements in an array.
 */
#define SDL_arraysize(array)    (sizeof(array)/sizeof(array[0]))
#define SDL_TABLESIZE(table)    SDL_arraysize(table)

/**
 *  \name Cast operators
 *
 *  Use proper C++ casts when compiled as C++ to be compatible with the option
 *  -Wold-style-cast of GCC (and -Werror=old-style-cast in GCC 4.2 and above).
 */
/* @{ */
#ifdef __cplusplus
#define SDL_reinterpret_cast(type, expression) reinterpret_cast<type>(expression)
#define SDL_static_cast(type, expression) static_cast<type>(expression)
#define SDL_const_cast(type, expression) const_cast<type>(expression)
#else
#define SDL_reinterpret_cast(type, expression) ((type)(expression))
#define SDL_static_cast(type, expression) ((type)(expression))
#define SDL_const_cast(type, expression) ((type)(expression))
#endif
/* @} *//* Cast operators */

/* Define a four character code as a Uint32 */
#define SDL_FOURCC(A, B, C, D) \
    ((SDL_static_cast(Uint32, SDL_static_cast(Uint8, (A))) << 0) | \
     (SDL_static_cast(Uint32, SDL_static_cast(Uint8, (B))) << 8) | \
     (SDL_static_cast(Uint32, SDL_static_cast(Uint8, (C))) << 16) | \
     (SDL_static_cast(Uint32, SDL_static_cast(Uint8, (D))) << 24))

/**
 *  \name Basic data types
 */
/* @{ */

typedef enum
{
    SDL_FALSE = 0,
    SDL_TRUE = 1
} SDL_bool;

/**
 * \brief A signed 8-bit integer type.
 */
typedef int8_t Sint8;
/**
 * \brief An unsigned 8-bit integer type.
 */
typedef uint8_t Uint8;
/**
 * \brief A signed 16-bit integer type.
 */
typedef int16_t Sint16;
/**
 * \brief An unsigned 16-bit integer type.
 */
typedef uint16_t Uint16;
/**
 * \brief A signed 32-bit integer type.
 */
typedef int32_t Sint32;
/**
 * \brief An unsigned 32-bit integer type.
 */
typedef uint32_t Uint32;

/**
 * \brief A signed 64-bit integer type.
 */
typedef int64_t Sint64;
/**
 * \brief An unsigned 64-bit integer type.
 */
typedef uint64_t Uint64;

/* @} *//* Basic data types */


#define SDL_COMPILE_TIME_ASSERT(name, x)               \
       typedef int SDL_dummy_ ## name[(x) * 2 - 1]
/** \cond */
#ifndef DOXYGEN_SHOULD_IGNORE_THIS
SDL_COMPILE_TIME_ASSERT(uint8, sizeof(Uint8) == 1);
SDL_COMPILE_TIME_ASSERT(sint8, sizeof(Sint8) == 1);
SDL_COMPILE_TIME_ASSERT(uint16, sizeof(Uint16) == 2);
SDL_COMPILE_TIME_ASSERT(sint16, sizeof(Sint16) == 2);
SDL_COMPILE_TIME_ASSERT(uint32, sizeof(Uint32) == 4);
SDL_COMPILE_TIME_ASSERT(sint32, sizeof(Sint32) == 4);
SDL_COMPILE_TIME_ASSERT(uint64, sizeof(Uint64) == 8);
SDL_COMPILE_TIME_ASSERT(sint64, sizeof(Sint64) == 8);
#endif /* DOXYGEN_SHOULD_IGNORE_THIS */
/** \endcond */

/* Check to make sure enums are the size of ints, for structure packing.
   For both Watcom C/C++ and Borland C/C++ the compiler option that makes
   enums having the size of an int must be enabled.
   This is "-b" for Borland C/C++ and "-ei" for Watcom C/C++ (v11).
*/

/** \cond */
#ifndef DOXYGEN_SHOULD_IGNORE_THIS
#if !defined(__ANDROID__)
   /* TODO: include/SDL_stdinc.h:174: error: size of array 'SDL_dummy_enum' is negative */
typedef enum
{
    DUMMY_ENUM_VALUE
} SDL_DUMMY_ENUM;

SDL_COMPILE_TIME_ASSERT(enum, sizeof(SDL_DUMMY_ENUM) == sizeof(int));
#endif
#endif /* DOXYGEN_SHOULD_IGNORE_THIS */
/** \endcond */

#include "begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

#if defined(HAVE_ALLOCA) && !defined(alloca)
# if defined(HAVE_ALLOCA_H)
#  include <alloca.h>
# elif defined(__GNUC__)
#  define alloca __builtin_alloca
# elif defined(_MSC_VER)
#  include <malloc.h>
#  define alloca _alloca
# elif defined(__WATCOMC__)
#  include <malloc.h>
# elif defined(__BORLANDC__)
#  include <malloc.h>
# elif defined(__DMC__)
#  include <stdlib.h>
# elif defined(__AIX__)
#pragma alloca
# elif defined(__MRC__)
void *alloca(unsigned);
# else
char *alloca();
# endif
#endif
#ifdef HAVE_ALLOCA
#define SDL_stack_alloc(type, count)    (type*)alloca(sizeof(type)*(count))
#define SDL_stack_free(data)
#else
#define SDL_stack_alloc(type, count)    (type*)malloc(sizeof(type)*(count))
#define SDL_stack_free(data)            free(data)
#endif

//extern DECLSPEC void *SDLCALL SDL_malloc(size_t size);
//extern DECLSPEC void *SDLCALL SDL_calloc(size_t nmemb, size_t size);
//extern DECLSPEC void *SDLCALL SDL_realloc(void *mem, size_t size);
//extern DECLSPEC void SDLCALL SDL_free(void *mem);

//extern DECLSPEC char *SDLCALL SDL_getenv(const char *name);
//extern DECLSPEC int SDLCALL SDL_setenv(const char *name, const char *value, int overwrite);

//extern DECLSPEC void SDLCALL SDL_qsort(void *base, size_t nmemb, size_t size, int (*compare) (const void *, const void *));

//extern DECLSPEC int SDLCALL SDL_abs(int x);

/* !!! FIXME: these have side effects. You probably shouldn't use them. */
/* !!! FIXME: Maybe we do forceinline functions of SDL_mini, SDL_minf, etc? */
#define SDL_min(x, y) (((x) < (y)) ? (x) : (y))
#define SDL_max(x, y) (((x) > (y)) ? (x) : (y))

#define SDL_isdigit isdigit
#define SDL_isspace isspace
#define SDL_toupper toupper
#define SDL_tolower tolower
#define SDL_memset memset
#define SDL_zero(x) memset(&(x), 0, sizeof((x)))
#define SDL_zerop(x) memset((x), 0, sizeof(*(x)))
#define SDL_memset4(dst, val, dwords) memset((dst), (val), (dwords)*4)
#define SDL_memcpy memcpy
#define SDL_memcpy4(dst, src, dwords) memcpy((dst), (src), (dwords)*4)
#define SDL_memmove memmove
#define SDL_memcmp memcmp
//extern DECLSPEC size_t SDLCALL SDL_wcslen(const wchar_t *wstr);
//extern DECLSPEC size_t SDLCALL SDL_wcslcpy(wchar_t *dst, const wchar_t *src, size_t maxlen);
//extern DECLSPEC size_t SDLCALL SDL_wcslcat(wchar_t *dst, const wchar_t *src, size_t maxlen);
#define SDL_strlen strlen
#define SDL_strlcpy(dst,src,maxlen) strncpy(dst,src,maxlen-1) //extern DECLSPEC size_t SDLCALL SDL_strlcpy(char *dst, const char *src, size_t maxlen);
#define SDL_utf8strlcpy(dst,src,dst_bytes) strncpy(dst,src,dst_bytes-1) //extern DECLSPEC size_t SDLCALL SDL_utf8strlcpy(char *dst, const char *src, size_t dst_bytes);
#define SDL_strlcat(dst,src,maxlen) strcat(dst,src) //extern DECLSPEC size_t SDLCALL SDL_strlcat(char *dst, const char *src, size_t maxlen);
#define SDL_strdup strdup
//extern DECLSPEC char *SDLCALL SDL_strrev(char *str);
//extern DECLSPEC char *SDLCALL SDL_strupr(char *str);
//extern DECLSPEC char *SDLCALL SDL_strlwr(char *str);
#define SDL_strchr strchr
#define SDL_strrchr strrchr
//extern DECLSPEC char *SDLCALL SDL_strrchr(const char *str, int c);
#define SDL_strstr strstr

#define SDL_itoa(i, s, rdx) sprintf(s, "%i", (int)i); { typedef int _chk[(rdx==10)?1:-1]; }
//#define SDL_itoa itoa
//extern DECLSPEC char *SDLCALL SDL_uitoa(unsigned int value, char *str, int radix);
//extern DECLSPEC char *SDLCALL SDL_ltoa(long value, char *str, int radix);
//extern DECLSPEC char *SDLCALL SDL_ultoa(unsigned long value, char *str, int radix);
//extern DECLSPEC char *SDLCALL SDL_lltoa(Sint64 value, char *str, int radix);
//extern DECLSPEC char *SDLCALL SDL_ulltoa(Uint64 value, char *str, int radix);

#define SDL_atoi atoi
#define SDL_atof atof
#define SDL_strtol strtol
#define SDL_strtoul strtoul
//extern DECLSPEC Sint64 SDLCALL SDL_strtoll(const char *str, char **endp, int base);
//extern DECLSPEC Uint64 SDLCALL SDL_strtoull(const char *str, char **endp, int base);
#define SDL_strtod strtod

#define SDL_strcmp strcmp
#define SDL_strncmp strncmp
#define SDL_strcasecmp strcmp //extern DECLSPEC int SDLCALL SDL_strcasecmp(const char *str1, const char *str2);
#define SDL_strncasecmp strncmp //extern DECLSPEC int SDLCALL SDL_strncasecmp(const char *str1, const char *str2, size_t len);


static int SDL_snprintf(char *text, size_t maxlen, const char *fmt, ...) { va_list ap; va_start(ap, fmt); return vsprintf(text, fmt, ap); } 
#define SDL_sscanf(t,f,v) NULL
//extern DECLSPEC int SDLCALL SDL_sscanf(const char *text, const char *fmt, ...);
//extern DECLSPEC int SDLCALL SDL_vsscanf(const char *text, const char *fmt, va_list ap);
//extern DECLSPEC int SDLCALL SDL_snprintf(char *text, size_t maxlen, const char *fmt, ...);
//extern DECLSPEC int SDLCALL SDL_vsnprintf(char *text, size_t maxlen, const char *fmt, va_list ap);

#ifndef HAVE_M_PI
#ifndef M_PI
#define M_PI    3.14159265358979323846264338327950288   /* pi */
#endif
#endif

#define SDL_ceil ceil
/*
extern DECLSPEC double SDLCALL SDL_acos(double x);
extern DECLSPEC double SDLCALL SDL_asin(double x);
extern DECLSPEC double SDLCALL SDL_atan(double x);
extern DECLSPEC double SDLCALL SDL_atan2(double x, double y);
extern DECLSPEC double SDLCALL SDL_ceil(double x);
extern DECLSPEC double SDLCALL SDL_copysign(double x, double y);
extern DECLSPEC double SDLCALL SDL_cos(double x);
extern DECLSPEC float SDLCALL SDL_cosf(float x);
extern DECLSPEC double SDLCALL SDL_fabs(double x);
extern DECLSPEC double SDLCALL SDL_floor(double x);
extern DECLSPEC double SDLCALL SDL_log(double x);
extern DECLSPEC double SDLCALL SDL_pow(double x, double y);
extern DECLSPEC double SDLCALL SDL_scalbn(double x, int n);
extern DECLSPEC double SDLCALL SDL_sin(double x);
extern DECLSPEC float SDLCALL SDL_sinf(float x);
extern DECLSPEC double SDLCALL SDL_sqrt(double x);
*/

/*
#define SDL_ICONV_ERROR     (size_t)-1
#define SDL_ICONV_E2BIG     (size_t)-2
#define SDL_ICONV_EILSEQ    (size_t)-3
#define SDL_ICONV_EINVAL    (size_t)-4
typedef struct _SDL_iconv_t *SDL_iconv_t;
extern DECLSPEC SDL_iconv_t SDLCALL SDL_iconv_open(const char *tocode,
                                                   const char *fromcode);
extern DECLSPEC int SDLCALL SDL_iconv_close(SDL_iconv_t cd);
extern DECLSPEC size_t SDLCALL SDL_iconv(SDL_iconv_t cd, const char **inbuf,
                                         size_t * inbytesleft, char **outbuf,
                                         size_t * outbytesleft);
extern DECLSPEC char *SDLCALL SDL_iconv_string(const char *tocode,
                                               const char *fromcode,
                                               const char *inbuf,
                                               size_t inbytesleft);
#define SDL_iconv_utf8_locale(S)    SDL_iconv_string("", "UTF-8", S, SDL_strlen(S)+1)
#define SDL_iconv_utf8_ucs2(S)      (Uint16 *)SDL_iconv_string("UCS-2-INTERNAL", "UTF-8", S, SDL_strlen(S)+1)
#define SDL_iconv_utf8_ucs4(S)      (Uint32 *)SDL_iconv_string("UCS-4-INTERNAL", "UTF-8", S, SDL_strlen(S)+1)
*/

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include "close_code.h"

#endif /* _SDL_stdinc_h_ZL */

/* vi: set ts=4 sw=4 expandtab: */
