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

#ifdef ZL_USE_POSIXTIME

#include <time.h>
#include <stdlib.h>
#include <sys/time.h>

#ifdef ZL_USE_CLOCKGETTIME
static struct timespec tstart;
#else
static struct timeval tstart;
#endif

void ZL_StartTicks()
{
	#ifdef ZL_USE_CLOCKGETTIME
	clock_gettime(CLOCK_MONOTONIC, &tstart);
	#else
	gettimeofday(&tstart, NULL);
	#endif
}

ticks_t ZL_GetTicks()
{
	#ifdef ZL_USE_CLOCKGETTIME
	struct timespec tnow;
	clock_gettime(CLOCK_MONOTONIC, &tnow);
	return (ticks_t)((tnow.tv_sec - tstart.tv_sec) * 1000 + (tnow.tv_nsec - tstart.tv_nsec) / 1000000);
	#else
	struct timeval tnow;
	gettimeofday(&tnow, NULL);
	return (ticks_t)((tnow.tv_sec - tstart.tv_sec) * 1000 + (tnow.tv_usec - tstart.tv_usec) / 1000);
	#endif
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

#endif

#ifdef ZL_USE_PTHREAD

pthread_t ZL_CreateThread(void *(*start_routine) (void *p), void *arg)
{
	pthread_t ret;
	pthread_create(&ret, NULL, start_routine, arg);
	return ret;
}

void ZL_WaitThread(pthread_t pthread, int *pstatus)
{
	pthread_join(pthread, (void**)&pstatus);
}

#endif
