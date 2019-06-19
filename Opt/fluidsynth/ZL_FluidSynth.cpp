//------------------------------------------------------------------------------
/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */
//------------------------------------------------------------------------------
/*
 * August 24, 1998
 * Copyright (C) 1998 Juergen Mueller And Sundry Contributors
 * This source code is freely redistributable and may be used for
 * any purpose.  This copyright notice must be maintained.
 * Juergen Mueller And Sundry Contributors are not responsible for
 * the consequences of using this software.
 */
// (USED IN FILE: fluid_chorus.c)
//------------------------------------------------------------------------------
/* SoundFont file loading code borrowed from Smurf SoundFont Editor
 * Copyright (C) 1999-2001 Josh Green
 */
// (USED IN FILES: fluid_sfont.h, fluid_sfont.c)
//------------------------------------------------------------------------------
/* GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
// (USED IN FILES: fluid_list.h, fluid_list.c)
//------------------------------------------------------------------------------
/*
  Freeverb

  Written by Jezar at Dreampoint, June 2000
  http://www.dreampoint.co.uk
  This code is public domain

  Translated to C by Peter Hanappe, Mai 2001
  */
// (USED IN FILES: fluid_rev.c)
//------------------------------------------------------------------------------

#include <math.h>

#ifndef _FLUIDSYNTH_PRIV_H
#define _FLUIDSYNTH_PRIV_H

#define __synth_reverb_active 0
#define __synth_ladspa_active 0
#define __synth_dump 0
#define __synth_polyphony 64
#define __synth_sample_rate 44100.0f
#define __synth_midi_channels 16
#define __synth_audio_channels 1
#define __synth_audio_groups 1
#define __synth_effects_channels 0
#define __synth_gain 0.2
#define __audio_sample_format "8bits"
#define __audio_output_channels 1
#define __audio_input_channels 0

#define WITH_FLOAT 1

#if (defined(WIN32) || defined(_WIN32)) && !defined(MINGW32)
#define HAVE_STRING_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STDIO_H 1
#define HAVE_MATH_H 1
#define HAVE_STDARG_H 1
#define HAVE_FCNTL_H 1
#define HAVE_LIMITS_H 1
#define HAVE_IO_H 1
#define HAVE_WINDOWS_H 1
#include <stdio.h>
#ifndef snprintf
#define snprintf _snprintf
#endif
#define strcasecmp _stricmp
#if _MSC_VER < 1500
#define vsnprintf _vsnprintf
#endif
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
#pragma warning(disable : 4244)
#pragma warning(disable : 4101)
#pragma warning(disable : 4305)
#pragma warning(disable : 4996)
#ifndef INLINE
#define INLINE __inline
#endif
#else
#define INLINE inline
#define DARWIN
#define HAVE_STRING_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STDIO_H 1
#define HAVE_MATH_H 1
#define HAVE_STDARG_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_FCNTL_H 1
#define HAVE_UNISTD_H 1
#define HAVE_LIMITS_H 1
#define HAVE_PTHREAD_H 1
#define HAVE_SYS_TIME_H 1
#ifdef ZL_USE_BIGENDIAN
#define WORDS_BIGENDIAN 1
#endif
#ifndef HAVE_UNISTD_H
#ifndef socklen_t
typedef int socklen_t;
#endif
#endif
#endif

#undef DEBUG
#define WITH_PROFILING 0
#define WITHOUT_SERVER 1

#if HAVE_STRING_H
#include <string.h>
#endif

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif

#if HAVE_STDIO_H
#include <stdio.h>
#endif

#if HAVE_MATH_H
#include <math.h>
#endif

#if HAVE_ERRNO_H
#include <errno.h>
#endif

#if HAVE_STDARG_H
#include <stdarg.h>
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if HAVE_FCNTL_H
#include <fcntl.h>
#endif

#if HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#if HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif

#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#if HAVE_LIMITS_H
#include <limits.h>
#endif

#if HAVE_PTHREAD_H
#ifndef _POSIX_THREAD_PRIORITY_SCHEDULING
#define _POSIX_THREAD_PRIORITY_SCHEDULING
#endif
#include <pthread.h>
#if defined(__native_client__) || defined(__EMSCRIPTEN__)
#define pthread_attr_setschedpolicy(a,s) 0
#define pthread_attr_setschedparam(a,s) 0
#endif
#endif

#if HAVE_IO_H
#include <io.h>
#endif

#if HAVE_WINDOWS_H
#include <windows.h>
#endif

#ifdef MINGW32

#include <stdint.h>
#include <stdio.h>
#ifndef snprintf
#define snprintf _snprintf
#endif
#define vsnprintf _vsnprintf

#define DSOUND_SUPPORT 1
#define WINMIDI_SUPPORT 1
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
#define WITHOUT_SERVER 1

#endif

#ifdef DARWIN
#define MACINTOSH
#define __Types__
#define WITHOUT_SERVER 1
#endif

#if defined(WITH_FLOAT)
typedef float fluid_real_t;
#else
typedef double fluid_real_t;
#endif

typedef enum {
	FLUID_OK = 0,
	FLUID_FAILED = -1
} fluid_status;

typedef int fluid_socket_t;
#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

#if defined(MINGW32)

typedef int8_t             sint8;
typedef uint8_t            uint8;
typedef int16_t            sint16;
typedef uint16_t           uint16;
typedef int32_t            sint32;
typedef uint32_t           uint32;
typedef int64_t            sint64;
typedef uint64_t           uint64;

#elif defined(_WIN32)

typedef signed __int8      sint8;
typedef unsigned __int8    uint8;
typedef signed __int16     sint16;
typedef unsigned __int16   uint16;
typedef signed __int32     sint32;
typedef unsigned __int32   uint32;
typedef signed __int64     sint64;
typedef unsigned __int64   uint64;

#elif defined(MACOS9)

typedef signed char        sint8;
typedef unsigned char      uint8;
typedef signed short       sint16;
typedef unsigned short     uint16;
typedef signed int         sint32;
typedef unsigned int       uint32;

typedef long long          sint64;
typedef unsigned long long uint64;

#elif defined(__native_client__)

typedef int8_t             sint8;
typedef uint8_t            uint8;
typedef int16_t            sint16;
typedef uint16_t           uint16;
typedef int32_t            sint32;
typedef uint32_t           uint32;
typedef int64_t            sint64;
typedef uint64_t           uint64;

#else

typedef int8_t             sint8;
typedef u_int8_t           uint8;
typedef int16_t            sint16;
typedef u_int16_t          uint16;
typedef int32_t            sint32;
typedef u_int32_t          uint32;
typedef int64_t            sint64;
typedef u_int64_t          uint64;

#endif

typedef struct _fluid_env_data_t fluid_env_data_t;
typedef struct _fluid_adriver_definition_t fluid_adriver_definition_t;
typedef struct _fluid_channel_t fluid_channel_t;
typedef struct _fluid_tuning_t fluid_tuning_t;
typedef struct _fluid_client_t fluid_client_t;
typedef struct _fluid_server_socket_t fluid_server_socket_t;

#define FLUID_BUFSIZE                64

#ifndef PI
#define PI                          3.141592654
#endif

typedef FILE*  fluid_file;

#define FLUID_MALLOC(_n)             malloc(_n)
#define FLUID_REALLOC(_p,_n)         realloc(_p,_n)
#define FLUID_NEW(_t)                (_t*)malloc(sizeof(_t))
#define FLUID_ARRAY(_t,_n)           (_t*)malloc((_n)*sizeof(_t))
#define FLUID_FREE(_p)               free(_p)
#define FLUID_FOPEN(_f,_m)           fopen(_f,_m)
#define FLUID_FCLOSE(_f)             fclose(_f)
#define FLUID_FREAD(_p,_s,_n,_f)     fread(_p,_s,_n,_f)
#define FLUID_FSEEK(_f,_n,_set)      fseek(_f,_n,_set)
#define FLUID_MEMCPY(_dst,_src,_n)   memcpy(_dst,_src,_n)
#define FLUID_MEMSET(_s,_c,_n)       memset(_s,_c,_n)
#define FLUID_STRLEN(_s)             strlen(_s)
#define FLUID_STRCMP(_s,_t)          strcmp(_s,_t)
#define FLUID_STRNCMP(_s,_t,_n)      strncmp(_s,_t,_n)
#define FLUID_STRCPY(_dst,_src)      strcpy(_dst,_src)
#define FLUID_STRCHR(_s,_c)          strchr(_s,_c)
#ifdef strdup
#define FLUID_STRDUP(s)              strdup(s)
#else
#define FLUID_STRDUP(s) 		    FLUID_STRCPY((char*)FLUID_MALLOC(FLUID_STRLEN(s) + 1), s)
#endif
#define FLUID_SPRINTF                sprintf
#define FLUID_FPRINTF                fprintf

#define fluid_clip(_val, _min, _max) \
	{ (_val) = ((_val) < (_min))? (_min) : (((_val) > (_max))? (_max) : (_val)); }

#if WITH_FTS
#define FLUID_PRINTF                 post
#define FLUID_FLUSH()
#else
#define FLUID_PRINTF                 printf
#define FLUID_FLUSH()                fflush(stdout)
#endif

#define FLUID_NO_ERR_OUTPUT
#define FLUID_LOG(a,b)               (void)(0)
#define FLUID_LOG1(a,b,c)            (void)(0)
#define FLUID_LOG2(a,b,c,d)          (void)(0)
#define FLUID_LOG3(a,b,c,d,e)        (void)(0)
#define FLUID_LOG4(a,b,c,d,e,f)      (void)(0)

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

#define FLUID_ASSERT(a,b)
#define FLUID_ASSERT_P(a,b)

char* fluid_error(void);

#define _(s) s

#endif

#define FLUID_NUM_PROGRAMS      128
#define DRUM_INST_BANK		128

#if defined(WITH_FLOAT)
#define FLUID_SAMPLE_FORMAT     FLUID_SAMPLE_FLOAT
#else
#define FLUID_SAMPLE_FORMAT     FLUID_SAMPLE_DOUBLE
#endif

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: fluidsynth.h
//

#ifndef _FLUIDSYNTH_H
#define _FLUIDSYNTH_H

#define FLUIDSYNTH_API static

#ifndef _FLUIDSYNTH_TYPES_H
#define _FLUIDSYNTH_TYPES_H

typedef struct _fluid_hashtable_t fluid_settings_t;
typedef struct _fluid_synth_t fluid_synth_t;
typedef struct _fluid_voice_t fluid_voice_t;
typedef struct _fluid_sfloader_t fluid_sfloader_t;
typedef struct _fluid_sfont_t fluid_sfont_t;
typedef struct _fluid_preset_t fluid_preset_t;
typedef struct _fluid_sample_t fluid_sample_t;
typedef struct _fluid_mod_t fluid_mod_t;
typedef struct _fluid_audio_driver_t fluid_audio_driver_t;
typedef struct _fluid_player_t fluid_player_t;
typedef struct _fluid_midi_event_t fluid_midi_event_t;
typedef struct _fluid_midi_driver_t fluid_midi_driver_t;
typedef struct _fluid_midi_router_t fluid_midi_router_t;
typedef struct _fluid_midi_router_rule_t fluid_midi_router_rule_t;
typedef struct _fluid_hashtable_t fluid_cmd_handler_t;
typedef struct _fluid_shell_t fluid_shell_t;
typedef struct _fluid_server_t fluid_server_t;
typedef struct _fluid_event_t fluid_event_t;
typedef struct _fluid_sequencer_t fluid_sequencer_t;
typedef struct _fluid_ramsfont_t fluid_ramsfont_t;
typedef struct _fluid_rampreset_t fluid_rampreset_t;

typedef int fluid_istream_t;
typedef int fluid_ostream_t;

#endif

#ifndef _FLUIDSYNTH_SYNTH_H
#define _FLUIDSYNTH_SYNTH_H

FLUIDSYNTH_API fluid_synth_t* new_fluid_synth();

FLUIDSYNTH_API int delete_fluid_synth(fluid_synth_t* synth);

FLUIDSYNTH_API int fluid_synth_noteon(fluid_synth_t* synth, int chan, int key, int vel);

FLUIDSYNTH_API int fluid_synth_noteoff(fluid_synth_t* synth, int chan, int key);

FLUIDSYNTH_API int fluid_synth_cc(fluid_synth_t* synth, int chan, int ctrl, int val);

FLUIDSYNTH_API int fluid_synth_get_cc(fluid_synth_t* synth, int chan, int ctrl, int* pval);

FLUIDSYNTH_API int fluid_synth_pitch_bend(fluid_synth_t* synth, int chan, int val);

FLUIDSYNTH_API int fluid_synth_get_pitch_bend(fluid_synth_t* synth, int chan, int* ppitch_bend);

FLUIDSYNTH_API int fluid_synth_pitch_wheel_sens(fluid_synth_t* synth, int chan, int val);

FLUIDSYNTH_API int fluid_synth_get_pitch_wheel_sens(fluid_synth_t* synth, int chan, int* pval);

FLUIDSYNTH_API int fluid_synth_program_change(fluid_synth_t* synth, int chan, int program);

FLUIDSYNTH_API int fluid_synth_bank_select(fluid_synth_t* synth, int chan, unsigned int bank);

FLUIDSYNTH_API int fluid_synth_sfont_select(fluid_synth_t* synth, int chan, unsigned int sfont_id);

FLUIDSYNTH_API int fluid_synth_program_select(fluid_synth_t* synth, int chan,
unsigned int sfont_id,
unsigned int bank_num,
unsigned int preset_num);

FLUIDSYNTH_API int fluid_synth_get_program(fluid_synth_t* synth, int chan,
unsigned int* sfont_id,
unsigned int* bank_num,
unsigned int* preset_num);

FLUIDSYNTH_API int fluid_synth_program_reset(fluid_synth_t* synth);

FLUIDSYNTH_API int fluid_synth_system_reset(fluid_synth_t* synth);

FLUIDSYNTH_API int fluid_synth_start(fluid_synth_t* synth, unsigned int id,
	fluid_preset_t* preset, int audio_chan,
	int midi_chan, int key, int vel);

FLUIDSYNTH_API int fluid_synth_stop(fluid_synth_t* synth, unsigned int id);

FLUIDSYNTH_API int fluid_synth_sfload(fluid_synth_t* synth, const char* filename, int reset_presets);

FLUIDSYNTH_API int fluid_synth_sfreload(fluid_synth_t* synth, unsigned int id);

FLUIDSYNTH_API int fluid_synth_sfunload(fluid_synth_t* synth, unsigned int id, int reset_presets);

FLUIDSYNTH_API int fluid_synth_add_sfont(fluid_synth_t* synth, fluid_sfont_t* sfont);

FLUIDSYNTH_API void fluid_synth_remove_sfont(fluid_synth_t* synth, fluid_sfont_t* sfont);

FLUIDSYNTH_API int fluid_synth_sfcount(fluid_synth_t* synth);

FLUIDSYNTH_API fluid_sfont_t* fluid_synth_get_sfont(fluid_synth_t* synth, unsigned int num);

FLUIDSYNTH_API fluid_sfont_t* fluid_synth_get_sfont_by_id(fluid_synth_t* synth, unsigned int id);

FLUIDSYNTH_API fluid_preset_t* fluid_synth_get_channel_preset(fluid_synth_t* synth, int chan);

FLUIDSYNTH_API int fluid_synth_set_bank_offset(fluid_synth_t* synth, int sfont_id, int offset);

FLUIDSYNTH_API int fluid_synth_get_bank_offset(fluid_synth_t* synth, int sfont_id);

FLUIDSYNTH_API void fluid_synth_set_reverb(fluid_synth_t* synth, double roomsize,
	double damping, double width, double level);

FLUIDSYNTH_API void fluid_synth_set_reverb_on(fluid_synth_t* synth, int on);

FLUIDSYNTH_API double fluid_synth_get_reverb_roomsize(fluid_synth_t* synth);
FLUIDSYNTH_API double fluid_synth_get_reverb_damp(fluid_synth_t* synth);
FLUIDSYNTH_API double fluid_synth_get_reverb_level(fluid_synth_t* synth);
FLUIDSYNTH_API double fluid_synth_get_reverb_width(fluid_synth_t* synth);

#define FLUID_REVERB_DEFAULT_ROOMSIZE 0.2f
#define FLUID_REVERB_DEFAULT_DAMP 0.0f
#define FLUID_REVERB_DEFAULT_WIDTH 0.5f
#define FLUID_REVERB_DEFAULT_LEVEL 0.9f

enum fluid_chorus_mod {
	FLUID_CHORUS_MOD_SINE = 0,
	FLUID_CHORUS_MOD_TRIANGLE = 1
};

FLUIDSYNTH_API void fluid_synth_set_chorus(fluid_synth_t* synth, int nr, double level,
	double speed, double depth_ms, int type);

FLUIDSYNTH_API void fluid_synth_set_chorus_on(fluid_synth_t* synth, int on);

FLUIDSYNTH_API int fluid_synth_get_chorus_nr(fluid_synth_t* synth);
FLUIDSYNTH_API double fluid_synth_get_chorus_level(fluid_synth_t* synth);
FLUIDSYNTH_API double fluid_synth_get_chorus_speed_Hz(fluid_synth_t* synth);
FLUIDSYNTH_API double fluid_synth_get_chorus_depth_ms(fluid_synth_t* synth);
FLUIDSYNTH_API int fluid_synth_get_chorus_type(fluid_synth_t* synth);

#define FLUID_CHORUS_DEFAULT_N 3
#define FLUID_CHORUS_DEFAULT_LEVEL 2.0f
#define FLUID_CHORUS_DEFAULT_SPEED 0.3f
#define FLUID_CHORUS_DEFAULT_DEPTH 8.0f
#define FLUID_CHORUS_DEFAULT_TYPE FLUID_CHORUS_MOD_SINE

FLUIDSYNTH_API int fluid_synth_count_midi_channels(fluid_synth_t* synth);

FLUIDSYNTH_API int fluid_synth_count_audio_channels(fluid_synth_t* synth);

FLUIDSYNTH_API int fluid_synth_count_audio_groups(fluid_synth_t* synth);

FLUIDSYNTH_API int fluid_synth_count_effects_channels(fluid_synth_t* synth);

FLUIDSYNTH_API void fluid_synth_set_gain(fluid_synth_t* synth, float gain);

FLUIDSYNTH_API float fluid_synth_get_gain(fluid_synth_t* synth);

FLUIDSYNTH_API int fluid_synth_set_polyphony(fluid_synth_t* synth, int polyphony);

FLUIDSYNTH_API int fluid_synth_get_polyphony(fluid_synth_t* synth);

FLUIDSYNTH_API int fluid_synth_get_internal_bufsize(fluid_synth_t* synth);

FLUIDSYNTH_API int fluid_synth_set_interp_method(fluid_synth_t* synth, int chan, int interp_method);

enum fluid_interp {
	FLUID_INTERP_NONE = 0,

	FLUID_INTERP_LINEAR = 1,

	FLUID_INTERP_DEFAULT = 4,
	FLUID_INTERP_4THORDER = 4,
	FLUID_INTERP_7THORDER = 7,
	FLUID_INTERP_HIGHEST = 7
};

FLUIDSYNTH_API int fluid_synth_set_gen(fluid_synth_t* synth, int chan, int param, float value);

FLUIDSYNTH_API float fluid_synth_get_gen(fluid_synth_t* synth, int chan, int param);

FLUIDSYNTH_API int fluid_synth_create_key_tuning(fluid_synth_t* synth, int tuning_bank, int tuning_prog,
char* name, double* pitch);

FLUIDSYNTH_API int fluid_synth_create_octave_tuning(fluid_synth_t* synth, int tuning_bank, int tuning_prog,
char* name, double* pitch);

FLUIDSYNTH_API int fluid_synth_tune_notes(fluid_synth_t* synth, int tuning_bank, int tuning_prog,
int len, int *keys, double* pitch, int apply);

FLUIDSYNTH_API int fluid_synth_select_tuning(fluid_synth_t* synth, int chan, int tuning_bank, int tuning_prog);

FLUIDSYNTH_API int fluid_synth_reset_tuning(fluid_synth_t* synth, int chan);

FLUIDSYNTH_API void fluid_synth_tuning_iteration_start(fluid_synth_t* synth);

FLUIDSYNTH_API int fluid_synth_tuning_iteration_next(fluid_synth_t* synth, int* bank, int* prog);

FLUIDSYNTH_API int fluid_synth_tuning_dump(fluid_synth_t* synth, int bank, int prog,
	char* name, int len, double* pitch);

FLUIDSYNTH_API double fluid_synth_get_cpu_load(fluid_synth_t* synth);

FLUIDSYNTH_API char* fluid_synth_error(fluid_synth_t* synth);

FLUIDSYNTH_API int fluid_synth_write_s16(fluid_synth_t* synth, int len,
	void* lout, int loff, int lincr,
	void* rout, int roff, int rincr);

FLUIDSYNTH_API int fluid_synth_write_float(fluid_synth_t* synth, int len,
	void* lout, int loff, int lincr,
	void* rout, int roff, int rincr);

FLUIDSYNTH_API int fluid_synth_nwrite_float(fluid_synth_t* synth, int len,
	float** left, float** right,
	float** fx_left, float** fx_right);

FLUIDSYNTH_API int fluid_synth_process(fluid_synth_t* synth, int len,
	int nin, float** in,
	int nout, float** out);

typedef int(*fluid_audio_callback_t)(fluid_synth_t* synth, int len,
	void* out1, int loff, int lincr,
	void* out2, int roff, int rincr);

FLUIDSYNTH_API void fluid_synth_add_sfloader(fluid_synth_t* synth, fluid_sfloader_t* loader);

FLUIDSYNTH_API fluid_voice_t* fluid_synth_alloc_voice(fluid_synth_t* synth, fluid_sample_t* sample,
	int channum, int key, int vel, unsigned int start_time);

FLUIDSYNTH_API void fluid_synth_start_voice(fluid_synth_t* synth, fluid_voice_t* voice);

FLUIDSYNTH_API void fluid_synth_get_voicelist(fluid_synth_t* synth,
	fluid_voice_t* buf[], int bufsize, int ID);

FLUIDSYNTH_API int fluid_synth_handle_midi_event(void* data, fluid_midi_event_t* event);

FLUIDSYNTH_API void fluid_synth_set_midi_router(fluid_synth_t* synth,
	fluid_midi_router_t* router);

#endif

#ifndef _FLUIDSYNTH_SFONT_H
#define _FLUIDSYNTH_SFONT_H

enum {
	FLUID_PRESET_SELECTED,
	FLUID_PRESET_UNSELECTED,
	FLUID_SAMPLE_DONE
};

struct _fluid_sfloader_t {
	void* data;

	int(*free)(fluid_sfloader_t* loader);

	fluid_sfont_t* (*load)(fluid_sfloader_t* loader, const char* filename);
};

struct _fluid_sfont_t {
	void* data;
	unsigned int id;

	int(*free)(fluid_sfont_t* sfont);

	char* (*get_name)(fluid_sfont_t* sfont);

	fluid_preset_t* (*get_preset)(fluid_sfont_t* sfont, unsigned int bank, unsigned int prenum);

	void(*iteration_start)(fluid_sfont_t* sfont);

	int(*iteration_next)(fluid_sfont_t* sfont, fluid_preset_t* preset);
};

#define fluid_sfont_get_id(_sf) ((_sf)->id)

struct _fluid_preset_t {
	void* data;
	fluid_sfont_t* sfont;
	int(*free)(fluid_preset_t* preset);
	char* (*get_name)(fluid_preset_t* preset);
	int(*get_banknum)(fluid_preset_t* preset);
	int(*get_num)(fluid_preset_t* preset);

	int(*noteon)(fluid_preset_t* preset, fluid_synth_t* synth, int chan, int key, int vel);

	int(*notify)(fluid_preset_t* preset, int reason, int chan);
};

struct _fluid_sample_t
{
	char name[21];
	unsigned int start;
	unsigned int end;
	unsigned int loopstart;
	unsigned int loopend;
	unsigned int samplerate;
	int origpitch;
	int pitchadj;
	int sampletype;
	int valid;
	short* data;

	int amplitude_that_reaches_noise_floor_is_valid;
	double amplitude_that_reaches_noise_floor;

	unsigned int refcount;

	int(*notify)(fluid_sample_t* sample, int reason);

	void* userdata;
};

#define fluid_sample_refcount(_sample) ((_sample)->refcount)

#define FLUID_SAMPLETYPE_MONO	1
#define FLUID_SAMPLETYPE_RIGHT	2
#define FLUID_SAMPLETYPE_LEFT	4
#define FLUID_SAMPLETYPE_LINKED	8
#define FLUID_SAMPLETYPE_ROM	0x8000

#endif

#ifndef _FLUIDSYNTH_RAMSFONT_H
#define _FLUIDSYNTH_RAMSFONT_H

FLUIDSYNTH_API fluid_sfont_t* fluid_ramsfont_create_sfont(void);

FLUIDSYNTH_API int fluid_ramsfont_set_name(fluid_ramsfont_t* sfont, char * name);

FLUIDSYNTH_API int fluid_ramsfont_add_izone(fluid_ramsfont_t* sfont,
unsigned int bank, unsigned int num, fluid_sample_t* sample,
int lokey, int hikey);

FLUIDSYNTH_API int fluid_ramsfont_remove_izone(fluid_ramsfont_t* sfont,
unsigned int bank, unsigned int num, fluid_sample_t* sample);

FLUIDSYNTH_API int fluid_ramsfont_izone_set_gen(fluid_ramsfont_t* sfont,
unsigned int bank, unsigned int num, fluid_sample_t* sample,
int gen_type, float value);

FLUIDSYNTH_API int fluid_ramsfont_izone_set_loop(fluid_ramsfont_t* sfont,
unsigned int bank, unsigned int num, fluid_sample_t* sample,
int on, float loopstart, float loopend);

FLUIDSYNTH_API fluid_sample_t* new_fluid_ramsample(void);
FLUIDSYNTH_API int delete_fluid_ramsample(fluid_sample_t* sample);
FLUIDSYNTH_API int fluid_sample_set_name(fluid_sample_t* sample, char * name);

FLUIDSYNTH_API int fluid_sample_set_sound_data(fluid_sample_t* sample, short *data,
unsigned int nbframes, short copy_data, int rootkey);

#endif

#ifndef _FLUIDSYNTH_AUDIO_H
#define _FLUIDSYNTH_AUDIO_H

typedef int(*fluid_audio_func_t)(void* data, int len,
	int nin, float** in,
	int nout, float** out);

FLUIDSYNTH_API fluid_audio_driver_t* new_fluid_audio_driver(fluid_synth_t* synth);

FLUIDSYNTH_API void delete_fluid_audio_driver(fluid_audio_driver_t* driver);

#endif

#ifndef _FLUIDSYNTH_EVENT_H
#define _FLUIDSYNTH_EVENT_H

enum fluid_seq_event_type {
	FLUID_SEQ_NOTE = 0,
	FLUID_SEQ_NOTEON,
	FLUID_SEQ_NOTEOFF,
	FLUID_SEQ_ALLSOUNDSOFF,
	FLUID_SEQ_ALLNOTESOFF,
	FLUID_SEQ_BANKSELECT,
	FLUID_SEQ_PROGRAMCHANGE,
	FLUID_SEQ_PROGRAMSELECT,
	FLUID_SEQ_PITCHBEND,
	FLUID_SEQ_PITCHWHHELSENS,
	FLUID_SEQ_MODULATION,
	FLUID_SEQ_SUSTAIN,
	FLUID_SEQ_CONTROLCHANGE,
	FLUID_SEQ_PAN,
	FLUID_SEQ_VOLUME,
	FLUID_SEQ_REVERBSEND,
	FLUID_SEQ_CHORUSSEND,
	FLUID_SEQ_TIMER,
	FLUID_SEQ_ANYCONTROLCHANGE,
	FLUID_SEQ_LASTEVENT
};

FLUIDSYNTH_API fluid_event_t* new_fluid_event(void);
FLUIDSYNTH_API void delete_fluid_event(fluid_event_t* evt);

FLUIDSYNTH_API void fluid_event_set_source(fluid_event_t* evt, short src);
FLUIDSYNTH_API void fluid_event_set_dest(fluid_event_t* evt, short dest);

FLUIDSYNTH_API void fluid_event_timer(fluid_event_t* evt, void* data);

FLUIDSYNTH_API void fluid_event_note(fluid_event_t* evt, int channel,
	short key, short vel,
	unsigned int duration);

FLUIDSYNTH_API void fluid_event_noteon(fluid_event_t* evt, int channel, short key, short vel);
FLUIDSYNTH_API void fluid_event_noteoff(fluid_event_t* evt, int channel, short key);
FLUIDSYNTH_API void fluid_event_all_sounds_off(fluid_event_t* evt, int channel);
FLUIDSYNTH_API void fluid_event_all_notes_off(fluid_event_t* evt, int channel);

FLUIDSYNTH_API void fluid_event_bank_select(fluid_event_t* evt, int channel, short bank_num);
FLUIDSYNTH_API void fluid_event_program_change(fluid_event_t* evt, int channel, short preset_num);
FLUIDSYNTH_API void fluid_event_program_select(fluid_event_t* evt, int channel, unsigned int sfont_id, short bank_num, short preset_num);

FLUIDSYNTH_API void fluid_event_control_change(fluid_event_t* evt, int channel, short control, short val);

FLUIDSYNTH_API void fluid_event_pitch_bend(fluid_event_t* evt, int channel, int val);
FLUIDSYNTH_API void fluid_event_pitch_wheelsens(fluid_event_t* evt, int channel, short val);
FLUIDSYNTH_API void fluid_event_modulation(fluid_event_t* evt, int channel, short val);
FLUIDSYNTH_API void fluid_event_sustain(fluid_event_t* evt, int channel, short val);
FLUIDSYNTH_API void fluid_event_pan(fluid_event_t* evt, int channel, short val);
FLUIDSYNTH_API void fluid_event_volume(fluid_event_t* evt, int channel, short val);
FLUIDSYNTH_API void fluid_event_reverb_send(fluid_event_t* evt, int channel, short val);
FLUIDSYNTH_API void fluid_event_chorus_send(fluid_event_t* evt, int channel, short val);

FLUIDSYNTH_API void fluid_event_any_control_change(fluid_event_t* evt, int channel);

FLUIDSYNTH_API int fluid_event_get_type(fluid_event_t* evt);
FLUIDSYNTH_API short fluid_event_get_source(fluid_event_t* evt);
FLUIDSYNTH_API short fluid_event_get_dest(fluid_event_t* evt);
FLUIDSYNTH_API int fluid_event_get_channel(fluid_event_t* evt);
FLUIDSYNTH_API short fluid_event_get_key(fluid_event_t* evt);
FLUIDSYNTH_API short fluid_event_get_velocity(fluid_event_t* evt);
FLUIDSYNTH_API short fluid_event_get_control(fluid_event_t* evt);
FLUIDSYNTH_API short fluid_event_get_value(fluid_event_t* evt);
FLUIDSYNTH_API short fluid_event_get_program(fluid_event_t* evt);
FLUIDSYNTH_API void* fluid_event_get_data(fluid_event_t* evt);
FLUIDSYNTH_API unsigned int fluid_event_get_duration(fluid_event_t* evt);
FLUIDSYNTH_API short fluid_event_get_bank(fluid_event_t* evt);
FLUIDSYNTH_API int fluid_event_get_pitch(fluid_event_t* evt);
FLUIDSYNTH_API unsigned int fluid_event_get_sfont_id(fluid_event_t* evt);

#endif

#ifndef _FLUIDSYNTH_MIDI_H
#define _FLUIDSYNTH_MIDI_H

FLUIDSYNTH_API fluid_midi_event_t* new_fluid_midi_event(void);
FLUIDSYNTH_API int delete_fluid_midi_event(fluid_midi_event_t* event);

FLUIDSYNTH_API int fluid_midi_event_set_type(fluid_midi_event_t* evt, int type);
FLUIDSYNTH_API int fluid_midi_event_get_type(fluid_midi_event_t* evt);
FLUIDSYNTH_API int fluid_midi_event_set_channel(fluid_midi_event_t* evt, int chan);
FLUIDSYNTH_API int fluid_midi_event_get_channel(fluid_midi_event_t* evt);
FLUIDSYNTH_API int fluid_midi_event_get_key(fluid_midi_event_t* evt);
FLUIDSYNTH_API int fluid_midi_event_set_key(fluid_midi_event_t* evt, int key);
FLUIDSYNTH_API int fluid_midi_event_get_velocity(fluid_midi_event_t* evt);
FLUIDSYNTH_API int fluid_midi_event_set_velocity(fluid_midi_event_t* evt, int vel);
FLUIDSYNTH_API int fluid_midi_event_get_control(fluid_midi_event_t* evt);
FLUIDSYNTH_API int fluid_midi_event_set_control(fluid_midi_event_t* evt, int ctrl);
FLUIDSYNTH_API int fluid_midi_event_get_value(fluid_midi_event_t* evt);
FLUIDSYNTH_API int fluid_midi_event_set_value(fluid_midi_event_t* evt, int val);
FLUIDSYNTH_API int fluid_midi_event_get_program(fluid_midi_event_t* evt);
FLUIDSYNTH_API int fluid_midi_event_set_program(fluid_midi_event_t* evt, int val);
FLUIDSYNTH_API int fluid_midi_event_get_pitch(fluid_midi_event_t* evt);
FLUIDSYNTH_API int fluid_midi_event_set_pitch(fluid_midi_event_t* evt, int val);

typedef int(*handle_midi_event_func_t)(void* data, fluid_midi_event_t* event);

FLUIDSYNTH_API fluid_midi_router_t* new_fluid_midi_router(fluid_settings_t* settings,
	handle_midi_event_func_t handler,
	void* event_handler_data);

FLUIDSYNTH_API int delete_fluid_midi_router(fluid_midi_router_t* handler);
FLUIDSYNTH_API int fluid_midi_router_handle_midi_event(void* data, fluid_midi_event_t* event);
FLUIDSYNTH_API int fluid_midi_dump_prerouter(void* data, fluid_midi_event_t* event);
FLUIDSYNTH_API int fluid_midi_dump_postrouter(void* data, fluid_midi_event_t* event);

FLUIDSYNTH_API fluid_midi_driver_t* new_fluid_midi_driver(fluid_settings_t* settings,
handle_midi_event_func_t handler,
void* event_handler_data);

FLUIDSYNTH_API void delete_fluid_midi_driver(fluid_midi_driver_t* driver);

FLUIDSYNTH_API fluid_player_t* new_fluid_player(fluid_synth_t* synth);
FLUIDSYNTH_API int delete_fluid_player(fluid_player_t* player);
FLUIDSYNTH_API int fluid_player_add(fluid_player_t* player, char* midifile);
FLUIDSYNTH_API int fluid_player_play(fluid_player_t* player);
FLUIDSYNTH_API int fluid_player_stop(fluid_player_t* player);
FLUIDSYNTH_API int fluid_player_join(fluid_player_t* player);
FLUIDSYNTH_API int fluid_player_set_loop(fluid_player_t* player, int loop);
FLUIDSYNTH_API int fluid_player_set_midi_tempo(fluid_player_t* player, int tempo);
FLUIDSYNTH_API int fluid_player_set_bpm(fluid_player_t* player, int bpm);

#endif

#ifndef _FLUIDSYNTH_SEQ_H
#define _FLUIDSYNTH_SEQ_H

typedef void(*fluid_event_callback_t)(unsigned int time, fluid_event_t* event,
	fluid_sequencer_t* seq, void* data);

FLUIDSYNTH_API fluid_sequencer_t* new_fluid_sequencer(void);

FLUIDSYNTH_API void delete_fluid_sequencer(fluid_sequencer_t* seq);

FLUIDSYNTH_API short fluid_sequencer_register_client(fluid_sequencer_t* seq, char* name,
fluid_event_callback_t callback, void* data);

FLUIDSYNTH_API void fluid_sequencer_unregister_client(fluid_sequencer_t* seq, short id);

FLUIDSYNTH_API int fluid_sequencer_count_clients(fluid_sequencer_t* seq);

FLUIDSYNTH_API short fluid_sequencer_get_client_id(fluid_sequencer_t* seq, int index);

FLUIDSYNTH_API char* fluid_sequencer_get_client_name(fluid_sequencer_t* seq, int id);

FLUIDSYNTH_API int fluid_sequencer_client_is_dest(fluid_sequencer_t* seq, int id);

FLUIDSYNTH_API void fluid_sequencer_send_now(fluid_sequencer_t* seq, fluid_event_t* evt);

FLUIDSYNTH_API int fluid_sequencer_send_at(fluid_sequencer_t* seq, fluid_event_t* evt,
unsigned int time, int absolute);

FLUIDSYNTH_API void fluid_sequencer_remove_events(fluid_sequencer_t* seq, short source, short dest, int type);

FLUIDSYNTH_API unsigned int fluid_sequencer_get_tick(fluid_sequencer_t* seq);

FLUIDSYNTH_API void fluid_sequencer_set_time_scale(fluid_sequencer_t* seq, double scale);

FLUIDSYNTH_API double fluid_sequencer_get_time_scale(fluid_sequencer_t* seq);

#define FLUID_SEQ_WITH_TRACE 0

#if FLUID_SEQ_WITH_TRACE
FLUIDSYNTH_API char * fluid_seq_gettrace(fluid_sequencer_t* seq);
FLUIDSYNTH_API void fluid_seq_cleartrace(fluid_sequencer_t* seq);
#endif

#endif

#ifndef _FLUIDSYNTH_SEQBIND_H
#define _FLUIDSYNTH_SEQBIND_H

FLUIDSYNTH_API short fluid_sequencer_register_fluidsynth(fluid_sequencer_t* seq, fluid_synth_t* synth);

#endif

#ifndef _FLUIDSYNTH_LOG_H
#define _FLUIDSYNTH_LOG_H

enum fluid_log_level {
	FLUID_PANIC,
	FLUID_ERR,
	FLUID_WARN,
	FLUID_INFO,
	FLUID_DBG,
	LAST_LOG_LEVEL
};

typedef void(*fluid_log_function_t)(int level, char* message, void* data);

FLUIDSYNTH_API fluid_log_function_t fluid_set_log_function(int level, fluid_log_function_t fun, void* data);

FLUIDSYNTH_API void fluid_default_log_function(int level, char* message, void* data);

FLUIDSYNTH_API int fluid_log(int level, const char * fmt, ...);

#endif

#ifndef _FLUIDSYNTH_MISC_H
#define _FLUIDSYNTH_MISC_H

FLUIDSYNTH_API int fluid_is_soundfont(char* filename);

FLUIDSYNTH_API int fluid_is_midifile(char* filename);

#ifdef WIN32

FLUIDSYNTH_API void* fluid_get_hinstance(void);
FLUIDSYNTH_API void fluid_set_hinstance(void* hinstance);
#endif

#endif

#ifndef _FLUIDSYNTH_MOD_H
#define _FLUIDSYNTH_MOD_H

#define FLUID_NUM_MOD           64

struct _fluid_mod_t
{
	unsigned char dest;
	unsigned char src1;
	unsigned char flags1;
	unsigned char src2;
	unsigned char flags2;
	double amount;

	fluid_mod_t * next;
};

enum fluid_mod_flags
{
	FLUID_MOD_POSITIVE = 0,
	FLUID_MOD_NEGATIVE = 1,
	FLUID_MOD_UNIPOLAR = 0,
	FLUID_MOD_BIPOLAR = 2,
	FLUID_MOD_LINEAR = 0,
	FLUID_MOD_CONCAVE = 4,
	FLUID_MOD_CONVEX = 8,
	FLUID_MOD_SWITCH = 12,
	FLUID_MOD_GC = 0,
	FLUID_MOD_CC = 16
};

enum fluid_mod_src
{
	FLUID_MOD_NONE = 0,
	FLUID_MOD_VELOCITY = 2,
	FLUID_MOD_KEY = 3,
	FLUID_MOD_KEYPRESSURE = 10,
	FLUID_MOD_CHANNELPRESSURE = 13,
	FLUID_MOD_PITCHWHEEL = 14,
	FLUID_MOD_PITCHWHEELSENS = 16
};

FLUIDSYNTH_API fluid_mod_t * fluid_mod_new(void);

FLUIDSYNTH_API void fluid_mod_delete(fluid_mod_t * mod);

FLUIDSYNTH_API void fluid_mod_set_source1(fluid_mod_t* mod, int src, int flags);
FLUIDSYNTH_API void fluid_mod_set_source2(fluid_mod_t* mod, int src, int flags);
FLUIDSYNTH_API void fluid_mod_set_dest(fluid_mod_t* mod, int dst);
FLUIDSYNTH_API void fluid_mod_set_amount(fluid_mod_t* mod, double amount);

FLUIDSYNTH_API int fluid_mod_get_source1(fluid_mod_t* mod);
FLUIDSYNTH_API int fluid_mod_get_flags1(fluid_mod_t* mod);
FLUIDSYNTH_API int fluid_mod_get_source2(fluid_mod_t* mod);
FLUIDSYNTH_API int fluid_mod_get_flags2(fluid_mod_t* mod);
FLUIDSYNTH_API int fluid_mod_get_dest(fluid_mod_t* mod);
FLUIDSYNTH_API double fluid_mod_get_amount(fluid_mod_t* mod);

FLUIDSYNTH_API int fluid_mod_test_identity(fluid_mod_t * mod1, fluid_mod_t * mod2);

#endif

#ifndef _FLUIDSYNTH_GEN_H
#define _FLUIDSYNTH_GEN_H

enum fluid_gen_type {
	GEN_STARTADDROFS,
	GEN_ENDADDROFS,
	GEN_STARTLOOPADDROFS,
	GEN_ENDLOOPADDROFS,
	GEN_STARTADDRCOARSEOFS,
	GEN_MODLFOTOPITCH,
	GEN_VIBLFOTOPITCH,
	GEN_MODENVTOPITCH,
	GEN_FILTERFC,
	GEN_FILTERQ,
	GEN_MODLFOTOFILTERFC,
	GEN_MODENVTOFILTERFC,
	GEN_ENDADDRCOARSEOFS,
	GEN_MODLFOTOVOL,
	GEN_UNUSED1,
	GEN_CHORUSSEND,
	GEN_REVERBSEND,
	GEN_PAN,
	GEN_UNUSED2,
	GEN_UNUSED3,
	GEN_UNUSED4,
	GEN_MODLFODELAY,
	GEN_MODLFOFREQ,
	GEN_VIBLFODELAY,
	GEN_VIBLFOFREQ,
	GEN_MODENVDELAY,
	GEN_MODENVATTACK,
	GEN_MODENVHOLD,
	GEN_MODENVDECAY,
	GEN_MODENVSUSTAIN,
	GEN_MODENVRELEASE,
	GEN_KEYTOMODENVHOLD,
	GEN_KEYTOMODENVDECAY,
	GEN_VOLENVDELAY,
	GEN_VOLENVATTACK,
	GEN_VOLENVHOLD,
	GEN_VOLENVDECAY,
	GEN_VOLENVSUSTAIN,
	GEN_VOLENVRELEASE,
	GEN_KEYTOVOLENVHOLD,
	GEN_KEYTOVOLENVDECAY,
	GEN_INSTRUMENT,
	GEN_RESERVED1,
	GEN_KEYRANGE,
	GEN_VELRANGE,
	GEN_STARTLOOPADDRCOARSEOFS,
	GEN_KEYNUM,
	GEN_VELOCITY,
	GEN_ATTENUATION,
	GEN_RESERVED2,
	GEN_ENDLOOPADDRCOARSEOFS,
	GEN_COARSETUNE,
	GEN_FINETUNE,
	GEN_SAMPLEID,
	GEN_SAMPLEMODE,
	GEN_RESERVED3,
	GEN_SCALETUNE,
	GEN_EXCLUSIVECLASS,
	GEN_OVERRIDEROOTKEY,

	GEN_PITCH,
	GEN_LAST
};

typedef struct _fluid_gen_t
{
	unsigned char flags;
	double val;
	double mod;
	double nrpn;
} fluid_gen_t;

enum fluid_gen_flags
{
	GEN_UNUSED,
	GEN_SET,
	GEN_ABS_NRPN
};

FLUIDSYNTH_API int fluid_gen_set_default_values(fluid_gen_t* gen);

#endif

#ifndef _FLUIDSYNTH_VOICE_H
#define _FLUIDSYNTH_VOICE_H

FLUIDSYNTH_API void fluid_voice_update_param(fluid_voice_t* voice, int gen);

enum fluid_voice_add_mod{
	FLUID_VOICE_OVERWRITE,
	FLUID_VOICE_ADD,
	FLUID_VOICE_DEFAULT
};

FLUIDSYNTH_API void fluid_voice_add_mod(fluid_voice_t* voice, fluid_mod_t* mod, int mode);

FLUIDSYNTH_API void fluid_voice_gen_set(fluid_voice_t* voice, int gen, float val);

FLUIDSYNTH_API float fluid_voice_gen_get(fluid_voice_t* voice, int gen);

FLUIDSYNTH_API void fluid_voice_gen_incr(fluid_voice_t* voice, int gen, float val);

FLUIDSYNTH_API unsigned int fluid_voice_get_id(fluid_voice_t* voice);

FLUIDSYNTH_API int fluid_voice_is_playing(fluid_voice_t* voice);

FLUIDSYNTH_API int fluid_voice_optimize_sample(fluid_sample_t* s);

#endif

#ifndef _FLUIDSYNTH_VERSION_H
#define _FLUIDSYNTH_VERSION_H

#define FLUIDSYNTH_VERSION       "1.0.9"
#define FLUIDSYNTH_VERSION_MAJOR 1
#define FLUIDSYNTH_VERSION_MINOR 0
#define FLUIDSYNTH_VERSION_MICRO 9

FLUIDSYNTH_API void fluid_version(int *major, int *minor, int *micro);

FLUIDSYNTH_API char* fluid_version_str(void);

#endif

#endif

//
//MERGED FILE END: fluidsynth.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: fluid_synth.c
//

#ifndef _FLUID_SYNTH_H
#define _FLUID_SYNTH_H

#ifndef _FLUID_LIST_H
#define _FLUID_LIST_H

typedef struct _fluid_list_t fluid_list_t;

typedef int(*fluid_compare_func_t)(void* a, void* b);

struct _fluid_list_t
{
	void* data;
	fluid_list_t *next;
};

fluid_list_t* new_fluid_list(void);
void delete_fluid_list(fluid_list_t *list);
void delete1_fluid_list(fluid_list_t *list);
fluid_list_t* fluid_list_sort(fluid_list_t *list, fluid_compare_func_t compare_func);
fluid_list_t* fluid_list_append(fluid_list_t *list, void* data);
fluid_list_t* fluid_list_prepend(fluid_list_t *list, void* data);
fluid_list_t* fluid_list_remove(fluid_list_t *list, void* data);
fluid_list_t* fluid_list_remove_link(fluid_list_t *list, fluid_list_t *llink);
fluid_list_t* fluid_list_nth(fluid_list_t *list, int n);
fluid_list_t* fluid_list_last(fluid_list_t *list);
fluid_list_t* fluid_list_insert_at(fluid_list_t *list, int n, void* data);
int fluid_list_size(fluid_list_t *list);

#define fluid_list_next(slist)	((slist) ? (((fluid_list_t *)(slist))->next) : NULL)
#define fluid_list_get(slist)	((slist) ? ((slist)->data) : NULL)

#endif

#ifndef _FLUID_REV_H
#define _FLUID_REV_H

typedef struct _fluid_revmodel_t fluid_revmodel_t;

fluid_revmodel_t* new_fluid_revmodel(void);
void delete_fluid_revmodel(fluid_revmodel_t* rev);

void fluid_revmodel_processmix(fluid_revmodel_t* rev, fluid_real_t *in,
	fluid_real_t *left_out, fluid_real_t *right_out);

void fluid_revmodel_processreplace(fluid_revmodel_t* rev, fluid_real_t *in,
	fluid_real_t *left_out, fluid_real_t *right_out);

void fluid_revmodel_reset(fluid_revmodel_t* rev);

void fluid_revmodel_setroomsize(fluid_revmodel_t* rev, fluid_real_t value);
void fluid_revmodel_setdamp(fluid_revmodel_t* rev, fluid_real_t value);
void fluid_revmodel_setlevel(fluid_revmodel_t* rev, fluid_real_t value);
void fluid_revmodel_setwidth(fluid_revmodel_t* rev, fluid_real_t value);
void fluid_revmodel_setmode(fluid_revmodel_t* rev, fluid_real_t value);

fluid_real_t fluid_revmodel_getroomsize(fluid_revmodel_t* rev);
fluid_real_t fluid_revmodel_getdamp(fluid_revmodel_t* rev);
fluid_real_t fluid_revmodel_getlevel(fluid_revmodel_t* rev);
fluid_real_t fluid_revmodel_getwidth(fluid_revmodel_t* rev);

typedef struct _fluid_revmodel_presets_t {
	char* name;
	fluid_real_t roomsize;
	fluid_real_t damp;
	fluid_real_t width;
	fluid_real_t level;
} fluid_revmodel_presets_t;

#endif

#ifndef _FLUID_VOICE_H
#define _FLUID_VOICE_H

#ifndef _FLUID_PHASE_H
#define _FLUID_PHASE_H

#ifndef _WIN32
typedef long long signed64;
typedef unsigned long long unsigned64;
#else
typedef __int64 signed64;
typedef unsigned __int64 unsigned64;
#endif

#define FLUID_INTERP_BITS        8
#define FLUID_INTERP_BITS_MASK   0xff000000
#define FLUID_INTERP_BITS_SHIFT  24
#define FLUID_INTERP_MAX         256

#define FLUID_FRACT_MAX ((double)4294967296.0)

typedef unsigned64 fluid_phase_t;

#define fluid_phase_set(a,b) a=b;

#define fluid_phase_set_int(a, b)    ((a) = ((unsigned64)(b)) << 32)

#define fluid_phase_set_float(a, b) \
  (a) = (((unsigned64)(b)) << 32) \
  | (uint32) (((double)(b) - (int)(b)) * (double)FLUID_FRACT_MAX)

#define fluid_phase_from_index_fract(index, fract) \
  ((((unsigned64)(index)) << 32) + (fract))

#define fluid_phase_index(_x) \
  ((unsigned int)((_x) >> 32))
#define fluid_phase_fract(_x) \
  ((uint32)((_x) & 0xFFFFFFFF))

#define fluid_phase_index_round(_x) \
  ((unsigned int)(((_x) + 0x80000000) >> 32))

#define fluid_phase_fract_to_tablerow(_x) \
  ((unsigned int)(fluid_phase_fract(_x) & FLUID_INTERP_BITS_MASK) >> FLUID_INTERP_BITS_SHIFT)

#define fluid_phase_double(_x) \
  ((double)(fluid_phase_index(_x)) + ((double)fluid_phase_fract(_x) / FLUID_FRACT_MAX))

#define fluid_phase_incr(a, b)  a += b

#define fluid_phase_decr(a, b)  a -= b

#define fluid_phase_sub_int(a, b)  ((a) -= (unsigned64)(b) << 32)

#define fluid_phase_index_plusplus(a)  (((a) += 0x100000000LL)

#endif

#ifndef _FLUID_GEN_H
#define _FLUID_GEN_H

typedef struct _fluid_gen_info_t {
	char num;
	char init;
	char nrpn_scale;
	float min;
	float max;
	float def;
} fluid_gen_info_t;

#define fluid_gen_set_mod(_gen, _val)  { (_gen)->mod = (double) (_val); }
#define fluid_gen_set_nrpn(_gen, _val) { (_gen)->nrpn = (double) (_val); }

fluid_real_t fluid_gen_scale(int gen, float value);
fluid_real_t fluid_gen_scale_nrpn(int gen, int nrpn);
int fluid_gen_init(fluid_gen_t* gen, fluid_channel_t* channel);

#endif

#ifndef _FLUID_MOD_H
#define _FLUID_MOD_H

#ifndef _FLUID_CONV_H
#define _FLUID_CONV_H

#define FLUID_CENTS_HZ_SIZE     1200
#define FLUID_VEL_CB_SIZE       128
#define FLUID_CB_AMP_SIZE       961
#define FLUID_ATTEN_AMP_SIZE    1441
#define FLUID_PAN_SIZE          1002

#define FLUID_ATTEN_POWER_FACTOR  (-200.0)

void fluid_conversion_config(void);

fluid_real_t fluid_ct2hz_real(fluid_real_t cents);
fluid_real_t fluid_ct2hz(fluid_real_t cents);
fluid_real_t fluid_cb2amp(fluid_real_t cb);
fluid_real_t fluid_atten2amp(fluid_real_t atten);
fluid_real_t fluid_tc2sec(fluid_real_t tc);
fluid_real_t fluid_tc2sec_delay(fluid_real_t tc);
fluid_real_t fluid_tc2sec_attack(fluid_real_t tc);
fluid_real_t fluid_tc2sec_release(fluid_real_t tc);
fluid_real_t fluid_act2hz(fluid_real_t c);
fluid_real_t fluid_hz2ct(fluid_real_t c);
fluid_real_t fluid_pan(fluid_real_t c, int left);
fluid_real_t fluid_concave(fluid_real_t val);
fluid_real_t fluid_convex(fluid_real_t val);

#endif

void fluid_mod_clone(fluid_mod_t* mod, fluid_mod_t* src);
fluid_real_t fluid_mod_get_value(fluid_mod_t* mod, fluid_channel_t* chan, fluid_voice_t* voice);
void fluid_dump_modulator(fluid_mod_t * mod);

#define fluid_mod_has_source(mod,cc,ctrl)  \
( ((((mod)->src1 == ctrl) && (((mod)->flags1 & FLUID_MOD_CC) != 0) && (cc != 0)) \
   || ((((mod)->src1 == ctrl) && (((mod)->flags1 & FLUID_MOD_CC) == 0) && (cc == 0)))) \
|| ((((mod)->src2 == ctrl) && (((mod)->flags2 & FLUID_MOD_CC) != 0) && (cc != 0)) \
    || ((((mod)->src2 == ctrl) && (((mod)->flags2 & FLUID_MOD_CC) == 0) && (cc == 0)))))

#define fluid_mod_has_dest(mod,gen)  ((mod)->dest == gen)

#endif

#define NO_CHANNEL             0xff

enum fluid_voice_status
{
	FLUID_VOICE_CLEAN,
	FLUID_VOICE_ON,
	FLUID_VOICE_SUSTAINED,
	FLUID_VOICE_OFF
};

struct _fluid_env_data_t {
	unsigned int count;
	fluid_real_t coeff;
	fluid_real_t incr;
	fluid_real_t min;
	fluid_real_t max;
};

enum fluid_voice_envelope_index_t{
	FLUID_VOICE_ENVDELAY,
	FLUID_VOICE_ENVATTACK,
	FLUID_VOICE_ENVHOLD,
	FLUID_VOICE_ENVDECAY,
	FLUID_VOICE_ENVSUSTAIN,
	FLUID_VOICE_ENVRELEASE,
	FLUID_VOICE_ENVFINISHED,
	FLUID_VOICE_ENVLAST
};

struct _fluid_voice_t
{
	unsigned int id;
	unsigned char status;
	unsigned char chan;
	unsigned char key;
	unsigned char vel;
	fluid_channel_t* channel;
	fluid_gen_t gen[GEN_LAST];
	fluid_mod_t mod[FLUID_NUM_MOD];
	int mod_count;
	int has_looped;
	fluid_sample_t* sample;
	int check_sample_sanity_flag;
#if 0

	short* sample_data;
	int sample_data_offset;
	int sample_data_length;
	unsigned int sample_start;
	unsigned int sample_end;
	unsigned int sample_loopstart;
	unsigned int sample_loopend;
	unsigned int sample_rate;
	int sample_origpitch;
	int sample_pitchadj;
	int sample_type;
	int(*sample_notify)(fluid_voice_t* voice, int reason);
	void* sample_userdata;
#endif

	fluid_real_t output_rate;

	unsigned int start_time;
	unsigned int ticks;

	fluid_real_t amp;
	fluid_phase_t phase;

	fluid_real_t phase_incr;
	fluid_real_t amp_incr;
	fluid_real_t *dsp_buf;

	fluid_real_t pitch;
	fluid_real_t attenuation;
	fluid_real_t min_attenuation_cB;
	fluid_real_t root_pitch;

	int start;
	int end;
	int loopstart;
	int loopend;

	fluid_real_t synth_gain;

	fluid_env_data_t volenv_data[FLUID_VOICE_ENVLAST];
	unsigned int volenv_count;
	int volenv_section;
	fluid_real_t volenv_val;
	fluid_real_t amplitude_that_reaches_noise_floor_nonloop;
	fluid_real_t amplitude_that_reaches_noise_floor_loop;

	fluid_env_data_t modenv_data[FLUID_VOICE_ENVLAST];
	unsigned int modenv_count;
	int modenv_section;
	fluid_real_t modenv_val;
	fluid_real_t modenv_to_fc;
	fluid_real_t modenv_to_pitch;

	fluid_real_t modlfo_val;
	unsigned int modlfo_delay;
	fluid_real_t modlfo_incr;
	fluid_real_t modlfo_to_fc;
	fluid_real_t modlfo_to_pitch;
	fluid_real_t modlfo_to_vol;

	fluid_real_t viblfo_val;
	unsigned int viblfo_delay;
	fluid_real_t viblfo_incr;
	fluid_real_t viblfo_to_pitch;

	fluid_real_t fres;
	fluid_real_t last_fres;

	fluid_real_t q_lin;
	fluid_real_t filter_gain;
	fluid_real_t hist1, hist2;
	int filter_startup;

	fluid_real_t b02;
	fluid_real_t b1;
	fluid_real_t a1;
	fluid_real_t a2;

	fluid_real_t b02_incr;
	fluid_real_t b1_incr;
	fluid_real_t a1_incr;
	fluid_real_t a2_incr;
	int filter_coeff_incr_count;

	fluid_real_t pan;
	fluid_real_t amp_left;
	fluid_real_t amp_right;

	fluid_real_t reverb_send;
	fluid_real_t amp_reverb;

	fluid_real_t chorus_send;
	fluid_real_t amp_chorus;

	int interp_method;

	int debug;
	double ref;
};

fluid_voice_t* new_fluid_voice(fluid_real_t output_rate);
int delete_fluid_voice(fluid_voice_t* voice);

void fluid_voice_start(fluid_voice_t* voice);

int fluid_voice_write(fluid_voice_t* voice,
	fluid_real_t* left, fluid_real_t* right,
	fluid_real_t* reverb_buf, fluid_real_t* chorus_buf);

int fluid_voice_init(fluid_voice_t* voice, fluid_sample_t* sample,
	fluid_channel_t* channel, int key, int vel,
	unsigned int id, unsigned int time, fluid_real_t gain);

int fluid_voice_modulate(fluid_voice_t* voice, int cc, int ctrl);
int fluid_voice_modulate_all(fluid_voice_t* voice);

int fluid_voice_set_param(fluid_voice_t* voice, int gen, fluid_real_t value, int abs);

int fluid_voice_set_gain(fluid_voice_t* voice, fluid_real_t gain);

void fluid_voice_update_param(fluid_voice_t* voice, int gen);

int fluid_voice_noteoff(fluid_voice_t* voice);
int fluid_voice_off(fluid_voice_t* voice);
int fluid_voice_calculate_runtime_synthesis_parameters(fluid_voice_t* voice);
fluid_channel_t* fluid_voice_get_channel(fluid_voice_t* voice);
int calculate_hold_decay_buffers(fluid_voice_t* voice, int gen_base,
	int gen_key2base, int is_decay);
int fluid_voice_kill_excl(fluid_voice_t* voice);
fluid_real_t fluid_voice_get_lower_boundary_for_attenuation(fluid_voice_t* voice);
fluid_real_t fluid_voice_determine_amplitude_that_reaches_noise_floor_for_sample(fluid_voice_t* voice);
void fluid_voice_check_sample_sanity(fluid_voice_t* voice);

#define fluid_voice_set_id(_voice, _id)  { (_voice)->id = (_id); }
#define fluid_voice_get_chan(_voice)     (_voice)->chan

#define _PLAYING(voice)  (((voice)->status == FLUID_VOICE_ON) || ((voice)->status == FLUID_VOICE_SUSTAINED))

#define _ON(voice)  ((voice)->status == FLUID_VOICE_ON && (voice)->volenv_section < FLUID_VOICE_ENVRELEASE)
#define _SUSTAINED(voice)  ((voice)->status == FLUID_VOICE_SUSTAINED)
#define _AVAILABLE(voice)  (((voice)->status == FLUID_VOICE_CLEAN) || ((voice)->status == FLUID_VOICE_OFF))
#define _RELEASED(voice)  ((voice)->chan == NO_CHANNEL)
#define _SAMPLEMODE(voice) ((int)(voice)->gen[GEN_SAMPLEMODE].val)

fluid_real_t fluid_voice_gen_value(fluid_voice_t* voice, int num);

#define _GEN(_voice, _n) \
  ((fluid_real_t)(_voice)->gen[_n].val \
   + (fluid_real_t)(_voice)->gen[_n].mod \
   + (fluid_real_t)(_voice)->gen[_n].nrpn)

#define FLUID_SAMPLESANITY_CHECK (1 << 0)
#define FLUID_SAMPLESANITY_STARTUP (1 << 1)

void fluid_dsp_float_config(void);
int fluid_dsp_float_interpolate_none(fluid_voice_t *voice);
int fluid_dsp_float_interpolate_linear(fluid_voice_t *voice);
int fluid_dsp_float_interpolate_4th_order(fluid_voice_t *voice);
int fluid_dsp_float_interpolate_7th_order(fluid_voice_t *voice);

#endif

#ifndef _FLUID_CHORUS_H
#define _FLUID_CHORUS_H

typedef struct _fluid_chorus_t fluid_chorus_t;

fluid_chorus_t* new_fluid_chorus(fluid_real_t sample_rate);
void delete_fluid_chorus(fluid_chorus_t* chorus);
void fluid_chorus_processmix(fluid_chorus_t* chorus, fluid_real_t *in,
	fluid_real_t *left_out, fluid_real_t *right_out);
void fluid_chorus_processreplace(fluid_chorus_t* chorus, fluid_real_t *in,
	fluid_real_t *left_out, fluid_real_t *right_out);

int fluid_chorus_init(fluid_chorus_t* chorus);
void fluid_chorus_reset(fluid_chorus_t* chorus);

void fluid_chorus_set_nr(fluid_chorus_t* chorus, int nr);
void fluid_chorus_set_level(fluid_chorus_t* chorus, fluid_real_t level);
void fluid_chorus_set_speed_Hz(fluid_chorus_t* chorus, fluid_real_t speed_Hz);
void fluid_chorus_set_depth_ms(fluid_chorus_t* chorus, fluid_real_t depth_ms);
void fluid_chorus_set_type(fluid_chorus_t* chorus, int type);
int fluid_chorus_update(fluid_chorus_t* chorus);
int fluid_chorus_get_nr(fluid_chorus_t* chorus);
fluid_real_t fluid_chorus_get_level(fluid_chorus_t* chorus);
fluid_real_t fluid_chorus_get_speed_Hz(fluid_chorus_t* chorus);
fluid_real_t fluid_chorus_get_depth_ms(fluid_chorus_t* chorus);
int fluid_chorus_get_type(fluid_chorus_t* chorus);

#endif

#ifndef _FLUID_MIDIROUTER_H
#define _FLUID_MIDIROUTER_H

#ifndef _FLUID_MIDI_H
#define _FLUID_MIDI_H

#ifndef _FLUID_SYS_H
#define _FLUID_SYS_H

void fluid_sys_config(void);
void fluid_log_config(void);
void fluid_time_config(void);

char *fluid_strtok(char **str, char *delim);

#if DEBUG

enum fluid_debug_level {
	FLUID_DBG_DRIVER = 1
};

int fluid_debug(int level, char * fmt, ...);

#else
#define fluid_debug
#endif

#if defined(WIN32)
#define fluid_curtime()   GetTickCount()

double fluid_utime(void);

#elif defined(MACOS9)
#include <OSUtils.h>
#include <Timer.h>

unsigned int fluid_curtime();
#define fluid_utime()  0.0

#elif defined(__OS2__)
#define INCL_DOS
#include <os2.h>

typedef int socklen_t;

unsigned int fluid_curtime(void);
double fluid_utime(void);

#else

unsigned int fluid_curtime(void);
double fluid_utime(void);

#endif

typedef int(*fluid_timer_callback_t)(void* data, unsigned int msec);

typedef struct _fluid_timer_t fluid_timer_t;

fluid_timer_t* new_fluid_timer(int msec, fluid_timer_callback_t callback,
	void* data, int new_thread, int auto_destroy);

int delete_fluid_timer(fluid_timer_t* timer);
int fluid_timer_join(fluid_timer_t* timer);
int fluid_timer_stop(fluid_timer_t* timer);

#if defined(MACOS9)
typedef int fluid_mutex_t;
#define fluid_mutex_init(_m)      { (_m) = 0; }
#define fluid_mutex_destroy(_m)
#define fluid_mutex_lock(_m)
#define fluid_mutex_unlock(_m)

#elif defined(WINAPI_FAMILY) && WINAPI_FAMILY==WINAPI_FAMILY_PHONE_APP
typedef HANDLE fluid_mutex_t;
#define fluid_mutex_init(_m)      { (_m) = CreateMutexEx(NULL, NULL, 0, SYNCHRONIZE); }
#define fluid_mutex_destroy(_m)   if (_m) { CloseHandle(_m); }
#define fluid_mutex_lock(_m)      WaitForSingleObjectEx(_m, INFINITE, FALSE)
#define fluid_mutex_unlock(_m)    ReleaseMutex(_m)
#define GetTickCount() rand()
#define CreateThread(a, b, c, d, e, f) NULL
#define SetThreadPriority(a, b) 1
#define Sleep(a) 1
#define ExitThread(a) 0
#define WaitForSingleObject(a, b) WAIT_OBJECT_0

#elif defined(WIN32)
typedef HANDLE fluid_mutex_t;
#define fluid_mutex_init(_m)      { (_m) = CreateMutex(NULL, 0, NULL); }
#define fluid_mutex_destroy(_m)   if (_m) { CloseHandle(_m); }
#define fluid_mutex_lock(_m)      WaitForSingleObject(_m, INFINITE)
#define fluid_mutex_unlock(_m)    ReleaseMutex(_m)

#elif defined(__OS2__)
typedef HMTX fluid_mutex_t;
#define fluid_mutex_init(_m)      { (_m) = 0; DosCreateMutexSem( NULL, &(_m), 0, FALSE ); }
#define fluid_mutex_destroy(_m)   if (_m) { DosCloseMutexSem(_m); }
#define fluid_mutex_lock(_m)      DosRequestMutexSem(_m, -1L)
#define fluid_mutex_unlock(_m)    DosReleaseMutexSem(_m)

#elif defined(__wasm__) || defined(__EMSCRIPTEN__)
typedef pthread_mutex_t fluid_mutex_t;
#define fluid_mutex_init(_m)
#define fluid_mutex_destroy(_m)
#define fluid_mutex_lock(_m)
#define fluid_mutex_unlock(_m)

#else
typedef pthread_mutex_t fluid_mutex_t;
#define fluid_mutex_init(_m)      pthread_mutex_init(&(_m), NULL)
#define fluid_mutex_destroy(_m)   pthread_mutex_destroy(&(_m))
#define fluid_mutex_lock(_m)      pthread_mutex_lock(&(_m))
#define fluid_mutex_unlock(_m)    pthread_mutex_unlock(&(_m))
#endif

typedef struct _fluid_thread_t fluid_thread_t;
typedef void(*fluid_thread_func_t)(void* data);

fluid_thread_t* new_fluid_thread(fluid_thread_func_t func, void* data, int detach);
int delete_fluid_thread(fluid_thread_t* thread);
int fluid_thread_join(fluid_thread_t* thread);

typedef int(*fluid_server_func_t)(void* data, fluid_socket_t client_socket, char* addr);

fluid_server_socket_t* new_fluid_server_socket(int port, fluid_server_func_t func, void* data);
int delete_fluid_server_socket(fluid_server_socket_t* sock);
int fluid_server_socket_join(fluid_server_socket_t* sock);

fluid_socket_t new_fluid_client_socket(char* host, int port);

void delete_fluid_client_socket(fluid_socket_t sock);

void fluid_socket_close(fluid_socket_t sock);
fluid_istream_t fluid_socket_get_istream(fluid_socket_t sock);
fluid_ostream_t fluid_socket_get_ostream(fluid_socket_t sock);

enum {
	FLUID_PROF_WRITE_S16,
	FLUID_PROF_ONE_BLOCK,
	FLUID_PROF_ONE_BLOCK_CLEAR,
	FLUID_PROF_ONE_BLOCK_VOICE,
	FLUID_PROF_ONE_BLOCK_VOICES,
	FLUID_PROF_ONE_BLOCK_REVERB,
	FLUID_PROF_ONE_BLOCK_CHORUS,
	FLUID_PROF_VOICE_NOTE,
	FLUID_PROF_VOICE_RELEASE,
	FLUID_PROF_LAST
};

#if WITH_PROFILING

void fluid_profiling_print(void);

typedef struct _fluid_profile_data_t {
	int num;
	char* description;
	double min, max, total;
	unsigned int count;
} fluid_profile_data_t;

static fluid_profile_data_t fluid_profile_data[];

#define fluid_profile_ref() fluid_utime()

#define fluid_profile(_num,_ref) { \
  double _now = fluid_utime(); \
  double _delta = _now - _ref; \
  fluid_profile_data[_num].min = _delta < fluid_profile_data[_num].min ? _delta : fluid_profile_data[_num].min; \
  fluid_profile_data[_num].max = _delta > fluid_profile_data[_num].max ? _delta : fluid_profile_data[_num].max; \
  fluid_profile_data[_num].total += _delta; \
  fluid_profile_data[_num].count++; \
  _ref = _now; \
	}

#else

#define fluid_profiling_print()
#define fluid_profile_ref()  0
#define fluid_profile(_num,_ref) ((void)(_ref))

#endif

#if defined(HAVE_SYS_MMAN_H) && !defined(__OS2__)
#define fluid_mlock(_p,_n)      mlock(_p, _n)
#define fluid_munlock(_p,_n)    munlock(_p,_n)
#else
#define fluid_mlock(_p,_n)      0
#define fluid_munlock(_p,_n)
#endif

#ifdef FPE_CHECK
#define fluid_check_fpe(expl) fluid_check_fpe_i386(expl)
#define fluid_clear_fpe() fluid_clear_fpe_i386()
#else
#define fluid_check_fpe(expl)
#define fluid_clear_fpe()
#endif

unsigned int fluid_check_fpe_i386(char * explanation_in_case_of_fpe);
void fluid_clear_fpe_i386(void);

#endif

typedef struct _fluid_midi_parser_t fluid_midi_parser_t;

fluid_midi_parser_t* new_fluid_midi_parser(void);
int delete_fluid_midi_parser(fluid_midi_parser_t* parser);
fluid_midi_event_t* fluid_midi_parser_parse(fluid_midi_parser_t* parser, unsigned char c);

int fluid_midi_send_event(fluid_synth_t* synth, fluid_player_t* player, fluid_midi_event_t* evt);

#define MAX_NUMBER_OF_TRACKS 128

enum fluid_midi_event_type {
	NOTE_OFF = 0x80,
	NOTE_ON = 0x90,
	KEY_PRESSURE = 0xa0,
	CONTROL_CHANGE = 0xb0,
	PROGRAM_CHANGE = 0xc0,
	CHANNEL_PRESSURE = 0xd0,
	PITCH_BEND = 0xe0,

	MIDI_SYSEX = 0xf0,

	MIDI_TIME_CODE = 0xf1,
	MIDI_SONG_POSITION = 0xf2,
	MIDI_SONG_SELECT = 0xf3,
	MIDI_TUNE_REQUEST = 0xf6,
	MIDI_EOX = 0xf7,

	MIDI_SYNC = 0xf8,
	MIDI_TICK = 0xf9,
	MIDI_START = 0xfa,
	MIDI_CONTINUE = 0xfb,
	MIDI_STOP = 0xfc,
	MIDI_ACTIVE_SENSING = 0xfe,
	MIDI_SYSTEM_RESET = 0xff,

	MIDI_META_EVENT = 0xff
};

enum fluid_midi_control_change {
	BANK_SELECT_MSB = 0x00,
	MODULATION_MSB = 0x01,
	BREATH_MSB = 0x02,
	FOOT_MSB = 0x04,
	PORTAMENTO_TIME_MSB = 0x05,
	DATA_ENTRY_MSB = 0x06,
	VOLUME_MSB = 0x07,
	BALANCE_MSB = 0x08,
	PAN_MSB = 0x0A,
	EXPRESSION_MSB = 0x0B,
	EFFECTS1_MSB = 0x0C,
	EFFECTS2_MSB = 0x0D,
	GPC1_MSB = 0x10,
	GPC2_MSB = 0x11,
	GPC3_MSB = 0x12,
	GPC4_MSB = 0x13,
	BANK_SELECT_LSB = 0x20,
	MODULATION_WHEEL_LSB = 0x21,
	BREATH_LSB = 0x22,
	FOOT_LSB = 0x24,
	PORTAMENTO_TIME_LSB = 0x25,
	DATA_ENTRY_LSB = 0x26,
	VOLUME_LSB = 0x27,
	BALANCE_LSB = 0x28,
	PAN_LSB = 0x2A,
	EXPRESSION_LSB = 0x2B,
	EFFECTS1_LSB = 0x2C,
	EFFECTS2_LSB = 0x2D,
	GPC1_LSB = 0x30,
	GPC2_LSB = 0x31,
	GPC3_LSB = 0x32,
	GPC4_LSB = 0x33,
	SUSTAIN_SWITCH = 0x40,
	PORTAMENTO_SWITCH = 0x41,
	SOSTENUTO_SWITCH = 0x42,
	SOFT_PEDAL_SWITCH = 0x43,
	LEGATO_SWITCH = 0x45,
	HOLD2_SWITCH = 0x45,
	SOUND_CTRL1 = 0x46,
	SOUND_CTRL2 = 0x47,
	SOUND_CTRL3 = 0x48,
	SOUND_CTRL4 = 0x49,
	SOUND_CTRL5 = 0x4A,
	SOUND_CTRL6 = 0x4B,
	SOUND_CTRL7 = 0x4C,
	SOUND_CTRL8 = 0x4D,
	SOUND_CTRL9 = 0x4E,
	SOUND_CTRL10 = 0x4F,
	GPC5 = 0x50,
	GPC6 = 0x51,
	GPC7 = 0x52,
	GPC8 = 0x53,
	PORTAMENTO_CTRL = 0x54,
	EFFECTS_DEPTH1 = 0x5B,
	EFFECTS_DEPTH2 = 0x5C,
	EFFECTS_DEPTH3 = 0x5D,
	EFFECTS_DEPTH4 = 0x5E,
	EFFECTS_DEPTH5 = 0x5F,
	DATA_ENTRY_INCR = 0x60,
	DATA_ENTRY_DECR = 0x61,
	NRPN_LSB = 0x62,
	NRPN_MSB = 0x63,
	RPN_LSB = 0x64,
	RPN_MSB = 0x65,
	ALL_SOUND_OFF = 0x78,
	ALL_CTRL_OFF = 0x79,
	LOCAL_CONTROL = 0x7A,
	ALL_NOTES_OFF = 0x7B,
	OMNI_OFF = 0x7C,
	OMNI_ON = 0x7D,
	POLY_OFF = 0x7E,
	POLY_ON = 0x7F
};

enum midi_rpn_event {
	RPN_PITCH_BEND_RANGE = 0x00,
	RPN_CHANNEL_FINE_TUNE = 0x01,
	RPN_CHANNEL_COARSE_TUNE = 0x02,
	RPN_TUNING_PROGRAM_CHANGE = 0x03,
	RPN_TUNING_BANK_SELECT = 0x04,
	RPN_MODULATION_DEPTH_RANGE = 0x05
};

enum midi_meta_event {
	MIDI_COPYRIGHT = 0x02,
	MIDI_TRACK_NAME = 0x03,
	MIDI_INST_NAME = 0x04,
	MIDI_LYRIC = 0x05,
	MIDI_MARKER = 0x06,
	MIDI_CUE_POINT = 0x07,
	MIDI_EOT = 0x2f,
	MIDI_SET_TEMPO = 0x51,
	MIDI_SMPTE_OFFSET = 0x54,
	MIDI_TIME_SIGNATURE = 0x58,
	MIDI_KEY_SIGNATURE = 0x59,
	MIDI_SEQUENCER_EVENT = 0x7f
};

enum fluid_player_status
{
	FLUID_PLAYER_READY,
	FLUID_PLAYER_PLAYING,
	FLUID_PLAYER_DONE
};

enum fluid_driver_status
{
	FLUID_MIDI_READY,
	FLUID_MIDI_LISTENING,
	FLUID_MIDI_DONE
};

#define fluid_isascii(c)    (((c) & ~0x7f) == 0)

struct _fluid_midi_event_t {
	fluid_midi_event_t* next;
	unsigned int dtime;
	unsigned char type;
	unsigned char channel;
	unsigned int param1;
	unsigned int param2;
};

struct _fluid_track_t {
	char* name;
	int num;
	fluid_midi_event_t *first;
	fluid_midi_event_t *cur;
	fluid_midi_event_t *last;
	unsigned int ticks;
};

typedef struct _fluid_track_t fluid_track_t;

fluid_track_t* new_fluid_track(int num);
int delete_fluid_track(fluid_track_t* track);
int fluid_track_set_name(fluid_track_t* track, char* name);
char* fluid_track_get_name(fluid_track_t* track);
int fluid_track_add_event(fluid_track_t* track, fluid_midi_event_t* evt);
fluid_midi_event_t* fluid_track_first_event(fluid_track_t* track);
fluid_midi_event_t* fluid_track_next_event(fluid_track_t* track);
int fluid_track_get_duration(fluid_track_t* track);
int fluid_track_reset(fluid_track_t* track);

int fluid_track_send_events(fluid_track_t* track,
	fluid_synth_t* synth,
	fluid_player_t* player,
	unsigned int ticks);

#define fluid_track_eot(track)  ((track)->cur == NULL)

struct _fluid_player_t {
	int status;
	int loop;
	int ntracks;
	fluid_track_t *track[MAX_NUMBER_OF_TRACKS];
	fluid_synth_t* synth;
	fluid_timer_t* timer;
	fluid_list_t* playlist;
	char* current_file;
	char send_program_change;
	int start_ticks;
	int cur_ticks;
	int begin_msec;
	int start_msec;
	int cur_msec;
	int miditempo;
	double deltatime;
	unsigned int division;
};

int fluid_player_add_track(fluid_player_t* player, fluid_track_t* track);
int fluid_player_callback(void* data, unsigned int msec);
int fluid_player_count_tracks(fluid_player_t* player);
fluid_track_t* fluid_player_get_track(fluid_player_t* player, int i);
int fluid_player_reset(fluid_player_t* player);
int fluid_player_load(fluid_player_t* player, char *filename);

typedef struct {
	fluid_file fp;
	int running_status;
	int c;
	int type;
	int ntracks;
	int uses_smpte;
	unsigned int smpte_fps;
	unsigned int smpte_res;
	unsigned int division;
	double tempo;
	int tracklen;
	int trackpos;
	int eot;
	int varlen;
	int dtime;
} fluid_midi_file;

fluid_midi_file* new_fluid_midi_file(char* filename);
void delete_fluid_midi_file(fluid_midi_file* mf);
int fluid_midi_file_read_mthd(fluid_midi_file* midifile);
int fluid_midi_file_load_tracks(fluid_midi_file* midifile, fluid_player_t* player);
int fluid_midi_file_read_track(fluid_midi_file* mf, fluid_player_t* player, int num);
int fluid_midi_file_read_event(fluid_midi_file* mf, fluid_track_t* track);
int fluid_midi_file_read_varlen(fluid_midi_file* mf);
int fluid_midi_file_getc(fluid_midi_file* mf);
int fluid_midi_file_push(fluid_midi_file* mf, int c);
int fluid_midi_file_read(fluid_midi_file* mf, void* buf, int len);
int fluid_midi_file_skip(fluid_midi_file* mf, int len);
int fluid_midi_file_read_tracklen(fluid_midi_file* mf);
int fluid_midi_file_eot(fluid_midi_file* mf);
int fluid_midi_file_get_division(fluid_midi_file* midifile);
int fluid_midi_event_length(unsigned char status);

#define FLUID_MIDI_PARSER_MAX_PAR 3

struct _fluid_midi_parser_t {
	unsigned char status;
	unsigned char channel;
	unsigned int nr_bytes;
	unsigned int nr_bytes_total;
	unsigned short p[FLUID_MIDI_PARSER_MAX_PAR];
	fluid_midi_event_t event;
};

int fluid_isasciistring(char* s);
long fluid_getlength(unsigned char *s);

int fluid_midi_router_send_event(fluid_midi_router_t* router, fluid_midi_event_t* event);

#endif

void fluid_midi_router_destroy_all_rules(fluid_midi_router_t* router);
fluid_midi_router_rule_t* new_fluid_midi_router_rule(void);

int fluid_midi_router_handle_clear(fluid_synth_t* synth, int ac, char** av, fluid_ostream_t out);
int fluid_midi_router_handle_default(fluid_synth_t* synth, int ac, char** av, fluid_ostream_t out);
int fluid_midi_router_handle_begin(fluid_synth_t* synth, int ac, char** av, fluid_ostream_t out);
int fluid_midi_router_handle_chan(fluid_synth_t* synth, int ac, char** av, fluid_ostream_t out);
int fluid_midi_router_handle_par1(fluid_synth_t* synth, int ac, char** av, fluid_ostream_t out);
int fluid_midi_router_handle_par2(fluid_synth_t* synth, int ac, char** av, fluid_ostream_t out);
int fluid_midi_router_handle_end(fluid_synth_t* synth, int ac, char** av, fluid_ostream_t out);

int fluid_midi_router_begin(fluid_midi_router_t* router, fluid_midi_router_rule_t** dest);
int fluid_midi_router_end(fluid_midi_router_t* router);
int fluid_midi_router_create_default_rules(fluid_midi_router_t* router);
void fluid_midi_router_disable_all_rules(fluid_midi_router_t* router);
void fluid_midi_router_free_unused_rules(fluid_midi_router_t* router);

struct _fluid_midi_router_t {
	fluid_synth_t* synth;

	fluid_midi_router_rule_t* note_rules;
	fluid_midi_router_rule_t* cc_rules;
	fluid_midi_router_rule_t* progchange_rules;
	fluid_midi_router_rule_t* pitchbend_rules;
	fluid_midi_router_rule_t* channel_pressure_rules;
	fluid_midi_router_rule_t* key_pressure_rules;

	int new_rule_chan_min;
	int new_rule_chan_max;
	double new_rule_chan_mul;
	int new_rule_chan_add;
	int new_rule_par1_min;
	int new_rule_par1_max;
	double new_rule_par1_mul;
	int new_rule_par1_add;
	int new_rule_par2_min;
	int new_rule_par2_max;
	double new_rule_par2_mul;
	int new_rule_par2_add;

	fluid_midi_router_rule_t** dest;

	handle_midi_event_func_t event_handler;
	void* event_handler_data;

	int nr_midi_channels;
	fluid_mutex_t ruletables_mutex;
};

struct _fluid_midi_router_rule_t {
	int chan_min;
	int chan_max;
	fluid_real_t chan_mul;
	int chan_add;

	int par1_min;
	int par1_max;
	fluid_real_t par1_mul;
	int par1_add;

	int par2_min;
	int par2_max;
	fluid_real_t par2_mul;
	int par2_add;

	int pending_events;
	char keys_cc[128];
	fluid_midi_router_rule_t* next;
	int state;
};

enum fluid_midirule_state {
	MIDIRULE_ACTIVE = 0x00,
	MIDIRULE_WAITING,
	MIDIRULE_DONE
};
#endif

enum fluid_loop {
	FLUID_UNLOOPED = 0,
	FLUID_LOOP_DURING_RELEASE = 1,
	FLUID_NOTUSED = 2,
	FLUID_LOOP_UNTIL_RELEASE = 3
};

enum fluid_synth_status
{
	FLUID_SYNTH_CLEAN,
	FLUID_SYNTH_PLAYING,
	FLUID_SYNTH_QUIET,
	FLUID_SYNTH_STOPPED
};

typedef struct _fluid_bank_offset_t fluid_bank_offset_t;

struct _fluid_bank_offset_t {
	int sfont_id;
	int offset;
};

struct _fluid_synth_t
{
	int polyphony;
	char with_reverb;
	char with_chorus;
	char verbose;
	char dump;
	double sample_rate;
	int midi_channels;
	int audio_channels;
	int audio_groups;
	int effects_channels;
	unsigned int state;
	unsigned int ticks;
	unsigned int start;

	fluid_list_t *loaders;
	fluid_list_t* sfont;
	unsigned int sfont_id;
	fluid_list_t* bank_offsets;

#if defined(MACOS9)
	fluid_list_t* unloading;
#endif

	double gain;
	fluid_channel_t** channel;
	int num_channels;
	int nvoice;
	fluid_voice_t** voice;
	unsigned int noteid;
	unsigned int storeid;
	int nbuf;

	fluid_real_t** left_buf;
	fluid_real_t** right_buf;
	fluid_real_t** fx_left_buf;
	fluid_real_t** fx_right_buf;

	fluid_revmodel_t* reverb;
	fluid_chorus_t* chorus;
	int cur;
	int dither_index;

	char outbuf[256];
	double cpu_load;

	fluid_tuning_t*** tuning;
	fluid_tuning_t* cur_tuning;

	fluid_midi_router_t* midi_router;
	fluid_mutex_t busy;
#ifdef LADSPA
	fluid_LADSPA_FxUnit_t* LADSPA_FxUnit;
#endif
};

int fluid_synth_set_reverb_preset(fluid_synth_t* synth, int num);

int fluid_synth_one_block(fluid_synth_t* synth, int do_not_mix_fx_to_out);

fluid_preset_t* fluid_synth_get_preset(fluid_synth_t* synth,
	unsigned int sfontnum,
	unsigned int banknum,
	unsigned int prognum);

fluid_preset_t* fluid_synth_find_preset(fluid_synth_t* synth,
	unsigned int banknum,
	unsigned int prognum);

int fluid_synth_all_notes_off(fluid_synth_t* synth, int chan);
int fluid_synth_all_sounds_off(fluid_synth_t* synth, int chan);
int fluid_synth_modulate_voices(fluid_synth_t* synth, int chan, int is_cc, int ctrl);
int fluid_synth_modulate_voices_all(fluid_synth_t* synth, int chan);
int fluid_synth_damp_voices(fluid_synth_t* synth, int chan);
int fluid_synth_kill_voice(fluid_synth_t* synth, fluid_voice_t * voice);
void fluid_synth_kill_by_exclusive_class(fluid_synth_t* synth, fluid_voice_t* voice);
void fluid_synth_release_voice_on_same_note(fluid_synth_t* synth, int chan, int key);
void fluid_synth_sfunload_macos9(fluid_synth_t* synth);

void fluid_synth_print_voice(fluid_synth_t* synth);

void fluid_synth_update_presets(fluid_synth_t* synth);

int fluid_synth_update_gain(fluid_synth_t* synth, char* name, double value);
int fluid_synth_update_polyphony(fluid_synth_t* synth, char* name, int value);

fluid_bank_offset_t* fluid_synth_get_bank_offset0(fluid_synth_t* synth, int sfont_id);
void fluid_synth_remove_bank_offset(fluid_synth_t* synth, int sfont_id);

void fluid_synth_dither_s16(int *dither_index, int len, float* lin, float* rin,
	void* lout, int loff, int lincr,
	void* rout, int roff, int rincr);

#endif

#ifndef _FLUID_CHAN_H
#define _FLUID_CHAN_H

#ifndef _FLUID_TUNING_H
#define _FLUID_TUNING_H

struct _fluid_tuning_t {
	char* name;
	int bank;
	int prog;
	double pitch[128];
};

fluid_tuning_t* new_fluid_tuning(char* name, int bank, int prog);
void delete_fluid_tuning(fluid_tuning_t* tuning);

void fluid_tuning_set_name(fluid_tuning_t* tuning, char* name);
char* fluid_tuning_get_name(fluid_tuning_t* tuning);

#define fluid_tuning_get_bank(_t) ((_t)->bank)
#define fluid_tuning_get_prog(_t) ((_t)->prog)

void fluid_tuning_set_pitch(fluid_tuning_t* tuning, int key, double pitch);
#define fluid_tuning_get_pitch(_t, _key) ((_t)->pitch[_key])

void fluid_tuning_set_octave(fluid_tuning_t* tuning, double* pitch_deriv);

void fluid_tuning_set_all(fluid_tuning_t* tuning, double* pitch);
#define fluid_tuning_get_all(_t) (&(_t)->pitch[0])

#endif

struct _fluid_channel_t
{
	int channum;
	unsigned int sfontnum;
	unsigned int banknum;
	unsigned int prognum;
	fluid_preset_t* preset;
	fluid_synth_t* synth;
	short key_pressure;
	short channel_pressure;
	short pitch_bend;
	short pitch_wheel_sensitivity;

	short cc[128];

	unsigned char bank_msb;
	int interp_method;

	fluid_tuning_t* tuning;

	short nrpn_select;
	short nrpn_active;

	fluid_real_t gen[GEN_LAST];

	char gen_abs[GEN_LAST];
};

fluid_channel_t* new_fluid_channel(fluid_synth_t* synth, int num);
int delete_fluid_channel(fluid_channel_t* chan);
void fluid_channel_init(fluid_channel_t* chan);
void fluid_channel_init_ctrl(fluid_channel_t* chan);
void fluid_channel_reset(fluid_channel_t* chan);
int fluid_channel_set_preset(fluid_channel_t* chan, fluid_preset_t* preset);
fluid_preset_t* fluid_channel_get_preset(fluid_channel_t* chan);
unsigned int fluid_channel_get_sfontnum(fluid_channel_t* chan);
int fluid_channel_set_sfontnum(fluid_channel_t* chan, unsigned int sfont);
unsigned int fluid_channel_get_banknum(fluid_channel_t* chan);
int fluid_channel_set_banknum(fluid_channel_t* chan, unsigned int bank);
int fluid_channel_set_prognum(fluid_channel_t* chan, int prognum);
int fluid_channel_get_prognum(fluid_channel_t* chan);
int fluid_channel_cc(fluid_channel_t* chan, int ctrl, int val);
int fluid_channel_pressure(fluid_channel_t* chan, int val);
int fluid_channel_pitch_bend(fluid_channel_t* chan, int val);
int fluid_channel_pitch_wheel_sens(fluid_channel_t* chan, int val);
int fluid_channel_get_cc(fluid_channel_t* chan, int num);
int fluid_channel_get_num(fluid_channel_t* chan);
void fluid_channel_set_interp_method(fluid_channel_t* chan, int new_method);
int fluid_channel_get_interp_method(fluid_channel_t* chan);

#define fluid_channel_set_tuning(_c, _t)        { (_c)->tuning = _t; }
#define fluid_channel_has_tuning(_c)            ((_c)->tuning != NULL)
#define fluid_channel_get_tuning(_c)            ((_c)->tuning)
#define fluid_channel_sustained(_c)             ((_c)->cc[SUSTAIN_SWITCH] >= 64)
#define fluid_channel_set_gen(_c, _n, _v, _a)   { (_c)->gen[_n] = _v; (_c)->gen_abs[_n] = _a; }
#define fluid_channel_get_gen(_c, _n)           ((_c)->gen[_n])
#define fluid_channel_get_gen_abs(_c, _n)       ((_c)->gen_abs[_n])

#endif

#ifndef _PRIV_FLUID_SFONT_H
#define _PRIV_FLUID_SFONT_H

#define fluid_sfloader_delete(_loader) { if ((_loader) && (_loader)->free) (*(_loader)->free)(_loader); }
#define fluid_sfloader_load(_loader, _filename) (*(_loader)->load)(_loader, _filename)

#define delete_fluid_sfont(_sf)   ( ((_sf) && (_sf)->free)? (*(_sf)->free)(_sf) : 0)
#define fluid_sfont_get_name(_sf) (*(_sf)->get_name)(_sf)
#define fluid_sfont_get_preset(_sf,_bank,_prenum) (*(_sf)->get_preset)(_sf,_bank,_prenum)
#define fluid_sfont_iteration_start(_sf) (*(_sf)->iteration_start)(_sf)
#define fluid_sfont_iteration_next(_sf,_pr) (*(_sf)->iteration_next)(_sf,_pr)
#define fluid_sfont_get_data(_sf) (_sf)->data
#define fluid_sfont_set_data(_sf,_p) { (_sf)->data = (void*) (_p); }

#define delete_fluid_preset(_preset) \
  { if ((_preset) && (_preset)->free) { (*(_preset)->free)(_preset); }}

#define fluid_preset_get_data(_preset) (_preset)->data
#define fluid_preset_set_data(_preset,_p) { (_preset)->data = (void*) (_p); }
#define fluid_preset_get_name(_preset) (*(_preset)->get_name)(_preset)
#define fluid_preset_get_banknum(_preset) (*(_preset)->get_banknum)(_preset)
#define fluid_preset_get_num(_preset) (*(_preset)->get_num)(_preset)

#define fluid_preset_noteon(_preset,_synth,_ch,_key,_vel) \
  (*(_preset)->noteon)(_preset,_synth,_ch,_key,_vel)

#define fluid_preset_notify(_preset,_reason,_chan) \
  { if ((_preset) && (_preset)->notify) { (*(_preset)->notify)(_preset,_reason,_chan); }}

#define fluid_sample_incr_ref(_sample) { (_sample)->refcount++; }

#define fluid_sample_decr_ref(_sample) \
  (_sample)->refcount--; \
  if (((_sample)->refcount == 0) && ((_sample)->notify)) \
    (*(_sample)->notify)(_sample, FLUID_SAMPLE_DONE);

#endif

#ifdef TRAP_ON_FPE
#define _GNU_SOURCE
#include <fenv.h>

static int feenableexcept(int excepts);
#endif

fluid_sfloader_t* new_fluid_defsfloader(void);

int fluid_synth_program_select2(fluid_synth_t* synth,
	int chan,
	char* sfont_name,
	unsigned int bank_num,
	unsigned int preset_num);

fluid_sfont_t* fluid_synth_get_sfont_by_name(fluid_synth_t* synth, char *name);

int fluid_synth_set_gen2(fluid_synth_t* synth, int chan,
	int param, float value,
	int absolute, int normalized);

static int fluid_synth_initialized = 0;
static void fluid_synth_init(void);
static void init_dither(void);

fluid_mod_t default_vel2att_mod;
fluid_mod_t default_vel2filter_mod;
fluid_mod_t default_at2viblfo_mod;
fluid_mod_t default_mod2viblfo_mod;
fluid_mod_t default_att_mod;
fluid_mod_t default_pan_mod;
fluid_mod_t default_expr_mod;
fluid_mod_t default_reverb_mod;
fluid_mod_t default_chorus_mod;
fluid_mod_t default_pitch_bend_mod;

static fluid_revmodel_presets_t revmodel_preset[] = {
	{ (char*)"Test 1", 0.2f, 0.0f, 0.5f, 0.9f },
	{ (char*)"Test 2", 0.4f, 0.2f, 0.5f, 0.8f },
	{ (char*)"Test 3", 0.6f, 0.4f, 0.5f, 0.7f },
	{ (char*)"Test 4", 0.8f, 0.7f, 0.5f, 0.6f },
	{ (char*)"Test 5", 0.8f, 1.0f, 0.5f, 0.5f },
	{ NULL, 0.0f, 0.0f, 0.0f, 0.0f }
};

void fluid_version(int *major, int *minor, int *micro)
{
	*major = FLUIDSYNTH_VERSION_MAJOR;
	*minor = FLUIDSYNTH_VERSION_MINOR;
	*micro = FLUIDSYNTH_VERSION_MICRO;
}

char* fluid_version_str(void)
{
	return (char*)FLUIDSYNTH_VERSION;
}

static void fluid_synth_init()
{
	fluid_synth_initialized++;

#ifdef TRAP_ON_FPE

	feenableexcept(FE_DIVBYZERO | FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID);
#endif

	fluid_conversion_config();

	fluid_dsp_float_config();

	fluid_sys_config();

	init_dither();

	fluid_mod_set_source1(&default_vel2att_mod,
		FLUID_MOD_VELOCITY,
		FLUID_MOD_GC
		| FLUID_MOD_CONCAVE
		| FLUID_MOD_UNIPOLAR
		| FLUID_MOD_NEGATIVE
		);
	fluid_mod_set_source2(&default_vel2att_mod, 0, 0);
	fluid_mod_set_dest(&default_vel2att_mod, GEN_ATTENUATION);
	fluid_mod_set_amount(&default_vel2att_mod, 960.0);

	fluid_mod_set_source1(&default_vel2filter_mod, FLUID_MOD_VELOCITY,
		FLUID_MOD_GC
		| FLUID_MOD_LINEAR
		| FLUID_MOD_UNIPOLAR
		| FLUID_MOD_NEGATIVE
		);
	fluid_mod_set_source2(&default_vel2filter_mod, FLUID_MOD_VELOCITY,
		FLUID_MOD_GC
		| FLUID_MOD_SWITCH
		| FLUID_MOD_UNIPOLAR
		| FLUID_MOD_POSITIVE
		);
	fluid_mod_set_dest(&default_vel2filter_mod, GEN_FILTERFC);
	fluid_mod_set_amount(&default_vel2filter_mod, -2400);

	fluid_mod_set_source1(&default_at2viblfo_mod, FLUID_MOD_CHANNELPRESSURE,
		FLUID_MOD_GC
		| FLUID_MOD_LINEAR
		| FLUID_MOD_UNIPOLAR
		| FLUID_MOD_POSITIVE
		);
	fluid_mod_set_source2(&default_at2viblfo_mod, 0, 0);
	fluid_mod_set_dest(&default_at2viblfo_mod, GEN_VIBLFOTOPITCH);
	fluid_mod_set_amount(&default_at2viblfo_mod, 50);

	fluid_mod_set_source1(&default_mod2viblfo_mod, 1,
		FLUID_MOD_CC
		| FLUID_MOD_LINEAR
		| FLUID_MOD_UNIPOLAR
		| FLUID_MOD_POSITIVE
		);
	fluid_mod_set_source2(&default_mod2viblfo_mod, 0, 0);
	fluid_mod_set_dest(&default_mod2viblfo_mod, GEN_VIBLFOTOPITCH);
	fluid_mod_set_amount(&default_mod2viblfo_mod, 50);

	fluid_mod_set_source1(&default_att_mod, 7,
		FLUID_MOD_CC
		| FLUID_MOD_CONCAVE
		| FLUID_MOD_UNIPOLAR
		| FLUID_MOD_NEGATIVE
		);
	fluid_mod_set_source2(&default_att_mod, 0, 0);
	fluid_mod_set_dest(&default_att_mod, GEN_ATTENUATION);
	fluid_mod_set_amount(&default_att_mod, 960.0);

	fluid_mod_set_source1(&default_pan_mod, 10,
		FLUID_MOD_CC
		| FLUID_MOD_LINEAR
		| FLUID_MOD_BIPOLAR
		| FLUID_MOD_POSITIVE
		);
	fluid_mod_set_source2(&default_pan_mod, 0, 0);
	fluid_mod_set_dest(&default_pan_mod, GEN_PAN);

	fluid_mod_set_amount(&default_pan_mod, 500.0);

	fluid_mod_set_source1(&default_expr_mod, 11,
		FLUID_MOD_CC
		| FLUID_MOD_CONCAVE
		| FLUID_MOD_UNIPOLAR
		| FLUID_MOD_NEGATIVE
		);
	fluid_mod_set_source2(&default_expr_mod, 0, 0);
	fluid_mod_set_dest(&default_expr_mod, GEN_ATTENUATION);
	fluid_mod_set_amount(&default_expr_mod, 960.0);

	fluid_mod_set_source1(&default_reverb_mod, 91,
		FLUID_MOD_CC
		| FLUID_MOD_LINEAR
		| FLUID_MOD_UNIPOLAR
		| FLUID_MOD_POSITIVE
		);
	fluid_mod_set_source2(&default_reverb_mod, 0, 0);
	fluid_mod_set_dest(&default_reverb_mod, GEN_REVERBSEND);
	fluid_mod_set_amount(&default_reverb_mod, 200);

	fluid_mod_set_source1(&default_chorus_mod, 93,
		FLUID_MOD_CC
		| FLUID_MOD_LINEAR
		| FLUID_MOD_UNIPOLAR
		| FLUID_MOD_POSITIVE
		);
	fluid_mod_set_source2(&default_chorus_mod, 0, 0);
	fluid_mod_set_dest(&default_chorus_mod, GEN_CHORUSSEND);
	fluid_mod_set_amount(&default_chorus_mod, 200);

	fluid_mod_set_source1(&default_pitch_bend_mod, FLUID_MOD_PITCHWHEEL,
		FLUID_MOD_GC
		| FLUID_MOD_LINEAR
		| FLUID_MOD_BIPOLAR
		| FLUID_MOD_POSITIVE
		);
	fluid_mod_set_source2(&default_pitch_bend_mod, FLUID_MOD_PITCHWHEELSENS,
		FLUID_MOD_GC
		| FLUID_MOD_LINEAR
		| FLUID_MOD_UNIPOLAR
		| FLUID_MOD_POSITIVE
		);
	fluid_mod_set_dest(&default_pitch_bend_mod, GEN_PITCH);
	fluid_mod_set_amount(&default_pitch_bend_mod, 12700.0);
}

fluid_synth_t*
new_fluid_synth() {
	int i;
	fluid_synth_t* synth;
	fluid_sfloader_t* loader;

	if (fluid_synth_initialized == 0) {
		fluid_synth_init();
	}

	synth = FLUID_NEW(fluid_synth_t);
	if (synth == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return NULL;
	}
	FLUID_MEMSET(synth, 0, sizeof(fluid_synth_t));

	fluid_mutex_init(synth->busy);

	synth->with_reverb = __synth_reverb_active;
	synth->with_chorus = __synth_reverb_active;
	synth->verbose = __synth_ladspa_active;
	synth->dump = __synth_dump;

	synth->polyphony = __synth_polyphony;
	synth->sample_rate = __synth_sample_rate;
	synth->midi_channels = __synth_midi_channels;
	synth->audio_channels = __synth_audio_channels;
	synth->audio_groups = __synth_audio_groups;
	synth->effects_channels = __synth_effects_channels;
	synth->gain = __synth_gain;

	synth->nbuf = synth->audio_channels;
	if (synth->audio_groups > synth->nbuf) {
		synth->nbuf = synth->audio_groups;
	}

#ifdef LADSPA

	synth->LADSPA_FxUnit = new_fluid_LADSPA_FxUnit(synth);
#endif

	synth->state = FLUID_SYNTH_PLAYING;
	synth->sfont = NULL;
	synth->noteid = 0;
	synth->ticks = 0;
	synth->tuning = NULL;

	loader = new_fluid_defsfloader();

	if (loader == NULL) {
		FLUID_LOG(FLUID_WARN, "Failed to create the default SoundFont loader");
	}
	else {
		fluid_synth_add_sfloader(synth, loader);
	}

	synth->channel = FLUID_ARRAY(fluid_channel_t*, synth->midi_channels);
	if (synth->channel == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		goto error_recovery;
	}
	for (i = 0; i < synth->midi_channels; i++) {
		synth->channel[i] = new_fluid_channel(synth, i);
		if (synth->channel[i] == NULL) {
			goto error_recovery;
		}
	}

	synth->nvoice = synth->polyphony;
	synth->voice = FLUID_ARRAY(fluid_voice_t*, synth->nvoice);
	if (synth->voice == NULL) {
		goto error_recovery;
	}
	for (i = 0; i < synth->nvoice; i++) {
		synth->voice[i] = new_fluid_voice(synth->sample_rate);
		if (synth->voice[i] == NULL) {
			goto error_recovery;
		}
	}

	synth->left_buf = NULL;
	synth->right_buf = NULL;
	synth->fx_left_buf = NULL;
	synth->fx_right_buf = NULL;

	synth->left_buf = FLUID_ARRAY(fluid_real_t*, synth->nbuf);
	synth->right_buf = FLUID_ARRAY(fluid_real_t*, synth->nbuf);

	if ((synth->left_buf == NULL) || (synth->right_buf == NULL)) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		goto error_recovery;
	}

	FLUID_MEMSET(synth->left_buf, 0, synth->nbuf * sizeof(fluid_real_t*));
	FLUID_MEMSET(synth->right_buf, 0, synth->nbuf * sizeof(fluid_real_t*));

	for (i = 0; i < synth->nbuf; i++) {
		synth->left_buf[i] = FLUID_ARRAY(fluid_real_t, FLUID_BUFSIZE);
		synth->right_buf[i] = FLUID_ARRAY(fluid_real_t, FLUID_BUFSIZE);

		if ((synth->left_buf[i] == NULL) || (synth->right_buf[i] == NULL)) {
			FLUID_LOG(FLUID_ERR, "Out of memory");
			goto error_recovery;
		}
	}

	synth->fx_left_buf = FLUID_ARRAY(fluid_real_t*, synth->effects_channels);
	synth->fx_right_buf = FLUID_ARRAY(fluid_real_t*, synth->effects_channels);

	if ((synth->fx_left_buf == NULL) || (synth->fx_right_buf == NULL)) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		goto error_recovery;
	}

	if (synth->effects_channels)
	{
		FLUID_MEMSET(synth->fx_left_buf, 0, synth->effects_channels * sizeof(fluid_real_t*));
		FLUID_MEMSET(synth->fx_right_buf, 0, synth->effects_channels * sizeof(fluid_real_t*));
	}

	for (i = 0; i < synth->effects_channels; i++) {
		synth->fx_left_buf[i] = FLUID_ARRAY(fluid_real_t, FLUID_BUFSIZE);
		synth->fx_right_buf[i] = FLUID_ARRAY(fluid_real_t, FLUID_BUFSIZE);

		if ((synth->fx_left_buf[i] == NULL) || (synth->fx_right_buf[i] == NULL)) {
			FLUID_LOG(FLUID_ERR, "Out of memory");
			goto error_recovery;
		}
	}

	synth->cur = FLUID_BUFSIZE;
	synth->dither_index = 0;

	synth->reverb = new_fluid_revmodel();
	if (synth->reverb == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		goto error_recovery;
	}

	fluid_synth_set_reverb(synth,
		FLUID_REVERB_DEFAULT_ROOMSIZE,
		FLUID_REVERB_DEFAULT_DAMP,
		FLUID_REVERB_DEFAULT_WIDTH,
		FLUID_REVERB_DEFAULT_LEVEL);

	synth->chorus = new_fluid_chorus(synth->sample_rate);
	if (synth->chorus == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		goto error_recovery;
	}

	synth->start = fluid_curtime();

	return synth;

error_recovery:
	delete_fluid_synth(synth);
	return NULL;
}

int delete_fluid_synth(fluid_synth_t* synth)
{
	int i, k;
	fluid_list_t *list;
	fluid_sfont_t* sfont;
	fluid_bank_offset_t* bank_offset;
	fluid_sfloader_t* loader;

	if (synth == NULL) {
		return FLUID_OK;
	}

	fluid_profiling_print();

	synth->state = FLUID_SYNTH_STOPPED;

	if (synth->voice != NULL) {
		for (i = 0; i < synth->nvoice; i++) {
			if (synth->voice[i] && fluid_voice_is_playing(synth->voice[i]))
				fluid_voice_off(synth->voice[i]);
		}
	}

	for (list = synth->sfont; list; list = fluid_list_next(list)) {
		sfont = (fluid_sfont_t*)fluid_list_get(list);
		delete_fluid_sfont(sfont);
	}

	delete_fluid_list(synth->sfont);

	for (list = synth->bank_offsets; list; list = fluid_list_next(list)) {
		bank_offset = (fluid_bank_offset_t*)fluid_list_get(list);
		FLUID_FREE(bank_offset);
	}

	delete_fluid_list(synth->bank_offsets);

	for (list = synth->loaders; list; list = fluid_list_next(list)) {
		loader = (fluid_sfloader_t*)fluid_list_get(list);
		fluid_sfloader_delete(loader);
	}

	delete_fluid_list(synth->loaders);

	if (synth->channel != NULL) {
		for (i = 0; i < synth->midi_channels; i++) {
			if (synth->channel[i] != NULL) {
				delete_fluid_channel(synth->channel[i]);
			}
		}
		FLUID_FREE(synth->channel);
	}

	if (synth->voice != NULL) {
		for (i = 0; i < synth->nvoice; i++) {
			if (synth->voice[i] != NULL) {
				delete_fluid_voice(synth->voice[i]);
			}
		}
		FLUID_FREE(synth->voice);
	}

	if (synth->left_buf != NULL) {
		for (i = 0; i < synth->nbuf; i++) {
			if (synth->left_buf[i] != NULL) {
				FLUID_FREE(synth->left_buf[i]);
			}
		}
		FLUID_FREE(synth->left_buf);
	}

	if (synth->right_buf != NULL) {
		for (i = 0; i < synth->nbuf; i++) {
			if (synth->right_buf[i] != NULL) {
				FLUID_FREE(synth->right_buf[i]);
			}
		}
		FLUID_FREE(synth->right_buf);
	}

	if (synth->fx_left_buf != NULL) {
		for (i = 0; i < 2; i++) {
			if (synth->fx_left_buf[i] != NULL) {
				FLUID_FREE(synth->fx_left_buf[i]);
			}
		}
		FLUID_FREE(synth->fx_left_buf);
	}

	if (synth->fx_right_buf != NULL) {
		for (i = 0; i < 2; i++) {
			if (synth->fx_right_buf[i] != NULL) {
				FLUID_FREE(synth->fx_right_buf[i]);
			}
		}
		FLUID_FREE(synth->fx_right_buf);
	}

	if (synth->reverb != NULL) {
		delete_fluid_revmodel(synth->reverb);
	}

	if (synth->chorus != NULL) {
		delete_fluid_chorus(synth->chorus);
	}

	if (synth->tuning != NULL) {
		for (i = 0; i < 128; i++) {
			if (synth->tuning[i] != NULL) {
				for (k = 0; k < 128; k++) {
					if (synth->tuning[i][k] != NULL) {
						FLUID_FREE(synth->tuning[i][k]);
					}
				}
				FLUID_FREE(synth->tuning[i]);
			}
		}
		FLUID_FREE(synth->tuning);
	}

#ifdef LADSPA

	fluid_LADSPA_shutdown(synth->LADSPA_FxUnit);
	FLUID_FREE(synth->LADSPA_FxUnit);
#endif

	fluid_mutex_destroy(synth->busy);

	FLUID_FREE(synth);

	return FLUID_OK;
}

char*
fluid_synth_error(fluid_synth_t* synth)
{
	return fluid_error();
}

int fluid_synth_noteon(fluid_synth_t* synth, int chan, int key, int vel)
{
	fluid_channel_t* channel;

	if ((chan < 0) || (chan >= synth->midi_channels)) {
		FLUID_LOG(FLUID_WARN, "Channel out of range");
		return FLUID_FAILED;
	}

	if (vel == 0) {
		return fluid_synth_noteoff(synth, chan, key);
	}

	channel = synth->channel[chan];

	if (channel->preset == NULL) {
		return FLUID_FAILED;
	}

	return fluid_synth_start(synth, synth->noteid++, channel->preset, 0, chan, key, vel);
}

int fluid_synth_noteoff(fluid_synth_t* synth, int chan, int key)
{
	int i;
	fluid_voice_t* voice;
	int status = FLUID_FAILED;

	unsigned int iOffStartTime = 0;
	for (i = synth->polyphony - 1; i >= 0; i--) {
		voice = synth->voice[i];
		if (_ON(voice) && (voice->chan == chan) && (voice->key == key)) {
			if ((!iOffStartTime) || (voice->start_time == iOffStartTime)) {
				if (!iOffStartTime) iOffStartTime = voice->start_time;
				fluid_voice_noteoff(voice);
				status = FLUID_OK;
			}
		}
	}
	return status;
}

int fluid_synth_damp_voices(fluid_synth_t* synth, int chan)
{
	int i;
	fluid_voice_t* voice;

	for (i = 0; i < synth->polyphony; i++) {
		voice = synth->voice[i];
		if ((voice->chan == chan) && _SUSTAINED(voice)) {
			fluid_voice_noteoff(voice);
		}
	}

	return FLUID_OK;
}

int fluid_synth_cc(fluid_synth_t* synth, int chan, int num, int val)
{
	if ((chan < 0) || (chan >= synth->midi_channels)) {
		FLUID_LOG(FLUID_WARN, "Channel out of range");
		return FLUID_FAILED;
	}
	if ((num < 0) || (num >= 128)) {
		FLUID_LOG(FLUID_WARN, "Ctrl out of range");
		return FLUID_FAILED;
	}
	if ((val < 0) || (val >= 128)) {
		FLUID_LOG(FLUID_WARN, "Value out of range");
		return FLUID_FAILED;
	}

	if (synth->verbose) {
		FLUID_LOG3(FLUID_INFO, "cc\t%d\t%d\t%d", chan, num, val);
	}

	fluid_channel_cc(synth->channel[chan], num, val);

	return FLUID_OK;
}

int fluid_synth_get_cc(fluid_synth_t* synth, int chan, int num, int* pval)
{
	if ((chan < 0) || (chan >= synth->midi_channels)) {
		FLUID_LOG(FLUID_WARN, "Channel out of range");
		return FLUID_FAILED;
	}
	if ((num < 0) || (num >= 128)) {
		FLUID_LOG(FLUID_WARN, "Ctrl out of range");
		return FLUID_FAILED;
	}

	*pval = synth->channel[chan]->cc[num];
	return FLUID_OK;
}

int fluid_synth_all_notes_off(fluid_synth_t* synth, int chan)
{
	int i;
	fluid_voice_t* voice;

	for (i = 0; i < synth->polyphony; i++) {
		voice = synth->voice[i];
		if (_PLAYING(voice) && (voice->chan == chan)) {
			fluid_voice_noteoff(voice);
		}
	}
	return FLUID_OK;
}

int fluid_synth_all_sounds_off(fluid_synth_t* synth, int chan)
{
	int i;
	fluid_voice_t* voice;

	for (i = 0; i < synth->polyphony; i++) {
		voice = synth->voice[i];
		if (_PLAYING(voice) && (voice->chan == chan)) {
			fluid_voice_off(voice);
		}
	}
	return FLUID_OK;
}

int fluid_synth_system_reset(fluid_synth_t* synth)
{
	int i;
	fluid_voice_t* voice;

	for (i = 0; i < synth->polyphony; i++) {
		voice = synth->voice[i];
		if (_PLAYING(voice)) {
			fluid_voice_off(voice);
		}
	}

	for (i = 0; i < synth->midi_channels; i++) {
		fluid_channel_reset(synth->channel[i]);
	}

	fluid_chorus_reset(synth->chorus);
	fluid_revmodel_reset(synth->reverb);

	return FLUID_OK;
}

int fluid_synth_modulate_voices(fluid_synth_t* synth, int chan, int is_cc, int ctrl)
{
	int i;
	fluid_voice_t* voice;

	for (i = 0; i < synth->polyphony; i++) {
		voice = synth->voice[i];
		if (voice->chan == chan) {
			fluid_voice_modulate(voice, is_cc, ctrl);
		}
	}
	return FLUID_OK;
}

int fluid_synth_modulate_voices_all(fluid_synth_t* synth, int chan)
{
	int i;
	fluid_voice_t* voice;

	for (i = 0; i < synth->polyphony; i++) {
		voice = synth->voice[i];
		if (voice->chan == chan) {
			fluid_voice_modulate_all(voice);
		}
	}
	return FLUID_OK;
}

int fluid_synth_channel_pressure(fluid_synth_t* synth, int chan, int val)
{
	if ((chan < 0) || (chan >= synth->midi_channels)) {
		FLUID_LOG(FLUID_WARN, "Channel out of range");
		return FLUID_FAILED;
	}

	if (synth->verbose) {
		FLUID_LOG2(FLUID_INFO, "channelpressure\t%d\t%d", chan, val);
	}

	fluid_channel_pressure(synth->channel[chan], val);

	return FLUID_OK;
}

int fluid_synth_pitch_bend(fluid_synth_t* synth, int chan, int val)
{
	if ((chan < 0) || (chan >= synth->midi_channels)) {
		FLUID_LOG(FLUID_WARN, "Channel out of range");
		return FLUID_FAILED;
	}

	if (synth->verbose) {
		FLUID_LOG2(FLUID_INFO, "pitchb\t%d\t%d", chan, val);
	}

	fluid_channel_pitch_bend(synth->channel[chan], val);

	return FLUID_OK;
}

int fluid_synth_get_pitch_bend(fluid_synth_t* synth, int chan, int* ppitch_bend)
{
	if ((chan < 0) || (chan >= synth->midi_channels)) {
		FLUID_LOG(FLUID_WARN, "Channel out of range");
		return FLUID_FAILED;
	}

	*ppitch_bend = synth->channel[chan]->pitch_bend;
	return FLUID_OK;
}

int fluid_synth_pitch_wheel_sens(fluid_synth_t* synth, int chan, int val)
{
	if ((chan < 0) || (chan >= synth->midi_channels)) {
		FLUID_LOG(FLUID_WARN, "Channel out of range");
		return FLUID_FAILED;
	}

	if (synth->verbose) {
		FLUID_LOG2(FLUID_INFO, "pitchsens\t%d\t%d", chan, val);
	}

	fluid_channel_pitch_wheel_sens(synth->channel[chan], val);

	return FLUID_OK;
}

int fluid_synth_get_pitch_wheel_sens(fluid_synth_t* synth, int chan, int* pval)
{
	if ((chan < 0) || (chan >= synth->midi_channels)) {
		FLUID_LOG(FLUID_WARN, "Channel out of range");
		return FLUID_FAILED;
	}

	*pval = synth->channel[chan]->pitch_wheel_sensitivity;

	return FLUID_OK;
}

fluid_preset_t* fluid_synth_get_preset(fluid_synth_t* synth, unsigned int sfontnum,
unsigned int banknum, unsigned int prognum)
{
	fluid_preset_t* preset = NULL;
	fluid_sfont_t* sfont = NULL;
	int offset;

	sfont = fluid_synth_get_sfont_by_id(synth, sfontnum);

	if (sfont != NULL) {
		offset = fluid_synth_get_bank_offset(synth, sfontnum);
		preset = fluid_sfont_get_preset(sfont, banknum - offset, prognum);
		if (preset != NULL) {
			return preset;
		}
	}
	return NULL;
}

fluid_preset_t* fluid_synth_get_preset2(fluid_synth_t* synth, char* sfont_name,
unsigned int banknum, unsigned int prognum)
{
	fluid_preset_t* preset = NULL;
	fluid_sfont_t* sfont = NULL;
	int offset;

	sfont = fluid_synth_get_sfont_by_name(synth, sfont_name);

	if (sfont != NULL) {
		offset = fluid_synth_get_bank_offset(synth, fluid_sfont_get_id(sfont));
		preset = fluid_sfont_get_preset(sfont, banknum - offset, prognum);
		if (preset != NULL) {
			return preset;
		}
	}
	return NULL;
}

fluid_preset_t* fluid_synth_find_preset(fluid_synth_t* synth,
	unsigned int banknum,
	unsigned int prognum)
{
	fluid_preset_t* preset = NULL;
	fluid_sfont_t* sfont = NULL;
	fluid_list_t* list = synth->sfont;
	int offset;

	while (list) {
		sfont = (fluid_sfont_t*)fluid_list_get(list);
		offset = fluid_synth_get_bank_offset(synth, fluid_sfont_get_id(sfont));
		preset = fluid_sfont_get_preset(sfont, banknum - offset, prognum);

		if (preset != NULL) {
			preset->sfont = sfont;
			return preset;
		}

		list = fluid_list_next(list);

	}
	return NULL;
}

int fluid_synth_program_change(fluid_synth_t* synth, int chan, int prognum)
{
	fluid_preset_t* preset = NULL;
	fluid_channel_t* channel;
	unsigned int banknum;
	unsigned int sfont_id;
	int subst_bank, subst_prog; (void)subst_bank; (void)subst_prog;

	if ((prognum < 0) || (prognum >= FLUID_NUM_PROGRAMS) ||
		(chan < 0) || (chan >= synth->midi_channels))
	{
		FLUID_LOG2(FLUID_ERR, "Index out of range (chan=%d, prog=%d)", chan, prognum);
		return FLUID_FAILED;
	}

	channel = synth->channel[chan];
	banknum = fluid_channel_get_banknum(channel);

	fluid_channel_set_prognum(channel, prognum);

	if (synth->verbose)
		FLUID_LOG3(FLUID_INFO, "prog\t%d\t%d\t%d", chan, banknum, prognum);

	if (channel->channum == 9)
		preset = fluid_synth_find_preset(synth, DRUM_INST_BANK, prognum);
	else preset = fluid_synth_find_preset(synth, banknum, prognum);

	if (!preset)
	{
		subst_bank = banknum;
		subst_prog = prognum;

		if (channel->channum != 9 && banknum != DRUM_INST_BANK)
		{
			subst_bank = 0;

			preset = fluid_synth_find_preset(synth, 0, prognum);

			if (!preset && prognum != 0)
			{
				preset = fluid_synth_find_preset(synth, 0, 0);
				subst_prog = 0;
			}
		}
		else
		{
			preset = fluid_synth_find_preset(synth, DRUM_INST_BANK, 0);
			subst_prog = 0;
		}

	}

	sfont_id = preset ? fluid_sfont_get_id(preset->sfont) : 0;
	fluid_channel_set_sfontnum(channel, sfont_id);
	fluid_channel_set_preset(channel, preset);

	return FLUID_OK;
}

int fluid_synth_bank_select(fluid_synth_t* synth, int chan, unsigned int bank)
{
	if ((chan >= 0) && (chan < synth->midi_channels)) {
		fluid_channel_set_banknum(synth->channel[chan], bank);
		return FLUID_OK;
	}
	return FLUID_FAILED;
}

int fluid_synth_sfont_select(fluid_synth_t* synth, int chan, unsigned int sfont_id)
{
	if ((chan >= 0) && (chan < synth->midi_channels)) {
		fluid_channel_set_sfontnum(synth->channel[chan], sfont_id);
		return FLUID_OK;
	}
	return FLUID_FAILED;
}

int fluid_synth_get_program(fluid_synth_t* synth, int chan,
unsigned int* sfont_id, unsigned int* bank_num, unsigned int* preset_num)
{
	fluid_channel_t* channel;
	if ((chan >= 0) && (chan < synth->midi_channels)) {
		channel = synth->channel[chan];
		*sfont_id = fluid_channel_get_sfontnum(channel);
		*bank_num = fluid_channel_get_banknum(channel);
		*preset_num = fluid_channel_get_prognum(channel);
		return FLUID_OK;
	}
	return FLUID_FAILED;
}

int fluid_synth_program_select(fluid_synth_t* synth,
	int chan,
	unsigned int sfont_id,
	unsigned int bank_num,
	unsigned int preset_num)
{
	fluid_preset_t* preset = NULL;
	fluid_channel_t* channel;

	if ((chan < 0) || (chan >= synth->midi_channels)) {
		FLUID_LOG1(FLUID_ERR, "Channel number out of range (chan=%d)", chan);
		return FLUID_FAILED;
	}
	channel = synth->channel[chan];

	preset = fluid_synth_get_preset(synth, sfont_id, bank_num, preset_num);
	if (preset == NULL) {
		FLUID_LOG3(FLUID_ERR,
			"There is no preset with bank number %d and preset number %d in SoundFont %d",
			bank_num, preset_num, sfont_id);
		return FLUID_FAILED;
	}

	fluid_channel_set_sfontnum(channel, sfont_id);
	fluid_channel_set_banknum(channel, bank_num);
	fluid_channel_set_prognum(channel, preset_num);

	fluid_channel_set_preset(channel, preset);

	return FLUID_OK;
}

int fluid_synth_program_select2(fluid_synth_t* synth,
	int chan,
	char* sfont_name,
	unsigned int bank_num,
	unsigned int preset_num)
{
	fluid_preset_t* preset = NULL;
	fluid_channel_t* channel;
	fluid_sfont_t* sfont = NULL;
	int offset;

	if ((chan < 0) || (chan >= synth->midi_channels)) {
		FLUID_LOG1(FLUID_ERR, "Channel number out of range (chan=%d)", chan);
		return FLUID_FAILED;
	}
	channel = synth->channel[chan];

	sfont = fluid_synth_get_sfont_by_name(synth, sfont_name);
	if (sfont == NULL) {
		FLUID_LOG1(FLUID_ERR, "Could not find SoundFont %s", sfont_name);
		return FLUID_FAILED;
	}

	offset = fluid_synth_get_bank_offset(synth, fluid_sfont_get_id(sfont));
	preset = fluid_sfont_get_preset(sfont, bank_num - offset, preset_num);
	if (preset == NULL) {
		FLUID_LOG3(FLUID_ERR,
			"There is no preset with bank number %d and preset number %d in SoundFont %s",
			bank_num, preset_num, sfont_name);
		return FLUID_FAILED;
	}

	fluid_channel_set_sfontnum(channel, fluid_sfont_get_id(sfont));
	fluid_channel_set_banknum(channel, bank_num);
	fluid_channel_set_prognum(channel, preset_num);

	fluid_channel_set_preset(channel, preset);

	return FLUID_OK;
}

void fluid_synth_update_presets(fluid_synth_t* synth)
{
	int chan;
	fluid_channel_t* channel;

	for (chan = 0; chan < synth->midi_channels; chan++) {
		channel = synth->channel[chan];
		fluid_channel_set_preset(channel,
			fluid_synth_get_preset(synth,
			fluid_channel_get_sfontnum(channel),
			fluid_channel_get_banknum(channel),
			fluid_channel_get_prognum(channel)));
	}
}

int fluid_synth_update_gain(fluid_synth_t* synth, char* name, double value)
{
	fluid_synth_set_gain(synth, (float)value);
	return 0;
}

void fluid_synth_set_gain(fluid_synth_t* synth, float gain)
{
	int i;

	fluid_clip(gain, 0.0f, 10.0f);
	synth->gain = gain;

	for (i = 0; i < synth->polyphony; i++) {
		fluid_voice_t* voice = synth->voice[i];
		if (_PLAYING(voice)) {
			fluid_voice_set_gain(voice, gain);
		}
	}
}

float fluid_synth_get_gain(fluid_synth_t* synth)
{
	return synth->gain;
}

int fluid_synth_update_polyphony(fluid_synth_t* synth, char* name, int value)
{
	fluid_synth_set_polyphony(synth, value);
	return 0;
}

int fluid_synth_set_polyphony(fluid_synth_t* synth, int polyphony)
{
	int i;

	if (polyphony < 1 || polyphony > synth->nvoice) {
		return FLUID_FAILED;
	}

	for (i = polyphony; i < synth->nvoice; i++) {
		fluid_voice_t* voice = synth->voice[i];
		if (_PLAYING(voice)) {
			fluid_voice_off(voice);
		}
	}

	synth->polyphony = polyphony;

	return FLUID_OK;
}

int fluid_synth_get_polyphony(fluid_synth_t* synth)
{
	return synth->polyphony;
}

int fluid_synth_get_internal_bufsize(fluid_synth_t* synth)
{
	return FLUID_BUFSIZE;
}

int fluid_synth_program_reset(fluid_synth_t* synth)
{
	int i;

	for (i = 0; i < synth->midi_channels; i++){
		fluid_synth_program_change(synth, i, fluid_channel_get_prognum(synth->channel[i]));
	}
	return FLUID_OK;
}

int fluid_synth_set_reverb_preset(fluid_synth_t* synth, int num)
{
	int i = 0;
	while (revmodel_preset[i].name != NULL) {
		if (i == num) {
			fluid_revmodel_setroomsize(synth->reverb, revmodel_preset[i].roomsize);
			fluid_revmodel_setdamp(synth->reverb, revmodel_preset[i].damp);
			fluid_revmodel_setwidth(synth->reverb, revmodel_preset[i].width);
			fluid_revmodel_setlevel(synth->reverb, revmodel_preset[i].level);
			return FLUID_OK;
		}
		i++;
	}
	return FLUID_FAILED;
}

void fluid_synth_set_reverb(fluid_synth_t* synth, double roomsize, double damping,
	double width, double level)
{
	fluid_revmodel_setroomsize(synth->reverb, roomsize);
	fluid_revmodel_setdamp(synth->reverb, damping);
	fluid_revmodel_setwidth(synth->reverb, width);
	fluid_revmodel_setlevel(synth->reverb, level);
}

void fluid_synth_set_chorus(fluid_synth_t* synth, int nr, double level,
	double speed, double depth_ms, int type)
{
	fluid_chorus_set_nr(synth->chorus, nr);
	fluid_chorus_set_level(synth->chorus, (fluid_real_t)level);
	fluid_chorus_set_speed_Hz(synth->chorus, (fluid_real_t)speed);
	fluid_chorus_set_depth_ms(synth->chorus, (fluid_real_t)depth_ms);
	fluid_chorus_set_type(synth->chorus, type);
	fluid_chorus_update(synth->chorus);
}

int fluid_synth_nwrite_float(fluid_synth_t* synth, int len,
float** left, float** right,
float** fx_left, float** fx_right)
{
	fluid_real_t** left_in = synth->left_buf;
	fluid_real_t** right_in = synth->right_buf;
	double time = fluid_utime();
	int i, num, available, count, bytes;

	if (synth->state != FLUID_SYNTH_PLAYING) {
		return 0;
	}

	count = 0;
	num = synth->cur;
	if (synth->cur < FLUID_BUFSIZE) {
		available = FLUID_BUFSIZE - synth->cur;

		num = (available > len) ? len : available;
		bytes = num * sizeof(float);

		for (i = 0; i < synth->audio_channels; i++) {
			FLUID_MEMCPY(left[i], left_in[i] + synth->cur, bytes);
			FLUID_MEMCPY(right[i], right_in[i] + synth->cur, bytes);
		}
		count += num;
		num += synth->cur;
	}

	while (count < len) {
		fluid_synth_one_block(synth, 1);

		num = (FLUID_BUFSIZE > len - count) ? len - count : FLUID_BUFSIZE;
		bytes = num * sizeof(float);

		for (i = 0; i < synth->audio_channels; i++) {
			FLUID_MEMCPY(left[i] + count, left_in[i], bytes);
			FLUID_MEMCPY(right[i] + count, right_in[i], bytes);
		}

		count += num;
	}

	synth->cur = num;

	time = fluid_utime() - time;
	synth->cpu_load = 0.5 * (synth->cpu_load +
		time * synth->sample_rate / len / 10000.0);

	return 0;
}

int fluid_synth_process(fluid_synth_t* synth, int len,
	int nin, float** in,
	int nout, float** out)
{
	if (nout == 2) {
		return fluid_synth_write_float(synth, len, out[0], 0, 1, out[1], 0, 1);
	}
	else {
		float **left, **right;
		int i;
		left = FLUID_ARRAY(float*, nout / 2);
		right = FLUID_ARRAY(float*, nout / 2);
		for (i = 0; i < nout / 2; i++) {
			left[i] = out[2 * i];
			right[i] = out[2 * i + 1];
		}
		fluid_synth_nwrite_float(synth, len, left, right, NULL, NULL);
		FLUID_FREE(left);
		FLUID_FREE(right);
		return 0;
	}
}

int fluid_synth_write_float(fluid_synth_t* synth, int len,
void* lout, int loff, int lincr,
void* rout, int roff, int rincr)
{
	int i, j, k, l;
	float* left_out = (float*)lout;
	float* right_out = (float*)rout;
	fluid_real_t* left_in = synth->left_buf[0];
	fluid_real_t* right_in = synth->right_buf[0];
	double time = fluid_utime();

	if (synth->state != FLUID_SYNTH_PLAYING) {
		return 0;
	}

	l = synth->cur;

	for (i = 0, j = loff, k = roff; i < len; i++, l++, j += lincr, k += rincr) {
		if (l == FLUID_BUFSIZE) {
			fluid_synth_one_block(synth, 0);
			l = 0;
		}

		left_out[j] = (float)left_in[l];
		right_out[k] = (float)right_in[l];
	}

	synth->cur = l;

	time = fluid_utime() - time;
	synth->cpu_load = 0.5 * (synth->cpu_load +
		time * synth->sample_rate / len / 10000.0);

	return 0;
}

#define DITHER_SIZE 48000
#define DITHER_CHANNELS 2

static float rand_table[DITHER_CHANNELS][DITHER_SIZE];

static void init_dither(void)
{
	float d, dp;
	int c, i;

	for (c = 0; c < DITHER_CHANNELS; c++) {
		dp = 0;
		for (i = 0; i < DITHER_SIZE - 1; i++) {
			d = rand() / (float)RAND_MAX - 0.5f;
			rand_table[c][i] = d - dp;
			dp = d;
		}
		rand_table[c][DITHER_SIZE - 1] = 0 - dp;
	}
}

static INLINE int
roundi(float x)
{
	if (x >= 0.0f)
		return (int)(x + 0.5f);
	else
		return (int)(x - 0.5f);
}

int fluid_synth_write_s16(fluid_synth_t* synth, int len,
void* lout, int loff, int lincr,
void* rout, int roff, int rincr)
{
	int i, j, k, cur;
	signed short* left_out = (signed short*)lout;
	signed short* right_out = (signed short*)rout;
	fluid_real_t* left_in = synth->left_buf[0];
	fluid_real_t* right_in = synth->right_buf[0];
	double prof_ref = fluid_profile_ref();
	fluid_real_t left_sample;
	fluid_real_t right_sample;
	double time = fluid_utime();
	int di = synth->dither_index;
	double prof_ref_on_block;

	if (synth->state != FLUID_SYNTH_PLAYING) {
		return 0;
	}

	cur = synth->cur;

	for (i = 0, j = loff, k = roff; i < len; i++, cur++, j += lincr, k += rincr) {
		if (cur == FLUID_BUFSIZE) {
			prof_ref_on_block = fluid_profile_ref();

			fluid_synth_one_block(synth, 0);
			cur = 0;

			fluid_profile(FLUID_PROF_ONE_BLOCK, prof_ref_on_block);
		}

		left_sample = roundi(left_in[cur] * 32766.0f + rand_table[0][di]);
		right_sample = roundi(right_in[cur] * 32766.0f + rand_table[1][di]);

		di++;
		if (di >= DITHER_SIZE) di = 0;

		if (left_sample > 32767.0f) left_sample = 32767.0f;
		if (left_sample < -32768.0f) left_sample = -32768.0f;
		if (right_sample > 32767.0f) right_sample = 32767.0f;
		if (right_sample < -32768.0f) right_sample = -32768.0f;

		left_out[j] = (signed short)left_sample;
		right_out[k] = (signed short)right_sample;
	}

	synth->cur = cur;
	synth->dither_index = di;

	fluid_profile(FLUID_PROF_WRITE_S16, prof_ref);

	time = fluid_utime() - time;
	synth->cpu_load = 0.5 * (synth->cpu_load +
		time * synth->sample_rate / len / 10000.0);

	return 0;
}

void fluid_synth_dither_s16(int *dither_index, int len, float* lin, float* rin,
void* lout, int loff, int lincr,
void* rout, int roff, int rincr)
{
	int i, j, k;
	signed short* left_out = (signed short*)lout;
	signed short* right_out = (signed short*)rout;
	double prof_ref = fluid_profile_ref();
	fluid_real_t left_sample;
	fluid_real_t right_sample;
	int di = *dither_index;

	for (i = 0, j = loff, k = roff; i < len; i++, j += lincr, k += rincr) {
		left_sample = roundi(lin[i] * 32766.0f + rand_table[0][di]);
		right_sample = roundi(rin[i] * 32766.0f + rand_table[1][di]);

		di++;
		if (di >= DITHER_SIZE) di = 0;

		if (left_sample > 32767.0f) left_sample = 32767.0f;
		if (left_sample < -32768.0f) left_sample = -32768.0f;
		if (right_sample > 32767.0f) right_sample = 32767.0f;
		if (right_sample < -32768.0f) right_sample = -32768.0f;

		left_out[j] = (signed short)left_sample;
		right_out[k] = (signed short)right_sample;
	}

	*dither_index = di;

	fluid_profile(FLUID_PROF_WRITE_S16, prof_ref);
}

int fluid_synth_one_block(fluid_synth_t* synth, int do_not_mix_fx_to_out)
{
	int i, auchan;
	fluid_voice_t* voice;
	fluid_real_t* left_buf;
	fluid_real_t* right_buf;
	fluid_real_t* reverb_buf;
	fluid_real_t* chorus_buf;
	int byte_size = FLUID_BUFSIZE * sizeof(fluid_real_t);
	double prof_ref = fluid_profile_ref();

	fluid_check_fpe("??? Just starting up ???");

	for (i = 0; i < synth->nbuf; i++) {
		FLUID_MEMSET(synth->left_buf[i], 0, byte_size);
		FLUID_MEMSET(synth->right_buf[i], 0, byte_size);
	}

	for (i = 0; i < synth->effects_channels; i++) {
		FLUID_MEMSET(synth->fx_left_buf[i], 0, byte_size);
		FLUID_MEMSET(synth->fx_right_buf[i], 0, byte_size);
	}

	reverb_buf = synth->with_reverb ? synth->fx_left_buf[0] : NULL;
	chorus_buf = synth->with_chorus ? synth->fx_left_buf[1] : NULL;

	fluid_profile(FLUID_PROF_ONE_BLOCK_CLEAR, prof_ref);

	for (i = 0; i < synth->polyphony; i++) {
		voice = synth->voice[i];

		if (_PLAYING(voice)) {
			double prof_ref_voice = fluid_profile_ref();

			auchan = fluid_channel_get_num(fluid_voice_get_channel(voice));
			auchan %= synth->audio_groups;
			left_buf = synth->left_buf[auchan];
			right_buf = synth->right_buf[auchan];

			fluid_voice_write(voice, left_buf, right_buf, reverb_buf, chorus_buf);

			fluid_profile(FLUID_PROF_ONE_BLOCK_VOICE, prof_ref_voice);
		}
	}

	fluid_check_fpe("Synthesis processes");

	fluid_profile(FLUID_PROF_ONE_BLOCK_VOICES, prof_ref);

	if (do_not_mix_fx_to_out) {
		if (reverb_buf) {
			fluid_revmodel_processreplace(synth->reverb, reverb_buf,
				synth->fx_left_buf[0], synth->fx_right_buf[0]);
			fluid_check_fpe("Reverb");
		}

		fluid_profile(FLUID_PROF_ONE_BLOCK_REVERB, prof_ref);

		if (chorus_buf) {
			fluid_chorus_processreplace(synth->chorus, chorus_buf,
				synth->fx_left_buf[1], synth->fx_right_buf[1]);
			fluid_check_fpe("Chorus");
		}

	}
	else {
		if (reverb_buf) {
			fluid_revmodel_processmix(synth->reverb, reverb_buf,
				synth->left_buf[0], synth->right_buf[0]);
			fluid_check_fpe("Reverb");
		}

		fluid_profile(FLUID_PROF_ONE_BLOCK_REVERB, prof_ref);

		if (chorus_buf) {
			fluid_chorus_processmix(synth->chorus, chorus_buf,
				synth->left_buf[0], synth->right_buf[0]);
			fluid_check_fpe("Chorus");
		}
	}

	fluid_profile(FLUID_PROF_ONE_BLOCK_CHORUS, prof_ref);

#ifdef LADSPA

	fluid_LADSPA_run(synth->LADSPA_FxUnit, synth->left_buf, synth->right_buf, synth->fx_left_buf, synth->fx_right_buf);
	fluid_check_fpe("LADSPA");
#endif

	synth->ticks += FLUID_BUFSIZE;

#if 0
	{float num = 1; while (num != 0){ num *= 0.5; }; };
#endif
	fluid_check_fpe("??? Remainder of synth_one_block ???");

	return 0;
}

fluid_voice_t*
fluid_synth_free_voice_by_kill(fluid_synth_t* synth)
{
	int i;
	fluid_real_t best_prio = 999999.;
	fluid_real_t this_voice_prio;
	fluid_voice_t* voice;
	int best_voice_index = -1;

	for (i = 0; i < synth->polyphony; i++) {
		voice = synth->voice[i];

		if (_AVAILABLE(voice)) {
			return voice;
		}

		this_voice_prio = 10000.;

		if (voice->chan == 9){
			this_voice_prio += 4000;

		}
		else if (_RELEASED(voice)){
			this_voice_prio -= 2000.;
		}

		if (_SUSTAINED(voice)){
			this_voice_prio -= 1000;
		}

		this_voice_prio -= (synth->noteid - fluid_voice_get_id(voice));

		if (voice->volenv_section != FLUID_VOICE_ENVATTACK){
			this_voice_prio += voice->volenv_val * 1000.;
		}

		if (this_voice_prio < best_prio)
			best_voice_index = i,
			best_prio = this_voice_prio;
	}

	if (best_voice_index < 0) {
		return NULL;
	}

	voice = synth->voice[best_voice_index];
	fluid_voice_off(voice);

	return voice;
}

fluid_voice_t*
fluid_synth_alloc_voice(fluid_synth_t* synth, fluid_sample_t* sample, int chan, int key, int vel, unsigned int start_time)
{
	int i;   fluid_voice_t* voice = NULL;
	fluid_channel_t* channel = NULL;

	for (i = 0; i < synth->polyphony; i++) {
		if (_AVAILABLE(synth->voice[i])) {
			voice = synth->voice[i];
			break;
		}
	}

	if (voice == NULL) {
		voice = fluid_synth_free_voice_by_kill(synth);
	}

	if (voice == NULL) {
		FLUID_LOG2(FLUID_WARN, "Failed to allocate a synthesis process. (chan=%d,key=%d)", chan, key);
		return NULL;
	}

	if (chan >= 0) {
		channel = synth->channel[chan];
	}

	if (fluid_voice_init(voice, sample, channel, key, vel,
		synth->storeid, start_time, synth->gain) != FLUID_OK) {
		FLUID_LOG(FLUID_WARN, "Failed to initialize voice");
		return NULL;
	}

	fluid_voice_add_mod(voice, &default_vel2att_mod, FLUID_VOICE_DEFAULT);
	fluid_voice_add_mod(voice, &default_vel2filter_mod, FLUID_VOICE_DEFAULT);
	fluid_voice_add_mod(voice, &default_at2viblfo_mod, FLUID_VOICE_DEFAULT);
	fluid_voice_add_mod(voice, &default_mod2viblfo_mod, FLUID_VOICE_DEFAULT);
	fluid_voice_add_mod(voice, &default_att_mod, FLUID_VOICE_DEFAULT);
	fluid_voice_add_mod(voice, &default_pan_mod, FLUID_VOICE_DEFAULT);
	fluid_voice_add_mod(voice, &default_expr_mod, FLUID_VOICE_DEFAULT);
	fluid_voice_add_mod(voice, &default_reverb_mod, FLUID_VOICE_DEFAULT);
	fluid_voice_add_mod(voice, &default_chorus_mod, FLUID_VOICE_DEFAULT);
	fluid_voice_add_mod(voice, &default_pitch_bend_mod, FLUID_VOICE_DEFAULT);

	return voice;
}

void fluid_synth_kill_by_exclusive_class(fluid_synth_t* synth, fluid_voice_t* new_voice)
{
	int i;
	int excl_class = _GEN(new_voice, GEN_EXCLUSIVECLASS);

	if (excl_class == 0) {
		return;
	}

	for (i = 0; i < synth->polyphony; i++) {
		fluid_voice_t* existing_voice = synth->voice[i];

		if (!_PLAYING(existing_voice)) {
			continue;
		}

		if (existing_voice->chan != new_voice->chan) {
			continue;
		}

		if ((int)_GEN(existing_voice, GEN_EXCLUSIVECLASS) != excl_class) {
			continue;
		}

		if (fluid_voice_get_id(existing_voice) == fluid_voice_get_id(new_voice)) {
			continue;
		}

		fluid_voice_kill_excl(existing_voice);
	};
}

void fluid_synth_start_voice(fluid_synth_t* synth, fluid_voice_t* voice)
{
	fluid_synth_kill_by_exclusive_class(synth, voice);

	fluid_voice_start(voice);
}

void fluid_synth_add_sfloader(fluid_synth_t* synth, fluid_sfloader_t* loader)
{
	synth->loaders = fluid_list_prepend(synth->loaders, loader);
}

int fluid_synth_sfload(fluid_synth_t* synth, const char* filename, int reset_presets)
{
	fluid_sfont_t* sfont;
	fluid_list_t* list;
	fluid_sfloader_t* loader;

#if defined(MACOS9)
	fluid_synth_sfunload_macos9(synth);
#endif

	if (filename == NULL) {
		FLUID_LOG(FLUID_ERR, "Invalid filename");
		return FLUID_FAILED;
	}

	for (list = synth->loaders; list; list = fluid_list_next(list)) {
		loader = (fluid_sfloader_t*)fluid_list_get(list);

		sfont = fluid_sfloader_load(loader, filename);

		if (sfont != NULL) {
			sfont->id = ++synth->sfont_id;

			synth->sfont = fluid_list_prepend(synth->sfont, sfont);

			if (reset_presets) {
				fluid_synth_program_reset(synth);
			}

			return (int)sfont->id;
		}
	}

	FLUID_LOG1(FLUID_ERR, "Failed to load SoundFont \"%s\"", filename);
	return -1;
}

static int fluid_synth_sfunload_callback(void* data, unsigned int msec)
{
	fluid_sfont_t* sfont = (fluid_sfont_t*)data;
	int r = delete_fluid_sfont(sfont);
	if (r == 0) {
		FLUID_LOG(FLUID_DBG, "Unloaded SoundFont");
	}
	return r != 0;
}

void fluid_synth_sfunload_macos9(fluid_synth_t* synth)
{
#if defined(MACOS9)
	fluid_list_t *list, *next;
	fluid_sfont_t* sfont;

	list = synth->unloading;
	while (list) {
		next = fluid_list_next(list);
		sfont = (fluid_sfont_t*)fluid_list_get(list);
		if (delete_fluid_sfont(sfont) == 0) {
			synth->unloading = fluid_list_remove(synth->unloading, sfont);
		}
		list = next;
	}
#endif
}

int fluid_synth_sfunload(fluid_synth_t* synth, unsigned int id, int reset_presets)
{
	fluid_sfont_t* sfont = fluid_synth_get_sfont_by_id(synth, id);

#if defined(MACOS9)
	fluid_synth_sfunload_macos9(synth);
#endif

	if (!sfont) {
		FLUID_LOG1(FLUID_ERR, "No SoundFont with id = %d", id);
		return FLUID_FAILED;
	}

	synth->sfont = fluid_list_remove(synth->sfont, sfont);

	if (reset_presets) {
		fluid_synth_program_reset(synth);
	}
	else {
		fluid_synth_update_presets(synth);
	}

	if (delete_fluid_sfont(sfont) != 0) {
#if defined(MACOS9)
		synth->unloading = fluid_list_prepend(synth->unloading, sfont);
#else

		new_fluid_timer(100, fluid_synth_sfunload_callback, sfont, 1, 1);
#endif
	}

	return FLUID_OK;
}

int fluid_synth_sfreload(fluid_synth_t* synth, unsigned int id)
{
	char filename[1024];
	fluid_sfont_t* sfont;
	int index = 0;
	fluid_list_t *list;
	fluid_sfloader_t* loader;

	sfont = fluid_synth_get_sfont_by_id(synth, id);
	if (!sfont) {
		FLUID_LOG1(FLUID_ERR, "No SoundFont with id = %d", id);
		return FLUID_FAILED;
	}

	list = synth->sfont;
	while (list) {
		if (sfont == (fluid_sfont_t*)fluid_list_get(list)) {
			break;
		}
		list = fluid_list_next(list);
		index++;
	}

	FLUID_STRCPY(filename, fluid_sfont_get_name(sfont));

	if (fluid_synth_sfunload(synth, id, 0) != FLUID_OK) {
		return FLUID_FAILED;
	}

	for (list = synth->loaders; list; list = fluid_list_next(list)) {
		loader = (fluid_sfloader_t*)fluid_list_get(list);

		sfont = fluid_sfloader_load(loader, filename);

		if (sfont != NULL) {
			sfont->id = id;

			synth->sfont = fluid_list_insert_at(synth->sfont, index, sfont);

			fluid_synth_update_presets(synth);

			return sfont->id;
		}
	}

	FLUID_LOG1(FLUID_ERR, "Failed to load SoundFont \"%s\"", filename);
	return -1;
}

int fluid_synth_add_sfont(fluid_synth_t* synth, fluid_sfont_t* sfont)
{
	sfont->id = ++synth->sfont_id;

	synth->sfont = fluid_list_prepend(synth->sfont, sfont);

	fluid_synth_program_reset(synth);

	return sfont->id;
}

void fluid_synth_remove_sfont(fluid_synth_t* synth, fluid_sfont_t* sfont)
{
	int sfont_id = fluid_sfont_get_id(sfont);

	synth->sfont = fluid_list_remove(synth->sfont, sfont);

	fluid_synth_remove_bank_offset(synth, sfont_id);

	fluid_synth_program_reset(synth);
}

int fluid_synth_sfcount(fluid_synth_t* synth)
{
	return fluid_list_size(synth->sfont);
}

fluid_sfont_t*
fluid_synth_get_sfont(fluid_synth_t* synth, unsigned int num)
{
	return (fluid_sfont_t*)fluid_list_get(fluid_list_nth(synth->sfont, num));
}

fluid_sfont_t* fluid_synth_get_sfont_by_id(fluid_synth_t* synth, unsigned int id)
{
	fluid_list_t* list = synth->sfont;
	fluid_sfont_t* sfont;

	while (list) {
		sfont = (fluid_sfont_t*)fluid_list_get(list);
		if (fluid_sfont_get_id(sfont) == id) {
			return sfont;
		}
		list = fluid_list_next(list);
	}
	return NULL;
}

fluid_sfont_t* fluid_synth_get_sfont_by_name(fluid_synth_t* synth, char *name)
{
	fluid_list_t* list = synth->sfont;
	fluid_sfont_t* sfont;

	while (list) {
		sfont = (fluid_sfont_t*)fluid_list_get(list);
		if (FLUID_STRCMP(fluid_sfont_get_name(sfont), name) == 0) {
			return sfont;
		}
		list = fluid_list_next(list);
	}
	return NULL;
}

fluid_preset_t* fluid_synth_get_channel_preset(fluid_synth_t* synth, int chan)
{
	if ((chan >= 0) && (chan < synth->midi_channels)) {
		return fluid_channel_get_preset(synth->channel[chan]);
	}

	return NULL;
}

void fluid_synth_get_voicelist(fluid_synth_t* synth, fluid_voice_t* buf[], int bufsize, int ID)
{
	int i;
	int count = 0;
	for (i = 0; i < synth->polyphony; i++) {
		fluid_voice_t* voice = synth->voice[i];
		if (count >= bufsize) {
			return;
		}

		if (_PLAYING(voice) && ((int)voice->id == ID || ID < 0)) {
			buf[count++] = voice;
		}
	}
	if (count >= bufsize) {
		return;
	}
	buf[count++] = NULL;
}

void fluid_synth_set_reverb_on(fluid_synth_t* synth, int on)
{
	synth->with_reverb = on;
}

void fluid_synth_set_chorus_on(fluid_synth_t* synth, int on)
{
	synth->with_chorus = on;
}

int fluid_synth_get_chorus_nr(fluid_synth_t* synth)
{
	return fluid_chorus_get_nr(synth->chorus);
}

double fluid_synth_get_chorus_level(fluid_synth_t* synth)
{
	return (double)fluid_chorus_get_level(synth->chorus);
}

double fluid_synth_get_chorus_speed_Hz(fluid_synth_t* synth)
{
	return (double)fluid_chorus_get_speed_Hz(synth->chorus);
}

double fluid_synth_get_chorus_depth_ms(fluid_synth_t* synth)
{
	return (double)fluid_chorus_get_depth_ms(synth->chorus);
}

int fluid_synth_get_chorus_type(fluid_synth_t* synth)
{
	return fluid_chorus_get_type(synth->chorus);
}

double fluid_synth_get_reverb_roomsize(fluid_synth_t* synth)
{
	return (double)fluid_revmodel_getroomsize(synth->reverb);
}

double fluid_synth_get_reverb_damp(fluid_synth_t* synth)
{
	return (double)fluid_revmodel_getdamp(synth->reverb);
}

double fluid_synth_get_reverb_level(fluid_synth_t* synth)
{
	return (double)fluid_revmodel_getlevel(synth->reverb);
}

double fluid_synth_get_reverb_width(fluid_synth_t* synth)
{
	return (double)fluid_revmodel_getwidth(synth->reverb);
}

void fluid_synth_release_voice_on_same_note(fluid_synth_t* synth, int chan, int key){
	int i;
	fluid_voice_t* voice;

	for (i = 0; i < synth->polyphony; i++) {
		voice = synth->voice[i];
		if (_PLAYING(voice)
			&& (voice->chan == chan)
			&& (voice->key == key)
			&& (fluid_voice_get_id(voice) != synth->noteid)) {
			fluid_voice_noteoff(voice);
		}
	}
}

int fluid_synth_set_interp_method(fluid_synth_t* synth, int chan, int interp_method){
	int i;
	for (i = 0; i < synth->midi_channels; i++) {
		if (synth->channel[i] == NULL){
			FLUID_LOG(FLUID_ERR, "Channels don't exist (yet)!");
			return FLUID_FAILED;
		};
		if (chan < 0 || fluid_channel_get_num(synth->channel[i]) == chan){
			fluid_channel_set_interp_method(synth->channel[i], interp_method);
		};
	};
	return FLUID_OK;
}

int fluid_synth_count_midi_channels(fluid_synth_t* synth)
{
	return synth->midi_channels;
}

int fluid_synth_count_audio_channels(fluid_synth_t* synth)
{
	return synth->audio_channels;
}

int fluid_synth_count_audio_groups(fluid_synth_t* synth)
{
	return synth->audio_groups;
}

int fluid_synth_count_effects_channels(fluid_synth_t* synth)
{
	return synth->effects_channels;
}

double fluid_synth_get_cpu_load(fluid_synth_t* synth)
{
	return synth->cpu_load;
}

static fluid_tuning_t*
fluid_synth_get_tuning(fluid_synth_t* synth, int bank, int prog)
{
	if ((bank < 0) || (bank >= 128)) {
		FLUID_LOG(FLUID_WARN, "Bank number out of range");
		return NULL;
	}
	if ((prog < 0) || (prog >= 128)) {
		FLUID_LOG(FLUID_WARN, "Program number out of range");
		return NULL;
	}
	if ((synth->tuning == NULL) ||
		(synth->tuning[bank] == NULL) ||
		(synth->tuning[bank][prog] == NULL)) {
		FLUID_LOG2(FLUID_WARN, "No tuning at bank %d, prog %d", bank, prog);
		return NULL;
	}
	return synth->tuning[bank][prog];
}

static fluid_tuning_t*
fluid_synth_create_tuning(fluid_synth_t* synth, int bank, int prog, char* name)
{
	if ((bank < 0) || (bank >= 128)) {
		FLUID_LOG(FLUID_WARN, "Bank number out of range");
		return NULL;
	}
	if ((prog < 0) || (prog >= 128)) {
		FLUID_LOG(FLUID_WARN, "Program number out of range");
		return NULL;
	}
	if (synth->tuning == NULL) {
		synth->tuning = FLUID_ARRAY(fluid_tuning_t**, 128);
		if (synth->tuning == NULL) {
			FLUID_LOG(FLUID_PANIC, "Out of memory");
			return NULL;
		}
		FLUID_MEMSET(synth->tuning, 0, 128 * sizeof(fluid_tuning_t**));
	}

	if (synth->tuning[bank] == NULL) {
		synth->tuning[bank] = FLUID_ARRAY(fluid_tuning_t*, 128);
		if (synth->tuning[bank] == NULL) {
			FLUID_LOG(FLUID_PANIC, "Out of memory");
			return NULL;
		}
		FLUID_MEMSET(synth->tuning[bank], 0, 128 * sizeof(fluid_tuning_t*));
	}

	if (synth->tuning[bank][prog] == NULL) {
		synth->tuning[bank][prog] = new_fluid_tuning(name, bank, prog);
		if (synth->tuning[bank][prog] == NULL) {
			return NULL;
		}
	}

	if ((fluid_tuning_get_name(synth->tuning[bank][prog]) == NULL)
		|| (FLUID_STRCMP(fluid_tuning_get_name(synth->tuning[bank][prog]), name) != 0)) {
		fluid_tuning_set_name(synth->tuning[bank][prog], name);
	}

	return synth->tuning[bank][prog];
}

int fluid_synth_create_key_tuning(fluid_synth_t* synth,
	int bank, int prog,
	char* name, double* pitch)
{
	fluid_tuning_t* tuning = fluid_synth_create_tuning(synth, bank, prog, name);
	if (tuning == NULL) {
		return FLUID_FAILED;
	}
	if (pitch) {
		fluid_tuning_set_all(tuning, pitch);
	}
	return FLUID_OK;
}

int fluid_synth_create_octave_tuning(fluid_synth_t* synth,
	int bank, int prog,
	char* name, double* pitch)
{
	fluid_tuning_t* tuning = fluid_synth_create_tuning(synth, bank, prog, name);
	if (tuning == NULL) {
		return FLUID_FAILED;
	}
	fluid_tuning_set_octave(tuning, pitch);
	return FLUID_OK;
}

int fluid_synth_tune_notes(fluid_synth_t* synth, int bank, int prog,
	int len, int *key, double* pitch, int apply)
{
	fluid_tuning_t* tuning = fluid_synth_get_tuning(synth, bank, prog);
	int i;

	if (tuning == NULL) {
		return FLUID_FAILED;
	}

	for (i = 0; i < len; i++) {
		fluid_tuning_set_pitch(tuning, key[i], pitch[i]);
	}

	return FLUID_OK;
}

int fluid_synth_select_tuning(fluid_synth_t* synth, int chan,
	int bank, int prog)
{
	fluid_tuning_t* tuning = fluid_synth_get_tuning(synth, bank, prog);

	if (tuning == NULL) {
		return FLUID_FAILED;
	}
	if ((chan < 0) || (chan >= synth->midi_channels)) {
		FLUID_LOG(FLUID_WARN, "Channel out of range");
		return FLUID_FAILED;
	}

	fluid_channel_set_tuning(synth->channel[chan], synth->tuning[bank][prog]);

	return FLUID_OK;
}

int fluid_synth_reset_tuning(fluid_synth_t* synth, int chan)
{
	if ((chan < 0) || (chan >= synth->midi_channels)) {
		FLUID_LOG(FLUID_WARN, "Channel out of range");
		return FLUID_FAILED;
	}

	fluid_channel_set_tuning(synth->channel[chan], NULL);

	return FLUID_OK;
}

void fluid_synth_tuning_iteration_start(fluid_synth_t* synth)
{
	synth->cur_tuning = NULL;
}

int fluid_synth_tuning_iteration_next(fluid_synth_t* synth, int* bank, int* prog)
{
	int b = 0, p = 0;

	if (synth->tuning == NULL) {
		return 0;
	}

	if (synth->cur_tuning != NULL) {
		b = fluid_tuning_get_bank(synth->cur_tuning);
		p = 1 + fluid_tuning_get_prog(synth->cur_tuning);
		if (p >= 128) {
			p = 0;
			b++;
		}
	}

	while (b < 128) {
		if (synth->tuning[b] != NULL) {
			while (p < 128) {
				if (synth->tuning[b][p] != NULL) {
					synth->cur_tuning = synth->tuning[b][p];
					*bank = b;
					*prog = p;
					return 1;
				}
				p++;
			}
		}
		p = 0;
		b++;
	}

	return 0;
}

int fluid_synth_tuning_dump(fluid_synth_t* synth, int bank, int prog,
	char* name, int len, double* pitch)
{
	fluid_tuning_t* tuning = fluid_synth_get_tuning(synth, bank, prog);

	if (tuning == NULL) {
		return FLUID_FAILED;
	}

	if (name) {
		snprintf(name, len - 1, "%s", fluid_tuning_get_name(tuning));
		name[len - 1] = 0;
	}
	if (pitch) {
		FLUID_MEMCPY(pitch, fluid_tuning_get_all(tuning), 128 * sizeof(double));
	}

	return FLUID_OK;
}

int fluid_synth_set_gen(fluid_synth_t* synth, int chan, int param, float value)
{
	int i;
	fluid_voice_t* voice;

	if ((chan < 0) || (chan >= synth->midi_channels)) {
		FLUID_LOG(FLUID_WARN, "Channel out of range");
		return FLUID_FAILED;
	}

	if ((param < 0) || (param >= GEN_LAST)) {
		FLUID_LOG(FLUID_WARN, "Parameter number out of range");
		return FLUID_FAILED;
	}

	fluid_channel_set_gen(synth->channel[chan], param, value, 0);

	for (i = 0; i < synth->polyphony; i++) {
		voice = synth->voice[i];
		if (voice->chan == chan) {
			fluid_voice_set_param(voice, param, value, 0);
		}
	}

	return FLUID_OK;
}

int fluid_synth_set_gen2(fluid_synth_t* synth, int chan, int param,
float value, int absolute, int normalized)
{
	int i;
	fluid_voice_t* voice;
	float v;

	if ((chan < 0) || (chan >= synth->midi_channels)) {
		FLUID_LOG(FLUID_WARN, "Channel out of range");
		return FLUID_FAILED;
	}

	if ((param < 0) || (param >= GEN_LAST)) {
		FLUID_LOG(FLUID_WARN, "Parameter number out of range");
		return FLUID_FAILED;
	}

	v = (normalized) ? fluid_gen_scale(param, value) : value;

	fluid_channel_set_gen(synth->channel[chan], param, v, absolute);

	for (i = 0; i < synth->polyphony; i++) {
		voice = synth->voice[i];
		if (voice->chan == chan) {
			fluid_voice_set_param(voice, param, v, absolute);
		}
	}

	return FLUID_OK;
}

float fluid_synth_get_gen(fluid_synth_t* synth, int chan, int param)
{
	if ((chan < 0) || (chan >= synth->midi_channels)) {
		FLUID_LOG(FLUID_WARN, "Channel out of range");
		return 0.0;
	}

	if ((param < 0) || (param >= GEN_LAST)) {
		FLUID_LOG(FLUID_WARN, "Parameter number out of range");
		return 0.0;
	}

	return fluid_channel_get_gen(synth->channel[chan], param);
}

int fluid_synth_handle_midi_event(void* data, fluid_midi_event_t* event)
{
	fluid_synth_t* synth = (fluid_synth_t*)data;
	int type = fluid_midi_event_get_type(event);
	int chan = fluid_midi_event_get_channel(event);

	switch (type) {
		case NOTE_ON:
			return fluid_synth_noteon(synth, chan,
				fluid_midi_event_get_key(event),
				fluid_midi_event_get_velocity(event));

		case NOTE_OFF:
			return fluid_synth_noteoff(synth, chan, fluid_midi_event_get_key(event));

		case CONTROL_CHANGE:
			return fluid_synth_cc(synth, chan,
				fluid_midi_event_get_control(event),
				fluid_midi_event_get_value(event));

		case PROGRAM_CHANGE:
			return fluid_synth_program_change(synth, chan, fluid_midi_event_get_program(event));

		case CHANNEL_PRESSURE:
			return fluid_synth_channel_pressure(synth, chan, fluid_midi_event_get_program(event));

		case PITCH_BEND:
			return fluid_synth_pitch_bend(synth, chan, fluid_midi_event_get_pitch(event));

		case MIDI_SYSTEM_RESET:
			return fluid_synth_system_reset(synth);
	}
	return FLUID_FAILED;
}

int fluid_synth_start(fluid_synth_t* synth, unsigned int id, fluid_preset_t* preset,
	int audio_chan, int midi_chan, int key, int vel)
{
	int r;

	if ((midi_chan < 0) || (midi_chan >= synth->midi_channels)) {
		FLUID_LOG(FLUID_WARN, "Channel out of range");
		return FLUID_FAILED;
	}

	if ((key < 0) || (key >= 128)) {
		FLUID_LOG(FLUID_WARN, "Key out of range");
		return FLUID_FAILED;
	}

	if ((vel <= 0) || (vel >= 128)) {
		FLUID_LOG(FLUID_WARN, "Velocity out of range");
		return FLUID_FAILED;
	}

	fluid_mutex_lock(synth->busy);

	synth->storeid = id;
	r = fluid_preset_noteon(preset, synth, midi_chan, key, vel);

	fluid_mutex_unlock(synth->busy);

	return r;
}

int fluid_synth_stop(fluid_synth_t* synth, unsigned int id)
{
	int i;
	fluid_voice_t* voice;
	int status = FLUID_FAILED;
	int count = 0;

	for (i = 0; i < synth->polyphony; i++) {
		voice = synth->voice[i];

		if (_ON(voice) && (fluid_voice_get_id(voice) == id)) {
			count++;
			fluid_voice_noteoff(voice);
			status = FLUID_OK;
		}
	}

	return status;
}

fluid_bank_offset_t*
fluid_synth_get_bank_offset0(fluid_synth_t* synth, int sfont_id)
{
	fluid_list_t* list = synth->bank_offsets;
	fluid_bank_offset_t* offset;

	while (list) {
		offset = (fluid_bank_offset_t*)fluid_list_get(list);
		if (offset->sfont_id == sfont_id) {
			return offset;
		}

		list = fluid_list_next(list);
	}

	return NULL;
}

int fluid_synth_set_bank_offset(fluid_synth_t* synth, int sfont_id, int offset)
{
	fluid_bank_offset_t* bank_offset;

	bank_offset = fluid_synth_get_bank_offset0(synth, sfont_id);

	if (bank_offset == NULL) {
		bank_offset = FLUID_NEW(fluid_bank_offset_t);
		if (bank_offset == NULL) {
			return -1;
		}
		bank_offset->sfont_id = sfont_id;
		bank_offset->offset = offset;
		synth->bank_offsets = fluid_list_prepend(synth->bank_offsets, bank_offset);
	}
	else {
		bank_offset->offset = offset;
	}

	return 0;
}

int fluid_synth_get_bank_offset(fluid_synth_t* synth, int sfont_id)
{
	fluid_bank_offset_t* bank_offset;

	bank_offset = fluid_synth_get_bank_offset0(synth, sfont_id);
	return (bank_offset == NULL) ? 0 : bank_offset->offset;
}

void fluid_synth_remove_bank_offset(fluid_synth_t* synth, int sfont_id)
{
	fluid_bank_offset_t* bank_offset;

	bank_offset = fluid_synth_get_bank_offset0(synth, sfont_id);
	if (bank_offset != NULL) {
		synth->bank_offsets = fluid_list_remove(synth->bank_offsets, bank_offset);
	}
}

//
//MERGED FILE END: fluid_synth.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: fluid_chan.c
//

#define SETCC(_c,_n,_v)  _c->cc[_n] = _v

fluid_channel_t*
new_fluid_channel(fluid_synth_t* synth, int num)
{
	fluid_channel_t* chan;

	chan = FLUID_NEW(fluid_channel_t);
	if (chan == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return NULL;
	}

	chan->synth = synth;
	chan->channum = num;
	chan->preset = NULL;

	fluid_channel_init(chan);
	fluid_channel_init_ctrl(chan);

	return chan;
}

void fluid_channel_init(fluid_channel_t* chan)
{
	chan->prognum = (chan->channum == 9) ? 0 : chan->channum;
	chan->banknum = (chan->channum == 9) ? 128 : 0;
	chan->sfontnum = 0;

	if (chan->preset) delete_fluid_preset(chan->preset);
	chan->preset = fluid_synth_find_preset(chan->synth, chan->banknum, chan->prognum);

	chan->interp_method = FLUID_INTERP_DEFAULT;
	chan->tuning = NULL;
	chan->nrpn_select = 0;
	chan->nrpn_active = 0;
}

void fluid_channel_init_ctrl(fluid_channel_t* chan)
{
	int i;

	chan->key_pressure = 0;
	chan->channel_pressure = 0;
	chan->pitch_bend = 0x2000;
	chan->pitch_wheel_sensitivity = 2;
	chan->bank_msb = 0;

	for (i = 0; i < GEN_LAST; i++) {
		chan->gen[i] = 0.0f;
		chan->gen_abs[i] = 0;
	}

	for (i = 0; i < 128; i++) {
		SETCC(chan, i, 0);
	}

	SETCC(chan, VOLUME_MSB, 127);
	SETCC(chan, VOLUME_LSB, 0);

	SETCC(chan, PAN_MSB, 64);
	SETCC(chan, PAN_LSB, 0);

	SETCC(chan, EXPRESSION_MSB, 127);
	SETCC(chan, EXPRESSION_LSB, 127);

	SETCC(chan, RPN_LSB, 127);
	SETCC(chan, RPN_MSB, 127);
}

void fluid_channel_reset(fluid_channel_t* chan)
{
	fluid_channel_init(chan);
	fluid_channel_init_ctrl(chan);
}

int delete_fluid_channel(fluid_channel_t* chan)
{
	if (chan->preset) delete_fluid_preset(chan->preset);
	FLUID_FREE(chan);
	return FLUID_OK;
}

int fluid_channel_set_preset(fluid_channel_t* chan, fluid_preset_t* preset)
{
	fluid_preset_notify(chan->preset, FLUID_PRESET_UNSELECTED, chan->channum);
	fluid_preset_notify(preset, FLUID_PRESET_SELECTED, chan->channum);

	if (chan->preset) delete_fluid_preset(chan->preset);
	chan->preset = preset;
	return FLUID_OK;
}

fluid_preset_t* fluid_channel_get_preset(fluid_channel_t* chan)
{
	return chan->preset;
}

unsigned int fluid_channel_get_banknum(fluid_channel_t* chan)
{
	return chan->banknum;
}

int fluid_channel_set_prognum(fluid_channel_t* chan, int prognum)
{
	chan->prognum = prognum;
	return FLUID_OK;
}

int fluid_channel_get_prognum(fluid_channel_t* chan)
{
	return chan->prognum;
}

int fluid_channel_set_banknum(fluid_channel_t* chan, unsigned int banknum)
{
	chan->banknum = banknum;
	return FLUID_OK;
}

int fluid_channel_cc(fluid_channel_t* chan, int num, int value)
{
	chan->cc[num] = value;

	switch (num) {
		case SUSTAIN_SWITCH:
		{
			if (value < 64) {
				fluid_synth_damp_voices(chan->synth, chan->channum);
			}
			else {
			}
		}
		break;

		case BANK_SELECT_MSB:
		{
			chan->bank_msb = (unsigned char)(value & 0x7f);

			fluid_channel_set_banknum(chan, (unsigned int)(value & 0x7f));
		}
		break;

		case BANK_SELECT_LSB:
		{
			fluid_channel_set_banknum(chan, (((unsigned int)value & 0x7f)
				+ ((unsigned int)chan->bank_msb << 7)));
		}
		break;

		case ALL_NOTES_OFF:
			fluid_synth_all_notes_off(chan->synth, chan->channum);
			break;

		case ALL_SOUND_OFF:
			fluid_synth_all_sounds_off(chan->synth, chan->channum);
			break;

		case ALL_CTRL_OFF:
			fluid_channel_init_ctrl(chan);
			fluid_synth_modulate_voices_all(chan->synth, chan->channum);
			break;

		case DATA_ENTRY_MSB:
		{
			int data = (value << 7) + chan->cc[DATA_ENTRY_LSB];

			if (chan->nrpn_active)
			{
				if ((chan->cc[NRPN_MSB] == 120) && (chan->cc[NRPN_LSB] < 100))
				{
					if (chan->nrpn_select < GEN_LAST)
					{
						float val = fluid_gen_scale_nrpn(chan->nrpn_select, data);
						fluid_synth_set_gen(chan->synth, chan->channum, chan->nrpn_select, val);
					}

					chan->nrpn_select = 0;
				}
			}
			else if (chan->cc[RPN_MSB] == 0)
			{
				switch (chan->cc[RPN_LSB])
				{
					case RPN_PITCH_BEND_RANGE:
						fluid_channel_pitch_wheel_sens(chan, value);
						break;
					case RPN_CHANNEL_FINE_TUNE:
						fluid_synth_set_gen(chan->synth, chan->channum, GEN_FINETUNE,
							(data - 8192) / 8192.0 * 50.0);
						break;
					case RPN_CHANNEL_COARSE_TUNE:
						fluid_synth_set_gen(chan->synth, chan->channum, GEN_COARSETUNE,
							value - 64);
						break;
					case RPN_TUNING_PROGRAM_CHANGE:
						break;
					case RPN_TUNING_BANK_SELECT:
						break;
					case RPN_MODULATION_DEPTH_RANGE:
						break;
				}
			}

			break;
		}

		case NRPN_MSB:
			chan->cc[NRPN_LSB] = 0;
			chan->nrpn_select = 0;
			chan->nrpn_active = 1;
			break;

		case NRPN_LSB:
			if (chan->cc[NRPN_MSB] == 120) {
				if (value == 100) {
					chan->nrpn_select += 100;
				}
				else if (value == 101) {
					chan->nrpn_select += 1000;
				}
				else if (value == 102) {
					chan->nrpn_select += 10000;
				}
				else if (value < 100) {
					chan->nrpn_select += value;
				}
			}

			chan->nrpn_active = 1;
			break;

		case RPN_MSB:
		case RPN_LSB:
			chan->nrpn_active = 0;
			break;

		default:
			fluid_synth_modulate_voices(chan->synth, chan->channum, 1, num);
	}

	return FLUID_OK;
}

int fluid_channel_get_cc(fluid_channel_t* chan, int num)
{
	return ((num >= 0) && (num < 128)) ? chan->cc[num] : 0;
}

int fluid_channel_pressure(fluid_channel_t* chan, int val)
{
	chan->channel_pressure = val;
	fluid_synth_modulate_voices(chan->synth, chan->channum, 0, FLUID_MOD_CHANNELPRESSURE);
	return FLUID_OK;
}

int fluid_channel_pitch_bend(fluid_channel_t* chan, int val)
{
	chan->pitch_bend = val;
	fluid_synth_modulate_voices(chan->synth, chan->channum, 0, FLUID_MOD_PITCHWHEEL);
	return FLUID_OK;
}

int fluid_channel_pitch_wheel_sens(fluid_channel_t* chan, int val)
{
	chan->pitch_wheel_sensitivity = val;
	fluid_synth_modulate_voices(chan->synth, chan->channum, 0, FLUID_MOD_PITCHWHEELSENS);
	return FLUID_OK;
}

int fluid_channel_get_num(fluid_channel_t* chan)
{
	return chan->channum;
}

void fluid_channel_set_interp_method(fluid_channel_t* chan, int new_method)
{
	chan->interp_method = new_method;
}

int fluid_channel_get_interp_method(fluid_channel_t* chan)
{
	return chan->interp_method;
}

unsigned int fluid_channel_get_sfontnum(fluid_channel_t* chan)
{
	return chan->sfontnum;
}

int fluid_channel_set_sfontnum(fluid_channel_t* chan, unsigned int sfontnum)
{
	chan->sfontnum = sfontnum;
	return FLUID_OK;
}

//
//MERGED FILE END: fluid_chan.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: fluid_chorus.c
//

#define MAX_CHORUS	99
#define MAX_DELAY	100
#define MAX_DEPTH	10
#define MIN_SPEED_HZ	0.29
#define MAX_SPEED_HZ    5

#define MAX_SAMPLES_LN2 12

#define MAX_SAMPLES (1 << (MAX_SAMPLES_LN2-1))
#define MAX_SAMPLES_ANDMASK (MAX_SAMPLES-1)

#define INTERPOLATION_SUBSAMPLES_LN2 8
#define INTERPOLATION_SUBSAMPLES (1 << (INTERPOLATION_SUBSAMPLES_LN2-1))
#define INTERPOLATION_SUBSAMPLES_ANDMASK (INTERPOLATION_SUBSAMPLES-1)

#define INTERPOLATION_SAMPLES 5

struct _fluid_chorus_t {
	int type;
	int new_type;
	fluid_real_t depth_ms;
	fluid_real_t new_depth_ms;
	fluid_real_t level;
	fluid_real_t new_level;
	fluid_real_t speed_Hz;
	fluid_real_t new_speed_Hz;
	int number_blocks;
	int new_number_blocks;

	fluid_real_t *chorusbuf;
	int counter;
	long phase[MAX_CHORUS];
	long modulation_period_samples;
	int *lookup_tab;
	fluid_real_t sample_rate;

	fluid_real_t sinc_table[INTERPOLATION_SAMPLES][INTERPOLATION_SUBSAMPLES];
};

void fluid_chorus_triangle(int *buf, int len, int depth);
void fluid_chorus_sine(int *buf, int len, int depth);

fluid_chorus_t*
new_fluid_chorus(fluid_real_t sample_rate)
{
	int i; int ii;
	fluid_chorus_t* chorus;

	chorus = FLUID_NEW(fluid_chorus_t);
	if (chorus == NULL) {
		fluid_log(FLUID_PANIC, "chorus: Out of memory");
		return NULL;
	}

	FLUID_MEMSET(chorus, 0, sizeof(fluid_chorus_t));

	chorus->sample_rate = sample_rate;

	for (i = 0; i < INTERPOLATION_SAMPLES; i++){
		for (ii = 0; ii < INTERPOLATION_SUBSAMPLES; ii++){
			double i_shifted = ((double)i - ((double)INTERPOLATION_SAMPLES) / 2.
				+ (double)ii / (double)INTERPOLATION_SUBSAMPLES);
			if (fabs(i_shifted) < 0.000001) {
				chorus->sinc_table[i][ii] = (fluid_real_t)1.;

			}
			else {
				chorus->sinc_table[i][ii] = (fluid_real_t)sin(i_shifted * M_PI) / (M_PI * i_shifted);

				chorus->sinc_table[i][ii] *= (fluid_real_t)0.5 * (1.0 + cos(2.0 * M_PI * i_shifted / (fluid_real_t)INTERPOLATION_SAMPLES));
			};
		};
	};

	chorus->lookup_tab = FLUID_ARRAY(int, (int)(chorus->sample_rate / MIN_SPEED_HZ));
	if (chorus->lookup_tab == NULL) {
		fluid_log(FLUID_PANIC, "chorus: Out of memory");
		goto error_recovery;
	}

	chorus->chorusbuf = FLUID_ARRAY(fluid_real_t, MAX_SAMPLES);
	if (chorus->chorusbuf == NULL) {
		fluid_log(FLUID_PANIC, "chorus: Out of memory");
		goto error_recovery;
	}

	if (fluid_chorus_init(chorus) != FLUID_OK){
		goto error_recovery;
	};

	return chorus;

error_recovery:
	delete_fluid_chorus(chorus);
	return NULL;
}

int fluid_chorus_init(fluid_chorus_t* chorus)
{
	int i;

	for (i = 0; i < MAX_SAMPLES; i++) {
		chorus->chorusbuf[i] = 0.0;
	}

	fluid_chorus_set_nr(chorus, FLUID_CHORUS_DEFAULT_N);
	fluid_chorus_set_level(chorus, FLUID_CHORUS_DEFAULT_LEVEL);
	fluid_chorus_set_speed_Hz(chorus, FLUID_CHORUS_DEFAULT_SPEED);
	fluid_chorus_set_depth_ms(chorus, FLUID_CHORUS_DEFAULT_DEPTH);
	fluid_chorus_set_type(chorus, FLUID_CHORUS_MOD_SINE);

	return fluid_chorus_update(chorus);
}

void fluid_chorus_set_nr(fluid_chorus_t* chorus, int nr)
{
	chorus->new_number_blocks = nr;
}

int fluid_chorus_get_nr(fluid_chorus_t* chorus)
{
	return chorus->number_blocks;
}

void fluid_chorus_set_level(fluid_chorus_t* chorus, fluid_real_t level)
{
	chorus->new_level = level;
}

fluid_real_t fluid_chorus_get_level(fluid_chorus_t* chorus)
{
	return chorus->level;
}

void fluid_chorus_set_speed_Hz(fluid_chorus_t* chorus, fluid_real_t speed_Hz)
{
	chorus->new_speed_Hz = speed_Hz;
}

fluid_real_t fluid_chorus_get_speed_Hz(fluid_chorus_t* chorus)
{
	return chorus->speed_Hz;
}

void fluid_chorus_set_depth_ms(fluid_chorus_t* chorus, fluid_real_t depth_ms)
{
	chorus->new_depth_ms = depth_ms;
}

fluid_real_t fluid_chorus_get_depth_ms(fluid_chorus_t* chorus)
{
	return chorus->depth_ms;
}

void fluid_chorus_set_type(fluid_chorus_t* chorus, int type)
{
	chorus->new_type = type;
}

int fluid_chorus_get_type(fluid_chorus_t* chorus)
{
	return chorus->type;
}

void delete_fluid_chorus(fluid_chorus_t* chorus)
{
	if (chorus == NULL) {
		return;
	}

	if (chorus->chorusbuf != NULL) {
		FLUID_FREE(chorus->chorusbuf);
	}

	if (chorus->lookup_tab != NULL) {
		FLUID_FREE(chorus->lookup_tab);
	}

	FLUID_FREE(chorus);
}

int fluid_chorus_update(fluid_chorus_t* chorus)
{
	int i;
	int modulation_depth_samples;

	if (chorus->new_number_blocks < 0) {
		fluid_log(FLUID_WARN, "chorus: number blocks must be >=0! Setting value to 0.");
		chorus->new_number_blocks = 0;
	}
	else if (chorus->new_number_blocks > MAX_CHORUS) {
		fluid_log(FLUID_WARN, "chorus: number blocks larger than max. allowed! Setting value to %d.",
			MAX_CHORUS);
		chorus->new_number_blocks = MAX_CHORUS;
	};

	if (chorus->new_speed_Hz < MIN_SPEED_HZ) {
		fluid_log(FLUID_WARN, "chorus: speed is too low (min %f)! Setting value to min.",
			(double)MIN_SPEED_HZ);
		chorus->new_speed_Hz = MIN_SPEED_HZ;
	}
	else if (chorus->new_speed_Hz > MAX_SPEED_HZ) {
		fluid_log(FLUID_WARN, "chorus: speed must be below %f Hz! Setting value to max.",
			(double)MAX_SPEED_HZ);
		chorus->new_speed_Hz = MAX_SPEED_HZ;
	}
	if (chorus->new_depth_ms < 0.0) {
		fluid_log(FLUID_WARN, "chorus: depth must be positive! Setting value to 0.");
		chorus->new_depth_ms = 0.0;
	}

	if (chorus->new_level < 0.0) {
		fluid_log(FLUID_WARN, "chorus: level must be positive! Setting value to 0.");
		chorus->new_level = 0.0;
	}
	else if (chorus->new_level > 10) {
		fluid_log(FLUID_WARN, "chorus: level must be < 10. A reasonable level is << 1! "
			"Setting it to 0.1.");
		chorus->new_level = 0.1;
	}

	chorus->modulation_period_samples = chorus->sample_rate / chorus->new_speed_Hz;

	modulation_depth_samples = (int)
		(chorus->new_depth_ms / 1000.0
		* chorus->sample_rate);

	if (modulation_depth_samples > MAX_SAMPLES) {
		fluid_log(FLUID_WARN, "chorus: Too high depth. Setting it to max (%d).", MAX_SAMPLES);
		modulation_depth_samples = MAX_SAMPLES;
	}

	if (chorus->type == FLUID_CHORUS_MOD_SINE) {
		fluid_chorus_sine(chorus->lookup_tab, chorus->modulation_period_samples,
			modulation_depth_samples);
	}
	else if (chorus->type == FLUID_CHORUS_MOD_TRIANGLE) {
		fluid_chorus_triangle(chorus->lookup_tab, chorus->modulation_period_samples,
			modulation_depth_samples);
	}
	else {
		fluid_log(FLUID_WARN, "chorus: Unknown modulation type. Using sinewave.");
		chorus->type = FLUID_CHORUS_MOD_SINE;
		fluid_chorus_sine(chorus->lookup_tab, chorus->modulation_period_samples,
			modulation_depth_samples);
	};

	for (i = 0; i < chorus->number_blocks; i++) {
		chorus->phase[i] = (int)((double)chorus->modulation_period_samples
			* (double)i / (double)chorus->number_blocks);
	}

	chorus->counter = 0;

	chorus->type = chorus->new_type;
	chorus->depth_ms = chorus->new_depth_ms;
	chorus->level = chorus->new_level;
	chorus->speed_Hz = chorus->new_speed_Hz;
	chorus->number_blocks = chorus->new_number_blocks;
	return FLUID_OK;

}

void fluid_chorus_processmix(fluid_chorus_t* chorus, fluid_real_t *in,
	fluid_real_t *left_out, fluid_real_t *right_out)
{
	int sample_index;
	int i;
	fluid_real_t d_in, d_out;

	for (sample_index = 0; sample_index < FLUID_BUFSIZE; sample_index++) {
		d_in = in[sample_index];
		d_out = 0.0f;

# if 0

		left_out[sample_index] = 0;
		right_out[sample_index] = 0;
#endif

		chorus->chorusbuf[chorus->counter] = d_in;

		for (i = 0; i < chorus->number_blocks; i++) {
			int ii;

			int pos_subsamples = (INTERPOLATION_SUBSAMPLES * chorus->counter
				- chorus->lookup_tab[chorus->phase[i]]);

			int pos_samples = pos_subsamples / INTERPOLATION_SUBSAMPLES;

			pos_subsamples &= INTERPOLATION_SUBSAMPLES_ANDMASK;

			for (ii = 0; ii < INTERPOLATION_SAMPLES; ii++){
				d_out += chorus->chorusbuf[pos_samples & MAX_SAMPLES_ANDMASK]
					* chorus->sinc_table[ii][pos_subsamples];

				pos_samples--;
			};

			chorus->phase[i]++;
			chorus->phase[i] %= (chorus->modulation_period_samples);
		}

		d_out *= chorus->level;

		left_out[sample_index] += d_out;
		right_out[sample_index] += d_out;

		chorus->counter++;
		chorus->counter %= MAX_SAMPLES;

	}
}

void fluid_chorus_processreplace(fluid_chorus_t* chorus, fluid_real_t *in,
	fluid_real_t *left_out, fluid_real_t *right_out)
{
	int sample_index;
	int i;
	fluid_real_t d_in, d_out;

	for (sample_index = 0; sample_index < FLUID_BUFSIZE; sample_index++) {
		d_in = in[sample_index];
		d_out = 0.0f;

# if 0

		left_out[sample_index] = 0;
		right_out[sample_index] = 0;
#endif

		chorus->chorusbuf[chorus->counter] = d_in;

		for (i = 0; i < chorus->number_blocks; i++) {
			int ii;

			int pos_subsamples = (INTERPOLATION_SUBSAMPLES * chorus->counter
				- chorus->lookup_tab[chorus->phase[i]]);

			int pos_samples = pos_subsamples / INTERPOLATION_SUBSAMPLES;

			pos_subsamples &= INTERPOLATION_SUBSAMPLES_ANDMASK;

			for (ii = 0; ii < INTERPOLATION_SAMPLES; ii++){
				d_out += chorus->chorusbuf[pos_samples & MAX_SAMPLES_ANDMASK]
					* chorus->sinc_table[ii][pos_subsamples];

				pos_samples--;
			};

			chorus->phase[i]++;
			chorus->phase[i] %= (chorus->modulation_period_samples);
		}

		d_out *= chorus->level;

		left_out[sample_index] = d_out;
		right_out[sample_index] = d_out;

		chorus->counter++;
		chorus->counter %= MAX_SAMPLES;

	}
}

void fluid_chorus_sine(int *buf, int len, int depth)
{
	int i;
	double val;

	for (i = 0; i < len; i++) {
		val = sin((double)i / (double)len * 2.0 * M_PI);
		buf[i] = (int)((1.0 + val) * (double)depth / 2.0 * (double)INTERPOLATION_SUBSAMPLES);
		buf[i] -= 3 * MAX_SAMPLES * INTERPOLATION_SUBSAMPLES;
	}
}

void fluid_chorus_triangle(int *buf, int len, int depth)
{
	int i = 0;
	int ii = len - 1;
	double val;
	double val2;

	while (i <= ii){
		val = i * 2.0 / len * (double)depth * (double)INTERPOLATION_SUBSAMPLES;
		val2 = (int)(val + 0.5) - 3 * MAX_SAMPLES * INTERPOLATION_SUBSAMPLES;
		buf[i++] = (int)val2;
		buf[ii--] = (int)val2;
	}
}

void fluid_chorus_reset(fluid_chorus_t* chorus)
{
	fluid_chorus_init(chorus);
}

//
//MERGED FILE END: fluid_chorus.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: fluid_conv.c
//

fluid_real_t fluid_ct2hz_tab[FLUID_CENTS_HZ_SIZE];
fluid_real_t fluid_cb2amp_tab[FLUID_CB_AMP_SIZE];
fluid_real_t fluid_atten2amp_tab[FLUID_ATTEN_AMP_SIZE];
fluid_real_t fluid_posbp_tab[128];
fluid_real_t fluid_concave_tab[128];
fluid_real_t fluid_convex_tab[128];
fluid_real_t fluid_pan_tab[FLUID_PAN_SIZE];

void fluid_conversion_config(void)
{
	int i;
	double x;

	for (i = 0; i < FLUID_CENTS_HZ_SIZE; i++) {
		fluid_ct2hz_tab[i] = (fluid_real_t)pow(2.0, (double)i / 1200.0);
	}

	for (i = 0; i < FLUID_CB_AMP_SIZE; i++) {
		fluid_cb2amp_tab[i] = (fluid_real_t)pow(10.0, (double)i / -200.0);
	}

	for (i = 0; i < FLUID_ATTEN_AMP_SIZE; i++) {
		fluid_atten2amp_tab[i] = (fluid_real_t)pow(10.0, (double)i / FLUID_ATTEN_POWER_FACTOR);
	}

	fluid_concave_tab[0] = 0.0;
	fluid_concave_tab[127] = 1.0;

	fluid_convex_tab[0] = 0;
	fluid_convex_tab[127] = 1.0;
	x = log10(128.0 / 127.0);

	for (i = 1; i < 127; i++) {
		x = -20.0 / 96.0 * log((i * i) / (127.0 * 127.0)) / log(10.0);
		fluid_convex_tab[i] = (fluid_real_t)(1.0 - x);
		fluid_concave_tab[127 - i] = (fluid_real_t)x;
	}

	x = PI / 2.0 / (FLUID_PAN_SIZE - 1.0);
	for (i = 0; i < FLUID_PAN_SIZE; i++) {
		fluid_pan_tab[i] = (fluid_real_t)sin(i * x);
	}
}

fluid_real_t
fluid_ct2hz_real(fluid_real_t cents)
{
	if (cents < 0)
		return (fluid_real_t) 1.0;
	else if (cents < 900) {
		return (fluid_real_t) 6.875 * fluid_ct2hz_tab[(int)(cents + 300)];
	}
	else if (cents < 2100) {
		return (fluid_real_t) 13.75 * fluid_ct2hz_tab[(int)(cents - 900)];
	}
	else if (cents < 3300) {
		return (fluid_real_t) 27.5 * fluid_ct2hz_tab[(int)(cents - 2100)];
	}
	else if (cents < 4500) {
		return (fluid_real_t) 55.0 * fluid_ct2hz_tab[(int)(cents - 3300)];
	}
	else if (cents < 5700) {
		return (fluid_real_t) 110.0 * fluid_ct2hz_tab[(int)(cents - 4500)];
	}
	else if (cents < 6900) {
		return (fluid_real_t) 220.0 * fluid_ct2hz_tab[(int)(cents - 5700)];
	}
	else if (cents < 8100) {
		return (fluid_real_t) 440.0 * fluid_ct2hz_tab[(int)(cents - 6900)];
	}
	else if (cents < 9300) {
		return (fluid_real_t) 880.0 * fluid_ct2hz_tab[(int)(cents - 8100)];
	}
	else if (cents < 10500) {
		return (fluid_real_t) 1760.0 * fluid_ct2hz_tab[(int)(cents - 9300)];
	}
	else if (cents < 11700) {
		return (fluid_real_t) 3520.0 * fluid_ct2hz_tab[(int)(cents - 10500)];
	}
	else if (cents < 12900) {
		return (fluid_real_t) 7040.0 * fluid_ct2hz_tab[(int)(cents - 11700)];
	}
	else if (cents < 14100) {
		return (fluid_real_t) 14080.0 * fluid_ct2hz_tab[(int)(cents - 12900)];
	}
	else {
		return (fluid_real_t) 1.0;
	}
}

fluid_real_t
fluid_ct2hz(fluid_real_t cents)
{
	if (cents >= 13500){
		cents = 13500;
	}
	else if (cents < 1500){
		cents = 1500;
	}
	return fluid_ct2hz_real(cents);
}

fluid_real_t
fluid_cb2amp(fluid_real_t cb)
{
	if (cb < 0) {
		return 1.0;
	}
	if (cb >= FLUID_CB_AMP_SIZE) {
		return 0.0;
	}
	return fluid_cb2amp_tab[(int)cb];
}

fluid_real_t
fluid_atten2amp(fluid_real_t atten)
{
	if (atten < 0) return 1.0;
	else if (atten >= FLUID_ATTEN_AMP_SIZE) return 0.0;
	else return fluid_atten2amp_tab[(int)atten];
}

fluid_real_t
fluid_tc2sec_delay(fluid_real_t tc)
{
	if (tc <= -32768.0f) {
		return (fluid_real_t) 0.0f;
	};
	if (tc < -12000.) {
		tc = (fluid_real_t)-12000.0f;
	}
	if (tc > 5000.0f) {
		tc = (fluid_real_t) 5000.0f;
	}
	return (fluid_real_t)pow(2.0, (double)tc / 1200.0);
}

fluid_real_t
fluid_tc2sec_attack(fluid_real_t tc)
{
	if (tc <= -32768.){ return (fluid_real_t) 0.0; };
	if (tc < -12000.){ tc = (fluid_real_t)-12000.0; };
	if (tc > 8000.){ tc = (fluid_real_t) 8000.0; };
	return (fluid_real_t)pow(2.0, (double)tc / 1200.0);
}

fluid_real_t
fluid_tc2sec(fluid_real_t tc)
{
	return (fluid_real_t)pow(2.0, (double)tc / 1200.0);
}

fluid_real_t
fluid_tc2sec_release(fluid_real_t tc)
{
	if (tc <= -32768.){ return (fluid_real_t) 0.0; };
	if (tc < -12000.){ tc = (fluid_real_t)-12000.0; };
	if (tc > 8000.){ tc = (fluid_real_t) 8000.0; };
	return (fluid_real_t)pow(2.0, (double)tc / 1200.0);
}

fluid_real_t
fluid_act2hz(fluid_real_t c)
{
	return (fluid_real_t)(8.176 * pow(2.0, (double)c / 1200.0));
}

fluid_real_t
fluid_hz2ct(fluid_real_t f)
{
	return (fluid_real_t)(6900 + 1200 * log(f / 440.0) / log(2.0));
}

fluid_real_t
fluid_pan(fluid_real_t c, int left)
{
	if (left) {
		c = -c;
	}
	if (c < -500) {
		return (fluid_real_t) 0.0;
	}
	else if (c > 500) {
		return (fluid_real_t) 1.0;
	}
	else {
		return fluid_pan_tab[(int)(c + 500)];
	}
}

fluid_real_t
fluid_concave(fluid_real_t val)
{
	if (val < 0) {
		return 0;
	}
	else if (val > 127) {
		return 1;
	}
	return fluid_concave_tab[(int)val];
}

fluid_real_t
fluid_convex(fluid_real_t val)
{
	if (val < 0) {
		return 0;
	}
	else if (val > 127) {
		return 1;
	}
	return fluid_convex_tab[(int)val];
}

//
//MERGED FILE END: fluid_conv.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: fluid_dsp_float.c
//

static fluid_real_t interp_coeff_linear[FLUID_INTERP_MAX][2];

static fluid_real_t interp_coeff[FLUID_INTERP_MAX][4];

static fluid_real_t sinc_table7[FLUID_INTERP_MAX][7];

#define SINC_INTERP_ORDER 7

void fluid_dsp_float_config(void)
{
	int i, i2;
	double x, v;
	double i_shifted;

	for (i = 0; i < FLUID_INTERP_MAX; i++)
	{
		x = (double)i / (double)FLUID_INTERP_MAX;

		interp_coeff[i][0] = (fluid_real_t)(x * (-0.5 + x * (1 - 0.5 * x)));
		interp_coeff[i][1] = (fluid_real_t)(1.0 + x * x * (1.5 * x - 2.5));
		interp_coeff[i][2] = (fluid_real_t)(x * (0.5 + x * (2.0 - 1.5 * x)));
		interp_coeff[i][3] = (fluid_real_t)(0.5 * x * x * (x - 1.0));

		interp_coeff_linear[i][0] = (fluid_real_t)(1.0 - x);
		interp_coeff_linear[i][1] = (fluid_real_t)x;
	}

	for (i = 0; i < SINC_INTERP_ORDER; i++)
	{
		for (i2 = 0; i2 < FLUID_INTERP_MAX; i2++)
		{
			i_shifted = (double)i - ((double)SINC_INTERP_ORDER / 2.0)
				+ (double)i2 / (double)FLUID_INTERP_MAX;

			if (fabs(i_shifted) > 0.000001)
			{
				v = (fluid_real_t)sin(i_shifted * M_PI) / (M_PI * i_shifted);

				v *= (fluid_real_t)0.5 * (1.0 + cos(2.0 * M_PI * i_shifted / (fluid_real_t)SINC_INTERP_ORDER));
			}
			else v = 1.0;

			sinc_table7[FLUID_INTERP_MAX - i2 - 1][i] = v;
		}
	}

#if 0
	for (i = 0; i < FLUID_INTERP_MAX; i++)
	{
		printf("%d %0.3f %0.3f %0.3f %0.3f %0.3f %0.3f %0.3f\n",
			i, sinc_table7[0][i], sinc_table7[1][i], sinc_table7[2][i],
			sinc_table7[3][i], sinc_table7[4][i], sinc_table7[5][i], sinc_table7[6][i]);
	}
#endif

	fluid_check_fpe("interpolation table calculation");
}

int fluid_dsp_float_interpolate_none(fluid_voice_t *voice)
{
	fluid_phase_t dsp_phase = voice->phase;
	fluid_phase_t dsp_phase_incr;   short int *dsp_data = voice->sample->data;
	fluid_real_t *dsp_buf = voice->dsp_buf;
	fluid_real_t dsp_amp = voice->amp;
	fluid_real_t dsp_amp_incr = voice->amp_incr;
	unsigned int dsp_i = 0;
	unsigned int dsp_phase_index;
	unsigned int end_index;
	int looping;

	fluid_phase_set_float(dsp_phase_incr, voice->phase_incr);

	looping = _SAMPLEMODE(voice) == FLUID_LOOP_DURING_RELEASE
		|| (_SAMPLEMODE(voice) == FLUID_LOOP_UNTIL_RELEASE
		&& voice->volenv_section < FLUID_VOICE_ENVRELEASE);

	end_index = looping ? voice->loopend - 1 : voice->end;

	while (1)
	{
		dsp_phase_index = fluid_phase_index_round(dsp_phase);

		for (; dsp_i < FLUID_BUFSIZE && dsp_phase_index <= end_index; dsp_i++)
		{
			dsp_buf[dsp_i] = dsp_amp * dsp_data[dsp_phase_index];

			fluid_phase_incr(dsp_phase, dsp_phase_incr);
			dsp_phase_index = fluid_phase_index_round(dsp_phase);
			dsp_amp += dsp_amp_incr;
		}

		if (!looping) break;

		if (dsp_phase_index > end_index)
		{
			fluid_phase_sub_int(dsp_phase, voice->loopend - voice->loopstart);
			voice->has_looped = 1;
		}

		if (dsp_i >= FLUID_BUFSIZE) break;
	}

	voice->phase = dsp_phase;
	voice->amp = dsp_amp;

	return (dsp_i);
}

int fluid_dsp_float_interpolate_linear(fluid_voice_t *voice)
{
	fluid_phase_t dsp_phase = voice->phase;
	fluid_phase_t dsp_phase_incr;   short int *dsp_data = voice->sample->data;
	fluid_real_t *dsp_buf = voice->dsp_buf;
	fluid_real_t dsp_amp = voice->amp;
	fluid_real_t dsp_amp_incr = voice->amp_incr;
	unsigned int dsp_i = 0;
	unsigned int dsp_phase_index;
	unsigned int end_index;
	short int point;
	fluid_real_t *coeffs;
	int looping;

	fluid_phase_set_float(dsp_phase_incr, voice->phase_incr);

	looping = _SAMPLEMODE(voice) == FLUID_LOOP_DURING_RELEASE
		|| (_SAMPLEMODE(voice) == FLUID_LOOP_UNTIL_RELEASE
		&& voice->volenv_section < FLUID_VOICE_ENVRELEASE);

	end_index = (looping ? voice->loopend - 1 : voice->end) - 1;

	if (looping) point = dsp_data[voice->loopstart];
	else point = dsp_data[voice->end];

	while (1)
	{
		dsp_phase_index = fluid_phase_index(dsp_phase);

		for (; dsp_i < FLUID_BUFSIZE && dsp_phase_index <= end_index; dsp_i++)
		{
			coeffs = interp_coeff_linear[fluid_phase_fract_to_tablerow(dsp_phase)];
			dsp_buf[dsp_i] = dsp_amp * (coeffs[0] * dsp_data[dsp_phase_index]
				+ coeffs[1] * dsp_data[dsp_phase_index + 1]);

			fluid_phase_incr(dsp_phase, dsp_phase_incr);
			dsp_phase_index = fluid_phase_index(dsp_phase);
			dsp_amp += dsp_amp_incr;
		}

		if (dsp_i >= FLUID_BUFSIZE) break;

		end_index++;

		for (; dsp_phase_index <= end_index && dsp_i < FLUID_BUFSIZE; dsp_i++)
		{
			coeffs = interp_coeff_linear[fluid_phase_fract_to_tablerow(dsp_phase)];
			dsp_buf[dsp_i] = dsp_amp * (coeffs[0] * dsp_data[dsp_phase_index]
				+ coeffs[1] * point);

			fluid_phase_incr(dsp_phase, dsp_phase_incr);
			dsp_phase_index = fluid_phase_index(dsp_phase);
			dsp_amp += dsp_amp_incr;
		}

		if (!looping) break;

		if (dsp_phase_index > end_index)
		{
			fluid_phase_sub_int(dsp_phase, voice->loopend - voice->loopstart);
			voice->has_looped = 1;
		}

		if (dsp_i >= FLUID_BUFSIZE) break;

		end_index--;
	}

	voice->phase = dsp_phase;
	voice->amp = dsp_amp;

	return (dsp_i);
}

int fluid_dsp_float_interpolate_4th_order(fluid_voice_t *voice)
{
	fluid_phase_t dsp_phase = voice->phase;
	fluid_phase_t dsp_phase_incr;   short int *dsp_data = voice->sample->data;
	fluid_real_t *dsp_buf = voice->dsp_buf;
	fluid_real_t dsp_amp = voice->amp;
	fluid_real_t dsp_amp_incr = voice->amp_incr;
	unsigned int dsp_i = 0;
	unsigned int dsp_phase_index;
	unsigned int start_index, end_index;
	short int start_point, end_point1, end_point2;
	fluid_real_t *coeffs;
	int looping;

	fluid_phase_set_float(dsp_phase_incr, voice->phase_incr);

	looping = _SAMPLEMODE(voice) == FLUID_LOOP_DURING_RELEASE
		|| (_SAMPLEMODE(voice) == FLUID_LOOP_UNTIL_RELEASE
		&& voice->volenv_section < FLUID_VOICE_ENVRELEASE);

	end_index = (looping ? voice->loopend - 1 : voice->end) - 2;

	if (voice->has_looped)
	{
		start_index = voice->loopstart;
		start_point = dsp_data[voice->loopend - 1];
	}
	else
	{
		start_index = voice->start;
		start_point = dsp_data[voice->start];
	}

	if (looping)
	{
		end_point1 = dsp_data[voice->loopstart];
		end_point2 = dsp_data[voice->loopstart + 1];
	}
	else
	{
		end_point1 = dsp_data[voice->end];
		end_point2 = end_point1;
	}

	while (1)
	{
		dsp_phase_index = fluid_phase_index(dsp_phase);

		for (; dsp_phase_index == start_index && dsp_i < FLUID_BUFSIZE; dsp_i++)
		{
			coeffs = interp_coeff[fluid_phase_fract_to_tablerow(dsp_phase)];
			dsp_buf[dsp_i] = dsp_amp * (coeffs[0] * start_point
				+ coeffs[1] * dsp_data[dsp_phase_index]
				+ coeffs[2] * dsp_data[dsp_phase_index + 1]
				+ coeffs[3] * dsp_data[dsp_phase_index + 2]);

			fluid_phase_incr(dsp_phase, dsp_phase_incr);
			dsp_phase_index = fluid_phase_index(dsp_phase);
			dsp_amp += dsp_amp_incr;
		}

		for (; dsp_i < FLUID_BUFSIZE && dsp_phase_index <= end_index; dsp_i++)
		{
			coeffs = interp_coeff[fluid_phase_fract_to_tablerow(dsp_phase)];
			dsp_buf[dsp_i] = dsp_amp * (coeffs[0] * dsp_data[dsp_phase_index - 1]
				+ coeffs[1] * dsp_data[dsp_phase_index]
				+ coeffs[2] * dsp_data[dsp_phase_index + 1]
				+ coeffs[3] * dsp_data[dsp_phase_index + 2]);

			fluid_phase_incr(dsp_phase, dsp_phase_incr);
			dsp_phase_index = fluid_phase_index(dsp_phase);
			dsp_amp += dsp_amp_incr;
		}

		if (dsp_i >= FLUID_BUFSIZE) break;

		end_index++;

		for (; dsp_phase_index <= end_index && dsp_i < FLUID_BUFSIZE; dsp_i++)
		{
			coeffs = interp_coeff[fluid_phase_fract_to_tablerow(dsp_phase)];
			dsp_buf[dsp_i] = dsp_amp * (coeffs[0] * dsp_data[dsp_phase_index - 1]
				+ coeffs[1] * dsp_data[dsp_phase_index]
				+ coeffs[2] * dsp_data[dsp_phase_index + 1]
				+ coeffs[3] * end_point1);

			fluid_phase_incr(dsp_phase, dsp_phase_incr);
			dsp_phase_index = fluid_phase_index(dsp_phase);
			dsp_amp += dsp_amp_incr;
		}

		end_index++;

		for (; dsp_phase_index <= end_index && dsp_i < FLUID_BUFSIZE; dsp_i++)
		{
			coeffs = interp_coeff[fluid_phase_fract_to_tablerow(dsp_phase)];
			dsp_buf[dsp_i] = dsp_amp * (coeffs[0] * dsp_data[dsp_phase_index - 1]
				+ coeffs[1] * dsp_data[dsp_phase_index]
				+ coeffs[2] * end_point1
				+ coeffs[3] * end_point2);

			fluid_phase_incr(dsp_phase, dsp_phase_incr);
			dsp_phase_index = fluid_phase_index(dsp_phase);
			dsp_amp += dsp_amp_incr;
		}

		if (!looping) break;

		if (dsp_phase_index > end_index)
		{
			fluid_phase_sub_int(dsp_phase, voice->loopend - voice->loopstart);

			if (!voice->has_looped)
			{
				voice->has_looped = 1;
				start_index = voice->loopstart;
				start_point = dsp_data[voice->loopend - 1];
			}
		}

		if (dsp_i >= FLUID_BUFSIZE) break;

		end_index -= 2;
	}

	voice->phase = dsp_phase;
	voice->amp = dsp_amp;

	return (dsp_i);
}

int fluid_dsp_float_interpolate_7th_order(fluid_voice_t *voice)
{
	fluid_phase_t dsp_phase = voice->phase;
	fluid_phase_t dsp_phase_incr;   short int *dsp_data = voice->sample->data;
	fluid_real_t *dsp_buf = voice->dsp_buf;
	fluid_real_t dsp_amp = voice->amp;
	fluid_real_t dsp_amp_incr = voice->amp_incr;
	unsigned int dsp_i = 0;
	unsigned int dsp_phase_index;
	unsigned int start_index, end_index;
	short int start_points[3];
	short int end_points[3];
	fluid_real_t *coeffs;
	int looping;

	fluid_phase_set_float(dsp_phase_incr, voice->phase_incr);

	fluid_phase_incr(dsp_phase, (fluid_phase_t)0x80000000);

	looping = _SAMPLEMODE(voice) == FLUID_LOOP_DURING_RELEASE
		|| (_SAMPLEMODE(voice) == FLUID_LOOP_UNTIL_RELEASE
		&& voice->volenv_section < FLUID_VOICE_ENVRELEASE);

	end_index = (looping ? voice->loopend - 1 : voice->end) - 3;

	if (voice->has_looped)
	{
		start_index = voice->loopstart;
		start_points[0] = dsp_data[voice->loopend - 1];
		start_points[1] = dsp_data[voice->loopend - 2];
		start_points[2] = dsp_data[voice->loopend - 3];
	}
	else
	{
		start_index = voice->start;
		start_points[0] = dsp_data[voice->start];
		start_points[1] = start_points[0];
		start_points[2] = start_points[0];
	}

	if (looping)
	{
		end_points[0] = dsp_data[voice->loopstart];
		end_points[1] = dsp_data[voice->loopstart + 1];
		end_points[2] = dsp_data[voice->loopstart + 2];
	}
	else
	{
		end_points[0] = dsp_data[voice->end];
		end_points[1] = end_points[0];
		end_points[2] = end_points[0];
	}

	while (1)
	{
		dsp_phase_index = fluid_phase_index(dsp_phase);

		for (; dsp_phase_index == start_index && dsp_i < FLUID_BUFSIZE; dsp_i++)
		{
			coeffs = sinc_table7[fluid_phase_fract_to_tablerow(dsp_phase)];

			dsp_buf[dsp_i] = dsp_amp
				* (coeffs[0] * (fluid_real_t)start_points[2]
				+ coeffs[1] * (fluid_real_t)start_points[1]
				+ coeffs[2] * (fluid_real_t)start_points[0]
				+ coeffs[3] * (fluid_real_t)dsp_data[dsp_phase_index]
				+ coeffs[4] * (fluid_real_t)dsp_data[dsp_phase_index + 1]
				+ coeffs[5] * (fluid_real_t)dsp_data[dsp_phase_index + 2]
				+ coeffs[6] * (fluid_real_t)dsp_data[dsp_phase_index + 3]);

			fluid_phase_incr(dsp_phase, dsp_phase_incr);
			dsp_phase_index = fluid_phase_index(dsp_phase);
			dsp_amp += dsp_amp_incr;
		}

		start_index++;

		for (; dsp_phase_index == start_index && dsp_i < FLUID_BUFSIZE; dsp_i++)
		{
			coeffs = sinc_table7[fluid_phase_fract_to_tablerow(dsp_phase)];

			dsp_buf[dsp_i] = dsp_amp
				* (coeffs[0] * (fluid_real_t)start_points[1]
				+ coeffs[1] * (fluid_real_t)start_points[0]
				+ coeffs[2] * (fluid_real_t)dsp_data[dsp_phase_index - 1]
				+ coeffs[3] * (fluid_real_t)dsp_data[dsp_phase_index]
				+ coeffs[4] * (fluid_real_t)dsp_data[dsp_phase_index + 1]
				+ coeffs[5] * (fluid_real_t)dsp_data[dsp_phase_index + 2]
				+ coeffs[6] * (fluid_real_t)dsp_data[dsp_phase_index + 3]);

			fluid_phase_incr(dsp_phase, dsp_phase_incr);
			dsp_phase_index = fluid_phase_index(dsp_phase);
			dsp_amp += dsp_amp_incr;
		}

		start_index++;

		for (; dsp_phase_index == start_index && dsp_i < FLUID_BUFSIZE; dsp_i++)
		{
			coeffs = sinc_table7[fluid_phase_fract_to_tablerow(dsp_phase)];

			dsp_buf[dsp_i] = dsp_amp
				* (coeffs[0] * (fluid_real_t)start_points[0]
				+ coeffs[1] * (fluid_real_t)dsp_data[dsp_phase_index - 2]
				+ coeffs[2] * (fluid_real_t)dsp_data[dsp_phase_index - 1]
				+ coeffs[3] * (fluid_real_t)dsp_data[dsp_phase_index]
				+ coeffs[4] * (fluid_real_t)dsp_data[dsp_phase_index + 1]
				+ coeffs[5] * (fluid_real_t)dsp_data[dsp_phase_index + 2]
				+ coeffs[6] * (fluid_real_t)dsp_data[dsp_phase_index + 3]);

			fluid_phase_incr(dsp_phase, dsp_phase_incr);
			dsp_phase_index = fluid_phase_index(dsp_phase);
			dsp_amp += dsp_amp_incr;
		}

		start_index -= 2;

		for (; dsp_i < FLUID_BUFSIZE && dsp_phase_index <= end_index; dsp_i++)
		{
			coeffs = sinc_table7[fluid_phase_fract_to_tablerow(dsp_phase)];

			dsp_buf[dsp_i] = dsp_amp
				* (coeffs[0] * (fluid_real_t)dsp_data[dsp_phase_index - 3]
				+ coeffs[1] * (fluid_real_t)dsp_data[dsp_phase_index - 2]
				+ coeffs[2] * (fluid_real_t)dsp_data[dsp_phase_index - 1]
				+ coeffs[3] * (fluid_real_t)dsp_data[dsp_phase_index]
				+ coeffs[4] * (fluid_real_t)dsp_data[dsp_phase_index + 1]
				+ coeffs[5] * (fluid_real_t)dsp_data[dsp_phase_index + 2]
				+ coeffs[6] * (fluid_real_t)dsp_data[dsp_phase_index + 3]);

			fluid_phase_incr(dsp_phase, dsp_phase_incr);
			dsp_phase_index = fluid_phase_index(dsp_phase);
			dsp_amp += dsp_amp_incr;
		}

		if (dsp_i >= FLUID_BUFSIZE) break;

		end_index++;

		for (; dsp_phase_index <= end_index && dsp_i < FLUID_BUFSIZE; dsp_i++)
		{
			coeffs = sinc_table7[fluid_phase_fract_to_tablerow(dsp_phase)];

			dsp_buf[dsp_i] = dsp_amp
				* (coeffs[0] * (fluid_real_t)dsp_data[dsp_phase_index - 3]
				+ coeffs[1] * (fluid_real_t)dsp_data[dsp_phase_index - 2]
				+ coeffs[2] * (fluid_real_t)dsp_data[dsp_phase_index - 1]
				+ coeffs[3] * (fluid_real_t)dsp_data[dsp_phase_index]
				+ coeffs[4] * (fluid_real_t)dsp_data[dsp_phase_index + 1]
				+ coeffs[5] * (fluid_real_t)dsp_data[dsp_phase_index + 2]
				+ coeffs[6] * (fluid_real_t)end_points[0]);

			fluid_phase_incr(dsp_phase, dsp_phase_incr);
			dsp_phase_index = fluid_phase_index(dsp_phase);
			dsp_amp += dsp_amp_incr;
		}

		end_index++;

		for (; dsp_phase_index <= end_index && dsp_i < FLUID_BUFSIZE; dsp_i++)
		{
			coeffs = sinc_table7[fluid_phase_fract_to_tablerow(dsp_phase)];

			dsp_buf[dsp_i] = dsp_amp
				* (coeffs[0] * (fluid_real_t)dsp_data[dsp_phase_index - 3]
				+ coeffs[1] * (fluid_real_t)dsp_data[dsp_phase_index - 2]
				+ coeffs[2] * (fluid_real_t)dsp_data[dsp_phase_index - 1]
				+ coeffs[3] * (fluid_real_t)dsp_data[dsp_phase_index]
				+ coeffs[4] * (fluid_real_t)dsp_data[dsp_phase_index + 1]
				+ coeffs[5] * (fluid_real_t)end_points[0]
				+ coeffs[6] * (fluid_real_t)end_points[1]);

			fluid_phase_incr(dsp_phase, dsp_phase_incr);
			dsp_phase_index = fluid_phase_index(dsp_phase);
			dsp_amp += dsp_amp_incr;
		}

		end_index++;

		for (; dsp_phase_index <= end_index && dsp_i < FLUID_BUFSIZE; dsp_i++)
		{
			coeffs = sinc_table7[fluid_phase_fract_to_tablerow(dsp_phase)];

			dsp_buf[dsp_i] = dsp_amp
				* (coeffs[0] * (fluid_real_t)dsp_data[dsp_phase_index - 3]
				+ coeffs[1] * (fluid_real_t)dsp_data[dsp_phase_index - 2]
				+ coeffs[2] * (fluid_real_t)dsp_data[dsp_phase_index - 1]
				+ coeffs[3] * (fluid_real_t)dsp_data[dsp_phase_index]
				+ coeffs[4] * (fluid_real_t)end_points[0]
				+ coeffs[5] * (fluid_real_t)end_points[1]
				+ coeffs[6] * (fluid_real_t)end_points[2]);

			fluid_phase_incr(dsp_phase, dsp_phase_incr);
			dsp_phase_index = fluid_phase_index(dsp_phase);
			dsp_amp += dsp_amp_incr;
		}

		if (!looping) break;

		if (dsp_phase_index > end_index)
		{
			fluid_phase_sub_int(dsp_phase, voice->loopend - voice->loopstart);

			if (!voice->has_looped)
			{
				voice->has_looped = 1;
				start_index = voice->loopstart;
				start_points[0] = dsp_data[voice->loopend - 1];
				start_points[1] = dsp_data[voice->loopend - 2];
				start_points[2] = dsp_data[voice->loopend - 3];
			}
		}

		if (dsp_i >= FLUID_BUFSIZE) break;

		end_index -= 3;
	}

	fluid_phase_decr(dsp_phase, (fluid_phase_t)0x80000000);

	voice->phase = dsp_phase;
	voice->amp = dsp_amp;

	return (dsp_i);
}

//
//MERGED FILE END: fluid_dsp_float.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: fluid_event.c
//

#ifndef _FLUID_EVENT_PRIV_H
#define _FLUID_EVENT_PRIV_H

struct _fluid_event_t {
	unsigned int time;
	int type;
	short src;
	short dest;
	int channel;
	short key;
	short vel;
	short control;
	short value;
	short id; 	int pitch;
	unsigned int duration;
	void* data;
};

unsigned int fluid_event_get_time(fluid_event_t* evt);
void fluid_event_set_time(fluid_event_t* evt, unsigned int time);

enum fluid_evt_entry_type {
	FLUID_EVT_ENTRY_INSERT = 0,
	FLUID_EVT_ENTRY_REMOVE
};

typedef struct _fluid_evt_entry fluid_evt_entry;
struct _fluid_evt_entry {
	fluid_evt_entry *next;
	short entryType;
	fluid_event_t evt;
};

#define HEAP_WITH_DYNALLOC 1

typedef struct _fluid_evt_heap_t {
#ifdef HEAP_WITH_DYNALLOC
	fluid_evt_entry* freelist;
	fluid_mutex_t mutex;
#else
	fluid_evt_entry* head;
	fluid_evt_entry* tail;
	fluid_evt_entry pool;
#endif
} fluid_evt_heap_t;

fluid_evt_heap_t* _fluid_evt_heap_init(int nbEvents);
void _fluid_evt_heap_free(fluid_evt_heap_t* heap);
fluid_evt_entry* _fluid_seq_heap_get_free(fluid_evt_heap_t* heap);
void _fluid_seq_heap_set_free(fluid_evt_heap_t* heap, fluid_evt_entry* evt);

#endif

fluid_event_t*
new_fluid_event()
{
	fluid_event_t* evt;

	evt = FLUID_NEW(fluid_event_t);
	if (evt == NULL) {
		fluid_log(FLUID_PANIC, "event: Out of memory\n");
		return NULL;
	}

	FLUID_MEMSET(evt, 0, sizeof(fluid_event_t));

	evt->dest = -1;
	evt->src = -1;
	evt->type = -1;

	return(evt);
}

void delete_fluid_event(fluid_event_t* evt)
{
	if (evt == NULL) {
		return;
	}

	FLUID_FREE(evt);
}

void fluid_event_set_time(fluid_event_t* evt, unsigned int time)
{
	evt->time = time;
}

void fluid_event_set_source(fluid_event_t* evt, short src)
{
	evt->src = src;
}

void fluid_event_set_dest(fluid_event_t* evt, short dest)
{
	evt->dest = dest;
}

void fluid_event_timer(fluid_event_t* evt, void* data)
{
	evt->type = FLUID_SEQ_TIMER;
	evt->data = data;
}

void fluid_event_noteon(fluid_event_t* evt, int channel, short key, short vel)
{
	evt->type = FLUID_SEQ_NOTEON;
	evt->channel = channel;
	evt->key = key;
	evt->vel = vel;
}

void fluid_event_noteoff(fluid_event_t* evt, int channel, short key)
{
	evt->type = FLUID_SEQ_NOTEOFF;
	evt->channel = channel;
	evt->key = key;
}

void fluid_event_note(fluid_event_t* evt, int channel, short key, short vel, unsigned int duration)
{
	evt->type = FLUID_SEQ_NOTE;
	evt->channel = channel;
	evt->key = key;
	evt->vel = vel;
	evt->duration = duration;
}

void fluid_event_all_sounds_off(fluid_event_t* evt, int channel)
{
	evt->type = FLUID_SEQ_ALLSOUNDSOFF;
	evt->channel = channel;
}

void fluid_event_all_notes_off(fluid_event_t* evt, int channel)
{
	evt->type = FLUID_SEQ_ALLNOTESOFF;
	evt->channel = channel;
}

void fluid_event_bank_select(fluid_event_t* evt, int channel, short bank_num)
{
	evt->type = FLUID_SEQ_BANKSELECT;
	evt->channel = channel;
	evt->control = bank_num;
}

void fluid_event_program_change(fluid_event_t* evt, int channel, short val)
{
	evt->type = FLUID_SEQ_PROGRAMCHANGE;
	evt->channel = channel;
	evt->value = val;
}

void fluid_event_program_select(fluid_event_t* evt, int channel,
unsigned int sfont_id, short bank_num, short preset_num)
{
	evt->type = FLUID_SEQ_PROGRAMSELECT;
	evt->channel = channel;
	evt->duration = sfont_id;
	evt->value = preset_num;
	evt->control = bank_num;
}

void fluid_event_any_control_change(fluid_event_t* evt, int channel)
{
	evt->type = FLUID_SEQ_ANYCONTROLCHANGE;
	evt->channel = channel;
}

void fluid_event_pitch_bend(fluid_event_t* evt, int channel, int pitch)
{
	evt->type = FLUID_SEQ_PITCHBEND;
	evt->channel = channel;
	if (pitch < 0) pitch = 0;
	if (pitch > 16383) pitch = 16383;
	evt->pitch = pitch;
}

void fluid_event_pitch_wheelsens(fluid_event_t* evt, int channel, short value)
{
	evt->type = FLUID_SEQ_PITCHWHHELSENS;
	evt->channel = channel;
	evt->value = value;
}

void fluid_event_modulation(fluid_event_t* evt, int channel, short val)
{
	evt->type = FLUID_SEQ_MODULATION;
	evt->channel = channel;
	if (val < 0) val = 0;
	if (val > 127) val = 127;
	evt->value = val;
}

void fluid_event_sustain(fluid_event_t* evt, int channel, short val)
{
	evt->type = FLUID_SEQ_SUSTAIN;
	evt->channel = channel;
	if (val < 0) val = 0;
	if (val > 127) val = 127;
	evt->value = val;
}

void fluid_event_control_change(fluid_event_t* evt, int channel, short control, short val)
{
	evt->type = FLUID_SEQ_CONTROLCHANGE;
	evt->channel = channel;
	evt->control = control;
	evt->value = val;
}

void fluid_event_pan(fluid_event_t* evt, int channel, short val)
{
	evt->type = FLUID_SEQ_PAN;
	evt->channel = channel;
	if (val < 0) val = 0;
	if (val > 127) val = 127;
	evt->value = val;
}

void fluid_event_volume(fluid_event_t* evt, int channel, short val)
{
	evt->type = FLUID_SEQ_VOLUME;
	evt->channel = channel;
	if (val < 0) val = 0;
	if (val > 127) val = 127;
	evt->value = val;
}

void fluid_event_reverb_send(fluid_event_t* evt, int channel, short val)
{
	evt->type = FLUID_SEQ_REVERBSEND;
	evt->channel = channel;
	if (val < 0) val = 0;
	if (val > 127) val = 127;
	evt->value = val;
}

void fluid_event_chorus_send(fluid_event_t* evt, int channel, short val)
{
	evt->type = FLUID_SEQ_CHORUSSEND;
	evt->channel = channel;
	if (val < 0) val = 0;
	if (val > 127) val = 127;
	evt->value = val;
}

int fluid_event_get_type(fluid_event_t* evt)
{
	return evt->type;
}

unsigned int fluid_event_get_time(fluid_event_t* evt)
{
	return evt->time;
}

short fluid_event_get_source(fluid_event_t* evt)
{
	return evt->src;
}

short fluid_event_get_dest(fluid_event_t* evt)
{
	return evt->dest;
}

int fluid_event_get_channel(fluid_event_t* evt)
{
	return evt->channel;
}

short fluid_event_get_key(fluid_event_t* evt)
{
	return evt->key;
}

short fluid_event_get_velocity(fluid_event_t* evt)

{
	return evt->vel;
}

short fluid_event_get_control(fluid_event_t* evt)
{
	return evt->control;
}

short fluid_event_get_value(fluid_event_t* evt)
{
	return evt->value;
}

void* fluid_event_get_data(fluid_event_t* evt)
{
	return evt->data;
}

unsigned int fluid_event_get_duration(fluid_event_t* evt)
{
	return evt->duration;
}

short fluid_event_get_bank(fluid_event_t* evt)
{
	return evt->control;
}

int fluid_event_get_pitch(fluid_event_t* evt)
{
	return evt->pitch;
}

short
fluid_event_get_program(fluid_event_t* evt)
{
	return evt->value;
}

unsigned int fluid_event_get_sfont_id(fluid_event_t* evt)
{
	return evt->duration;
}

fluid_evt_heap_t*
_fluid_evt_heap_init(int nbEvents)
{
#ifdef HEAP_WITH_DYNALLOC

	int i;
	fluid_evt_heap_t* heap;
	fluid_evt_entry *tmp;

	heap = FLUID_NEW(fluid_evt_heap_t);
	if (heap == NULL) {
		fluid_log(FLUID_PANIC, "sequencer: Out of memory\n");
		return NULL;
	}

	heap->freelist = NULL;
	fluid_mutex_init(heap->mutex);

	fluid_mutex_lock(heap->mutex);

	for (i = 0; i < nbEvents; i++) {
		tmp = FLUID_NEW(fluid_evt_entry);
		tmp->next = heap->freelist;
		heap->freelist = tmp;
	}

	fluid_mutex_unlock(heap->mutex);

#else
	int i;
	fluid_evt_heap_t* heap;
	int siz = 2 * sizeof(fluid_evt_entry *) + sizeof(fluid_evt_entry)*nbEvents;

	heap = (fluid_evt_heap_t *)FLUID_MALLOC(siz);
	if (heap == NULL) {
		fluid_log(FLUID_PANIC, "sequencer: Out of memory\n");
		return NULL;
	}
	FLUID_MEMSET(heap, 0, siz);

	{
		fluid_evt_entry *tmp = &(heap->pool);
		for (i = 0; i < nbEvents - 1; i++)
			tmp[i].next = &(tmp[i + 1]);
		tmp[nbEvents - 1].next = NULL;

		heap->tail = &(tmp[nbEvents - 1]);
		heap->head = &(heap->pool);
	}
#endif
	return (heap);
}

void _fluid_evt_heap_free(fluid_evt_heap_t* heap)
{
#ifdef HEAP_WITH_DYNALLOC
	fluid_evt_entry *tmp, *next;

	fluid_mutex_lock(heap->mutex);

	tmp = heap->freelist;
	while (tmp) {
		next = tmp->next;
		FLUID_FREE(tmp);
		tmp = next;
	}

	fluid_mutex_unlock(heap->mutex);
	fluid_mutex_destroy(heap->mutex);

	FLUID_FREE(heap);

#else
	FLUID_FREE(heap);
#endif
}

fluid_evt_entry*
_fluid_seq_heap_get_free(fluid_evt_heap_t* heap)
{
#ifdef HEAP_WITH_DYNALLOC
	fluid_evt_entry* evt = NULL;

	fluid_mutex_lock(heap->mutex);

#if !defined(MACOS9)
	if (heap->freelist == NULL) {
		heap->freelist = FLUID_NEW(fluid_evt_entry);
		if (heap->freelist != NULL) {
			heap->freelist->next = NULL;
		}
	}
#endif

	evt = heap->freelist;

	if (evt != NULL) {
		heap->freelist = heap->freelist->next;
		evt->next = NULL;
	}

	fluid_mutex_unlock(heap->mutex);

	return evt;

#else
	fluid_evt_entry* evt;
	if (heap->head == NULL) return NULL;

	evt = heap->head;
	heap->head = heap->head->next;

	return evt;
#endif
}

void _fluid_seq_heap_set_free(fluid_evt_heap_t* heap, fluid_evt_entry* evt)
{
#ifdef HEAP_WITH_DYNALLOC

	fluid_mutex_lock(heap->mutex);

	evt->next = heap->freelist;
	heap->freelist = evt;

	fluid_mutex_unlock(heap->mutex);

#else

	heap->tail->next = evt;
	heap->tail = evt;
	evt->next = NULL;
#endif
}

//
//MERGED FILE END: fluid_event.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: fluid_gen.c
//

fluid_gen_info_t fluid_gen_info[] = {
	{ GEN_STARTADDROFS, 1, 1, 0.0f, 1e10f, 0.0f },
	{ GEN_ENDADDROFS, 1, 1, -1e10f, 0.0f, 0.0f },
	{ GEN_STARTLOOPADDROFS, 1, 1, -1e10f, 1e10f, 0.0f },
	{ GEN_ENDLOOPADDROFS, 1, 1, -1e10f, 1e10f, 0.0f },
	{ GEN_STARTADDRCOARSEOFS, 0, 1, 0.0f, 1e10f, 0.0f },
	{ GEN_MODLFOTOPITCH, 1, 2, -12000.0f, 12000.0f, 0.0f },
	{ GEN_VIBLFOTOPITCH, 1, 2, -12000.0f, 12000.0f, 0.0f },
	{ GEN_MODENVTOPITCH, 1, 2, -12000.0f, 12000.0f, 0.0f },
	{ GEN_FILTERFC, 1, 2, 1500.0f, 13500.0f, 13500.0f },
	{ GEN_FILTERQ, 1, 1, 0.0f, 960.0f, 0.0f },
	{ GEN_MODLFOTOFILTERFC, 1, 2, -12000.0f, 12000.0f, 0.0f },
	{ GEN_MODENVTOFILTERFC, 1, 2, -12000.0f, 12000.0f, 0.0f },
	{ GEN_ENDADDRCOARSEOFS, 0, 1, -1e10f, 0.0f, 0.0f },
	{ GEN_MODLFOTOVOL, 1, 1, -960.0f, 960.0f, 0.0f },
	{ GEN_UNUSED1, 0, 0, 0.0f, 0.0f, 0.0f },
	{ GEN_CHORUSSEND, 1, 1, 0.0f, 1000.0f, 0.0f },
	{ GEN_REVERBSEND, 1, 1, 0.0f, 1000.0f, 0.0f },
	{ GEN_PAN, 1, 1, -500.0f, 500.0f, 0.0f },
	{ GEN_UNUSED2, 0, 0, 0.0f, 0.0f, 0.0f },
	{ GEN_UNUSED3, 0, 0, 0.0f, 0.0f, 0.0f },
	{ GEN_UNUSED4, 0, 0, 0.0f, 0.0f, 0.0f },
	{ GEN_MODLFODELAY, 1, 2, -12000.0f, 5000.0f, -12000.0f },
	{ GEN_MODLFOFREQ, 1, 4, -16000.0f, 4500.0f, 0.0f },
	{ GEN_VIBLFODELAY, 1, 2, -12000.0f, 5000.0f, -12000.0f },
	{ GEN_VIBLFOFREQ, 1, 4, -16000.0f, 4500.0f, 0.0f },
	{ GEN_MODENVDELAY, 1, 2, -12000.0f, 5000.0f, -12000.0f },
	{ GEN_MODENVATTACK, 1, 2, -12000.0f, 8000.0f, -12000.0f },
	{ GEN_MODENVHOLD, 1, 2, -12000.0f, 5000.0f, -12000.0f },
	{ GEN_MODENVDECAY, 1, 2, -12000.0f, 8000.0f, -12000.0f },
	{ GEN_MODENVSUSTAIN, 0, 1, 0.0f, 1000.0f, 0.0f },
	{ GEN_MODENVRELEASE, 1, 2, -12000.0f, 8000.0f, -12000.0f },
	{ GEN_KEYTOMODENVHOLD, 0, 1, -1200.0f, 1200.0f, 0.0f },
	{ GEN_KEYTOMODENVDECAY, 0, 1, -1200.0f, 1200.0f, 0.0f },
	{ GEN_VOLENVDELAY, 1, 2, -12000.0f, 5000.0f, -12000.0f },
	{ GEN_VOLENVATTACK, 1, 2, -12000.0f, 8000.0f, -12000.0f },
	{ GEN_VOLENVHOLD, 1, 2, -12000.0f, 5000.0f, -12000.0f },
	{ GEN_VOLENVDECAY, 1, 2, -12000.0f, 8000.0f, -12000.0f },
	{ GEN_VOLENVSUSTAIN, 0, 1, 0.0f, 1440.0f, 0.0f },
	{ GEN_VOLENVRELEASE, 1, 2, -12000.0f, 8000.0f, -12000.0f },
	{ GEN_KEYTOVOLENVHOLD, 0, 1, -1200.0f, 1200.0f, 0.0f },
	{ GEN_KEYTOVOLENVDECAY, 0, 1, -1200.0f, 1200.0f, 0.0f },
	{ GEN_INSTRUMENT, 0, 0, 0.0f, 0.0f, 0.0f },
	{ GEN_RESERVED1, 0, 0, 0.0f, 0.0f, 0.0f },
	{ GEN_KEYRANGE, 0, 0, 0.0f, 127.0f, 0.0f },
	{ GEN_VELRANGE, 0, 0, 0.0f, 127.0f, 0.0f },
	{ GEN_STARTLOOPADDRCOARSEOFS, 0, 1, -1e10f, 1e10f, 0.0f },
	{ GEN_KEYNUM, 1, 0, 0.0f, 127.0f, -1.0f },
	{ GEN_VELOCITY, 1, 1, 0.0f, 127.0f, -1.0f },
	{ GEN_ATTENUATION, 1, 1, 0.0f, 1440.0f, 0.0f },
	{ GEN_RESERVED2, 0, 0, 0.0f, 0.0f, 0.0f },
	{ GEN_ENDLOOPADDRCOARSEOFS, 0, 1, -1e10f, 1e10f, 0.0f },
	{ GEN_COARSETUNE, 0, 1, -120.0f, 120.0f, 0.0f },
	{ GEN_FINETUNE, 0, 1, -99.0f, 99.0f, 0.0f },
	{ GEN_SAMPLEID, 0, 0, 0.0f, 0.0f, 0.0f },
	{ GEN_SAMPLEMODE, 0, 0, 0.0f, 0.0f, 0.0f },
	{ GEN_RESERVED3, 0, 0, 0.0f, 0.0f, 0.0f },
	{ GEN_SCALETUNE, 0, 1, 0.0f, 1200.0f, 100.0f },
	{ GEN_EXCLUSIVECLASS, 0, 0, 0.0f, 0.0f, 0.0f },
	{ GEN_OVERRIDEROOTKEY, 1, 0, 0.0f, 127.0f, -1.0f },
	{ GEN_PITCH, 1, 0, 0.0f, 127.0f, 0.0f }
};

int fluid_gen_set_default_values(fluid_gen_t* gen)
{
	int i;

	for (i = 0; i < GEN_LAST; i++) {
		gen[i].flags = GEN_UNUSED;
		gen[i].mod = 0.0;
		gen[i].nrpn = 0.0;
		gen[i].val = fluid_gen_info[i].def;
	}

	return FLUID_OK;
}

int fluid_gen_init(fluid_gen_t* gen, fluid_channel_t* channel)
{
	int i;

	fluid_gen_set_default_values(gen);

	for (i = 0; i < GEN_LAST; i++) {
		gen[i].nrpn = fluid_channel_get_gen(channel, i);

		if (fluid_channel_get_gen_abs(channel, i)) {
			gen[i].flags = GEN_ABS_NRPN;
		}
	}

	return FLUID_OK;
}

fluid_real_t fluid_gen_scale(int gen, float value)
{
	return (fluid_gen_info[gen].min
		+ value * (fluid_gen_info[gen].max - fluid_gen_info[gen].min));
}

fluid_real_t fluid_gen_scale_nrpn(int gen, int data)
{
	fluid_real_t value = (float)data - 8192.0f;
	fluid_clip(value, -8192, 8192);
	return value * (float)fluid_gen_info[gen].nrpn_scale;
}

//
//MERGED FILE END: fluid_gen.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: fluid_io.c
//

#ifndef _FLUID_IO_H
#define _FLUID_IO_H

int fluid_istream_readline(fluid_istream_t in, char* prompt, char* buf, int len);

int fluid_ostream_printf(fluid_ostream_t out, char* format, ...);

#endif

#if WITH_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

int fluid_istream_gets(fluid_istream_t in, char* buf, int len);

fluid_istream_t fluid_get_stdin()
{
#ifdef MACOS9
	return 0;
#else
	return STDIN_FILENO;
#endif
}

fluid_ostream_t fluid_get_stdout()
{
#ifdef MACOS9
	return 1;
#else
	return STDOUT_FILENO;
#endif
}

int fluid_istream_readline(fluid_istream_t in, char* prompt, char* buf, int len)
{
#if WITH_READLINE
	if (in == fluid_get_stdin()) {
		char* line;

		line = readline(prompt);
		if (line == NULL) {
			return -1;
		}

		snprintf(buf, len, "%s", line);
		buf[len - 1] = 0;

		free(line);
		return 1;
	}
	else {
		return fluid_istream_gets(in, buf, len);
	}
#else
	return fluid_istream_gets(in, buf, len);
#endif
}

int fluid_istream_gets(fluid_istream_t in, char* buf, int len)
{
	char c;
	int n;

	buf[len - 1] = 0;

	while (--len > 0) {
		n = read(in, &c, 1);
		if (n == 0) {
			*buf++ = 0;
			return 0;
		}
		if (n < 0) {
			return n;
		}
		if ((c == '\n') || (c == '\r')) {
			*buf++ = 0;
			return 1;
		}
		*buf++ = c;
	}

	return -1;
}

int fluid_ostream_printf(fluid_ostream_t out, char* format, ...)
{
	char buf[4096];
	va_list args;
	int len;

	va_start(args, format);
	len = vsnprintf(buf, 4095, format, args);
	va_end(args);

	if (len <= 0) {
		printf("fluid_ostream_printf: buffer overflow");
		return -1;
	}

	buf[4095] = 0;

	return write(out, buf, (unsigned int)strlen(buf));
}

//
//MERGED FILE END: fluid_io.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: fluid_list.c
//

fluid_list_t*
new_fluid_list(void)
{
	fluid_list_t* list;
	list = (fluid_list_t*)FLUID_MALLOC(sizeof(fluid_list_t));
	list->data = NULL;
	list->next = NULL;
	return list;
}

void delete_fluid_list(fluid_list_t *list)
{
	fluid_list_t *next;
	while (list) {
		next = list->next;
		FLUID_FREE(list);
		list = next;
	}
}

void delete1_fluid_list(fluid_list_t *list)
{
	if (list) {
		FLUID_FREE(list);
	}
}

fluid_list_t*
fluid_list_append(fluid_list_t *list, void*  data)
{
	fluid_list_t *new_list;
	fluid_list_t *last;

	new_list = new_fluid_list();
	new_list->data = data;

	if (list)
	{
		last = fluid_list_last(list);

		last->next = new_list;

		return list;
	}
	else
		return new_list;
}

fluid_list_t*
fluid_list_prepend(fluid_list_t *list, void* data)
{
	fluid_list_t *new_list;

	new_list = new_fluid_list();
	new_list->data = data;
	new_list->next = list;

	return new_list;
}

fluid_list_t*
fluid_list_nth(fluid_list_t *list, int n)
{
	while ((n-- > 0) && list) {
		list = list->next;
	}

	return list;
}

fluid_list_t*
fluid_list_remove(fluid_list_t *list, void* data)
{
	fluid_list_t *tmp;
	fluid_list_t *prev;

	prev = NULL;
	tmp = list;

	while (tmp) {
		if (tmp->data == data) {
			if (prev) {
				prev->next = tmp->next;
			}
			if (list == tmp) {
				list = list->next;
			}
			tmp->next = NULL;
			delete_fluid_list(tmp);

			break;
		}

		prev = tmp;
		tmp = tmp->next;
	}

	return list;
}

fluid_list_t*
fluid_list_remove_link(fluid_list_t *list, fluid_list_t *link)
{
	fluid_list_t *tmp;
	fluid_list_t *prev;

	prev = NULL;
	tmp = list;

	while (tmp) {
		if (tmp == link) {
			if (prev) {
				prev->next = tmp->next;
			}
			if (list == tmp) {
				list = list->next;
			}
			tmp->next = NULL;
			break;
		}

		prev = tmp;
		tmp = tmp->next;
	}

	return list;
}

static fluid_list_t*
fluid_list_sort_merge(fluid_list_t *l1, fluid_list_t *l2, fluid_compare_func_t compare_func)
{
	fluid_list_t list, *l;

	l = &list;

	while (l1 && l2) {
		if (compare_func(l1->data, l2->data) < 0) {
			l = l->next = l1;
			l1 = l1->next;
		}
		else {
			l = l->next = l2;
			l2 = l2->next;
		}
	}
	l->next = l1 ? l1 : l2;

	return list.next;
}

fluid_list_t*
fluid_list_sort(fluid_list_t *list, fluid_compare_func_t compare_func)
{
	fluid_list_t *l1, *l2;

	if (!list) {
		return NULL;
	}
	if (!list->next) {
		return list;
	}

	l1 = list;
	l2 = list->next;

	while ((l2 = l2->next) != NULL) {
		if ((l2 = l2->next) == NULL)
			break;
		l1 = l1->next;
	}
	l2 = l1->next;
	l1->next = NULL;

	return fluid_list_sort_merge(fluid_list_sort(list, compare_func),
		fluid_list_sort(l2, compare_func),
		compare_func);
}

fluid_list_t*
fluid_list_last(fluid_list_t *list)
{
	if (list) {
		while (list->next)
			list = list->next;
	}

	return list;
}

int fluid_list_size(fluid_list_t *list)
{
	int n = 0;
	while (list) {
		n++;
		list = list->next;
	}
	return n;
}

fluid_list_t* fluid_list_insert_at(fluid_list_t *list, int n, void* data)
{
	fluid_list_t *new_list;
	fluid_list_t *cur;
	fluid_list_t *prev = NULL;

	new_list = new_fluid_list();
	new_list->data = data;

	cur = list;
	while ((n-- > 0) && cur) {
		prev = cur;
		cur = cur->next;
	}

	new_list->next = cur;

	if (prev) {
		prev->next = new_list;
		return list;
	}
	else {
		return new_list;
	}
}

//
//MERGED FILE END: fluid_list.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: fluid_midi.c
//

#define MIDI_MESSAGE_LENGTH 1024
char midi_message_buffer[MIDI_MESSAGE_LENGTH];

static int remains_f0f6[] = {
	0,
	2,
	3,
	2,
	2,
	2,
	1
};

static int remains_80e0[] = {
	3,
	3,
	3,
	3,
	2,
	2,
	3
};

fluid_midi_file* new_fluid_midi_file(char* filename)
{
	fluid_midi_file* mf;

	mf = FLUID_NEW(fluid_midi_file);
	if (mf == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return NULL;
	}
	FLUID_MEMSET(mf, 0, sizeof(fluid_midi_file));

	mf->c = -1;
	mf->running_status = -1;
	mf->fp = FLUID_FOPEN(filename, "rb");

	if (mf->fp == NULL) {
		FLUID_LOG(FLUID_ERR, "Couldn't open the MIDI file");
		FLUID_FREE(mf);
		return NULL;
	}

	if (fluid_midi_file_read_mthd(mf) != FLUID_OK) {
		FLUID_FREE(mf);
		return NULL;
	}
	return mf;
}

void delete_fluid_midi_file(fluid_midi_file* mf)
{
	if (mf == NULL) {
		return;
	}
	if (mf->fp != NULL) {
		FLUID_FCLOSE(mf->fp);
	}
	FLUID_FREE(mf);
	return;
}

int fluid_midi_file_getc(fluid_midi_file* mf)
{
	unsigned char c;
	int n; (void)n;
	if (mf->c >= 0) {
		c = mf->c;
		mf->c = -1;
	}
	else {
		n = (int)FLUID_FREAD(&c, 1, 1, mf->fp);
		mf->trackpos++;
	}
	return (int)c;
}

int fluid_midi_file_push(fluid_midi_file* mf, int c)
{
	mf->c = c;
	return FLUID_OK;
}

int fluid_midi_file_read(fluid_midi_file* mf, void* buf, int len)
{
	int num = (int)FLUID_FREAD(buf, 1, len, mf->fp);
	mf->trackpos += num;
#if DEBUG
	if (num != len) {
		FLUID_LOG(FLUID_DBG, "Coulnd't read the requested number of bytes");
	}
#endif
	return (num != len) ? FLUID_FAILED : FLUID_OK;
}

int fluid_midi_file_skip(fluid_midi_file* mf, int skip)
{
	int err = FLUID_FSEEK(mf->fp, skip, SEEK_CUR);
	if (err) {
		FLUID_LOG(FLUID_ERR, "Failed to seek position in file");
		return FLUID_FAILED;
	}
	return FLUID_OK;
}

int fluid_midi_file_read_mthd(fluid_midi_file* mf)
{
	char mthd[15];
	if (fluid_midi_file_read(mf, mthd, 14) != FLUID_OK) {
		return FLUID_FAILED;
	}
	if ((FLUID_STRNCMP(mthd, "MThd", 4) != 0) || (mthd[7] != 6) || (mthd[9] > 2)) {
		FLUID_LOG(FLUID_ERR, "Doesn't look like a MIDI file: invalid MThd header");
		return FLUID_FAILED;
	}
	mf->type = mthd[9];
	mf->ntracks = (unsigned)mthd[11];
	mf->ntracks += (unsigned int)(mthd[10]) << 16;
	if ((mthd[12]) < 0){
		mf->uses_smpte = 1;
		mf->smpte_fps = -mthd[12];
		mf->smpte_res = (unsigned)mthd[13];
		FLUID_LOG(FLUID_ERR, "File uses SMPTE timing -- Not implemented yet");
		return FLUID_FAILED;
	}
	else {
		mf->uses_smpte = 0;
		mf->division = (mthd[12] << 8) | (mthd[13] & 0xff);
		FLUID_LOG1(FLUID_DBG, "Division=%d", mf->division);
	}
	return FLUID_OK;
}

int fluid_midi_file_load_tracks(fluid_midi_file* mf, fluid_player_t* player)
{
	int i;
	for (i = 0; i < mf->ntracks; i++) {
		if (fluid_midi_file_read_track(mf, player, i) != FLUID_OK) {
			return FLUID_FAILED;
		}
	}
	return FLUID_OK;
}

int fluid_isasciistring(char* s)
{
	int i;
	int len = (int)FLUID_STRLEN(s);
	for (i = 0; i < len; i++) {
		if (!fluid_isascii(s[i])) {
			return 0;
		}
	}
	return 1;
}

long fluid_getlength(unsigned char *s)
{
	long i = 0;
	i = s[3] | (s[2] << 8) | (s[1] << 16) | (s[0] << 24);
	return i;
}

int fluid_midi_file_read_tracklen(fluid_midi_file* mf)
{
	unsigned char length[5];
	if (fluid_midi_file_read(mf, length, 4) != FLUID_OK) {
		return FLUID_FAILED;
	}
	mf->tracklen = fluid_getlength(length);
	mf->trackpos = 0;
	mf->eot = 0;
	return FLUID_OK;
}

int fluid_midi_file_eot(fluid_midi_file* mf)
{
#if DEBUG
	if (mf->trackpos > mf->tracklen) {
		printf("track overrun: %d > %d\n", mf->trackpos, mf->tracklen);
	}
#endif
	return mf->eot || (mf->trackpos >= mf->tracklen);
}

int fluid_midi_file_read_track(fluid_midi_file* mf, fluid_player_t* player, int num)
{
	fluid_track_t* track;
	unsigned char id[5], length[5];
	int found_track = 0;
	int skip;

	if (fluid_midi_file_read(mf, id, 4) != FLUID_OK) {
		return FLUID_FAILED;
	}
	id[4] = '\0';
	mf->dtime = 0;

	while (!found_track){
		if (fluid_isasciistring((char*)id) == 0) {
			FLUID_LOG(FLUID_ERR, "An non-ascii track header found, currupt file");
			return FLUID_FAILED;

		}
		else if (strcmp((char*)id, "MTrk") == 0) {
			found_track = 1;

			if (fluid_midi_file_read_tracklen(mf) != FLUID_OK) {
				return FLUID_FAILED;
			}

			track = new_fluid_track(num);
			if (track == NULL) {
				FLUID_LOG(FLUID_ERR, "Out of memory");
				return FLUID_FAILED;
			}

			while (!fluid_midi_file_eot(mf)) {
				if (fluid_midi_file_read_event(mf, track) != FLUID_OK) {
					return FLUID_FAILED;
				}
			}

			fluid_player_add_track(player, track);

		}
		else {
			found_track = 0;
			if (fluid_midi_file_read(mf, length, 4) != FLUID_OK) {
				return FLUID_FAILED;
			}
			skip = fluid_getlength(length);

			if (fluid_midi_file_skip(mf, skip) != FLUID_OK) {
				return FLUID_FAILED;
			}
		}
	}
	if (feof(mf->fp)) {
		FLUID_LOG(FLUID_ERR, "Unexpected end of file");
		return FLUID_FAILED;
	}
	return FLUID_OK;
}

int fluid_midi_file_read_varlen(fluid_midi_file* mf)
{
	int i;
	int c;
	mf->varlen = 0;
	for (i = 0;; i++) {
		if (i == 4) {
			FLUID_LOG(FLUID_ERR, "Invalid variable length number");
			return FLUID_FAILED;
		}
		c = fluid_midi_file_getc(mf);
		if (c < 0) {
			FLUID_LOG(FLUID_ERR, "Unexpected end of file");
			return FLUID_FAILED;
		}
		if (c & 0x80){
			mf->varlen |= (int)(c & 0x7F);
			mf->varlen <<= 7;
		}
		else {
			mf->varlen += c;
			break;
		}
	}
	return FLUID_OK;
}

int fluid_midi_file_read_event(fluid_midi_file* mf, fluid_track_t* track)
{
	int status;
	int type;
	int tempo;
	unsigned char* metadata = NULL;
	unsigned char* dyn_buf = NULL;
	unsigned char static_buf[256];
	int nominator, denominator, clocks, notes, sf, mi; (void)nominator; (void)denominator; (void)clocks; (void)notes; (void)sf; (void)mi;
	fluid_midi_event_t* evt;
	int channel = 0;
	int param1 = 0;
	int param2 = 0;

	if (fluid_midi_file_read_varlen(mf) != FLUID_OK) {
		return FLUID_FAILED;
	}
	mf->dtime += mf->varlen;

	status = fluid_midi_file_getc(mf);
	if (status < 0) {
		FLUID_LOG(FLUID_ERR, "Unexpected end of file");
		return FLUID_FAILED;
	}

	if ((status & 0x80) == 0) {
		if ((mf->running_status & 0x80) == 0) {
			FLUID_LOG(FLUID_ERR, "Undefined status and invalid running status");
			return FLUID_FAILED;
		}
		fluid_midi_file_push(mf, status);
		status = mf->running_status;
	}

	if (status & 0x80) {
		mf->running_status = status;

		if ((status == MIDI_SYSEX) || (status == MIDI_EOX)) {
			if (fluid_midi_file_read_varlen(mf) != FLUID_OK) {
				return FLUID_FAILED;
			}

			if (mf->varlen) {
				if (mf->varlen < 255) {
					metadata = &static_buf[0];
				}
				else {
					FLUID_LOG3(FLUID_DBG, "%s: %d: alloc metadata, len = %d", __FILE__, __LINE__, mf->varlen);
					dyn_buf = (unsigned char*)FLUID_MALLOC(mf->varlen + 1);
					if (dyn_buf == NULL) {
						FLUID_LOG(FLUID_PANIC, "Out of memory");
						return FLUID_FAILED;
					}
					metadata = dyn_buf;
				}

				if (fluid_midi_file_read(mf, metadata, mf->varlen) != FLUID_OK) {
					if (dyn_buf) {
						FLUID_FREE(dyn_buf);
					}
					return FLUID_FAILED;
				}

				if (dyn_buf) {
					FLUID_LOG2(FLUID_DBG, "%s: %d: free metadata", __FILE__, __LINE__);
					FLUID_FREE(dyn_buf);
				}
			}

			return FLUID_OK;

		}
		else if (status == MIDI_META_EVENT) {
			int result = FLUID_OK;

			type = fluid_midi_file_getc(mf);
			if (type < 0) {
				FLUID_LOG(FLUID_ERR, "Unexpected end of file");
				return FLUID_FAILED;
			}

			if (fluid_midi_file_read_varlen(mf) != FLUID_OK) {
				return FLUID_FAILED;
			}

			if (mf->varlen < 255) {
				metadata = &static_buf[0];
			}
			else {
				FLUID_LOG3(FLUID_DBG, "%s: %d: alloc metadata, len = %d", __FILE__, __LINE__, mf->varlen);
				dyn_buf = (unsigned char*)FLUID_MALLOC(mf->varlen + 1);
				if (dyn_buf == NULL) {
					FLUID_LOG(FLUID_PANIC, "Out of memory");
					return FLUID_FAILED;
				}
				metadata = dyn_buf;
			}

			if (mf->varlen)
			{
				if (fluid_midi_file_read(mf, metadata, mf->varlen) != FLUID_OK) {
					if (dyn_buf) {
						FLUID_FREE(dyn_buf);
					}
					return FLUID_FAILED;
				}
			}

			switch (type) {
				case MIDI_COPYRIGHT:
					metadata[mf->varlen] = 0;
					break;

				case MIDI_TRACK_NAME:
					metadata[mf->varlen] = 0;
					fluid_track_set_name(track, (char*)metadata);
					break;

				case MIDI_INST_NAME:
					metadata[mf->varlen] = 0;
					break;

				case MIDI_LYRIC:
					break;

				case MIDI_MARKER:
					break;

				case MIDI_CUE_POINT:
					break;

				case MIDI_EOT:
					if (mf->varlen != 0) {
						FLUID_LOG(FLUID_ERR, "Invalid length for EndOfTrack event");
						result = FLUID_FAILED;
						break;
					}
					mf->eot = 1;
					break;

				case MIDI_SET_TEMPO:
					if (mf->varlen != 3) {
						FLUID_LOG(FLUID_ERR, "Invalid length for SetTempo meta event");
						result = FLUID_FAILED;
						break;
					}
					tempo = (metadata[0] << 16) + (metadata[1] << 8) + metadata[2];
					evt = new_fluid_midi_event();
					if (evt == NULL) {
						FLUID_LOG(FLUID_ERR, "Out of memory");
						result = FLUID_FAILED;
						break;
					}
					evt->dtime = mf->dtime;
					evt->type = MIDI_SET_TEMPO;
					evt->channel = 0;
					evt->param1 = tempo;
					evt->param2 = 0;
					fluid_track_add_event(track, evt);
					mf->dtime = 0;
					break;

				case MIDI_SMPTE_OFFSET:
					if (mf->varlen != 5) {
						FLUID_LOG(FLUID_ERR, "Invalid length for SMPTE Offset meta event");
						result = FLUID_FAILED;
						break;
					}
					break;

				case MIDI_TIME_SIGNATURE:
					if (mf->varlen != 4) {
						FLUID_LOG(FLUID_ERR, "Invalid length for TimeSignature meta event");
						result = FLUID_FAILED;
						break;
					}
					nominator = metadata[0];
					denominator = pow(2.0, (double)metadata[1]);
					clocks = metadata[2];
					notes = metadata[3];

					FLUID_LOG4(FLUID_DBG, "signature=%d/%d, metronome=%d, 32nd-notes=%d",
						nominator, denominator, clocks, notes);

					break;

				case MIDI_KEY_SIGNATURE:
					if (mf->varlen != 2) {
						FLUID_LOG(FLUID_ERR, "Invalid length for KeySignature meta event");
						result = FLUID_FAILED;
						break;
					}
					sf = metadata[0];
					mi = metadata[1];
					break;

				case MIDI_SEQUENCER_EVENT:
					break;

				default:
					break;
			}

			if (dyn_buf) {
				FLUID_LOG2(FLUID_DBG, "%s: %d: free metadata", __FILE__, __LINE__);
				FLUID_FREE(dyn_buf);
			}

			return result;

		}
		else {
			type = status & 0xf0;
			channel = status & 0x0f;

			if ((param1 = fluid_midi_file_getc(mf)) < 0) {
				FLUID_LOG(FLUID_ERR, "Unexpected end of file");
				return FLUID_FAILED;
			}

			switch (type) {
				case NOTE_ON:
					if ((param2 = fluid_midi_file_getc(mf)) < 0) {
						FLUID_LOG(FLUID_ERR, "Unexpected end of file");
						return FLUID_FAILED;
					}
					break;

				case NOTE_OFF:
					if ((param2 = fluid_midi_file_getc(mf)) < 0) {
						FLUID_LOG(FLUID_ERR, "Unexpected end of file");
						return FLUID_FAILED;
					}
					break;

				case KEY_PRESSURE:
					if ((param2 = fluid_midi_file_getc(mf)) < 0) {
						FLUID_LOG(FLUID_ERR, "Unexpected end of file");
						return FLUID_FAILED;
					}
					break;

				case CONTROL_CHANGE:
					if ((param2 = fluid_midi_file_getc(mf)) < 0) {
						FLUID_LOG(FLUID_ERR, "Unexpected end of file");
						return FLUID_FAILED;
					}
					break;

				case PROGRAM_CHANGE:
					break;

				case CHANNEL_PRESSURE:
					break;

				case PITCH_BEND:
					if ((param2 = fluid_midi_file_getc(mf)) < 0) {
						FLUID_LOG(FLUID_ERR, "Unexpected end of file");
						return FLUID_FAILED;
					}

					param1 = ((param2 & 0x7f) << 7) | (param1 & 0x7f);
					param2 = 0;
					break;

				default:
					FLUID_LOG(FLUID_ERR, "Unrecognized MIDI event");
					return FLUID_FAILED;
			}
			evt = new_fluid_midi_event();
			if (evt == NULL) {
				FLUID_LOG(FLUID_ERR, "Out of memory");
				return FLUID_FAILED;
			}
			evt->dtime = mf->dtime;
			evt->type = type;
			evt->channel = channel;
			evt->param1 = param1;
			evt->param2 = param2;
			fluid_track_add_event(track, evt);
			mf->dtime = 0;
		}
	}
	return FLUID_OK;
}

int fluid_midi_file_get_division(fluid_midi_file* midifile)
{
	return midifile->division;
}

fluid_midi_event_t* new_fluid_midi_event()
{
	fluid_midi_event_t* evt;
	evt = FLUID_NEW(fluid_midi_event_t);
	if (evt == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return NULL;
	}
	evt->dtime = 0;
	evt->type = 0;
	evt->channel = 0;
	evt->param1 = 0;
	evt->param2 = 0;
	evt->next = NULL;
	return evt;
}

int delete_fluid_midi_event(fluid_midi_event_t* evt)
{
	fluid_midi_event_t *temp;

	while (evt)
	{
		temp = evt->next;
		FLUID_FREE(evt);
		evt = temp;
	}
	return FLUID_OK;
}

int fluid_midi_event_get_type(fluid_midi_event_t* evt)
{
	return evt->type;
}

int fluid_midi_event_set_type(fluid_midi_event_t* evt, int type)
{
	evt->type = type;
	return FLUID_OK;
}

int fluid_midi_event_get_channel(fluid_midi_event_t* evt)
{
	return evt->channel;
}

int fluid_midi_event_set_channel(fluid_midi_event_t* evt, int chan)
{
	evt->channel = chan;
	return FLUID_OK;
}

int fluid_midi_event_get_key(fluid_midi_event_t* evt)
{
	return evt->param1;
}

int fluid_midi_event_set_key(fluid_midi_event_t* evt, int v)
{
	evt->param1 = v;
	return FLUID_OK;
}

int fluid_midi_event_get_velocity(fluid_midi_event_t* evt)
{
	return evt->param2;
}

int fluid_midi_event_set_velocity(fluid_midi_event_t* evt, int v)
{
	evt->param2 = v;
	return FLUID_OK;
}

int fluid_midi_event_get_control(fluid_midi_event_t* evt)
{
	return evt->param1;
}

int fluid_midi_event_set_control(fluid_midi_event_t* evt, int v)
{
	evt->param1 = v;
	return FLUID_OK;
}

int fluid_midi_event_get_value(fluid_midi_event_t* evt)
{
	return evt->param2;
}

int fluid_midi_event_set_value(fluid_midi_event_t* evt, int v)
{
	evt->param2 = v;
	return FLUID_OK;
}

int fluid_midi_event_get_program(fluid_midi_event_t* evt)
{
	return evt->param1;
}

int fluid_midi_event_set_program(fluid_midi_event_t* evt, int val)
{
	evt->param1 = val;
	return FLUID_OK;
}

int fluid_midi_event_get_pitch(fluid_midi_event_t* evt)
{
	return evt->param1;
}

int fluid_midi_event_set_pitch(fluid_midi_event_t* evt, int val)
{
	evt->param1 = val;
	return FLUID_OK;
}

fluid_track_t* new_fluid_track(int num)
{
	fluid_track_t* track;
	track = FLUID_NEW(fluid_track_t);
	if (track == NULL) {
		return NULL;
	}
	track->name = NULL;
	track->num = num;
	track->first = NULL;
	track->cur = NULL;
	track->last = NULL;
	track->ticks = 0;
	return track;
}

int delete_fluid_track(fluid_track_t* track)
{
	if (track->name != NULL) {
		FLUID_FREE(track->name);
	}
	if (track->first != NULL) {
		delete_fluid_midi_event(track->first);
	}
	FLUID_FREE(track);
	return FLUID_OK;
}

int fluid_track_set_name(fluid_track_t* track, char* name)
{
	int len;
	if (track->name != NULL) {
		FLUID_FREE(track->name);
	}
	if (name == NULL) {
		track->name = NULL;
		return FLUID_OK;
	}
	len = (int)FLUID_STRLEN(name);
	track->name = (char*)FLUID_MALLOC(len + 1);
	if (track->name == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return FLUID_FAILED;
	}
	FLUID_STRCPY(track->name, name);
	return FLUID_OK;
}

char* fluid_track_get_name(fluid_track_t* track)
{
	return track->name;
}

int fluid_track_get_duration(fluid_track_t* track)
{
	int time = 0;
	fluid_midi_event_t* evt = track->first;
	while (evt != NULL) {
		time += evt->dtime;
		evt = evt->next;
	}
	return time;
}

int fluid_track_count_events(fluid_track_t* track, int* on, int* off)
{
	fluid_midi_event_t* evt = track->first;
	while (evt != NULL) {
		if (evt->type == NOTE_ON) {
			(*on)++;
		}
		else if (evt->type == NOTE_OFF) {
			(*off)++;
		}
		evt = evt->next;
	}
	return FLUID_OK;
}

int fluid_track_add_event(fluid_track_t* track, fluid_midi_event_t* evt)
{
	evt->next = NULL;
	if (track->first == NULL) {
		track->first = evt;
		track->cur = evt;
		track->last = evt;
	}
	else {
		track->last->next = evt;
		track->last = evt;
	}
	return FLUID_OK;
}

fluid_midi_event_t* fluid_track_first_event(fluid_track_t* track)
{
	track->cur = track->first;
	return track->cur;
}

fluid_midi_event_t* fluid_track_next_event(fluid_track_t* track)
{
	if (track->cur != NULL) {
		track->cur = track->cur->next;
	}
	return track->cur;
}

int fluid_track_reset(fluid_track_t* track)
{
	track->ticks = 0;
	track->cur = track->first;
	return FLUID_OK;
}

int fluid_track_send_events(fluid_track_t* track,
fluid_synth_t* synth,
fluid_player_t* player,
unsigned int ticks)
{
	int status = FLUID_OK;
	fluid_midi_event_t* event;

	while (1) {
		event = track->cur;
		if (event == NULL) {
			return status;
		}

		if (track->ticks + event->dtime > ticks) {
			return status;
		}

		track->ticks += event->dtime;
		status = fluid_midi_send_event(synth, player, event);
		fluid_track_next_event(track);

	}
	return status;
}

fluid_player_t* new_fluid_player(fluid_synth_t* synth)
{
	int i;
	fluid_player_t* player;
	player = FLUID_NEW(fluid_player_t);
	if (player == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return NULL;
	}
	player->status = FLUID_PLAYER_READY;
	player->loop = 0;
	player->ntracks = 0;
	for (i = 0; i < MAX_NUMBER_OF_TRACKS; i++) {
		player->track[i] = NULL;
	}
	player->synth = synth;
	player->timer = NULL;
	player->playlist = NULL;
	player->current_file = NULL;
	player->division = 0;
	player->send_program_change = 1;
	player->miditempo = 480000;
	player->deltatime = 4.0;
	return player;
}

int delete_fluid_player(fluid_player_t* player)
{
	if (player == NULL) {
		return FLUID_OK;
	}
	fluid_player_stop(player);
	fluid_player_reset(player);
	FLUID_FREE(player);
	return FLUID_OK;
}

int fluid_player_reset(fluid_player_t* player)
{
	int i;

	for (i = 0; i < MAX_NUMBER_OF_TRACKS; i++) {
		if (player->track[i] != NULL) {
			delete_fluid_track(player->track[i]);
			player->track[i] = NULL;
		}
	}
	player->current_file = NULL;
	player->status = FLUID_PLAYER_READY;
	player->loop = 0;
	player->ntracks = 0;
	player->division = 0;
	player->send_program_change = 1;
	player->miditempo = 480000;
	player->deltatime = 4.0;
	return 0;
}

int fluid_player_add_track(fluid_player_t* player, fluid_track_t* track)
{
	if (player->ntracks < MAX_NUMBER_OF_TRACKS) {
		player->track[player->ntracks++] = track;
		return FLUID_OK;
	}
	else {
		return FLUID_FAILED;
	}
}

int fluid_player_count_tracks(fluid_player_t* player)
{
	return player->ntracks;
}

fluid_track_t* fluid_player_get_track(fluid_player_t* player, int i)
{
	if ((i >= 0) && (i < MAX_NUMBER_OF_TRACKS)) {
		return player->track[i];
	}
	else {
		return NULL;
	}
}

int fluid_player_add(fluid_player_t* player, char* midifile)
{
	char *s = FLUID_STRDUP(midifile);
	player->playlist = fluid_list_append(player->playlist, s);
	return 0;
}

int fluid_player_load(fluid_player_t* player, char *filename)
{
	fluid_midi_file* midifile;

	midifile = new_fluid_midi_file(filename);
	if (midifile == NULL) {
		return FLUID_FAILED;
	}
	player->division = fluid_midi_file_get_division(midifile);

	if (fluid_midi_file_load_tracks(midifile, player) != FLUID_OK){
		return FLUID_FAILED;
	}
	delete_fluid_midi_file(midifile);
	return FLUID_OK;
}

int fluid_player_callback(void* data, unsigned int msec)
{
	int i;
	int status = FLUID_PLAYER_DONE;
	fluid_player_t* player;
	fluid_synth_t* synth;
	player = (fluid_player_t*)data;
	synth = player->synth;

	while (player->current_file == NULL) {
		if (player->playlist == NULL) {
			return 0;
		}

		fluid_player_reset(player);

		player->current_file = (char*)fluid_list_get(player->playlist);
		player->playlist = fluid_list_next(player->playlist);

		FLUID_LOG3(FLUID_DBG, "%s: %d: Loading midifile %s", __FILE__, __LINE__, player->current_file);

		if (fluid_player_load(player, player->current_file) == FLUID_OK) {
			player->begin_msec = msec;
			player->start_msec = msec;
			player->start_ticks = 0;
			player->cur_ticks = 0;

			for (i = 0; i < player->ntracks; i++) {
				if (player->track[i] != NULL) {
					fluid_track_reset(player->track[i]);
				}
			}

		}
		else {
			player->current_file = NULL;
		}
	}

	player->cur_msec = msec;
	player->cur_ticks = (player->start_ticks +
		(int)((double)(player->cur_msec - player->start_msec) / player->deltatime));

	for (i = 0; i < player->ntracks; i++) {
		if (!fluid_track_eot(player->track[i])) {
			status = FLUID_PLAYER_PLAYING;
			if (fluid_track_send_events(player->track[i], synth, player, player->cur_ticks) != FLUID_OK) {
			}
		}
	}

	player->status = status;

	if (player->status == FLUID_PLAYER_DONE) {
		FLUID_LOG3(FLUID_DBG, "%s: %d: Duration=%.3f sec",
			__FILE__, __LINE__, (msec - player->begin_msec) / 1000.0);
		player->current_file = NULL;
	}

	return 1;
}

int fluid_player_play(fluid_player_t* player)
{
	if (player->status == FLUID_PLAYER_PLAYING) {
		return FLUID_OK;
	}

	if (player->playlist == NULL) {
		return FLUID_OK;
	}

	player->status = FLUID_PLAYER_PLAYING;

	player->timer = new_fluid_timer((int)player->deltatime, fluid_player_callback,
		(void*)player, 1, 0);
	if (player->timer == NULL) {
		return FLUID_FAILED;
	}
	return FLUID_OK;
}

int fluid_player_stop(fluid_player_t* player)
{
	if (player->timer != NULL) {
		delete_fluid_timer(player->timer);
	}
	player->status = FLUID_PLAYER_DONE;
	player->timer = NULL;
	return FLUID_OK;
}

int fluid_player_set_loop(fluid_player_t* player, int loop)
{
	player->loop = loop;
	return FLUID_OK;
}

int fluid_player_set_midi_tempo(fluid_player_t* player, int tempo)
{
	player->miditempo = tempo;
	player->deltatime = (double)tempo / player->division / 1000.0;
	player->start_msec = player->cur_msec;
	player->start_ticks = player->cur_ticks;

	FLUID_LOG4(FLUID_DBG, "tempo=%d, tick time=%f msec, cur time=%d msec, cur tick=%d",
		tempo, player->deltatime, player->cur_msec, player->cur_ticks);

	return FLUID_OK;
}

int fluid_player_set_bpm(fluid_player_t* player, int bpm)
{
	return fluid_player_set_midi_tempo(player, (int)((double)60 * 1e6 / bpm));
}

int fluid_player_join(fluid_player_t* player)
{
	return player->timer ? fluid_timer_join(player->timer) : FLUID_OK;
}

fluid_midi_parser_t* new_fluid_midi_parser()
{
	fluid_midi_parser_t* parser;
	parser = FLUID_NEW(fluid_midi_parser_t);
	if (parser == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return NULL;
	}
	parser->status = 0;
	return parser;
}

int delete_fluid_midi_parser(fluid_midi_parser_t* parser)
{
	FLUID_FREE(parser);
	return FLUID_OK;
}

fluid_midi_event_t* fluid_midi_parser_parse(fluid_midi_parser_t* parser, unsigned char c)
{
	if (c >= 0xF8){
		if (c == MIDI_SYSTEM_RESET){
			parser->event.type = c;
			parser->status = 0;
			return &parser->event;
		};
		return NULL;
	};

	if (c > 0xF0){
		parser->status = 0;
		return NULL;
	};

	if (c & 0x80){
		parser->channel = c & 0x0F;
		parser->status = c & 0xF0;

		parser->nr_bytes_total = fluid_midi_event_length(parser->status) - 1;

		parser->nr_bytes = 0;
		return NULL;
	};

	if (parser->status == 0){
		return NULL;
	};

	if (parser->nr_bytes < FLUID_MIDI_PARSER_MAX_PAR){
		parser->p[parser->nr_bytes] = c;
	};
	parser->nr_bytes++;

	if (parser->nr_bytes < parser->nr_bytes_total){
		return NULL;
	};

	parser->event.type = parser->status;
	parser->event.channel = parser->channel;
	parser->nr_bytes = 0;
	switch (parser->status){
		case NOTE_OFF:
		case NOTE_ON:
		case KEY_PRESSURE:
		case CONTROL_CHANGE:
		case PROGRAM_CHANGE:
		case CHANNEL_PRESSURE:
			parser->event.param1 = parser->p[0];
			parser->event.param2 = parser->p[1];
			break;
		case PITCH_BEND:
			parser->event.param1 = ((parser->p[1] << 7) | parser->p[0]);
			break;
		default:
			return NULL;
	}
	return &parser->event;
}

int fluid_midi_event_length(unsigned char event){
	if (event < 0xf0) {
		return remains_80e0[((event - 0x80) >> 4) & 0x0f];
	}
	else if (event < 0xf7) {
		return remains_f0f6[event - 0xf0];
	}
	else {
		return 1;
	}
}

int fluid_midi_send_event(fluid_synth_t* synth, fluid_player_t* player, fluid_midi_event_t* event)
{
	switch (event->type) {
		case NOTE_ON:
			if (fluid_synth_noteon(synth, event->channel, event->param1, event->param2) != FLUID_OK) {
				return FLUID_FAILED;
			}
			break;
		case NOTE_OFF:
			if (fluid_synth_noteoff(synth, event->channel, event->param1) != FLUID_OK) {
				return FLUID_FAILED;
			}
			break;
		case CONTROL_CHANGE:
			if (fluid_synth_cc(synth, event->channel, event->param1, event->param2) != FLUID_OK) {
				return FLUID_FAILED;
			}
			break;
		case MIDI_SET_TEMPO:
			if (player != NULL) {
				if (fluid_player_set_midi_tempo(player, event->param1) != FLUID_OK) {
					return FLUID_FAILED;
				}
			}
			break;
		case PROGRAM_CHANGE:
			if (fluid_synth_program_change(synth, event->channel, event->param1) != FLUID_OK) {
				return FLUID_FAILED;
			}
			break;
		case PITCH_BEND:
			if (fluid_synth_pitch_bend(synth, event->channel, event->param1) != FLUID_OK) {
				return FLUID_FAILED;
			}
			break;
		default:
			break;
	}
	return FLUID_OK;
}

//
//MERGED FILE END: fluid_midi.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: fluid_mod.c
//

void fluid_mod_clone(fluid_mod_t* mod, fluid_mod_t* src)
{
	mod->dest = src->dest;
	mod->src1 = src->src1;
	mod->flags1 = src->flags1;
	mod->src2 = src->src2;
	mod->flags2 = src->flags2;
	mod->amount = src->amount;
}

void fluid_mod_set_source1(fluid_mod_t* mod, int src, int flags)
{
	mod->src1 = src;
	mod->flags1 = flags;
}

void fluid_mod_set_source2(fluid_mod_t* mod, int src, int flags)
{
	mod->src2 = src;
	mod->flags2 = flags;
}

void fluid_mod_set_dest(fluid_mod_t* mod, int dest)
{
	mod->dest = dest;
}

void fluid_mod_set_amount(fluid_mod_t* mod, double amount)
{
	mod->amount = (double)amount;
}

int fluid_mod_get_source1(fluid_mod_t* mod)
{
	return mod->src1;
}

int fluid_mod_get_flags1(fluid_mod_t* mod)
{
	return mod->flags1;
}

int fluid_mod_get_source2(fluid_mod_t* mod)
{
	return mod->src2;
}

int fluid_mod_get_flags2(fluid_mod_t* mod)
{
	return mod->flags2;
}

int fluid_mod_get_dest(fluid_mod_t* mod)
{
	return mod->dest;
}

double fluid_mod_get_amount(fluid_mod_t* mod)
{
	return (fluid_real_t)mod->amount;
}

fluid_real_t
fluid_mod_get_value(fluid_mod_t* mod, fluid_channel_t* chan, fluid_voice_t* voice)
{
	fluid_real_t v1 = 0.0, v2 = 1.0;
	fluid_real_t range1 = 127.0, range2 = 127.0;

	if (chan == NULL) {
		return 0.0f;
	}

	if ((mod->src2 == FLUID_MOD_VELOCITY) &&
		(mod->src1 == FLUID_MOD_VELOCITY) &&
		(mod->flags1 == (FLUID_MOD_GC | FLUID_MOD_UNIPOLAR
		| FLUID_MOD_NEGATIVE | FLUID_MOD_LINEAR)) &&
		(mod->flags2 == (FLUID_MOD_GC | FLUID_MOD_UNIPOLAR
		| FLUID_MOD_POSITIVE | FLUID_MOD_SWITCH)) &&
		(mod->dest == GEN_FILTERFC)) {
		return 0;
	}

	if (mod->src1 > 0) {
		if (mod->flags1 & FLUID_MOD_CC) {
			v1 = fluid_channel_get_cc(chan, mod->src1);
		}
		else {
			switch (mod->src1) {
				case FLUID_MOD_NONE:
					v1 = range1;
					break;
				case FLUID_MOD_VELOCITY:
					v1 = voice->vel;
					break;
				case FLUID_MOD_KEY:
					v1 = voice->key;
					break;
				case FLUID_MOD_KEYPRESSURE:
					v1 = chan->key_pressure;
					break;
				case FLUID_MOD_CHANNELPRESSURE:
					v1 = chan->channel_pressure;
					break;
				case FLUID_MOD_PITCHWHEEL:
					v1 = chan->pitch_bend;
					range1 = 0x4000;
					break;
				case FLUID_MOD_PITCHWHEELSENS:
					v1 = chan->pitch_wheel_sensitivity;
					break;
				default:
					v1 = 0.0;
			}
		}

		switch (mod->flags1 & 0x0f) {
			case 0:
				v1 /= range1;
				break;
			case 1:
				v1 = 1.0f - v1 / range1;
				break;
			case 2:
				v1 = -1.0f + 2.0f * v1 / range1;
				break;
			case 3:
				v1 = 1.0f - 2.0f * v1 / range1;
				break;
			case 4:
				v1 = fluid_concave(v1);
				break;
			case 5:
				v1 = fluid_concave(127 - v1);
				break;
			case 6:
				v1 = (v1 > 64) ? fluid_concave(2 * (v1 - 64)) : -fluid_concave(2 * (64 - v1));
				break;
			case 7:
				v1 = (v1 > 64) ? -fluid_concave(2 * (v1 - 64)) : fluid_concave(2 * (64 - v1));
				break;
			case 8:
				v1 = fluid_convex(v1);
				break;
			case 9:
				v1 = fluid_convex(127 - v1);
				break;
			case 10:
				v1 = (v1 > 64) ? fluid_convex(2 * (v1 - 64)) : -fluid_convex(2 * (64 - v1));
				break;
			case 11:
				v1 = (v1 > 64) ? -fluid_convex(2 * (v1 - 64)) : fluid_convex(2 * (64 - v1));
				break;
			case 12:
				v1 = (v1 >= 64) ? 1.0f : 0.0f;
				break;
			case 13:
				v1 = (v1 >= 64) ? 0.0f : 1.0f;
				break;
			case 14:
				v1 = (v1 >= 64) ? 1.0f : -1.0f;
				break;
			case 15:
				v1 = (v1 >= 64) ? -1.0f : 1.0f;
				break;
		}
	}
	else {
		return 0.0;
	}

	if (v1 == 0.0f) {
		return 0.0f;
	}

	if (mod->src2 > 0) {
		if (mod->flags2 & FLUID_MOD_CC) {
			v2 = fluid_channel_get_cc(chan, mod->src2);
		}
		else {
			switch (mod->src2) {
				case FLUID_MOD_NONE:
					v2 = range2;
					break;
				case FLUID_MOD_VELOCITY:
					v2 = voice->vel;
					break;
				case FLUID_MOD_KEY:
					v2 = voice->key;
					break;
				case FLUID_MOD_KEYPRESSURE:
					v2 = chan->key_pressure;
					break;
				case FLUID_MOD_CHANNELPRESSURE:
					v2 = chan->channel_pressure;
					break;
				case FLUID_MOD_PITCHWHEEL:
					v2 = chan->pitch_bend;
					break;
				case FLUID_MOD_PITCHWHEELSENS:
					v2 = chan->pitch_wheel_sensitivity;
					break;
				default:
					v1 = 0.0f;
			}
		}

		switch (mod->flags2 & 0x0f) {
			case 0:
				v2 /= range2;
				break;
			case 1:
				v2 = 1.0f - v2 / range2;
				break;
			case 2:
				v2 = -1.0f + 2.0f * v2 / range2;
				break;
			case 3:
				v2 = -1.0f + 2.0f * v2 / range2;
				break;
			case 4:
				v2 = fluid_concave(v2);
				break;
			case 5:
				v2 = fluid_concave(127 - v2);
				break;
			case 6:
				v2 = (v2 > 64) ? fluid_concave(2 * (v2 - 64)) : -fluid_concave(2 * (64 - v2));
				break;
			case 7:
				v2 = (v2 > 64) ? -fluid_concave(2 * (v2 - 64)) : fluid_concave(2 * (64 - v2));
				break;
			case 8:
				v2 = fluid_convex(v2);
				break;
			case 9:
				v2 = 1.0f - fluid_convex(v2);
				break;
			case 10:
				v2 = (v2 > 64) ? -fluid_convex(2 * (v2 - 64)) : fluid_convex(2 * (64 - v2));
				break;
			case 11:
				v2 = (v2 > 64) ? -fluid_convex(2 * (v2 - 64)) : fluid_convex(2 * (64 - v2));
				break;
			case 12:
				v2 = (v2 >= 64) ? 1.0f : 0.0f;
				break;
			case 13:
				v2 = (v2 >= 64) ? 0.0f : 1.0f;
				break;
			case 14:
				v2 = (v2 >= 64) ? 1.0f : -1.0f;
				break;
			case 15:
				v2 = (v2 >= 64) ? -1.0f : 1.0f;
				break;
		}
	}
	else {
		v2 = 1.0f;
	}

	return (fluid_real_t)mod->amount * v1 * v2;
}

fluid_mod_t*
fluid_mod_new()
{
	fluid_mod_t* mod = FLUID_NEW(fluid_mod_t);
	if (mod == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return NULL;
	}
	return mod;
}

void fluid_mod_delete(fluid_mod_t * mod)
{
	FLUID_FREE(mod);
}

int fluid_mod_test_identity(fluid_mod_t * mod1, fluid_mod_t * mod2){
	if (mod1->dest != mod2->dest){ return 0; };
	if (mod1->src1 != mod2->src1){ return 0; };
	if (mod1->src2 != mod2->src2){ return 0; };
	if (mod1->flags1 != mod2->flags1){ return 0; }
	if (mod1->flags2 != mod2->flags2){ return 0; }
	return 1;
}

void fluid_dump_modulator(fluid_mod_t * mod){
	int src1 = mod->src1;
	int dest = mod->dest;
	int src2 = mod->src2;
	int flags1 = mod->flags1;
	int flags2 = mod->flags2;
	fluid_real_t amount = (fluid_real_t)mod->amount;

	printf("Src: ");
	if (flags1 & FLUID_MOD_CC){
		printf("MIDI CC=%i", src1);
	}
	else {
		switch (src1){
			case FLUID_MOD_NONE:
				printf("None"); break;
			case FLUID_MOD_VELOCITY:
				printf("note-on velocity"); break;
			case FLUID_MOD_KEY:
				printf("Key nr"); break;
			case FLUID_MOD_KEYPRESSURE:
				printf("Poly pressure"); break;
			case FLUID_MOD_CHANNELPRESSURE:
				printf("Chan pressure"); break;
			case FLUID_MOD_PITCHWHEEL:
				printf("Pitch Wheel"); break;
			case FLUID_MOD_PITCHWHEELSENS:
				printf("Pitch Wheel sens"); break;
			default:
				printf("(unknown: %i)", src1);
		};
	};
	if (flags1 & FLUID_MOD_NEGATIVE){ printf("- "); }
	else { printf("+ "); };
	if (flags1 & FLUID_MOD_BIPOLAR){ printf("bip "); }
	else { printf("unip "); };
	printf("-> ");
	switch (dest){
		case GEN_FILTERQ: printf("Q"); break;
		case GEN_FILTERFC: printf("fc"); break;
		case GEN_VIBLFOTOPITCH: printf("VibLFO-to-pitch"); break;
		case GEN_MODENVTOPITCH: printf("ModEnv-to-pitch"); break;
		case GEN_MODLFOTOPITCH: printf("ModLFO-to-pitch"); break;
		case GEN_CHORUSSEND: printf("Chorus send"); break;
		case GEN_REVERBSEND: printf("Reverb send"); break;
		case GEN_PAN: printf("pan"); break;
		case GEN_ATTENUATION: printf("att"); break;
		default: printf("dest %i", dest);
	};
	printf(", amount %f flags %i src2 %i flags2 %i\n", amount, flags1, src2, flags2);
}

//
//MERGED FILE END: fluid_mod.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: fluid_rev.c
//

# if defined(WITH_FLOATX)
# define zap_almost_zero(sample) (((*(unsigned int*)&(sample))&0x7f800000) < 0x08000000)?0.0f:(sample)
# else

#define zap_almost_zero(sample) fabs(sample)<1e-10 ? 0 : sample;
#endif

#define DC_OFFSET 1e-8
typedef struct _fluid_allpass fluid_allpass;
typedef struct _fluid_comb fluid_comb;

struct _fluid_allpass {
	fluid_real_t feedback;
	fluid_real_t *buffer;
	int bufsize;
	int bufidx;
};

void fluid_allpass_setbuffer(fluid_allpass* allpass, fluid_real_t *buf, int size);
void fluid_allpass_init(fluid_allpass* allpass);
void fluid_allpass_setfeedback(fluid_allpass* allpass, fluid_real_t val);
fluid_real_t fluid_allpass_getfeedback(fluid_allpass* allpass);

void fluid_allpass_setbuffer(fluid_allpass* allpass, fluid_real_t *buf, int size)
{
	allpass->bufidx = 0;
	allpass->buffer = buf;
	allpass->bufsize = size;
}

void fluid_allpass_init(fluid_allpass* allpass)
{
	int i;
	int len = allpass->bufsize;
	fluid_real_t* buf = allpass->buffer;
	for (i = 0; i < len; i++) {
		buf[i] = DC_OFFSET;
	}
}

void fluid_allpass_setfeedback(fluid_allpass* allpass, fluid_real_t val)
{
	allpass->feedback = val;
}

fluid_real_t
fluid_allpass_getfeedback(fluid_allpass* allpass)
{
	return allpass->feedback;
}

#define fluid_allpass_process(_allpass, _input) \
{ \
  fluid_real_t output; \
  fluid_real_t bufout; \
  bufout = _allpass.buffer[_allpass.bufidx]; \
  output = bufout-_input; \
  _allpass.buffer[_allpass.bufidx] = _input + (bufout * _allpass.feedback); \
  if (++_allpass.bufidx >= _allpass.bufsize) { \
    _allpass.bufidx = 0; \
      } \
  _input = output; \
}

struct _fluid_comb {
	fluid_real_t feedback;
	fluid_real_t filterstore;
	fluid_real_t damp1;
	fluid_real_t damp2;
	fluid_real_t *buffer;
	int bufsize;
	int bufidx;
};

void fluid_comb_setbuffer(fluid_comb* comb, fluid_real_t *buf, int size);
void fluid_comb_init(fluid_comb* comb);
void fluid_comb_setdamp(fluid_comb* comb, fluid_real_t val);
fluid_real_t fluid_comb_getdamp(fluid_comb* comb);
void fluid_comb_setfeedback(fluid_comb* comb, fluid_real_t val);
fluid_real_t fluid_comb_getfeedback(fluid_comb* comb);

void fluid_comb_setbuffer(fluid_comb* comb, fluid_real_t *buf, int size)
{
	comb->filterstore = 0;
	comb->bufidx = 0;
	comb->buffer = buf;
	comb->bufsize = size;
}

void fluid_comb_init(fluid_comb* comb)
{
	int i;
	fluid_real_t* buf = comb->buffer;
	int len = comb->bufsize;
	for (i = 0; i < len; i++) {
		buf[i] = DC_OFFSET;
	}
}

void fluid_comb_setdamp(fluid_comb* comb, fluid_real_t val)
{
	comb->damp1 = val;
	comb->damp2 = 1 - val;
}

fluid_real_t
fluid_comb_getdamp(fluid_comb* comb)
{
	return comb->damp1;
}

void fluid_comb_setfeedback(fluid_comb* comb, fluid_real_t val)
{
	comb->feedback = val;
}

fluid_real_t
fluid_comb_getfeedback(fluid_comb* comb)
{
	return comb->feedback;
}

#define fluid_comb_process(_comb, _input, _output) \
{ \
  fluid_real_t _tmp = _comb.buffer[_comb.bufidx]; \
  _comb.filterstore = (_tmp * _comb.damp2) + (_comb.filterstore * _comb.damp1); \
  _comb.buffer[_comb.bufidx] = _input + (_comb.filterstore * _comb.feedback); \
  if (++_comb.bufidx >= _comb.bufsize) { \
    _comb.bufidx = 0; \
      } \
  _output += _tmp; \
}

#define numcombs 8
#define numallpasses 4
#define	fixedgain 0.015f
#define scalewet 3.0f
#define scaledamp 1.0f
#define scaleroom 0.28f
#define offsetroom 0.7f
#define initialroom 0.5f
#define initialdamp 0.2f
#define initialwet 1
#define initialdry 0
#define initialwidth 1
#define stereospread 23

#define combtuningL1 1116
#define combtuningR1 1116 + stereospread
#define combtuningL2 1188
#define combtuningR2 1188 + stereospread
#define combtuningL3 1277
#define combtuningR3 1277 + stereospread
#define combtuningL4 1356
#define combtuningR4 1356 + stereospread
#define combtuningL5 1422
#define combtuningR5 1422 + stereospread
#define combtuningL6 1491
#define combtuningR6 1491 + stereospread
#define combtuningL7 1557
#define combtuningR7 1557 + stereospread
#define combtuningL8 1617
#define combtuningR8 1617 + stereospread
#define allpasstuningL1 556
#define allpasstuningR1 556 + stereospread
#define allpasstuningL2 441
#define allpasstuningR2 441 + stereospread
#define allpasstuningL3 341
#define allpasstuningR3 341 + stereospread
#define allpasstuningL4 225
#define allpasstuningR4 225 + stereospread

struct _fluid_revmodel_t {
	fluid_real_t roomsize;
	fluid_real_t damp;
	fluid_real_t wet, wet1, wet2;
	fluid_real_t width;
	fluid_real_t gain;

	fluid_comb combL[numcombs];
	fluid_comb combR[numcombs];

	fluid_allpass allpassL[numallpasses];
	fluid_allpass allpassR[numallpasses];

	fluid_real_t bufcombL1[combtuningL1];
	fluid_real_t bufcombR1[combtuningR1];
	fluid_real_t bufcombL2[combtuningL2];
	fluid_real_t bufcombR2[combtuningR2];
	fluid_real_t bufcombL3[combtuningL3];
	fluid_real_t bufcombR3[combtuningR3];
	fluid_real_t bufcombL4[combtuningL4];
	fluid_real_t bufcombR4[combtuningR4];
	fluid_real_t bufcombL5[combtuningL5];
	fluid_real_t bufcombR5[combtuningR5];
	fluid_real_t bufcombL6[combtuningL6];
	fluid_real_t bufcombR6[combtuningR6];
	fluid_real_t bufcombL7[combtuningL7];
	fluid_real_t bufcombR7[combtuningR7];
	fluid_real_t bufcombL8[combtuningL8];
	fluid_real_t bufcombR8[combtuningR8];

	fluid_real_t bufallpassL1[allpasstuningL1];
	fluid_real_t bufallpassR1[allpasstuningR1];
	fluid_real_t bufallpassL2[allpasstuningL2];
	fluid_real_t bufallpassR2[allpasstuningR2];
	fluid_real_t bufallpassL3[allpasstuningL3];
	fluid_real_t bufallpassR3[allpasstuningR3];
	fluid_real_t bufallpassL4[allpasstuningL4];
	fluid_real_t bufallpassR4[allpasstuningR4];
};

void fluid_revmodel_update(fluid_revmodel_t* rev);
void fluid_revmodel_init(fluid_revmodel_t* rev);

fluid_revmodel_t*
new_fluid_revmodel()
{
	fluid_revmodel_t* rev;
	rev = FLUID_NEW(fluid_revmodel_t);
	if (rev == NULL) {
		return NULL;
	}

	fluid_comb_setbuffer(&rev->combL[0], rev->bufcombL1, combtuningL1);
	fluid_comb_setbuffer(&rev->combR[0], rev->bufcombR1, combtuningR1);
	fluid_comb_setbuffer(&rev->combL[1], rev->bufcombL2, combtuningL2);
	fluid_comb_setbuffer(&rev->combR[1], rev->bufcombR2, combtuningR2);
	fluid_comb_setbuffer(&rev->combL[2], rev->bufcombL3, combtuningL3);
	fluid_comb_setbuffer(&rev->combR[2], rev->bufcombR3, combtuningR3);
	fluid_comb_setbuffer(&rev->combL[3], rev->bufcombL4, combtuningL4);
	fluid_comb_setbuffer(&rev->combR[3], rev->bufcombR4, combtuningR4);
	fluid_comb_setbuffer(&rev->combL[4], rev->bufcombL5, combtuningL5);
	fluid_comb_setbuffer(&rev->combR[4], rev->bufcombR5, combtuningR5);
	fluid_comb_setbuffer(&rev->combL[5], rev->bufcombL6, combtuningL6);
	fluid_comb_setbuffer(&rev->combR[5], rev->bufcombR6, combtuningR6);
	fluid_comb_setbuffer(&rev->combL[6], rev->bufcombL7, combtuningL7);
	fluid_comb_setbuffer(&rev->combR[6], rev->bufcombR7, combtuningR7);
	fluid_comb_setbuffer(&rev->combL[7], rev->bufcombL8, combtuningL8);
	fluid_comb_setbuffer(&rev->combR[7], rev->bufcombR8, combtuningR8);
	fluid_allpass_setbuffer(&rev->allpassL[0], rev->bufallpassL1, allpasstuningL1);
	fluid_allpass_setbuffer(&rev->allpassR[0], rev->bufallpassR1, allpasstuningR1);
	fluid_allpass_setbuffer(&rev->allpassL[1], rev->bufallpassL2, allpasstuningL2);
	fluid_allpass_setbuffer(&rev->allpassR[1], rev->bufallpassR2, allpasstuningR2);
	fluid_allpass_setbuffer(&rev->allpassL[2], rev->bufallpassL3, allpasstuningL3);
	fluid_allpass_setbuffer(&rev->allpassR[2], rev->bufallpassR3, allpasstuningR3);
	fluid_allpass_setbuffer(&rev->allpassL[3], rev->bufallpassL4, allpasstuningL4);
	fluid_allpass_setbuffer(&rev->allpassR[3], rev->bufallpassR4, allpasstuningR4);

	fluid_allpass_setfeedback(&rev->allpassL[0], 0.5f);
	fluid_allpass_setfeedback(&rev->allpassR[0], 0.5f);
	fluid_allpass_setfeedback(&rev->allpassL[1], 0.5f);
	fluid_allpass_setfeedback(&rev->allpassR[1], 0.5f);
	fluid_allpass_setfeedback(&rev->allpassL[2], 0.5f);
	fluid_allpass_setfeedback(&rev->allpassR[2], 0.5f);
	fluid_allpass_setfeedback(&rev->allpassL[3], 0.5f);
	fluid_allpass_setfeedback(&rev->allpassR[3], 0.5f);

	rev->roomsize = initialroom * scaleroom + offsetroom;
	rev->damp = initialdamp * scaledamp;
	rev->wet = initialwet * scalewet;
	rev->width = initialwidth;
	rev->gain = fixedgain;

	fluid_revmodel_update(rev);

	fluid_revmodel_init(rev);
	return rev;
}

void delete_fluid_revmodel(fluid_revmodel_t* rev)
{
	FLUID_FREE(rev);
}

void fluid_revmodel_init(fluid_revmodel_t* rev)
{
	int i;
	for (i = 0; i < numcombs; i++) {
		fluid_comb_init(&rev->combL[i]);
		fluid_comb_init(&rev->combR[i]);
	}
	for (i = 0; i < numallpasses; i++) {
		fluid_allpass_init(&rev->allpassL[i]);
		fluid_allpass_init(&rev->allpassR[i]);
	}
}

void fluid_revmodel_reset(fluid_revmodel_t* rev)
{
	fluid_revmodel_init(rev);
}

void fluid_revmodel_processreplace(fluid_revmodel_t* rev, fluid_real_t *in,
fluid_real_t *left_out, fluid_real_t *right_out)
{
	int i, k = 0;
	fluid_real_t outL, outR, input;

	for (k = 0; k < FLUID_BUFSIZE; k++) {
		outL = outR = 0;

		input = (2 * in[k] + DC_OFFSET) * rev->gain;

		for (i = 0; i < numcombs; i++) {
			fluid_comb_process(rev->combL[i], input, outL);
			fluid_comb_process(rev->combR[i], input, outR);
		}

		for (i = 0; i < numallpasses; i++) {
			fluid_allpass_process(rev->allpassL[i], outL);
			fluid_allpass_process(rev->allpassR[i], outR);
		}

		outL -= DC_OFFSET;
		outR -= DC_OFFSET;

		left_out[k] = outL * rev->wet1 + outR * rev->wet2;
		right_out[k] = outR * rev->wet1 + outL * rev->wet2;
	}
}

void fluid_revmodel_processmix(fluid_revmodel_t* rev, fluid_real_t *in,
fluid_real_t *left_out, fluid_real_t *right_out)
{
	int i, k = 0;
	fluid_real_t outL, outR, input;

	for (k = 0; k < FLUID_BUFSIZE; k++) {
		outL = outR = 0;

		input = (2 * in[k] + DC_OFFSET) * rev->gain;

		for (i = 0; i < numcombs; i++) {
			fluid_comb_process(rev->combL[i], input, outL);
			fluid_comb_process(rev->combR[i], input, outR);
		}

		for (i = 0; i < numallpasses; i++) {
			fluid_allpass_process(rev->allpassL[i], outL);
			fluid_allpass_process(rev->allpassR[i], outR);
		}

		outL -= DC_OFFSET;
		outR -= DC_OFFSET;

		left_out[k] += outL * rev->wet1 + outR * rev->wet2;
		right_out[k] += outR * rev->wet1 + outL * rev->wet2;
	}
}

void fluid_revmodel_update(fluid_revmodel_t* rev)
{
	int i;

	rev->wet1 = rev->wet * (rev->width / 2 + 0.5f);
	rev->wet2 = rev->wet * ((1 - rev->width) / 2);

	for (i = 0; i < numcombs; i++) {
		fluid_comb_setfeedback(&rev->combL[i], rev->roomsize);
		fluid_comb_setfeedback(&rev->combR[i], rev->roomsize);
	}

	for (i = 0; i < numcombs; i++) {
		fluid_comb_setdamp(&rev->combL[i], rev->damp);
		fluid_comb_setdamp(&rev->combR[i], rev->damp);
	}
}

void fluid_revmodel_setroomsize(fluid_revmodel_t* rev, fluid_real_t value)
{
	rev->roomsize = (value * scaleroom) + offsetroom;
	fluid_revmodel_update(rev);
}

fluid_real_t
fluid_revmodel_getroomsize(fluid_revmodel_t* rev)
{
	return (rev->roomsize - offsetroom) / scaleroom;
}

void fluid_revmodel_setdamp(fluid_revmodel_t* rev, fluid_real_t value)
{
	rev->damp = value * scaledamp;
	fluid_revmodel_update(rev);
}

fluid_real_t
fluid_revmodel_getdamp(fluid_revmodel_t* rev)
{
	return rev->damp / scaledamp;
}

void fluid_revmodel_setlevel(fluid_revmodel_t* rev, fluid_real_t value)
{
	fluid_clip(value, 0.0f, 1.0f);
	rev->wet = value * scalewet;
	fluid_revmodel_update(rev);
}

fluid_real_t
fluid_revmodel_getlevel(fluid_revmodel_t* rev)
{
	return rev->wet / scalewet;
}

void fluid_revmodel_setwidth(fluid_revmodel_t* rev, fluid_real_t value)
{
	rev->width = value;
	fluid_revmodel_update(rev);
}

fluid_real_t
fluid_revmodel_getwidth(fluid_revmodel_t* rev)
{
	return rev->width;
}

//
//MERGED FILE END: fluid_rev.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: fluid_seq.c
//

#define FLUID_SEQUENCER_EVENTS_MAX	1000

struct _fluid_sequencer_t {
	unsigned int startMs;
	double scale; 	fluid_list_t* clients;
	short clientsID;

	fluid_evt_entry* preQueue;
	fluid_evt_entry* preQueueLast;
	fluid_timer_t* timer;
	int queue0StartTime;
	short prevCellNb;
	fluid_evt_entry* queue0[256][2];
	fluid_evt_entry* queue1[255][2];
	fluid_evt_entry* queueLater;
	fluid_evt_heap_t* heap;
	fluid_mutex_t mutex;
#if FLUID_SEQ_WITH_TRACE
	char *tracebuf;
	char *traceptr;
	int tracelen;
#endif
};

typedef struct _fluid_sequencer_client_t {
	short id;
	char* name;
	fluid_event_callback_t callback;
	void* data;
} fluid_sequencer_client_t;

short _fluid_seq_queue_init(fluid_sequencer_t* seq, int nbEvents);
void _fluid_seq_queue_end(fluid_sequencer_t* seq);
short _fluid_seq_queue_pre_insert(fluid_sequencer_t* seq, fluid_event_t * evt);
void _fluid_seq_queue_pre_remove(fluid_sequencer_t* seq, short src, short dest, int type);
int _fluid_seq_queue_process(void* data, unsigned int msec);

fluid_sequencer_t*
new_fluid_sequencer()
{
	fluid_sequencer_t* seq;

	seq = FLUID_NEW(fluid_sequencer_t);
	if (seq == NULL) {
		fluid_log(FLUID_PANIC, "sequencer: Out of memory\n");
		return NULL;
	}

	FLUID_MEMSET(seq, 0, sizeof(fluid_sequencer_t));

	seq->scale = 1000;		seq->startMs = fluid_curtime();
	seq->clients = NULL;
	seq->clientsID = 0;

	if (-1 == _fluid_seq_queue_init(seq, FLUID_SEQUENCER_EVENTS_MAX)) {
		FLUID_FREE(seq);
		fluid_log(FLUID_PANIC, "sequencer: Out of memory\n");
		return NULL;
	}

#if FLUID_SEQ_WITH_TRACE
	seq->tracelen = 1024 * 100;
	seq->tracebuf = (char *)FLUID_MALLOC(seq->tracelen);
	if (seq->tracebuf == NULL) {
		_fluid_seq_queue_end(seq);
		FLUID_FREE(seq);
		fluid_log(FLUID_PANIC, "sequencer: Out of memory\n");
		return NULL;
	}
	seq->traceptr = seq->tracebuf;
#endif

	return(seq);
}

void delete_fluid_sequencer(fluid_sequencer_t* seq)
{
	if (seq == NULL) {
		return;
	}

	_fluid_seq_queue_end(seq);

	if (seq->clients) {
		fluid_list_t *tmp = seq->clients;
		while (tmp != NULL) {
			fluid_sequencer_client_t *client = (fluid_sequencer_client_t*)tmp->data;
			if (client->name) FLUID_FREE(client->name);
			tmp = tmp->next;
		}
		delete_fluid_list(seq->clients);
		seq->clients = NULL;
	}

#if FLUID_SEQ_WITH_TRACE
	if (seq->tracebuf != NULL)
		FLUID_FREE(seq->tracebuf);
	seq->tracebuf = NULL;
#endif

	FLUID_FREE(seq);
}

#if FLUID_SEQ_WITH_TRACE

void fluid_seq_dotrace(fluid_sequencer_t* seq, char *fmt, ...)
{
	va_list args;
	int len, remain = seq->tracelen - (seq->traceptr - seq->tracebuf);
	if (remain <= 0) return;

	va_start(args, fmt);
	len = vsnprintf(seq->traceptr, remain, fmt, args);
	va_end(args);

	if (len > 0) {
		if (len <= remain) {
			seq->traceptr += len;
		}
		else {
			seq->traceptr = seq->tracebuf + seq->tracelen;
		}
	}

	return;
}

void fluid_seq_cleartrace(fluid_sequencer_t* seq)
{
	seq->traceptr = seq->tracebuf;
}

char *
fluid_seq_gettrace(fluid_sequencer_t* seq)
{
	return seq->tracebuf;
}
#else

void fluid_seq_dotrace(fluid_sequencer_t* seq, char *fmt, ...) {}

#endif

short fluid_sequencer_register_client(fluid_sequencer_t* seq, char* name,
	fluid_event_callback_t callback, void* data) {
	fluid_sequencer_client_t * client;
	char * nameCopy;

	client = FLUID_NEW(fluid_sequencer_client_t);
	if (client == NULL) {
		fluid_log(FLUID_PANIC, "sequencer: Out of memory\n");
		return -1;
	}

	nameCopy = FLUID_STRDUP(name);
	if (nameCopy == NULL) {
		fluid_log(FLUID_PANIC, "sequencer: Out of memory\n");
		return -1;
	}

	seq->clientsID++;

	client->name = nameCopy;
	client->id = seq->clientsID;
	client->callback = callback;
	client->data = data;

	seq->clients = fluid_list_append(seq->clients, (void *)client);

	return (client->id);
}

void fluid_sequencer_unregister_client(fluid_sequencer_t* seq, short id)
{
	fluid_list_t *tmp;

	if (seq->clients == NULL) return;

	tmp = seq->clients;
	while (tmp) {
		fluid_sequencer_client_t *client = (fluid_sequencer_client_t*)tmp->data;

		if (client->id == id) {
			if (client->name)
				FLUID_FREE(client->name);
			seq->clients = fluid_list_remove_link(seq->clients, tmp);
			delete1_fluid_list(tmp);
			return;
		}
		tmp = tmp->next;
	}
	return;
}

int fluid_sequencer_count_clients(fluid_sequencer_t* seq)
{
	if (seq->clients == NULL)
		return 0;
	return fluid_list_size(seq->clients);
}

short fluid_sequencer_get_client_id(fluid_sequencer_t* seq, int index)
{
	fluid_list_t *tmp = fluid_list_nth(seq->clients, index);
	if (tmp == NULL) {
		return -1;
	}
	else {
		fluid_sequencer_client_t *client = (fluid_sequencer_client_t*)tmp->data;
		return client->id;
	}
}

char* fluid_sequencer_get_client_name(fluid_sequencer_t* seq, int id)
{
	fluid_list_t *tmp;

	if (seq->clients == NULL)
		return NULL;

	tmp = seq->clients;
	while (tmp) {
		fluid_sequencer_client_t *client = (fluid_sequencer_client_t*)tmp->data;

		if (client->id == id)
			return client->name;

		tmp = tmp->next;
	}
	return NULL;
}

int fluid_sequencer_client_is_dest(fluid_sequencer_t* seq, int id)
{
	fluid_list_t *tmp;

	if (seq->clients == NULL) return 0;

	tmp = seq->clients;
	while (tmp) {
		fluid_sequencer_client_t *client = (fluid_sequencer_client_t*)tmp->data;

		if (client->id == id)
			return (client->callback != NULL);

		tmp = tmp->next;
	}
	return 0;
}

void fluid_sequencer_send_now(fluid_sequencer_t* seq, fluid_event_t* evt)
{
	short destID = fluid_event_get_dest(evt);

	fluid_list_t *tmp = seq->clients;
	while (tmp) {
		fluid_sequencer_client_t *dest = (fluid_sequencer_client_t*)tmp->data;

		if (dest->id == destID) {
			if (dest->callback)
				(dest->callback)(fluid_sequencer_get_tick(seq),
				evt, seq, dest->data);
			return;
		}
		tmp = tmp->next;
	}
}

int fluid_sequencer_send_at(fluid_sequencer_t* seq, fluid_event_t* evt, unsigned int time, int absolute)
{
	unsigned int now = fluid_sequencer_get_tick(seq);

	if (!absolute)
		time = now + time;

	fluid_event_set_time(evt, time);

	if (time < now) {
		fluid_sequencer_send_now(seq, evt);
		return 0;
	}

	if (time == now) {
		fluid_sequencer_send_now(seq, evt);
		return 0;
	}

	return _fluid_seq_queue_pre_insert(seq, evt);
}

void fluid_sequencer_remove_events(fluid_sequencer_t* seq, short source, short dest, int type)
{
	_fluid_seq_queue_pre_remove(seq, source, dest, type);
}

unsigned int fluid_sequencer_get_tick(fluid_sequencer_t* seq)
{
	unsigned int absMs = fluid_curtime();
	double nowFloat;
	unsigned int now;
	nowFloat = ((double)(absMs - seq->startMs))*seq->scale / 1000.0f;
	now = nowFloat;
	return now;
}

void fluid_sequencer_set_time_scale(fluid_sequencer_t* seq, double scale)
{
	if (scale <= 0) {
		fluid_log(FLUID_WARN, "sequencer: scale <= 0 : %f\n", scale);
		return;
	}

	if (scale > 1000.0)
		scale = 1000.0;

	if (seq->scale != scale) {
		double oldScale = seq->scale;

		if (seq->timer) {
			delete_fluid_timer(seq->timer);
			seq->timer = NULL;
		}

		seq->scale = scale;

		seq->queue0StartTime = (seq->queue0StartTime + seq->prevCellNb)*(seq->scale / oldScale) - seq->prevCellNb;

		{
			fluid_evt_entry* tmp;
			tmp = seq->preQueue;
			while (tmp) {
				if (tmp->entryType == FLUID_EVT_ENTRY_INSERT)
					tmp->evt.time = tmp->evt.time*seq->scale / oldScale;

				tmp = tmp->next;
			}
		}

		seq->timer = new_fluid_timer((int)(1000 / seq->scale), _fluid_seq_queue_process, (void *)seq, 1, 0);
	}
}

double fluid_sequencer_get_time_scale(fluid_sequencer_t* seq)
{
	return seq->scale;
}

void _fluid_seq_queue_insert_entry(fluid_sequencer_t* seq, fluid_evt_entry * evtentry);
void _fluid_seq_queue_remove_entries_matching(fluid_sequencer_t* seq, fluid_evt_entry* temp);
void _fluid_seq_queue_send_queued_events(fluid_sequencer_t* seq);

short
_fluid_seq_queue_init(fluid_sequencer_t* seq, int maxEvents)
{
	seq->heap = _fluid_evt_heap_init(maxEvents);
	if (seq->heap == NULL) {
		fluid_log(FLUID_PANIC, "sequencer: Out of memory\n");
		return -1;
	}

	seq->preQueue = NULL;
	seq->preQueueLast = NULL;

	FLUID_MEMSET(seq->queue0, 0, 2 * 256 * sizeof(fluid_evt_entry *));
	FLUID_MEMSET(seq->queue1, 0, 2 * 255 * sizeof(fluid_evt_entry *));

	seq->queueLater = NULL;
	seq->queue0StartTime = fluid_sequencer_get_tick(seq);
	seq->prevCellNb = -1;

	fluid_mutex_init(seq->mutex);

	seq->timer = new_fluid_timer((int)(1000 / seq->scale), _fluid_seq_queue_process,
		(void *)seq, 1, 0);
	return (0);
}

void _fluid_seq_queue_end(fluid_sequencer_t* seq)
{
	if (seq->timer) {
		delete_fluid_timer(seq->timer);
		seq->timer = NULL;
	}

	if (seq->heap) {
		_fluid_evt_heap_free(seq->heap);
		seq->heap = NULL;
	}
	fluid_mutex_destroy(seq->mutex);
}

short
_fluid_seq_queue_pre_insert(fluid_sequencer_t* seq, fluid_event_t * evt)
{
	fluid_evt_entry * evtentry = _fluid_seq_heap_get_free(seq->heap);
	if (evtentry == NULL) {
		fluid_log(FLUID_PANIC, "sequencer: no more free events\n");
		return -1;
	}

	evtentry->next = NULL;
	evtentry->entryType = FLUID_EVT_ENTRY_INSERT;
	FLUID_MEMCPY(&(evtentry->evt), evt, sizeof(fluid_event_t));

	fluid_mutex_lock(seq->mutex);

	if (seq->preQueueLast) {
		seq->preQueueLast->next = evtentry;
	}
	else {
		seq->preQueue = evtentry;
	}
	seq->preQueueLast = evtentry;

	fluid_mutex_unlock(seq->mutex);

	return (0);
}

void _fluid_seq_queue_pre_remove(fluid_sequencer_t* seq, short src, short dest, int type)
{
	fluid_evt_entry * evtentry = _fluid_seq_heap_get_free(seq->heap);
	if (evtentry == NULL) {
		fluid_log(FLUID_PANIC, "sequencer: no more free events\n");
		return;
	}

	evtentry->next = NULL;
	evtentry->entryType = FLUID_EVT_ENTRY_REMOVE;
	{
		fluid_event_t* evt = &(evtentry->evt);
		fluid_event_set_source(evt, src);
		fluid_event_set_source(evt, src);
		fluid_event_set_dest(evt, dest);
		evt->type = type;
	}

	fluid_mutex_lock(seq->mutex);

	if (seq->preQueueLast) {
		seq->preQueueLast->next = evtentry;
	}
	else {
		seq->preQueue = evtentry;
	}
	seq->preQueueLast = evtentry;

	fluid_mutex_unlock(seq->mutex);
	return;
}

int _fluid_seq_queue_process(void* data, unsigned int msec)
{
	fluid_sequencer_t* seq = (fluid_sequencer_t *)data;

	fluid_evt_entry* tmp;
	fluid_evt_entry* next;

	fluid_mutex_lock(seq->mutex);

	tmp = seq->preQueue;
	seq->preQueue = NULL;
	seq->preQueueLast = NULL;

	fluid_mutex_unlock(seq->mutex);

	while (tmp) {
		next = tmp->next;

		if (tmp->entryType == FLUID_EVT_ENTRY_REMOVE) {
			_fluid_seq_queue_remove_entries_matching(seq, tmp);
		}
		else {
			_fluid_seq_queue_insert_entry(seq, tmp);
		}

		tmp = next;
	}

	_fluid_seq_queue_send_queued_events(seq);

	return 1;
}

void _fluid_seq_queue_print_later(fluid_sequencer_t* seq)
{
	int count = 0;
	fluid_evt_entry* tmp = seq->queueLater;

	printf("queueLater:\n");

	while (tmp) {
		unsigned int delay = tmp->evt.time - seq->queue0StartTime;
		printf("queueLater: Delay = %i\n", delay);
		tmp = tmp->next;
		count++;
	}
	printf("queueLater: Total of %i events\n", count);
}

void _fluid_seq_queue_insert_queue0(fluid_sequencer_t* seq, fluid_evt_entry* tmp, int cell)
{
	if (seq->queue0[cell][1] == NULL) {
		seq->queue0[cell][1] = seq->queue0[cell][0] = tmp;
	}
	else {
		seq->queue0[cell][1]->next = tmp;
		seq->queue0[cell][1] = tmp;
	}
	tmp->next = NULL;
}

void _fluid_seq_queue_insert_queue1(fluid_sequencer_t* seq, fluid_evt_entry* tmp, int cell)
{
	if (seq->queue1[cell][1] == NULL) {
		seq->queue1[cell][1] = seq->queue1[cell][0] = tmp;
	}
	else {
		seq->queue1[cell][1]->next = tmp;
		seq->queue1[cell][1] = tmp;
	}
	tmp->next = NULL;
}

void _fluid_seq_queue_insert_queue_later(fluid_sequencer_t* seq, fluid_evt_entry* evtentry)
{
	fluid_evt_entry* prev;
	fluid_evt_entry* tmp;
	unsigned int time = evtentry->evt.time;

	if ((seq->queueLater == NULL)
		|| (seq->queueLater->evt.time > time)) {
		evtentry->next = seq->queueLater;
		seq->queueLater = evtentry;
		return;
	}

	prev = seq->queueLater;
	tmp = prev->next;
	while (tmp) {
		if (tmp->evt.time > time) {
			evtentry->next = tmp;
			prev->next = evtentry;
			return;
		}
		prev = tmp;
		tmp = prev->next;
	}

	evtentry->next = NULL;
	prev->next = evtentry;
}

void _fluid_seq_queue_insert_entry(fluid_sequencer_t* seq, fluid_evt_entry * evtentry)
{
	fluid_event_t * evt = &(evtentry->evt);
	unsigned int time = evt->time;
	unsigned int delay;

	if (seq->queue0StartTime > 0) {
		if (time < (unsigned int)seq->queue0StartTime) {
			fluid_sequencer_send_now(seq, evt);

			_fluid_seq_heap_set_free(seq->heap, evtentry);
			return;
		}
	}

	if (seq->prevCellNb >= 0) {
		if (time <= (unsigned int)(seq->queue0StartTime + seq->prevCellNb)) {
			fluid_sequencer_send_now(seq, evt);

			_fluid_seq_heap_set_free(seq->heap, evtentry);
			return;
		}
	}

	delay = time - seq->queue0StartTime;

	if (delay > 65535) {
		_fluid_seq_queue_insert_queue_later(seq, evtentry);

	}
	else if (delay > 255) {
		_fluid_seq_queue_insert_queue1(seq, evtentry, delay / 256 - 1);

	}
	else {
		_fluid_seq_queue_insert_queue0(seq, evtentry, delay);
	}
}

int _fluid_seq_queue_matchevent(fluid_event_t* evt, int templType, short templSrc, short templDest)
{
	int eventType;

	if (templSrc != -1 && templSrc != fluid_event_get_source(evt))
		return 0;

	if (templDest != -1 && templDest != fluid_event_get_dest(evt))
		return 0;

	if (templType == -1)
		return 1;

	eventType = fluid_event_get_type(evt);

	if (templType == eventType)
		return 1;

	if (templType == FLUID_SEQ_ANYCONTROLCHANGE)
		if (eventType == FLUID_SEQ_PITCHBEND ||
			eventType == FLUID_SEQ_MODULATION ||
			eventType == FLUID_SEQ_SUSTAIN ||
			eventType == FLUID_SEQ_PAN ||
			eventType == FLUID_SEQ_VOLUME ||
			eventType == FLUID_SEQ_REVERBSEND ||
			eventType == FLUID_SEQ_CONTROLCHANGE ||
			eventType == FLUID_SEQ_CHORUSSEND)
			return 1;

	return 0;
}

void _fluid_seq_queue_remove_entries_matching(fluid_sequencer_t* seq, fluid_evt_entry* templ)
{
	int i, type;
	short src, dest;

	src = templ->evt.src;
	dest = templ->evt.dest;
	type = templ->evt.type;

	_fluid_seq_heap_set_free(seq->heap, templ);

	for (i = 0; i < 256; i++) {
		fluid_evt_entry* tmp = seq->queue0[i][0];
		fluid_evt_entry* prev = NULL;
		while (tmp) {
			if (_fluid_seq_queue_matchevent((&tmp->evt), type, src, dest)) {
				if (prev) {
					prev->next = tmp->next;
					if (tmp == seq->queue0[i][1]) 						seq->queue0[i][1] = prev;

					_fluid_seq_heap_set_free(seq->heap, tmp);
					tmp = prev->next;
				}
				else {
					seq->queue0[i][0] = tmp->next;
					if (tmp == seq->queue0[i][1]) 						seq->queue0[i][1] = NULL;

					_fluid_seq_heap_set_free(seq->heap, tmp);
					tmp = seq->queue0[i][0];
				}
			}
			else {
				prev = tmp;
				tmp = prev->next;
			}
		}
	}

	for (i = 0; i < 255; i++) {
		fluid_evt_entry* tmp = seq->queue1[i][0];
		fluid_evt_entry* prev = NULL;
		while (tmp) {
			if (_fluid_seq_queue_matchevent((&tmp->evt), type, src, dest)) {
				if (prev) {
					prev->next = tmp->next;
					if (tmp == seq->queue1[i][1]) 						seq->queue1[i][1] = prev;

					_fluid_seq_heap_set_free(seq->heap, tmp);
					tmp = prev->next;
				}
				else {
					seq->queue1[i][0] = tmp->next;
					if (tmp == seq->queue1[i][1]) 						seq->queue1[i][1] = NULL;

					_fluid_seq_heap_set_free(seq->heap, tmp);
					tmp = seq->queue1[i][0];
				}
			}
			else {
				prev = tmp;
				tmp = prev->next;
			}
		}
	}

	{
		fluid_evt_entry* tmp = seq->queueLater;
		fluid_evt_entry* prev = NULL;
		while (tmp) {
			if (_fluid_seq_queue_matchevent((&tmp->evt), type, src, dest)) {
				if (prev) {
					prev->next = tmp->next;

					_fluid_seq_heap_set_free(seq->heap, tmp);
					tmp = prev->next;
				}
				else {
					seq->queueLater = tmp->next;

					_fluid_seq_heap_set_free(seq->heap, tmp);
					tmp = seq->queueLater;
				}
			}
			else {
				prev = tmp;
				tmp = prev->next;
			}
		}
	}
}

void _fluid_seq_queue_send_cell_events(fluid_sequencer_t* seq, int cellNb)
{
	fluid_evt_entry* next;
	fluid_evt_entry* tmp;

	tmp = seq->queue0[cellNb][0];
	while (tmp) {
		fluid_sequencer_send_now(seq, &(tmp->evt));

		next = tmp->next;

		_fluid_seq_heap_set_free(seq->heap, tmp);
		tmp = next;
	}
	seq->queue0[cellNb][0] = NULL;
	seq->queue0[cellNb][1] = NULL;
}

void _fluid_seq_queue_slide(fluid_sequencer_t* seq)
{
	short i;
	fluid_evt_entry* next;
	fluid_evt_entry* tmp;
	int count = 0;

	seq->queue0StartTime += 256;

	tmp = seq->queue1[0][0];
	while (tmp) {
		unsigned int delay = tmp->evt.time - seq->queue0StartTime;
		next = tmp->next;
		if (delay > 255) {
			_fluid_seq_queue_insert_queue1(seq, tmp, 1);
		}
		else {
			_fluid_seq_queue_insert_queue0(seq, tmp, delay);
		}
		tmp = next;
		count++;
	}

	for (i = 1; i < 255; i++) {
		seq->queue1[i - 1][0] = seq->queue1[i][0];
		seq->queue1[i - 1][1] = seq->queue1[i][1];
	}
	seq->queue1[254][0] = NULL;
	seq->queue1[254][1] = NULL;

	count = 0;
	tmp = seq->queueLater;
	while (tmp) {
		unsigned int delay = tmp->evt.time - seq->queue0StartTime;

		if (delay > 65535) {
			break;
		}

		next = tmp->next;

		_fluid_seq_queue_insert_queue1(seq, tmp, 254);
		tmp = next;
		count++;
	}

	seq->queueLater = tmp;
}

void _fluid_seq_queue_send_queued_events(fluid_sequencer_t* seq)
{
	unsigned int nowTicks = fluid_sequencer_get_tick(seq);
	short cellNb;

	cellNb = seq->prevCellNb + 1;
	while (cellNb <= (int)(nowTicks - seq->queue0StartTime)) {
		if (cellNb == 256) {
			cellNb = 0;
			_fluid_seq_queue_slide(seq);
		}

		_fluid_seq_queue_send_cell_events(seq, cellNb);

		cellNb++;
	}

	seq->prevCellNb = cellNb - 1;
}

//
//MERGED FILE END: fluid_seq.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: fluid_seqbind.c
//

void fluid_seq_fluidsynth_callback(unsigned int time, fluid_event_t* event, fluid_sequencer_t* seq, void* data);

short fluid_sequencer_register_fluidsynth(fluid_sequencer_t* seq, fluid_synth_t* synth)
{
	return fluid_sequencer_register_client(seq, (char*)"fluidsynth", fluid_seq_fluidsynth_callback, (void *)synth);
}

void fluid_seq_fluidsynth_callback(unsigned int time, fluid_event_t* evt, fluid_sequencer_t* seq, void* data)
{
	fluid_synth_t* synth = (fluid_synth_t *)data;

	switch (fluid_event_get_type(evt)) {
		case FLUID_SEQ_NOTEON:
			fluid_synth_noteon(synth, fluid_event_get_channel(evt), fluid_event_get_key(evt), fluid_event_get_velocity(evt));
			break;

		case FLUID_SEQ_NOTEOFF:
			fluid_synth_noteoff(synth, fluid_event_get_channel(evt), fluid_event_get_key(evt));
			break;

		case FLUID_SEQ_NOTE:
		{
			unsigned int dur;
			fluid_synth_noteon(synth, fluid_event_get_channel(evt), fluid_event_get_key(evt), fluid_event_get_velocity(evt));
			dur = fluid_event_get_duration(evt);
			fluid_event_noteoff(evt, fluid_event_get_channel(evt), fluid_event_get_key(evt));
			fluid_sequencer_send_at(seq, evt, dur, 0);
		}
		break;

		case FLUID_SEQ_ALLSOUNDSOFF:
			break;

		case FLUID_SEQ_ALLNOTESOFF:
			fluid_synth_cc(synth, fluid_event_get_channel(evt), 0x7B, 0);
			break;

		case FLUID_SEQ_BANKSELECT:
			fluid_synth_bank_select(synth, fluid_event_get_channel(evt), fluid_event_get_bank(evt));
			break;

		case FLUID_SEQ_PROGRAMCHANGE:
			fluid_synth_program_change(synth, fluid_event_get_channel(evt), fluid_event_get_program(evt));
			break;

		case FLUID_SEQ_PROGRAMSELECT:
			fluid_synth_program_select(synth, fluid_event_get_channel(evt), fluid_event_get_sfont_id(evt),
				fluid_event_get_bank(evt), fluid_event_get_program(evt));
			break;

		case FLUID_SEQ_ANYCONTROLCHANGE:
			break;

		case FLUID_SEQ_PITCHBEND:
			fluid_synth_pitch_bend(synth, fluid_event_get_channel(evt), fluid_event_get_pitch(evt));
			break;

		case FLUID_SEQ_PITCHWHHELSENS:
			fluid_synth_pitch_wheel_sens(synth, fluid_event_get_channel(evt), fluid_event_get_value(evt));
			break;

		case FLUID_SEQ_CONTROLCHANGE:
			fluid_synth_cc(synth, fluid_event_get_channel(evt), fluid_event_get_control(evt), fluid_event_get_value(evt));
			break;

		case FLUID_SEQ_MODULATION:
		{
			short ctrl = 0x01;		  	fluid_synth_cc(synth, fluid_event_get_channel(evt), ctrl, fluid_event_get_value(evt));
		}
		break;

		case FLUID_SEQ_SUSTAIN:
		{
			short ctrl = 0x40;		  	fluid_synth_cc(synth, fluid_event_get_channel(evt), ctrl, fluid_event_get_value(evt));
		}
		break;

		case FLUID_SEQ_PAN:
		{
			short ctrl = 0x0A;		  	fluid_synth_cc(synth, fluid_event_get_channel(evt), ctrl, fluid_event_get_value(evt));
		}
		break;

		case FLUID_SEQ_VOLUME:
		{
			short ctrl = 0x07;		  	fluid_synth_cc(synth, fluid_event_get_channel(evt), ctrl, fluid_event_get_value(evt));
		}
		break;

		case FLUID_SEQ_REVERBSEND:
		{
			short ctrl = 0x5B;		  	fluid_synth_cc(synth, fluid_event_get_channel(evt), ctrl, fluid_event_get_value(evt));
		}
		break;

		case FLUID_SEQ_CHORUSSEND:
		{
			short ctrl = 0x5D;		  	fluid_synth_cc(synth, fluid_event_get_channel(evt), ctrl, fluid_event_get_value(evt));
		}
		break;

		case FLUID_SEQ_TIMER:
			break;

		default:
			break;
	}
}

//
//MERGED FILE END: fluid_seqbind.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: fluid_sys.c
//

static char fluid_errbuf[512];

static fluid_log_function_t fluid_log_function[LAST_LOG_LEVEL];
static void* fluid_log_user_data[LAST_LOG_LEVEL];
static int fluid_log_initialized = 0;

static char* fluid_libname = (char*)"fluidsynth";

void fluid_sys_config()
{
	fluid_log_config();
	fluid_time_config();
}


#if DEBUG
static unsigned int fluid_debug_flags = 0;

int fluid_debug(int level, char * fmt, ...)
{
	if (fluid_debug_flags & level) {
		fluid_log_function_t fun;
		va_list args;

		va_start(args, fmt);
		vsnprintf(fluid_errbuf, sizeof(fluid_errbuf), fmt, args);
		va_end(args);

		fun = fluid_log_function[FLUID_DBG];
		if (fun != NULL) {
			(*fun)(level, fluid_errbuf, fluid_log_user_data[FLUID_DBG]);
		}
	}
	return 0;
}
#endif

fluid_log_function_t
fluid_set_log_function(int level, fluid_log_function_t fun, void* data)
{
	fluid_log_function_t old = NULL;

	if ((level >= 0) && (level < LAST_LOG_LEVEL)) {
		old = fluid_log_function[level];
		fluid_log_function[level] = fun;
		fluid_log_user_data[level] = data;
	}
	return old;
}

void fluid_default_log_function(int level, char* message, void* data)
{
	FILE* out;

#if defined(WIN32)
	out = stdout;
#else
	out = stderr;
#endif

	if (fluid_log_initialized == 0) {
		fluid_log_config();
	}

	switch (level) {
		case FLUID_PANIC:
			FLUID_FPRINTF(out, "%s: panic: %s\n", fluid_libname, message);
			break;
		case FLUID_ERR:
			FLUID_FPRINTF(out, "%s: error: %s\n", fluid_libname, message);
			break;
		case FLUID_WARN:
			FLUID_FPRINTF(out, "%s: warning: %s\n", fluid_libname, message);
			break;
		case FLUID_INFO:
			FLUID_FPRINTF(out, "%s: %s\n", fluid_libname, message);
			break;
		case FLUID_DBG:
#if DEBUG
			FLUID_FPRINTF(out, "%s: debug: %s\n", fluid_libname, message);
#endif
			break;
		default:
			FLUID_FPRINTF(out, "%s: %s\n", fluid_libname, message);
			break;
	}
	fflush(out);
}

void fluid_log_config(void)
{
	if (fluid_log_initialized == 0) {
		fluid_log_initialized = 1;

		if (fluid_log_function[FLUID_PANIC] == NULL) {
			fluid_set_log_function(FLUID_PANIC, fluid_default_log_function, NULL);
		}

		if (fluid_log_function[FLUID_ERR] == NULL) {
			fluid_set_log_function(FLUID_ERR, fluid_default_log_function, NULL);
		}

		if (fluid_log_function[FLUID_WARN] == NULL) {
			fluid_set_log_function(FLUID_WARN, fluid_default_log_function, NULL);
		}

		if (fluid_log_function[FLUID_INFO] == NULL) {
			fluid_set_log_function(FLUID_INFO, fluid_default_log_function, NULL);
		}

		if (fluid_log_function[FLUID_DBG] == NULL) {
			fluid_set_log_function(FLUID_DBG, fluid_default_log_function, NULL);
		}
	}
}

int fluid_log(int level, const char* fmt, ...)
{
	return FLUID_FAILED;
}

char *fluid_strtok(char **str, char *delim)
{
	char *s, *d, *token;
	char c;

	if (str == NULL || delim == NULL || !*delim)
	{
		FLUID_LOG(FLUID_ERR, "Null pointer");
		return NULL;
	}

	s = *str;
	if (!s) return NULL;

	do
	{
		c = *s;
		if (!c)
		{
			*str = NULL;
			return NULL;
		}

		for (d = delim; *d; d++)
		{
			if (c == *d)
			{
				s++;
				break;
			}
		}
	} while (*d);

	token = s;

	for (s = s + 1; *s; s++)
	{
		c = *s;

		for (d = delim; *d; d++)
		{
			if (c == *d)
			{
				*s = '\0';
				*str = s + 1;
				return token;
			}
		}
	}

	*str = NULL;
	return token;
}

char*
fluid_error()
{
	return fluid_errbuf;
}

int fluid_is_midifile(char* filename)
{
	FILE* fp = fopen(filename, "rb");
	char id[4];

	if (fp == NULL) {
		return 0;
	}
	if (fread((void*)id, 1, 4, fp) != 4) {
		fclose(fp);
		return 0;
	}
	fclose(fp);

	return strncmp(id, "MThd", 4) == 0;
}

int fluid_is_soundfont(char* filename)
{
	FILE* fp = fopen(filename, "rb");
	char id[4];

	if (fp == NULL) {
		return 0;
	}
	if (fread((void*)id, 1, 4, fp) != 4) {
		fclose(fp);
		return 0;
	}
	fclose(fp);

	return strncmp(id, "RIFF", 4) == 0;
}

#if defined(WIN32)

struct _fluid_timer_t
{
	long msec;
	fluid_timer_callback_t callback;
	void* data;
	HANDLE thread;
	DWORD thread_id;
	int cont;
	int auto_destroy;
};

static int fluid_timer_count = 0;
DWORD WINAPI fluid_timer_run(LPVOID data);

fluid_timer_t*
new_fluid_timer(int msec, fluid_timer_callback_t callback, void* data,
int new_thread, int auto_destroy)
{
	fluid_timer_t* timer = FLUID_NEW(fluid_timer_t);
	if (timer == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return NULL;
	}

	timer->cont = 1;
	timer->msec = msec;
	timer->callback = callback;
	timer->data = data;
	timer->thread = 0;
	timer->auto_destroy = auto_destroy;

	if (new_thread) {
		timer->thread = CreateThread(NULL, 0, fluid_timer_run, (LPVOID)timer, 0, &timer->thread_id);
		if (timer->thread == NULL) {
			FLUID_LOG(FLUID_ERR, "Couldn't create timer thread");
			FLUID_FREE(timer);
			return NULL;
		}
		SetThreadPriority(timer->thread, THREAD_PRIORITY_TIME_CRITICAL);
	}
	else {
		fluid_timer_run((LPVOID)timer);
	}
	return timer;
}

DWORD WINAPI
fluid_timer_run(LPVOID data)
{
	int count = 0;
	int cont = 1;
	long start;
	long delay;
	fluid_timer_t* timer;
	timer = (fluid_timer_t*)data;

	if ((timer == NULL) || (timer->callback == NULL)) {
		return 0;
	}

	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

	start = fluid_curtime();

	while (cont) {
		cont = (*timer->callback)(timer->data, fluid_curtime() - start);

		count++;

		delay = (count * timer->msec) - (fluid_curtime() - start);
		if (delay > 0) {
			Sleep(delay);
		}

		cont &= timer->cont;
	}

	FLUID_LOG(FLUID_DBG, "Timer thread finished");

	if (timer->auto_destroy) {
		FLUID_FREE(timer);
	}

	ExitThread(0);
	return 0;
}

int delete_fluid_timer(fluid_timer_t* timer)
{
	timer->cont = 0;
	fluid_timer_join(timer);
	FLUID_FREE(timer);
	return FLUID_OK;
}

int fluid_timer_join(fluid_timer_t* timer)
{
	DWORD wait_result;
	if (timer->thread == 0) {
		return FLUID_OK;
	}
	wait_result = WaitForSingleObject(timer->thread, INFINITE);
	return (wait_result == WAIT_OBJECT_0) ? FLUID_OK : FLUID_FAILED;
}

double rdtsc(void);
double fluid_estimate_cpu_frequency(void);

static double fluid_cpu_frequency = -1.0;

void fluid_time_config(void)
{
	if (fluid_cpu_frequency < 0.0) {
		fluid_cpu_frequency = fluid_estimate_cpu_frequency() / 1000000.0;
	}
}

double fluid_utime(void)
{
	return (rdtsc() / fluid_cpu_frequency);
}

double rdtsc(void)
{
	LARGE_INTEGER t;
	QueryPerformanceCounter(&t);
	return (double)t.QuadPart;
}

double fluid_estimate_cpu_frequency(void)
{
#if 0
	LONGLONG start, stop, ticks;
	unsigned int before, after, delta;
	double freq;

	start = rdtsc();
	stop = start;
	before = fluid_curtime();
	after = before;

	while (1) {
		if (after - before > 1000) {
			break;
		}
		after = fluid_curtime();
		stop = rdtsc();
	}

	delta = after - before;
	ticks = stop - start;

	freq = 1000 * ticks / delta;

	return freq;

#else
	unsigned int before, after;
	LARGE_INTEGER start, stop;

	before = fluid_curtime();
	QueryPerformanceCounter(&start);

	Sleep(10);

	after = fluid_curtime();
	QueryPerformanceCounter(&stop);

	return (double)10 * (stop.QuadPart - start.QuadPart) / (after - before);
#endif
}

#elif defined(MACOS9)

struct _fluid_timer_t
{
	TMTask myTmTask;
	long msec;
	unsigned int start;
	unsigned int count;
	int isInstalled;
	fluid_timer_callback_t callback;
	void* data;
	int auto_destroy;
};

static TimerUPP	myTimerUPP;

void _timerCallback(fluid_timer_t *timer)
{
	int cont;
	cont = (*timer->callback)(timer->data, fluid_curtime() - timer->start);
	if (cont) {
		PrimeTime((QElemPtr)timer, timer->msec);
	}
	else {
		timer->isInstalled = 0;
	}
	timer->count++;
}

fluid_timer_t*
new_fluid_timer(int msec, fluid_timer_callback_t callback, void* data,
int new_thread, int auto_destroy)
{
	fluid_timer_t* timer = FLUID_NEW(fluid_timer_t);
	if (timer == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return NULL;
	}

	if (!myTimerUPP)
		myTimerUPP = NewTimerProc(_timerCallback);

	timer->myTmTask.tmAddr = myTimerUPP;
	timer->myTmTask.qLink = NULL;
	timer->myTmTask.qType = 0;
	timer->myTmTask.tmCount = 0L;
	timer->myTmTask.tmWakeUp = 0L;
	timer->myTmTask.tmReserved = 0L;

	timer->callback = callback;

	timer->msec = msec;
	timer->data = data;
	timer->start = fluid_curtime();
	timer->isInstalled = 1;
	timer->count = 0;
	timer->auto_destroy = auto_destroy;

	InsXTime((QElemPtr)timer);
	PrimeTime((QElemPtr)timer, msec);

	return timer;
}

int delete_fluid_timer(fluid_timer_t* timer)
{
	if (timer->isInstalled) {
		RmvTime((QElemPtr)timer);
	}
	FLUID_FREE(timer);
	return FLUID_OK;
}

int fluid_timer_join(fluid_timer_t* timer)
{
	if (timer->isInstalled) {
		int count = timer->count;

		while (count == timer->count) {}
	}
	return FLUID_OK;
}

#define kTwoPower32 (4294967296.0)

void fluid_time_config(void)
{
}

unsigned int fluid_curtime()
{
	UnsignedWide    uS;
	double mSf;
	unsigned int ms;

	Microseconds(&uS);

	mSf = ((((double)uS.hi) * kTwoPower32) + uS.lo) / 1000.0f;

	ms = mSf;

	return (ms);
}

#elif defined(__OS2__)

struct _fluid_timer_t
{
	long msec;
	fluid_timer_callback_t callback;
	void* data;
	int thread_id;
	int cont;
	int auto_destroy;
};

static int fluid_timer_count = 0;
void fluid_timer_run(void *data);

fluid_timer_t*
new_fluid_timer(int msec, fluid_timer_callback_t callback, void* data,
int new_thread, int auto_destroy)
{
	fluid_timer_t* timer = FLUID_NEW(fluid_timer_t);
	if (timer == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return NULL;
	}

	timer->cont = 1;
	timer->msec = msec;
	timer->callback = callback;
	timer->data = data;
	timer->thread_id = -1;
	timer->auto_destroy = auto_destroy;

	if (new_thread) {
		timer->thread_id = _beginthread(fluid_timer_run, NULL, 256 * 1024, (void *)timer);
		if (timer->thread_id == -1) {
			FLUID_LOG(FLUID_ERR, "Couldn't create timer thread");
			FLUID_FREE(timer);
			return NULL;
		}
		DosSetPriority(PRTYS_THREAD, PRTYC_TIMECRITICAL, PRTYD_MAXIMUM, timer->thread_id);
	}
	else {
		fluid_timer_run((void *)timer);
	}
	return timer;
}

void fluid_timer_run(void *data)
{
	int count = 0;
	int cont = 1;
	long start;
	long delay;
	fluid_timer_t* timer;
	timer = (fluid_timer_t*)data;

	if ((timer == NULL) || (timer->callback == NULL)) {
		return;
	}

	DosSetPriority(PRTYS_THREAD, PRTYC_REGULAR, PRTYD_MAXIMUM, 0);

	start = fluid_curtime();

	while (cont) {
		cont = (*timer->callback)(timer->data, fluid_curtime() - start);

		count++;

		delay = (count * timer->msec) - (fluid_curtime() - start);
		if (delay > 0) {
			DosSleep(delay);
		}

		cont &= timer->cont;
	}

	FLUID_LOG(FLUID_DBG, "Timer thread finished");

	if (timer->auto_destroy) {
		FLUID_FREE(timer);
	}

	return;
}

int delete_fluid_timer(fluid_timer_t* timer)
{
	timer->cont = 0;
	fluid_timer_join(timer);
	FLUID_FREE(timer);
	return FLUID_OK;
}

int fluid_timer_join(fluid_timer_t* timer)
{
	ULONG wait_result;
	if (timer->thread_id == -1) {
		return FLUID_OK;
	}
	wait_result = DosWaitThread(&timer->thread_id, DCWW_WAIT);
	return (wait_result == 0) ? FLUID_OK : FLUID_FAILED;
}

double rdtsc(void);
double fluid_estimate_cpu_frequency(void);

static double fluid_cpu_frequency = -1.0;

void fluid_time_config(void)
{
	if (fluid_cpu_frequency < 0.0) {
		fluid_cpu_frequency = fluid_estimate_cpu_frequency() / 1000000.0;
	}
}

unsigned int fluid_curtime(void)
{
	ULONG ulMS;
	DosQuerySysInfo(QSV_MS_COUNT, QSV_MS_COUNT, &ulMS, sizeof(ULONG));
	return ulMS;
}

double fluid_utime(void)
{
	return (rdtsc() / fluid_cpu_frequency);
}

#define Q2ULL( q ) (*(unsigned long long *)&q)

double rdtsc(void)
{
	QWORD t;
	DosTmrQueryTime(&t);
	return (double)Q2ULL(t);
}

double fluid_estimate_cpu_frequency(void)
{
	unsigned int before, after;
	QWORD start, stop;

	before = fluid_curtime();
	DosTmrQueryTime(&start);

	DosSleep(10);

	after = fluid_curtime();
	DosTmrQueryTime(&stop);

	return (double)10 * (Q2ULL(stop) - Q2ULL(start)) / (after - before);
}

#else

struct _fluid_timer_t
{
	long msec;
	fluid_timer_callback_t callback;
	void* data;
	pthread_t thread;
	int cont;
	int auto_destroy;
};

void*
fluid_timer_start(void *data)
{
	int count = 0;
	int cont = 1;
	long start;
	long delay;
	fluid_timer_t* timer;
	timer = (fluid_timer_t*)data;

	start = fluid_curtime();

	while (cont) {
		cont = (*timer->callback)(timer->data, fluid_curtime() - start);

		count++;

		delay = (count * timer->msec) - (fluid_curtime() - start);
		if (delay > 0) {
			usleep(delay * 1000);
		}

		cont &= timer->cont;
	}

	FLUID_LOG(FLUID_DBG, "Timer thread finished");
	if (timer->thread != 0) {
		pthread_exit(NULL);
	}

	if (timer->auto_destroy) {
		FLUID_FREE(timer);
	}

	return NULL;
}

fluid_timer_t*
new_fluid_timer(int msec, fluid_timer_callback_t callback, void* data,
int new_thread, int auto_destroy)
{
	pthread_attr_t *attr = NULL;
	pthread_attr_t rt_attr;
	struct sched_param priority;
	int err;

	fluid_timer_t* timer = FLUID_NEW(fluid_timer_t);
	if (timer == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return NULL;
	}
	timer->msec = msec;
	timer->callback = callback;
	timer->data = data;
	timer->cont = 1;
	timer->thread = 0;
	timer->auto_destroy = auto_destroy;

	err = pthread_attr_init(&rt_attr);
	if (err == 0) {
		err = pthread_attr_setschedpolicy(&rt_attr, SCHED_FIFO);
		if (err == 0) {
			priority.sched_priority = 10;
			err = pthread_attr_setschedparam(&rt_attr, &priority);
			if (err == 0) {
				attr = &rt_attr;
			}
		}
	}

	if (new_thread) {
		err = pthread_create(&timer->thread, attr, fluid_timer_start, (void*)timer);
		if (err == 0) {
			FLUID_LOG(FLUID_DBG, "The timer thread was created with real-time priority");
		}
		else {
			err = pthread_create(&timer->thread, NULL, fluid_timer_start, (void*)timer);
			if (err != 0) {
				FLUID_LOG(FLUID_ERR, "Failed to create the timer thread");
				FLUID_FREE(timer);
				return NULL;
			}
			else {
				FLUID_LOG(FLUID_DBG, "The timer thread does not have real-time priority");
			}
		}
	}
	else {
		fluid_timer_start((void*)timer);
	}
	return timer;
}

int delete_fluid_timer(fluid_timer_t* timer)
{
	timer->cont = 0;
	fluid_timer_join(timer);
	FLUID_LOG(FLUID_DBG, "Joined player thread");
	FLUID_FREE(timer);
	return FLUID_OK;
}

int fluid_timer_join(fluid_timer_t* timer)
{
	int err = 0;

	if (timer->thread != 0) {
		err = pthread_join(timer->thread, NULL);
	}
	FLUID_LOG(FLUID_DBG, "Joined player thread");
	return (err == 0) ? FLUID_OK : FLUID_FAILED;
}

static double fluid_cpu_frequency = -1.0;

double rdtsc(void);
double fluid_estimate_cpu_frequency(void);

void fluid_time_config(void)
{
	if (fluid_cpu_frequency < 0.0) {
		fluid_cpu_frequency = fluid_estimate_cpu_frequency() / 1000000.0;
		if (fluid_cpu_frequency == 0.0) fluid_cpu_frequency = 1.0;
	}
}

unsigned int fluid_curtime()
{
	struct timeval now;
	gettimeofday(&now, NULL);
	return now.tv_sec * 1000 + now.tv_usec / 1000;
}

double fluid_utime(void)
{
	return (rdtsc() / fluid_cpu_frequency);
}

#if !defined(__i386__)

double rdtsc(void)
{
	return 0.0;
}

double fluid_estimate_cpu_frequency(void)
{
	return 1.0;
}

#else

double rdtsc(void)
{
	unsigned int a, b;

	__asm__("rdtsc" : "=a" (a), "=d" (b));
	return (double)b * (double)0x10000 * (double)0x10000 + a;
}

double fluid_estimate_cpu_frequency(void)
{
	double start, stop;
	unsigned int a0, b0, a1, b1;
	unsigned int before, after;

	before = fluid_curtime();
	__asm__("rdtsc" : "=a" (a0), "=d" (b0));

	usleep(10 * 1000);

	after = fluid_curtime();
	__asm__("rdtsc" : "=a" (a1), "=d" (b1));

	start = (double)b0 * (double)0x10000 * (double)0x10000 + a0;
	stop = (double)b1 * (double)0x10000 * (double)0x10000 + a1;

	return 10 * (stop - start) / (after - before);
}
#endif

#ifdef FPE_CHECK

#define _FPU_STATUS_IE    0x001
#define _FPU_STATUS_DE    0x002
#define _FPU_STATUS_ZE    0x004
#define _FPU_STATUS_OE    0x008
#define _FPU_STATUS_UE    0x010
#define _FPU_STATUS_PE    0x020
#define _FPU_STATUS_SF    0x040
#define _FPU_STATUS_ES    0x080

#define _FPU_GET_SW(sw) __asm__ ("fnstsw %0" : "=m" (*&sw))

#define _FPU_CLR_SW() __asm__ ("fnclex" : : )

unsigned int fluid_check_fpe_i386(char* explanation)
{
	unsigned int s;

	_FPU_GET_SW(s);
	_FPU_CLR_SW();

	s &= _FPU_STATUS_IE | _FPU_STATUS_DE | _FPU_STATUS_ZE | _FPU_STATUS_OE | _FPU_STATUS_UE;

	if (s)
	{
		FLUID_LOG(FLUID_WARN, "FPE exception (before or in %s): %s%s%s%s%s", explanation,
			(s & _FPU_STATUS_IE) ? "Invalid operation " : "",
			(s & _FPU_STATUS_DE) ? "Denormal number " : "",
			(s & _FPU_STATUS_ZE) ? "Zero divide " : "",
			(s & _FPU_STATUS_OE) ? "Overflow " : "",
			(s & _FPU_STATUS_UE) ? "Underflow " : "");
	}

	return s;
}

void fluid_clear_fpe_i386(void)
{
	_FPU_CLR_SW();
}

#endif

#endif

#if WITH_PROFILING

fluid_profile_data_t fluid_profile_data[] =
{
	{ FLUID_PROF_WRITE_S16, "fluid_synth_write_s16           ", 1e10, 0.0, 0.0, 0 },
	{ FLUID_PROF_ONE_BLOCK, "fluid_synth_one_block           ", 1e10, 0.0, 0.0, 0 },
	{ FLUID_PROF_ONE_BLOCK_CLEAR, "fluid_synth_one_block:clear     ", 1e10, 0.0, 0.0, 0 },
	{ FLUID_PROF_ONE_BLOCK_VOICE, "fluid_synth_one_block:one voice ", 1e10, 0.0, 0.0, 0 },
	{ FLUID_PROF_ONE_BLOCK_VOICES, "fluid_synth_one_block:all voices", 1e10, 0.0, 0.0, 0 },
	{ FLUID_PROF_ONE_BLOCK_REVERB, "fluid_synth_one_block:reverb    ", 1e10, 0.0, 0.0, 0 },
	{ FLUID_PROF_ONE_BLOCK_CHORUS, "fluid_synth_one_block:chorus    ", 1e10, 0.0, 0.0, 0 },
	{ FLUID_PROF_VOICE_NOTE, "fluid_voice:note                ", 1e10, 0.0, 0.0, 0 },
	{ FLUID_PROF_VOICE_RELEASE, "fluid_voice:release             ", 1e10, 0.0, 0.0, 0 },
	{ FLUID_PROF_LAST, "last", 1e100, 0.0, 0.0, 0 }
};

void fluid_profiling_print(void)
{
	int i;

	printf("fluid_profiling_print\n");

	FLUID_LOG(FLUID_INFO, "Estimated CPU frequency: %.0f MHz", fluid_cpu_frequency);
	FLUID_LOG(FLUID_INFO, "Estimated times: min/avg/max (micro seconds)");

	for (i = 0; i < FLUID_PROF_LAST; i++) {
		if (fluid_profile_data[i].count > 0) {
			FLUID_LOG(FLUID_INFO, "%s: %.3f/%.3f/%.3f",
				fluid_profile_data[i].description,
				fluid_profile_data[i].min,
				fluid_profile_data[i].total / fluid_profile_data[i].count,
				fluid_profile_data[i].max);
		}
		else {
			FLUID_LOG(FLUID_DBG, "%s: no profiling available", fluid_profile_data[i].description);
		}
	}
}

#endif

#if defined(MACOS9)

fluid_thread_t* new_fluid_thread(fluid_thread_func_t func, void* data, int detach) { return NULL; }
int delete_fluid_thread(fluid_thread_t* thread) { return 0; }
int fluid_thread_join(fluid_thread_t* thread) { return 0; }

#elif defined(WIN32)

struct _fluid_thread_t {
	HANDLE thread;
	DWORD thread_id;
	fluid_thread_func_t func;
	void* data;
	int detached;
};

static DWORD WINAPI fluid_thread_start(LPVOID data)
{
	fluid_thread_t* thread = (fluid_thread_t*)data;

	thread->func(thread->data);

	if (thread->detached) {
		FLUID_FREE(thread);
	}

	return 0;
}

fluid_thread_t* new_fluid_thread(fluid_thread_func_t func, void* data, int detach)
{
	fluid_thread_t* thread;

	if (func == NULL) {
		FLUID_LOG(FLUID_ERR, "Invalid thread function");
		return NULL;
	}

	thread = FLUID_NEW(fluid_thread_t);
	if (thread == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return NULL;
	}

	thread->data = data;
	thread->func = func;
	thread->detached = detach;

	thread->thread = CreateThread(NULL, 0, fluid_thread_start, (LPVOID)thread,
		0, &thread->thread_id);
	if (thread->thread == NULL) {
		FLUID_LOG(FLUID_ERR, "Couldn't create the thread");
		FLUID_FREE(thread);
		return NULL;
	}

	return thread;
}

int delete_fluid_thread(fluid_thread_t* thread)
{
	FLUID_FREE(thread);
	return FLUID_OK;
}

int fluid_thread_join(fluid_thread_t* thread)
{
	DWORD wait_result;
	if (thread->thread == 0) {
		return FLUID_OK;
	}
	wait_result = WaitForSingleObject(thread->thread, INFINITE);
	return (wait_result == WAIT_OBJECT_0) ? FLUID_OK : FLUID_FAILED;
}

#elif defined(__OS2__)

struct _fluid_thread_t {
	int thread_id;
	fluid_thread_func_t func;
	void* data;
	int detached;
};

static void fluid_thread_start(void *data)
{
	fluid_thread_t* thread = (fluid_thread_t*)data;

	thread->func(thread->data);

	if (thread->detached) {
		FLUID_FREE(thread);
	}

	return 0;
}

fluid_thread_t* new_fluid_thread(fluid_thread_func_t func, void* data, int detach)
{
	fluid_thread_t* thread;

	if (func == NULL) {
		FLUID_LOG(FLUID_ERR, "Invalid thread function");
		return NULL;
	}

	thread = FLUID_NEW(fluid_thread_t);
	if (thread == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return NULL;
	}

	thread->data = data;
	thread->func = func;
	thread->detached = detach;

	thread->thread_id = _beginthread(fluid_thread_start, NULL, 256 * 1024, (void *)thread);
	if (thread->thread_id == -1) {
		FLUID_LOG(FLUID_ERR, "Couldn't create the thread");
		FLUID_FREE(thread);
		return NULL;
	}

	return thread;
}

int delete_fluid_thread(fluid_thread_t* thread)
{
	FLUID_FREE(thread);
	return FLUID_OK;
}

int fluid_thread_join(fluid_thread_t* thread)
{
	ULONG wait_result;
	if (thread->thread_id == -1) {
		return FLUID_OK;
	}
	wait_result = DosWaitThread(&thread->thread_id, DCWW_WAIT);
	return (wait_result == 0) ? FLUID_OK : FLUID_FAILED;
}

#else

struct _fluid_thread_t {
	pthread_t pthread;
	fluid_thread_func_t func;
	void* data;
	int detached;
};

static void* fluid_thread_start(void *data)
{
	fluid_thread_t* thread = (fluid_thread_t*)data;

	thread->func(thread->data);

	if (thread->detached) {
		FLUID_FREE(thread);
	}

	return NULL;
}

fluid_thread_t* new_fluid_thread(fluid_thread_func_t func, void* data, int detach)
{
	fluid_thread_t* thread;
	pthread_attr_t attr;

	if (func == NULL) {
		FLUID_LOG(FLUID_ERR, "Invalid thread function");
		return NULL;
	}

	thread = FLUID_NEW(fluid_thread_t);
	if (thread == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return NULL;
	}

	thread->data = data;
	thread->func = func;
	thread->detached = detach;

	pthread_attr_init(&attr);

	if (detach) {
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	}

	if (pthread_create(&thread->pthread, &attr, fluid_thread_start, thread)) {
		FLUID_LOG(FLUID_ERR, "Failed to create the thread");
		FLUID_FREE(thread);
		return NULL;
	}

	return thread;
}

int delete_fluid_thread(fluid_thread_t* thread)
{
	FLUID_FREE(thread);
	return FLUID_OK;
}

int fluid_thread_join(fluid_thread_t* thread)
{
	int err = 0;

	if (thread->pthread != 0) {
		err = pthread_join(thread->pthread, NULL);
	}
	return (err == 0) ? FLUID_OK : FLUID_FAILED;
}

#endif

#if defined(MACINTOSH)

#elif defined(WIN32)

#if 0
typedef unsigned int socklen_t;

#define fluid_socket_read(_S,_B,_L) recv(_S,_B,_L,0)
#define fluid_socket_write(_S,_B,_L) send(_S,_B,_L,0)

void fluid_socket_close(fluid_socket_t sock)
{
	int r;
	char buf[1024];
	if (sock != INVALID_SOCKET) {
		shutdown(sock, 0x02);
		while (1) {
			r = recv(sock, buf, 1024, 0);
			if ((r == 0) || (r == SOCKET_ERROR)) {
				break;
			}
		}
		closesocket(sock);
	}
}
#endif

#else
#define fluid_socket_read(_S,_B,_L) read(_S,_B,_L)
#define fluid_socket_write(_S,_B,_L) write(_S,_B,_L)
#define SOCKET_ERROR -1

void fluid_socket_close(fluid_socket_t sock)
{
	if (sock != INVALID_SOCKET) {
		close(sock);
	}
}

#endif

#if !defined(MACINTOSH) && !defined(WIN32)

fluid_istream_t fluid_socket_get_istream(fluid_socket_t sock)
{
	return sock;
}

fluid_ostream_t fluid_socket_get_ostream(fluid_socket_t sock)
{
	return sock;
}

struct _fluid_server_socket_t {
	fluid_socket_t socket;
	fluid_thread_t* thread;
	int cont;
	fluid_server_func_t func;
	void* data;
};

static void fluid_server_socket_run(void* data)
{
	fluid_server_socket_t* server_socket = (fluid_server_socket_t*)data;
	fluid_socket_t client_socket;
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	FLUID_LOG(FLUID_DBG, "Server listening for connections");

	while (server_socket->cont) {
		client_socket = accept(server_socket->socket, (struct sockaddr*) &addr, &addrlen);

		FLUID_LOG(FLUID_DBG, "New client connection");

		if (client_socket == INVALID_SOCKET) {
			if (server_socket->cont) {
				FLUID_LOG(FLUID_ERR, "Failed to accept connection");
			}
			server_socket->cont = 0;
			return;
		}
		else {
			int r;
			r = (*server_socket->func)(server_socket->data, client_socket, inet_ntoa(addr.sin_addr));
			if (r != 0) {
				fluid_socket_close(client_socket);
			}
		}
	}

	FLUID_LOG(FLUID_DBG, "Server closing");
}

fluid_server_socket_t*
new_fluid_server_socket(int port, fluid_server_func_t func, void* data)
{
	fluid_server_socket_t* server_socket;
	struct sockaddr_in addr;
	fluid_socket_t sock;

	if (func == NULL) {
		FLUID_LOG(FLUID_ERR, "Invalid callback function");
		return NULL;
	}

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) {
		FLUID_LOG(FLUID_ERR, "Failed to create server socket");
		return NULL;
	}

	FLUID_MEMSET((char *)&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	if (bind(sock, (const struct sockaddr *) &addr, sizeof(struct sockaddr_in)) == SOCKET_ERROR) {
		FLUID_LOG(FLUID_ERR, "Failed to bind server socket");
		fluid_socket_close(sock);
		return NULL;
	}

	if (listen(sock, 10) == SOCKET_ERROR) {
		FLUID_LOG(FLUID_ERR, "Failed listen on server socket");
		fluid_socket_close(sock);
		return NULL;
	}

	server_socket = FLUID_NEW(fluid_server_socket_t);
	if (server_socket == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		fluid_socket_close(sock);
		return NULL;
	}

	server_socket->socket = sock;
	server_socket->func = func;
	server_socket->data = data;
	server_socket->cont = 1;

	server_socket->thread = new_fluid_thread(fluid_server_socket_run, server_socket, 0);
	if (server_socket->thread == NULL) {
		FLUID_FREE(server_socket);
		fluid_socket_close(sock);
		return NULL;
	}

	return server_socket;
}

int delete_fluid_server_socket(fluid_server_socket_t* server_socket)
{
	server_socket->cont = 0;
	if (server_socket->socket != INVALID_SOCKET) {
		fluid_socket_close(server_socket->socket);
	}
	if (server_socket->thread) {
		delete_fluid_thread(server_socket->thread);
	}
	FLUID_FREE(server_socket);
	return FLUID_OK;
}

int fluid_server_socket_join(fluid_server_socket_t* server_socket)
{
	return fluid_thread_join(server_socket->thread);
}

#endif

//
//MERGED FILE END: fluid_sys.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: fluid_tuning.c
//

fluid_tuning_t* new_fluid_tuning(char* name, int bank, int prog)
{
	fluid_tuning_t* tuning;
	int i;

	tuning = FLUID_NEW(fluid_tuning_t);
	if (tuning == NULL) {
		FLUID_LOG(FLUID_PANIC, "Out of memory");
		return NULL;
	}

	tuning->name = NULL;

	if (name != NULL) {
		tuning->name = FLUID_STRDUP(name);
	}

	tuning->bank = bank;
	tuning->prog = prog;

	for (i = 0; i < 128; i++) {
		tuning->pitch[i] = i * 100.0;
	}

	return tuning;
}

void delete_fluid_tuning(fluid_tuning_t* tuning)
{
	if (tuning == NULL) {
		return;
	}
	if (tuning->name != NULL) {
		FLUID_FREE(tuning->name);
	}
	FLUID_FREE(tuning);
}

void fluid_tuning_set_name(fluid_tuning_t* tuning, char* name)
{
	if (tuning->name != NULL) {
		FLUID_FREE(tuning->name);
		tuning->name = NULL;
	}
	if (name != NULL) {
		tuning->name = FLUID_STRDUP(name);
	}
}

char* fluid_tuning_get_name(fluid_tuning_t* tuning)
{
	return tuning->name;
}

void fluid_tuning_set_key(fluid_tuning_t* tuning, int key, double pitch)
{
	tuning->pitch[key] = pitch;
}

void fluid_tuning_set_octave(fluid_tuning_t* tuning, double* pitch_deriv)
{
	int i;

	for (i = 0; i < 128; i++) {
		tuning->pitch[i] = i * 100.0 + pitch_deriv[i % 12];
	}
}

void fluid_tuning_set_all(fluid_tuning_t* tuning, double* pitch)
{
	int i;

	for (i = 0; i < 128; i++) {
		tuning->pitch[i] = pitch[i];
	}
}

void fluid_tuning_set_pitch(fluid_tuning_t* tuning, int key, double pitch)
{
	if ((key >= 0) && (key < 128)) {
		tuning->pitch[key] = pitch;
	}
}

//
//MERGED FILE END: fluid_tuning.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: fluid_voice.c
//

#define FLUID_MAX_AUDIBLE_FILTER_FC 19000.0f
#define FLUID_MIN_AUDIBLE_FILTER_Q 1.2f

#define FLUID_NOISE_FLOOR 0.00003

#define FLUID_MIN_LOOP_SIZE 2
#define FLUID_MIN_LOOP_PAD 0

#define FLUID_MIN_VOLENVRELEASE -7200.0f

static INLINE void fluid_voice_effects(fluid_voice_t *voice, int count,
	fluid_real_t* dsp_left_buf,
	fluid_real_t* dsp_right_buf,
	fluid_real_t* dsp_reverb_buf,
	fluid_real_t* dsp_chorus_buf);

fluid_voice_t*
new_fluid_voice(fluid_real_t output_rate)
{
	fluid_voice_t* voice;
	voice = FLUID_NEW(fluid_voice_t);
	if (voice == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return NULL;
	}
	voice->status = FLUID_VOICE_CLEAN;
	voice->chan = NO_CHANNEL;
	voice->key = 0;
	voice->vel = 0;
	voice->channel = NULL;
	voice->sample = NULL;
	voice->output_rate = output_rate;

	voice->volenv_data[FLUID_VOICE_ENVSUSTAIN].count = 0xffffffff;
	voice->volenv_data[FLUID_VOICE_ENVSUSTAIN].coeff = 1.0f;
	voice->volenv_data[FLUID_VOICE_ENVSUSTAIN].incr = 0.0f;
	voice->volenv_data[FLUID_VOICE_ENVSUSTAIN].min = -1.0f;
	voice->volenv_data[FLUID_VOICE_ENVSUSTAIN].max = 2.0f;

	voice->volenv_data[FLUID_VOICE_ENVFINISHED].count = 0xffffffff;
	voice->volenv_data[FLUID_VOICE_ENVFINISHED].coeff = 0.0f;
	voice->volenv_data[FLUID_VOICE_ENVFINISHED].incr = 0.0f;
	voice->volenv_data[FLUID_VOICE_ENVFINISHED].min = -1.0f;
	voice->volenv_data[FLUID_VOICE_ENVFINISHED].max = 1.0f;

	voice->modenv_data[FLUID_VOICE_ENVSUSTAIN].count = 0xffffffff;
	voice->modenv_data[FLUID_VOICE_ENVSUSTAIN].coeff = 1.0f;
	voice->modenv_data[FLUID_VOICE_ENVSUSTAIN].incr = 0.0f;
	voice->modenv_data[FLUID_VOICE_ENVSUSTAIN].min = -1.0f;
	voice->modenv_data[FLUID_VOICE_ENVSUSTAIN].max = 2.0f;

	voice->modenv_data[FLUID_VOICE_ENVFINISHED].count = 0xffffffff;
	voice->modenv_data[FLUID_VOICE_ENVFINISHED].coeff = 0.0f;
	voice->modenv_data[FLUID_VOICE_ENVFINISHED].incr = 0.0f;
	voice->modenv_data[FLUID_VOICE_ENVFINISHED].min = -1.0f;
	voice->modenv_data[FLUID_VOICE_ENVFINISHED].max = 1.0f;

	return voice;
}

int delete_fluid_voice(fluid_voice_t* voice)
{
	if (voice == NULL) {
		return FLUID_OK;
	}
	FLUID_FREE(voice);
	return FLUID_OK;
}

int fluid_voice_init(fluid_voice_t* voice, fluid_sample_t* sample,
fluid_channel_t* channel, int key, int vel, unsigned int id,
unsigned int start_time, fluid_real_t gain)
{
	voice->id = id;
	voice->chan = fluid_channel_get_num(channel);
	voice->key = (unsigned char)key;
	voice->vel = (unsigned char)vel;
	voice->channel = channel;
	voice->mod_count = 0;
	voice->sample = sample;
	voice->start_time = start_time;
	voice->ticks = 0;
	voice->debug = 0;
	voice->has_looped = 0;
	voice->last_fres = -1;
	voice->filter_startup = 1;
	voice->interp_method = fluid_channel_get_interp_method(voice->channel);

	voice->volenv_count = 0;
	voice->volenv_section = 0;
	voice->volenv_val = 0.0f;
	voice->amp = 0.0f;

	voice->modenv_count = 0;
	voice->modenv_section = 0;
	voice->modenv_val = 0.0f;

	voice->modlfo_val = 0.0;

	voice->viblfo_val = 0.0f;

	voice->hist1 = 0;
	voice->hist2 = 0;

	fluid_gen_init(&voice->gen[0], channel);

	voice->synth_gain = gain;

	if (voice->synth_gain < 0.0000001){
		voice->synth_gain = 0.0000001;
	}

	voice->amplitude_that_reaches_noise_floor_nonloop = FLUID_NOISE_FLOOR / voice->synth_gain;
	voice->amplitude_that_reaches_noise_floor_loop = FLUID_NOISE_FLOOR / voice->synth_gain;

	fluid_sample_incr_ref(voice->sample);

	return FLUID_OK;
}

void fluid_voice_gen_set(fluid_voice_t* voice, int i, float val)
{
	voice->gen[i].val = val;
	voice->gen[i].flags = GEN_SET;
}

void fluid_voice_gen_incr(fluid_voice_t* voice, int i, float val)
{
	voice->gen[i].val += val;
	voice->gen[i].flags = GEN_SET;
}

float fluid_voice_gen_get(fluid_voice_t* voice, int gen)
{
	return voice->gen[gen].val;
}

fluid_real_t fluid_voice_gen_value(fluid_voice_t* voice, int num)
{
	if (voice->gen[num].flags == GEN_ABS_NRPN) {
		return (fluid_real_t)voice->gen[num].nrpn;
	}
	else {
		return (fluid_real_t)(voice->gen[num].val + voice->gen[num].mod + voice->gen[num].nrpn);
	}
}

int fluid_voice_write(fluid_voice_t* voice,
fluid_real_t* dsp_left_buf, fluid_real_t* dsp_right_buf,
fluid_real_t* dsp_reverb_buf, fluid_real_t* dsp_chorus_buf)
{
	fluid_real_t fres;
	fluid_real_t target_amp;
	int count;

	fluid_real_t dsp_buf[FLUID_BUFSIZE];
	fluid_env_data_t* env_data;
	fluid_real_t x;

	if (!_PLAYING(voice)) return FLUID_OK;

	if (voice->sample == NULL)
	{
		fluid_voice_off(voice);
		return FLUID_OK;
	}

	fluid_check_fpe("voice_write startup");

	fluid_voice_check_sample_sanity(voice);

	env_data = &voice->volenv_data[voice->volenv_section];

	while (voice->volenv_count >= env_data->count)
	{
		if (env_data && voice->volenv_section == FLUID_VOICE_ENVDECAY)
			voice->volenv_val = env_data->min * env_data->coeff;

		env_data = &voice->volenv_data[++voice->volenv_section];
		voice->volenv_count = 0;
	}

	x = env_data->coeff * voice->volenv_val + env_data->incr;
	if (x < env_data->min)
	{
		x = env_data->min;
		voice->volenv_section++;
		voice->volenv_count = 0;
	}
	else if (x > env_data->max)
	{
		x = env_data->max;
		voice->volenv_section++;
		voice->volenv_count = 0;
	}

	voice->volenv_val = x;
	voice->volenv_count++;

	if (voice->volenv_section == FLUID_VOICE_ENVFINISHED)
	{
		fluid_profile(FLUID_PROF_VOICE_RELEASE, voice->ref);
		fluid_voice_off(voice);
		return FLUID_OK;
	}

	fluid_check_fpe("voice_write vol env");

	env_data = &voice->modenv_data[voice->modenv_section];

	while (voice->modenv_count >= env_data->count)
	{
		env_data = &voice->modenv_data[++voice->modenv_section];
		voice->modenv_count = 0;
	}

	x = env_data->coeff * voice->modenv_val + env_data->incr;

	if (x < env_data->min)
	{
		x = env_data->min;
		voice->modenv_section++;
		voice->modenv_count = 0;
	}
	else if (x > env_data->max)
	{
		x = env_data->max;
		voice->modenv_section++;
		voice->modenv_count = 0;
	}

	voice->modenv_val = x;
	voice->modenv_count++;
	fluid_check_fpe("voice_write mod env");

	if (voice->ticks >= voice->modlfo_delay)
	{
		voice->modlfo_val += voice->modlfo_incr;

		if (voice->modlfo_val > 1.0)
		{
			voice->modlfo_incr = -voice->modlfo_incr;
			voice->modlfo_val = (fluid_real_t) 2.0 - voice->modlfo_val;
		}
		else if (voice->modlfo_val < -1.0)
		{
			voice->modlfo_incr = -voice->modlfo_incr;
			voice->modlfo_val = (fluid_real_t)-2.0 - voice->modlfo_val;
		}
	}

	fluid_check_fpe("voice_write mod LFO");

	if (voice->ticks >= voice->viblfo_delay)
	{
		voice->viblfo_val += voice->viblfo_incr;

		if (voice->viblfo_val > (fluid_real_t) 1.0)
		{
			voice->viblfo_incr = -voice->viblfo_incr;
			voice->viblfo_val = (fluid_real_t) 2.0 - voice->viblfo_val;
		}
		else if (voice->viblfo_val < -1.0)
		{
			voice->viblfo_incr = -voice->viblfo_incr;
			voice->viblfo_val = (fluid_real_t)-2.0 - voice->viblfo_val;
		}
	}

	fluid_check_fpe("voice_write Vib LFO");

	if (voice->volenv_section == FLUID_VOICE_ENVDELAY)
		goto post_process;

	if (voice->volenv_section == FLUID_VOICE_ENVATTACK)
	{
		target_amp = fluid_atten2amp(voice->attenuation)
			* fluid_cb2amp(voice->modlfo_val * -voice->modlfo_to_vol)
			* voice->volenv_val;
	}
	else
	{
		fluid_real_t amplitude_that_reaches_noise_floor;
		fluid_real_t amp_max;

		target_amp = fluid_atten2amp(voice->attenuation)
			* fluid_cb2amp(960.0f * (1.0f - voice->volenv_val)
			+ voice->modlfo_val * -voice->modlfo_to_vol);

		if (voice->has_looped)
			amplitude_that_reaches_noise_floor = voice->amplitude_that_reaches_noise_floor_loop;
		else
			amplitude_that_reaches_noise_floor = voice->amplitude_that_reaches_noise_floor_nonloop;

		amp_max = fluid_atten2amp(voice->min_attenuation_cB) * voice->volenv_val;

		if (amp_max < amplitude_that_reaches_noise_floor)
		{
			fluid_profile(FLUID_PROF_VOICE_RELEASE, voice->ref);
			fluid_voice_off(voice);
			goto post_process;
		}
	}

	voice->amp_incr = (target_amp - voice->amp) / FLUID_BUFSIZE;

	fluid_check_fpe("voice_write amplitude calculation");

	if ((voice->amp == 0.0f) && (voice->amp_incr == 0.0f))
		goto post_process;

	voice->phase_incr = fluid_ct2hz_real
		(voice->pitch + voice->modlfo_val * voice->modlfo_to_pitch
		+ voice->viblfo_val * voice->viblfo_to_pitch
		+ voice->modenv_val * voice->modenv_to_pitch) / voice->root_pitch;

	fluid_check_fpe("voice_write phase calculation");

	if (voice->phase_incr == 0) voice->phase_incr = 1;

	fres = fluid_ct2hz(voice->fres
		+ voice->modlfo_val * voice->modlfo_to_fc
		+ voice->modenv_val * voice->modenv_to_fc);

	if (fres > 0.45f * voice->output_rate)
		fres = 0.45f * voice->output_rate;
	else if (fres < 5)
		fres = 5;

	if ((fabs(fres - voice->last_fres) > 0.01))
	{
		fluid_real_t omega = (fluid_real_t)(2.0 * M_PI * (fres / ((float)voice->output_rate)));
		fluid_real_t sin_coeff = (fluid_real_t)sin(omega);
		fluid_real_t cos_coeff = (fluid_real_t)cos(omega);
		fluid_real_t alpha_coeff = sin_coeff / (2.0f * voice->q_lin);
		fluid_real_t a0_inv = 1.0f / (1.0f + alpha_coeff);

		fluid_real_t a1_temp = -2.0f * cos_coeff * a0_inv;
		fluid_real_t a2_temp = (1.0f - alpha_coeff) * a0_inv;
		fluid_real_t b1_temp = (1.0f - cos_coeff) * a0_inv * voice->filter_gain;

		fluid_real_t b02_temp = b1_temp * 0.5f;

		if (voice->filter_startup)
		{
			voice->a1 = a1_temp;
			voice->a2 = a2_temp;
			voice->b02 = b02_temp;
			voice->b1 = b1_temp;
			voice->filter_coeff_incr_count = 0;
			voice->filter_startup = 0;
		}
		else
		{
#define FILTER_TRANSITION_SAMPLES (FLUID_BUFSIZE)

			voice->a1_incr = (a1_temp - voice->a1) / FILTER_TRANSITION_SAMPLES;
			voice->a2_incr = (a2_temp - voice->a2) / FILTER_TRANSITION_SAMPLES;
			voice->b02_incr = (b02_temp - voice->b02) / FILTER_TRANSITION_SAMPLES;
			voice->b1_incr = (b1_temp - voice->b1) / FILTER_TRANSITION_SAMPLES;

			voice->filter_coeff_incr_count = FILTER_TRANSITION_SAMPLES;
		}
		voice->last_fres = fres;
		fluid_check_fpe("voice_write filter calculation");
	}

	fluid_check_fpe("voice_write DSP coefficients");

	voice->dsp_buf = dsp_buf;

	switch (voice->interp_method)
	{
		case FLUID_INTERP_NONE:
			count = fluid_dsp_float_interpolate_none(voice);
			break;
		case FLUID_INTERP_LINEAR:
			count = fluid_dsp_float_interpolate_linear(voice);
			break;
		case FLUID_INTERP_4THORDER:
		default:
			count = fluid_dsp_float_interpolate_4th_order(voice);
			break;
		case FLUID_INTERP_7THORDER:
			count = fluid_dsp_float_interpolate_7th_order(voice);
			break;
	}

	fluid_check_fpe("voice_write interpolation");

	if (count > 0)
		fluid_voice_effects(voice, count, dsp_left_buf, dsp_right_buf,
		dsp_reverb_buf, dsp_chorus_buf);

	if (count < FLUID_BUFSIZE)
	{
		fluid_profile(FLUID_PROF_VOICE_RELEASE, voice->ref);
		fluid_voice_off(voice);
	}

post_process:
	voice->ticks += FLUID_BUFSIZE;
	fluid_check_fpe("voice_write postprocess");
	return FLUID_OK;
}

static INLINE void
fluid_voice_effects(fluid_voice_t *voice, int count,
fluid_real_t* dsp_left_buf, fluid_real_t* dsp_right_buf,
fluid_real_t* dsp_reverb_buf, fluid_real_t* dsp_chorus_buf)
{
	fluid_real_t dsp_hist1 = voice->hist1;
	fluid_real_t dsp_hist2 = voice->hist2;

	fluid_real_t dsp_a1 = voice->a1;
	fluid_real_t dsp_a2 = voice->a2;
	fluid_real_t dsp_b02 = voice->b02;
	fluid_real_t dsp_b1 = voice->b1;
	fluid_real_t dsp_a1_incr = voice->a1_incr;
	fluid_real_t dsp_a2_incr = voice->a2_incr;
	fluid_real_t dsp_b02_incr = voice->b02_incr;
	fluid_real_t dsp_b1_incr = voice->b1_incr;
	int dsp_filter_coeff_incr_count = voice->filter_coeff_incr_count;

	fluid_real_t *dsp_buf = voice->dsp_buf;

	fluid_real_t dsp_centernode;
	int dsp_i;
	float v;

	if (fabs(dsp_hist1) < 1e-20) dsp_hist1 = 0.0f;

	if (dsp_filter_coeff_incr_count > 0)
	{
		for (dsp_i = 0; dsp_i < count; dsp_i++)
		{
			dsp_centernode = dsp_buf[dsp_i] - dsp_a1 * dsp_hist1 - dsp_a2 * dsp_hist2;
			dsp_buf[dsp_i] = dsp_b02 * (dsp_centernode + dsp_hist2) + dsp_b1 * dsp_hist1;
			dsp_hist2 = dsp_hist1;
			dsp_hist1 = dsp_centernode;

			if (dsp_filter_coeff_incr_count-- > 0)
			{
				dsp_a1 += dsp_a1_incr;
				dsp_a2 += dsp_a2_incr;
				dsp_b02 += dsp_b02_incr;
				dsp_b1 += dsp_b1_incr;
			}
		}
	}
	else
	{
		for (dsp_i = 0; dsp_i < count; dsp_i++)
		{
			dsp_centernode = dsp_buf[dsp_i] - dsp_a1 * dsp_hist1 - dsp_a2 * dsp_hist2;
			dsp_buf[dsp_i] = dsp_b02 * (dsp_centernode + dsp_hist2) + dsp_b1 * dsp_hist1;
			dsp_hist2 = dsp_hist1;
			dsp_hist1 = dsp_centernode;
		}
	}

	if ((-0.5 < voice->pan) && (voice->pan < 0.5))
	{
		for (dsp_i = 0; dsp_i < count; dsp_i++)
		{
			v = voice->amp_left * dsp_buf[dsp_i];
			dsp_left_buf[dsp_i] += v;
			dsp_right_buf[dsp_i] += v;
		}
	}
	else
	{
		if (voice->amp_left != 0.0)
		{
			for (dsp_i = 0; dsp_i < count; dsp_i++)
				dsp_left_buf[dsp_i] += voice->amp_left * dsp_buf[dsp_i];
		}

		if (voice->amp_right != 0.0)
		{
			for (dsp_i = 0; dsp_i < count; dsp_i++)
				dsp_right_buf[dsp_i] += voice->amp_right * dsp_buf[dsp_i];
		}
	}

	if ((dsp_reverb_buf != NULL) && (voice->amp_reverb != 0.0))
	{
		for (dsp_i = 0; dsp_i < count; dsp_i++)
			dsp_reverb_buf[dsp_i] += voice->amp_reverb * dsp_buf[dsp_i];
	}

	if ((dsp_chorus_buf != NULL) && (voice->amp_chorus != 0))
	{
		for (dsp_i = 0; dsp_i < count; dsp_i++)
			dsp_chorus_buf[dsp_i] += voice->amp_chorus * dsp_buf[dsp_i];
	}

	voice->hist1 = dsp_hist1;
	voice->hist2 = dsp_hist2;
	voice->a1 = dsp_a1;
	voice->a2 = dsp_a2;
	voice->b02 = dsp_b02;
	voice->b1 = dsp_b1;
	voice->filter_coeff_incr_count = dsp_filter_coeff_incr_count;

	fluid_check_fpe("voice_effects");
}

fluid_channel_t*
fluid_voice_get_channel(fluid_voice_t* voice)
{
	return voice->channel;
}

void fluid_voice_start(fluid_voice_t* voice)
{
	fluid_voice_calculate_runtime_synthesis_parameters(voice);

	voice->check_sample_sanity_flag = FLUID_SAMPLESANITY_STARTUP;

	voice->ref = fluid_profile_ref();

	voice->status = FLUID_VOICE_ON;
}

int fluid_voice_calculate_runtime_synthesis_parameters(fluid_voice_t* voice)
{
	int i;

	int list_of_generators_to_initialize[35] = {
		GEN_STARTADDROFS,
		GEN_ENDADDROFS,
		GEN_STARTLOOPADDROFS,
		GEN_ENDLOOPADDROFS,

		GEN_MODLFOTOPITCH,
		GEN_VIBLFOTOPITCH,
		GEN_MODENVTOPITCH,
		GEN_FILTERFC,
		GEN_FILTERQ,
		GEN_MODLFOTOFILTERFC,
		GEN_MODENVTOFILTERFC,

		GEN_MODLFOTOVOL,

		GEN_CHORUSSEND,
		GEN_REVERBSEND,
		GEN_PAN,

		GEN_MODLFODELAY,
		GEN_MODLFOFREQ,
		GEN_VIBLFODELAY,
		GEN_VIBLFOFREQ,
		GEN_MODENVDELAY,
		GEN_MODENVATTACK,
		GEN_MODENVHOLD,
		GEN_MODENVDECAY,

		GEN_MODENVRELEASE,

		GEN_VOLENVDELAY,
		GEN_VOLENVATTACK,
		GEN_VOLENVHOLD,
		GEN_VOLENVDECAY,

		GEN_VOLENVRELEASE,

		GEN_KEYNUM,
		GEN_VELOCITY,
		GEN_ATTENUATION,

		GEN_OVERRIDEROOTKEY,
		GEN_PITCH,
		-1 };

	for (i = 0; i < voice->mod_count; i++) {
		fluid_mod_t* mod = &voice->mod[i];
		fluid_real_t modval = fluid_mod_get_value(mod, voice->channel, voice);
		int dest_gen_index = mod->dest;
		fluid_gen_t* dest_gen = &voice->gen[dest_gen_index];
		dest_gen->mod += modval;

	}

	if (fluid_channel_has_tuning(voice->channel)) {
#define __pitch(_k) fluid_tuning_get_pitch(tuning, _k)
		fluid_tuning_t* tuning = fluid_channel_get_tuning(voice->channel);
		voice->gen[GEN_PITCH].val = (__pitch(60) + (voice->gen[GEN_SCALETUNE].val / 100.0f *
			(__pitch(voice->key) - __pitch(60))));
	}
	else {
		voice->gen[GEN_PITCH].val = (voice->gen[GEN_SCALETUNE].val * (voice->key - 60.0f)
			+ 100.0f * 60.0f);
	}

	for (i = 0; list_of_generators_to_initialize[i] != -1; i++) {
		fluid_voice_update_param(voice, list_of_generators_to_initialize[i]);
	}

	voice->min_attenuation_cB = fluid_voice_get_lower_boundary_for_attenuation(voice);

	return FLUID_OK;
}

int calculate_hold_decay_buffers(fluid_voice_t* voice, int gen_base,
	int gen_key2base, int is_decay)
{
	fluid_real_t timecents;
	fluid_real_t seconds;
	int buffers;

	timecents = (_GEN(voice, gen_base) + _GEN(voice, gen_key2base) * (60.0 - voice->key));

	if (is_decay){
		if (timecents > 8000.0) {
			timecents = 8000.0;
		}
	}
	else {
		if (timecents > 5000) {
			timecents = 5000.0;
		}

		if (timecents <= -32768.) {
			return 0;
		}
	}

	if (timecents < -12000.0) {
		timecents = -12000.0;
	}

	seconds = fluid_tc2sec(timecents);

	buffers = (int)(((fluid_real_t)voice->output_rate * seconds)
		/ (fluid_real_t)FLUID_BUFSIZE
		+ 0.5);

	return buffers;
}

void fluid_voice_update_param(fluid_voice_t* voice, int gen)
{
	double q_dB;
	fluid_real_t x;
	fluid_real_t y;
	unsigned int count;
	static const float ALT_ATTENUATION_SCALE = 0.4;

	switch (gen) {
		case GEN_PAN:
			voice->pan = _GEN(voice, GEN_PAN);
			voice->amp_left = fluid_pan(voice->pan, 1) * voice->synth_gain / 32768.0f;
			voice->amp_right = fluid_pan(voice->pan, 0) * voice->synth_gain / 32768.0f;
			break;

		case GEN_ATTENUATION:
			voice->attenuation = ((fluid_real_t)(voice)->gen[GEN_ATTENUATION].val*ALT_ATTENUATION_SCALE) +
				(fluid_real_t)(voice)->gen[GEN_ATTENUATION].mod + (fluid_real_t)(voice)->gen[GEN_ATTENUATION].nrpn;

			fluid_clip(voice->attenuation, 0.0, 1440.0);
			break;

		case GEN_PITCH:
		case GEN_COARSETUNE:
		case GEN_FINETUNE:
			voice->pitch = (_GEN(voice, GEN_PITCH)
				+ 100.0f * _GEN(voice, GEN_COARSETUNE)
				+ _GEN(voice, GEN_FINETUNE));
			break;

		case GEN_REVERBSEND:
			voice->reverb_send = _GEN(voice, GEN_REVERBSEND) / 1000.0f;
			fluid_clip(voice->reverb_send, 0.0, 1.0);
			voice->amp_reverb = voice->reverb_send * voice->synth_gain / 32768.0f;
			break;

		case GEN_CHORUSSEND:
			voice->chorus_send = _GEN(voice, GEN_CHORUSSEND) / 1000.0f;
			fluid_clip(voice->chorus_send, 0.0, 1.0);
			voice->amp_chorus = voice->chorus_send * voice->synth_gain / 32768.0f;
			break;

		case GEN_OVERRIDEROOTKEY:
			if (voice->gen[GEN_OVERRIDEROOTKEY].val > -1) {
				voice->root_pitch = voice->gen[GEN_OVERRIDEROOTKEY].val * 100.0f
					- voice->sample->pitchadj;
			}
			else {
				voice->root_pitch = voice->sample->origpitch * 100.0f - voice->sample->pitchadj;
			}
			voice->root_pitch = fluid_ct2hz(voice->root_pitch);
			if (voice->sample != NULL) {
				voice->root_pitch *= (fluid_real_t)voice->output_rate / voice->sample->samplerate;
			}
			break;

		case GEN_FILTERFC:
			voice->fres = _GEN(voice, GEN_FILTERFC);

			voice->last_fres = -1.0f;
			break;

		case GEN_FILTERQ:
			q_dB = _GEN(voice, GEN_FILTERQ) / 10.0f;

			fluid_clip(q_dB, 0.0f, 96.0f);

			q_dB -= 3.01f;

			voice->q_lin = (fluid_real_t)(pow((fluid_real_t)10.0f, (fluid_real_t)(q_dB / 20.0f)));

			voice->filter_gain = (fluid_real_t)(1.0 / sqrt(voice->q_lin));

			voice->last_fres = -1.;
			break;

		case GEN_MODLFOTOPITCH:
			voice->modlfo_to_pitch = _GEN(voice, GEN_MODLFOTOPITCH);
			fluid_clip(voice->modlfo_to_pitch, -12000.0, 12000.0);
			break;

		case GEN_MODLFOTOVOL:
			voice->modlfo_to_vol = _GEN(voice, GEN_MODLFOTOVOL);
			fluid_clip(voice->modlfo_to_vol, -960.0, 960.0);
			break;

		case GEN_MODLFOTOFILTERFC:
			voice->modlfo_to_fc = _GEN(voice, GEN_MODLFOTOFILTERFC);
			fluid_clip(voice->modlfo_to_fc, -12000, 12000);
			break;

		case GEN_MODLFODELAY:
			x = _GEN(voice, GEN_MODLFODELAY);
			fluid_clip(x, -12000.0f, 5000.0f);
			voice->modlfo_delay = (unsigned int)(voice->output_rate * fluid_tc2sec_delay(x));
			break;

		case GEN_MODLFOFREQ:
			x = _GEN(voice, GEN_MODLFOFREQ);
			fluid_clip(x, -16000.0f, 4500.0f);
			voice->modlfo_incr = (4.0f * FLUID_BUFSIZE * fluid_act2hz(x) / voice->output_rate);
			break;

		case GEN_VIBLFOFREQ:
			x = _GEN(voice, GEN_VIBLFOFREQ);
			fluid_clip(x, -16000.0f, 4500.0f);
			voice->viblfo_incr = (4.0f * FLUID_BUFSIZE * fluid_act2hz(x) / voice->output_rate);
			break;

		case GEN_VIBLFODELAY:
			x = _GEN(voice, GEN_VIBLFODELAY);
			fluid_clip(x, -12000.0f, 5000.0f);
			voice->viblfo_delay = (unsigned int)(voice->output_rate * fluid_tc2sec_delay(x));
			break;

		case GEN_VIBLFOTOPITCH:
			voice->viblfo_to_pitch = _GEN(voice, GEN_VIBLFOTOPITCH);
			fluid_clip(voice->viblfo_to_pitch, -12000.0, 12000.0);
			break;

		case GEN_KEYNUM:
			x = _GEN(voice, GEN_KEYNUM);
			if (x >= 0){
				voice->key = x;
			}
			break;

		case GEN_VELOCITY:
			x = _GEN(voice, GEN_VELOCITY);
			if (x > 0) {
				voice->vel = x;
			}
			break;

		case GEN_MODENVTOPITCH:
			voice->modenv_to_pitch = _GEN(voice, GEN_MODENVTOPITCH);
			fluid_clip(voice->modenv_to_pitch, -12000.0, 12000.0);
			break;

		case GEN_MODENVTOFILTERFC:
			voice->modenv_to_fc = _GEN(voice, GEN_MODENVTOFILTERFC);

			fluid_clip(voice->modenv_to_fc, -12000.0, 12000.0);
			break;

		case GEN_STARTADDROFS:
		case GEN_STARTADDRCOARSEOFS:
			if (voice->sample != NULL) {
				voice->start = (voice->sample->start
					+ (int)_GEN(voice, GEN_STARTADDROFS)
					+ 32768 * (int)_GEN(voice, GEN_STARTADDRCOARSEOFS));
				voice->check_sample_sanity_flag = FLUID_SAMPLESANITY_CHECK;
			}
			break;
		case GEN_ENDADDROFS:
		case GEN_ENDADDRCOARSEOFS:
			if (voice->sample != NULL) {
				voice->end = (voice->sample->end
					+ (int)_GEN(voice, GEN_ENDADDROFS)
					+ 32768 * (int)_GEN(voice, GEN_ENDADDRCOARSEOFS));
				voice->check_sample_sanity_flag = FLUID_SAMPLESANITY_CHECK;
			}
			break;
		case GEN_STARTLOOPADDROFS:
		case GEN_STARTLOOPADDRCOARSEOFS:
			if (voice->sample != NULL) {
				voice->loopstart = (voice->sample->loopstart
					+ (int)_GEN(voice, GEN_STARTLOOPADDROFS)
					+ 32768 * (int)_GEN(voice, GEN_STARTLOOPADDRCOARSEOFS));
				voice->check_sample_sanity_flag = FLUID_SAMPLESANITY_CHECK;
			}
			break;

		case GEN_ENDLOOPADDROFS:
		case GEN_ENDLOOPADDRCOARSEOFS:
			if (voice->sample != NULL) {
				voice->loopend = (voice->sample->loopend
					+ (int)_GEN(voice, GEN_ENDLOOPADDROFS)
					+ 32768 * (int)_GEN(voice, GEN_ENDLOOPADDRCOARSEOFS));
				voice->check_sample_sanity_flag = FLUID_SAMPLESANITY_CHECK;
			}
			break;

#define NUM_BUFFERS_DELAY(_v)   (unsigned int) (voice->output_rate * fluid_tc2sec_delay(_v) / FLUID_BUFSIZE)
#define NUM_BUFFERS_ATTACK(_v)  (unsigned int) (voice->output_rate * fluid_tc2sec_attack(_v) / FLUID_BUFSIZE)
#define NUM_BUFFERS_RELEASE(_v) (unsigned int) (voice->output_rate * fluid_tc2sec_release(_v) / FLUID_BUFSIZE)

		case GEN_VOLENVDELAY:
			x = _GEN(voice, GEN_VOLENVDELAY);
			fluid_clip(x, -12000.0f, 5000.0f);
			count = NUM_BUFFERS_DELAY(x);
			voice->volenv_data[FLUID_VOICE_ENVDELAY].count = count;
			voice->volenv_data[FLUID_VOICE_ENVDELAY].coeff = 0.0f;
			voice->volenv_data[FLUID_VOICE_ENVDELAY].incr = 0.0f;
			voice->volenv_data[FLUID_VOICE_ENVDELAY].min = -1.0f;
			voice->volenv_data[FLUID_VOICE_ENVDELAY].max = 1.0f;
			break;

		case GEN_VOLENVATTACK:
			x = _GEN(voice, GEN_VOLENVATTACK);
			fluid_clip(x, -12000.0f, 8000.0f);
			count = 1 + NUM_BUFFERS_ATTACK(x);
			voice->volenv_data[FLUID_VOICE_ENVATTACK].count = count;
			voice->volenv_data[FLUID_VOICE_ENVATTACK].coeff = 1.0f;
			voice->volenv_data[FLUID_VOICE_ENVATTACK].incr = count ? 1.0f / count : 0.0f;
			voice->volenv_data[FLUID_VOICE_ENVATTACK].min = -1.0f;
			voice->volenv_data[FLUID_VOICE_ENVATTACK].max = 1.0f;
			break;

		case GEN_VOLENVHOLD:
		case GEN_KEYTOVOLENVHOLD:
			count = calculate_hold_decay_buffers(voice, GEN_VOLENVHOLD, GEN_KEYTOVOLENVHOLD, 0);
			voice->volenv_data[FLUID_VOICE_ENVHOLD].count = count;
			voice->volenv_data[FLUID_VOICE_ENVHOLD].coeff = 1.0f;
			voice->volenv_data[FLUID_VOICE_ENVHOLD].incr = 0.0f;
			voice->volenv_data[FLUID_VOICE_ENVHOLD].min = -1.0f;
			voice->volenv_data[FLUID_VOICE_ENVHOLD].max = 2.0f;
			break;

		case GEN_VOLENVDECAY:
		case GEN_VOLENVSUSTAIN:
		case GEN_KEYTOVOLENVDECAY:
			y = 1.0f - 0.001f * _GEN(voice, GEN_VOLENVSUSTAIN);
			fluid_clip(y, 0.0f, 1.0f);
			count = calculate_hold_decay_buffers(voice, GEN_VOLENVDECAY, GEN_KEYTOVOLENVDECAY, 1);
			voice->volenv_data[FLUID_VOICE_ENVDECAY].count = count;
			voice->volenv_data[FLUID_VOICE_ENVDECAY].coeff = 1.0f;
			voice->volenv_data[FLUID_VOICE_ENVDECAY].incr = count ? -1.0f / count : 0.0f;
			voice->volenv_data[FLUID_VOICE_ENVDECAY].min = y;
			voice->volenv_data[FLUID_VOICE_ENVDECAY].max = 2.0f;
			break;

		case GEN_VOLENVRELEASE:
			x = _GEN(voice, GEN_VOLENVRELEASE);
			fluid_clip(x, FLUID_MIN_VOLENVRELEASE, 8000.0f);
			count = 1 + NUM_BUFFERS_RELEASE(x);
			voice->volenv_data[FLUID_VOICE_ENVRELEASE].count = count;
			voice->volenv_data[FLUID_VOICE_ENVRELEASE].coeff = 1.0f;
			voice->volenv_data[FLUID_VOICE_ENVRELEASE].incr = count ? -1.0f / count : 0.0f;
			voice->volenv_data[FLUID_VOICE_ENVRELEASE].min = 0.0f;
			voice->volenv_data[FLUID_VOICE_ENVRELEASE].max = 1.0f;
			break;

		case GEN_MODENVDELAY:
			x = _GEN(voice, GEN_MODENVDELAY);
			fluid_clip(x, -12000.0f, 5000.0f);
			voice->modenv_data[FLUID_VOICE_ENVDELAY].count = NUM_BUFFERS_DELAY(x);
			voice->modenv_data[FLUID_VOICE_ENVDELAY].coeff = 0.0f;
			voice->modenv_data[FLUID_VOICE_ENVDELAY].incr = 0.0f;
			voice->modenv_data[FLUID_VOICE_ENVDELAY].min = -1.0f;
			voice->modenv_data[FLUID_VOICE_ENVDELAY].max = 1.0f;
			break;

		case GEN_MODENVATTACK:
			x = _GEN(voice, GEN_MODENVATTACK);
			fluid_clip(x, -12000.0f, 8000.0f);
			count = 1 + NUM_BUFFERS_ATTACK(x);
			voice->modenv_data[FLUID_VOICE_ENVATTACK].count = count;
			voice->modenv_data[FLUID_VOICE_ENVATTACK].coeff = 1.0f;
			voice->modenv_data[FLUID_VOICE_ENVATTACK].incr = count ? 1.0f / count : 0.0f;
			voice->modenv_data[FLUID_VOICE_ENVATTACK].min = -1.0f;
			voice->modenv_data[FLUID_VOICE_ENVATTACK].max = 1.0f;
			break;

		case GEN_MODENVHOLD:
		case GEN_KEYTOMODENVHOLD:
			count = calculate_hold_decay_buffers(voice, GEN_MODENVHOLD, GEN_KEYTOMODENVHOLD, 0);
			voice->modenv_data[FLUID_VOICE_ENVHOLD].count = count;
			voice->modenv_data[FLUID_VOICE_ENVHOLD].coeff = 1.0f;
			voice->modenv_data[FLUID_VOICE_ENVHOLD].incr = 0.0f;
			voice->modenv_data[FLUID_VOICE_ENVHOLD].min = -1.0f;
			voice->modenv_data[FLUID_VOICE_ENVHOLD].max = 2.0f;
			break;

		case GEN_MODENVDECAY:
		case GEN_MODENVSUSTAIN:
		case GEN_KEYTOMODENVDECAY:
			count = calculate_hold_decay_buffers(voice, GEN_MODENVDECAY, GEN_KEYTOMODENVDECAY, 1);
			y = 1.0f - 0.001f * _GEN(voice, GEN_MODENVSUSTAIN);
			fluid_clip(y, 0.0f, 1.0f);
			voice->modenv_data[FLUID_VOICE_ENVDECAY].count = count;
			voice->modenv_data[FLUID_VOICE_ENVDECAY].coeff = 1.0f;
			voice->modenv_data[FLUID_VOICE_ENVDECAY].incr = count ? -1.0f / count : 0.0f;
			voice->modenv_data[FLUID_VOICE_ENVDECAY].min = y;
			voice->modenv_data[FLUID_VOICE_ENVDECAY].max = 2.0f;
			break;

		case GEN_MODENVRELEASE:
			x = _GEN(voice, GEN_MODENVRELEASE);
			fluid_clip(x, -12000.0f, 8000.0f);
			count = 1 + NUM_BUFFERS_RELEASE(x);
			voice->modenv_data[FLUID_VOICE_ENVRELEASE].count = count;
			voice->modenv_data[FLUID_VOICE_ENVRELEASE].coeff = 1.0f;
			voice->modenv_data[FLUID_VOICE_ENVRELEASE].incr = count ? -1.0f / count : 0.0;
			voice->modenv_data[FLUID_VOICE_ENVRELEASE].min = 0.0f;
			voice->modenv_data[FLUID_VOICE_ENVRELEASE].max = 2.0f;
			break;
	}
}

int fluid_voice_modulate(fluid_voice_t* voice, int cc, int ctrl)
{
	int i, k;
	fluid_mod_t* mod;
	int gen;
	fluid_real_t modval;

	for (i = 0; i < voice->mod_count; i++) {
		mod = &voice->mod[i];

		if (fluid_mod_has_source(mod, cc, ctrl)) {
			gen = fluid_mod_get_dest(mod);
			modval = 0.0;

			for (k = 0; k < voice->mod_count; k++) {
				if (fluid_mod_has_dest(&voice->mod[k], gen)) {
					modval += fluid_mod_get_value(&voice->mod[k], voice->channel, voice);
				}
			}

			fluid_gen_set_mod(&voice->gen[gen], modval);

			fluid_voice_update_param(voice, gen);
		}
	}
	return FLUID_OK;
}

int fluid_voice_modulate_all(fluid_voice_t* voice)
{
	fluid_mod_t* mod;
	int i, k, gen;
	fluid_real_t modval;

	for (i = 0; i < voice->mod_count; i++) {
		mod = &voice->mod[i];
		gen = fluid_mod_get_dest(mod);
		modval = 0.0;

		for (k = 0; k < voice->mod_count; k++) {
			if (fluid_mod_has_dest(&voice->mod[k], gen)) {
				modval += fluid_mod_get_value(&voice->mod[k], voice->channel, voice);
			}
		}

		fluid_gen_set_mod(&voice->gen[gen], modval);

		fluid_voice_update_param(voice, gen);
	}

	return FLUID_OK;
}

int fluid_voice_noteoff(fluid_voice_t* voice)
{
	fluid_profile(FLUID_PROF_VOICE_NOTE, voice->ref);

	if (voice->channel && fluid_channel_sustained(voice->channel)) {
		voice->status = FLUID_VOICE_SUSTAINED;
	}
	else {
		if (voice->volenv_section == FLUID_VOICE_ENVATTACK) {
			if (voice->volenv_val > 0){
				fluid_real_t lfo = voice->modlfo_val * -voice->modlfo_to_vol;
				fluid_real_t amp = voice->volenv_val * pow((fluid_real_t)10.0, (fluid_real_t)(lfo / -200));
				fluid_real_t env_value = -((-200 * log(amp) / log(10.0) - lfo) / 960.0 - 1);
				fluid_clip(env_value, 0.0, 1.0);
				voice->volenv_val = env_value;
			}
		}
		voice->volenv_section = FLUID_VOICE_ENVRELEASE;
		voice->volenv_count = 0;
		voice->modenv_section = FLUID_VOICE_ENVRELEASE;
		voice->modenv_count = 0;
	}

	return FLUID_OK;
}

int fluid_voice_kill_excl(fluid_voice_t* voice){
	if (!_PLAYING(voice)) {
		return FLUID_OK;
	}

	fluid_voice_gen_set(voice, GEN_EXCLUSIVECLASS, 0);

	if (voice->volenv_section != FLUID_VOICE_ENVRELEASE){
		voice->volenv_section = FLUID_VOICE_ENVRELEASE;
		voice->volenv_count = 0;
		voice->modenv_section = FLUID_VOICE_ENVRELEASE;
		voice->modenv_count = 0;
	}

	fluid_voice_gen_set(voice, GEN_VOLENVRELEASE, -200);
	fluid_voice_update_param(voice, GEN_VOLENVRELEASE);

	fluid_voice_gen_set(voice, GEN_MODENVRELEASE, -200);
	fluid_voice_update_param(voice, GEN_MODENVRELEASE);

	return FLUID_OK;
}

int fluid_voice_off(fluid_voice_t* voice)
{
	fluid_profile(FLUID_PROF_VOICE_RELEASE, voice->ref);

	voice->chan = NO_CHANNEL;
	voice->volenv_section = FLUID_VOICE_ENVFINISHED;
	voice->volenv_count = 0;
	voice->modenv_section = FLUID_VOICE_ENVFINISHED;
	voice->modenv_count = 0;
	voice->status = FLUID_VOICE_OFF;

	if (voice->sample) {
		fluid_sample_decr_ref(voice->sample);
		voice->sample = NULL;
	}

	return FLUID_OK;
}

void fluid_voice_add_mod(fluid_voice_t* voice, fluid_mod_t* mod, int mode)
{
	int i;

	if (((mod->flags1 & FLUID_MOD_CC) == 0)
		&& ((mod->src1 != 0)
		&& (mod->src1 != 2)
		&& (mod->src1 != 3)
		&& (mod->src1 != 10)
		&& (mod->src1 != 13)
		&& (mod->src1 != 14)
		&& (mod->src1 != 16))) {
		FLUID_LOG1(FLUID_WARN, "Ignoring invalid controller, using non-CC source %i.", mod->src1);
		return;
	}

	if (mode == FLUID_VOICE_ADD) {
		for (i = 0; i < voice->mod_count; i++) {
			if (fluid_mod_test_identity(&voice->mod[i], mod)) {
				voice->mod[i].amount += mod->amount;
				return;
			}
		}

	}
	else if (mode == FLUID_VOICE_OVERWRITE) {
		for (i = 0; i < voice->mod_count; i++) {
			if (fluid_mod_test_identity(&voice->mod[i], mod)) {
				voice->mod[i].amount = mod->amount;
				return;
			}
		}
	}

	if (voice->mod_count < FLUID_NUM_MOD) {
		fluid_mod_clone(&voice->mod[voice->mod_count++], mod);
	}
}

unsigned int fluid_voice_get_id(fluid_voice_t* voice)
{
	return voice->id;
}

int fluid_voice_is_playing(fluid_voice_t* voice)
{
	return _PLAYING(voice);
}

fluid_real_t fluid_voice_get_lower_boundary_for_attenuation(fluid_voice_t* voice)
{
	int i;
	fluid_mod_t* mod;
	fluid_real_t possible_att_reduction_cB = 0;
	fluid_real_t lower_bound;

	for (i = 0; i < voice->mod_count; i++) {
		mod = &voice->mod[i];

		if ((mod->dest == GEN_ATTENUATION)
			&& ((mod->flags1 & FLUID_MOD_CC) || (mod->flags2 & FLUID_MOD_CC))) {
			fluid_real_t current_val = fluid_mod_get_value(mod, voice->channel, voice);
			fluid_real_t v = fabs(mod->amount);

			if ((mod->src1 == FLUID_MOD_PITCHWHEEL)
				|| (mod->flags1 & FLUID_MOD_BIPOLAR)
				|| (mod->flags2 & FLUID_MOD_BIPOLAR)
				|| (mod->amount < 0)) {
				v *= -1.0;
			}
			else {
				v = 0;
			}

			if (current_val > v){
				possible_att_reduction_cB += (current_val - v);
			}
		}
	}

	lower_bound = voice->attenuation - possible_att_reduction_cB;

	if (lower_bound < 0) {
		lower_bound = 0;
	}
	return lower_bound;
}

void fluid_voice_check_sample_sanity(fluid_voice_t* voice)
{
	int min_index_nonloop = (int)voice->sample->start;
	int max_index_nonloop = (int)voice->sample->end;

	int min_index_loop = (int)voice->sample->start + FLUID_MIN_LOOP_PAD;
	int max_index_loop = (int)voice->sample->end - FLUID_MIN_LOOP_PAD + 1;
	fluid_check_fpe("voice_check_sample_sanity start");

	if (!voice->check_sample_sanity_flag){
		return;
	}

#if 0
	printf("Sample from %i to %i\n", voice->sample->start, voice->sample->end);
	printf("Sample loop from %i %i\n", voice->sample->loopstart, voice->sample->loopend);
	printf("Playback from %i to %i\n", voice->start, voice->end);
	printf("Playback loop from %i to %i\n", voice->loopstart, voice->loopend);
#endif

	if (voice->start < min_index_nonloop){
		voice->start = min_index_nonloop;
	}
	else if (voice->start > max_index_nonloop){
		voice->start = max_index_nonloop;
	}

	if (voice->end < min_index_nonloop){
		voice->end = min_index_nonloop;
	}
	else if (voice->end > max_index_nonloop){
		voice->end = max_index_nonloop;
	}

	if (voice->start > voice->end){
		int temp = voice->start;
		voice->start = voice->end;
		voice->end = temp;

	}

	if (voice->start == voice->end){
		fluid_voice_off(voice);
		return;
	}

	if ((_SAMPLEMODE(voice) == FLUID_LOOP_UNTIL_RELEASE)
		|| (_SAMPLEMODE(voice) == FLUID_LOOP_DURING_RELEASE)) {
		if (voice->loopstart < min_index_loop){
			voice->loopstart = min_index_loop;
		}
		else if (voice->loopstart > max_index_loop){
			voice->loopstart = max_index_loop;
		}

		if (voice->loopend < min_index_loop){
			voice->loopend = min_index_loop;
		}
		else if (voice->loopend > max_index_loop){
			voice->loopend = max_index_loop;
		}

		if (voice->loopstart > voice->loopend){
			int temp = voice->loopstart;
			voice->loopstart = voice->loopend;
			voice->loopend = temp;

		}

		if (voice->loopend < voice->loopstart + FLUID_MIN_LOOP_SIZE){
			voice->gen[GEN_SAMPLEMODE].val = FLUID_UNLOOPED;
		}

		if ((int)voice->loopstart >= (int)voice->sample->loopstart
			&& (int)voice->loopend <= (int)voice->sample->loopend){
			if (voice->sample->amplitude_that_reaches_noise_floor_is_valid){
				voice->amplitude_that_reaches_noise_floor_loop = voice->sample->amplitude_that_reaches_noise_floor / voice->synth_gain;
			}
			else {
				voice->amplitude_that_reaches_noise_floor_loop = voice->amplitude_that_reaches_noise_floor_nonloop;
			};
		};

	}

	if (voice->check_sample_sanity_flag & FLUID_SAMPLESANITY_STARTUP){
		if (max_index_loop - min_index_loop < FLUID_MIN_LOOP_SIZE){
			if ((_SAMPLEMODE(voice) == FLUID_LOOP_UNTIL_RELEASE)
				|| (_SAMPLEMODE(voice) == FLUID_LOOP_DURING_RELEASE)){
				voice->gen[GEN_SAMPLEMODE].val = FLUID_UNLOOPED;
			}
		}

		fluid_phase_set_int(voice->phase, voice->start);
	}

	if (((_SAMPLEMODE(voice) == FLUID_LOOP_UNTIL_RELEASE) && (voice->volenv_section < FLUID_VOICE_ENVRELEASE))
		|| (_SAMPLEMODE(voice) == FLUID_LOOP_DURING_RELEASE)) {
		int index_in_sample = fluid_phase_index(voice->phase);
		if (index_in_sample >= voice->loopend){
			fluid_phase_set_int(voice->phase, voice->loopstart);
		}
	}

	voice->check_sample_sanity_flag = 0;
#if 0
	printf("Sane? playback loop from %i to %i\n", voice->loopstart, voice->loopend);
#endif
	fluid_check_fpe("voice_check_sample_sanity");
}

int fluid_voice_set_param(fluid_voice_t* voice, int gen, fluid_real_t nrpn_value, int abs)
{
	voice->gen[gen].nrpn = nrpn_value;
	voice->gen[gen].flags = (abs) ? GEN_ABS_NRPN : GEN_SET;
	fluid_voice_update_param(voice, gen);
	return FLUID_OK;
}

int fluid_voice_set_gain(fluid_voice_t* voice, fluid_real_t gain)
{
	if (gain < 0.0000001){
		gain = 0.0000001;
	}

	voice->synth_gain = gain;
	voice->amp_left = fluid_pan(voice->pan, 1) * gain / 32768.0f;
	voice->amp_right = fluid_pan(voice->pan, 0) * gain / 32768.0f;
	voice->amp_reverb = voice->reverb_send * gain / 32768.0f;
	voice->amp_chorus = voice->chorus_send * gain / 32768.0f;

	return FLUID_OK;
}

int fluid_voice_optimize_sample(fluid_sample_t* s)
{
	signed short peak_max = 0;
	signed short peak_min = 0;
	signed short peak;
	fluid_real_t normalized_amplitude_during_loop;
	double result;
	int i;

	if (!s->valid) return (FLUID_OK);

	if (!s->amplitude_that_reaches_noise_floor_is_valid){
		for (i = (int)s->loopstart; i < (int)s->loopend; i++){
			signed short val = s->data[i];
			if (val > peak_max) {
				peak_max = val;
			}
			else if (val < peak_min) {
				peak_min = val;
			}
		}

		if (peak_max >-peak_min){
			peak = peak_max;
		}
		else {
			peak = -peak_min;
		};
		if (peak == 0){
			peak = 1;
		};

		normalized_amplitude_during_loop = ((fluid_real_t)peak) / 32768.;
		result = FLUID_NOISE_FLOOR / normalized_amplitude_during_loop;

		s->amplitude_that_reaches_noise_floor = (double)result;
		s->amplitude_that_reaches_noise_floor_is_valid = 1;
#if 0
		printf("Sample peak detection: factor %f\n", (double)result);
#endif
	};
	return FLUID_OK;
}

//
//MERGED FILE END: fluid_voice.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: fluid_adriver.h
//

#ifndef _FLUID_AUDRIVER_H
#define _FLUID_AUDRIVER_H

void fluid_audio_driver_settings(fluid_settings_t* settings);

struct _fluid_audio_driver_t
{
	char* name;
};

#endif

//
//MERGED FILE END: fluid_adriver.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: fluid_zlsfont.cpp
//

#include "../../Source/ZL_File_Impl.h"
#include <ZL_Application.h>

#ifndef _FLUID_DEFSFONT_H
#define _FLUID_DEFSFONT_H

#define SF_SAMPMODES_LOOP	1
#define SF_SAMPMODES_UNROLL	2

#define SF_MIN_SAMPLERATE	400
#define SF_MAX_SAMPLERATE	50000

#define SF_MIN_SAMPLE_LENGTH	32

typedef struct _SFVersion
{
	unsigned short major;
	unsigned short minor;
}
SFVersion;

typedef struct _SFMod
{
	unsigned short src;
	unsigned short dest;
	signed short amount;
	unsigned short amtsrc;
	unsigned short trans;
}
SFMod;

typedef union _SFGenAmount
{
	signed short sword;
	unsigned short uword;
	struct
	{
		unsigned char lo;
		unsigned char hi;
	}
	range;
}
SFGenAmount;

typedef struct _SFGen
{
	unsigned short id;
	SFGenAmount amount;
}
SFGen;

typedef struct _SFZone
{
	fluid_list_t *instsamp;
	fluid_list_t *gen;
	fluid_list_t *mod;
}
SFZone;

typedef struct _SFSample
{
	char name[21];
	unsigned char samfile;
	unsigned int start;
	unsigned int end;
	unsigned int loopstart;
	unsigned int loopend;
	unsigned int samplerate;
	unsigned char origpitch;
	signed char pitchadj;
	unsigned short sampletype;
}
SFSample;

typedef struct _SFInst
{
	char name[21];
	fluid_list_t *zone;
}
SFInst;

typedef struct _SFPreset
{
	char name[21];
	unsigned short prenum;
	unsigned short bank;
	unsigned int libr;
	unsigned int genre;
	unsigned int morph;
	fluid_list_t *zone;
}
SFPreset;

typedef struct _SFData
{
	SFVersion version;
	SFVersion romver;
	unsigned int samplepos;
	unsigned int samplesize;
	ZL_RWops* sfrw;
	fluid_list_t *info;
	fluid_list_t *preset;
	fluid_list_t *inst;
	fluid_list_t *sample;
}
SFData;

enum
{
	UNKN_ID, RIFF_ID, LIST_ID, SFBK_ID,
	INFO_ID, SDTA_ID, PDTA_ID,

	IFIL_ID, ISNG_ID, INAM_ID, IROM_ID,
	IVER_ID, ICRD_ID, IENG_ID, IPRD_ID,
	ICOP_ID, ICMT_ID, ISFT_ID,

	SNAM_ID, SMPL_ID,
	PHDR_ID, PBAG_ID, PMOD_ID, PGEN_ID,
	IHDR_ID, IBAG_ID, IMOD_ID, IGEN_ID,
	SHDR_ID
};

typedef enum
{
	Gen_StartAddrOfs, Gen_EndAddrOfs, Gen_StartLoopAddrOfs,
	Gen_EndLoopAddrOfs, Gen_StartAddrCoarseOfs, Gen_ModLFO2Pitch,
	Gen_VibLFO2Pitch, Gen_ModEnv2Pitch, Gen_FilterFc, Gen_FilterQ,
	Gen_ModLFO2FilterFc, Gen_ModEnv2FilterFc, Gen_EndAddrCoarseOfs,
	Gen_ModLFO2Vol, Gen_Unused1, Gen_ChorusSend, Gen_ReverbSend, Gen_Pan,
	Gen_Unused2, Gen_Unused3, Gen_Unused4,
	Gen_ModLFODelay, Gen_ModLFOFreq, Gen_VibLFODelay, Gen_VibLFOFreq,
	Gen_ModEnvDelay, Gen_ModEnvAttack, Gen_ModEnvHold, Gen_ModEnvDecay,
	Gen_ModEnvSustain, Gen_ModEnvRelease, Gen_Key2ModEnvHold,
	Gen_Key2ModEnvDecay, Gen_VolEnvDelay, Gen_VolEnvAttack,
	Gen_VolEnvHold, Gen_VolEnvDecay, Gen_VolEnvSustain, Gen_VolEnvRelease,
	Gen_Key2VolEnvHold, Gen_Key2VolEnvDecay, Gen_Instrument,
	Gen_Reserved1, Gen_KeyRange, Gen_VelRange,
	Gen_StartLoopAddrCoarseOfs, Gen_Keynum, Gen_Velocity,
	Gen_Attenuation, Gen_Reserved2, Gen_EndLoopAddrCoarseOfs,
	Gen_CoarseTune, Gen_FineTune, Gen_SampleId, Gen_SampleModes,
	Gen_Reserved3, Gen_ScaleTune, Gen_ExclusiveClass, Gen_OverrideRootKey,
	Gen_Dummy
}
Gen_Type;

#define Gen_MaxValid 	Gen_Dummy - 1
#define Gen_Count	Gen_Dummy
#define GenArrSize sizeof(SFGenAmount)*Gen_Count

typedef enum
{
	None,
	Unit_Smpls,
	Unit_32kSmpls,
	Unit_Cent,
	Unit_HzCent,
	Unit_TCent,
	Unit_cB,
	Unit_Percent,
	Unit_Semitone,
	Unit_Range
}
Gen_Unit;

void sfont_init_chunks(void);

void sfont_close(SFData * sf);
void sfont_free_zone(SFZone * zone);
int sfont_preset_compare_func(void* a, void* b);

void sfont_zone_delete(SFData * sf, fluid_list_t ** zlist, SFZone * zone);

fluid_list_t *gen_inlist(int gen, fluid_list_t * genlist);
int gen_valid(int gen);
int gen_validp(int gen);

#define CHNKIDSTR(id)           &idlist[(id - 1) * 4]

#define SFPHDRSIZE	38
#define SFBAGSIZE	4
#define SFMODSIZE	10
#define SFGENSIZE	4
#define SFIHDRSIZE	22
#define SFSHDRSIZE	46

typedef struct _SFChunk
{
	unsigned int id;
	unsigned int size;
}
SFChunk;

typedef struct _SFPhdr
{
	unsigned char name[20];
	unsigned short preset;
	unsigned short bank;
	unsigned short pbagndx;
	unsigned int library;
	unsigned int genre;
	unsigned int morphology;
}
SFPhdr;

typedef struct _SFBag
{
	unsigned short genndx;
	unsigned short modndx;
}
SFBag;

typedef struct _SFIhdr
{
	char name[20];
	unsigned short ibagndx;
}
SFIhdr;

typedef struct _SFShdr
{
	char name[20];
	unsigned int start;
	unsigned int end;
	unsigned int loopstart;
	unsigned int loopend;
	unsigned int samplerate;
	unsigned char origpitch;
	signed char pitchadj;
	unsigned short samplelink;
	unsigned short sampletype;
}
SFShdr;

SFData *sfload_file(const char * fname);

#ifndef	FALSE
#define	FALSE	(0)
#endif

#ifndef	TRUE
#define	TRUE	(!FALSE)
#endif

#define GPOINTER_TO_INT(p) ((int)(size_t)(p))
#define GINT_TO_POINTER(i) ((void *)(size_t)(i))

char*	 g_strdup(const char *str);

#if !(defined (G_STMT_START) && defined (G_STMT_END))
#  if defined (__GNUC__) && !defined (__STRICT_ANSI__) && !defined (__cplusplus)
#    define G_STMT_START	(void)(
#    define G_STMT_END		)
#  else
#    if (defined (sun) || defined (__sun__))
#      define G_STMT_START	if (1)
#      define G_STMT_END	else (void)0
#    else
#      define G_STMT_START	do
#      define G_STMT_END	while (0)
#    endif
#  endif
#endif

#define GUINT16_SWAP_LE_BE_CONSTANT(val)	((unsigned short) ( \
    (((unsigned short) (val) & (unsigned short) 0x00ffU) << 8) | \
    (((unsigned short) (val) & (unsigned short) 0xff00U) >> 8)))
#define GUINT32_SWAP_LE_BE_CONSTANT(val)	((unsigned int) ( \
    (((unsigned int) (val) & (unsigned int) 0x000000ffU) << 24) | \
    (((unsigned int) (val) & (unsigned int) 0x0000ff00U) <<  8) | \
    (((unsigned int) (val) & (unsigned int) 0x00ff0000U) >>  8) | \
    (((unsigned int) (val) & (unsigned int) 0xff000000U) >> 24)))

#define GUINT16_SWAP_LE_BE(val) (GUINT16_SWAP_LE_BE_CONSTANT (val))
#define GUINT32_SWAP_LE_BE(val) (GUINT32_SWAP_LE_BE_CONSTANT (val))

#define GINT16_TO_LE(val)	((signed short) (val))
#define GUINT16_TO_LE(val)	((unsigned short) (val))
#define GINT16_TO_BE(val)	((signed short) GUINT16_SWAP_LE_BE (val))
#define GUINT16_TO_BE(val)	(GUINT16_SWAP_LE_BE (val))
#define GINT32_TO_LE(val)	((signed int) (val))
#define GUINT32_TO_LE(val)	((unsigned int) (val))
#define GINT32_TO_BE(val)	((signed int) GUINT32_SWAP_LE_BE (val))
#define GUINT32_TO_BE(val)	(GUINT32_SWAP_LE_BE (val))

#define GINT16_FROM_LE(val)	(GINT16_TO_LE (val))
#define GUINT16_FROM_LE(val)	(GUINT16_TO_LE (val))
#define GINT16_FROM_BE(val)	(GINT16_TO_BE (val))
#define GUINT16_FROM_BE(val)	(GUINT16_TO_BE (val))
#define GINT32_FROM_LE(val)	(GINT32_TO_LE (val))
#define GUINT32_FROM_LE(val)	(GUINT32_TO_LE (val))
#define GINT32_FROM_BE(val)	(GINT32_TO_BE (val))
#define GUINT32_FROM_BE(val)	(GUINT32_TO_BE (val))

#define FAIL	0
#define OK	1

enum
{
	ErrWarn, ErrFatal, ErrStatus, ErrCorr, ErrEof, ErrMem, Errno,
	ErrRead, ErrWrite
};

#define ErrMax		ErrWrite
#define ErrnoStart	Errno
#define ErrnoEnd	ErrWrite

#ifdef FLUID_NO_ERR_OUTPUT
#define FGERR0(a, b)       FAIL
#define FGERR1(a, b, c)    FAIL
#define FGERR2(a, b, c, d) FAIL
#else
#define FGERR0(a, b)       FLUID_LOG(a,b) && FAIL
#define FGERR1(a, b, c)    FLUID_LOG1(a,b,c) && FAIL
#define FGERR2(a, b, c, d) FLUID_LOG2(a,b,c,d) && FAIL
#endif

int safe_fread(void *buf, int count, ZL_RWops* fd);
int safe_fwrite(void *buf, int count, ZL_RWops* fd);
int safe_fseek(ZL_RWops* fd, long ofs, int whence);

typedef struct _fluid_defsfont_t fluid_defsfont_t;
typedef struct _fluid_defpreset_t fluid_defpreset_t;
typedef struct _fluid_preset_zone_t fluid_preset_zone_t;
typedef struct _fluid_inst_t fluid_inst_t;
typedef struct _fluid_inst_zone_t fluid_inst_zone_t;

//fluid_sfloader_t* new_fluid_defsfloader(void);
int delete_fluid_defsfloader(fluid_sfloader_t* loader);
fluid_sfont_t* fluid_defsfloader_load(fluid_sfloader_t* loader, const char* filename);

int fluid_defsfont_sfont_delete(fluid_sfont_t* sfont);
char* fluid_defsfont_sfont_get_name(fluid_sfont_t* sfont);
fluid_preset_t* fluid_defsfont_sfont_get_preset(fluid_sfont_t* sfont, unsigned int bank, unsigned int prenum);
void fluid_defsfont_sfont_iteration_start(fluid_sfont_t* sfont);
int fluid_defsfont_sfont_iteration_next(fluid_sfont_t* sfont, fluid_preset_t* preset);

int fluid_defpreset_preset_delete(fluid_preset_t* preset);
char* fluid_defpreset_preset_get_name(fluid_preset_t* preset);
int fluid_defpreset_preset_get_banknum(fluid_preset_t* preset);
int fluid_defpreset_preset_get_num(fluid_preset_t* preset);
int fluid_defpreset_preset_noteon(fluid_preset_t* preset, fluid_synth_t* synth, int chan, int key, int vel);

struct _fluid_defsfont_t
{
	ZL_FileLink flink;
	unsigned int samplepos;
	unsigned int samplesize;
	short* sampledata;
	fluid_list_t* sample;
	fluid_defpreset_t* preset;

	fluid_preset_t iter_preset;
	fluid_defpreset_t* iter_cur;
};

fluid_defsfont_t* new_fluid_defsfont(void);
int delete_fluid_defsfont(fluid_defsfont_t* sfont);
int fluid_defsfont_load(fluid_defsfont_t* sfont, const char* file);
char* fluid_defsfont_get_name(fluid_defsfont_t* sfont);
fluid_defpreset_t* fluid_defsfont_get_preset(fluid_defsfont_t* sfont, unsigned int bank, unsigned int prenum);
void fluid_defsfont_iteration_start(fluid_defsfont_t* sfont);
int fluid_defsfont_iteration_next(fluid_defsfont_t* sfont, fluid_preset_t* preset);
int fluid_defsfont_load_sampledata(fluid_defsfont_t* sfont, ZL_RWops* fd);
int fluid_defsfont_add_sample(fluid_defsfont_t* sfont, fluid_sample_t* sample);
int fluid_defsfont_add_preset(fluid_defsfont_t* sfont, fluid_defpreset_t* preset);
fluid_sample_t* fluid_defsfont_get_sample(fluid_defsfont_t* sfont, char *s);

struct _fluid_defpreset_t
{
	fluid_defpreset_t* next;
	fluid_defsfont_t* sfont;
	char name[21];
	unsigned int bank;
	unsigned int num;
	fluid_preset_zone_t* global_zone;
	fluid_preset_zone_t* zone;
};

fluid_defpreset_t* new_fluid_defpreset(fluid_defsfont_t* sfont);
int delete_fluid_defpreset(fluid_defpreset_t* preset);
fluid_defpreset_t* fluid_defpreset_next(fluid_defpreset_t* preset);
int fluid_defpreset_import_sfont(fluid_defpreset_t* preset, SFPreset* sfpreset, fluid_defsfont_t* sfont);
int fluid_defpreset_set_global_zone(fluid_defpreset_t* preset, fluid_preset_zone_t* zone);
int fluid_defpreset_add_zone(fluid_defpreset_t* preset, fluid_preset_zone_t* zone);
fluid_preset_zone_t* fluid_defpreset_get_zone(fluid_defpreset_t* preset);
fluid_preset_zone_t* fluid_defpreset_get_global_zone(fluid_defpreset_t* preset);
int fluid_defpreset_get_banknum(fluid_defpreset_t* preset);
int fluid_defpreset_get_num(fluid_defpreset_t* preset);
char* fluid_defpreset_get_name(fluid_defpreset_t* preset);
int fluid_defpreset_noteon(fluid_defpreset_t* preset, fluid_synth_t* synth, int chan, int key, int vel);

struct _fluid_preset_zone_t
{
	fluid_preset_zone_t* next;
	char* name;
	fluid_inst_t* inst;
	int keylo;
	int keyhi;
	int vello;
	int velhi;
	fluid_gen_t gen[GEN_LAST];
	fluid_mod_t * mod;
};

fluid_preset_zone_t* new_fluid_preset_zone(char* name);
int delete_fluid_preset_zone(fluid_preset_zone_t* zone);
fluid_preset_zone_t* fluid_preset_zone_next(fluid_preset_zone_t* preset);
int fluid_preset_zone_import_sfont(fluid_preset_zone_t* zone, SFZone* sfzone, fluid_defsfont_t* sfont);
int fluid_preset_zone_inside_range(fluid_preset_zone_t* zone, int key, int vel);
fluid_inst_t* fluid_preset_zone_get_inst(fluid_preset_zone_t* zone);

struct _fluid_inst_t
{
	char name[21];
	fluid_inst_zone_t* global_zone;
	fluid_inst_zone_t* zone;
};

fluid_inst_t* new_fluid_inst(void);
int delete_fluid_inst(fluid_inst_t* inst);
int fluid_inst_import_sfont(fluid_inst_t* inst, SFInst *sfinst, fluid_defsfont_t* sfont);
int fluid_inst_set_global_zone(fluid_inst_t* inst, fluid_inst_zone_t* zone);
int fluid_inst_add_zone(fluid_inst_t* inst, fluid_inst_zone_t* zone);
fluid_inst_zone_t* fluid_inst_get_zone(fluid_inst_t* inst);
fluid_inst_zone_t* fluid_inst_get_global_zone(fluid_inst_t* inst);

struct _fluid_inst_zone_t
{
	fluid_inst_zone_t* next;
	char* name;
	fluid_sample_t* sample;
	int keylo;
	int keyhi;
	int vello;
	int velhi;
	fluid_gen_t gen[GEN_LAST];
	fluid_mod_t * mod;
};

fluid_inst_zone_t* new_fluid_inst_zone(char* name);
int delete_fluid_inst_zone(fluid_inst_zone_t* zone);
fluid_inst_zone_t* fluid_inst_zone_next(fluid_inst_zone_t* zone);
int fluid_inst_zone_import_sfont(fluid_inst_zone_t* zone, SFZone *sfzone, fluid_defsfont_t* sfont);
int fluid_inst_zone_inside_range(fluid_inst_zone_t* zone, int key, int vel);
fluid_sample_t* fluid_inst_zone_get_sample(fluid_inst_zone_t* zone);

fluid_sample_t* new_fluid_sample(void);
int delete_fluid_sample(fluid_sample_t* sample);
int fluid_sample_import_sfont(fluid_sample_t* sample, SFSample* sfsample, fluid_defsfont_t* sfont);
int fluid_sample_in_rom(fluid_sample_t* sample);

#endif

SFData * sfload_zlfile(ZL_RWops *fd);

int fluid_zlsfont_log(int level, const char* fmt, ...)
{
	char fluid_errbuf[512];
	va_list args;
	va_start(args, fmt);
	vsnprintf(fluid_errbuf, sizeof(fluid_errbuf), fmt, args);
	va_end(args);
	ZL_Application::Log("FLUID", fluid_errbuf);
	return 0;
}

fluid_sfloader_t* new_fluid_defsfloader()
{
	fluid_sfloader_t* loader;

	loader = FLUID_NEW(fluid_sfloader_t);
	if (loader == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return NULL;
	}

	loader->data = NULL;
	loader->free = delete_fluid_defsfloader;
	loader->load = fluid_defsfloader_load;

	return loader;
}

int delete_fluid_defsfloader(fluid_sfloader_t* loader)
{
	if (loader) {
		FLUID_FREE(loader);
	}
	return FLUID_OK;
}

fluid_sfont_t* fluid_defsfloader_load(fluid_sfloader_t* loader, const char* filename)
{
	fluid_defsfont_t* defsfont;
	fluid_sfont_t* sfont;

	defsfont = new_fluid_defsfont();

	if (defsfont == NULL) {
		return NULL;
	}

	if (fluid_defsfont_load(defsfont, filename) == FLUID_FAILED) {
		delete_fluid_defsfont(defsfont);
		return NULL;
	}

	sfont = FLUID_NEW(fluid_sfont_t);
	if (sfont == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return NULL;
	}

	sfont->data = defsfont;
	sfont->free = fluid_defsfont_sfont_delete;
	sfont->get_name = fluid_defsfont_sfont_get_name;
	sfont->get_preset = fluid_defsfont_sfont_get_preset;
	sfont->iteration_start = fluid_defsfont_sfont_iteration_start;
	sfont->iteration_next = fluid_defsfont_sfont_iteration_next;

	return sfont;
}

int fluid_defsfont_sfont_delete(fluid_sfont_t* sfont)
{
	if (delete_fluid_defsfont((fluid_defsfont_t*)sfont->data) != 0) {
		return -1;
	}
	FLUID_FREE(sfont);
	return 0;
}

char* fluid_defsfont_sfont_get_name(fluid_sfont_t* sfont)
{
	return fluid_defsfont_get_name((fluid_defsfont_t*)sfont->data);
}

fluid_preset_t* fluid_defsfont_sfont_get_preset(fluid_sfont_t* sfont, unsigned int bank, unsigned int prenum)
{
	fluid_preset_t* preset;
	fluid_defpreset_t* defpreset;

	defpreset = fluid_defsfont_get_preset((fluid_defsfont_t*)sfont->data, bank, prenum);

	if (defpreset == NULL) {
		return NULL;
	}

	preset = FLUID_NEW(fluid_preset_t);
	if (preset == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return NULL;
	}

	preset->sfont = sfont;
	preset->data = defpreset;
	preset->free = fluid_defpreset_preset_delete;
	preset->get_name = fluid_defpreset_preset_get_name;
	preset->get_banknum = fluid_defpreset_preset_get_banknum;
	preset->get_num = fluid_defpreset_preset_get_num;
	preset->noteon = fluid_defpreset_preset_noteon;
	preset->notify = NULL;

	return preset;
}

void fluid_defsfont_sfont_iteration_start(fluid_sfont_t* sfont)
{
	fluid_defsfont_iteration_start((fluid_defsfont_t*)sfont->data);
}

int fluid_defsfont_sfont_iteration_next(fluid_sfont_t* sfont, fluid_preset_t* preset)
{
	preset->free = fluid_defpreset_preset_delete;
	preset->get_name = fluid_defpreset_preset_get_name;
	preset->get_banknum = fluid_defpreset_preset_get_banknum;
	preset->get_num = fluid_defpreset_preset_get_num;
	preset->noteon = fluid_defpreset_preset_noteon;
	preset->notify = NULL;

	return fluid_defsfont_iteration_next((fluid_defsfont_t*)sfont->data, preset);
}

int fluid_defpreset_preset_delete(fluid_preset_t* preset)
{
	FLUID_FREE(preset);

	return 0;
}

char* fluid_defpreset_preset_get_name(fluid_preset_t* preset)
{
	return fluid_defpreset_get_name((fluid_defpreset_t*)preset->data);
}

int fluid_defpreset_preset_get_banknum(fluid_preset_t* preset)
{
	return fluid_defpreset_get_banknum((fluid_defpreset_t*)preset->data);
}

int fluid_defpreset_preset_get_num(fluid_preset_t* preset)
{
	return fluid_defpreset_get_num((fluid_defpreset_t*)preset->data);
}

int fluid_defpreset_preset_noteon(fluid_preset_t* preset, fluid_synth_t* synth,
	int chan, int key, int vel)
{
	return fluid_defpreset_noteon((fluid_defpreset_t*)preset->data, synth, chan, key, vel);
}

fluid_defsfont_t* new_fluid_defsfont()
{
	fluid_defsfont_t* sfont;

	sfont = FLUID_NEW(fluid_defsfont_t);
	if (sfont == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return NULL;
	}

	memset(sfont, 0, sizeof(fluid_defsfont_t));

	return sfont;
}

int delete_fluid_defsfont(fluid_defsfont_t* sfont)
{
	fluid_list_t *list;
	fluid_defpreset_t* preset;
	fluid_sample_t* sample;

	for (list = sfont->sample; list; list = fluid_list_next(list)) {
		sample = (fluid_sample_t*)fluid_list_get(list);
		if (fluid_sample_refcount(sample) != 0) {
			return -1;
		}
	}

	sfont->flink = ZL_FileLink();

	for (list = sfont->sample; list; list = fluid_list_next(list)) {
		delete_fluid_sample((fluid_sample_t*)fluid_list_get(list));
	}

	if (sfont->sample) {
		delete_fluid_list(sfont->sample);
	}

	if (sfont->sampledata != NULL) {
		fluid_munlock(sfont->sampledata, sfont->samplesize);
		FLUID_FREE(sfont->sampledata);
	}

	preset = sfont->preset;
	while (preset != NULL) {
		sfont->preset = preset->next;
		delete_fluid_defpreset(preset);
		preset = sfont->preset;
	}

	FLUID_FREE(sfont);
	return FLUID_OK;
}

char* fluid_defsfont_get_name(fluid_defsfont_t* sfont)
{
	return NULL;
}

int fluid_defsfont_load(fluid_defsfont_t* sfont, const char* _file)
{
	SFData* sfdata;
	fluid_list_t *p;
	SFPreset* sfpreset;
	SFSample* sfsample;
	fluid_sample_t* sample;
	fluid_defpreset_t* preset;
	ZL_File *file = (ZL_File*)_file;

	sfont->flink = ZL_FileLink(*file);
	ZL_File_Impl* fileimpl = ZL_ImplFromOwner<ZL_File_Impl>(*file);

	if (!fileimpl)
	{
		FLUID_LOG1(FLUID_ERR, _("Unable to open file %s"), fileimpl->filename.c_str());
		return FLUID_FAILED;
	}

	ZL_RWops* fd = fileimpl->src;

	sfdata = sfload_zlfile(fd);
	if (sfdata == NULL) {
		FLUID_LOG(FLUID_ERR, "Couldn't load soundfont file");
		return FLUID_FAILED;
	}

	sfont->samplepos = sfdata->samplepos;
	sfont->samplesize = sfdata->samplesize;

	if (fluid_defsfont_load_sampledata(sfont, fileimpl->src) != FLUID_OK)
		goto err_exit;

	p = sfdata->sample;
	while (p != NULL) {
		sfsample = (SFSample *)p->data;

		sample = new_fluid_sample();
		if (sample == NULL) goto err_exit;

		if (fluid_sample_import_sfont(sample, sfsample, sfont) != FLUID_OK)
			goto err_exit;

		fluid_defsfont_add_sample(sfont, sample);
		fluid_voice_optimize_sample(sample);
		p = fluid_list_next(p);
	}

	p = sfdata->preset;
	while (p != NULL) {
		sfpreset = (SFPreset *)p->data;
		preset = new_fluid_defpreset(sfont);
		if (preset == NULL) goto err_exit;

		if (fluid_defpreset_import_sfont(preset, sfpreset, sfont) != FLUID_OK)
			goto err_exit;

		fluid_defsfont_add_preset(sfont, preset);
		p = fluid_list_next(p);
	}
	sfont_close(sfdata);

	return FLUID_OK;

err_exit:
	sfont_close(sfdata);
	return FLUID_FAILED;
}

int fluid_defsfont_add_sample(fluid_defsfont_t* sfont, fluid_sample_t* sample)
{
	sfont->sample = fluid_list_append(sfont->sample, sample);
	return FLUID_OK;
}

int fluid_defsfont_add_preset(fluid_defsfont_t* sfont, fluid_defpreset_t* preset)
{
	fluid_defpreset_t *cur, *prev;
	if (sfont->preset == NULL) {
		preset->next = NULL;
		sfont->preset = preset;
	}
	else {
		cur = sfont->preset;
		prev = NULL;
		while (cur != NULL) {
			if ((preset->bank < cur->bank)
				|| ((preset->bank == cur->bank) && (preset->num < cur->num))) {
				if (prev == NULL) {
					preset->next = cur;
					sfont->preset = preset;
				}
				else {
					preset->next = cur;
					prev->next = preset;
				}
				return FLUID_OK;
			}
			prev = cur;
			cur = cur->next;
		}
		preset->next = NULL;
		prev->next = preset;
	}
	return FLUID_OK;
}

int fluid_defsfont_load_sampledata(fluid_defsfont_t* sfont, ZL_RWops* fd)
{
	unsigned short endian;

	if (fd->seektell(sfont->samplepos, RW_SEEK_SET) == -1) {
		perror("error");
		FLUID_LOG(FLUID_ERR, "Failed to seek position in data file");
		return FLUID_FAILED;
	}
	sfont->sampledata = (short*)FLUID_MALLOC(sfont->samplesize);
	if (sfont->sampledata == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return FLUID_FAILED;
	}
	if (fd->read(sfont->sampledata, 1, sfont->samplesize) < sfont->samplesize) {
		FLUID_LOG(FLUID_ERR, "Failed to read sample data");
		return FLUID_FAILED;
	}

	if (fluid_mlock(sfont->sampledata, sfont->samplesize) != 0) {
		FLUID_LOG(FLUID_WARN, "Failed to pin the sample data to RAM; swapping is possible.");
	}

	endian = 0x0100;

	if (((char *)&endian)[0]) {
		unsigned char* cbuf;
		unsigned char hi, lo;
		unsigned int i, j;
		short s;
		cbuf = (unsigned char*)sfont->sampledata;
		for (i = 0, j = 0; j < sfont->samplesize; i++) {
			lo = cbuf[j++];
			hi = cbuf[j++];
			s = (hi << 8) | lo;
			sfont->sampledata[i] = s;
		}
	}
	return FLUID_OK;
}

fluid_sample_t* fluid_defsfont_get_sample(fluid_defsfont_t* sfont, char *s)
{
	fluid_list_t* list;
	fluid_sample_t* sample;

	for (list = sfont->sample; list; list = fluid_list_next(list)) {
		sample = (fluid_sample_t*)fluid_list_get(list);

		if (FLUID_STRCMP(sample->name, s) == 0) {
			return sample;
		}
	}

	return NULL;
}

fluid_defpreset_t* fluid_defsfont_get_preset(fluid_defsfont_t* sfont, unsigned int bank, unsigned int num)
{
	fluid_defpreset_t* preset = sfont->preset;
	while (preset != NULL) {
		if ((preset->bank == bank) && ((preset->num == num))) {
			return preset;
		}
		preset = preset->next;
	}
	return NULL;
}

void fluid_defsfont_iteration_start(fluid_defsfont_t* sfont)
{
	sfont->iter_cur = sfont->preset;
}

int fluid_defsfont_iteration_next(fluid_defsfont_t* sfont, fluid_preset_t* preset)
{
	if (sfont->iter_cur == NULL) {
		return 0;
	}

	preset->data = (void*)sfont->iter_cur;
	sfont->iter_cur = fluid_defpreset_next(sfont->iter_cur);
	return 1;
}

fluid_defpreset_t*
new_fluid_defpreset(fluid_defsfont_t* sfont)
{
	fluid_defpreset_t* preset = FLUID_NEW(fluid_defpreset_t);
	if (preset == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return NULL;
	}
	preset->next = NULL;
	preset->sfont = sfont;
	preset->name[0] = 0;
	preset->bank = 0;
	preset->num = 0;
	preset->global_zone = NULL;
	preset->zone = NULL;
	return preset;
}

int delete_fluid_defpreset(fluid_defpreset_t* preset)
{
	int err = FLUID_OK;
	fluid_preset_zone_t* zone;
	if (preset->global_zone != NULL) {
		if (delete_fluid_preset_zone(preset->global_zone) != FLUID_OK) {
			err = FLUID_FAILED;
		}
		preset->global_zone = NULL;
	}
	zone = preset->zone;
	while (zone != NULL) {
		preset->zone = zone->next;
		if (delete_fluid_preset_zone(zone) != FLUID_OK) {
			err = FLUID_FAILED;
		}
		zone = preset->zone;
	}
	FLUID_FREE(preset);
	return err;
}

int fluid_defpreset_get_banknum(fluid_defpreset_t* preset)
{
	return preset->bank;
}

int fluid_defpreset_get_num(fluid_defpreset_t* preset)
{
	return preset->num;
}

char*
fluid_defpreset_get_name(fluid_defpreset_t* preset)
{
	return preset->name;
}

fluid_defpreset_t*
fluid_defpreset_next(fluid_defpreset_t* preset)
{
	return preset->next;
}

int fluid_defpreset_noteon(fluid_defpreset_t* preset, fluid_synth_t* synth, int chan, int key, int vel)
{
	fluid_preset_zone_t *preset_zone, *global_preset_zone;
	fluid_inst_t* inst;
	fluid_inst_zone_t *inst_zone, *global_inst_zone, *z; (void)z;
	fluid_sample_t* sample;
	fluid_voice_t* voice;
	fluid_mod_t * mod;
	fluid_mod_t * mod_list[FLUID_NUM_MOD];
	int mod_list_count;
	int i;

	unsigned int start_time = synth->ticks;

	global_preset_zone = fluid_defpreset_get_global_zone(preset);

	preset_zone = fluid_defpreset_get_zone(preset);
	while (preset_zone != NULL) {
		if (fluid_preset_zone_inside_range(preset_zone, key, vel)) {
			inst = fluid_preset_zone_get_inst(preset_zone);
			global_inst_zone = fluid_inst_get_global_zone(inst);

			inst_zone = fluid_inst_get_zone(inst);
			while (inst_zone != NULL) {
				sample = fluid_inst_zone_get_sample(inst_zone);
				if (fluid_sample_in_rom(sample) || (sample == NULL)) {
					inst_zone = fluid_inst_zone_next(inst_zone);
					continue;
				}

				if (fluid_inst_zone_inside_range(inst_zone, key, vel) && (sample != NULL)) {
					voice = fluid_synth_alloc_voice(synth, sample, chan, key, vel, start_time);
					if (voice == NULL) {
						return FLUID_FAILED;
					}

					z = inst_zone;

					for (i = 0; i < GEN_LAST; i++) {
						if (inst_zone->gen[i].flags){
							fluid_voice_gen_set(voice, i, inst_zone->gen[i].val);

						}
						else if ((global_inst_zone != NULL) && (global_inst_zone->gen[i].flags)) {
							fluid_voice_gen_set(voice, i, global_inst_zone->gen[i].val);

						}
						else {
						}

					}

					mod_list_count = 0;

					if (global_inst_zone){
						mod = global_inst_zone->mod;
						while (mod){
							mod_list[mod_list_count++] = mod;
							mod = mod->next;
						}
					}

					mod = inst_zone->mod;

					while (mod){
						for (i = 0; i < mod_list_count; i++){
							if (mod_list[i] && fluid_mod_test_identity(mod, mod_list[i])){
								mod_list[i] = NULL;
							}
						}

						mod_list[mod_list_count++] = mod;
						mod = mod->next;
					}

					for (i = 0; i < mod_list_count; i++){
						mod = mod_list[i];

						if (mod != NULL){
							fluid_voice_add_mod(voice, mod, FLUID_VOICE_OVERWRITE);
						}
					}

					for (i = 0; i < GEN_LAST; i++) {
						if ((i != GEN_STARTADDROFS)
							&& (i != GEN_ENDADDROFS)
							&& (i != GEN_STARTLOOPADDROFS)
							&& (i != GEN_ENDLOOPADDROFS)
							&& (i != GEN_STARTADDRCOARSEOFS)
							&& (i != GEN_ENDADDRCOARSEOFS)
							&& (i != GEN_STARTLOOPADDRCOARSEOFS)
							&& (i != GEN_KEYNUM)
							&& (i != GEN_VELOCITY)
							&& (i != GEN_ENDLOOPADDRCOARSEOFS)
							&& (i != GEN_SAMPLEMODE)
							&& (i != GEN_EXCLUSIVECLASS)
							&& (i != GEN_OVERRIDEROOTKEY)) {
							if (preset_zone->gen[i].flags) {
								fluid_voice_gen_incr(voice, i, preset_zone->gen[i].val);
							}
							else if ((global_preset_zone != NULL) && global_preset_zone->gen[i].flags) {
								fluid_voice_gen_incr(voice, i, global_preset_zone->gen[i].val);
							}
							else {
							}
						}
					}

					mod_list_count = 0;
					if (global_preset_zone){
						mod = global_preset_zone->mod;
						while (mod){
							mod_list[mod_list_count++] = mod;
							mod = mod->next;
						}
					}

					mod = preset_zone->mod;
					while (mod){
						for (i = 0; i < mod_list_count; i++){
							if (mod_list[i] && fluid_mod_test_identity(mod, mod_list[i])){
								mod_list[i] = NULL;
							}
						}

						mod_list[mod_list_count++] = mod;
						mod = mod->next;
					}

					for (i = 0; i < mod_list_count; i++){
						mod = mod_list[i];
						if ((mod != NULL) && (mod->amount != 0)) {
							fluid_voice_add_mod(voice, mod, FLUID_VOICE_ADD);
						}
					}

					fluid_synth_start_voice(synth, voice);

				}

				inst_zone = fluid_inst_zone_next(inst_zone);
			}
		}
		preset_zone = fluid_preset_zone_next(preset_zone);
	}

	return FLUID_OK;
}

int fluid_defpreset_set_global_zone(fluid_defpreset_t* preset, fluid_preset_zone_t* zone)
{
	preset->global_zone = zone;
	return FLUID_OK;
}

int fluid_defpreset_import_sfont(fluid_defpreset_t* preset,
SFPreset* sfpreset,
fluid_defsfont_t* sfont)
{
	fluid_list_t *p;
	SFZone* sfzone;
	fluid_preset_zone_t* zone;
	int count;
	char zone_name[256];
	if (/*(sfpreset->name != NULL) && */(FLUID_STRLEN(sfpreset->name) > 0)) {
		FLUID_STRCPY(preset->name, sfpreset->name);
	}
	else {
		FLUID_SPRINTF(preset->name, "Bank%d,Preset%d", sfpreset->bank, sfpreset->prenum);
	}
	preset->bank = sfpreset->bank;
	preset->num = sfpreset->prenum;
	p = sfpreset->zone;
	count = 0;
	while (p != NULL) {
		sfzone = (SFZone *)p->data;
		FLUID_SPRINTF(zone_name, "%s/%d", preset->name, count);
		zone = new_fluid_preset_zone(zone_name);
		if (zone == NULL) {
			return FLUID_FAILED;
		}
		if (fluid_preset_zone_import_sfont(zone, sfzone, sfont) != FLUID_OK) {
			return FLUID_FAILED;
		}
		if ((count == 0) && (fluid_preset_zone_get_inst(zone) == NULL)) {
			fluid_defpreset_set_global_zone(preset, zone);
		}
		else if (fluid_defpreset_add_zone(preset, zone) != FLUID_OK) {
			return FLUID_FAILED;
		}
		p = fluid_list_next(p);
		count++;
	}
	return FLUID_OK;
}

int fluid_defpreset_add_zone(fluid_defpreset_t* preset, fluid_preset_zone_t* zone)
{
	if (preset->zone == NULL) {
		zone->next = NULL;
		preset->zone = zone;
	}
	else {
		zone->next = preset->zone;
		preset->zone = zone;
	}
	return FLUID_OK;
}

fluid_preset_zone_t*
fluid_defpreset_get_zone(fluid_defpreset_t* preset)
{
	return preset->zone;
}

fluid_preset_zone_t*
fluid_defpreset_get_global_zone(fluid_defpreset_t* preset)
{
	return preset->global_zone;
}

fluid_preset_zone_t*
fluid_preset_zone_next(fluid_preset_zone_t* preset)
{
	return preset->next;
}

fluid_preset_zone_t*
new_fluid_preset_zone(char *name)
{
	int size;
	fluid_preset_zone_t* zone = NULL;
	zone = FLUID_NEW(fluid_preset_zone_t);
	if (zone == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return NULL;
	}
	zone->next = NULL;
	size = 1 + (int)FLUID_STRLEN(name);
	zone->name = (char*)FLUID_MALLOC(size);
	if (zone->name == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		FLUID_FREE(zone);
		return NULL;
	}
	FLUID_STRCPY(zone->name, name);
	zone->inst = NULL;
	zone->keylo = 0;
	zone->keyhi = 128;
	zone->vello = 0;
	zone->velhi = 128;

	fluid_gen_set_default_values(&zone->gen[0]);
	zone->mod = NULL;
	return zone;
}

int delete_fluid_preset_zone(fluid_preset_zone_t* zone)
{
	fluid_mod_t *mod, *tmp;

	mod = zone->mod;
	while (mod)
	{
		tmp = mod;
		mod = mod->next;
		fluid_mod_delete(tmp);
	}

	if (zone->name) FLUID_FREE(zone->name);
	if (zone->inst) delete_fluid_inst(zone->inst);
	FLUID_FREE(zone);
	return FLUID_OK;
}

int fluid_preset_zone_import_sfont(fluid_preset_zone_t* zone, SFZone *sfzone, fluid_defsfont_t* sfont)
{
	fluid_list_t *r;
	SFGen* sfgen;
	int count;
	for (count = 0, r = sfzone->gen; r != NULL; count++) {
		sfgen = (SFGen *)r->data;
		switch (sfgen->id) {
			case GEN_KEYRANGE:
				zone->keylo = (int)sfgen->amount.range.lo;
				zone->keyhi = (int)sfgen->amount.range.hi;
				break;
			case GEN_VELRANGE:
				zone->vello = (int)sfgen->amount.range.lo;
				zone->velhi = (int)sfgen->amount.range.hi;
				break;
			default:
				zone->gen[sfgen->id].val = (fluid_real_t)sfgen->amount.sword;
				zone->gen[sfgen->id].flags = GEN_SET;
				break;
		}
		r = fluid_list_next(r);
	}
	if ((sfzone->instsamp != NULL) && (sfzone->instsamp->data != NULL)) {
		zone->inst = (fluid_inst_t*)new_fluid_inst();
		if (zone->inst == NULL) {
			FLUID_LOG(FLUID_ERR, "Out of memory");
			return FLUID_FAILED;
		}
		if (fluid_inst_import_sfont(zone->inst, (SFInst *)sfzone->instsamp->data, sfont) != FLUID_OK) {
			return FLUID_FAILED;
		}
	}

	for (count = 0, r = sfzone->mod; r != NULL; count++) {
		SFMod* mod_src = (SFMod *)r->data;
		fluid_mod_t * mod_dest = fluid_mod_new();
		int type;

		if (mod_dest == NULL){
			return FLUID_FAILED;
		}
		mod_dest->next = NULL;

		mod_dest->amount = mod_src->amount;

		mod_dest->src1 = mod_src->src & 127;
		mod_dest->flags1 = 0;

		if (mod_src->src & (1 << 7)){
			mod_dest->flags1 |= FLUID_MOD_CC;
		}
		else {
			mod_dest->flags1 |= FLUID_MOD_GC;
		}

		if (mod_src->src & (1 << 8)){
			mod_dest->flags1 |= FLUID_MOD_NEGATIVE;
		}
		else {
			mod_dest->flags1 |= FLUID_MOD_POSITIVE;
		}

		if (mod_src->src & (1 << 9)){
			mod_dest->flags1 |= FLUID_MOD_BIPOLAR;
		}
		else {
			mod_dest->flags1 |= FLUID_MOD_UNIPOLAR;
		}

		type = (mod_src->src) >> 10;
		type &= 63;
		if (type == 0){
			mod_dest->flags1 |= FLUID_MOD_LINEAR;
		}
		else if (type == 1){
			mod_dest->flags1 |= FLUID_MOD_CONCAVE;
		}
		else if (type == 2){
			mod_dest->flags1 |= FLUID_MOD_CONVEX;
		}
		else if (type == 3){
			mod_dest->flags1 |= FLUID_MOD_SWITCH;
		}
		else {
			mod_dest->amount = 0;
		}

		mod_dest->dest = mod_src->dest;

		mod_dest->src2 = mod_src->amtsrc & 127;
		mod_dest->flags2 = 0;

		if (mod_src->amtsrc & (1 << 7)){
			mod_dest->flags2 |= FLUID_MOD_CC;
		}
		else {
			mod_dest->flags2 |= FLUID_MOD_GC;
		}

		if (mod_src->amtsrc & (1 << 8)){
			mod_dest->flags2 |= FLUID_MOD_NEGATIVE;
		}
		else {
			mod_dest->flags2 |= FLUID_MOD_POSITIVE;
		}

		if (mod_src->amtsrc & (1 << 9)){
			mod_dest->flags2 |= FLUID_MOD_BIPOLAR;
		}
		else {
			mod_dest->flags2 |= FLUID_MOD_UNIPOLAR;
		}

		type = (mod_src->amtsrc) >> 10;
		type &= 63;
		if (type == 0){
			mod_dest->flags2 |= FLUID_MOD_LINEAR;
		}
		else if (type == 1){
			mod_dest->flags2 |= FLUID_MOD_CONCAVE;
		}
		else if (type == 2){
			mod_dest->flags2 |= FLUID_MOD_CONVEX;
		}
		else if (type == 3){
			mod_dest->flags2 |= FLUID_MOD_SWITCH;
		}
		else {
			mod_dest->amount = 0;
		}

		if (mod_src->trans != 0){
			mod_dest->amount = 0;
		}

		if (count == 0){
			zone->mod = mod_dest;
		}
		else {
			fluid_mod_t * last_mod = zone->mod;

			while (last_mod->next != NULL){
				last_mod = last_mod->next;
			}

			last_mod->next = mod_dest;
		}

		r = fluid_list_next(r);
	}

	return FLUID_OK;
}

fluid_inst_t*
fluid_preset_zone_get_inst(fluid_preset_zone_t* zone)
{
	return zone->inst;
}

int fluid_preset_zone_inside_range(fluid_preset_zone_t* zone, int key, int vel)
{
	return ((zone->keylo <= key) &&
		(zone->keyhi >= key) &&
		(zone->vello <= vel) &&
		(zone->velhi >= vel));
}

fluid_inst_t*
new_fluid_inst()
{
	fluid_inst_t* inst = FLUID_NEW(fluid_inst_t);
	if (inst == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return NULL;
	}
	inst->name[0] = 0;
	inst->global_zone = NULL;
	inst->zone = NULL;
	return inst;
}

int delete_fluid_inst(fluid_inst_t* inst)
{
	fluid_inst_zone_t* zone;
	int err = FLUID_OK;
	if (inst->global_zone != NULL) {
		if (delete_fluid_inst_zone(inst->global_zone) != FLUID_OK) {
			err = FLUID_FAILED;
		}
		inst->global_zone = NULL;
	}
	zone = inst->zone;
	while (zone != NULL) {
		inst->zone = zone->next;
		if (delete_fluid_inst_zone(zone) != FLUID_OK) {
			err = FLUID_FAILED;
		}
		zone = inst->zone;
	}
	FLUID_FREE(inst);
	return err;
}

int fluid_inst_set_global_zone(fluid_inst_t* inst, fluid_inst_zone_t* zone)
{
	inst->global_zone = zone;
	return FLUID_OK;
}

int fluid_inst_import_sfont(fluid_inst_t* inst, SFInst *sfinst, fluid_defsfont_t* sfont)
{
	fluid_list_t *p;
	SFZone* sfzone;
	fluid_inst_zone_t* zone;
	char zone_name[256];
	int count;

	p = sfinst->zone;
	if (/*(sfinst->name != NULL) && */(FLUID_STRLEN(sfinst->name) > 0)) {
		FLUID_STRCPY(inst->name, sfinst->name);
	}
	else {
		FLUID_STRCPY(inst->name, "<untitled>");
	}

	count = 0;
	while (p != NULL) {
		sfzone = (SFZone *)p->data;
		FLUID_SPRINTF(zone_name, "%s/%d", inst->name, count);

		zone = new_fluid_inst_zone(zone_name);
		if (zone == NULL) {
			return FLUID_FAILED;
		}

		if (fluid_inst_zone_import_sfont(zone, sfzone, sfont) != FLUID_OK) {
			return FLUID_FAILED;
		}

		if ((count == 0) && (fluid_inst_zone_get_sample(zone) == NULL)) {
			fluid_inst_set_global_zone(inst, zone);

		}
		else if (fluid_inst_add_zone(inst, zone) != FLUID_OK) {
			return FLUID_FAILED;
		}

		p = fluid_list_next(p);
		count++;
	}
	return FLUID_OK;
}

int fluid_inst_add_zone(fluid_inst_t* inst, fluid_inst_zone_t* zone)
{
	if (inst->zone == NULL) {
		zone->next = NULL;
		inst->zone = zone;
	}
	else {
		zone->next = inst->zone;
		inst->zone = zone;
	}
	return FLUID_OK;
}

fluid_inst_zone_t*
fluid_inst_get_zone(fluid_inst_t* inst)
{
	return inst->zone;
}

fluid_inst_zone_t*
fluid_inst_get_global_zone(fluid_inst_t* inst)
{
	return inst->global_zone;
}

fluid_inst_zone_t*
new_fluid_inst_zone(char* name)
{
	int size;
	fluid_inst_zone_t* zone = NULL;
	zone = FLUID_NEW(fluid_inst_zone_t);
	if (zone == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return NULL;
	}
	zone->next = NULL;
	size = 1 + (int)FLUID_STRLEN(name);
	zone->name = (char*)FLUID_MALLOC(size);
	if (zone->name == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		FLUID_FREE(zone);
		return NULL;
	}
	FLUID_STRCPY(zone->name, name);
	zone->sample = NULL;
	zone->keylo = 0;
	zone->keyhi = 128;
	zone->vello = 0;
	zone->velhi = 128;

	fluid_gen_set_default_values(&zone->gen[0]);
	zone->mod = NULL;
	return zone;
}

int delete_fluid_inst_zone(fluid_inst_zone_t* zone)
{
	fluid_mod_t *mod, *tmp;

	mod = zone->mod;
	while (mod)
	{
		tmp = mod;
		mod = mod->next;
		fluid_mod_delete(tmp);
	}

	if (zone->name) FLUID_FREE(zone->name);
	FLUID_FREE(zone);
	return FLUID_OK;
}

fluid_inst_zone_t*
fluid_inst_zone_next(fluid_inst_zone_t* zone)
{
	return zone->next;
}

int fluid_inst_zone_import_sfont(fluid_inst_zone_t* zone, SFZone *sfzone, fluid_defsfont_t* sfont)
{
	fluid_list_t *r;
	SFGen* sfgen;
	int count;

	for (count = 0, r = sfzone->gen; r != NULL; count++) {
		sfgen = (SFGen *)r->data;
		switch (sfgen->id) {
			case GEN_KEYRANGE:
				zone->keylo = (int)sfgen->amount.range.lo;
				zone->keyhi = (int)sfgen->amount.range.hi;
				break;
			case GEN_VELRANGE:
				zone->vello = (int)sfgen->amount.range.lo;
				zone->velhi = (int)sfgen->amount.range.hi;
				break;
			default:
				zone->gen[sfgen->id].val = (fluid_real_t)sfgen->amount.sword;
				zone->gen[sfgen->id].flags = GEN_SET;
				break;
		}
		r = fluid_list_next(r);
	}

	if ((sfzone->instsamp != NULL) && (sfzone->instsamp->data != NULL)) {
		zone->sample = fluid_defsfont_get_sample(sfont, ((SFSample *)sfzone->instsamp->data)->name);
		if (zone->sample == NULL) {
			FLUID_LOG(FLUID_ERR, "Couldn't find sample name");
			return FLUID_FAILED;
		}
	}

	for (count = 0, r = sfzone->mod; r != NULL; count++) {
		SFMod* mod_src = (SFMod *)r->data;
		int type;
		fluid_mod_t* mod_dest;

		mod_dest = fluid_mod_new();
		if (mod_dest == NULL){
			return FLUID_FAILED;
		}

		mod_dest->next = NULL;

		mod_dest->amount = mod_src->amount;

		mod_dest->src1 = mod_src->src & 127;
		mod_dest->flags1 = 0;

		if (mod_src->src & (1 << 7)){
			mod_dest->flags1 |= FLUID_MOD_CC;
		}
		else {
			mod_dest->flags1 |= FLUID_MOD_GC;
		}

		if (mod_src->src & (1 << 8)){
			mod_dest->flags1 |= FLUID_MOD_NEGATIVE;
		}
		else {
			mod_dest->flags1 |= FLUID_MOD_POSITIVE;
		}

		if (mod_src->src & (1 << 9)){
			mod_dest->flags1 |= FLUID_MOD_BIPOLAR;
		}
		else {
			mod_dest->flags1 |= FLUID_MOD_UNIPOLAR;
		}

		type = (mod_src->src) >> 10;
		type &= 63;
		if (type == 0){
			mod_dest->flags1 |= FLUID_MOD_LINEAR;
		}
		else if (type == 1){
			mod_dest->flags1 |= FLUID_MOD_CONCAVE;
		}
		else if (type == 2){
			mod_dest->flags1 |= FLUID_MOD_CONVEX;
		}
		else if (type == 3){
			mod_dest->flags1 |= FLUID_MOD_SWITCH;
		}
		else {
			mod_dest->amount = 0;
		}

		mod_dest->dest = mod_src->dest;

		mod_dest->src2 = mod_src->amtsrc & 127;
		mod_dest->flags2 = 0;

		if (mod_src->amtsrc & (1 << 7)){
			mod_dest->flags2 |= FLUID_MOD_CC;
		}
		else {
			mod_dest->flags2 |= FLUID_MOD_GC;
		}

		if (mod_src->amtsrc & (1 << 8)){
			mod_dest->flags2 |= FLUID_MOD_NEGATIVE;
		}
		else {
			mod_dest->flags2 |= FLUID_MOD_POSITIVE;
		}

		if (mod_src->amtsrc & (1 << 9)){
			mod_dest->flags2 |= FLUID_MOD_BIPOLAR;
		}
		else {
			mod_dest->flags2 |= FLUID_MOD_UNIPOLAR;
		}

		type = (mod_src->amtsrc) >> 10;
		type &= 63;
		if (type == 0){
			mod_dest->flags2 |= FLUID_MOD_LINEAR;
		}
		else if (type == 1){
			mod_dest->flags2 |= FLUID_MOD_CONCAVE;
		}
		else if (type == 2){
			mod_dest->flags2 |= FLUID_MOD_CONVEX;
		}
		else if (type == 3){
			mod_dest->flags2 |= FLUID_MOD_SWITCH;
		}
		else {
			mod_dest->amount = 0;
		}

		if (mod_src->trans != 0){
			mod_dest->amount = 0;
		}

		if (count == 0){
			zone->mod = mod_dest;
		}
		else {
			fluid_mod_t * last_mod = zone->mod;

			while (last_mod->next != NULL){
				last_mod = last_mod->next;
			}
			last_mod->next = mod_dest;
		}

		r = fluid_list_next(r);
	}
	return FLUID_OK;
}

fluid_sample_t*
fluid_inst_zone_get_sample(fluid_inst_zone_t* zone)
{
	return zone->sample;
}

int fluid_inst_zone_inside_range(fluid_inst_zone_t* zone, int key, int vel)
{
	return ((zone->keylo <= key) &&
		(zone->keyhi >= key) &&
		(zone->vello <= vel) &&
		(zone->velhi >= vel));
}

fluid_sample_t*
new_fluid_sample()
{
	fluid_sample_t* sample = NULL;

	sample = FLUID_NEW(fluid_sample_t);
	if (sample == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return NULL;
	}

	memset(sample, 0, sizeof(fluid_sample_t));
	sample->valid = 1;

	return sample;
}

int delete_fluid_sample(fluid_sample_t* sample)
{
	FLUID_FREE(sample);
	return FLUID_OK;
}

int fluid_sample_in_rom(fluid_sample_t* sample)
{
	return (sample->sampletype & FLUID_SAMPLETYPE_ROM);
}

int fluid_sample_import_sfont(fluid_sample_t* sample, SFSample* sfsample, fluid_defsfont_t* sfont)
{
	FLUID_STRCPY(sample->name, sfsample->name);
	sample->data = sfont->sampledata;
	sample->start = sfsample->start;
	sample->end = sfsample->start + sfsample->end;
	sample->loopstart = sfsample->start + sfsample->loopstart;
	sample->loopend = sfsample->start + sfsample->loopend;
	sample->samplerate = sfsample->samplerate;
	sample->origpitch = sfsample->origpitch;
	sample->pitchadj = sfsample->pitchadj;
	sample->sampletype = sfsample->sampletype;

	if (sample->sampletype & FLUID_SAMPLETYPE_ROM) {
		sample->valid = 0;
		FLUID_LOG1(FLUID_WARN, "Ignoring sample %s: can't use ROM samples", sample->name);
	}
	if (sample->end - sample->start < 8) {
		sample->valid = 0;
		FLUID_LOG1(FLUID_WARN, "Ignoring sample %s: too few sample data points", sample->name);
	}
	else {
	}
	return FLUID_OK;
}

#ifdef WORDS_BIGENDIAN
#define READCHUNK(var,fd)	G_STMT_START {		\
	if (!safe_fread(var, 8, fd))			\
	return(FAIL);					\
	((SFChunk *)(var))->size = GUINT32_FROM_BE(((SFChunk *)(var))->size);  \
} G_STMT_END
#else
#define READCHUNK(var,fd)	G_STMT_START {		\
    if (!safe_fread(var, 8, fd))			\
	return(FAIL);					\
    ((SFChunk *)(var))->size = GUINT32_FROM_LE(((SFChunk *)(var))->size);  \
} G_STMT_END
#endif
#define READID(var,fd)		G_STMT_START {		\
    if (!safe_fread(var, 4, fd))			\
	return(FAIL);					\
} G_STMT_END
#define READSTR(var,fd)		G_STMT_START {		\
    if (!safe_fread(var, 20, fd))			\
	return(FAIL);					\
    (*var)[20] = '\0';					\
} G_STMT_END
#ifdef WORDS_BIGENDIAN
#define READD(var,fd)		G_STMT_START {		\
	unsigned int _temp;					\
	if (!safe_fread(&_temp, 4, fd))			\
	return(FAIL);					\
	var = GINT32_FROM_BE(_temp);			\
} G_STMT_END
#else
#define READD(var,fd)		G_STMT_START {		\
    unsigned int _temp;					\
    if (!safe_fread(&_temp, 4, fd))			\
	return(FAIL);					\
    var = GINT32_FROM_LE(_temp);			\
} G_STMT_END
#endif
#ifdef WORDS_BIGENDIAN
#define READW(var,fd)		G_STMT_START {		\
	unsigned short _temp;					\
	if (!safe_fread(&_temp, 2, fd))			\
	return(FAIL);					\
var = GINT16_FROM_BE(_temp);			\
} G_STMT_END
#else
#define READW(var,fd)		G_STMT_START {		\
    unsigned short _temp;					\
    if (!safe_fread(&_temp, 2, fd))			\
	return(FAIL);					\
    var = GINT16_FROM_LE(_temp);			\
} G_STMT_END
#endif
#define READB(var,fd)		G_STMT_START {		\
    if (!safe_fread(&var, 1, fd))			\
	return(FAIL);					\
} G_STMT_END
#define FSKIP(size,fd)		G_STMT_START {		\
    if (!safe_fseek(fd, size, RW_SEEK_CUR))		\
	return(FAIL);					\
} G_STMT_END
#define FSKIPW(fd)		G_STMT_START {		\
    if (!safe_fseek(fd, 2, RW_SEEK_CUR))			\
	return(FAIL);					\
} G_STMT_END

#define SLADVREM(list, item)	G_STMT_START {		\
    fluid_list_t *_temp = item;				\
    item = fluid_list_next(item);				\
    list = fluid_list_remove_link(list, _temp);		\
    delete1_fluid_list(_temp);				\
} G_STMT_END

static int chunkid(unsigned int id);
static int load_body(unsigned int size, SFData * sf, ZL_RWops* fd);
static int read_listchunk(SFChunk * chunk, ZL_RWops* fd);
static int process_info(int size, SFData * sf, ZL_RWops* fd);
static int process_sdta(int size, SFData * sf, ZL_RWops* fd);
static int pdtahelper(unsigned int expid, unsigned int reclen, SFChunk * chunk,
	int * size, ZL_RWops* fd);
static int process_pdta(int size, SFData * sf, ZL_RWops* fd);
static int load_phdr(int size, SFData * sf, ZL_RWops* fd);
static int load_pbag(int size, SFData * sf, ZL_RWops* fd);
static int load_pmod(int size, SFData * sf, ZL_RWops* fd);
static int load_pgen(int size, SFData * sf, ZL_RWops* fd);
static int load_ihdr(int size, SFData * sf, ZL_RWops* fd);
static int load_ibag(int size, SFData * sf, ZL_RWops* fd);
static int load_imod(int size, SFData * sf, ZL_RWops* fd);
static int load_igen(int size, SFData * sf, ZL_RWops* fd);
static int load_shdr(unsigned int size, SFData * sf, ZL_RWops* fd);
static int fixup_pgen(SFData * sf);
static int fixup_igen(SFData * sf);
static int fixup_sample(SFData * sf);

static char idlist[] = {
	"RIFFLISTsfbkINFOsdtapdtaifilisngINAMiromiverICRDIENGIPRD"
	"ICOPICMTISFTsnamsmplphdrpbagpmodpgeninstibagimodigenshdr"
};

static unsigned int sdtachunk_size;

static int chunkid(unsigned int id)
{
	unsigned int i;
	unsigned int *p;

	p = (unsigned int *)& idlist;
	for (i = 0; i < sizeof(idlist) / sizeof(int); i++, p += 1)
		if (*p == id)
			return (i + 1);

	return (UNKN_ID);
}

SFData *
sfload_zlfile(ZL_RWops *fd)
{
	SFData *sf = NULL;
	int fsize = 0;
	int err = FALSE;

	if (!(sf = FLUID_NEW(SFData)))
	{
		FLUID_LOG(FLUID_ERR, "Out of memory");
		err = TRUE;
	}

	if (!err)
	{
		memset(sf, 0, sizeof(SFData));
		sf->sfrw = fd;
	}

	if (!err && fd->seektell(0L, RW_SEEK_END) == -1)
	{
		err = TRUE;
		FLUID_LOG(FLUID_ERR, _("Seek to end of file failed"));
	}
	if (!err && (fsize = ZL_RWtell(fd)) == -1)
	{
		err = TRUE;
		FLUID_LOG(FLUID_ERR, _("Get end of file position failed"));
	}
	if (!err)
		ZL_RWrewind(fd);

	if (!err && !load_body(fsize, sf, fd))
		err = TRUE;

	if (err)
	{
		if (sf)
			sfont_close(sf);
		return (NULL);
	}

	return (sf);
}

static int load_body(unsigned int size, SFData * sf, ZL_RWops* fd)
{
	SFChunk chunk;

	READCHUNK(&chunk, fd);
	if (chunkid(chunk.id) != RIFF_ID) {
		FLUID_LOG(FLUID_ERR, _("Not a RIFF file"));
		return (FAIL);
	}

	READID(&chunk.id, fd);
	if (chunkid(chunk.id) != SFBK_ID) {
		FLUID_LOG(FLUID_ERR, _("Not a sound font file"));
		return (FAIL);
	}

	if (chunk.size != size - 8) {
		FLUID_LOG(FLUID_ERR, _("Sound font file size mismatch"));
		return (FAIL);
	}

	if (!read_listchunk(&chunk, fd))
		return (FAIL);
	if (chunkid(chunk.id) != INFO_ID)
		return (FGERR0(ErrCorr, _("Invalid ID found when expecting INFO chunk")));
	if (!process_info(chunk.size, sf, fd))
		return (FAIL);

	if (!read_listchunk(&chunk, fd))
		return (FAIL);
	if (chunkid(chunk.id) != SDTA_ID)
		return (FGERR0(ErrCorr,
		_("Invalid ID found when expecting SAMPLE chunk")));
	if (!process_sdta(chunk.size, sf, fd))
		return (FAIL);

	if (!read_listchunk(&chunk, fd))
		return (FAIL);
	if (chunkid(chunk.id) != PDTA_ID)
		return (FGERR0(ErrCorr, _("Invalid ID found when expecting HYDRA chunk")));
	if (!process_pdta(chunk.size, sf, fd))
		return (FAIL);

	if (!fixup_pgen(sf))
		return (FAIL);
	if (!fixup_igen(sf))
		return (FAIL);
	if (!fixup_sample(sf))
		return (FAIL);

	sf->preset = fluid_list_sort(sf->preset,
		(fluid_compare_func_t)sfont_preset_compare_func);

	return (OK);
}

static int read_listchunk(SFChunk * chunk, ZL_RWops* fd)
{
	READCHUNK(chunk, fd);
	if (chunkid(chunk->id) != LIST_ID)
		return (FGERR0(ErrCorr, _("Invalid chunk id in level 0 parse")));
	READID(&chunk->id, fd);
	chunk->size -= 4;
	return (OK);
}

static int process_info(int size, SFData * sf, ZL_RWops* fd)
{
	SFChunk chunk;
	unsigned char id;
	char *item;
	unsigned short ver;

	while (size > 0)
	{
		READCHUNK(&chunk, fd);
		size -= 8;

		id = chunkid(chunk.id);

		if (id == IFIL_ID)
		{
			if (chunk.size != 4)
				return (FGERR0(ErrCorr,
				_("Sound font version info chunk has invalid size")));

			READW(ver, fd);
			sf->version.major = ver;
			READW(ver, fd);
			sf->version.minor = ver;

			if (sf->version.major < 2) {
				FLUID_LOG2(FLUID_ERR,
					_("Sound font version is %d.%d which is not"
					" supported, convert to version 2.0x"),
					sf->version.major,
					sf->version.minor);
				return (FAIL);
			}

			if (sf->version.major > 2) {
				FLUID_LOG2(FLUID_WARN,
					_("Sound font version is %d.%d which is newer than"
					" what this version of FLUID Synth was designed for (v2.0x)"),
					sf->version.major,
					sf->version.minor);
				return (FAIL);
			}
		}
		else if (id == IVER_ID)
		{
			if (chunk.size != 4)
				return (FGERR0(ErrCorr,
				_("ROM version info chunk has invalid size")));

			READW(ver, fd);
			sf->romver.major = ver;
			READW(ver, fd);
			sf->romver.minor = ver;
		}
		else if (id != UNKN_ID)
		{
			if ((id != ICMT_ID && chunk.size > 256) || (chunk.size > 65536)
				|| (chunk.size % 2))
				return (FGERR2(ErrCorr,
				_("INFO sub chunk %.4s has invalid chunk size"
				" of %d bytes"), &chunk.id, chunk.size));

			if (!(item = (char*)FLUID_MALLOC(chunk.size + 1)))
			{
				FLUID_LOG(FLUID_ERR, "Out of memory");
				return (FAIL);
			}

			sf->info = fluid_list_append(sf->info, item);

			*(unsigned char *)item = id;
			if (!safe_fread(&item[1], chunk.size, fd))
				return (FAIL);

			*(item + chunk.size) = '\0';
		}
		else
			return (FGERR0(ErrCorr, _("Invalid chunk id in INFO chunk")));
		size -= chunk.size;
	}

	if (size < 0)
		return (FGERR0(ErrCorr, _("INFO chunk size mismatch")));

	return (OK);
}

static int process_sdta(int size, SFData * sf, ZL_RWops* fd)
{
	SFChunk chunk;

	if (size == 0)
		return (OK);

	READCHUNK(&chunk, fd);
	size -= 8;

	if (chunkid(chunk.id) != SMPL_ID)
		return (FGERR0(ErrCorr,
		_("Expected SMPL chunk found invalid id instead")));

	if ((size - chunk.size) != 0)
		return (FGERR0(ErrCorr, _("SDTA chunk size mismatch")));

	sf->samplepos = ZL_RWtell(fd);

	sdtachunk_size = chunk.size;
	sf->samplesize = chunk.size;

	FSKIP(chunk.size, fd);

	return (OK);
}

static int pdtahelper(unsigned int expid, unsigned int reclen, SFChunk * chunk,
int * size, ZL_RWops* fd)
{
	unsigned int id;
	char *expstr; (void)expstr;

	expstr = CHNKIDSTR(expid);

	READCHUNK(chunk, fd);
	*size -= 8;

	if ((id = chunkid(chunk->id)) != expid)
		return (FGERR1(ErrCorr, _("Expected"
		" PDTA sub-chunk \"%.4s\" found invalid id instead"), expstr));

	if (chunk->size % reclen)
		return (FGERR2(ErrCorr,
		_("\"%.4s\" chunk size is not a multiple of %d bytes"), expstr,
		reclen));
	if ((*size -= chunk->size) < 0)
		return (FGERR1(ErrCorr,
		_("\"%.4s\" chunk size exceeds remaining PDTA chunk size"), expstr));
	return (OK);
}

static int process_pdta(int size, SFData * sf, ZL_RWops* fd)
{
	SFChunk chunk;

	if (!pdtahelper(PHDR_ID, SFPHDRSIZE, &chunk, &size, fd))
		return (FAIL);
	if (!load_phdr(chunk.size, sf, fd))
		return (FAIL);

	if (!pdtahelper(PBAG_ID, SFBAGSIZE, &chunk, &size, fd))
		return (FAIL);
	if (!load_pbag(chunk.size, sf, fd))
		return (FAIL);

	if (!pdtahelper(PMOD_ID, SFMODSIZE, &chunk, &size, fd))
		return (FAIL);
	if (!load_pmod(chunk.size, sf, fd))
		return (FAIL);

	if (!pdtahelper(PGEN_ID, SFGENSIZE, &chunk, &size, fd))
		return (FAIL);
	if (!load_pgen(chunk.size, sf, fd))
		return (FAIL);

	if (!pdtahelper(IHDR_ID, SFIHDRSIZE, &chunk, &size, fd))
		return (FAIL);
	if (!load_ihdr(chunk.size, sf, fd))
		return (FAIL);

	if (!pdtahelper(IBAG_ID, SFBAGSIZE, &chunk, &size, fd))
		return (FAIL);
	if (!load_ibag(chunk.size, sf, fd))
		return (FAIL);

	if (!pdtahelper(IMOD_ID, SFMODSIZE, &chunk, &size, fd))
		return (FAIL);
	if (!load_imod(chunk.size, sf, fd))
		return (FAIL);

	if (!pdtahelper(IGEN_ID, SFGENSIZE, &chunk, &size, fd))
		return (FAIL);
	if (!load_igen(chunk.size, sf, fd))
		return (FAIL);

	if (!pdtahelper(SHDR_ID, SFSHDRSIZE, &chunk, &size, fd))
		return (FAIL);
	if (!load_shdr(chunk.size, sf, fd))
		return (FAIL);

	return (OK);
}

static int load_phdr(int size, SFData * sf, ZL_RWops* fd)
{
	int i, i2;
	SFPreset *p, *pr = NULL;
	unsigned short zndx, pzndx = 0;

	if (size % SFPHDRSIZE || size == 0)
		return (FGERR0(ErrCorr, _("Preset header chunk size is invalid")));

	i = size / SFPHDRSIZE - 1;
	if (i == 0)
	{
		FLUID_LOG(FLUID_WARN, _("File contains no presets"));
		FSKIP(SFPHDRSIZE, fd);
		return (OK);
	}

	for (; i > 0; i--)
	{
		p = FLUID_NEW(SFPreset);
		sf->preset = fluid_list_append(sf->preset, p);
		p->zone = NULL;
		READSTR(&p->name, fd);
		READW(p->prenum, fd);
		READW(p->bank, fd);
		READW(zndx, fd);
		READD(p->libr, fd);
		READD(p->genre, fd);
		READD(p->morph, fd);

		if (pr)
		{
			if (zndx < pzndx)
				return (FGERR0(ErrCorr, _("Preset header indices not monotonic")));
			i2 = zndx - pzndx;
			while (i2--)
			{
				pr->zone = fluid_list_prepend(pr->zone, NULL);
			}
		}
		else if (zndx > 0)
			FLUID_LOG1(FLUID_WARN, _("%d preset zones not referenced, discarding"), zndx);
		pr = p;
		pzndx = zndx;
	}

	FSKIP(24, fd);
	READW(zndx, fd);
	FSKIP(12, fd);

	if (zndx < pzndx)
		return (FGERR0(ErrCorr, _("Preset header indices not monotonic")));
	i2 = zndx - pzndx;
	while (i2--)
	{
		pr->zone = fluid_list_prepend(pr->zone, NULL);
	}

	return (OK);
}

static int load_pbag(int size, SFData * sf, ZL_RWops* fd)
{
	fluid_list_t *p, *p2;
	SFZone *z, *pz = NULL;
	unsigned short genndx, modndx;
	unsigned short pgenndx = 0, pmodndx = 0;
	unsigned short i;

	if (size % SFBAGSIZE || size == 0)
		return (FGERR0(ErrCorr, _("Preset bag chunk size is invalid")));

	p = sf->preset;
	while (p)
	{
		p2 = ((SFPreset *)(p->data))->zone;
		while (p2)
		{
			if ((size -= SFBAGSIZE) < 0)
				return (FGERR0(ErrCorr, _("Preset bag chunk size mismatch")));
			z = FLUID_NEW(SFZone);
			p2->data = z;
			z->gen = NULL;
			z->mod = NULL;
			READW(genndx, fd);
			READW(modndx, fd);
			z->instsamp = NULL;

			if (pz)
			{
				if (genndx < pgenndx)
					return (FGERR0(ErrCorr,
					_("Preset bag generator indices not monotonic")));
				if (modndx < pmodndx)
					return (FGERR0(ErrCorr,
					_("Preset bag modulator indices not monotonic")));
				i = genndx - pgenndx;
				while (i--)
					pz->gen = fluid_list_prepend(pz->gen, NULL);
				i = modndx - pmodndx;
				while (i--)
					pz->mod = fluid_list_prepend(pz->mod, NULL);
			}
			pz = z;
			pgenndx = genndx;
			pmodndx = modndx;
			p2 = fluid_list_next(p2);
		}
		p = fluid_list_next(p);
	}

	size -= SFBAGSIZE;
	if (size != 0)
		return (FGERR0(ErrCorr, _("Preset bag chunk size mismatch")));

	READW(genndx, fd);
	READW(modndx, fd);

	if (!pz)
	{
		if (genndx > 0)
			FLUID_LOG(FLUID_WARN, _("No preset generators and terminal index not 0"));
		if (modndx > 0)
			FLUID_LOG(FLUID_WARN, _("No preset modulators and terminal index not 0"));
		return (OK);
	}

	if (genndx < pgenndx)
		return (FGERR0(ErrCorr, _("Preset bag generator indices not monotonic")));
	if (modndx < pmodndx)
		return (FGERR0(ErrCorr, _("Preset bag modulator indices not monotonic")));
	i = genndx - pgenndx;
	while (i--)
		pz->gen = fluid_list_prepend(pz->gen, NULL);
	i = modndx - pmodndx;
	while (i--)
		pz->mod = fluid_list_prepend(pz->mod, NULL);

	return (OK);
}

static int load_pmod(int size, SFData * sf, ZL_RWops* fd)
{
	fluid_list_t *p, *p2, *p3;
	SFMod *m;

	p = sf->preset;
	while (p)
	{
		p2 = ((SFPreset *)(p->data))->zone;
		while (p2)
		{
			p3 = ((SFZone *)(p2->data))->mod;
			while (p3)
			{
				if ((size -= SFMODSIZE) < 0)
					return (FGERR0(ErrCorr,
					_("Preset modulator chunk size mismatch")));
				m = FLUID_NEW(SFMod);
				p3->data = m;
				READW(m->src, fd);
				READW(m->dest, fd);
				READW(m->amount, fd);
				READW(m->amtsrc, fd);
				READW(m->trans, fd);
				p3 = fluid_list_next(p3);
			}
			p2 = fluid_list_next(p2);
		}
		p = fluid_list_next(p);
	}

	if (size == 0)
		return (OK);

	size -= SFMODSIZE;
	if (size != 0)
		return (FGERR0(ErrCorr, _("Preset modulator chunk size mismatch")));
	FSKIP(SFMODSIZE, fd);

	return (OK);
}

static int load_pgen(int size, SFData * sf, ZL_RWops* fd)
{
	fluid_list_t *p, *p2, *p3, *dup, **hz = NULL;
	SFZone *z;
	SFGen *g;
	SFGenAmount genval;
	unsigned short genid;
	int level, skip, drop, gzone, discarded;

	p = sf->preset;
	while (p)
	{
		gzone = FALSE;
		discarded = FALSE;
		p2 = ((SFPreset *)(p->data))->zone;
		if (p2)
			hz = &p2;
		while (p2)
		{
			level = 0;
			z = (SFZone *)(p2->data);
			p3 = z->gen;
			while (p3)
			{
				dup = NULL;
				skip = FALSE;
				drop = FALSE;
				if ((size -= SFGENSIZE) < 0)
					return (FGERR0(ErrCorr,
					_("Preset generator chunk size mismatch")));

				READW(genid, fd);

				if (genid == Gen_KeyRange)
				{
					if (level == 0)
					{
						level = 1;
						READB(genval.range.lo, fd);
						READB(genval.range.hi, fd);
					}
					else
						skip = TRUE;
				}
				else if (genid == Gen_VelRange)
				{
					if (level <= 1)
					{
						level = 2;
						READB(genval.range.lo, fd);
						READB(genval.range.hi, fd);
					}
					else
						skip = TRUE;
				}
				else if (genid == Gen_Instrument)
				{
					level = 3;
					READW(genval.uword, fd);
					((SFZone *)(p2->data))->instsamp = (_fluid_list_t*)GINT_TO_POINTER(genval.uword + 1);
					break;
				}
				else
				{
					level = 2;
					if (gen_validp(genid))
					{
						READW(genval.sword, fd);
						dup = gen_inlist(genid, z->gen);
					}
					else
						skip = TRUE;
				}

				if (!skip)
				{
					if (!dup)
					{
						g = FLUID_NEW(SFGen);
						p3->data = g;
						g->id = genid;
					}
					else
					{
						g = (SFGen *)(dup->data);
						drop = TRUE;
					}
					g->amount = genval;
				}
				else
				{
					discarded = TRUE;
					drop = TRUE;
					FSKIPW(fd);
				}

				if (!drop)
					p3 = fluid_list_next(p3);
				else
					SLADVREM(z->gen, p3);

			}

			if (level == 3)
				SLADVREM(z->gen, p3);
			else
			{
				if (!gzone)
				{
					gzone = TRUE;

					if (*hz != p2)
					{
						void* save = p2->data;
						FLUID_LOG1(FLUID_WARN,
							_("Preset \"%s\": Global zone is not first zone"),
							((SFPreset *)(p->data))->name);
						SLADVREM(*hz, p2);
						*hz = fluid_list_prepend(*hz, save);
						continue;
					}
				}
				else
				{
					FLUID_LOG1(FLUID_WARN,
						_("Preset \"%s\": Discarding invalid global zone"),
						((SFPreset *)(p->data))->name);
					sfont_zone_delete(sf, hz, (SFZone *)(p2->data));
				}
			}

			while (p3)
			{
				discarded = TRUE;
				if ((size -= SFGENSIZE) < 0)
					return (FGERR0(ErrCorr,
					_("Preset generator chunk size mismatch")));
				FSKIP(SFGENSIZE, fd);
				SLADVREM(z->gen, p3);
			}

			p2 = fluid_list_next(p2);
		}
		if (discarded)
			FLUID_LOG1(FLUID_WARN,
			_("Preset \"%s\": Some invalid generators were discarded"),
			((SFPreset *)(p->data))->name);
		p = fluid_list_next(p);
	}

	if (size == 0)
		return (OK);

	size -= SFGENSIZE;
	if (size != 0)
		return (FGERR0(ErrCorr, _("Preset generator chunk size mismatch")));
	FSKIP(SFGENSIZE, fd);

	return (OK);
}

static int load_ihdr(int size, SFData * sf, ZL_RWops* fd)
{
	int i, i2;
	SFInst *p, *pr = NULL;
	unsigned short zndx, pzndx = 0;

	if (size % SFIHDRSIZE || size == 0)
		return (FGERR0(ErrCorr, _("Instrument header has invalid size")));

	size = size / SFIHDRSIZE - 1;
	if (size == 0)
	{
		FLUID_LOG(FLUID_WARN, _("File contains no instruments"));
		FSKIP(SFIHDRSIZE, fd);
		return (OK);
	}

	for (i = 0; i < size; i++)
	{
		p = FLUID_NEW(SFInst);
		sf->inst = fluid_list_append(sf->inst, p);
		p->zone = NULL;
		READSTR(&p->name, fd);
		READW(zndx, fd);

		if (pr)
		{
			if (zndx < pzndx)
				return (FGERR0(ErrCorr,
				_("Instrument header indices not monotonic")));
			i2 = zndx - pzndx;
			while (i2--)
				pr->zone = fluid_list_prepend(pr->zone, NULL);
		}
		else if (zndx > 0)
			FLUID_LOG1(FLUID_WARN, _("%d instrument zones not referenced, discarding"),
			zndx);
		pzndx = zndx;
		pr = p;
	}

	FSKIP(20, fd);
	READW(zndx, fd);

	if (zndx < pzndx)
		return (FGERR0(ErrCorr, _("Instrument header indices not monotonic")));
	i2 = zndx - pzndx;
	while (i2--)
		pr->zone = fluid_list_prepend(pr->zone, NULL);

	return (OK);
}

static int load_ibag(int size, SFData * sf, ZL_RWops* fd)
{
	fluid_list_t *p, *p2;
	SFZone *z, *pz = NULL;
	unsigned short genndx, modndx, pgenndx = 0, pmodndx = 0;
	int i;

	if (size % SFBAGSIZE || size == 0)
		return (FGERR0(ErrCorr, _("Instrument bag chunk size is invalid")));

	p = sf->inst;
	while (p)
	{
		p2 = ((SFInst *)(p->data))->zone;
		while (p2)
		{
			if ((size -= SFBAGSIZE) < 0)
				return (FGERR0(ErrCorr, _("Instrument bag chunk size mismatch")));
			z = FLUID_NEW(SFZone);
			p2->data = z;
			z->gen = NULL;
			z->mod = NULL;
			READW(genndx, fd);
			READW(modndx, fd);
			z->instsamp = NULL;

			if (pz)
			{
				if (genndx < pgenndx)
					return (FGERR0(ErrCorr,
					_("Instrument generator indices not monotonic")));
				if (modndx < pmodndx)
					return (FGERR0(ErrCorr,
					_("Instrument modulator indices not monotonic")));
				i = genndx - pgenndx;
				while (i--)
					pz->gen = fluid_list_prepend(pz->gen, NULL);
				i = modndx - pmodndx;
				while (i--)
					pz->mod = fluid_list_prepend(pz->mod, NULL);
			}
			pz = z;
			pgenndx = genndx;
			pmodndx = modndx;
			p2 = fluid_list_next(p2);
		}
		p = fluid_list_next(p);
	}

	size -= SFBAGSIZE;
	if (size != 0)
		return (FGERR0(ErrCorr, _("Instrument chunk size mismatch")));

	READW(genndx, fd);
	READW(modndx, fd);

	if (!pz)
	{
		if (genndx > 0)
			FLUID_LOG(FLUID_WARN,
			_("No instrument generators and terminal index not 0"));
		if (modndx > 0)
			FLUID_LOG(FLUID_WARN,
			_("No instrument modulators and terminal index not 0"));
		return (OK);
	}

	if (genndx < pgenndx)
		return (FGERR0(ErrCorr, _("Instrument generator indices not monotonic")));
	if (modndx < pmodndx)
		return (FGERR0(ErrCorr, _("Instrument modulator indices not monotonic")));
	i = genndx - pgenndx;
	while (i--)
		pz->gen = fluid_list_prepend(pz->gen, NULL);
	i = modndx - pmodndx;
	while (i--)
		pz->mod = fluid_list_prepend(pz->mod, NULL);

	return (OK);
}

static int load_imod(int size, SFData * sf, ZL_RWops* fd)
{
	fluid_list_t *p, *p2, *p3;
	SFMod *m;

	p = sf->inst;
	while (p)
	{
		p2 = ((SFInst *)(p->data))->zone;
		while (p2)
		{
			p3 = ((SFZone *)(p2->data))->mod;
			while (p3)
			{
				if ((size -= SFMODSIZE) < 0)
					return (FGERR0(ErrCorr,
					_("Instrument modulator chunk size mismatch")));
				m = FLUID_NEW(SFMod);
				p3->data = m;
				READW(m->src, fd);
				READW(m->dest, fd);
				READW(m->amount, fd);
				READW(m->amtsrc, fd);
				READW(m->trans, fd);
				p3 = fluid_list_next(p3);
			}
			p2 = fluid_list_next(p2);
		}
		p = fluid_list_next(p);
	}

	if (size == 0)
		return (OK);

	size -= SFMODSIZE;
	if (size != 0)
		return (FGERR0(ErrCorr, _("Instrument modulator chunk size mismatch")));
	FSKIP(SFMODSIZE, fd);

	return (OK);
}

static int load_igen(int size, SFData * sf, ZL_RWops* fd)
{
	fluid_list_t *p, *p2, *p3, *dup, **hz = NULL;
	SFZone *z;
	SFGen *g;
	SFGenAmount genval;
	unsigned short genid;
	int level, skip, drop, gzone, discarded;

	p = sf->inst;
	while (p)
	{
		gzone = FALSE;
		discarded = FALSE;
		p2 = ((SFInst *)(p->data))->zone;
		if (p2)
			hz = &p2;
		while (p2)
		{
			level = 0;
			z = (SFZone *)(p2->data);
			p3 = z->gen;
			while (p3)
			{
				dup = NULL;
				skip = FALSE;
				drop = FALSE;
				if ((size -= SFGENSIZE) < 0)
					return (FGERR0(ErrCorr, _("IGEN chunk size mismatch")));

				READW(genid, fd);

				if (genid == Gen_KeyRange)
				{
					if (level == 0)
					{
						level = 1;
						READB(genval.range.lo, fd);
						READB(genval.range.hi, fd);
					}
					else
						skip = TRUE;
				}
				else if (genid == Gen_VelRange)
				{
					if (level <= 1)
					{
						level = 2;
						READB(genval.range.lo, fd);
						READB(genval.range.hi, fd);
					}
					else
						skip = TRUE;
				}
				else if (genid == Gen_SampleId)
				{
					level = 3;
					READW(genval.uword, fd);
					((SFZone *)(p2->data))->instsamp = (_fluid_list_t *)GINT_TO_POINTER(genval.uword + 1);
					break;
				}
				else
				{
					level = 2;
					if (gen_valid(genid))
					{
						READW(genval.sword, fd);
						dup = gen_inlist(genid, z->gen);
					}
					else
						skip = TRUE;
				}

				if (!skip)
				{
					if (!dup)
					{
						g = FLUID_NEW(SFGen);
						p3->data = g;
						g->id = genid;
					}
					else
					{
						g = (SFGen *)(dup->data);
						drop = TRUE;
					}
					g->amount = genval;
				}
				else
				{
					discarded = TRUE;
					drop = TRUE;
					FSKIPW(fd);
				}

				if (!drop)
					p3 = fluid_list_next(p3);
				else
					SLADVREM(z->gen, p3);

			}

			if (level == 3)
				SLADVREM(z->gen, p3);
			else
			{
				if (!gzone)
				{
					gzone = TRUE;

					if (*hz != p2)
					{
						void* save = p2->data;
						FLUID_LOG1(FLUID_WARN,
							_("Instrument \"%s\": Global zone is not first zone"),
							((SFPreset *)(p->data))->name);
						SLADVREM(*hz, p2);
						*hz = fluid_list_prepend(*hz, save);
						continue;
					}
				}
				else
				{
					FLUID_LOG1(FLUID_WARN,
						_("Instrument \"%s\": Discarding invalid global zone"),
						((SFInst *)(p->data))->name);
					sfont_zone_delete(sf, hz, (SFZone *)(p2->data));
				}
			}

			while (p3)
			{
				discarded = TRUE;
				if ((size -= SFGENSIZE) < 0)
					return (FGERR0(ErrCorr,
					_("Instrument generator chunk size mismatch")));
				FSKIP(SFGENSIZE, fd);
				SLADVREM(z->gen, p3);
			}

			p2 = fluid_list_next(p2);
		}
		if (discarded)
			FLUID_LOG1(FLUID_WARN,
			_("Instrument \"%s\": Some invalid generators were discarded"),
			((SFInst *)(p->data))->name);
		p = fluid_list_next(p);
	}

	if (size == 0)
		return (OK);

	size -= SFGENSIZE;
	if (size != 0)
		return (FGERR0(ErrCorr, _("IGEN chunk size mismatch")));
	FSKIP(SFGENSIZE, fd);

	return (OK);
}

static int load_shdr(unsigned int size, SFData * sf, ZL_RWops* fd)
{
	unsigned int i;
	SFSample *p;

	if (size % SFSHDRSIZE || size == 0)
		return (FGERR0(ErrCorr, _("Sample header has invalid size")));

	size = size / SFSHDRSIZE - 1;
	if (size == 0)
	{
		FLUID_LOG(FLUID_WARN, _("File contains no samples"));
		FSKIP(SFSHDRSIZE, fd);
		return (OK);
	}

	for (i = 0; i < size; i++)
	{
		p = FLUID_NEW(SFSample);
		sf->sample = fluid_list_append(sf->sample, p);
		READSTR(&p->name, fd);
		READD(p->start, fd);
		READD(p->end, fd);
		READD(p->loopstart, fd);
		READD(p->loopend, fd);
		READD(p->samplerate, fd);
		READB(p->origpitch, fd);
		READB(p->pitchadj, fd);
		FSKIPW(fd);
		READW(p->sampletype, fd);
		p->samfile = 0;
	}

	FSKIP(SFSHDRSIZE, fd);

	return (OK);
}

static int fixup_pgen(SFData * sf)
{
	fluid_list_t *p, *p2, *p3;
	SFZone *z;
	int i;

	p = sf->preset;
	while (p)
	{
		p2 = ((SFPreset *)(p->data))->zone;
		while (p2)
		{
			z = (SFZone *)(p2->data);
			if ((i = GPOINTER_TO_INT(z->instsamp)))
			{
				p3 = fluid_list_nth(sf->inst, i - 1);
				if (!p3)
					return (FGERR2(ErrCorr,
					_("Preset %03d %03d: Invalid instrument reference"),
					((SFPreset *)(p->data))->bank,
					((SFPreset *)(p->data))->prenum));
				z->instsamp = p3;
			}
			else
				z->instsamp = NULL;
			p2 = fluid_list_next(p2);
		}
		p = fluid_list_next(p);
	}

	return (OK);
}

static int fixup_igen(SFData * sf)
{
	fluid_list_t *p, *p2, *p3;
	SFZone *z;
	int i;

	p = sf->inst;
	while (p)
	{
		p2 = ((SFInst *)(p->data))->zone;
		while (p2)
		{
			z = (SFZone *)(p2->data);
			if ((i = GPOINTER_TO_INT(z->instsamp)))
			{
				p3 = fluid_list_nth(sf->sample, i - 1);
				if (!p3)
					return (FGERR1(ErrCorr,
					_("Instrument \"%s\": Invalid sample reference"),
					((SFInst *)(p->data))->name));
				z->instsamp = p3;
			}
			p2 = fluid_list_next(p2);
		}
		p = fluid_list_next(p);
	}

	return (OK);
}

static int fixup_sample(SFData * sf)
{
	fluid_list_t *p;
	SFSample *sam;

	p = sf->sample;
	while (p)
	{
		sam = (SFSample *)(p->data);

		if ((!(sam->sampletype & FLUID_SAMPLETYPE_ROM)
			&& sam->end > sdtachunk_size) || sam->start > (sam->end - 4))
		{
			FLUID_LOG1(FLUID_WARN, _("Sample '%s' start/end file positions are invalid,"
				" disabling and will not be saved"), sam->name);

			sam->start = sam->end = sam->loopstart = sam->loopend = 0;

			return (OK);
		}
		else if (sam->loopend > sam->end || sam->loopstart >= sam->loopend
			|| sam->loopstart <= sam->start)
		{
			if ((sam->end - sam->start) >= 20)
			{
				sam->loopstart = sam->start + 8;
				sam->loopend = sam->end - 8;
			}
			else
			{
				sam->loopstart = sam->start + 1;
				sam->loopend = sam->end - 1;
			}
		}

		sam->end -= sam->start + 1;
		sam->loopstart -= sam->start;
		sam->loopend -= sam->start;

		p = fluid_list_next(p);
	}

	return (OK);
}

#define PRESET_CHUNK_OPTIMUM_AREA	256
#define INST_CHUNK_OPTIMUM_AREA		256
#define SAMPLE_CHUNK_OPTIMUM_AREA	256
#define ZONE_CHUNK_OPTIMUM_AREA		256
#define MOD_CHUNK_OPTIMUM_AREA		256
#define GEN_CHUNK_OPTIMUM_AREA		256

static unsigned short badgen[] = { Gen_Unused1, Gen_Unused2, Gen_Unused3, Gen_Unused4,
Gen_Reserved1, Gen_Reserved2, Gen_Reserved3, 0
};

static unsigned short badpgen[] = { Gen_StartAddrOfs, Gen_EndAddrOfs, Gen_StartLoopAddrOfs,
Gen_EndLoopAddrOfs, Gen_StartAddrCoarseOfs, Gen_EndAddrCoarseOfs,
Gen_StartLoopAddrCoarseOfs, Gen_Keynum, Gen_Velocity,
Gen_EndLoopAddrCoarseOfs, Gen_SampleModes, Gen_ExclusiveClass,
Gen_OverrideRootKey, 0
};

void sfont_close(SFData * sf)
{
	fluid_list_t *p, *p2;

	p = sf->info;
	while (p)
	{
		free(p->data);
		p = fluid_list_next(p);
	}
	delete_fluid_list(sf->info);
	sf->info = NULL;

	p = sf->preset;
	while (p)
	{
		p2 = ((SFPreset *)(p->data))->zone;
		while (p2)
		{
			sfont_free_zone((SFZone*)p2->data);
			p2 = fluid_list_next(p2);
		}
		delete_fluid_list(((SFPreset *)(p->data))->zone);
		FLUID_FREE(p->data);
		p = fluid_list_next(p);
	}
	delete_fluid_list(sf->preset);
	sf->preset = NULL;

	p = sf->inst;
	while (p)
	{
		p2 = ((SFInst *)(p->data))->zone;
		while (p2)
		{
			sfont_free_zone((SFZone*)p2->data);
			p2 = fluid_list_next(p2);
		}
		delete_fluid_list(((SFInst *)(p->data))->zone);
		FLUID_FREE(p->data);
		p = fluid_list_next(p);
	}
	delete_fluid_list(sf->inst);
	sf->inst = NULL;

	p = sf->sample;
	while (p)
	{
		FLUID_FREE(p->data);
		p = fluid_list_next(p);
	}
	delete_fluid_list(sf->sample);
	sf->sample = NULL;

	FLUID_FREE(sf);
}

void sfont_free_zone(SFZone * zone)
{
	fluid_list_t *p;

	if (!zone)
		return;

	p = zone->gen;
	while (p)
	{
		if (p->data)
			FLUID_FREE(p->data);
		p = fluid_list_next(p);
	}
	delete_fluid_list(zone->gen);

	p = zone->mod;
	while (p)
	{
		if (p->data)
			FLUID_FREE(p->data);
		p = fluid_list_next(p);
	}
	delete_fluid_list(zone->mod);

	FLUID_FREE(zone);
}

int sfont_preset_compare_func(void* a, void* b)
{
	int aval, bval;

	aval = (int)(((SFPreset *)a)->bank) << 16 | ((SFPreset *)a)->prenum;
	bval = (int)(((SFPreset *)b)->bank) << 16 | ((SFPreset *)b)->prenum;

	return (aval - bval);
}

void sfont_zone_delete(SFData * sf, fluid_list_t ** zlist, SFZone * zone)
{
	*zlist = fluid_list_remove(*zlist, (void*)zone);
	sfont_free_zone(zone);
}

fluid_list_t *
gen_inlist(int gen, fluid_list_t * genlist)
{
	fluid_list_t *p;

	p = genlist;
	while (p)
	{
		if (p->data == NULL)
			return (NULL);
		if (gen == ((SFGen *)p->data)->id)
			break;
		p = fluid_list_next(p);
	}
	return (p);
}

int gen_valid(int gen)
{
	int i = 0;

	if (gen > Gen_MaxValid)
		return (FALSE);
	while (badgen[i] && badgen[i] != gen)
		i++;
	return (badgen[i] == 0);
}

int gen_validp(int gen)
{
	int i = 0;

	if (!gen_valid(gen))
		return (FALSE);
	while (badpgen[i] && badpgen[i] != (unsigned short)gen)
		i++;
	return (badpgen[i] == 0);
}

int safe_fread(void *buf, int count, ZL_RWops* fd)
{
	if (fd->read(buf, count, 1) != 1)
	{
		FLUID_LOG(FLUID_ERR, _("File read failed"));
		return (FAIL);
	}
	return (OK);
}

int safe_fseek(ZL_RWops* fd, long ofs, int whence)
{
	if (fd->seektell(ofs, whence) == -1) {
		FLUID_LOG2(FLUID_ERR, _("File seek failed with offset = %ld and whence = %d"), ofs, whence);
		return (FAIL);
	}
	return (OK);
}

//
//MERGED FILE END: fluid_zlsfont.cpp
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: fluid_zldriver.cpp
//

#include <ZL_Audio.h>

static fluid_synth_t* mysynth;
#define FLUID_ZL_CHANNELS 2
#define FLUID_ZL_RATE 44100

static bool mix_music(char *stream, int len)
{
	if (mysynth->state != FLUID_SYNTH_PLAYING) return false;
	fluid_synth_write_s16(mysynth, len / (2 * FLUID_ZL_CHANNELS), stream, 0, FLUID_ZL_CHANNELS, stream, 1, FLUID_ZL_CHANNELS);
	return true;
}

fluid_audio_driver_t* new_fluid_audio_driver(fluid_synth_t* synth)
{
	fluid_audio_driver_t* dev = NULL;

	synth->sample_rate = FLUID_ZL_RATE;
	mysynth = synth;

	ZL_Audio::HookMusicMix(&mix_music);

	return (fluid_audio_driver_t*)dev;
}

FLUIDSYNTH_API void delete_fluid_audio_driver(fluid_audio_driver_t* p)
{
	if (p) FLUID_FREE(p);
}

//
//MERGED FILE END: fluid_zldriver.cpp
//------------------------------------------------------------------------------------------------------------------------------------------------------

#include "ZL_FluidSynth.h"

static fluid_synth_t *pFluidSynth = NULL;

#define ZL_SYNTH_FLUID_FULL_GAIN_VALUE 0.5f

void ZL_FluidSynth::InitSynth(const ZL_File& SoundFontFile)
{
	if (pFluidSynth) { new_fluid_audio_driver(pFluidSynth); return; }

	pFluidSynth = new_fluid_synth();
	fluid_synth_sfload(pFluidSynth, (char*)&SoundFontFile, 0);
	fluid_synth_set_reverb_on(pFluidSynth, 0);
	fluid_synth_set_chorus_on(pFluidSynth, 0);
	new_fluid_audio_driver(pFluidSynth);
	SetSynthGain(1);
}

void ZL_FluidSynth::SetSynthGain(float gain)
{
	if (!pFluidSynth) return;
	fluid_synth_set_gain(pFluidSynth, ZL_SYNTH_FLUID_FULL_GAIN_VALUE * gain);
}

void ZL_FluidSynth::NoteOn(unsigned char chan, int key, int vel)
{
	if (!pFluidSynth) return;
	fluid_synth_noteon(pFluidSynth, chan, key, vel);
}

void ZL_FluidSynth::NoteOff(unsigned char chan, int key)
{
	if (!pFluidSynth) return;
	fluid_synth_noteoff(pFluidSynth, chan, key);
}

void ZL_FluidSynth::StopAllNotes()
{
	if (!pFluidSynth) return;
	fluid_synth_program_reset(pFluidSynth);
	fluid_synth_system_reset(pFluidSynth);
}

void ZL_FluidSynth::SynthEvent(unsigned char chan, unsigned char type, unsigned int param1, unsigned int param2)
{
	if (!pFluidSynth) return;
	switch (type)
	{
		case CONTROL_CHANGE: fluid_synth_cc(pFluidSynth, chan, param1, param2); break;
		case PROGRAM_CHANGE: fluid_synth_program_change(pFluidSynth, chan, param1); break;
		case PITCH_BEND:     fluid_synth_pitch_bend(pFluidSynth, chan, param1); break;
	}
}
