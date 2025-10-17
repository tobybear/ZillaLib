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
WARNINGS    := -pedantic -Wall -Wno-long-long -Wno-error=unused-parameter -Wno-pedantic -Wno-unused-local-typedefs -Wno-unused-function -Wno-strict-aliasing -Wno-psabi
DEPWARNINGS := -Wno-main -Wno-empty-body -Wno-char-subscripts -Wno-sign-compare -Wno-unused-value -Wno-unused-variable -Wno-unused-but-set-variable -Wno-nonnull -Wno-stringop-truncation -Werror
CFLAGS      := $(WARNINGS) -pthread -ffast-math -fomit-frame-pointer -fvisibility=hidden -fno-exceptions -fno-non-call-exceptions -ffunction-sections 
LDFLAGS     := -static -static-libgcc -static-libstdc++ -Wl,--gc-sections -lgdi32 -lopengl32 -lWinmm -lOle32 -lstdc++
CXXFLAGS    := -std=gnu++11 -fno-rtti -D_DEFAULT_SOURCE -D_POSIX_SOURCE
CCFLAGS     := -std=gnu11 -D_DEFAULT_SOURCE -D_POSIX_SOURCE

ifeq ($(CC),clang)
  CFLAGS    += -Wno-unknown-warning-option -Wno-error=unused-but-set-variable -Wno-error=unused-variable -Wno-deprecated-register -Wno-unused-but-set-variable -Wno-unused-variable
  CFLAGS    += -fdata-sections #saves around 3 kb on clang but wrongfully adds up to 50MB on gcc
endif

ifeq ($(BUILD),RELEASE)
  ZLOUTDIR  := $(ZILLALIB_DIR)Linux/build-msys-release
  APPOUTDIR := Release-msys
  CFLAGS    += -DNDEBUG -g0 -O2 -fno-ident
  LDFLAGS   += -O2 -s -Wl,--strip-all -Wl,--subsystem,windows
else ifeq ($(BUILD),RELEASESMALL)
  ZLOUTDIR  := $(ZILLALIB_DIR)Linux/build-msys-releasesmall
  APPOUTDIR := ReleaseSmall-msys
  CFLAGS    += -DNDEBUG -ggdb -Os -fno-ident
  LDFLAGS   += -Os -s -Wl,--strip-all
else ifeq ($(BUILD),RELEASEDBG)
  ZLOUTDIR  := $(ZILLALIB_DIR)Linux/build-msys-releasedbg
  APPOUTDIR := ReleaseDbg-msys
  CFLAGS    += -DNDEBUG -ggdb -O2
  LDFLAGS   += -ggdb -O2
else ifeq ($(BUILD),ASAN)
  ZLOUTDIR  := $(ZILLALIB_DIR)Linux/build-msys-asan
  APPOUTDIR := ASAN-msys
  CFLAGS    += -DDEBUG -D_DEBUG -DZILLALOG -g -O0 -fsanitize=address -fno-omit-frame-pointer
  LDFLAGS   += -g -O0 -fsanitize=address -latomic
else
  ZLOUTDIR  := $(ZILLALIB_DIR)Linux/build-msys-debug
  APPOUTDIR := Debug-msys
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

UNAME    := $(or $(subst ARM,arm,$(MSYSTEM)),$(shell uname -m))
CPUTYPE  := $(if $(or $(findstring arm,$(UNAME)),$(findstring aarch,$(UNAME))),arm_$(or $(findstring 64,$(UNAME)),32),x86_$(if $(if $(M32),,$(or $(M64),$(findstring 64,$(UNAME)))),64,32))
OBJEXT   := $(if $(findstring 32,$(CPUTYPE)),_32.o,_64.o)
GCCMFLAG :=

ifeq ($(findstring x86,$(CPUTYPE)),x86)
  GCCMFLAG := $(if $(findstring 32,$(CPUTYPE)),-m32,-m64)
  CFLAGS += -msse -mfpmath=sse
endif

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
ASSET_ZIP       := $(if $(ASSET_ALL_STARS),$(if $(ZLMSYS_ASSETS_OUTFILE),$(APPOUTDIR)/$(ZLMSYS_ASSETS_OUTFILE),$(APPOUTDIR)/$(ZillaApp).dat),)
endif

ifeq ($(if $(ASSET_ZIP),$(ZLMSYS_ASSETS_EMBED),),1)
APPOUTBIN := $(APPOUTDIR)/$(ZillaApp)_$(CPUTYPE)_WithData.exe
else
APPOUTBIN := $(APPOUTDIR)/$(ZillaApp)_$(CPUTYPE).exe
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
	@$(CMD_DEL_FILES) $(APPOBJS) $(APPOBJS:%.o=%.d) $(ASSET_ZIP) $(APPOUTDIR)/$(ZillaApp)_$(CPUTYPE).exe $(APPOUTDIR)/$(ZillaApp)_$(CPUTYPE)_WithData.exe $(APPOUTDIR)/$(ZillaApp).rc_$(CPUTYPE).o

run: $(APPOUTBIN) $(ASSET_ZIP)
	$(APPOUTBIN)

gdb: $(APPOUTBIN) $(ASSET_ZIP)
	gdb $(APPOUTBIN)

$(ASSET_ZIP) : $(if $(ASSET_ALL_STARS),assets.mk $(subst *,\ ,$(ASSET_ALL_STARS)))
	$(info Building $@ with $(words $(ASSET_ALL_STARS)) assets ...)
	@$(PYTHON) -c "import sys,zipfile;z=zipfile.ZipFile('$@','w');[z.write(f) for f in sys.argv[1:]]" $(subst *, ,$(subst $(sp)," ","$(ASSET_ALL_STARS)"))

$(APPOUTDIR)/$(ZillaApp).rc_$(CPUTYPE).o : $(ZillaApp).rc
	$(info Compiling $@ ...)
	windres  -i $^ -o $@ $(filter -I%,$(APPFLAGS)) $(filter -D%,$(APPFLAGS))

$(APPOUTDIR)/$(ZillaApp)_$(CPUTYPE).exe : $(APPOBJS) $(ZLOUTDIR)/ZillaLib_$(CPUTYPE).a $(APPOUTDIR)/$(ZillaApp).rc_$(CPUTYPE).o
	$(info Linking $@ ...)
	$(CXX) -o $@ $^ $(GCCMFLAG) $(LDFLAGS)
	@-$(if $(filter RELEASE,$(BUILD)),$(if $(STRIP),$(STRIP) -R .comment $@))

$(APPOUTDIR)/$(ZillaApp)_$(CPUTYPE)_WithData.exe : $(APPOUTDIR)/$(ZillaApp)_$(CPUTYPE).exe $(ASSET_ZIP)
	$(info Packing data assets into $@ ...)
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
	$(wildcard $(ZILLALIB_DIR)Source/sdl/audio/directsound/SDL_directsound.c) \
	$(wildcard $(ZILLALIB_DIR)Source/sdl/audio/xaudio2/SDL_xaudio2.c) \
	$(wildcard $(ZILLALIB_DIR)Source/sdl/events/*.c) \
	$(wildcard $(ZILLALIB_DIR)Source/sdl/joystick/SDL_joystick.c) \
	$(wildcard $(ZILLALIB_DIR)Source/sdl/joystick/windows/SDL_dxjoystick.c) \
	$(wildcard $(ZILLALIB_DIR)Source/sdl/loadso/windows/SDL_sysloadso.c) \
	$(wildcard $(ZILLALIB_DIR)Source/sdl/thread/SDL_thread.c) \
	$(wildcard $(ZILLALIB_DIR)Source/sdl/thread/generic/SDL_syscond.c) \
	$(wildcard $(ZILLALIB_DIR)Source/sdl/thread/windows/*.c) \
	$(wildcard $(ZILLALIB_DIR)Source/sdl/timer/windows/*.c) \
	$(wildcard $(ZILLALIB_DIR)Source/sdl/video/*.c) \
	$(wildcard $(ZILLALIB_DIR)Source/sdl/video/windows/*.c)
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
