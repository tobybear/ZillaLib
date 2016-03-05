#ifndef _SDL_CONFIG_H
#define _SDL_CONFIG_H

#include "SDL_platform.h"

#if defined(__WIN32__)
#define HAVE_LIBC //Avoid SDL_CreateThread mockery
#include "SDL_config_windows.h"
#elif defined(__MACOSX__)
#include "SDL_config_macosx.h"
#elif defined(__LINUX__)
#include "SDL_config_linux.h"
#else
#include "SDL_config_minimal.h"
#endif

#include "SDL_config_zillalib.h"

#endif
