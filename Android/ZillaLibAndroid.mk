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
ZILLALIB_DIR := $(dir $(subst / *,,$(subst \,/,$(dir $(THIS_MAKEFILE))) *))

ifneq ($(words $(ZILLALIB_DIR)),1)
  $(info )
  $(info ZILLALIB_DIR is set to "$(ZILLALIB_DIR)" which contains spaces. Paths with spaces are not supported for Android builds at the moment.)
  $(info )
  $(error Aborting)
endif

include $(ZILLALIB_DIR)Android/ZillaAppLocalConfig.mk
ifeq ($(if $(ANDROID_SDK),$(ANDROID_NDK)),)
  $(info )
  $(info Please create the file $(ZILLALIB_DIR)Android/ZillaAppLocalConfig.mk with the following definitions:)
  $(info )
  ifeq ($(findstring :,$(firstword $(subst \, ,$(subst /, ,$(CURDIR))))),)
    $(info ANDROID_SDK = /path/to/android-sdk)
    $(info ANDROID_NDK = /path/to/android-ndk)
  else
    $(info ANDROID_SDK = d:/path/to/android-sdk-windows)
    $(info ANDROID_NDK = d:/path/to/android-ndk)
  endif
  $(info )
  $(error Aborting)
endif

ifeq ($(wildcard $(subst $(sp),\ ,$(subst \,/,$(ANDROID_SDK)))/platforms),)
  $(error Could not find ANDROID_SDK in $(ANDROID_SDK). Please set $(ZILLALIB_DIR)Android/ZillaAppLocalConfig.mk correctly.)
endif

ifeq ($(wildcard $(subst $(sp),\ ,$(subst \,/,$(ANDROID_NDK)))/build),)
  $(error Could not find ANDROID_NDK in $(ANDROID_NDK). Please set $(ZILLALIB_DIR)Android/ZillaAppLocalConfig.mk correctly.)
endif

ifneq ($(words $(ANDROID_NDK)),1)
  $(info )
  $(info ANDROID_NDK is set to "$(ANDROID_NDK)" which contains spaces. Unfortunately NDK does not work with such a path.)
  $(info )
  $(error Aborting)
endif

ifndef NDK_ALL_APPS
  NDK_ROOT = $(ANDROID_NDK)
  include $(subst $(sp),\ ,$(subst \,/,$(ANDROID_NDK)))/build/core/init.mk
  ZL_MAIN_MAKE_FILE := 1
endif

ifeq ($(HOST_OS),cygwin)
  $(error This makefile cannot be used within cygwin, please run natively. Aborting)
endif

ifeq ($(HOST_OS),windows)
  ifeq ($(subst bash,sh,$(SHELL)),/bin/sh)
    $(error Unable to use make compiled for cygwin, use a native make)
  endif
endif

APP_PLATFORM := $(firstword $(NDK_ALL_PLATFORMS))

NDK_BUILD_OPTS := $(strip $(if $(filter 1,$(V)),V=1,-j 4) $(if $(filter 1,$(ZLDEBUG)),NDK_DEBUG=1,) $(if $(filter B,$(MAKEFLAGS)),-B,))

ifeq ($(HOST_OS),windows)
	DEVNUL := nul
	OSJAVA := $(if $(OSJAVA),$(OSJAVA),$(strip $(shell "$(HOST_ECHO)" $(wildcard $(subst $(sp),\ ,$(subst \,/,$(ProgramW6432)))/Java/*/bin/java.exe) | "$(HOST_AWK)" "END{L=$$ 0;while(i=match(L,/ .:/)){L=substr(L,i+1)}print}")))
	OSJAVA := $(if $(OSJAVA),$(OSJAVA),$(strip $(shell "$(HOST_ECHO)" $(wildcard $(subst $(sp),\ ,$(subst \,/,$(ProgramFiles)))/Java/*/bin/java.exe) | "$(HOST_AWK)" "END{L=$$ 0;while(i=match(L,/ .:/)){L=substr(L,i+1)}print}")))
	OSJAVA := $(if $(OSJAVA),$(OSJAVA),java.exe)
else
	DEVNUL := /dev/null
	OSJAVA := $(if $(OSJAVA),$(OSJAVA),java)
endif

sub_checkexe_run = $(if $($(1)),$($(1)),$(if $(2),$(if $(shell "$(2)" $(3) 2>$(DEVNUL)),$(2),),))
sub_checkexe_find = $(if $($(1)),$($(1)),$(call sub_checkexe_run,$(1),$(wildcard $(subst $(sp),\ ,$(subst \,/,$(2)$(if $(filter windows,$(HOST_OS_BASE)),.exe,)))),$(3)))
JAVA_EXE := $(call sub_checkexe_run,JAVA_EXE,$(subst \,/,$(JAVA_HOME))/bin/java,-verbose -version)
JAVA_EXE := $(call sub_checkexe_run,JAVA_EXE,$(OSJAVA),-verbose -version)
JAVA_EXE := $(call sub_checkexe_find,JAVA_EXE,$(dir $(ANDROID_SDK))jdk/bin/java,-verbose -version)
JAVA_EXE := $(call sub_checkexe_find,JAVA_EXE,$(dir $(ANDROID_SDK))jre/bin/java,-verbose -version)
JAVA_EXE := $(call sub_checkexe_find,JAVA_EXE,$(dir $(ANDROID_SDK))j*/bin/java,-verbose -version)
JAVA_EXE := $(call sub_checkexe_find,JAVA_EXE,$(dir $(ANDROID_SDK))java/*/bin/java,-verbose -version)
JDK_BIN := $(if $(subst ./,,$(subst \,/,$(dir $(JAVA_EXE)))),$(dir $(JAVA_EXE)),$(if $(JAVA_HOME),$(subst \,/,$(JAVA_HOME))/bin/,))
ifeq ($(JAVA_EXE),)
  $(info Could not find Java$(if $(JDK_BIN), in the directory $(JDK_BIN)))
  $(info Java development kit needs to be installed with environment variable or Makefile variable JAVA_HOME set, somewhere in the PATH or placed under the Android SDK directory at $(dir $(ANDROID_SDK))jdk - JAVA_HOME can also be set in $(ZILLALIB_DIR)Android/ZillaAppLocalConfig.mk)
  $(error Aborting)
endif
ifeq ($(wildcard $(subst $(sp),\ ,$(subst \,/,$(JDK_BIN)javac$(if $(filter windows,$(HOST_OS_BASE)),.exe,)))),)
  $(info Could not find the Java compiler javac$(if $(JDK_BIN), in the directory $(JDK_BIN)))
  $(info Java development kit needs to be installed with environment variable or Makefile variable JAVA_HOME set, somewhere in the PATH or placed under the Android SDK directory at $(dir $(ANDROID_SDK))jdk - JAVA_HOME can also be set in $(ZILLALIB_DIR)Android/ZillaAppLocalConfig.mk)
  $(error Aborting)
endif

ifeq ($(wildcard $(ZILLALIB_DIR)Android/stlport),)
  $(info $(shell cd "$(ZILLALIB_DIR)Android" && "$(JDK_BIN)jar" xf stlport.zip))
  $(if $(wildcard $(ZILLALIB_DIR)Android/stlport),,$(error Could not extract "stlport.zip" with the JDK tool jar. Please extract it manually inside "$(ZILLALIB_DIR)Android"))
endif

ZILLALIB_OUTBASE := $(ZILLALIB_DIR)Android/$(if $(filter 1,$(ZLDEBUG)),build-debug,build)

#------------------------------------------------------------------------------------------------------
ifdef ZillaApp
#------------------------------------------------------------------------------------------------------

NDK_BUILD_OPTS += $(if $(D), "D=$(D)",)
NDK_BUILD_OPTS += $(subst \\\,$(sp),$(foreach F,$(subst \$(sp),\\\,$(W)),-W "$(F)"))

ANDROIDPROJ_DIR := Android

ifeq ($(wildcard $(ANDROIDPROJ_DIR)/project.properties),)
  $(error File $(ANDROIDPROJ_DIR)/project.properties not found. Make sure it exists and that the working directory is the project root)
endif
ifeq ($(wildcard $(ANDROIDPROJ_DIR)/jni/Android.mk),)
  $(error File $(ANDROIDPROJ_DIR)/jni/Android.mk not found. Make sure it exists and that the working directory is the project root)
endif
ifeq ($(wildcard $(ANDROIDPROJ_DIR)/jni/Application.mk),)
  $(error File $(ANDROIDPROJ_DIR)/jni/Application.mk not found. Make sure it exists and that the working directory is the project root)
endif

app.activity    := $(shell "$(HOST_AWK)" "match($$ 0,/<activity[^>]+android:name *= *\"\./){L=substr($$ 0,RSTART+RLENGTH-1);print substr(L,0,match(L,/\"/)-1);exit}" $(ANDROIDPROJ_DIR)/AndroidManifest.xml)
app.package     := $(shell "$(HOST_AWK)" "match($$ 0,/package *= *\"/){L=substr($$ 0,RSTART+RLENGTH);print substr(L,0,match(L,/\"/)-1);exit}" $(ANDROIDPROJ_DIR)/AndroidManifest.xml)
app.activity    := $(shell "$(HOST_AWK)" "match($$ 0,/<activity[^>]+android:name *= *\"\./){L=substr($$ 0,RSTART+RLENGTH-1);print substr(L,0,match(L,/\"/)-1);exit}" $(ANDROIDPROJ_DIR)/AndroidManifest.xml)
app.versioncode := $(shell "$(HOST_AWK)" "match($$ 0,/android:versionCode *= *\"/){L=substr($$ 0,RSTART+RLENGTH);print substr(L,0,match(L,/\"/)-1);exit}" $(ANDROIDPROJ_DIR)/AndroidManifest.xml)
app.minsdk      := $(shell "$(HOST_AWK)" "match($$ 0,/android:minSdkVersion *= *\"/){L=substr($$ 0,RSTART+RLENGTH);print substr(L,0,match(L,/\"/)-1);exit}" $(ANDROIDPROJ_DIR)/AndroidManifest.xml)
ifneq ($(if $(app.package),,$(if $(app.activity),,$(if $(app.versioncode),,1))),)
  $(info Could not read package name, activity name or version code from $(ANDROIDPROJ_DIR)/AndroidManifest.xml.)
  $(info Make sure these values are correctly set.)
  $(error Aborting)
endif
ifneq ($(app.minsdk),)
	APP_PLATFORM := android-$(app.minsdk)
endif

-include assets.mk
ASSET_ALL_PATHS := $(strip $(foreach F,$(ASSETS),$(wildcard ./$(F) ./$(F)/* ./$(F)/*/* ./$(F)/*/*/* ./$(F)/*/*/*/* ./$(F)/*/*/*/*/*)))
ASSET_ALL_STARS := $(if $(ASSET_ALL_PATHS),$(strip $(foreach F,$(subst *./, ,*$(subst $(sp),*,$(ASSET_ALL_PATHS))),$(if $(wildcard $(subst *,\ ,$(F))/.),,$(F)))))
ASSET_PREREQUIS := $(if $(ASSET_ALL_STARS),assets.mk $(subst *,\ ,$(ASSET_ALL_STARS)))

ANDROID_PROJECT_APK := $(ANDROIDPROJ_DIR)/bin/$(ZillaApp)-$(if $(filter 1,$(ZLDEBUG)),debug,release).apk
ANDROID_PROJECT_TARGET := $(shell "$(HOST_AWK)" "/^[\t ]*target[\t ]*=/{gsub(/^[^=]+=[ \t]*/,null);print;exit} " $(ANDROIDPROJ_DIR)/project.properties)
ANDROID_PROJECT_ABI := $(shell "$(HOST_AWK)" "/^[\t ]*APP_ABI/{gsub(/^[^=]+=[ \t]*/,null);gsub(/[ \t].*/,null);print;exit}" $(ANDROIDPROJ_DIR)/jni/Application.mk)
ANDROID_PROJECT_ABI := $(if $(ANDROID_PROJECT_ABI),$(ANDROID_PROJECT_ABI),$(firstword $(NDK_ALL_ABIS)))
ANDROID_PROJ_LIBOUT := $(ANDROIDPROJ_DIR)/libs/$(if $(filter 1,$(ZLDEBUG)),debug,release)
ANDROID_PROJECT_LIB := $(ANDROID_PROJ_LIBOUT)/$(ANDROID_PROJECT_ABI)/lib$(ZillaApp).so
ANDROID_RELEASE_APK := $(if $(SIGN_OUTAPK),$(if $(findstring .apk,$(SIGN_OUTAPK)),$(SIGN_OUTAPK),$(SIGN_OUTAPK)-$(app.versioncode).apk),$(ANDROIDPROJ_DIR)/$(ZillaApp)-$(app.versioncode).apk)

ANDROID_BUILDTOOLS := $(subst *, ,$(lastword $(sort $(subst *$(subst \,/,$(subst $(sp),*,$(ANDROID_SDK))), $(subst \,/,$(subst $(sp),*,$(ANDROID_SDK))),$(subst $(sp),*,$(wildcard $(subst $(sp),\ ,$(subst \,/,$(ANDROID_SDK)))/build-tools/*))))))
ANDROID_ZIPALIGN := $(ANDROID_BUILDTOOLS)/zipalign
ANDROID_DX := $(ANDROID_BUILDTOOLS)/lib/dx.jar
ANDROID_AAPT := $(ANDROID_BUILDTOOLS)/aapt
ANDROID_ADB := $(ANDROID_SDK)/platform-tools/adb

ifeq ($(HOST_OS_BASE),windows)
	ANDROID_DEBUG_KEYSTORE := $(subst \,/,$(USERPROFILE))/.android/debug.keystore
else
	ANDROID_DEBUG_KEYSTORE := ~/.android/debug.keystore
endif

ifeq ($(wildcard $(subst $(sp),\ ,$(ANDROID_DEBUG_KEYSTORE))),)
  $(info )
  $(info Could not find android debug keystore at:$(ANDROID_DEBUG_KEYSTORE))
  $(info Try running a regular android app or sample through the android build system to have it created.)
  $(info Alternatively, create it manually with the following command:)
  $(info      $(JDK_BIN)keytool -genkey -v -keystore $(ANDROID_DEBUG_KEYSTORE) -alias androiddebugkey -storepass android -keypass android -keyalg RSA -validity 14000)
  $(info )
  $(error Aborting)
endif

ZILLALIB_OUTBASE := $(ZILLALIB_DIR)Android/$(if $(filter 1,$(ZLDEBUG)),build-debug,build)

ZILLALIB_OUT := $(ZILLALIB_OUTBASE)/$(ANDROID_PROJECT_ABI)

.PHONY: all sign install run uninstall clean pauseatend
all: $(ANDROID_PROJECT_APK)

CLASSES_DIR := $(ANDROIDPROJ_DIR)/bin/classes
CLASSES_PREREQS := $(if $(filter B,$(MAKEFLAGS)),$(CLASSES_DIR),$(if $(wildcard $(CLASSES_DIR)),$(wildcard $(CLASSES_DIR)/*.class $(CLASSES_DIR)/*/*.class $(CLASSES_DIR)/*/*/*.class $(CLASSES_DIR)/*/*/*/*.class $(CLASSES_DIR)/*/*/*/*/*.class),$(CLASSES_DIR)))

$(CLASSES_PREREQS) : $(wildcard $(ZILLALIB_DIR)Android/*.java) $(or $(wildcard $(ANDROIDPROJ_DIR)/*.java),$(ANDROIDPROJ_DIR)/$(ZillaApp).java)
	$(info Compiling Java classes ...)
	@$(if $(wildcard $(ANDROIDPROJ_DIR)/bin/classes),,$(call host-mkdir,$(ANDROIDPROJ_DIR)/bin/classes))
	@"$(JDK_BIN)javac.exe" -d $(ANDROIDPROJ_DIR)/bin/classes -classpath $(ANDROIDPROJ_DIR)/bin/classes -sourcepath $(ZILLALIB_DIR)Android;. -target 1.5 -bootclasspath "$(ANDROID_SDK)/platforms/$(ANDROID_PROJECT_TARGET)/android.jar" -encoding UTF-8 -g:none -Xlint:-options -source 1.5 $^

$(ANDROIDPROJ_DIR)/bin/base/classes.dex: $(ANDROID_PROJECT_LIB) $(CLASSES_PREREQS)
	$(info Creating classes.dex ...)
	@$(call host-mkdir,$(ANDROIDPROJ_DIR)/bin/base)
	@"$(JDK_BIN)java" -jar "$(ANDROID_DX)" "--dex" "--output=$@" "$(ANDROIDPROJ_DIR)/bin/classes"

$(ANDROIDPROJ_DIR)/bin/res.prepare: $(ANDROIDPROJ_DIR)/res
	$(info Preparing Android resources for packaging...)
	@"$(ANDROID_AAPT)" c -S "$(ANDROIDPROJ_DIR)/res" -C "$(ANDROIDPROJ_DIR)/bin/res" >$(DEVNUL)
	@"$(HOST_ECHO)" 1>"$@"

define BUILD_APK_UNSIGNED
	$(info Building APK Package...)
	@$(if $(filter 1,$(ZLDEBUG)),,$(call host-rm,$(wildcard $(ANDROID_PROJ_LIBOUT)/*/gdb*)))
	@$(if $(filter 1,$(ZLDEBUG)),,$(call host-rm,$(wildcard $(ANDROIDPROJ_DIR)/bin/base/lib/*/gdb*)))
	@$(call host-mkdir,$(ANDROIDPROJ_DIR)/bin/base/lib)
	@$(if $(filter windows,$(HOST_OS)),xcopy /Y /E /Q /I $(subst /,\,$(ANDROID_PROJ_LIBOUT))\* $(ANDROIDPROJ_DIR)\bin\base\lib\ >NUL,cp -rfp $(ANDROID_PROJ_LIBOUT)/* $(ANDROIDPROJ_DIR)/bin/base/lib/)
	@"$(ANDROID_AAPT)" p -f -M "$(ANDROIDPROJ_DIR)/AndroidManifest.xml" -S "$(ANDROIDPROJ_DIR)/bin/res" \
		-I "$(ANDROID_SDK)/platforms/$(ANDROID_PROJECT_TARGET)/android.jar" \
		-F "$(ANDROID_PROJECT_APK).unsigned" $(ANDROIDPROJ_DIR)/bin/base
	$(if $(ASSET_ALL_STARS),$(info    Packing in $(words $(ASSET_ALL_STARS)) binary assets into APK...))
	@$(if $(ASSET_ALL_STARS),"$(ANDROID_AAPT)" add -0 "" "$(ANDROID_PROJECT_APK).unsigned" $(subst *, ,$(subst $(sp)," ","$(ASSET_ALL_STARS)")) >$(DEVNUL))
endef

define BUILD_ZIPALIGN
	@$(if $(wildcard $2),$(if $(filter windows,$(HOST_OS)),del $(subst /,\,"$2"),rm "$2"),)
	$(info    Zipaligning APK...)
	@"$(ANDROID_ZIPALIGN)" 4 "$1" "$2"
	@$(if $(filter windows,$(HOST_OS)),del $(subst /,\,"$(ANDROID_PROJECT_APK).unsigned" "$1"),rm "$(ANDROID_PROJECT_APK).unsigned" "$1")
	$(info APK file $2 done!)
endef

$(ANDROID_PROJECT_APK): $(ANDROIDPROJ_DIR)/bin/base/classes.dex $(ANDROIDPROJ_DIR)/bin/res.prepare $(ANDROIDPROJ_DIR)/AndroidManifest.xml $(ANDROID_PROJECT_LIB) $(ASSET_PREREQUIS)
	$(BUILD_APK_UNSIGNED)
	$(info    Signing APK with debug key $(strip $(notdir $(subst $(sp),_,$(subst \,/,$(ANDROID_DEBUG_KEYSTORE)))))...)
	@"$(JDK_BIN)java" -cp "$(ZILLALIB_DIR)Android/tools.jar" "sun.security.tools.JarSigner" -keystore "$(ANDROID_DEBUG_KEYSTORE)" -storepass android -keypass android -signedjar "$(ANDROID_PROJECT_APK).debugkey" "$(ANDROID_PROJECT_APK).unsigned" androiddebugkey >$(DEVNUL)
	$(call BUILD_ZIPALIGN,$(ANDROID_PROJECT_APK).debugkey,$(ANDROID_PROJECT_APK))

sign: $(ANDROIDPROJ_DIR)/bin/base/classes.dex $(ANDROIDPROJ_DIR)/bin/res.prepare $(ANDROIDPROJ_DIR)/AndroidManifest.xml $(ANDROID_PROJECT_LIB) $(ASSET_PREREQUIS)
	$(if $(SIGN_KEYSTORE),,$(error Please supply keystore file via SIGN_KEYSTORE=<...> make parameter))
	$(if $(SIGN_STOREPASS),,$(error Please supply keystore file password via SIGN_STOREPASS=<...> make parameter))
	$(if $(SIGN_KEYALIAS),,$(error Please supply key alias name via SIGN_KEYALIAS=<...> make parameter))
	$(if $(SIGN_KEYPASS),,$(error Please supply key password file via SIGN_KEYPASS=<...> make parameter))
	$(BUILD_APK_UNSIGNED)
	$(info    Signing APK with distribution key $(SIGN_KEYSTORE)...)
	@"$(JDK_BIN)java" -cp "$(ZILLALIB_DIR)Android/tools.jar" "sun.security.tools.JarSigner" -keystore "$(SIGN_KEYSTORE)" -storepass "$(SIGN_STOREPASS)" -keypass "$(SIGN_KEYPASS)" -signedjar "$(ANDROID_PROJECT_APK).signed" "$(ANDROID_PROJECT_APK).unsigned" "$(SIGN_KEYALIAS)"
	$(call BUILD_ZIPALIGN,$(ANDROID_PROJECT_APK).signed,$(ANDROID_RELEASE_APK))

install: $(ANDROID_PROJECT_APK)
	$(info Installing and running $(app.package) on the default emulator or device...)
	@"$(ANDROID_ADB)" ${adb.device.arg} shell "if [ `cat /sys/class/backlight/*/brightness` = 0 ]; then input keyevent 26; fi 2>/dev/null" >$(DEVNUL)
	@"$(ANDROID_ADB)" ${adb.device.arg} install -r $(ANDROID_PROJECT_APK)
	@"$(ANDROID_ADB)" shell am start -n $(app.package)/$(app.package)$(app.activity)
	$(info Done)

uninstall:
	$(info Uninstalling $(app.package) from the default emulator or device...)
	@"$(ANDROID_ADB)" ${adb.device.arg} uninstall $(app.package)
	$(info Done)

pauseatend:
	$(info )
	@pause

clean:
	$(info  Cleaning build files ...)
	@$(if $(wildcard $(ANDROIDPROJ_DIR)/libs),$(if $(filter windows,$(HOST_OS)),rmdir /S /Q,rm -rf) "$(ANDROIDPROJ_DIR)/libs" >$(DEVNUL),)
	@$(if $(wildcard $(ANDROIDPROJ_DIR)/obj),$(if $(filter windows,$(HOST_OS)),rmdir /S /Q,rm -rf) "$(ANDROIDPROJ_DIR)/obj" >$(DEVNUL),)
	@$(if $(wildcard $(ANDROIDPROJ_DIR)/bin),$(if $(filter windows,$(HOST_OS)),rmdir /S /Q,rm -rf) "$(ANDROIDPROJ_DIR)/bin" >$(DEVNUL),)
	$(info Done)

-include sources.mk
$(ANDROID_PROJECT_LIB): $(wildcard *.cpp *.c) $(foreach F, $(ZL_ADD_SRC_FILES), $(wildcard $(F))) $(ZILLALIB_OUT)/libZillaLib.a $(ZILLALIB_OUT)/libstlport.a #$(NEEDREMOVEGDBSERVER)
	$(info Building JNI Library $(ANDROID_PROJECT_LIB))
	@"$(ANDROID_NDK)/ndk-build" --no-print-directory "NDK_PROJECT_PATH=$(ANDROIDPROJ_DIR)" "NDK_APP_LIBS_OUT=$(ANDROID_PROJ_LIBOUT)" $(NDK_BUILD_OPTS) NDK_APP.local.cleaned_binaries=1 "APP_PLATFORM=$(APP_PLATFORM)" "ZillaApp=$(ZillaApp)"
	@$(if $(filter windows,$(HOST_OS)),dir "$(subst /,\,$(ANDROID_PROJECT_LIB))" 2>$(DEVNUL) >$(DEVNUL),)

#if we're being called with B flag (always-make), build the libraries only if they don't exist at all
ifeq ($(if $(filter B,$(MAKEFLAGS)),$(wildcard $(ZILLALIB_OUT)/libZillaLib.a)),)
$(ZILLALIB_OUT)/libZillaLib.a: $(wildcard $(ZILLALIB_DIR)Include/*.h $(ZILLALIB_DIR)Source/*.cpp $(ZILLALIB_DIR)Source/enet/*.c $(ZILLALIB_DIR)Source/libtess2/*.c $(ZILLALIB_DIR)Source/stb/*.cpp)
	@$(ANDROID_NDK)/ndk-build --no-print-directory "NDK_PROJECT_PATH=$(ZILLALIB_DIR)Android" "APP_BUILD_SCRIPT=$(ZILLALIB_DIR)Android/Android.mk" $(NDK_BUILD_OPTS) TARGET_SONAME_EXTENSION=.a APP_PLATFORM=$(APP_PLATFORM)
$(ZILLALIB_OUT)/libstlport.a:
	@$(ANDROID_NDK)/ndk-build --no-print-directory "NDK_PROJECT_PATH=$(ZILLALIB_DIR)Android/stlport" "APP_BUILD_SCRIPT=$(ZILLALIB_DIR)Android/stlport/Android.mk" -s $(NDK_BUILD_OPTS) TARGET_SONAME_EXTENSION=.a APP_PLATFORM=$(APP_PLATFORM)
endif

#------------------------------------------------------------------------------------------------------
else
#------------------------------------------------------------------------------------------------------

.PHONY: ZillaLibBaseLibs
all: ZillaLibBaseLibs

clean:
	$(info Cleaning everything in $(ZILLALIB_OUTBASE) ...)
	@$(if $(wildcard $(subst $(sp),\ ,$(subst \,/,$(ZILLALIB_OUTBASE)))),$(if $(filter windows,$(HOST_OS)),rmdir /S /Q,rm -rf) "$(ZILLALIB_OUTBASE)" >$(DEVNUL))

ZillaLibBaseLibs:
	@"$(ANDROID_NDK)/ndk-build" --no-print-directory "NDK_PROJECT_PATH=$(ZILLALIB_DIR)Android" "APP_BUILD_SCRIPT=$(ZILLALIB_DIR)Android/Android.mk" $(NDK_BUILD_OPTS) TARGET_SONAME_EXTENSION=.a APP_PLATFORM=$(APP_PLATFORM)
	@"$(ANDROID_NDK)/ndk-build" --no-print-directory "NDK_PROJECT_PATH=$(ZILLALIB_DIR)Android/stlport" "APP_BUILD_SCRIPT=$(ZILLALIB_DIR)Android/stlport/Android.mk" -s $(NDK_BUILD_OPTS) TARGET_SONAME_EXTENSION=.a APP_PLATFORM=$(APP_PLATFORM)

#------------------------------------------------------------------------------------------------------
endif
#------------------------------------------------------------------------------------------------------
