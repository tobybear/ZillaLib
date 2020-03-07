#
#  ZillaLib
#  Copyright (C) 2010-2020 Bernhard Schelling
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
THIS_MAKEFILE := $(patsubst ./%,%,$(subst \,/,$(lastword $(MAKEFILE_LIST))))
ISWIN := $(findstring :,$(firstword $(subst \, ,$(subst /, ,$(abspath .)))))
ZILLALIB_DIR = $(or $(subst |,/,$(subst <,,$(subst <.|,,<$(subst /,|,$(dir $(subst $(sp),/,$(strip $(subst /,$(sp),$(dir $(THIS_MAKEFILE)))))))))),$(if $(subst ./,,$(dir $(THIS_MAKEFILE))),,../))
sub_checkexe_run = $(if $(1),$(if $(shell "$(1)" $(2) 2>$(if $(ISWIN),nul,/dev/null)),$(1),),)
-include $(dir $(THIS_MAKEFILE))ZillaAppLocalConfig.mk
ifeq ($(and $(LLVM_ROOT),$(SYSTEM_ROOT),$(WASMOPT)),)
  $(info )
  $(info Please create the file $(dir $(THIS_MAKEFILE))ZillaAppLocalConfig.mk with at least the following definitions:)
  $(info )
  $(info LLVM_ROOT = $(if $(ISWIN),d:)/path/to/llvm)
  $(info SYSTEM_ROOT = $(if $(ISWIN),d:)/path/to/emscripten/system)
  $(info WASMOPT = $(if $(ISWIN),d:)/path/to/wasm-opt$(if $(ISWIN),.exe))
  $(info )
  $(error Aborting)
endif

#------------------------------------------------------------------------------------------------------

ifeq ($(BUILD),RELEASE)
  LIBOUTDIR := $(dir $(THIS_MAKEFILE))build
  SYSOUTDIR := $(dir $(THIS_MAKEFILE))build
  APPOUTDIR := Release-wasm
  DBGCFLAGS := -DNDEBUG
  LDFLAGS   := -strip-all -gc-sections
  WOPTFLAGS := -O3
else
  LIBOUTDIR := $(dir $(THIS_MAKEFILE))build-debug
  SYSOUTDIR := $(dir $(THIS_MAKEFILE))build
  APPOUTDIR := Debug-wasm
  DBGCFLAGS := -DDEBUG -D_DEBUG -DZILLALOG
  LDFLAGS   :=
  WOPTFLAGS := -g -O0
endif

# Project Build flags
APPFLAGS := -I$(ZILLALIB_DIR)Include
LIBFLAGS := -I$(ZILLALIB_DIR)Include -I$(ZILLALIB_DIR)Source/zlib
DEPFLAGS := -Wno-unused-value -Wno-dangling-else

# Global compiler flags
CXXFLAGS := $(DBGCFLAGS) -Ofast -std=c++11 -fno-rtti -Wno-writable-strings -Wno-unknown-pragmas
CCFLAGS  := $(DBGCFLAGS) -Ofast -std=c99

# Flags for wasm-ld
LDFLAGS += -no-entry -allow-undefined -import-memory
LDFLAGS += -export=__wasm_call_ctors -export=malloc -export=free -export=main
LDFLAGS += -export=ZLFNDraw -export=ZLFNText -export=ZLFNKey -export=ZLFNMove -export=ZLFNMouse
LDFLAGS += -export=ZLFNWheel -export=ZLFNWindow -export=ZLFNAudio -export=ZLFNHTTP -export=ZLFNWebSocket

# Global compiler flags for Wasm targeting
CLANGFLAGS := -target wasm32 -nostdinc
CLANGFLAGS += -D__EMSCRIPTEN__ -D_LIBCPP_ABI_VERSION=2
CLANGFLAGS += -fvisibility=hidden -fno-builtin -fno-exceptions -fno-threadsafe-statics
CLANGFLAGS += -isystem$(SYSTEM_ROOT)/include/libcxx
CLANGFLAGS += -isystem$(SYSTEM_ROOT)/include/compat
CLANGFLAGS += -isystem$(SYSTEM_ROOT)/include
CLANGFLAGS += -isystem$(SYSTEM_ROOT)/include/libc
CLANGFLAGS += -isystem$(SYSTEM_ROOT)/lib/libc/musl/arch/emscripten

# Flags that don't seem to have an influence on Wasm builds
#CLANGFLAGS += -fomit-frame-pointer -ffunction-sections -fdata-sections
#CLANGFLAGS += -fno-vectorize -fno-slp-vectorize

#add defines from the make command line (e.g. D=MACRO=VALUE)
APPFLAGS += $(subst \\\,$(sp),$(foreach F,$(subst \$(sp),\\\,$(D)),"-D$(F)"))

# Compute tool paths
ifeq ($(wildcard $(subst $(sp),\ ,$(LLVM_ROOT))/clang*),)
  $(error clang executables not found in set LLVM_ROOT path ($(LLVM_ROOT)). Set custom path in $(dir $(THIS_MAKEFILE))ZillaAppLocalConfig.mk with LLVM_ROOT = $(if $(ISWIN),d:)/path/to/clang)
endif
ifeq ($(wildcard $(subst $(sp),\ ,$(WASMOPT))),)
  $(error wasm-opt executable not found in set WASMOPT path ($(WASMOPT)). Fix path in $(dir $(THIS_MAKEFILE))ZillaAppLocalConfig.mk with WASMOPT = $(if $(ISWIN),d:)/path/to/wasm-opt$(if $(ISWIN),.exe))
endif
PYTHON_NOQUOTE := $(if $(PYTHON),$(wildcard $(PYTHON)),$(call sub_checkexe_run,python,-c "print 1"))
ifeq ($(PYTHON_NOQUOTE),)
  $(error Python executable not found in PATH and not correctly set with PYTHON setting ($(PYTHON)). Set custom path in $(dir $(THIS_MAKEFILE))ZillaAppLocalConfig.mk with PYTHON = $(if $(ISWIN),d:)/path/to/python/python$(if $(ISWIN),.exe))
endif
ifeq ($(if $(7ZIP),$(wildcard $(7ZIP)),-,),)
  $(error 7-Zip executable not found in set 7ZIP path ($(7ZIP)). Fix path in $(dir $(THIS_MAKEFILE))ZillaAppLocalConfig.mk with 7ZIP = $(if $(ISWIN),d:)/path/to/7zip/7z$(if $(ISWIN),.exe))
endif

#surround used commands with double quotes
CC      := "$(LLVM_ROOT)/clang"
CXX     := "$(LLVM_ROOT)/clang" -x c++
LD      := "$(LLVM_ROOT)/wasm-ld"
WASMOPT := "$(WASMOPT)"
PYTHON  := "$(PYTHON_NOQUOTE)"
7ZIP    := $(if $(7ZIP),"$(7ZIP)")

# Python one liner to delete all .o files when the dependency files were created empty due to compile error
CMD_DEL_OLD_OBJ := $(PYTHON) -c "import sys,os;[os.path.exists(a) and os.path.getsize(a)==0 and os.path.exists(a.rstrip('d')+'o') and os.remove(a.rstrip('d')+'o') for a in sys.argv[1:]]"
CMD_DEL_FILES   := $(PYTHON) -c "import sys,os;[os.path.exists(a) and os.remove(a) for a in sys.argv[1:]]"
CMD_MAKE_DIRS   := $(PYTHON) -c "import sys,os;os.path.exists(sys.argv[1]) or os.makedirs(sys.argv[1])"
CMD_MAKE_ZIP    := $(PYTHON) -c "import sys,zipfile;z=zipfile.ZipFile(sys.argv[1],'w');[z.write(f) for f in sys.argv[2:]]"
CMD_APP_JS      := $(PYTHON) -c "import sys,os,base64;open(sys.argv[1],'wb').write(''.join([os.path.exists(a) and 'ZL.'+(i==0 and 'wasm' or 'files')+'='+chr(39)+base64.b64encode(open(a,'rb').read())+chr(39)+';'+chr(10) for i, a in enumerate(sys.argv[3:])])+open(sys.argv[2],'r').read())"
CMD_ASSET_JS    := $(PYTHON) -c "import sys,os,base64;open(sys.argv[1],'wb').write('ZL.files='+chr(39)+base64.b64encode(open(sys.argv[2],'rb').read())+chr(39)+';'+chr(10))"
CMD_MAKE_GZ     := $(PYTHON) -c "import sys,gzip;gzip.GzipFile('','wb',9,open(sys.argv[1],'wb'),0).write(open(sys.argv[2],'r').read())"

# Disable DOS PATH warning when using Cygwin based tools Windows
CYGWIN ?= nodosfilewarning
export CYGWIN

ifeq ($(MSVC),1)
CMD_MSVC_FILTER := $(PYTHON) -u -c "import re,sys,subprocess;r1=re.compile(':(\\d+):');p=subprocess.Popen(sys.argv[1:],shell=False,stdout=subprocess.PIPE,stderr=subprocess.STDOUT); [sys.stderr.write(r1.sub('(\\1) :',l).rstrip()+chr(10)) for l in iter(p.stdout.readline, b'')]; sys.stderr.write(chr(10));sys.exit(p.wait())"
all:;+@$(CMD_MSVC_FILTER) "$(MAKE)" --no-print-directory -f "$(THIS_MAKEFILE)" -j 4 "MSVC=0"
else #!MSVC

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

LDFLAGS += $(addprefix -export=,$(patsubst _%,%,$(strip $(ZLEMSCRIPTEN_ADD_EXPORTS))))

-include assets.mk
ASSET_ALL_PATHS := $(strip $(foreach F,$(ASSETS),$(wildcard ./$(F)) $(wildcard ./$(F)/*) $(wildcard ./$(F)/*/*) $(wildcard ./$(F)/*/*/*) $(wildcard ./$(F)/*/*/*/*) $(wildcard ./$(F)/*/*/*/*/*)))
ASSET_ALL_STARS := $(if $(ASSET_ALL_PATHS),$(strip $(foreach F,$(subst *./, ,*$(subst $(sp),*,$(ASSET_ALL_PATHS))),$(if $(wildcard $(subst *,\ ,$(F))/.),,$(F)))))
ASSET_ZIP       := $(if $(ASSET_ALL_STARS),$(APPOUTDIR)/$(ZillaApp)_Files.zip)
ASSET_JS        := $(if $(ASSET_ALL_STARS),$(APPOUTDIR)/$(if $(ZLWASM_ASSETS_OUTFILE),$(ZLWASM_ASSETS_OUTFILE),$(ZillaApp)_Files.js))

ifeq ($(BUILD),RELEASE)
  APPOUTJSS := $(if $(ZLWASM_ASSETS_EMBED),,$(ASSET_JS).gz )$(APPOUTDIR)/$(ZillaApp).js.gz
  APPGLUEJS := $(dir $(THIS_MAKEFILE))ZillaLibWasm.minified.js
else
  ZLWASM_ASSETS_EMBED :=
  APPOUTJSS := $(ASSET_JS) $(APPOUTDIR)/$(ZillaApp).js
  APPGLUEJS := $(dir $(THIS_MAKEFILE))ZillaLibWasm.js
endif

ZILLAAPPSCRIPT := $(subst > <,><,$(foreach F,$(patsubst $(APPOUTDIR)/%.js.gz,%.js,$(patsubst $(APPOUTDIR)/%.js,%.js,$(APPOUTJSS))),<script src="$(F)" type="text/javascript"></script>))

all: $(APPOUTJSS) $(APPOUTDIR)/$(ZillaApp).html
.PHONY: clean cleanall run web

define MAKEAPPOBJ

$(APPOUTDIR)/$(basename $(notdir $(1))).o: $(1) ; $$(call COMPILEMMD,$$@,$$<,$(2),$(3) $$(APPFLAGS))

endef
APPOBJS := $(addprefix $(APPOUTDIR)/,$(notdir $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(APPSOURCES)))))
$(shell $(CMD_DEL_OLD_OBJ) $(APPOBJS:%.o=%.d))
-include $(APPOBJS:%.o=%.d)
$(foreach F,$(filter %.cpp,$(APPSOURCES)),$(eval $(call MAKEAPPOBJ,$(F),$$(CXX),$$(CXXFLAGS))))
$(foreach F,$(filter %.c  ,$(APPSOURCES)),$(eval $(call MAKEAPPOBJ,$(F),$$(CC),$$(CCFLAGS))))

clean:
	$(info Removing all build files ...)
	@$(CMD_DEL_FILES) $(APPOBJS) $(APPOBJS:%.o=%.d) \
	  $(APPOUTDIR)/$(ZillaApp).js $(ASSET_ZIP) \
	  $(APPOUTDIR)/$(ZillaApp).js.gz \

$(APPOUTDIR)/$(ZillaApp).bc : $(APPOBJS)
	$(info Creating archive $@ ...)
	@$(LD) $^ -r -o $@

$(APPOUTDIR)/$(ZillaApp).wasm : $(THIS_MAKEFILE) $(APPOUTDIR)/$(ZillaApp).bc $(LIBOUTDIR)/ZillaLib.bc $(SYSOUTDIR)/System.bc
	$(info Linking $@ ...)
	@$(LD) $(LDFLAGS) -o $@ $(APPOUTDIR)/$(ZillaApp).bc $(LIBOUTDIR)/ZillaLib.bc $(SYSOUTDIR)/System.bc
	@$(WASMOPT) --legalize-js-interface $(WOPTFLAGS) $@ -o $@
	D:\dev\emscripten_sdk\wabt\wasm-objdump.exe -x $@ >$@.objdump
	D:\dev\emscripten_sdk\wabt\wasm2c $@ -o $@.c

$(ASSET_ZIP) : $(if $(ASSET_ALL_STARS),assets.mk $(subst *,\ ,$(ASSET_ALL_STARS)))
	$(info Building $@ with $(words $(ASSET_ALL_STARS)) assets ...)
	@$(if $(wildcard $(dir $@)),,$(shell $(CMD_MAKE_DIRS) $(dir $@)))
	@$(CMD_MAKE_ZIP) $@ $(subst *, ,$(subst $(sp)," ","$(ASSET_ALL_STARS)"))

$(APPOUTDIR)/$(ZillaApp).js : $(APPGLUEJS) $(APPOUTDIR)/$(ZillaApp).wasm $(if $(ZLWASM_ASSETS_EMBED),$(ASSET_ZIP))
	$(info Generating $@ from $(APPOUTDIR)/$(ZillaApp).wasm$(if $(ZLWASM_ASSETS_EMBED), and $(ASSET_ZIP)) ...)
	@$(CMD_APP_JS) $@ $^

$(APPOUTDIR)/%.js.gz : $(APPOUTDIR)/%.js
	$(info Compressing $^ to $@ ...)
	@$(if $(wildcard $@),$(if $(ISWIN),del "$(subst /,\,$@)" ,rm "$@" >/dev/null),)
	@$(if $(7ZIP),$(7ZIP) a -bd -si -tgzip -mx9 $@ <$^ >$(if $(ISWIN),nul,/dev/null),$(CMD_MAKE_GZ) $@ $^)

$(if $(ZLWASM_ASSETS_EMBED),,$(ASSET_JS)) : $(ASSET_ZIP)
	$(info Generating $@ from $^ ...)
	@$(CMD_ASSET_JS) $@ $^

$(if $(ZLWASM_ASSETS_EMBED),,$(ASSET_JS).gz) : $(ASSET_JS)
	$(info Compressing $^ to $@ ...)
	@$(if $(wildcard $@),$(if $(ISWIN),del "$(subst /,\,$@)" ,rm "$@" >/dev/null),)
	@$(if $(7ZIP),$(7ZIP) a -bd -si -tgzip -mx9 $@ <$^ >$(if $(ISWIN),nul,/dev/null),$(CMD_MAKE_GZ) $@ $^)

$(APPOUTDIR)/$(ZillaApp).html : $(dir $(THIS_MAKEFILE))ZillaLibWasm.html
	$(info $(if $(wildcard $@),Warning: Template $^ is newer than $@ - delete the local build file to have it regenerated,Generating $@ ...))
	@$(if $(wildcard $@),,$(PYTHON) -c "open('$@','wb').write(file('$^','rb').read().replace('{{ZILLAAPP}}','$(ZillaApp)').replace('{{ZILLAAPPSCRIPT}}','$(subst ",'+chr(34)+',$(ZILLAAPPSCRIPT))'))")

run web : $(if $(RUNWITHOUTBUILD),,all)
	@$(PYTHON) "$(dir $(THIS_MAKEFILE))ZillaLibWasm.py" $@ "$(APPOUTDIR)" "$(ZillaApp).html"

#------------------------------------------------------------------------------------------------------
else #!ZillaApp
#------------------------------------------------------------------------------------------------------

all: $(LIBOUTDIR)/ZillaLib.bc $(SYSOUTDIR)/System.bc

.PHONY: clean

clean:
	$(info Removing all temporary build files ...)
	@$(CMD_DEL_FILES) $(LIBOUTDIR)/ZillaLib.bc $(WASM_CPP_LIBOBJS) $(WASM_CPP_LIBOBJS:%.o=%.d) $(WASM_CC_LIBOBJS) $(WASM_CC_LIBOBJS:%.o=%.d) $(WASM_CPP_DEPOBJS) $(WASM_CC_DEPOBJS)

#------------------------------------------------------------------------------------------------------
endif #!ZillaApp
#------------------------------------------------------------------------------------------------------

#if we're building not the library itself and are being called with B flag (always-make), build the library only if its output doesn't exist at all
ifeq ($(if $(ZillaApp),$(if $(filter B,$(MAKEFLAGS)),$(wildcard $(LIBOUTDIR)/ZillaLib.bc))),)

#if System.bc exists, don't even bother checking sources, build once and forget for now
ifeq ($(if $(wildcard $(SYSOUTDIR)/System.bc),1,0),0)
SYS_ADDS := dlmalloc.c libcxx/*.cpp libcxxabi/src/cxa_guard.cpp compiler-rt/lib/builtins/*.c libc/wasi-helpers.c
SYS_MUSL := complex crypt ctype dirent errno fcntl fenv internal locale math misc mman multibyte prng regex select stat stdio stdlib string termios unistd

SYS_IGNORE := iostream.cpp strstream.cpp locale.cpp thread.cpp exception.cpp
SYS_IGNORE += abs.c acos.c acosf.c acosl.c asin.c asinf.c asinl.c atan.c atan2.c atan2f.c atan2l.c atanf.c atanl.c ceil.c ceilf.c ceill.c cos.c cosf.c cosl.c exp.c expf.c expl.c 
SYS_IGNORE += fabs.c fabsf.c fabsl.c floor.c floorf.c floorl.c log.c logf.c logl.c pow.c powf.c powl.c rintf.c round.c roundf.c sin.c sinf.c sinl.c sqrt.c sqrtf.c sqrtl.c tan.c tanf.c tanl.c
SYS_IGNORE += syscall.c wordexp.c initgroups.c getgrouplist.c popen.c _exit.c alarm.c usleep.c faccessat.c iconv.c

SYSSOURCES := $(filter-out $(SYS_IGNORE:%=\%/%),$(wildcard $(addprefix $(SYSTEM_ROOT)/lib/,$(SYS_ADDS) $(SYS_MUSL:%=libc/musl/src/%/*.c))))
SYSSOURCES := $(subst $(SYSTEM_ROOT)/lib/,,$(SYSSOURCES))

ifeq ($(findstring !,$(SYSSOURCES)),!)
  $(error SYSSOURCES contains a filename with a ! character in it - Unable to continue)
endif

SYS_MISSING := $(filter-out $(SYSSOURCES) $(dir $(SYSSOURCES)),$(subst *.c,,$(subst *.cpp,,$(SYS_ADDS))) $(SYS_MUSL:%=libc/musl/src/%/))
ifeq ($(if $(SYS_MISSING),1,0),1)
  $(error SYSSOURCES missing the following files in $(SYSTEM_ROOT)/lib: $(SYS_MISSING))
endif

SYS_OLDFILES := $(filter-out $(subst /,!,$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SYSSOURCES)))),$(notdir $(wildcard $(SYSOUTDIR)/temp/*.o)))
ifeq ($(if $(SYS_OLDFILES),1,0),1)
  $(shell $(CMD_DEL_FILES) $(addprefix $(SYSOUTDIR)/temp/,$(SYS_OLDFILES)) $(SYSOUTDIR)/System.bc)
endif

SYSCXXFLAGS := -Ofast -std=c++11 -fno-threadsafe-statics -fno-rtti -I$(SYSTEM_ROOT)/lib/libcxxabi/include
SYSCXXFLAGS += -DNDEBUG -D_LIBCPP_BUILDING_LIBRARY -D_LIBCPP_DISABLE_VISIBILITY_ANNOTATIONS

SYSCCFLAGS := -Ofast -std=gnu99 -fno-threadsafe-statics
SYSCCFLAGS += -DNDEBUG -Dunix -D__unix -D__unix__
SYSCCFLAGS += -isystem$(SYSTEM_ROOT)/lib/libc/musl/src/internal
SYSCCFLAGS += -Wno-dangling-else -Wno-ignored-attributes -Wno-bitwise-op-parentheses -Wno-logical-op-parentheses -Wno-shift-op-parentheses -Wno-string-plus-int -Wno-unknown-pragmas -Wno-shift-count-overflow -Wno-return-type -Wno-macro-redefined -Wno-unused-result -Wno-pointer-sign

WASM_CPP_SYSOBJS := $(addprefix $(SYSOUTDIR)/temp/,$(subst /,!,$(patsubst %.cpp,%.o,$(filter %.cpp,$(SYSSOURCES)))))
WASM_CC_SYSOBJS  := $(addprefix $(SYSOUTDIR)/temp/,$(subst /,!,$(patsubst   %.c,%.o,$(filter   %.c,$(SYSSOURCES)))))
$(WASM_CPP_SYSOBJS) : ; $(call COMPILE,$@,$(subst !,/,$(patsubst $(SYSOUTDIR)/temp/%.o,$(SYSTEM_ROOT)/lib/%.cpp,$@)),$(CXX),$(SYSCXXFLAGS))
$(WASM_CC_SYSOBJS)  : ; $(call COMPILE,$@,$(subst !,/,$(patsubst $(SYSOUTDIR)/temp/%.o,$(SYSTEM_ROOT)/lib/%.c,$@)),$(CC),$(SYSCCFLAGS))

$(SYSOUTDIR)/System.bc : $(WASM_CPP_SYSOBJS) $(WASM_CC_SYSOBJS)
	$(info Creating archive $@ ...)
	@$(if $(wildcard $(dir $@)),,$(shell $(CMD_MAKE_DIRS) $(dir $@)))
	@$(LD) $(if $(ISWIN),"$(SYSOUTDIR)/temp/*.o",$(SYSOUTDIR)/temp/*.o) -r -o $@
	@$(if $(ISWIN),rmdir /S /Q,rm -rf) "$(SYSOUTDIR)/temp"
endif #need System.bc

LIBSOURCES := $(wildcard $(ZILLALIB_DIR)Source/*.cpp)

DEPSOURCES := \
	$(wildcard $(ZILLALIB_DIR)Source/zlib/*.c) \
	$(wildcard $(ZILLALIB_DIR)Source/enet/callbacks.c) \
	$(wildcard $(ZILLALIB_DIR)Source/enet/host.c) \
	$(wildcard $(ZILLALIB_DIR)Source/enet/list.c) \
	$(wildcard $(ZILLALIB_DIR)Source/enet/packet.c) \
	$(wildcard $(ZILLALIB_DIR)Source/enet/peer.c) \
	$(wildcard $(ZILLALIB_DIR)Source/enet/protocol.c) \
	$(wildcard $(ZILLALIB_DIR)Source/libtess2/*.c) \
	$(wildcard $(ZILLALIB_DIR)Source/stb/*.cpp)

LIBSOURCES := $(subst $(ZILLALIB_DIR),,$(LIBSOURCES))
DEPSOURCES := $(subst $(ZILLALIB_DIR),,$(DEPSOURCES))

WASM_CPP_LIBOBJS := $(addprefix $(LIBOUTDIR)/,$(patsubst %.cpp,%.o,$(filter %.cpp,$(LIBSOURCES))))
WASM_CC_LIBOBJS  := $(addprefix $(LIBOUTDIR)/,$(patsubst   %.c,%.o,$(filter   %.c,$(LIBSOURCES))))
WASM_CPP_DEPOBJS := $(addprefix $(LIBOUTDIR)/,$(patsubst %.cpp,%.o,$(filter %.cpp,$(DEPSOURCES))))
WASM_CC_DEPOBJS  := $(addprefix $(LIBOUTDIR)/,$(patsubst   %.c,%.o,$(filter   %.c,$(DEPSOURCES))))

$(shell $(CMD_DEL_OLD_OBJ) $(WASM_CPP_LIBOBJS:%.o=%.d) $(WASM_CC_LIBOBJS:%.o=%.d))
-include $(WASM_CPP_LIBOBJS:%.o=%.d) $(WASM_CC_LIBOBJS:%.o=%.d)
$(WASM_CPP_LIBOBJS) : $(LIBOUTDIR)/%.o : $(ZILLALIB_DIR)%.cpp ; $(call COMPILEMMD,$@,$<,$(CXX),$(CXXFLAGS) $(LIBFLAGS))
$(WASM_CC_LIBOBJS)  : $(LIBOUTDIR)/%.o : $(ZILLALIB_DIR)%.c   ; $(call COMPILEMMD,$@,$<,$(CC),$(CCFLAGS) $(LIBFLAGS))
$(WASM_CPP_DEPOBJS) : $(LIBOUTDIR)/%.o : $(ZILLALIB_DIR)%.cpp ; $(call COMPILE,$@,$<,$(CXX),$(CXXFLAGS) $(LIBFLAGS) $(DEPFLAGS))
$(WASM_CC_DEPOBJS)  : $(LIBOUTDIR)/%.o : $(ZILLALIB_DIR)%.c   ; $(call COMPILE,$@,$<,$(CC),$(CCFLAGS) $(LIBFLAGS) $(DEPFLAGS))

$(LIBOUTDIR)/ZillaLib.bc : $(WASM_CPP_LIBOBJS) $(WASM_CC_LIBOBJS) $(WASM_CPP_DEPOBJS) $(WASM_CC_DEPOBJS)
	$(info Creating archive $@ ...)
	@$(LD) $^ -r -o $@

endif #!ZillaApp || !AlwaysMake

define COMPILE
	$(info $2)
	@$(if $(wildcard $(dir $1)),,$(shell $(CMD_MAKE_DIRS) $(dir $1)))
	@$3 $4 $(CLANGFLAGS) -o $1 -c $2
endef
define COMPILEMMD
	$(info $2)
	@$(if $(wildcard $(dir $1)),,$(shell $(CMD_MAKE_DIRS) $(dir $1)))
	@$3 $4 $(CLANGFLAGS) -MMD -MP -MF $(patsubst %.o,%.d,$1) -o $1 -c $2
endef

endif #!MSVC
