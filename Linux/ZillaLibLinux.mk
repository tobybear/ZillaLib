#
#  ZillaLib
#  Copyright (C) 2010-2025 Bernhard Schelling
#
#  This software is provided 'as-is', without any express or implied
#  warranty.  In no event will the authors be held liable for any damages
#  arising from the use of this software.
#
#  Permission is granted to anyone to use this software for any purpose,
#  including commercial applications, and to alter it and redistribute it
#  freely, subject to the following restrictions:
#
#  1. The origin of this software must not be misrepresented; you must not
#     claim that you wrote the original software. If you use this software
#     in a product, an acknowledgment in the product documentation would be
#     appreciated but is not required.
#  2. Altered source versions must be plainly marked as such, and must not be
#     misrepresented as being the original software.
#  3. This notice may not be removed or altered from any source distribution.
#

sp := $(*NOTEXIST*) $(*NOTEXIST*)
THIS_MAKEFILE := $(subst \,/,$(lastword $(MAKEFILE_LIST)))
ZILLALIB_DIR = $(subst |,/,$(subst <,,$(subst <.|,,<$(subst /,|,$(dir $(subst $(sp),/,$(strip $(subst /,$(sp),$(if $(filter ./,$(dir $(THIS_MAKEFILE))),../a,$(dir $(THIS_MAKEFILE)))))))))))
-include $(dir $(THIS_MAKEFILE))ZillaAppLocalConfig.mk

#------------------------------------------------------------------------------------------------------

# Build toolchain
CXX:=g++
CC:=gcc
AR:=ar
STRIP:=strip

# Build global settings
APPFLAGS    := -I$(ZILLALIB_DIR)Include
ZLFLAGS     := -I$(ZILLALIB_DIR)Include -I$(ZILLALIB_DIR)Source/zlib
WARNINGS    := -pedantic -Wall -Wno-long-long -Wno-error=unused-parameter -Wno-pedantic -Wno-unused-local-typedefs -Wno-unused-function -Wno-strict-aliasing
DEPWARNINGS := -Wno-main -Wno-empty-body -Wno-char-subscripts -Wno-sign-compare -Wno-unused-value -Wno-unused-variable -Wno-unused-but-set-variable -Wno-nonnull -Wno-stringop-truncation -Werror
CFLAGS      := $(WARNINGS) -pthread -msse -mfpmath=sse -ffast-math -fomit-frame-pointer -fvisibility=hidden -fno-exceptions -fno-non-call-exceptions -ffunction-sections -fdata-sections
LDFLAGS     := -lGL -lpthread -ldl -Wl,--gc-sections
CXXFLAGS    := -std=c++11 -fno-rtti
CCFLAGS     := -std=gnu99 -D_DEFAULT_SOURCE

ifeq ($(BUILD),RELEASE)
  ZLOUTDIR  := $(ZILLALIB_DIR)Linux/build-release
  APPOUTDIR := Release-linux
  CFLAGS    += -DNDEBUG -O2 -s
  LDFLAGS   += -O2 -s -Wl,--strip-all -fno-ident
else ifeq ($(BUILD),RELEASEDBG)
  ZLOUTDIR  := $(ZILLALIB_DIR)Linux/build-releasedbg
  APPOUTDIR := ReleaseDbg-linux
  CFLAGS    += -DNDEBUG -ggdb -O2
  LDFLAGS   += -ggdb -O2
else
  ZLOUTDIR  := $(ZILLALIB_DIR)Linux/build-debug
  APPOUTDIR := Debug-linux
  CFLAGS    += -DDEBUG -D_DEBUG -DZILLALOG -g -O0
  LDFLAGS   += -g -O0
endif

ifdef USE_EXTERNAL_SDL2
  ifeq ($(if $(EXTERNAL_SDL2_INCLUDE),$(wildcard $(EXTERNAL_SDL2_INCLUDE)/SDL.h),),)
    $(error External SDL2 includes not found in set path ($(EXTERNAL_SDL2_INCLUDE)). Fix path in $(dir $(THIS_MAKEFILE))ZillaAppLocalConfig.mk with EXTERNAL_SDL2_INCLUDE = /path/to/SDL2/include)
  endif
  ifeq ($(if $(EXTERNAL_SDL2_SO),$(wildcard $(EXTERNAL_SDL2_SO)),),)
    $(error External SDL2 .so file not found in set path ($(EXTERNAL_SDL2_SO)). Fix path in $(dir $(THIS_MAKEFILE))ZillaAppLocalConfig.mk with EXTERNAL_SDL2_SO = /path/to/libSDL2.so)
  endif
  ZLFLAGS   += -DZL_USE_EXTERNAL_SDL -I$(EXTERNAL_SDL2_INCLUDE)
  LDFLAGS   += $(EXTERNAL_SDL2_SO)
else
  ZLFLAGS   += -I$(ZILLALIB_DIR)Source/sdl/include
endif

#add defines from the make command line (e.g. D=MACRO=VALUE)
APPFLAGS += $(subst \\\,$(sp),$(foreach F,$(subst \$(sp),\\\,$(D)),"-D$(F)"))

# Compute tool paths
PYTHON := python3
ifeq ($(shell $(PYTHON) -c "print(1)" 2>/dev/null),)
  PYTHON := python
  ifeq ($(shell $(PYTHON) -c "print(1)" 2>/dev/null),)
    PYTHON := python2
    ifeq ($(shell $(PYTHON) -c "print(1)" 2>/dev/null),)
      $(error Python executable not found in PATH)
    endif
  endif
endif

# Python one liner to delete all .o files when the dependency files were created empty due to compile error
CMD_DEL_OLD_OBJ := $(PYTHON) -c "import sys,os;[os.path.exists(a) and os.path.getsize(a)==0 and os.path.exists(a.rstrip('d')+'o') and os.remove(a.rstrip('d')+'o') for a in sys.argv[1:]]"
CMD_DEL_FILES := $(PYTHON) -c "import sys,os;[os.path.exists(a) and os.remove(a) for a in sys.argv[1:]]"

# Disable DOS PATH warning when using Cygwin based tools Windows
CYGWIN ?= nodosfilewarning
export CYGWIN

CPUTYPE  := $(if $(if $(M32),,$(or $(M64),$(findstring 64,$(shell uname -m)))),x86_64,x86_32)
OBJEXT   := $(if $(filter $(CPUTYPE),x86_32),_32.o,_64.o)
GCCMFLAG := $(if $(filter $(CPUTYPE),x86_32),-m32,-m64)

#------------------------------------------------------------------------------------------------------
ifdef ZillaApp
#------------------------------------------------------------------------------------------------------

ZL_IS_APP_MAKE = 1
-include Makefile
APPSOURCES := $(wildcard *.cpp *.c)
-include sources.mk
APPSOURCES += $(foreach F, $(ZL_ADD_SRC_FILES), $(wildcard $(F)))
ifeq ($(APPSOURCES),)
  $(error No source files found for $(ZillaApp))
endif

ifeq ($(findstring RELEASE,$(BUILD)),RELEASE)
-include assets.mk
ASSET_ALL_PATHS := $(strip $(foreach F,$(ASSETS),$(wildcard ./$(F)) $(wildcard ./$(F)/*) $(wildcard ./$(F)/*/*) $(wildcard ./$(F)/*/*/*) $(wildcard ./$(F)/*/*/*/*) $(wildcard ./$(F)/*/*/*/*/*)))
ASSET_ALL_STARS := $(if $(ASSET_ALL_PATHS),$(strip $(foreach F,$(subst *./, ,*$(subst $(sp),*,$(ASSET_ALL_PATHS))),$(if $(wildcard $(subst *,\ ,$(F))/.),,$(F)))))
ASSET_ZIP       := $(if $(ASSET_ALL_STARS),$(if $(ZLLINUX_ASSETS_OUTFILE),$(APPOUTDIR)/$(ZLLINUX_ASSETS_OUTFILE),$(APPOUTDIR)/$(ZillaApp).dat),)
endif

ifeq ($(if $(ASSET_ZIP),$(ZLLINUX_ASSETS_EMBED),),1)
APPOUTBIN := $(APPOUTDIR)/$(ZillaApp)_$(CPUTYPE)_WithData
else
APPOUTBIN := $(APPOUTDIR)/$(ZillaApp)_$(CPUTYPE)
endif

.PHONY: all clean run gdb
all: $(APPOUTBIN) $(ASSET_ZIP)

MAKEAPPOBJ = $(APPOUTDIR)/$(basename $(notdir $(1)))$(OBJEXT): $(1) ; $$(call COMPILE,$$@,$$<,$(2),$(GCCMFLAG) $(3),$$(APPFLAGS) -MMD -MP)

APPOBJS := $(addprefix $(APPOUTDIR)/,$(notdir $(patsubst %.c,%$(OBJEXT),$(patsubst %.cpp,%$(OBJEXT),$(APPSOURCES)))))
$(shell $(CMD_DEL_OLD_OBJ) $(APPOBJS:%.o=%.d))
-include $(APPOBJS:%.o=%.d)
$(foreach F,$(filter %.cpp,$(APPSOURCES)),$(eval $(call MAKEAPPOBJ,$(F),$$(CXX),$$(CXXFLAGS))))
$(foreach F,$(filter %.c  ,$(APPSOURCES)),$(eval $(call MAKEAPPOBJ,$(F),$$(CC),$$(CCFLAGS))))

clean:
	$(info Removing all build files ...)
	@$(CMD_DEL_FILES) $(APPOBJS) $(APPOBJS:%.o=%.d) $(ASSET_ZIP) $(APPOUTDIR)/$(ZillaApp)_$(CPUTYPE) $(APPOUTDIR)/$(ZillaApp)_$(CPUTYPE)_WithData

run: $(APPOUTBIN) $(ASSET_ZIP)
	$(APPOUTBIN)

gdb: $(APPOUTBIN) $(ASSET_ZIP)
	gdb $(APPOUTBIN)

$(ASSET_ZIP) : $(if $(ASSET_ALL_STARS),assets.mk $(subst *,\ ,$(ASSET_ALL_STARS)))
	$(info Building $@ with $(words $(ASSET_ALL_STARS)) assets ...)
	@$(PYTHON) -c "import sys,zipfile;z=zipfile.ZipFile('$@','w');[z.write(f) for f in sys.argv[1:]]" $(subst *, ,$(subst $(sp)," ","$(ASSET_ALL_STARS)"))

$(APPOUTDIR)/$(ZillaApp)_$(CPUTYPE) : $(APPOBJS) $(ZLOUTDIR)/ZillaLib_$(CPUTYPE).a
	$(info Linking $@ ...)
	$(CXX) -o $@ $^ $(GCCMFLAG) $(LDFLAGS)
	@-$(if $(filter RELEASE,$(BUILD)),$(if $(STRIP),$(STRIP) -R .comment $@))

$(APPOUTDIR)/$(ZillaApp)_$(CPUTYPE)_WithData : $(APPOUTDIR)/$(ZillaApp)_$(CPUTYPE) $(ASSET_ZIP)
	$(info packing data assets into $@ ...)
	@cat $(APPOUTDIR)/$(ZillaApp)_$(CPUTYPE) >$@
	@cat $(ASSET_ZIP) >>$@
	@chmod 755 $@

#------------------------------------------------------------------------------------------------------
else
#------------------------------------------------------------------------------------------------------

.PHONY: all clean

all: $(ZLOUTDIR)/ZillaLib_$(CPUTYPE).a

clean:
	$(info Removing all build files ...)
	@$(CMD_DEL_FILES) $(ZLOUTDIR)/ZillaLib_$(CPUTYPE).a $(CPP_ZLOBJS) $(CPP_DEPOBJS) $(CC_DEPOBJS) $(CPP_ZLOBJS:%.o=%.d) $(CPP_DEPOBJS:%.o=%.d) $(CC_DEPOBJS:%.o=%.d)

#------------------------------------------------------------------------------------------------------
endif
#------------------------------------------------------------------------------------------------------

ZLSOURCES := $(wildcard $(ZILLALIB_DIR)Source/*.cpp)

DEPSOURCES := \
	$(wildcard $(ZILLALIB_DIR)Source/zlib/*.c) \
	$(wildcard $(ZILLALIB_DIR)Source/libtess2/*.c) \
	$(wildcard $(ZILLALIB_DIR)Source/enet/*.c) \
	$(wildcard $(ZILLALIB_DIR)Source/stb/*.cpp)

ifndef USE_EXTERNAL_SDL2
DEPSOURCES += \
	$(wildcard $(ZILLALIB_DIR)Source/sdl/audio/SDL_audio.c) \
	$(wildcard $(ZILLALIB_DIR)Source/sdl/audio/SDL_audiodev.c) \
	$(wildcard $(ZILLALIB_DIR)Source/sdl/audio/alsa/SDL_alsa_audio.c) \
	$(wildcard $(ZILLALIB_DIR)Source/sdl/audio/dsp/SDL_dspaudio.c) \
	$(wildcard $(ZILLALIB_DIR)Source/sdl/events/*.c) \
	$(wildcard $(ZILLALIB_DIR)Source/sdl/joystick/SDL_joystick.c) \
	$(wildcard $(ZILLALIB_DIR)Source/sdl/joystick/linux/SDL_sysjoystick.c) \
	$(wildcard $(ZILLALIB_DIR)Source/sdl/loadso/dlopen/SDL_sysloadso.c) \
	$(wildcard $(ZILLALIB_DIR)Source/sdl/thread/SDL_thread.c) \
	$(wildcard $(ZILLALIB_DIR)Source/sdl/thread/pthread/*.c) \
	$(wildcard $(ZILLALIB_DIR)Source/sdl/timer/unix/SDL_systimer.c) \
	$(wildcard $(ZILLALIB_DIR)Source/sdl/video/SDL_rect.c) \
	$(wildcard $(ZILLALIB_DIR)Source/sdl/video/SDL_video.c) \
	$(wildcard $(ZILLALIB_DIR)Source/sdl/video/x11/*.c)
endif

ZLSOURCES := $(subst $(ZILLALIB_DIR),,$(ZLSOURCES))
DEPSOURCES := $(subst $(ZILLALIB_DIR),,$(DEPSOURCES))

CPP_DEPOBJS := $(addprefix $(ZLOUTDIR)/,$(patsubst %.cpp,%$(OBJEXT),$(filter %.cpp,$(DEPSOURCES))))
CC_DEPOBJS  := $(addprefix $(ZLOUTDIR)/,$(patsubst   %.c,%$(OBJEXT),$(filter   %.c,$(DEPSOURCES))))
$(CPP_DEPOBJS) : $(ZLOUTDIR)/%$(OBJEXT) : $(ZILLALIB_DIR)%.cpp ; $(call COMPILE,$@,$<,$(CXX),$(GCCMFLAG) $(CXXFLAGS) $(ZLFLAGS) $(DEPWARNINGS))
$(CC_DEPOBJS)  : $(ZLOUTDIR)/%$(OBJEXT) : $(ZILLALIB_DIR)%.c   ; $(call   COMPILE,$@,$<,$(CC),$(GCCMFLAG) $(CCFLAGS) $(ZLFLAGS) $(DEPWARNINGS))

CPP_ZLOBJS  := $(addprefix $(ZLOUTDIR)/,$(patsubst %.cpp,%$(OBJEXT),$(filter %.cpp,$(ZLSOURCES))))
$(shell $(CMD_DEL_OLD_OBJ) $(CPP_ZLOBJS:%.o=%.d))
-include $(CPP_ZLOBJS:%.o=%.d)
$(CPP_ZLOBJS)  : $(ZLOUTDIR)/%$(OBJEXT) : $(ZILLALIB_DIR)%.cpp ; $(call COMPILE,$@,$<,$(CXX),$(GCCMFLAG) $(CXXFLAGS) $(ZLFLAGS) -MMD -MP)

$(ZLOUTDIR)/ZillaLib_$(CPUTYPE).a : $(CPP_ZLOBJS) $(CPP_DEPOBJS) $(CC_DEPOBJS)
	$(info Creating static library $@ ...)
	@$(AR) rcs $@ $^

#------------------------------------------------------------------------------------------------------

define COMPILE
	$(info $2)
	@$(if $(wildcard $(dir $1)),,$(shell $(PYTHON) -c "import os;os.makedirs('$(dir $1)')"))
	@$3 $4 $(CFLAGS) $5 $(COMMONFLAGS) -o $1 -c $2
endef
