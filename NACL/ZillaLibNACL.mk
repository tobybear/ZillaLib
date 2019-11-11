#
#  ZillaLib
#  Copyright (C) 2010-2019 Bernhard Schelling
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
ISWIN := $(findstring :,$(firstword $(subst \, ,$(subst /, ,$(abspath .)))))
ZILLALIB_DIR = $(subst |,/,$(subst <,,$(subst <.|,,<$(subst /,|,$(dir $(subst $(sp),/,$(strip $(subst /,$(sp),$(dir $(THIS_MAKEFILE))))))))))
-include $(dir $(THIS_MAKEFILE))ZillaAppLocalConfig.mk
ifeq ($(NACL_SDK),)
  $(info )
  $(info Please create the file $(dir $(THIS_MAKEFILE))ZillaAppLocalConfig.mk with the following definitions:)
  $(info )
  $(info NACL_SDK = $(if $(ISWIN),d:)/path/to/nacl_sdk/pepper_47)
  $(info )
  $(error Aborting)
endif

#------------------------------------------------------------------------------------------------------

ifeq ($(BUILD),RELEASE)
  ZLOUTDIR  := $(ZILLALIB_DIR)NACL/build
  APPOUTDIR := Release-nacl
  ZILLAFLAG :=
  DBGCFLAGS := -DNDEBUG
  FINALIZEFLAGS := --compress
else ifeq ($(BUILD),GDB)
  ZLOUTDIR  := $(ZILLALIB_DIR)NACL/build-gdb
  APPOUTDIR := Debug-nacl-gdb
  ZILLAFLAG := -DZILLALOG
  DBGCFLAGS := -DDEBUG -D_DEBUG
else
  ZLOUTDIR  := $(ZILLALIB_DIR)NACL/build-debug
  APPOUTDIR := Debug-nacl
  ZILLAFLAG := -DZILLALOG
  DBGCFLAGS := -DDEBUG -D_DEBUG
endif

# Project Build flags
APPFLAGS    := -I$(ZILLALIB_DIR)Include
ZLFLAGS     := -I$(ZILLALIB_DIR)Include -I$(ZILLALIB_DIR)Source/zlib
WARNINGS    := -Wall -Wno-long-long -Wno-error=unused-parameter -Wno-unused-function -Wno-unused-local-typedef
DEPWARNINGS := -Wno-main -Wno-empty-body -Wno-char-subscripts -Wno-sign-compare -Wno-unused-value -Wno-unused-variable
CXXFLAGS    := -pthread -std=gnu++11 $(WARNINGS) $(DBGCFLAGS) -I$(NACL_SDK)/include -I$(NACL_SDK)/include/pnacl
CCFLAGS     := -pthread -std=gnu99 $(WARNINGS) $(DBGCFLAGS) -I$(NACL_SDK)/include -I$(NACL_SDK)/include/pnacl
LDFLAGS     := -L$(NACL_SDK)/lib/pnacl/Release -lppapi_gles2 -lppapi

ifeq ($(BUILD),GDB)
  CXXFLAGS += -g -O0 -pthread
  CCFLAGS += -g -O0 -pthread
  LDFLAGS += -g -O0 -pthread
else
  CXXFLAGS += -O2 -pthread -s -msse -ffast-math -fomit-frame-pointer -fvisibility=hidden -fno-exceptions -fno-non-call-exceptions -ffunction-sections -fdata-sections $(ZILLAFLAG) -fno-rtti
  CCFLAGS += -O2 -pthread -s -msse -ffast-math -fomit-frame-pointer -fvisibility=hidden -fno-exceptions -fno-non-call-exceptions -ffunction-sections -fdata-sections $(ZILLAFLAG)
  LDFLAGS += -O2 -pthread -Wl,--gc-sections -Wl,--wrap=_ZNSt8ios_base4InitC1Ev -Wl,--wrap=__gcclibcxx_demangle_callback -Wl,--wrap=_ZN9__gnu_cxx27__verbose_terminate_handlerEv -Wl,--wrap=_ZNSt6locale18_S_initialize_onceEv
endif

#add defines from the make command line (e.g. D=MACRO=VALUE)
APPFLAGS += $(subst \\\,$(sp),$(foreach F,$(subst \$(sp),\\\,$(D)),"-D$(F)"))

# Compute tool paths
ifneq ($(PYTHON),)
  PATH := $(PATH)$(if $(ISWIN),;,:)$(subst $(if $(ISWIN),/,\),$(if $(ISWIN),\,/),$(abspath $(dir $(PYTHON))))
  export PATH
endif
ifeq ($(shell python -c "print 1"),)
  $(error Python executable not found in PATH and not set with PYTHON setting in $(dir $(THIS_MAKEFILE))ZillaAppLocalConfig.mk)
endif
OSNAME:=$(shell python $(NACL_SDK)/tools/getos.py)
ifeq ($(OSNAME),)
  $(error Python was unable to run the script $(NACL_SDK)/tools/getos.py)
endif
TC_PATH:=$(abspath $(NACL_SDK)/toolchain/$(OSNAME)_pnacl)
CXX:=$(TC_PATH)/bin/pnacl-clang++
CC:=$(TC_PATH)/bin/pnacl-clang
AR:=$(TC_PATH)/bin/pnacl-ar
FINALIZE:=$(TC_PATH)/bin/pnacl-finalize
STRIP:=$(TC_PATH)/bin/pnacl-strip

# Python one liner to delete all .o files when the dependency files were created empty due to compile error
CMD_DEL_OLD_OBJ     = python -c "import sys,os;[os.path.exists(a) and os.path.getsize(a)==0 and os.path.exists(a.rstrip('d')+'o') and os.remove(a.rstrip('d')+'o') for a in sys.argv[1:]]"
CMD_DEL_FILES       = python -c "import sys,os;[os.path.exists(a) and os.remove(a) for a in sys.argv[1:]]"
CMD_MERGE_BIN_FILES = python -c "import sys;f = open(sys.argv[1],'wb');[f.write(open(i,'rb').read()) for i in sys.argv[2:]];f.close()"

# Disable DOS PATH warning when using Cygwin based tools Windows
CYGWIN ?= nodosfilewarning
export CYGWIN

#------------------------------------------------------------------------------------------------------
ifeq ($(MSVC),1)
#------------------------------------------------------------------------------------------------------

CMD_MSVC_FILTER := python -u -c "import re,sys,subprocess;r1=re.compile(':(\\d+):');p=subprocess.Popen(sys.argv[1:],shell=False,stdout=subprocess.PIPE,stderr=subprocess.STDOUT); [sys.stderr.write(r1.sub('(\\1) :',l).rstrip()+chr(10)) for l in iter(p.stdout.readline, b'')]; sys.stderr.write(chr(10));sys.exit(p.wait())"

all:;@+$(CMD_MSVC_FILTER) "$(MAKE)" --no-print-directory -f "$(THIS_MAKEFILE)" -j 4 "MSVC=0"

#------------------------------------------------------------------------------------------------------
else ifdef ZillaApp
#------------------------------------------------------------------------------------------------------

ZL_IS_APP_MAKE = 1
-include Makefile
APPSOURCES := $(wildcard *.cpp *.c)
-include sources.mk
APPSOURCES += $(foreach F, $(ZL_ADD_SRC_FILES), $(wildcard $(F)))
ifeq ($(APPSOURCES),)
  $(error No source files found for $(ZillaApp))
endif

-include assets.mk
ASSET_ALL_PATHS := $(strip $(foreach F,$(ASSETS),$(wildcard ./$(F)) $(wildcard ./$(F)/*) $(wildcard ./$(F)/*/*) $(wildcard ./$(F)/*/*/*) $(wildcard ./$(F)/*/*/*/*) $(wildcard ./$(F)/*/*/*/*/*)))
ASSET_ALL_STARS := $(if $(ASSET_ALL_PATHS),$(strip $(foreach F,$(subst *./, ,*$(subst $(sp),*,$(ASSET_ALL_PATHS))),$(if $(wildcard $(subst *,\ ,$(F))/.),,$(F)))))
ASSET_ZIP       := $(if $(ASSET_ALL_STARS),$(if $(ZLNACL_ASSETS_OUTFILE),$(APPOUTDIR)/$(ZLNACL_ASSETS_OUTFILE),$(APPOUTDIR)/$(ZillaApp)_Files.dat))
ASSET_LOADFILE  := $(if $(ASSET_ALL_STARS),$(if $(ZLNACL_ASSETS_OUTFILE),$(ZLNACL_ASSETS_OUTFILE),$(ZillaApp)_Files.dat))

ifeq ($(BUILD),RELEASE)
  ifeq ($(if $(ASSET_ALL_STARS),$(ZLNACL_ASSETS_EMBED),),1)
    APPOUTBINS := $(APPOUTDIR)/$(ZillaApp)_WithData.pexe.gz
    undefine ASSET_LOADFILE
  else
    APPOUTBINS := $(APPOUTDIR)/$(ZillaApp).pexe.gz $(ASSET_ZIP)
  endif
  ifeq ($(if $(7ZIP),$(wildcard $(7ZIP)),-,),)
    $(error 7-Zip executable not found in set 7ZIP path ($(7ZIP)). Fix path in $(dir $(THIS_MAKEFILE))ZillaAppLocalConfig.mk with 7ZIP = $(if $(ISWIN),d:)/path/to/7zip/7z$(if $(ISWIN),.exe))
  endif
else
  APPOUTBINS := $(APPOUTDIR)/$(ZillaApp).pexe $(ASSET_ZIP)
endif

all: $(APPOUTBINS) $(APPOUTDIR)/$(ZillaApp).html
.PHONY: clean run web

#HTML generation settings
HTML_ZLPEXE   = q+'$(firstword $(APPOUTBINS:$(APPOUTDIR)/%=%))'+q
HTML_ZLASSETS = $(if $(ASSET_ALL_STARS),$(if $(ASSET_LOADFILE),q+'$(ASSET_LOADFILE)'+q,'url_pexe'),'null')
CMD_GENERATE_HTML = python -c "q=chr(39);open('$@','wb').write(file('$^','rb').read().replace('{{ZILLAAPP}}','$(ZillaApp)').replace('{{ZLPEXE}}',$(HTML_ZLPEXE)).replace('{{ZLASSETS}}',$(HTML_ZLASSETS)))"

define MAKEAPPOBJ

$(APPOUTDIR)/$(basename $(notdir $(1))).o: $(1) ; $$(call COMPILE,$$@,$$<,$(2),$(3) $$(APPFLAGS) -MMD -MP)

endef
APPOBJS := $(addprefix $(APPOUTDIR)/,$(notdir $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(APPSOURCES)))))
$(shell $(CMD_DEL_OLD_OBJ) $(APPOBJS:%.o=%.d))
-include $(APPOBJS:%.o=%.d)
$(foreach F,$(filter %.cpp,$(APPSOURCES)),$(eval $(call MAKEAPPOBJ,$(F),$$(CXX),$$(CXXFLAGS))))
$(foreach F,$(filter %.c  ,$(APPSOURCES)),$(eval $(call MAKEAPPOBJ,$(F),$$(CC),$$(CCFLAGS))))

clean:
	$(info Removing all build files ...)
	@$(CMD_DEL_FILES) $(APPOBJS) $(APPOBJS:%.o=%.d) \
	  $(APPOUTDIR)/$(ZillaApp).nmf $(ASSET_ZIP) \
	  $(APPOUTDIR)/$(ZillaApp).pexe $(APPOUTDIR)/$(ZillaApp).pexe.gz \
	  $(APPOUTDIR)/$(ZillaApp)_WithData.pexe $(APPOUTDIR)/$(ZillaApp)_WithData.pexe.gz

$(APPOUTDIR)/$(ZillaApp)_unstripped.bc : $(APPOBJS) $(ZLOUTDIR)/ZillaLib.a
	$(info Linking $@ ...)
	@$(CXX) -o $@ $^ $(LDFLAGS)

$(APPOUTDIR)/$(ZillaApp).pexe : $(APPOUTDIR)/$(ZillaApp)_unstripped.bc
	$(info Finalizing $@ ...)
	@$(FINALIZE) $(FINALIZEFLAGS) -o $@ $^

$(APPOUTDIR)/%.pexe.gz : $(APPOUTDIR)/%.pexe
	$(info Compressing $^ to $@ ...)
	@$(if $(wildcard $@),$(if $(ISWIN),del "$(subst /,\,$@)" 2>nul,rm "$@" >/dev/null),)
	@$(if $(7ZIP),"$(7ZIP)" a -bd -si -tgzip -mx9 $@ <$^ >$(if $(ISWIN),nul,/dev/null),python -c "import gzip;gzip.GzipFile('','wb',9,open('$@','wb'),0).writelines(open('$^','r'))")

$(ASSET_ZIP) : $(if $(ASSET_ALL_STARS),assets.mk $(subst *,\ ,$(ASSET_ALL_STARS)))
	$(info Building $@ with $(words $(ASSET_ALL_STARS)) assets ...)
	@python -c "import sys,zipfile;z=zipfile.ZipFile('$@','w');[z.write(f) for f in sys.argv[1:]]" $(subst *, ,$(subst $(sp)," ","$(ASSET_ALL_STARS)"))

$(APPOUTDIR)/$(ZillaApp)_WithData.pexe : $(APPOUTDIR)/$(ZillaApp).pexe $(ASSET_ZIP)
	$(info Merging $(APPOUTDIR)/$(ZillaApp).pexe and $(ASSET_ZIP) into $@ ...)
	@$(CMD_MERGE_BIN_FILES) "$@" "$(APPOUTDIR)/$(ZillaApp).pexe" "$(ASSET_ZIP)"

$(APPOUTDIR)/$(ZillaApp).html : $(dir $(THIS_MAKEFILE))ZillaLibNACL.html
	$(info $(if $(wildcard $@),Warning: $^ is newer than $@ - delete the local build file to have it regenerated,Generating $@ ...))
	@$(if $(wildcard $@),,$(CMD_GENERATE_HTML))

$(APPOUTDIR)/$(ZillaApp).nmf : 
	$(info Creating $@ ...)
ifeq ($(BUILD),RELEASE)
	@python -c "open('$@','wb').write('{\"program\":{\"portable\":{\"pnacl-translate\":{\"url\":\"$(firstword $(APPOUTBINS:$(APPOUTDIR)/%=%))\"}}}}')"
else
	@python -c "open('$@','wb').write('{\"program\":{\"portable\":{\"pnacl-translate\":{\"url\":\"$(firstword $(APPOUTBINS:$(APPOUTDIR)/%=%))\"},\"pnacl-debug\":{\"url\":\"$(ZillaApp)_unstripped.bc\"}}}}')"
endif

web run : $(if $(RUNWITHOUTBUILD),,all)
	@python "$(dir $(THIS_MAKEFILE))ZillaLibNACL.py" $@ "$(APPOUTDIR)" "$(ZillaApp).html"

#------------------------------------------------------------------------------------------------------
else
#------------------------------------------------------------------------------------------------------

all: $(ZLOUTDIR)/ZillaLib.a

.PHONY: clean

clean:
	$(info Removing all build files ...)
	@$(CMD_DEL_FILES) $(ZLOUTDIR)/ZillaLib.a $(CPP_ZLOBJS) $(CPP_ZLOBJS:%.o=%.d) $(CC_ZLOBJS) $(CC_ZLOBJS:%.o=%.d) $(CPP_DEPOBJS) $(CC_DEPOBJS)

#------------------------------------------------------------------------------------------------------
endif
#------------------------------------------------------------------------------------------------------

#if we're building not the library itself and are being called with B flag (always-make), build the library only if its output doesn't exist at all
ifeq ($(if $(ZillaApp),$(if $(filter B,$(MAKEFLAGS)),$(wildcard $(ZLOUTDIR)/ZillaLib.a))),)

ZLSOURCES := $(wildcard $(ZILLALIB_DIR)Source/*.cpp)

DEPSOURCES := \
	$(wildcard $(ZILLALIB_DIR)Source/zlib/*.c) \
	$(wildcard $(ZILLALIB_DIR)Source/libtess2/*.c) \
	$(wildcard $(ZILLALIB_DIR)Source/stb/*.cpp)

ZLSOURCES := $(subst $(ZILLALIB_DIR),,$(ZLSOURCES))
DEPSOURCES := $(subst $(ZILLALIB_DIR),,$(DEPSOURCES))

CPP_DEPOBJS := $(addprefix $(ZLOUTDIR)/,$(patsubst %.cpp,%.o,$(filter %.cpp,$(DEPSOURCES))))
CC_DEPOBJS  := $(addprefix $(ZLOUTDIR)/,$(patsubst   %.c,%.o,$(filter   %.c,$(DEPSOURCES))))
$(CPP_DEPOBJS) : $(ZLOUTDIR)/%.o : $(ZILLALIB_DIR)%.cpp ; $(call COMPILE,$@,$<,$(CXX),$(CXXFLAGS) $(ZLFLAGS) $(DEPWARNINGS))
$(CC_DEPOBJS)  : $(ZLOUTDIR)/%.o : $(ZILLALIB_DIR)%.c   ; $(call   COMPILE,$@,$<,$(CC),$(CCFLAGS) $(ZLFLAGS) $(DEPWARNINGS))

CPP_ZLOBJS  := $(addprefix $(ZLOUTDIR)/,$(patsubst %.cpp,%.o,$(filter %.cpp,$(ZLSOURCES))))
CC_ZLOBJS   := $(addprefix $(ZLOUTDIR)/,$(patsubst   %.c,%.o,$(filter   %.c,$(ZLSOURCES))))
$(shell $(CMD_DEL_OLD_OBJ) $(CPP_ZLOBJS:%.o=%.d) $(CC_ZLOBJS:%.o=%.d))
-include $(CPP_ZLOBJS:%.o=%.d) $(CC_ZLOBJS:%.o=%.d)
$(CPP_ZLOBJS)  : $(ZLOUTDIR)/%.o : $(ZILLALIB_DIR)%.cpp ; $(call COMPILE,$@,$<,$(CXX),$(CXXFLAGS) $(ZLFLAGS) -MMD -MP)
$(CC_ZLOBJS)   : $(ZLOUTDIR)/%.o : $(ZILLALIB_DIR)%.c   ; $(call   COMPILE,$@,$<,$(CC),$(CCFLAGS) $(ZLFLAGS) -MMD -MP)

$(ZLOUTDIR)/ZillaLib.a : $(CPP_ZLOBJS) $(CC_ZLOBJS) $(CPP_DEPOBJS) $(CC_DEPOBJS)
	$(info Creating static library $@ ...)
	@$(AR) rcs $@ $^

endif

#------------------------------------------------------------------------------------------------------

define COMPILE
	$(info $2)
	@$(if $(wildcard $(dir $1)),,$(shell python -c "import os;os.makedirs('$(dir $1)')"))
	@"$3" -o $1 -c $2 $4
endef
