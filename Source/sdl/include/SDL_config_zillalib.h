#ifdef DECLSPEC
#undef DECLSPEC
#endif
#define DECLSPEC //disable function exporting

#ifndef __ZL_PLATFORM_SDL__

#if (defined(_MSC_VER) && !defined(WINAPI_FAMILY))
#pragma warning(disable:4002) //too many actual parameters for macro
#pragma warning(disable:4003) //not enough actual parameters for macro
#pragma warning(disable:4018) //signed/unsigned mismatch
#pragma warning(disable:4244) //'function' : conversion from 'WPARAM' to 'UINT', possible loss of data      
#pragma once
#endif

#define SDL_INIT_AUDIO          0x00000010
#define SDL_INIT_VIDEO          0x00000020
#define SDL_INIT_EVENTS         0x00004000
#define SDL_QuitSubSystem(s) NULL //no stopping of subsystems in ZL

#define HAVE_ITOA 1
//#undef SDL_AUDIO_DRIVER_XAUDIO2 /* use */
//#undef SDL_AUDIO_DRIVER_DSOUND /* fallback */
#undef SDL_AUDIO_DRIVER_WINMM
#undef SDL_AUDIO_DRIVER_DUMMY
#undef SDL_AUDIO_DRIVER_DISK
#undef SDL_VIDEO_DRIVER_DUMMY
#undef SDL_VIDEO_RENDER_OGL_ES2
#undef SDL_VIDEO_OPENGL_ES2
#undef SDL_VIDEO_OPENGL_EGL
#undef SDL_FILESYSTEM_WINDOWS
#undef SDL_FILESYSTEM_COCOA
#undef SDL_FILESYSTEM_DUMMY
#undef SDL_FILESYSTEM_UNIX
#undef SDL_HAPTIC_DINPUT
#undef SDL_JOYSTICK_WINMM
#undef SDL_HAPTIC_IOKIT
#define SDL_DISABLE_WINDOWS_IME 1
#define SDL_DISABLE_SCANCODENAMES 1

#define _SDL_H //not required
#define _SDL_error_h //not required
#define _SDL_rwops_h //not required
#define _SDL_hints_h //not required
#define _SDL_assert_h //not required
#define _SDL_main_h //not required
#define _SDL_render_h //not required
#define _SDL_gesture_c_h //not required
#define _SDL_stdinc_h //set so compile fails without our customized SDL_stdinc.h

#if (defined(_MSC_VER) && !defined(WINAPI_FAMILY))
#define SDL_SetError(errstr) -1
#else
#define SDL_SetError(errstr,...) -1
#endif
//#define SDL_SetError printf
#define SDL_assert(a)
#define SDL_Error(e)
#define SDL_ASSERT_LEVEL 0
#define SDL_ClearError()
#define SDL_OutOfMemory() 1
#define SDL_Unsupported() 1
#define SDL_InvalidParamError(p) 1
#define SDL_arraysize(array) (sizeof(array)/sizeof(array[0]))
#define SDL_RWops void
#define SDL_RWFromMem(b,l) NULL
#define SDL_RWread(ctx, ptr, size, n) 0
#define SDL_RWwrite(ctx, ptr, size, n) 0
#define SDL_RWclose(ctx) 0
#define SDL_WriteLE16(a, b) 0
#define SDL_WriteLE32(a, b) 0

#define SDL_GestureAddTouch(t) NULL
#define SDL_GestureProcessEvent(e) NULL

#define SDL_GetHint(n) NULL
#define SDL_AddHintCallback(n, f, x) NULL
#define SDL_DelHintCallback(n, f, x) NULL
#undef HAVE_M_PI
#include <stdlib.h>
#define HAVE_GETENV 1
#undef getenv
#define getenv(env) NULL
#define HAVE_STRDUP 1
#define HAVE_SNPRINTF 1
#include <stdio.h>
#ifndef snprintf
#define snprintf _snprintf
#endif

#define SDL_Renderer void
#define SDL_Texture void
typedef struct SDL_RendererInfo { const char* name; int num_texture_formats; int texture_formats[1]; } SDL_RendererInfo;
#define SDL_GetNumRenderDrivers() 0
#define SDL_GetRenderDriverInfo(i, info) { (info)->name = NULL; (info)->num_texture_formats = 0; }
#define SDL_CreateRenderer(a,b,c) NULL
#define SDL_DestroyRenderer(a)
#define SDL_DestroyTexture(a)
#define SDL_GetRendererInfo(a, b) (memset(&(a), 0, sizeof(a)),NULL)
#define SDL_TEXTUREACCESS_STREAMING 0
#define SDL_RenderSetViewport(a,b)
#define SDL_UpdateTexture(a,b,c,d) 0
#define SDL_RenderCopy(a,b,c,d) 0
#define SDL_RenderPresent(a)
#define SDL_CreateTexture(a, b, c, d, e) NULL

#define SDL_malloc(s) malloc(s)
#define SDL_calloc(n,s) calloc(n,s)
#define SDL_realloc(m,s) realloc(m,s)
#define SDL_free(m) free(m)
#define SDL_getenv(e) NULL
#define SDL_qsort(a,b,c,d) NULL
#define SDL_abs(v) abs(v)

#define SDL_iconv_utf8_locale(S) strdup(S)
#define SDL_iconv_string(t,f,S,l) strdup(S)

#ifdef __WIN32__
#define _INCLUDED_WINDOWS_H //not required
#define WIN32_LEAN_AND_MEAN
#define STRICT 1
#define UNICODE 1
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x501
#include <windows.h>
#undef CreateWindow
#ifdef __WINRT__
#define WIN_CoInitialize() S_OK
#define WIN_CoUninitialize() NULL
#else 
#define WIN_CoInitialize() S_OK;{HRESULT hr=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(hr==RPC_E_CHANGED_MODE)hr=CoInitializeEx(NULL,COINIT_MULTITHREADED);}
#define WIN_CoUninitialize() {CoUninitialize();}
#endif

#define WIN_SetError(e) -1
#define CreateWindow(a, b, c, d, e, f, g, h, i, j, k) CreateWindowW(a, b, c, d, e, f, g, h, i, j, k)
#endif

#include "SDL_stdinc.h"

//stubs for unused SDL stuff to reduce binary size without altering the SDL source
#ifdef __cplusplus
extern "C" {
#endif
extern DECLSPEC void SDLCALL SDL_Delay(Uint32 ms);
extern DECLSPEC Uint32 SDLCALL SDL_WasInit(Uint32 flags);
extern DECLSPEC int SDLCALL SDL_InitSubSystem(Uint32 flags);
#ifdef __WIN32__
extern DECLSPEC int SDLCALL SDL_RegisterApp(char *name, Uint32 style, void *hInst);
extern DECLSPEC void SDLCALL SDL_UnregisterApp(void);
char* WIN_StringToUTF8(const unsigned short* s);
unsigned short* WIN_UTF8ToString(const char* s);
#endif
#define SDL_TLSCreate() 0
#if defined(__LP64__) || defined(_LP64) || defined(__LLP64__) || defined(__x86_64__) || defined(__ia64__) || defined(_WIN64) || defined(_M_X64)
unsigned int SDL_TLSSet64(void*);
void* SDL_TLSGet64(unsigned int);
#define SDL_TLSSet(tls, v, cb) tls = SDL_TLSSet64(v)
#define SDL_TLSGet(tls) SDL_TLSGet64(tls)
#else
#define SDL_TLSSet(tls, v, cb) tls = (SDL_TLSID)(v)
#define SDL_TLSGet(tls) (tls)
#endif
#ifdef __cplusplus
}
#endif

#elif defined(ZL_SDL_DO_OVERRIDE_IMPLEMENTATIONS)

extern "C" {
int SDL_ConvertAudio(SDL_AudioCVT* c) { return 0; }
int SDL_BuildAudioCVT(SDL_AudioCVT* c,SDL_AudioFormat sf, Uint8 sc, int sr, SDL_AudioFormat df, Uint8 dc, int dr) { return 0; }
void SDL_MixAudioFormat(Uint8 * dst, const Uint8 * src, SDL_AudioFormat format, Uint32 len, int volume) { }
void SDL_FreeSurface(SDL_Surface * surface) { }
SDL_Surface* SDL_CreateRGBSurfaceFrom(void*, int, int, int, int, Uint32, Uint32, Uint32, Uint32) { return NULL; }
SDL_Surface* SDL_CreateRGBSurface(Uint32, int, int, int, Uint32, Uint32, Uint32, Uint32) { return NULL; }
SDL_bool SDL_PixelFormatEnumToMasks(Uint32, int*, Uint32*, Uint32*, Uint32*, Uint32*) { return SDL_FALSE; }
Uint32 SDL_MasksToPixelFormatEnum(int, Uint32, Uint32, Uint32, Uint32) { return 0; }
void SDL_CalculateGammaRamp(float, Uint16 *) { }
SDL_Surface* SDL_ConvertSurfaceFormat(SDL_Surface *, Uint32, Uint32) { return NULL; }
SDL_bool SDL_IsShapedWindow(const SDL_Window *window) { return SDL_FALSE; }
static char SDL_Audio_Init_Done = 0;
Uint32 SDL_WasInit(Uint32 flags) { return (flags & /*SDL_INIT_AUDIO*/16 ? SDL_Audio_Init_Done : 0); }
int SDL_InitSubSystem(Uint32 flags) { return (flags & /*SDL_INIT_AUDIO*/16 ? (SDL_Audio_Init_Done = 1) : 0); }
#ifdef __WIN32__
char* WIN_StringToUTF8(const unsigned short* s) { int l = WideCharToMultiByte(CP_UTF8,0,(LPCWSTR)s,-1,0,0,0,0); char* b = (char*)malloc(l); WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)s, -1, b, l, NULL, NULL); return b; }
unsigned short* WIN_UTF8ToString(const char* s) { int l = MultiByteToWideChar(CP_UTF8,0,s,-1,NULL,0); unsigned short* b = (unsigned short*)malloc(l*sizeof(short)); MultiByteToWideChar(CP_UTF8,0,s,-1,(LPWSTR)b,l); return b; }
SDL_WindowShaper* Win32_CreateShaper(SDL_Window * window) { return NULL; }
int Win32_SetWindowShape(SDL_WindowShaper *shaper,SDL_Surface *shape,SDL_WindowShapeMode *shape_mode) { return 0; }
int Win32_ResizeWindowShape(SDL_Window *window) { return 0; }
#else
SDL_WindowShaper* X11_CreateShaper(int) { return NULL; }
int X11_ResizeWindowShape(int) { return 0; }
int X11_SetWindowShape(int, int, int) { return 0; }
SDL_WindowShaper* Cocoa_CreateShaper(int) { return NULL; }
int Cocoa_SetWindowShape(int,int,int) { return 0; }
int Cocoa_ResizeWindowShape(int) { return 0; }
#endif
#if defined(__LP64__) || defined(_LP64) || defined(__LLP64__) || defined(__x86_64__) || defined(__ia64__) || defined(_WIN64) || defined(_M_X64)
static void* ZLFakeTLSData[2];
unsigned int SDL_TLSSet64(void* v) { static int ZLFakeTLSCount; ZLFakeTLSData[ZLFakeTLSCount] = v; return ZLFakeTLSCount++; }
void* SDL_TLSGet64(unsigned int i) { return ZLFakeTLSData[i]; }
#endif
}

#endif
