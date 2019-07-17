/*
  ZillaLib
  Copyright (C) 2010-2019 Bernhard Schelling

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

#ifdef __native_client__
#include "ZL_Platform.h"
#include <ZL_Math.h>
#include <ZL_Application.h>
#include <ZL_File.h>
#include "ZL_Impl.h"
#include "ZL_Signal.h"
#include "ZL_Display.h"
#include "ZL_Display_Impl.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/nacl_syscalls.h>
#include <stdarg.h>
#include <cassert>
#include <cmath>
#include <limits>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <map>

#include "ppapi/c/pp_errors.h"
#include "ppapi/c/pp_module.h"
#include "ppapi/c/pp_graphics_3d.h"

#include "ppapi/c/ppp.h"
#include "ppapi/c/ppp_instance.h"
#include "ppapi/c/ppp_messaging.h"
#include <ppapi/c/ppp_input_event.h>
#include <ppapi/c/ppp_mouse_lock.h>

#include "ppapi/c/ppb.h"
#include "ppapi/c/ppb_instance.h"
#include "ppapi/c/ppb_messaging.h"
#include "ppapi/c/ppb_var.h"
#include "ppapi/c/ppb_var_array_buffer.h"
#include "ppapi/c/ppb_core.h"
#include "ppapi/c/ppb_audio.h"
#include "ppapi/c/ppb_audio_config.h"
#include "ppapi/c/ppb_graphics_3d.h"
#include "ppapi/c/ppb_view.h"
#include <ppapi/c/ppb_url_loader.h>
#include <ppapi/c/ppb_url_request_info.h>
#include <ppapi/c/ppb_url_response_info.h>
#include <ppapi/c/ppb_input_event.h>
#include <ppapi/c/ppb_fullscreen.h>
#include <ppapi/c/ppb_mouse_lock.h>
#include <ppapi/c/ppb_websocket.h>

#include <GLES2/gl2.h>
#include <ppapi/gles2/gl2ext_ppapi.h>

static PPB_Core*               ppb_core_interface = NULL;
static PPB_Messaging*          ppb_messaging_interface = NULL;
static PPB_Audio*              ppb_audio_interface = NULL;
static PPB_AudioConfig*        ppb_audioconfig_interface = NULL;
static PPB_Var*                ppb_var_interface = NULL;
static PPB_Instance*           ppb_instance_interface = NULL;
static PPB_Graphics3D*         ppb_graphics3d_interface = NULL;
static PPB_View*               ppb_view_interface = NULL;
static PPB_URLLoader*          ppb_urlloader_interface = NULL;
static PPB_URLRequestInfo*     ppb_urlrequestinfo_interface = NULL;
static PPB_URLResponseInfo*    ppb_urlresponseinfo_interface = NULL;
static PPB_InputEvent*         ppb_inputevent_interface = NULL;
static PPB_MouseInputEvent*    ppb_mouseinputevent_interface = NULL;
static PPB_WheelInputEvent*    ppb_wheelinputevent_interface = NULL;
static PPB_KeyboardInputEvent* ppb_keyboardinputevent_interface = NULL;
static PPB_Fullscreen*         ppb_fullscreen_interface = NULL;
static PPB_MouseLock*          ppb_mouselock_interface = NULL;
static PPB_WebSocket*          ppb_websocket_interface = NULL;
static PPB_VarArrayBuffer*     ppb_vararraybuffer_interface = NULL;

static PP_Resource context3d = 0;
static bool gl_tpf_flush_pending = false;
static PP_Resource pp_audio = NULL;
static PP_Instance instance_ = NULL;
static bool has_focus_ = false, inited_ = false, running_ = false, rdysent_ = false;
static PP_Rect recView;
static int NACL_Width = -1, NACL_Height = -1;
static unsigned int NACL_WindowFlags = ZL_WINDOW_INPUT_FOCUS | ZL_WINDOW_MOUSE_FOCUS;
static std::map<ZL_String, ZL_String> settings;
static double nacl_tstart;
static std::vector<char> vecUrlLoaderBuf;
#define URLLOAD_BUFSIZE 1024*16

static void ZL_WindowEvent(unsigned char event, int data1 = 0, int data2 = 0);

static struct PP_Var ZLStrToVar(const ZL_String& str) { return ppb_var_interface->VarFromUtf8(str, str.length()); }

void swapbuffer_tpf_callback(void*, int32_t)
{
	gl_tpf_flush_pending = false;
}

void update_tpf_callback(void*, int32_t)
{
	PP_CompletionCallback cbu = { &update_tpf_callback, 0, 0 };
	ppb_core_interface->CallOnMainThread((int)ZL_TPF_Limit, cbu, 0);
	if (gl_tpf_flush_pending) return;
	ZL_MainApplication->Frame();
	gl_tpf_flush_pending = true;
	const PP_CompletionCallback cbf = { &swapbuffer_tpf_callback, NULL, 0 };
	ppb_graphics3d_interface->SwapBuffers(context3d, cbf);
}

void update_asap_callback(void*, int32_t)
{
	if (!has_focus_) ZL_Delay(1000/60);
	ZL_MainApplication->Frame();
	const PP_CompletionCallback cbf = { &update_asap_callback, NULL, 0 };
	ppb_graphics3d_interface->SwapBuffers(context3d, cbf);
}

void PacketLoader_Read_Callback(void* urlloader, int32_t result)
{
	int64_t bytes_received, total_bytes_to_be_received;
	ppb_urlloader_interface->GetDownloadProgress((PP_Resource)urlloader, &bytes_received, &total_bytes_to_be_received);
	if (total_bytes_to_be_received > 0 && vecUrlLoaderBuf.capacity() < (size_t)total_bytes_to_be_received) vecUrlLoaderBuf.reserve((size_t)total_bytes_to_be_received);

	if (result != URLLOAD_BUFSIZE) vecUrlLoaderBuf.resize(vecUrlLoaderBuf.size() - URLLOAD_BUFSIZE + (result > 0 ? result : 0));

	//ZL_LOG("PKGLOAD", "PacketLoader_Read_Callback - read: %d - bufsize = %d - total_bytes_to_be_received: %d - Total Bytes So Far: %d - bytes_received: %d", (int)result, (int)vecUrlLoaderBuf.size(), (int)total_bytes_to_be_received, vecUrlLoaderBuf.size(), (int)bytes_received);

	if (result > 0 && (total_bytes_to_be_received < 0 || (vecUrlLoaderBuf.size() < total_bytes_to_be_received)))
	{
		if (total_bytes_to_be_received > 0)
		{
			//PKP = Package Progress
			char msgmsg[1024];
			uint32_t msgmsglen = sprintf(msgmsg, "PKP%d/%d", (int)bytes_received, (int)total_bytes_to_be_received);
			if (ppb_messaging_interface) ppb_messaging_interface->PostMessage(instance_, ppb_var_interface->VarFromUtf8(msgmsg, msgmsglen));
		}

		vecUrlLoaderBuf.resize(vecUrlLoaderBuf.size() + URLLOAD_BUFSIZE);
		const PP_CompletionCallback cc = { &PacketLoader_Read_Callback, urlloader, 0 };
		ppb_urlloader_interface->ReadResponseBody((PP_Resource)urlloader, &vecUrlLoaderBuf[vecUrlLoaderBuf.size() - URLLOAD_BUFSIZE], URLLOAD_BUFSIZE, cc);
		return;
	}

	int32_t status = ppb_urlresponseinfo_interface->GetProperty(ppb_urlloader_interface->GetResponseInfo((PP_Resource)urlloader), PP_URLRESPONSEPROPERTY_STATUSCODE).value.as_int;
	ppb_urlloader_interface->Close((PP_Resource)urlloader);

	if (status == 200 && vecUrlLoaderBuf.size() > 20)
	{
		#ifdef ZL_NACL_URL_LOADER_CAN_STILL_BE_GZ_COMPRESSED //this should not be required with recent Chrome/NACL/PNACL/Pepper versions
		if (vecUrlLoaderBuf[0] == 0x1F && (unsigned char)vecUrlLoaderBuf[1] == 0x8B && vecUrlLoaderBuf[2] == 8)
		{
			//ZL_LOG("PEXEGZ", "Found GZ header, decompressing ...");
			std::vector<char> uncompr;
			uncompr.clear();

			unsigned char *gzstart = (unsigned char*)&vecUrlLoaderBuf[0], *zstart = gzstart + 10; //after gz header
			if (vecUrlLoaderBuf[3] & 0x4) zstart += 2; //skip 2 bytes extra header
			if (vecUrlLoaderBuf[3] & 0x8) while (*(zstart++) != '\0'); //skip to end of zero terminated file name string
			if (vecUrlLoaderBuf[3] & 0x10) while (*(zstart++) != '\0'); //skip to end of zero terminated comment string
			if (vecUrlLoaderBuf[3] & 0x2) zstart += 2; //skip 2 bytes crc

			z_stream dec_stream;
			memset(&dec_stream, 0, sizeof(dec_stream));
			dec_stream.avail_in = vecUrlLoaderBuf.size() - (zstart - gzstart);
			dec_stream.next_in  = zstart;
			enum { UNCOMP_BUF_SIZE = 2048 };
			size_t out_size = 0;
			for (int err = inflateInit2(&dec_stream, -MZ_DEFAULT_WINDOW_BITS); err == Z_OK; out_size += UNCOMP_BUF_SIZE - dec_stream.avail_out)
			{
				uncompr.resize(out_size + UNCOMP_BUF_SIZE);
				dec_stream.next_out = (unsigned char*)&uncompr[out_size];
				dec_stream.avail_out = UNCOMP_BUF_SIZE;
				err = inflate(&dec_stream, Z_NO_FLUSH);
			}
			uncompr.resize(out_size);
			inflateEnd(&dec_stream);
			//ZL_LOG("PEXEGZ", "Decompressed %d bytes to %d bytes", vecUrlLoaderBuf.size(), uncompr.size());
			vecUrlLoaderBuf.swap(uncompr);
		}
		#endif

		uint32_t size_before_zip = 0, loaded_size = (uint32_t)vecUrlLoaderBuf.size();
		char* pPEXEBegin = &*vecUrlLoaderBuf.begin(), *pZipHdr = pPEXEBegin + loaded_size - 20;
		for (; pZipHdr > pPEXEBegin; pZipHdr--) if (pZipHdr[0] == 0x50 && pZipHdr[1] == 0x4b && pZipHdr[2] == 0x05 && pZipHdr[3] == 0x06) break;
		if (pZipHdr > pPEXEBegin)
		{
			uint32_t size_central_dir = *(uint32_t*)&pZipHdr[12];
			uint32_t offset_central_dir = *(uint32_t*)&pZipHdr[16];
			size_before_zip = (uint32_t)(pZipHdr - pPEXEBegin) - (offset_central_dir + size_central_dir);
			if (size_central_dir > loaded_size || offset_central_dir > loaded_size || size_before_zip > loaded_size) size_before_zip = 0;
			//ZL_LOG("PEXEZIP", "central_pos: %u - size_central_dir: %u - offset_central_dir: %u - size_before_zip: %u", (uint32_t)(pZipHdr - pPEXEBegin), size_central_dir, offset_central_dir, size_before_zip);
		}
		if (size_before_zip > (uint32_t)(loaded_size/16)) //if size before start of asset zip archive is more than one 16th of the entire file, deallocate that part
		{
			//ZL_LOG("PEXEZIP", "Shrink by %u - VecSize %u -> %u (Capacity Before: %u)", size_before_zip, (uint32_t)loaded_size, (uint32_t)loaded_size - size_before_zip, (uint32_t)vecUrlLoaderBuf.capacity());
			vecUrlLoaderBuf.erase(vecUrlLoaderBuf.begin(), vecUrlLoaderBuf.begin() + size_before_zip);
			vecUrlLoaderBuf.shrink_to_fit();
			//ZL_LOG("PEXEZIP", "(Capacity After: %u)", (uint32_t)vecUrlLoaderBuf.capacity());
		}
	}

	if (status != 200)
	{
		//PKE = Package Error
		if (ppb_messaging_interface) ppb_messaging_interface->PostMessage(instance_, ppb_var_interface->VarFromUtf8("PKE", 3));
		return;
	}

	ZL_File::DefaultReadFileContainer = ZL_FileContainer_ZIP(ZL_File((void*)(vecUrlLoaderBuf.empty() ? NULL : &vecUrlLoaderBuf[0]), vecUrlLoaderBuf.size()));

	//PKP = Package Done
	if (ppb_messaging_interface) ppb_messaging_interface->PostMessage(instance_, ppb_var_interface->VarFromUtf8("PKD", 3));
}

void PackageLoader_Open_Callback(void* urlloader, int32_t result)
{
	if (result != PP_OK)
	{
		//PKE = Package Error
		if (ppb_messaging_interface) ppb_messaging_interface->PostMessage(instance_, ppb_var_interface->VarFromUtf8("PKE", 3));
		ppb_urlloader_interface->Close((PP_Resource)urlloader);
		return;
	}
	int64_t bytes_received, total_bytes_to_be_received;
	ppb_urlloader_interface->GetDownloadProgress((PP_Resource)urlloader, &bytes_received, &total_bytes_to_be_received);
	vecUrlLoaderBuf.clear();
	if (total_bytes_to_be_received > 0) vecUrlLoaderBuf.reserve((size_t)total_bytes_to_be_received);
	vecUrlLoaderBuf.resize(URLLOAD_BUFSIZE);
	const PP_CompletionCallback cc = { &PacketLoader_Read_Callback, urlloader, 0 };
	ppb_urlloader_interface->ReadResponseBody((PP_Resource)urlloader, &vecUrlLoaderBuf[0], URLLOAD_BUFSIZE, cc);
}

static void HandleMessage(PP_Instance instance, PP_Var msg)
{
	instance_ = instance;
	uint32_t utf8len;
	const char* utf8msg = ppb_var_interface->VarToUtf8(msg, &utf8len);
	if (utf8len < 3) return;
	//ZL_LOG3("NACL", "Got Message: [%*s] (GRAPHICS: %d)", utf8len, utf8msg, (context3d!=0));
	if (!memcmp(utf8msg, "PKG", 3) && utf8len > 3 && !inited_)
	{
		ZL_String url(utf8msg+3, utf8len-3);
		ZL_LOG1("PKG", "Requesting package: [%s]", url.c_str());
		PP_Resource urlloader = ppb_urlloader_interface->Create(instance);
		PP_Resource urlrequestinfo = ppb_urlrequestinfo_interface->Create(instance);
		ppb_urlrequestinfo_interface->SetProperty(urlrequestinfo, PP_URLREQUESTPROPERTY_URL, ZLStrToVar(url));
		ppb_urlrequestinfo_interface->SetProperty(urlrequestinfo, PP_URLREQUESTPROPERTY_RECORDDOWNLOADPROGRESS, PP_MakeBool(PP_TRUE));

		const PP_CompletionCallback cc = { &PackageLoader_Open_Callback, (void*)urlloader, 0 };
		if (ppb_urlloader_interface->Open(urlloader, urlrequestinfo, cc) != PP_OK_COMPLETIONPENDING) PackageLoader_Open_Callback(cc.user_data, -1);
	}
	else if (!memcmp(utf8msg, "SZX", 3) && utf8len > 3 && !inited_)
	{
		NACL_Width = atoi(utf8msg+3);
	}
	else if (!memcmp(utf8msg, "SZY", 3) && utf8len > 3 && !inited_)
	{
		NACL_Height = atoi(utf8msg+3);
	}
	else if (!memcmp(utf8msg, "INI", 3) && !inited_)
	{
		ZillaLibInit(0, NULL);
		inited_ = true;
	}
	else if (!memcmp(utf8msg, "RUN", 3) && inited_ && !running_)
	{
		ZL_LOG2("NACL", "Running with %s callback (%d)", ((ZL_TPF_Limit > (unsigned short)(1000/55)) ? "tpf" : "asap"), ZL_TPF_Limit);
		if (ZL_TPF_Limit > (unsigned int)(1000/55)) update_tpf_callback(NULL, 0);
		else { update_asap_callback(NULL, 0); ZL_TPF_Limit = 0; }
		running_ = true;
	}
	else if (!memcmp(utf8msg, "VAR", 3) && utf8len > 5)
	{
		char* varname;
		uint32_t namelen = strtol(utf8msg+3, &varname, 10);
		if (utf8len < (varname-utf8msg)+namelen+1) return;
		settings[ZL_String(varname+1, namelen)] = ZL_String(varname+1+namelen, utf8len-((varname-utf8msg)+namelen+1));
	}
	else if (!memcmp(utf8msg, "FUL", 3) && inited_)
	{
		//Unfortunately there is not a reliable way to initiate fullscreen with a message from javascript.
		//Currently this seems to fail if the NACL module never had input focus initiated by the user (was never clicked or had no key presses)
		ZL_LOG1("NACL", "Request full screen toggle by message - Current fullscreen status: %d", !!(NACL_WindowFlags & ZL_WINDOW_FULLSCREEN));
		ZL_SetFullscreen(!(NACL_WindowFlags & ZL_WINDOW_FULLSCREEN));
	}
}

static PP_Bool Instance_DidCreate(PP_Instance instance, uint32_t /*argc*/, const char* /*argn*/[], const char* /*argv*/[])
{
	if (instance_) return PP_FALSE;
	instance_ = instance;
	ppb_inputevent_interface->RequestInputEvents(instance_, 0xFFFFFF);
	ZL_LOG0("NACL", "I got created!");
	return PP_TRUE;
}

static void Instance_DidDestroy(PP_Instance instance)
{
	instance_ = instance;
}

static void Instance_DidChangeView(PP_Instance instance, PP_Resource view_resource)
{
	instance_ = instance;

	PP_Rect recViewOld = recView;
	ppb_view_interface->GetRect(view_resource, &recView);
	//ZL_LOG("NACL", "Instance_DidChangeView: x = %d -> %d - y = %d -> %d - w = %d -> %d - h = %d -> %d - fs: %d -> %d", recViewOld.point.x, recView.point.x, recViewOld.point.y, recView.point.y, recViewOld.size.width, recView.size.width, recViewOld.size.height, recView.size.height, (NACL_WindowFlags&ZL_WINDOW_FULLSCREEN?1:0), ppb_fullscreen_interface->IsFullscreen(instance));

	if (!context3d && !rdysent_)
	{
		//during startup
		ppb_messaging_interface->PostMessage(instance, ppb_var_interface->VarFromUtf8("RDY", 3));
		rdysent_ = true;
	}
	else if (recView.size.width != recViewOld.size.width || recView.size.height != recViewOld.size.height)
	{
		//ResizeContext
		ppb_graphics3d_interface->ResizeBuffers(context3d, recView.size.width, recView.size.height);
	}
	else if ((NACL_WindowFlags&ZL_WINDOW_FULLSCREEN?1:0) == ppb_fullscreen_interface->IsFullscreen(instance))
	{
		//no resize or full-screen change happened
		return;
	}

	//read fullscreen status
	NACL_WindowFlags = (ppb_fullscreen_interface->IsFullscreen(instance) ? (NACL_WindowFlags|ZL_WINDOW_FULLSCREEN) : (NACL_WindowFlags&~ZL_WINDOW_FULLSCREEN));

	//PP_Size screensize;ZL_LOG("NACL", "Is fullscreen: %d - Is fullscreen: %d - got size: %d - size: %d x %d - recview: %d x %d", ppb_fullscreen_interface->IsFullscreen(instance), (NACL_WindowFlags & ZL_WINDOW_FULLSCREEN), ppb_fullscreen_interface->GetScreenSize(instance, &screensize), screensize.width, screensize.height, recView.size.width, recView.size.height);

	if (inited_)
	{
		NACL_Width = recView.size.width;
		NACL_Height = recView.size.height;
		ZL_WindowEvent(ZL_WINDOWEVENT_RESIZED, recView.size.width, recView.size.height);
	}
}

static PP_Bool HandleInputEvent(PP_Instance instance, PP_Resource input_event)
{
	if (!running_) return PP_FALSE;
	static const ZL_Key nacl_zlkey_table[] = {
		ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_CANCEL,        ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_HELP,          ZLK_UNKNOWN,       ZLK_BACKSPACE,     ZLK_TAB,
		ZLK_KP_ENTER,      ZLK_UNKNOWN,       ZLK_CLEAR,         ZLK_RETURN,        ZLK_KP_ENTER,      ZLK_UNKNOWN,       ZLK_LSHIFT,        ZLK_LCTRL,         ZLK_LALT,          ZLK_PAUSE,
		ZLK_CAPSLOCK,      ZLK_LANG3,         ZLK_LANG6,         ZLK_LANG7,         ZLK_LANG8,         ZLK_LANG2,         ZLK_LANG9,         ZLK_ESCAPE,        ZLK_UNKNOWN,       ZLK_UNKNOWN,
		ZLK_UNKNOWN,       ZLK_MODE,          ZLK_SPACE,         ZLK_PAGEUP,        ZLK_PAGEDOWN,      ZLK_END,           ZLK_HOME,          ZLK_LEFT,          ZLK_UP,            ZLK_RIGHT,
		ZLK_DOWN,          ZLK_SELECT,        ZLK_PRINTSCREEN,   ZLK_EXECUTE,       ZLK_PRINTSCREEN,   ZLK_INSERT,        ZLK_DELETE,        ZLK_HELP,          ZLK_0,             ZLK_1,
		ZLK_2,             ZLK_3,             ZLK_4,             ZLK_5,             ZLK_6,             ZLK_7,             ZLK_8,             ZLK_9,             ZLK_KP_COLON,      ZLK_SEMICOLON,
		ZLK_KP_LESS,       ZLK_EQUALS,        ZLK_KP_GREATER,    ZLK_UNKNOWN,       ZLK_KP_AT,         ZLK_A,             ZLK_B,             ZLK_C,             ZLK_D,             ZLK_E,
		ZLK_F,             ZLK_G,             ZLK_H,             ZLK_I,             ZLK_J,             ZLK_K,             ZLK_L,             ZLK_M,             ZLK_N,             ZLK_O,
		ZLK_P,             ZLK_Q,             ZLK_R,             ZLK_S,             ZLK_T,             ZLK_U,             ZLK_V,             ZLK_W,             ZLK_X,             ZLK_Y,
		ZLK_Z,             ZLK_LGUI,          ZLK_RGUI,          ZLK_APPLICATION,   ZLK_UNKNOWN,       ZLK_SLEEP,         ZLK_KP_0,          ZLK_KP_1,          ZLK_KP_2,          ZLK_KP_3,
		ZLK_KP_4,          ZLK_KP_5,          ZLK_KP_6,          ZLK_KP_7,          ZLK_KP_8,          ZLK_KP_9,          ZLK_KP_MULTIPLY,   ZLK_KP_PLUS,       ZLK_SEPARATOR,     ZLK_KP_MINUS,
		ZLK_KP_DECIMAL,    ZLK_KP_DIVIDE,     ZLK_F1,            ZLK_F2,            ZLK_F3,            ZLK_F4,            ZLK_F5,            ZLK_F6,            ZLK_F7,            ZLK_F8,
		ZLK_F9,            ZLK_F10,           ZLK_F11,           ZLK_F12,           ZLK_F13,           ZLK_F14,           ZLK_F15,           ZLK_F16,           ZLK_F17,           ZLK_F18,
		ZLK_F19,           ZLK_F20,           ZLK_F21,           ZLK_F22,           ZLK_F23,           ZLK_F24,           ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,
		ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_NUMLOCKCLEAR,  ZLK_SCROLLLOCK,    ZLK_KP_EQUALS,     ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,
		ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,
		ZLK_LSHIFT,        ZLK_RSHIFT,        ZLK_LCTRL,         ZLK_RCTRL,         ZLK_LALT,          ZLK_RALT,          ZLK_AC_BACK,       ZLK_AC_FORWARD,    ZLK_AC_REFRESH,    ZLK_AC_STOP,
		ZLK_AC_SEARCH,     ZLK_AC_BOOKMARKS,  ZLK_AC_HOME,       ZLK_AUDIOMUTE,     ZLK_VOLUMEDOWN,    ZLK_VOLUMEUP,      ZLK_UNKNOWN,       ZLK_KP_000,        ZLK_KP_EQUALS,     ZLK_KP_00,
		ZLK_MAIL,          ZLK_MEDIASELECT,   ZLK_WWW,           ZLK_CALCULATOR,    ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_SEMICOLON,     ZLK_EQUALS,        ZLK_COMMA,         ZLK_MINUS,
		ZLK_PERIOD,        ZLK_SLASH,         ZLK_GRAVE,         ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,
		ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,
		ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_LEFTBRACKET,
		ZLK_BACKSLASH,     ZLK_RIGHTBRACKET,  ZLK_APOSTROPHE,    ZLK_UNKNOWN,       ZLK_LGUI,          ZLK_RALT,          ZLK_NONUSBACKSLASH,ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,
		ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,
		ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_SYSREQ,        ZLK_CRSEL,         ZLK_EXSEL,         ZLK_UNKNOWN,
		ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_UNKNOWN,       ZLK_CLEAR,         ZLK_UNKNOWN
	};
	static PP_Point nacl_last_mouse_pnt = { 200 , 200 };
	PP_InputEvent_Type type = ppb_inputevent_interface->GetType(input_event);
	PP_Point pnt;
	ZL_Event e;
	uint32_t mod;
	switch (type)
	{
		case PP_INPUTEVENT_TYPE_MOUSEDOWN:
		case PP_INPUTEVENT_TYPE_MOUSEUP:{
			PP_InputEvent_MouseButton btn = ppb_mouseinputevent_interface->GetButton(input_event);
			pnt = ppb_mouseinputevent_interface->GetPosition(input_event);
			mod = ppb_inputevent_interface->GetModifiers(input_event);
			e.type = (type == PP_INPUTEVENT_TYPE_MOUSEDOWN ? ZL_EVENT_MOUSEBUTTONDOWN : ZL_EVENT_MOUSEBUTTONUP);
			e.button.which = 0;
			e.button.button = (btn == PP_INPUTEVENT_MOUSEBUTTON_LEFT ? ZL_BUTTON_LEFT : (btn == PP_INPUTEVENT_MOUSEBUTTON_MIDDLE ? ZL_BUTTON_MIDDLE : (btn == PP_INPUTEVENT_MOUSEBUTTON_RIGHT ? ZL_BUTTON_RIGHT : 0)));
			e.button.is_down = (type == PP_INPUTEVENT_TYPE_MOUSEDOWN);
			e.button.x = pnt.x;
			e.button.y = pnt.y;
			ZL_Display_Process_Event(e);
			}break;
		case PP_INPUTEVENT_TYPE_MOUSEMOVE:
			mod = ppb_inputevent_interface->GetModifiers(input_event);
			e.type = ZL_EVENT_MOUSEMOTION;
			e.motion.which = 0;
			e.motion.state = (mod & PP_INPUTEVENT_MODIFIER_LEFTBUTTONDOWN ? (1<<(ZL_BUTTON_LEFT-1)) : 0) + (mod & PP_INPUTEVENT_MODIFIER_MIDDLEBUTTONDOWN ? (1<<(ZL_BUTTON_MIDDLE-1)) : 0) + (mod & PP_INPUTEVENT_MODIFIER_RIGHTBUTTONDOWN ? (1<<(ZL_BUTTON_RIGHT-1)) : 0);
			pnt = ppb_mouseinputevent_interface->GetMovement(input_event);
			e.motion.xrel = pnt.x; //(nacl_last_mouse_pnt.x >= 0 ? pnt.x - nacl_last_mouse_pnt.x : 0);
			e.motion.yrel = pnt.y; //(nacl_last_mouse_pnt.y >= 0 ? pnt.y - nacl_last_mouse_pnt.y : 0);
			if (ZL_WINDOWFLAGS_HAS(ZL_WINDOW_POINTERLOCK))
			{
				nacl_last_mouse_pnt.x += pnt.x;
				nacl_last_mouse_pnt.y += pnt.y;
				if      (nacl_last_mouse_pnt.x <                     0) nacl_last_mouse_pnt.x = 0;
				else if (nacl_last_mouse_pnt.x >  recView.size.width-1) nacl_last_mouse_pnt.x = recView.size.width-1;
				if      (nacl_last_mouse_pnt.y <                     0) nacl_last_mouse_pnt.y = 0;
				else if (nacl_last_mouse_pnt.y > recView.size.height-1) nacl_last_mouse_pnt.y = recView.size.height-1;
			}
			else { nacl_last_mouse_pnt = ppb_mouseinputevent_interface->GetPosition(input_event); }
			e.motion.x = nacl_last_mouse_pnt.x;
			e.motion.y = nacl_last_mouse_pnt.y;
			if (/*nacl_last_mouse_pnt.x < 0 || */e.motion.xrel || e.motion.yrel) ZL_Display_Process_Event(e);
			//nacl_last_mouse_pnt = pnt;
			break;
		case PP_INPUTEVENT_TYPE_MOUSEENTER:
			//if (pp_audio && !has_focus_) ppb_audio_interface->StartPlayback(pp_audio);
			if ((NACL_WindowFlags & ZL_WINDOW_MOUSE_FOCUS) == 0) ZL_WindowEvent(ZL_WINDOWEVENT_FOCUS, (NACL_WindowFlags |= ZL_WINDOW_MOUSE_FOCUS));
			break;
		case PP_INPUTEVENT_TYPE_MOUSELEAVE:
			//if (pp_audio && !has_focus_) ppb_audio_interface->StopPlayback(pp_audio);
			if (NACL_WindowFlags & ZL_WINDOW_MOUSE_FOCUS) ZL_WindowEvent(ZL_WINDOWEVENT_FOCUS, (NACL_WindowFlags &= ~ZL_WINDOW_MOUSE_FOCUS));
			//nacl_last_mouse_pnt.x = nacl_last_mouse_pnt.y = -1;
			break;
		case PP_INPUTEVENT_TYPE_WHEEL:
			e.type = ZL_EVENT_MOUSEWHEEL;
			e.wheel.which = 0;
			e.wheel.x = (ppb_wheelinputevent_interface->GetDelta(input_event).x*0.72f);
			e.wheel.y = (ppb_wheelinputevent_interface->GetDelta(input_event).y*0.72f);
			ZL_Display_Process_Event(e);
			break;
		case PP_INPUTEVENT_TYPE_KEYDOWN:
		case PP_INPUTEVENT_TYPE_KEYUP:{
			uint32_t key = ppb_keyboardinputevent_interface->GetKeyCode(input_event);
			mod = ppb_inputevent_interface->GetModifiers(input_event);
			e.type = (type == PP_INPUTEVENT_TYPE_KEYDOWN ? ZL_EVENT_KEYDOWN : ZL_EVENT_KEYUP);
			e.key.is_down = (type == PP_INPUTEVENT_TYPE_KEYDOWN);
			e.key.key = nacl_zlkey_table[key & 255];
			e.key.mod = (mod & PP_INPUTEVENT_MODIFIER_SHIFTKEY ? ZLKMOD_SHIFT : 0) +
			            (mod & PP_INPUTEVENT_MODIFIER_CONTROLKEY ? ZLKMOD_CTRL : 0) +
			            (mod & PP_INPUTEVENT_MODIFIER_ALTKEY ? ZLKMOD_ALT : 0) +
			            (mod & PP_INPUTEVENT_MODIFIER_METAKEY ? ZLKMOD_META : 0) +
			            (mod & PP_INPUTEVENT_MODIFIER_CAPSLOCKKEY ? ZLKMOD_CAPS : 0) +
			            (mod & PP_INPUTEVENT_MODIFIER_NUMLOCKKEY ? ZLKMOD_NUM : 0) +
			            (mod & PP_INPUTEVENT_MODIFIER_ISKEYPAD ? ZLKMOD_MODE : 0) +
			            (mod & PP_INPUTEVENT_MODIFIER_ISAUTOREPEAT ? ZLKMOD_RESERVED : 0);
			ZL_Display_Process_Event(e);
			if (e.key.is_down && e.key.key == ZLK_BACKSPACE) { e.type = ZL_EVENT_TEXTINPUT; e.text.text[0] = '\b'; e.text.text[1] = 0; ZL_Display_Process_Event(e); }
			}break;
		case PP_INPUTEVENT_TYPE_CHAR:{
			uint32_t utf8len;
			const char* utf8char = ppb_var_interface->VarToUtf8(ppb_keyboardinputevent_interface->GetCharacterText(input_event), &utf8len);
			e.type = ZL_EVENT_TEXTINPUT;
			if (utf8len == 1) { e.text.text[0] = (utf8char[0] == '\r' ? '\n' : utf8char[0]); e.text.text[1] = 0; }
			else
			{
				int maxlen = (utf8len >= sizeof(e.text.text) ? sizeof(e.text.text)-1: utf8len);
				memcpy(e.text.text, utf8char, maxlen);
				e.text.text[maxlen] = 0;
			}
			ZL_Display_Process_Event(e);
			}break;
		default:
		//case PP_INPUTEVENT_TYPE_RAWKEYDOWN:
		//case PP_INPUTEVENT_TYPE_CONTEXTMENU:
		//case PP_INPUTEVENT_TYPE_IME_COMPOSITION_START:
		//case PP_INPUTEVENT_TYPE_IME_COMPOSITION_UPDATE:
		//case PP_INPUTEVENT_TYPE_IME_COMPOSITION_END:
			break;
	}
	return PP_TRUE;
}

static void HandleMouseLockLost(PP_Instance instance)
{
	NACL_WindowFlags &= ~ZL_WINDOW_POINTERLOCK;
}

static void Instance_DidChangeFocus(PP_Instance instance, PP_Bool has_focus)
{
	instance_ = instance;
	has_focus_ = has_focus;
	//if (pp_audio) { if (has_focus) ppb_audio_interface->StartPlayback(pp_audio); else ppb_audio_interface->StopPlayback(pp_audio); }
	if (running_)
	{
		if (has_focus && ((NACL_WindowFlags & ZL_WINDOW_INPUT_FOCUS) == 0)) ZL_WindowEvent(ZL_WINDOWEVENT_FOCUS, (NACL_WindowFlags = (NACL_WindowFlags|ZL_WINDOW_INPUT_FOCUS)&(~(ZL_WINDOW_MINIMIZED))));
		if (!has_focus && (NACL_WindowFlags & ZL_WINDOW_INPUT_FOCUS))       ZL_WindowEvent(ZL_WINDOWEVENT_FOCUS, (NACL_WindowFlags = (NACL_WindowFlags|ZL_WINDOW_MINIMIZED)&(~(ZL_WINDOW_INPUT_FOCUS))));
	}
}

static PP_Bool Instance_HandleDocumentLoad(PP_Instance /*instance*/, PP_Resource /*url_loader*/) { return PP_FALSE; }
PP_EXPORT void PPP_ShutdownModule() { }

PP_EXPORT int32_t PPP_InitializeModule(PP_Module /*a_module_id*/, PPB_GetInterface get_browser)
{
	ppb_core_interface =                             (PPB_Core*)(get_browser(PPB_CORE_INTERFACE));
	ppb_messaging_interface =                   (PPB_Messaging*)(get_browser(PPB_MESSAGING_INTERFACE));
	ppb_var_interface =                               (PPB_Var*)(get_browser(PPB_VAR_INTERFACE));
	ppb_audio_interface =                           (PPB_Audio*)(get_browser(PPB_AUDIO_INTERFACE));
	ppb_audioconfig_interface =               (PPB_AudioConfig*)(get_browser(PPB_AUDIO_CONFIG_INTERFACE));
	ppb_instance_interface =                     (PPB_Instance*)(get_browser(PPB_INSTANCE_INTERFACE));
	ppb_graphics3d_interface =                 (PPB_Graphics3D*)(get_browser(PPB_GRAPHICS_3D_INTERFACE));
	ppb_view_interface =                             (PPB_View*)(get_browser(PPB_VIEW_INTERFACE));
	ppb_urlloader_interface =                   (PPB_URLLoader*)(get_browser(PPB_URLLOADER_INTERFACE));
	ppb_urlrequestinfo_interface =         (PPB_URLRequestInfo*)(get_browser(PPB_URLREQUESTINFO_INTERFACE));
	ppb_urlresponseinfo_interface =       (PPB_URLResponseInfo*)(get_browser(PPB_URLRESPONSEINFO_INTERFACE));
	ppb_inputevent_interface =                 (PPB_InputEvent*)(get_browser(PPB_INPUT_EVENT_INTERFACE));
	ppb_mouseinputevent_interface =       (PPB_MouseInputEvent*)(get_browser(PPB_MOUSE_INPUT_EVENT_INTERFACE));
	ppb_wheelinputevent_interface =       (PPB_WheelInputEvent*)(get_browser(PPB_WHEEL_INPUT_EVENT_INTERFACE));
	ppb_keyboardinputevent_interface = (PPB_KeyboardInputEvent*)(get_browser(PPB_KEYBOARD_INPUT_EVENT_INTERFACE));
	ppb_fullscreen_interface =                 (PPB_Fullscreen*)(get_browser(PPB_FULLSCREEN_INTERFACE));
	ppb_mouselock_interface =                   (PPB_MouseLock*)(get_browser(PPB_MOUSELOCK_INTERFACE));
	ppb_websocket_interface =                   (PPB_WebSocket*)(get_browser(PPB_WEBSOCKET_INTERFACE));
	ppb_vararraybuffer_interface =         (PPB_VarArrayBuffer*)(get_browser(PPB_VAR_ARRAY_BUFFER_INTERFACE));
	if (glInitializePPAPI(get_browser) != GL_TRUE) return PP_ERROR_NOTSUPPORTED;
	return PP_OK;
}

PP_EXPORT const void* PPP_GetInterface(const char* interface_name)
{
	if (!strcmp(interface_name, PPP_INSTANCE_INTERFACE))
	{
		static PPP_Instance instance_interface = { &Instance_DidCreate, &Instance_DidDestroy, &Instance_DidChangeView, &Instance_DidChangeFocus, &Instance_HandleDocumentLoad };
		return &instance_interface;
	}
	if (!strcmp(interface_name, PPP_INPUT_EVENT_INTERFACE))
	{
		static PPP_InputEvent inputevent_interface = { &HandleInputEvent };
		return &inputevent_interface;
	}
	if (!strcmp(interface_name, PPP_MESSAGING_INTERFACE))
	{
		static PPP_Messaging message_interface = { &HandleMessage };
		return &message_interface;
	}
	if (!strcmp(interface_name, PPP_MOUSELOCK_INTERFACE))
	{
		static PPP_MouseLock mouse_lock_interface = { &HandleMouseLockLost };
		return &mouse_lock_interface;
	}
	return NULL;
}

void ZL_StartTicks()
{
	nacl_tstart = ppb_core_interface->GetTimeTicks();
}

ticks_t ZL_GetTicks()
{
	return (ticks_t)(1000.0 * (ppb_core_interface->GetTimeTicks() - nacl_tstart));
}

void ZL_Delay(ticks_t ms)
{
	int was_error;
	timespec elapsed, tv;
	elapsed.tv_sec = ms / 1000;
	elapsed.tv_nsec = (ms % 1000) * 1000000;
	do
	{
		tv.tv_sec = elapsed.tv_sec;
		tv.tv_nsec = elapsed.tv_nsec;
		was_error = nanosleep(&tv, &elapsed);
	} while (was_error);
}

void __nacl_log_mainthread(void* user_data, int32_t)
{
	ppb_messaging_interface->PostMessage(instance_, *(PP_Var *)user_data);
	ppb_var_interface->Release(*(PP_Var *)user_data);
	delete (PP_Var *)user_data;
}

void __nacl_log(const char* logtag, const char* logtext)
{
	if (!ppb_core_interface || !ppb_messaging_interface) return;
	char loglog[1024];
	uint32_t logloglen = snprintf(loglog, 1024, "LOG[%s] %s", logtag, logtext);
	PP_Var ppmsg = ppb_var_interface->VarFromUtf8(loglog, logloglen);
	if (ppb_core_interface->IsMainThread())
	{
		ppb_messaging_interface->PostMessage(instance_, ppmsg);
	}
	else
	{
		PP_Var *ppmsgcp = new PP_Var();
		memcpy(ppmsgcp, &ppmsg, sizeof(ppmsg));
		ppb_var_interface->AddRef(*ppmsgcp);
		PP_CompletionCallback cbu = { &__nacl_log_mainthread, ppmsgcp, 0 };
		ppb_core_interface->CallOnMainThread(0, cbu, 0);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ZL_WindowEvent(unsigned char event, int data1, int data2)
{
	ZL_Event e; e.type = ZL_EVENT_WINDOW;
	e.window.event = event; e.window.data1 = data1; e.window.data2 = data2;
	ZL_Display_Process_Event(e);
}

// WINDOW
bool ZL_CreateWindow(const char*, int width, int height, int displayflags)
{
	ZL_LOG("NACL", "ZL_CreateWindow - Code Window Size: %d / %d - Requested Size: %d / %d - Current View Size: %d / %d", width, height, NACL_Width, NACL_Height, recView.size.width, recView.size.height);

	//Window size
	if (NACL_Width > 0 && NACL_Height > 0) { } //use as is
	else if (NACL_Width  > 0) NACL_Height = NACL_Width*height/width;
	else if (NACL_Height > 0) NACL_Width = NACL_Height*width/height;
	else if (recView.size.width < 2 && recView.size.height < 2) NACL_Width = width, NACL_Height = height;
	else NACL_Height = recView.size.width, NACL_Height = recView.size.height;
	recView.size.width = NACL_Width, recView.size.height = NACL_Height;

	if (!context3d)
	{
		//Create context
		int32_t attribs[] = { PP_GRAPHICS3DATTRIB_ALPHA_SIZE, 0, PP_GRAPHICS3DATTRIB_DEPTH_SIZE, (displayflags & ZL_DISPLAY_DEPTHBUFFER ? 16 : 0), PP_GRAPHICS3DATTRIB_STENCIL_SIZE, 0,
		                      PP_GRAPHICS3DATTRIB_SAMPLES, 4, PP_GRAPHICS3DATTRIB_SAMPLE_BUFFERS, 1, PP_GRAPHICS3DATTRIB_WIDTH, NACL_Width, PP_GRAPHICS3DATTRIB_HEIGHT, NACL_Height, PP_GRAPHICS3DATTRIB_NONE };
		if (!(context3d = ppb_graphics3d_interface->Create(instance_, NULL, attribs)) || !ppb_instance_interface->BindGraphics(instance_, context3d))
		{
			ZL_LOG0("NACL", "Failed to start graphics - No OpenGL available");
			if (ppb_messaging_interface) ppb_messaging_interface->PostMessage(instance_, ppb_var_interface->VarFromUtf8("NOG", 3));
			return false;
		}
		glSetCurrentContextPPAPI(context3d);
	}

	//WIN = Window created
	char msgmsg[1024];
	uint32_t msgmsglen = sprintf(msgmsg, "WIN%d,%d", NACL_Width, NACL_Height);
	if (ppb_messaging_interface) ppb_messaging_interface->PostMessage(instance_, ppb_var_interface->VarFromUtf8(msgmsg, msgmsglen));

	//PrepareOpenGL
	ZLGLSL::CreateShaders();
	glClearColor(1, 0, 0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	//WindowFlags
	pZL_WindowFlags = &NACL_WindowFlags;

	return true;
}

void ZL_GetWindowSize(int *w, int *h)
{
	*w = NACL_Width;
	*h = NACL_Height;
}

void ZL_SetFullscreen(bool toFullscreen)
{
	ZL_LOG("NACL", "SetFullscreen - To Status: %d - Interface: %d - Instance: %d", toFullscreen, (int)ppb_fullscreen_interface, (int)instance_);
	ppb_fullscreen_interface->SetFullscreen(instance_, (PP_Bool)toFullscreen);
}

void ZL_SetPointerLock_Callback(void*, int32_t) { NACL_WindowFlags |= ZL_WINDOW_POINTERLOCK; }

void ZL_SetPointerLock(bool doLockPointer)
{
	if (doLockPointer)
	{
		PP_CompletionCallback cc = { &ZL_SetPointerLock_Callback, NULL, 0 };
		ppb_mouselock_interface->LockMouse(instance_,  cc);
	}
	else ppb_mouselock_interface->UnlockMouse(instance_);
}

// JOYSTICK
int ZL_NumJoysticks() { return 0; }
ZL_JoystickData* ZL_JoystickHandleOpen(int index) { return NULL; }
void ZL_JoystickHandleClose(ZL_JoystickData* joystick) { }

//Audio
#ifdef PPB_AUDIO_CONFIG_INTERFACE_1_1
void nacl_audio_callback(void* sample_buffer, uint32_t buffer_size_in_bytes, PP_TimeDelta, void*) { ZL_PlatformAudioMix((short*)sample_buffer, buffer_size_in_bytes); }
#else
void nacl_audio_callback(void* sample_buffer, uint32_t buffer_size_in_bytes, void*) { ZL_PlatformAudioMix((short*)sample_buffer, buffer_size_in_bytes); }
#endif

bool ZL_AudioOpen()
{
	ZL_LOG0("NACL AUDIO", "Starting audio");
	#ifdef PPB_AUDIO_CONFIG_INTERFACE_1_1
	uint32_t count = ppb_audioconfig_interface->RecommendSampleFrameCount(instance_, PP_AUDIOSAMPLERATE_44100, 1024);
	#else
	uint32_t count = ppb_audioconfig_interface->RecommendSampleFrameCount(PP_AUDIOSAMPLERATE_44100, 1024);
	#endif
	//ZL_LOG1("NACL AUDIO", "Sample buffe count: %d", count);
	PP_Resource pp_audio_config = ppb_audioconfig_interface->CreateStereo16Bit(instance_, PP_AUDIOSAMPLERATE_44100, count);
	//ZL_LOG1("NACL AUDIO", "Got sound config: %d", pp_audio_config);
	pp_audio = ppb_audio_interface->Create(instance_, pp_audio_config, nacl_audio_callback, NULL);
	//ZL_LOG1("NACL AUDIO", "Got audio interface: %d", pp_audio);
	ppb_audio_interface->StartPlayback(pp_audio);
	//ZL_LOG1("NACL AUDIO", "Started audio: %d", started);
	return true;
}

void ZL_OpenExternalUrl(const char* url)
{
	if (ppb_messaging_interface) ppb_messaging_interface->PostMessage(instance_, ZLStrToVar(ZL_String("URL") << url));
}

//settings
void ZL_SettingsInit(const char* FallbackConfigFilePrefix)
{
	if (ppb_messaging_interface) ppb_messaging_interface->PostMessage(instance_, ZLStrToVar(ZL_String("GET") << FallbackConfigFilePrefix));
}

const ZL_String ZL_SettingsGet(const char* Key)
{
	if (!settings.count(Key)) return ZL_String::EmptyString;
	return settings[Key];
}

void ZL_SettingsSet(const char* Key, const ZL_String& Value)
{
	ZL_String strKey(Key);
	if (ppb_messaging_interface) ppb_messaging_interface->PostMessage(instance_, ZLStrToVar(ZL_String("SET") << strKey.length() << ' ' << strKey << Value));
	settings[strKey] = Value;
}

void ZL_SettingsDel(const char* Key)
{
	if (!settings.count(Key)) return;
	if (ppb_messaging_interface) ppb_messaging_interface->PostMessage(instance_, ZLStrToVar(ZL_String("DEL") << Key));
	settings.erase(Key);
}

bool ZL_SettingsHas(const char* Key)
{
	return (settings.count(Key) ? true : false);
}

void ZL_SettingsSynchronize()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

ZL_HTTPCONNECTION_IMPL_INTERFACE

ZL_HttpConnection_Impl::ZL_HttpConnection_Impl() : dostream(false), urlloader(0) { }
void ZL_HttpConnection_Disconnect(ZL_HttpConnection_Impl *impl)
{
	if (!impl->urlloader) return;
	ppb_urlloader_interface->Close(impl->urlloader);
	impl->urlloader = 0;
	impl->DelRef();
}
void ZL_HttpConnection_Read_Callback(void* vimpl, int32_t result)
{
	ZL_HttpConnection_Impl *impl = (ZL_HttpConnection_Impl*)vimpl;
	if (!impl || !impl->urlloader) return;
	//ZL_LOG2("NACL", "ZL_HttpConnection_Read_Callback: %d - dostream: %d", result, impl->dostream);

	int32_t status = ppb_urlresponseinfo_interface->GetProperty(ppb_urlloader_interface->GetResponseInfo(impl->urlloader), PP_URLRESPONSEPROPERTY_STATUSCODE).value.as_int;

	if (!impl->dostream)
	{
		int64_t bytes_received, total_bytes_to_be_received;
		ppb_urlloader_interface->GetDownloadProgress(impl->urlloader, &bytes_received, &total_bytes_to_be_received);
		if (total_bytes_to_be_received > 0 && impl->data.capacity() < (size_t)total_bytes_to_be_received) impl->data.reserve((size_t)total_bytes_to_be_received);

		if (result != URLLOAD_BUFSIZE) impl->data.resize(impl->data.size() - URLLOAD_BUFSIZE + (result > 0 ? result : 0));
	}

	if (impl->dostream && result > 0)
	{
		if (impl->sigReceivedString.HasConnections()) impl->sigReceivedString.call(status, ZL_String(&impl->data[0], (size_t)result));
		if (impl->sigReceivedData.HasConnections())   impl->sigReceivedData.call(status, &impl->data[0], (size_t)result);
	}

	if (result > 0)
	{
		if (!impl->dostream) impl->data.resize(impl->data.size() + URLLOAD_BUFSIZE);
		const PP_CompletionCallback cc = { &ZL_HttpConnection_Read_Callback, vimpl, 0 };
		ppb_urlloader_interface->ReadResponseBody(impl->urlloader, &impl->data[impl->data.size() - URLLOAD_BUFSIZE], URLLOAD_BUFSIZE, cc);
		return;
	}

	if (impl->dostream) impl->data.clear(); //just send a 0 byte length final termination packet

	if (impl->sigReceivedString.HasConnections()) impl->sigReceivedString.call(status, (impl->data.empty() ? ZL_String::EmptyString : ZL_String(&impl->data[0], impl->data.size())));
	if (impl->sigReceivedData.HasConnections())   impl->sigReceivedData.call(status, (impl->data.empty() ? NULL : &impl->data[0]), impl->data.size());
	ZL_HttpConnection_Disconnect(impl);
}
void ZL_HttpConnection_Open_Callback(void* vimpl, int32_t result)
{
	ZL_HttpConnection_Impl *impl = (ZL_HttpConnection_Impl*)vimpl;
	//ZL_LOG1("NACL", "ZL_HttpConnection_Open_Callback: %d", result);
	if (result != PP_OK)
	{
		impl->sigReceivedString.call(result, ZL_String::EmptyString);
		impl->sigReceivedData.call(result, NULL, 0);
		ZL_HttpConnection_Disconnect(impl);
		return;
	}
	int64_t bytes_received, total_bytes_to_be_received;
	ppb_urlloader_interface->GetDownloadProgress(impl->urlloader, &bytes_received, &total_bytes_to_be_received);
	if (total_bytes_to_be_received > 0) impl->data.reserve((size_t)total_bytes_to_be_received);
	impl->data.resize(URLLOAD_BUFSIZE);
	const PP_CompletionCallback cc = { &ZL_HttpConnection_Read_Callback, vimpl, 0 };
	ppb_urlloader_interface->ReadResponseBody(impl->urlloader, &impl->data[0], URLLOAD_BUFSIZE, cc);
}
void ZL_HttpConnection_Impl::Connect()
{
	if (!url.length()) return;
	urlloader = ppb_urlloader_interface->Create(instance_);
	PP_Resource urlrequestinfo = ppb_urlrequestinfo_interface->Create(instance_);
	ppb_urlrequestinfo_interface->SetProperty(urlrequestinfo, PP_URLREQUESTPROPERTY_URL, ZLStrToVar(url));
	ppb_urlrequestinfo_interface->SetProperty(urlrequestinfo, PP_URLREQUESTPROPERTY_ALLOWCROSSORIGINREQUESTS, PP_MakeBool(PP_TRUE));
	ppb_urlrequestinfo_interface->SetProperty(urlrequestinfo, PP_URLREQUESTPROPERTY_RECORDDOWNLOADPROGRESS, PP_MakeBool(PP_TRUE));
	if (post_data.size())
	{
		ppb_urlrequestinfo_interface->SetProperty(urlrequestinfo, PP_URLREQUESTPROPERTY_METHOD, ppb_var_interface->VarFromUtf8("POST", 4));
		ppb_urlrequestinfo_interface->AppendDataToBody(urlrequestinfo, &post_data[0], post_data.size());
	}
	//ZL_LOG3("NACL", "Loading URL: %s (DoStream: %d - Post data: %d bytes)", url.c_str(), dostream, post_data.size());
	AddRef();
	const PP_CompletionCallback cc = { &ZL_HttpConnection_Open_Callback, (void*)this, 0 };
	if (ppb_urlloader_interface->Open(urlloader, urlrequestinfo, cc) != PP_OK_COMPLETIONPENDING) ZL_HttpConnection_Open_Callback(cc.user_data, -1);
}

ZL_WEBSOCKETCONNECTION_IMPL_INTERFACE
ZL_WebSocketConnection_Impl::ZL_WebSocketConnection_Impl() : websocket_active(false) { }
void ZL_WebSocketConnection_Impl::Connect()
{
	struct ZL_WebSocket_Callbacks
	{
		static void OnConnect(ZL_WebSocketConnection_Impl* impl, int32_t result)
		{
			//ZL_LOG("NACLWSC", "ONCONNECT - WS: %d - RESULT: %d - READYSTATE: %d", impl->websocket, result, (int32_t)ppb_websocket_interface->GetReadyState(impl->websocket));
			if (result < 0) { impl->Disconnect(PP_WEBSOCKETSTATUSCODE_ABNORMAL_CLOSURE, NULL, 0); return; }
			impl->websocket_active = true;
			impl->sigConnected.call();
			if (ppb_websocket_interface->ReceiveMessage(impl->websocket, &impl->data, PP_MakeCompletionCallback((PP_CompletionCallback_Func)&OnReceiveMessage, impl)) == PP_OK)
				OnReceiveMessage(impl, PP_OK);
		}
		static void OnReceiveMessage(ZL_WebSocketConnection_Impl* impl, int32_t result)
		{
			//ZL_LOG("NACLWSC", "ONRECEIVE - WS: %d - RESULT: %d - READYSTATE: %d - BUFFER: %d", impl->websocket, result, (int32_t)ppb_websocket_interface->GetReadyState(impl->websocket), (int32_t)ppb_websocket_interface->GetBufferedAmount(impl->websocket));
			if (result < 0) { impl->Disconnect(PP_WEBSOCKETSTATUSCODE_ABNORMAL_CLOSURE, NULL, 0); return; }
			HandleMessage(impl);
			const PP_CompletionCallback cc = PP_MakeCompletionCallback((PP_CompletionCallback_Func)&OnReceiveMessage, (void*)impl);
			while (ppb_websocket_interface->ReceiveMessage(impl->websocket, &impl->data, cc) == PP_OK) HandleMessage(impl);
		}
		static void HandleMessage(ZL_WebSocketConnection_Impl* impl)
		{
			//ZL_LOG("NACLWSC", "HANDLEMESSAGE - WS: %d - VALUE_ID: %d - READYSTATE: %d", impl->websocket, (int32_t)impl->data.value.as_id, (int32_t)ppb_websocket_interface->GetReadyState(impl->websocket));
			if (!impl->data.value.as_id) return;
			if (impl->data.type == PP_VARTYPE_STRING) 
			{
				uint32_t length;
				const char* data = ppb_var_interface->VarToUtf8(impl->data, &length);
				impl->sigReceivedText.call(ZL_String(data, length));
			}
			else if (impl->data.type == PP_VARTYPE_ARRAY_BUFFER)
			{
				uint32_t length;
				ppb_vararraybuffer_interface->ByteLength(impl->data, &length);
				const char* data = (const char*)ppb_vararraybuffer_interface->Map(impl->data);
				impl->sigReceivedBinary.call(data, length);
			}
			ppb_var_interface->Release(impl->data);
		}
	};
	websocket = ppb_websocket_interface->Create(instance_);
	ppb_websocket_interface->Connect(websocket, ZLStrToVar(url), NULL, 0, PP_MakeCompletionCallback((PP_CompletionCallback_Func)&ZL_WebSocket_Callbacks::OnConnect, this));
}
void ZL_WebSocketConnection_Impl::SendText(const char* buf, size_t len)
{
	if (!websocket) return;
	ppb_websocket_interface->SendMessage(websocket, ppb_var_interface->VarFromUtf8(buf, len));
}
void ZL_WebSocketConnection_Impl::SendBinary(const void* buf, size_t len)
{
	if (!websocket) return;
	PP_Var v = ppb_vararraybuffer_interface->Create(len);
	memcpy(ppb_vararraybuffer_interface->Map(v), buf, len);
	ppb_websocket_interface->SendMessage(websocket, v);
}
void ZL_WebSocketConnection_Impl::Disconnect(unsigned short code, const char* buf, size_t len)
{
	if (!websocket) return;
	struct ZL_WebSocket_Callbacks { static void OnDisconnect(void*, int32_t) {} };
	ppb_websocket_interface->Close(websocket, code, (buf ? ppb_var_interface->VarFromUtf8(buf, len) : PP_MakeUndefined()), PP_MakeCompletionCallback(&ZL_WebSocket_Callbacks::OnDisconnect, NULL));
	websocket = 0;
	websocket_active = false;
	sigDisconnected.call();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

//empty wrappers to reduce linked binary size
extern "C"{
void __wrap__ZNSt8ios_base4InitC1Ev(){}
void __wrap__ZN9__gnu_cxx27__verbose_terminate_handlerEv(){}
void _ZNKSt8time_getIcSt19istreambuf_iteratorIcSt11char_traitsIcEEE11do_get_dateES3_S3_RSt8ios_baseRSt12_Ios_IostateP2tm(){ZL_LOG1("NACL", "ERROR! Seemingly unused function accessed [%d]", 1);}
void _ZNKSt8time_getIcSt19istreambuf_iteratorIcSt11char_traitsIcEEE11do_get_timeES3_S3_RSt8ios_baseRSt12_Ios_IostateP2tm(){ZL_LOG1("NACL", "ERROR! Seemingly unused function accessed [%d]", 2);}
void _ZNKSt8time_getIcSt19istreambuf_iteratorIcSt11char_traitsIcEEE14do_get_weekdayES3_S3_RSt8ios_baseRSt12_Ios_IostateP2tm(){ZL_LOG1("NACL", "ERROR! Seemingly unused function accessed [%d]", 3);}
void _ZNKSt8time_getIcSt19istreambuf_iteratorIcSt11char_traitsIcEEE16do_get_monthnameES3_S3_RSt8ios_baseRSt12_Ios_IostateP2tm(){ZL_LOG1("NACL", "ERROR! Seemingly unused function accessed [%d]", 4);}
void _ZNKSt8time_getIwSt19istreambuf_iteratorIwSt11char_traitsIwEEE11do_get_dateES3_S3_RSt8ios_baseRSt12_Ios_IostateP2tm(){ZL_LOG1("NACL", "ERROR! Seemingly unused function accessed [%d]", 5);}
void _ZNKSt8time_getIwSt19istreambuf_iteratorIwSt11char_traitsIwEEE11do_get_timeES3_S3_RSt8ios_baseRSt12_Ios_IostateP2tm(){ZL_LOG1("NACL", "ERROR! Seemingly unused function accessed [%d]", 6);}
void _ZNKSt8time_getIwSt19istreambuf_iteratorIwSt11char_traitsIwEEE14do_get_weekdayES3_S3_RSt8ios_baseRSt12_Ios_IostateP2tm(){ZL_LOG1("NACL", "ERROR! Seemingly unused function accessed [%d]", 7);}
void _ZNKSt8time_getIwSt19istreambuf_iteratorIwSt11char_traitsIwEEE16do_get_monthnameES3_S3_RSt8ios_baseRSt12_Ios_IostateP2tm(){ZL_LOG1("NACL", "ERROR! Seemingly unused function accessed [%d]", 8);}
void _ZNKSt8time_putIwSt19ostreambuf_iteratorIwSt11char_traitsIwEEE6do_putES3_RSt8ios_basewPK2tmcc(){ZL_LOG1("NACL", "ERROR! Seemingly unused function accessed [%d]", 9);}
void _ZNKSt8time_putIcSt19ostreambuf_iteratorIcSt11char_traitsIcEEE6do_putES3_RSt8ios_basecPK2tmcc(){ZL_LOG1("NACL", "ERROR! Seemingly unused function accessed [%d]", 10);}
void _ZNKSt8time_getIcSt19istreambuf_iteratorIcSt11char_traitsIcEEE11do_get_yearES3_S3_RSt8ios_baseRSt12_Ios_IostateP2tm(){ZL_LOG1("NACL", "ERROR! Seemingly unused function accessed [%d]", 11);}
void _ZNKSt8time_getIwSt19istreambuf_iteratorIwSt11char_traitsIwEEE11do_get_yearES3_S3_RSt8ios_baseRSt12_Ios_IostateP2tm(){ZL_LOG1("NACL", "ERROR! Seemingly unused function accessed [%d]", 12);}
void _ZNKSt9money_getIcSt19istreambuf_iteratorIcSt11char_traitsIcEEE6do_getES3_S3_bRSt8ios_baseRSt12_Ios_IostateRSs(){ZL_LOG1("NACL", "ERROR! Seemingly unused function accessed [%d]", 13);}
void _ZNKSt9money_getIcSt19istreambuf_iteratorIcSt11char_traitsIcEEE6do_getES3_S3_bRSt8ios_baseRSt12_Ios_IostateRe(){ZL_LOG1("NACL", "ERROR! Seemingly unused function accessed [%d]", 14);}
void _ZNKSt9money_getIwSt19istreambuf_iteratorIwSt11char_traitsIwEEE6do_getES3_S3_bRSt8ios_baseRSt12_Ios_IostateRSbIwS2_SaIwEE(){ZL_LOG1("NACL", "ERROR! Seemingly unused function accessed [%d]", 15);}
void _ZNKSt9money_getIwSt19istreambuf_iteratorIwSt11char_traitsIwEEE6do_getES3_S3_bRSt8ios_baseRSt12_Ios_IostateRe(){ZL_LOG1("NACL", "ERROR! Seemingly unused function accessed [%d]", 16);}
void _ZNKSt9money_putIcSt19ostreambuf_iteratorIcSt11char_traitsIcEEE6do_putES3_bRSt8ios_basecRKSs(){ZL_LOG1("NACL", "ERROR! Seemingly unused function accessed [%d]", 17);}
void _ZNKSt9money_putIcSt19ostreambuf_iteratorIcSt11char_traitsIcEEE6do_putES3_bRSt8ios_basece(){ZL_LOG1("NACL", "ERROR! Seemingly unused function accessed [%d]", 18);}
void _ZNKSt9money_putIwSt19ostreambuf_iteratorIwSt11char_traitsIwEEE6do_putES3_bRSt8ios_basewRKSbIwS2_SaIwEE(){ZL_LOG1("NACL", "ERROR! Seemingly unused function accessed [%d]", 19);}
void _ZNKSt9money_putIwSt19ostreambuf_iteratorIwSt11char_traitsIwEEE6do_putES3_bRSt8ios_basewe(){ZL_LOG1("NACL", "ERROR! Seemingly unused function accessed [%d]", 20);}
}

#endif // __native_client__
