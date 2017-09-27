#
#  ZillaLib
#  Copyright (C) 2010-2016 Bernhard Schelling
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
ZILLALIB_DIR = $(or $(subst |,/,$(subst <,,$(subst <.|,,<$(subst /,|,$(dir $(subst $(sp),/,$(strip $(subst /,$(sp),$(dir $(THIS_MAKEFILE)))))))))),$(if $(subst ./,,$(dir $(THIS_MAKEFILE))),,../))
sub_checkexe_run = $(if $(1),$(if $(shell "$(1)" $(2) 2>$(if $(ISWIN),nul,/dev/null)),$(1),),)
-include $(dir $(THIS_MAKEFILE))ZillaAppLocalConfig.mk
ifeq ($(EMSCRIPTEN_ROOT),)
  $(info )
  $(info Please create the file $(dir $(THIS_MAKEFILE))ZillaAppLocalConfig.mk with the following definitions:)
  $(info )
  $(info EMSCRIPTEN_ROOT = $(if $(ISWIN),d:)/path/to/emscripten)
  $(info LLVM_ROOT = $(if $(ISWIN),d:)/path/to/clang/bin)
  $(info EMSCRIPTEN_NATIVE_OPTIMIZER = $(if $(ISWIN),d:)/path/to/emscripten/tools/optimizer/optimizer$(if $(ISWIN),.exe))
  $(info NODE_JS = $(if $(ISWIN),d:)/path/to/node/node$(if $(ISWIN),.exe))
  $(info PYTHON = $(if $(ISWIN),d:)/path/to/python/python$(if $(ISWIN),.exe))
  $(info )
  $(error Aborting)
endif

#------------------------------------------------------------------------------------------------------

ifeq ($(BUILD),RELEASE)
  ZLOUTDIR  := $(ZILLALIB_DIR)Emscripten/build
  APPOUTDIR := Release-emscripten
  ZILLAFLAG :=
  LDFLAGS   := -O3
  DBGCFLAGS := -DNDEBUG
else
  ZLOUTDIR  := $(ZILLALIB_DIR)Emscripten/build-debug
  APPOUTDIR := Debug-emscripten
  ZILLAFLAG := -DZILLALOG
  LDFLAGS   := -O1
  DBGCFLAGS := -DDEBUG -D_DEBUG
endif

# Project Build flags
APPFLAGS := -I$(ZILLALIB_DIR)Include
ZLFLAGS  := -I$(ZILLALIB_DIR)Include -I$(ZILLALIB_DIR)Source/zlib
DEPFLAGS := -Wno-unused-value -Wno-dangling-else
CXXFLAGS := $(ZILLAFLAG) $(DBGCFLAGS) -std=c++11 -O3 -fno-exceptions -fno-rtti
CCFLAGS  := $(ZILLAFLAG) $(DBGCFLAGS) -std=c99 -O3

CC  := "$(LLVM_ROOT)/clang"
CXX := "$(LLVM_ROOT)/clang++"
LD  := "$(LLVM_ROOT)/llvm-link"

# Clang flags for emscripten
CLANGFLAGS := -target asmjs-unknown-emscripten -emit-llvm -D__EMSCRIPTEN__ -D_LIBCPP_ABI_VERSION=2
CLANGFLAGS += -isystem$(EMSCRIPTEN_ROOT)/system/include/compat
CLANGFLAGS += -isystem$(EMSCRIPTEN_ROOT)/system/include
CLANGFLAGS += -isystem$(EMSCRIPTEN_ROOT)/system/include/emscripten
CLANGFLAGS += -isystem$(EMSCRIPTEN_ROOT)/system/include/libcxx
CLANGFLAGS += -isystem$(EMSCRIPTEN_ROOT)/system/include/libc
CLANGFLAGS += -isystem$(EMSCRIPTEN_ROOT)/system/lib/libc/musl/arch/emscripten
CLANGFLAGS += -fno-vectorize -fno-slp-vectorize

#add defines from the make command line (e.g. D=MACRO=VALUE)
APPFLAGS += $(subst \\\,$(sp),$(foreach F,$(subst \$(sp),\\\,$(D)),"-D$(F)"))

#global LD flags
LDFLAGS += --memory-init-file 0 -s "EXPORTED_FUNCTIONS=['_main','_ZLFNDraw','_ZLFNText','_ZLFNKey','_ZLFNMove','_ZLFNMouse','_ZLFNWheel','_ZLFNWindow','_ZLFNAudio','_ZLFNHTTP','_ZLFNWebSocket']"

# Compute tool paths
ifeq ($(wildcard $(EMSCRIPTEN_ROOT)/emcc),)
  $(error Emscripten not found in set EMSCRIPTEN_ROOT path ($(EMSCRIPTEN_ROOT)))
endif
ifeq ($(wildcard $(LLVM_ROOT)/clang*),)
  $(error clang executables not found in set LLVM_ROOT path ($(LLVM_ROOT)). Set custom path in $(dir $(THIS_MAKEFILE))ZillaAppLocalConfig.mk with LLVM_ROOT = $(if $(ISWIN),d:)/path/to/clang/bin)
endif
PYTHON_NOQUOTE := $(if $(PYTHON),$(wildcard $(PYTHON)),$(call sub_checkexe_run,python,-c "print 1"))
ifeq ($(PYTHON_NOQUOTE),)
  $(error Python executable not found in PATH and not correctly set with PYTHON setting ($(PYTHON)). Set custom path in $(dir $(THIS_MAKEFILE))ZillaAppLocalConfig.mk with PYTHON = $(if $(ISWIN),d:)/path/to/python/python$(if $(ISWIN),.exe))
endif
NODE_JS_CHECK := $(if $(NODE_JS),$(wildcard $(NODE_JS)),$(call sub_checkexe_run,node,-v))
ifeq ($(NODE_JS_CHECK),)
  $(error Node.JS executable not found in PATH and not correctly set with NODE_JS setting ($(NODE_JS)). Set custom path in $(dir $(THIS_MAKEFILE))ZillaAppLocalConfig.mk with NODE_JS = $(if $(ISWIN),d:)/path/to/node/node$(if $(ISWIN),.exe))
endif
NODE_JS := $(NODE_JS_CHECK)
ifeq ($(if $(EMSCRIPTEN_NATIVE_OPTIMIZER),$(wildcard $(EMSCRIPTEN_NATIVE_OPTIMIZER)),-,),)
  $(error emscripten native optimizer executable not found in set EMSCRIPTEN_NATIVE_OPTIMIZER path ($(EMSCRIPTEN_NATIVE_OPTIMIZER)). Set custom path in $(dir $(THIS_MAKEFILE))ZillaAppLocalConfig.mk with EMSCRIPTEN_NATIVE_OPTIMIZER = $(if $(ISWIN),d:)/path/to/emscripten/tools/optimizer/optimizer$(if $(ISWIN),.exe))
endif
ifeq ($(if $(7ZIP),$(wildcard $(7ZIP)),-,),)
  $(error 7-Zip executable not found in set 7ZIP path ($(7ZIP)). Fix path in $(dir $(THIS_MAKEFILE))ZillaAppLocalConfig.mk with 7ZIP = $(if $(ISWIN),d:)/path/to/7zip/7z$(if $(ISWIN),.exe))
endif

#as of Emscripten 1.35.0, closure is broken for SIMD.js code and calls to Math.fround in library_gl.js
undefine JAVA
LDFLAGS += --closure 0
##setup closure if building release and java is available
#ifeq ($(BUILD),RELEASE)
#  #quickly check for java by running either $(JAVA), the java found in environment variable JAVA_HOME, or java in PATH
#  ifeq ($(if $(JAVA),$(wildcard $(JAVA)),-,),)
#    $(error Java executable not found in set JAVA path '$(JAVA)'. Set custom path in $(dir $(THIS_MAKEFILE))ZillaAppLocalConfig.mk with JAVA = $(if $(ISWIN),d:)/path/to/java/bin/java$(if $(ISWIN),.exe))
#  endif
#  JAVA := $(if $(JAVA),$(JAVA),$(if $(JAVA_HOME),$(call sub_checkexe_run,$(subst \,/,$(JAVA_HOME))/bin/java,-verbose -version)))
#  JAVA := $(if $(JAVA),$(JAVA),$(call sub_checkexe_run,java,-verbose -version))
#  LDFLAGS += --closure $(if $(JAVA),1,0)
#else
#  LDFLAGS += --closure 0
#endif

#set EM_CACHE environment variable because setting only --cache commandline parameter is not enough
EM_CACHE := $(abspath $(ZILLALIB_DIR)Emscripten/cache)
export EM_CACHE

#setup em-config parameter (Emscripten configuration file replacement)
EM_CONFIG := --em-config "$(strip \
		)EMSCRIPTEN_ROOT='$(EMSCRIPTEN_ROOT)';$(strip \
		)LLVM_ROOT='$(LLVM_ROOT)';$(strip \
		)PYTHON='$(subst \,/,$(PYTHON_NOQUOTE))';$(strip \
		)EMSCRIPTEN_NATIVE_OPTIMIZER='$(EMSCRIPTEN_NATIVE_OPTIMIZER)';$(strip \
		)NODE_JS='$(NODE_JS)';$(strip \
		)TEMP_DIR='$(abspath $(APPOUTDIR))';$(strip \
		)COMPILER_ENGINE=NODE_JS;$(strip \
		)JS_ENGINES=[NODE_JS];$(strip \
		)JAVA='$(JAVA)';$(strip \
	)" --cache "$(EM_CACHE)"

#surround used commands with double quotes
PYTHON := "$(PYTHON_NOQUOTE)"
7ZIP := $(if $(7ZIP),"$(7ZIP)")

# Python one liner to delete all .o files when the dependency files were created empty due to compile error
CMD_DEL_OLD_OBJ := $(PYTHON) -c "import sys,os;[os.path.exists(a) and os.path.getsize(a)==0 and os.path.exists(a.rstrip('d')+'o') and os.remove(a.rstrip('d')+'o') for a in sys.argv[1:]]"
CMD_DEL_FILES := $(PYTHON) -c "import sys,os;[os.path.exists(a) and os.remove(a) for a in sys.argv[1:]]"
CMD_MAKE_EMBED_JS := $(PYTHON) -u -c "import sys,os;sys.stdout.write('try{this['+chr(34)+'Module'+chr(34)+']=Module;}catch(e){this['+chr(34)+'Module'+chr(34)+']=Module={};}Module.preInit=function(){try{FS}catch(e){return}'+chr(10)+''.join([a and 'FS.createPath('+chr(34)+'/'+os.path.dirname(a)+chr(34)+','+chr(34)+os.path.basename(a)+chr(34)+',!0,!0);'+chr(10) for a in sorted(list(set([os.path.dirname(a) for a in sys.argv[1:]])))]+[os.path.exists(a) and 'FS.createDataFile('+chr(34)+'/'+os.path.dirname(a)+chr(34)+','+chr(34)+os.path.basename(a)+chr(34)+',['+','.join([str(ord(x)) for x in open(a,'rb').read()])+'],!0,!0);'+chr(10) for a in sys.argv[1:]])+'};'+chr(10))"
CMD_MERGE_JS_FILES := $(PYTHON) -u -c "import sys,re;b=chr(92);r=b+'r';n=b+'n';[sys.stdout.write(re.sub(n+n+'+',n,re.sub(n+'//.*'+n,n,re.sub(r,n,open(a,'rb').read())))) for a in sys.argv[1:]]"

# Disable DOS PATH warning when using Cygwin based tools Windows
CYGWIN ?= nodosfilewarning
export CYGWIN

#------------------------------------------------------------------------------------------------------
ifdef ZillaApp
#------------------------------------------------------------------------------------------------------

APPSOURCES := $(wildcard *.cpp)
-include sources.mk
APPSOURCES += $(foreach F, $(ZL_ADD_SRC_FILES), $(wildcard $(F)))
ifeq ($(APPSOURCES),)
  $(error No source files found for $(ZillaApp))
endif

-include assets.mk
ASSET_ALL_PATHS := $(strip $(foreach F,$(ASSETS),$(wildcard ./$(F)) $(wildcard ./$(F)/*) $(wildcard ./$(F)/*/*) $(wildcard ./$(F)/*/*/*) $(wildcard ./$(F)/*/*/*/*) $(wildcard ./$(F)/*/*/*/*/*)))
ASSET_ALL_STARS := $(if $(ASSET_ALL_PATHS),$(strip $(foreach F,$(subst *./, ,*$(subst $(sp),*,$(ASSET_ALL_PATHS))),$(if $(wildcard $(subst *,\ ,$(F))/.),,$(F)))))
ASSET_JS        := $(if $(ASSET_ALL_STARS),$(if $(ASSETS_OUTFILE),$(APPOUTDIR)/$(ASSETS_OUTFILE),$(APPOUTDIR)/$(ZillaApp)_Files.js),)

ifeq ($(BUILD),RELEASE)
  ifeq ($(if $(ASSET_ALL_STARS),$(ASSETS_EMBED),),1)
    APPOUTBIN := $(APPOUTDIR)/$(ZillaApp)_WithData.js.gz
  else
    APPOUTBIN := $(if $(ASSET_ALL_STARS),$(ASSET_JS).gz )$(APPOUTDIR)/$(ZillaApp).js.gz
  endif
else
  APPOUTBIN := $(if $(ASSET_ALL_STARS),$(ASSET_JS) )$(APPOUTDIR)/$(ZillaApp).js
endif

ZILLAAPPSCRIPT := $(subst > <,><,$(foreach F,$(patsubst $(APPOUTDIR)/%.js.gz,%.js,$(patsubst $(APPOUTDIR)/%.js,%.js,$(APPOUTBIN))),<script src="$(F)" type="text/javascript"></script>))

all: $(APPOUTBIN) $(APPOUTDIR)/$(ZillaApp).html
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
	  $(APPOUTDIR)/$(ZillaApp).js $(ASSET_JS) \
	  $(APPOUTDIR)/$(ZillaApp).js.gz $(ASSET_JS).gz \
	  $(APPOUTDIR)/$(ZillaApp)_WithData.js $(APPOUTDIR)/$(ZillaApp)_WithData.js.gz

$(APPOUTDIR)/$(ZillaApp).bc : $(APPOBJS)
	$(info Creating archive $@ ...)
	@$(LD) $^ -o $@

$(APPOUTDIR)/$(ZillaApp).js : $(APPOUTDIR)/$(ZillaApp).bc $(ZLOUTDIR)/ZillaLib.bc $(ZILLALIB_DIR)Emscripten/ZillaLibEmscripten.js
	$(info Linking $@ ...)
	@$(PYTHON) "$(EMSCRIPTEN_ROOT)/emcc" -o $@ $(APPOUTDIR)/$(ZillaApp).bc $(ZLOUTDIR)/ZillaLib.bc $(LDFLAGS) --js-library $(ZILLALIB_DIR)Emscripten/ZillaLibEmscripten.js $(EM_CONFIG)
#	$(if $(ISWIN),set ,)EMCC_DEBUG=1$(if $(ISWIN),&&, )$(PYTHON) "$(EMSCRIPTEN_ROOT)/emcc" -s VERBOSE=1 -v -o $@ $(APPOUTDIR)/$(ZillaApp).bc $(ZLOUTDIR)/ZillaLib.bc $(LDFLAGS) --js-library $(ZILLALIB_DIR)Emscripten/ZillaLibEmscripten.js $(EM_CONFIG)

$(ASSET_JS) : $(if $(ASSET_ALL_STARS),assets.mk $(subst *,\ ,$(ASSET_ALL_STARS)))
	$(info Building $@ with $(words $(ASSET_ALL_STARS)) assets ...)
	@$(CMD_MAKE_EMBED_JS) $(subst *, ,$(subst $(sp)," ","$(ASSET_ALL_STARS)")) >"$@"

$(APPOUTDIR)/$(ZillaApp)_WithData.js : $(APPOUTDIR)/$(ZillaApp).js $(ASSET_JS)
	$(info Merging $(ZillaApp)_Files.js and $(ZillaApp).js into $@ ...)
	@$(CMD_MERGE_JS_FILES) "$(ASSET_JS)" "$(APPOUTDIR)/$(ZillaApp).js" >"$@"

$(APPOUTDIR)/%.js.gz : $(APPOUTDIR)/%.js
	$(info Compressing $^ to $@ ...)
	@$(if $(wildcard $@),$(if $(ISWIN),del "$(subst /,\,$@)" ,rm "$@" >/dev/null),)
	@$(if $(7ZIP),$(7ZIP) a -bd -si -tgzip -mx9 $@ <$^ >$(if $(ISWIN),nul,/dev/null),$(PYTHON) -c "import gzip;gzip.GzipFile('','wb',9,open('$@','wb'),0).writelines(open('$^','r'))")

$(APPOUTDIR)/$(ZillaApp).html : $(dir $(THIS_MAKEFILE))ZillaLibEmscripten.html
	$(info $(if $(wildcard $@),Warning: $^ is newer than $@ - delete the local build file to have it regenerated,Generating $@ ...))
	@$(if $(wildcard $@),,$(PYTHON) -c "open('$@','wb').write(file('$^','rb').read().replace('{{ZILLAAPP}}','$(ZillaApp)').replace('{{ZILLAAPPSCRIPT}}','$(subst ",'+chr(34)+',$(ZILLAAPPSCRIPT))'))")

run web : all
	@$(PYTHON) "$(dir $(THIS_MAKEFILE))ZillaLibEmscripten.py" $@ "$(APPOUTDIR)" "$(ZillaApp).html"

#------------------------------------------------------------------------------------------------------
else
#------------------------------------------------------------------------------------------------------

all: $(ZLOUTDIR)/ZillaLib.bc

.PHONY: clean

clean:
	$(info Removing all temporary build files ...)
	@$(CMD_DEL_FILES) $(ZLOUTDIR)/ZillaLib.bc $(EM_CPP_ZLOBJS) $(EM_CPP_ZLOBJS:%.o=%.d) $(EM_CC_ZLOBJS) $(EM_CC_ZLOBJS:%.o=%.d) $(EM_CPP_DEPOBJS) $(EM_CC_DEPOBJS)

#------------------------------------------------------------------------------------------------------
endif
#------------------------------------------------------------------------------------------------------

#if we're building not the library itself and are being called with B flag (always-make), build the library only if its output doesn't exist at all
ifeq ($(if $(ZillaApp),$(if $(filter B,$(MAKEFLAGS)),$(wildcard $(ZLOUTDIR)/ZillaLib.bc))),)

ZLSOURCES := $(wildcard $(ZILLALIB_DIR)Source/*.cpp)

DEPSOURCES := \
	$(wildcard $(ZILLALIB_DIR)Source/zlib/*.c) \
	$(wildcard $(ZILLALIB_DIR)Source/libtess2/*.c) \
	$(wildcard $(ZILLALIB_DIR)Source/stb/*.cpp)

ZLSOURCES := $(subst $(ZILLALIB_DIR),,$(ZLSOURCES))
DEPSOURCES := $(subst $(ZILLALIB_DIR),,$(DEPSOURCES))

EM_CPP_DEPOBJS := $(addprefix $(ZLOUTDIR)/,$(patsubst %.cpp,%.o,$(filter %.cpp,$(DEPSOURCES))))
EM_CC_DEPOBJS  := $(addprefix $(ZLOUTDIR)/,$(patsubst   %.c,%.o,$(filter   %.c,$(DEPSOURCES))))
$(EM_CPP_DEPOBJS) : $(ZLOUTDIR)/%.o : $(ZILLALIB_DIR)%.cpp ; $(call COMPILE,$@,$<,$(CXX),$(CXXFLAGS) $(ZLFLAGS) $(DEPFLAGS))
$(EM_CC_DEPOBJS)  : $(ZLOUTDIR)/%.o : $(ZILLALIB_DIR)%.c   ; $(call COMPILE,$@,$<,$(CC),$(CCFLAGS) $(ZLFLAGS) $(DEPFLAGS))

EM_CPP_ZLOBJS  := $(addprefix $(ZLOUTDIR)/,$(patsubst %.cpp,%.o,$(filter %.cpp,$(ZLSOURCES))))
EM_CC_ZLOBJS   := $(addprefix $(ZLOUTDIR)/,$(patsubst   %.c,%.o,$(filter   %.c,$(ZLSOURCES))))
$(shell $(CMD_DEL_OLD_OBJ) $(EM_CPP_ZLOBJS:%.o=%.d) $(EM_CC_ZLOBJS:%.o=%.d))
-include $(EM_CPP_ZLOBJS:%.o=%.d) $(EM_CC_ZLOBJS:%.o=%.d)
$(EM_CPP_ZLOBJS)  : $(ZLOUTDIR)/%.o : $(ZILLALIB_DIR)%.cpp ; $(call COMPILEMMD,$@,$<,$(CXX),$(CXXFLAGS) $(ZLFLAGS))
$(EM_CC_ZLOBJS)   : $(ZLOUTDIR)/%.o : $(ZILLALIB_DIR)%.c   ; $(call COMPILEMMD,$@,$<,$(CC),$(CCFLAGS) $(ZLFLAGS))

$(ZLOUTDIR)/ZillaLib.bc : $(EM_CPP_ZLOBJS) $(EM_CC_ZLOBJS) $(EM_CPP_DEPOBJS) $(EM_CC_DEPOBJS)
	$(info Creating archive $@ ...)
	@$(LD) $^ -o $@

endif

#------------------------------------------------------------------------------------------------------

define COMPILE
	$(info $2)
	@$(if $(wildcard $(dir $1)),,$(shell $(PYTHON) -c "import os;os.makedirs('$(dir $1)')"))
	@$3 $4 $(CLANGFLAGS) -o $1 -c $2
endef
define COMPILEMMD
	$(info $2)
	@$(if $(wildcard $(dir $1)),,$(shell $(PYTHON) -c "import os;os.makedirs('$(dir $1)')"))
	@$3 $4 $(CLANGFLAGS) -MMD -MP -MF $(patsubst %.o,%.d,$1) -o $1 -c $2
endef
