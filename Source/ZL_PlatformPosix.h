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

#ifndef __ZL_PLATFORM_POSIX__
#define __ZL_PLATFORM_POSIX__

//Thread
#if defined(ZL_USE_PTHREAD)

#include <pthread.h>

#define ZL_ThreadHandle pthread_t
pthread_t ZL_CreateThread(void *(*start_routine) (void *p), void *arg);
void ZL_WaitThread(pthread_t pthread, int *pstatus);

#define ZL_MutexHandle pthread_mutex_t
#define ZL_MutexLock(m) pthread_mutex_lock(&m)
#define ZL_MutexUnlock(m) pthread_mutex_unlock(&m)
#define ZL_MutexTryLock(m) (pthread_mutex_trylock(&m)==0)
#define ZL_MutexInit(m) { pthread_mutexattr_t m_attr; pthread_mutexattr_init(&m_attr); pthread_mutexattr_settype(&m_attr, PTHREAD_MUTEX_RECURSIVE); pthread_mutex_init(&m, &m_attr); }
#define ZL_MutexDestroy(m) pthread_mutex_destroy(&m)
#define ZL_MutexNone PTHREAD_MUTEX_INITIALIZER
#define ZL_MutexIsNone(m) (0)
#endif

#endif
