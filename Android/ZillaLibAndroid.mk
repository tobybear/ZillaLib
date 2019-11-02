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
ZILLALIB_DIR := $(patsubst ./%,%,$(dir $(subst / *,,$(subst \,/,$(dir $(THIS_MAKEFILE))) *)))

ifneq ($(words $(ZILLALIB_DIR)/),1)
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
  ISWIN := 1
  RMDIR = rmdir /S /Q $(subst /,\,"$1") >NUL 2>NUL || rem
else
  RMDIR = rm -rf "$1"
endif

ifdef ISWIN
	DEVNUL := nul
	OSJAVA := $(if $(OSJAVA),$(OSJAVA),$(subst *, ,$(lastword $(subst *$(subst \,/,$(ProgramW6432)), $(subst \,/,$(ProgramW6432)),$(subst $(sp),*,$(wildcard $(subst $(sp),\ ,$(subst \,/,$(ProgramW6432)))/Java/*/bin/java.exe))))))
	OSJAVA := $(if $(OSJAVA),$(OSJAVA),$(subst *, ,$(lastword $(subst *$(subst \,/,$(ProgramFiles)), $(subst \,/,$(ProgramFiles)),$(subst $(sp),*,$(wildcard $(subst $(sp),\ ,$(subst \,/,$(ProgramFiles)))/Java/*/bin/java.exe))))))
	OSJAVA := $(if $(OSJAVA),$(OSJAVA),java.exe)
else
	DEVNUL := /dev/null
	OSJAVA := $(if $(OSJAVA),$(OSJAVA),java)
endif

sub_checkexe_run = $(if $($(1)),$($(1)),$(if $(2),$(if $(shell "$(2)" $(3) 2>$(DEVNUL)),$(2),),))
sub_checkexe_find = $(if $($(1)),$($(1)),$(call sub_checkexe_run,$(1),$(wildcard $(subst $(sp),\ ,$(subst \,/,$(2)$(if $(ISWIN),.exe)))),$(3)))
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
ifeq ($(wildcard $(subst $(sp),\ ,$(subst \,/,$(JDK_BIN)javac$(if $(ISWIN),.exe)))),)
  $(info Could not find the Java compiler javac$(if $(JDK_BIN), in the directory $(JDK_BIN)))
  $(info Java development kit needs to be installed with environment variable or Makefile variable JAVA_HOME set, somewhere in the PATH or placed under the Android SDK directory at $(dir $(ANDROID_SDK))jdk - JAVA_HOME can also be set in $(ZILLALIB_DIR)Android/ZillaAppLocalConfig.mk)
  $(error Aborting)
endif

# Find the earliest possible platforms available for each abi
ZL_PLATFORM.armeabi     := $(firstword $(patsubst $(subst \,/,$(NDK_PLATFORMS_ROOT))/%/arch-arm,%,$(sort $(filter %/arch-arm,$(wildcard $(subst $(sp),\ ,$(subst \,/,$(NDK_PLATFORMS_ROOT)))/*/*)))))
ZL_PLATFORM.armeabi-v7a := $(firstword $(patsubst $(subst \,/,$(NDK_PLATFORMS_ROOT))/%/arch-arm,%,$(sort $(filter %/arch-arm,$(wildcard $(subst $(sp),\ ,$(subst \,/,$(NDK_PLATFORMS_ROOT)))/*/*)))))
ZL_PLATFORM.arm64-v8a   := $(firstword $(patsubst $(subst \,/,$(NDK_PLATFORMS_ROOT))/%/arch-arm64,%,$(sort $(filter %/arch-arm64,$(wildcard $(subst $(sp),\ ,$(subst \,/,$(NDK_PLATFORMS_ROOT)))/*/*)))))
ZL_PLATFORM.x86         := $(firstword $(patsubst $(subst \,/,$(NDK_PLATFORMS_ROOT))/%/arch-x86,%,$(sort $(filter %/arch-x86,$(wildcard $(subst $(sp),\ ,$(subst \,/,$(NDK_PLATFORMS_ROOT)))/*/*)))))
ZL_PLATFORM.x86_64      := $(firstword $(patsubst $(subst \,/,$(NDK_PLATFORMS_ROOT))/%/arch-x86_64,%,$(sort $(filter %/arch-x86_64,$(wildcard $(subst $(sp),\ ,$(subst \,/,$(NDK_PLATFORMS_ROOT)))/*/*)))))

# Numbers used as the last digit of deployed version code in AndroidManifest.xml depending on built ABI
ZL_VERSIONCODE.armeabi     := 0
ZL_VERSIONCODE.armeabi-v7a := 1
ZL_VERSIONCODE.arm64-v8a   := 2
ZL_VERSIONCODE.x86         := 3
ZL_VERSIONCODE.x86_64      := 4

NDK_BUILD_OPTS := $(strip $(if $(filter 1,$(V)),V=1,-j 4) $(if $(filter 1,$(ZLDEBUG)),NDK_DEBUG=1,) $(if $(filter B,$(MAKEFLAGS)),-B,))
NDK_BUILD_OPTS += --no-print-directory NDK_NO_INFO=1# Don't print repeated info we already output here
NDK_BUILD_OPTS += "NDK_APP_GDBSERVER=" "NDK_APP_GDBSETUP="# This disables output of gdb files in builds
NDK_BUILD_OPTS += "APP_STL=c++_static"# Build against static libc++
NDK_BUILD_OPTS += "RELEASE_OPTIMIZATION=$(if $(RELEASE_OPTIMIZATION),$(RELEASE_OPTIMIZATION),s)"# (s = optimize size, z = smaller size, lower performance, 2 = larger size, better performance)

ZILLALIB_OUTBASE := $(ZILLALIB_DIR)Android/$(if $(filter 1,$(ZLDEBUG)),build-debug,build)
PTN_ZILLALIB_LIB := $(ZILLALIB_OUTBASE)/%/libZillaLib.a

# default ABIs, can be overridden by Application.mk
BUILD_ABIS := $(strip $(filter $(NDK_ALL_ABIS),armeabi-v7a arm64-v8a))
ifeq ($(BUILD_ABIS),)
  $(error No ABI was selected for building (NDK_ALL_ABIS = $(NDK_ALL_ABIS)))
endif
BUILD_ABIS := $(if $(filter 1,$(ZLDEBUG)),$(firstword $(BUILD_ABIS)),$(BUILD_ABIS))

#------------------------------------------------------------------------------------------------------
ifdef ZillaApp
#------------------------------------------------------------------------------------------------------

NDK_BUILD_OPTS += $(if $(D), "D=$(D)",)
NDK_BUILD_OPTS += $(subst \\\,$(sp),$(foreach F,$(subst \$(sp),\\\,$(W)),-W "$(F)"))

ANDROID_BUILDTOOLS := $(subst *, ,$(lastword $(sort $(subst *$(subst \,/,$(subst $(sp),*,$(ANDROID_SDK))), $(subst \,/,$(subst $(sp),*,$(ANDROID_SDK))),$(subst $(sp),*,$(wildcard $(subst $(sp),\ ,$(subst \,/,$(ANDROID_SDK)))/build-tools/*))))))
ANDROID_ZIPALIGN   := "$(ANDROID_BUILDTOOLS)/zipalign"
ANDROID_AAPT       := "$(ANDROID_BUILDTOOLS)/aapt"
ANDROID_ADB        := "$(ANDROID_SDK)/platform-tools/adb"
ANDROID_DX         := "$(JDK_BIN)java" -jar "$(ANDROID_BUILDTOOLS)/lib/dx.jar"
ANDROID_JARSIGNER  := "$(JDK_BIN)java" -cp "$(ZILLALIB_DIR)Android/tools.jar" "sun.security.tools.JarSigner"

CMD_GETXMLVAR  := "$(HOST_PYTHON)" -c "import sys,re;A=sys.argv;Q=chr(34);m=re.search('<'+A[2]+' [^>]*'+A[3]+'\s*=\s*'+Q+'([^'+Q+']+)',open(A[1],'rb').read(),re.S);[m and sys.stdout.write(m.group(1))]"
CMD_MKMANIFEST := "$(HOST_PYTHON)" -c "import sys,re;A=sys.argv;Q=chr(34);B=chr(92);open(A[2],'wb').write(re.sub('(<manifest [^>]*android:versionCode'+B+'s*='+B+'s*'+Q+'[^'+Q+']*)'+Q,B+'g<1>'+A[3]+Q,open(A[1],'rb').read(),re.S))"

-include assets.mk
ASSET_ALL_PATHS := $(strip $(foreach F,$(ASSETS),$(wildcard ./$(F) ./$(F)/* ./$(F)/*/* ./$(F)/*/*/* ./$(F)/*/*/*/* ./$(F)/*/*/*/*/*)))
ASSET_ALL_STARS := $(if $(ASSET_ALL_PATHS),$(strip $(foreach F,$(subst *./, ,*$(subst $(sp),*,$(ASSET_ALL_PATHS))),$(if $(wildcard $(subst *,\ ,$(F))/.),,$(F)))))
ASSET_PREREQUIS := $(if $(ASSET_ALL_STARS),assets.mk $(subst *,\ ,$(ASSET_ALL_STARS)))

ifdef ISWIN
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

ANDROIDPROJ_DIR := Android

ifeq ($(wildcard $(ANDROIDPROJ_DIR)/jni/Android.mk),)
  $(error File $(ANDROIDPROJ_DIR)/jni/Android.mk not found. Make sure it exists and that the working directory is the project root)
endif
ifeq ($(wildcard $(ANDROIDPROJ_DIR)/jni/Application.mk),)
  $(error File $(ANDROIDPROJ_DIR)/jni/Application.mk not found. Make sure it exists and that the working directory is the project root)
endif

app.package     := $(shell $(CMD_GETXMLVAR) "$(ANDROIDPROJ_DIR)/AndroidManifest.xml" "manifest" "package")
app.versioncode := $(shell $(CMD_GETXMLVAR) "$(ANDROIDPROJ_DIR)/AndroidManifest.xml" "manifest" "android:versionCode)
app.minsdk      := $(shell $(CMD_GETXMLVAR) "$(ANDROIDPROJ_DIR)/AndroidManifest.xml" "uses-sdk" "android:minSdkVersion)
app.targetsdk   := $(shell $(CMD_GETXMLVAR) "$(ANDROIDPROJ_DIR)/AndroidManifest.xml" "uses-sdk" "android:targetSdkVersion)
app.activity    := $(shell $(CMD_GETXMLVAR) "$(ANDROIDPROJ_DIR)/AndroidManifest.xml" "activity" "android:name" ".")
ifneq ($(if $(app.package),,$(if $(app.activity),,$(if $(app.versioncode),,$(if $(app.targetsdk),,1)))),)
  $(info Could not read package name, version code, target sdk or activity name from $(ANDROIDPROJ_DIR)/AndroidManifest.xml.)
  $(info Make sure these values are correctly set.)
  $(error Aborting)
endif
ifneq ($(app.minsdk),)
ifneq ($(call gt,$(lastword $(subst -, ,$(ZL_PLATFORM.armeabi))),$(app.minsdk)),)
  $(warning App minSdkVersion defined in $(ANDROIDPROJ_DIR)/AndroidManifest.xml is $(app.minsdk) which is smaller than the earliest SDK ($(lastword $(subst -, ,$(ZL_PLATFORM.armeabi)))) supported by this NDK installation. Please update the manifest.)
endif
endif
ifneq ($(call lt,$(app.targetsdk),26),)
  $(info App targetSdkVersion defined in $(ANDROIDPROJ_DIR)/AndroidManifest.xml is $(app.targetsdk) which is smaller than the earliest required SDK (26). Highest available SDK is $(lastword $(sort $(subst $(ANDROID_SDK)/platforms/android-,,$(wildcard $(subst $(sp),\ ,$(ANDROID_SDK)/platforms/android-*))))). Please update the manifest.)
  $(error Aborting)
endif
ifeq ($(wildcard $(subst $(sp),\ ,$(ANDROID_SDK)/platforms/android-$(app.targetsdk)/android.jar)),)
  $(info Could not find platform support for selected target sdk $(app.targetsdk) in android sdk installation at $(ANDROID_SDK)/platforms/android-$(app.targetsdk)/android.jar)
  $(error Aborting)
endif

ANDROID_PROJECT_APKBASE := $(ANDROIDPROJ_DIR)/bin/$(ZillaApp)-$(if $(filter 1,$(ZLDEBUG)),debug,release)
ANDROID_PROJECT_OUTBASE := $(ANDROIDPROJ_DIR)/bin/$(if $(filter 1,$(ZLDEBUG)),debug,release)-
ANDROID_SIGN_APKBASE    := $(if $(SIGN_OUTAPK),$(if $(filter .apk,$(suffix $(SIGN_OUTAPK))),$(basename $(SIGN_OUTAPK)),$(SIGN_OUTAPK)-$(app.versioncode)),$(ANDROIDPROJ_DIR)/$(ZillaApp)-$(app.versioncode))

-include $(ANDROIDPROJ_DIR)/jni/Application.mk
ifneq ($(APP_ABI),)
  BUILD_ABIS := $(filter $(NDK_ALL_ABIS),$(APP_ABI))
  $(if $(filter-out $(NDK_ALL_ABIS),$(APP_ABI)),$(error The projects Application.mk requested to build for an invalid ABI - Requested: [$(APP_ABI)] - Supported by NDK: [$(NDK_ALL_ABIS)]))
  BUILD_ABIS := $(if $(filter 1,$(ZLDEBUG)),$(firstword $(BUILD_ABIS)),$(BUILD_ABIS))
endif

PTN_ANDROID_PROJECT_LIB := $(ANDROID_PROJECT_OUTBASE)%/lib/%/lib$(ZillaApp).so
PTN_ANDROID_PROJECT_APK := $(ANDROID_PROJECT_APKBASE)-%.apk
PTN_ANDROID_SIGN_APK    := $(ANDROID_SIGN_APKBASE)$(if $(and $(filter .apk,$(suffix $(SIGN_OUTAPK))),$(filter 1,$(words $(BUILD_ABIS)))),,-%).apk
GET_PROJLIB_ABI = $(notdir $(patsubst $(ANDROID_PROJECT_OUTBASE)%/lib$(ZillaApp).so,%,$1))
ANDROID_PROJECT_LIBS := $(foreach F,$(BUILD_ABIS),$(ANDROID_PROJECT_OUTBASE)$(F)/lib/$(F)/lib$(ZillaApp).so)
ANDROID_PROJECT_APKS := $(BUILD_ABIS:%=$(PTN_ANDROID_PROJECT_APK))
ANDROID_SIGN_APKS    := $(BUILD_ABIS:%=$(PTN_ANDROID_SIGN_APK))
ZILLALIB_LIBS        := $(BUILD_ABIS:%=$(PTN_ZILLALIB_LIB))

.PHONY: all sign install run uninstall clean pauseatend
all: $(ANDROID_PROJECT_APKS)

define GENERATE_ABI_PREREQUIREMENT
$(subst %,$1,$(PTN_ANDROID_PROJECT_LIB)): $(subst %,$1,$(PTN_ZILLALIB_LIB))
$(subst %,$1,$(PTN_ANDROID_PROJECT_APK)): $(subst %,$1,$(PTN_ANDROID_PROJECT_LIB))
$(subst %,$1,$(PTN_ANDROID_SIGN_APK)):    $(subst %,$1,$(PTN_ANDROID_PROJECT_LIB))
endef
$(foreach F,$(BUILD_ABIS),$(eval $(call GENERATE_ABI_PREREQUIREMENT,$(F))))

CLASSES_DIR := $(ANDROIDPROJ_DIR)/bin/classes
CLASSES_DEFAULT := $(CLASSES_DIR)/org/zillalib/ZillaActivity.class
CLASSES_PREREQS := $(if $(filter B,$(MAKEFLAGS)),$(CLASSES_DEFAULT),$(if $(wildcard $(CLASSES_DEFAULT)),$(wildcard $(CLASSES_DIR)/*.class $(CLASSES_DIR)/*/*.class $(CLASSES_DIR)/*/*/*.class $(CLASSES_DIR)/*/*/*/*.class $(CLASSES_DIR)/*/*/*/*/*.class),$(CLASSES_DEFAULT)))

$(CLASSES_PREREQS) : $(wildcard $(ZILLALIB_DIR)Android/*.java) $(or $(wildcard $(ANDROIDPROJ_DIR)/*.java),$(ANDROIDPROJ_DIR)/$(ZillaApp).java)
	$(info Compiling Java classes)
	@$(call RMDIR,$(ANDROIDPROJ_DIR)/bin/classes)
	@$(call host-mkdir,$(ANDROIDPROJ_DIR)/bin/classes)
	@"$(JDK_BIN)javac.exe" -d $(ANDROIDPROJ_DIR)/bin/classes -classpath $(ANDROIDPROJ_DIR)/bin/classes -sourcepath $(ZILLALIB_DIR)Android;. -target 1.5 -bootclasspath "$(ANDROID_SDK)/platforms/android-$(app.targetsdk)/android.jar" -encoding UTF-8 -g:none -Xlint:deprecation -Xlint:-options -source 1.5 $^

$(ANDROIDPROJ_DIR)/bin/dex/classes.dex: $(CLASSES_PREREQS)
	$(info Creating classes.dex)
	@$(call host-mkdir,$(ANDROIDPROJ_DIR)/bin/dex)
	@$(ANDROID_DX) "--dex" "--output=$@" "$(ANDROIDPROJ_DIR)/bin/classes"

$(ANDROIDPROJ_DIR)/bin/res.prepare: $(ANDROIDPROJ_DIR)/res
	$(info Preparing Android resources for packaging...)
	@$(ANDROID_AAPT) c -S "$(ANDROIDPROJ_DIR)/res" -C "$(ANDROIDPROJ_DIR)/bin/res" >$(DEVNUL)
	@"$(HOST_ECHO)" 1>"$@"

define BUILD_APK_UNSIGNED
	@"$(HOST_ECHO)" "    Building APK Package $(1)..."
	@$(CMD_MKMANIFEST) "$(ANDROIDPROJ_DIR)/AndroidManifest.xml" "$(ANDROID_PROJECT_OUTBASE)$2/AndroidManifest.xml" "$(ZL_VERSIONCODE.$(2))"
	$(ANDROID_AAPT) p -f -S "$(ANDROIDPROJ_DIR)/bin/res" -I "$(ANDROID_SDK)/platforms/android-$(app.targetsdk)/android.jar" -F "$(1)" "$(ANDROID_PROJECT_OUTBASE)$2" "$(ANDROIDPROJ_DIR)/bin/dex"
	@$(if $(ASSET_ALL_STARS),"$(HOST_ECHO)" "    Packing in $(words $(ASSET_ALL_STARS)) binary assets into APK...")
	@$(if $(ASSET_ALL_STARS),$(ANDROID_AAPT) add -0 "" "$(1)" $(subst *, ,$(subst $(sp)," ","$(ASSET_ALL_STARS)")) >$(DEVNUL))
endef

define BUILD_ZIPALIGN
	@$(if $(wildcard $2),$(if $(ISWIN),del $(subst /,\,"$2"),rm "$2"),)
	@"$(HOST_ECHO)" "    Zipaligning APK..."
	@$(ANDROID_ZIPALIGN) 4 "$1" "$2"
	@$(if $(ISWIN),del $(subst /,\,"$(2).unsigned" "$1"),rm "$(2).unsigned" "$1") >$(DEVNUL)
	@"$(HOST_ECHO)" "    APK file $2 done!"
endef

$(ANDROID_PROJECT_APKS): $(ANDROIDPROJ_DIR)/bin/dex/classes.dex $(ANDROIDPROJ_DIR)/bin/res.prepare $(ANDROIDPROJ_DIR)/AndroidManifest.xml $(ASSET_PREREQUIS)
	$(info Creating APK file $@ signed with debug key...)
	$(call BUILD_APK_UNSIGNED,$(@).unsigned,$(patsubst $(PTN_ANDROID_PROJECT_APK),%,$@))
	@"$(HOST_ECHO)" "    Signing APK with debug key $(strip $(notdir $(subst $(sp),_,$(subst \,/,$(ANDROID_DEBUG_KEYSTORE)))))..."
	@$(ANDROID_JARSIGNER) -keystore "$(ANDROID_DEBUG_KEYSTORE)" -storepass android -keypass android -signedjar "$(@).debugkey" "$(@).unsigned" androiddebugkey >$(DEVNUL)
	$(call BUILD_ZIPALIGN,$(@).debugkey,$(@))

sign: $(ANDROID_SIGN_APKS)
$(ANDROID_SIGN_APKS): $(ANDROIDPROJ_DIR)/bin/dex/classes.dex $(ANDROIDPROJ_DIR)/bin/res.prepare $(ANDROIDPROJ_DIR)/AndroidManifest.xml $(ASSET_PREREQUIS)
	$(info Creating signed APK file $@ ...)
	$(if $(SIGN_KEYSTORE),,$(error Please supply keystore file via SIGN_KEYSTORE=<...> make parameter))
	$(if $(SIGN_STOREPASS),,$(error Please supply keystore file password via SIGN_STOREPASS=<...> make parameter))
	$(if $(SIGN_KEYALIAS),,$(error Please supply key alias name via SIGN_KEYALIAS=<...> make parameter))
	$(if $(SIGN_KEYPASS),,$(error Please supply key password file via SIGN_KEYPASS=<...> make parameter))
	$(call BUILD_APK_UNSIGNED,$(@).unsigned,$(if $(findstring %,$(PTN_ANDROID_SIGN_APK)),$(patsubst $(PTN_ANDROID_SIGN_APK),%,$@),$(BUILD_ABIS)))
	@"$(HOST_ECHO)" "    Signing APK with distribution key $(SIGN_KEYSTORE)..."
	@$(ANDROID_JARSIGNER) -keystore "$(SIGN_KEYSTORE)" -storepass "$(SIGN_STOREPASS)" -keypass "$(SIGN_KEYPASS)" -signedjar "$(@).signed" "$(@).unsigned" "$(SIGN_KEYALIAS)"
	$(call BUILD_ZIPALIGN,$(@).signed,$(@))

install: $(ANDROID_PROJECT_APKBASE)-$(firstword $(BUILD_ABIS)).apk
	$(info Installing and running $(app.package) on the default emulator or device...)
	@$(ANDROID_ADB) ${adb.device.arg} install -r "$^"
	@$(ANDROID_ADB) ${adb.device.arg} shell "if [ `cat /sys/class/backlight/*/brightness` = 0 ]; then input keyevent 26; else input keyevent -1; fi 2>/dev/null" >$(DEVNUL)
	@$(ANDROID_ADB) shell am start -n $(app.package)/$(app.package)$(app.activity)
	@"$(HOST_ECHO)" "Done"

uninstall:
	$(info Uninstalling $(app.package) from the default emulator or device...)
	@$(ANDROID_ADB) ${adb.device.arg} uninstall $(app.package)
	@"$(HOST_ECHO)" "Done"

pauseatend:
	$(info )
	@pause

clean:
	$(info  Cleaning build files ...)
	@$(if $(wildcard $(ANDROIDPROJ_DIR)/libs),$(call RMDIR,$(ANDROIDPROJ_DIR)/libs))
	@$(if $(wildcard $(ANDROIDPROJ_DIR)/obj),$(call RMDIR,$(ANDROIDPROJ_DIR)/obj))
	@$(if $(wildcard $(ANDROIDPROJ_DIR)/bin),$(call RMDIR,$(ANDROIDPROJ_DIR)/bin))
	@"$(HOST_ECHO)" "Done"

-include sources.mk
$(ANDROID_PROJECT_LIBS): $(wildcard *.cpp *.c) $(foreach F, $(ZL_ADD_SRC_FILES), $(wildcard $(F)))
	$(info Building JNI Library $@ (platform: $(ZL_PLATFORM.$(call GET_PROJLIB_ABI,$@)), abi: $(call GET_PROJLIB_ABI,$@)))
	@"$(ANDROID_NDK)/ndk-build" "NDK_PROJECT_PATH=$(ANDROIDPROJ_DIR)" "NDK_APP_LIBS_OUT=$(ANDROID_PROJECT_OUTBASE)$(call GET_PROJLIB_ABI,$@)/lib" $(NDK_BUILD_OPTS) "APP_PLATFORM=$(ZL_PLATFORM.$(call GET_PROJLIB_ABI,$@))" "APP_ABI=$(call GET_PROJLIB_ABI,$@)" NDK_APP.local.cleaned_binaries=1 "ZillaApp=$(ZillaApp)"
	@$(if $(ISWIN),dir "$(subst /,\,$@)" 2>$(DEVNUL) >$(DEVNUL),)

#if we're being called with B flag (always-make), build the libraries only if they don't exist at all
ifeq ($(if $(filter B,$(MAKEFLAGS)),$(wildcard $(firstword $(ZILLALIB_LIBS)))),)
$(ZILLALIB_LIBS): $(wildcard $(ZILLALIB_DIR)Include/*.h $(ZILLALIB_DIR)Source/*.cpp $(ZILLALIB_DIR)Source/enet/*.c $(ZILLALIB_DIR)Source/libtess2/*.c $(ZILLALIB_DIR)Source/stb/*.cpp)
	@"$(ANDROID_NDK)/ndk-build" "NDK_PROJECT_PATH=$(ZILLALIB_DIR)Android" "APP_BUILD_SCRIPT=$(ZILLALIB_DIR)Android/Android.mk" $(NDK_BUILD_OPTS) "APP_PLATFORM=$(ZL_PLATFORM.$(patsubst $(PTN_ZILLALIB_LIB),%,$@))" "APP_ABI=$(patsubst $(PTN_ZILLALIB_LIB),%,$@)"
endif

#------------------------------------------------------------------------------------------------------
else
#------------------------------------------------------------------------------------------------------

ZILLALIB_LIBS := $(BUILD_ABIS:%=$(PTN_ZILLALIB_LIB))

.PHONY: $(ZILLALIB_LIBS)
all: $(ZILLALIB_LIBS)

clean:
	$(info Cleaning everything in $(ZILLALIB_OUTBASE) ...)
	@$(if $(wildcard $(subst $(sp),\ ,$(subst \,/,$(ZILLALIB_OUTBASE)))),$(call RMDIR,$(ZILLALIB_OUTBASE)))

$(ZILLALIB_LIBS):
	@"$(ANDROID_NDK)/ndk-build" "NDK_PROJECT_PATH=$(ZILLALIB_DIR)Android" "APP_BUILD_SCRIPT=$(ZILLALIB_DIR)Android/Android.mk" $(NDK_BUILD_OPTS) "APP_PLATFORM=$(ZL_PLATFORM.$(patsubst $(PTN_ZILLALIB_LIB),%,$@))" "APP_ABI=$(patsubst $(PTN_ZILLALIB_LIB),%,$@)"

#------------------------------------------------------------------------------------------------------
endif
#------------------------------------------------------------------------------------------------------
