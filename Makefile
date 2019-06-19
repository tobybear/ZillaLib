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

ifndef ZL_IS_APP_MAKE

ZILLALIB_PATH := $(or $(ZILLALIB_PATH),$(subst / *,,$(dir $(subst \,/,$(lastword $(MAKEFILE_LIST)))) *))

.PHONY: all help helpheader linux-help wasm-help emscripten-help nacl-help android-help osx-help ios-help help-all
all: help
help-all: help helpheader linux-help wasm-help emscripten-help nacl-help android-help osx-help ios-help
ISWIN = $(findstring :,$(firstword $(subst \, ,$(subst /, ,$(abspath .)))))
ISOSX = $(wildcard /Applications)
ISLIN = $(wildcard /proc)
helpheader:
	@:    $(info )$(eval #: don't print "Nothing to be done for" message)
	$(info $( ) ZillaLib Makefile Help)
	$(info ========================)
	$(info )
help: ZLHELP_OTHER = $(strip $(if $(ISLIN),,linux )$(if $(wildcard $(ZILLALIB_PATH)/WebAssembly/ZillaAppLocalConfig.mk),,wasm )$(if $(wildcard $(ZILLALIB_PATH)/Emscripten/ZillaAppLocalConfig.mk),,emscripten )$(if $(wildcard $(ZILLALIB_PATH)/NACL/ZillaAppLocalConfig.mk),,nacl )$(if $(wildcard Android),,android )$(if $(ISOSX),,osx ios ))
help: ZLHELP_ECHOOTHER = $(if $(ZLHELP_OTHER),$(info )$(info Other platforms (requiring further setup): $(ZLHELP_OTHER)))
help:helpheader
	$(if $(ISLIN),$(info Platform: linux      - Commands: linux linux-debug linux-release linux-releasedbg linux-clean linux-debug-clean linux-release-clean linux-releasedbg-clean$(if $(ZillaApp), linux-run linux-gdb linux-debug-run linux-release-run linux-releasedbg-run linux-debug-gdb linux-release-gdb linux-releasedbg-gdb)))
	$(if $(wildcard $(ZILLALIB_PATH)/WebAssembly/ZillaAppLocalConfig.mk),$(info Platform: wasm       - Commands: wasm wasm-clean wasm-debug wasm-release wasm-debug-clean wasm-release-clean$(if $(ZillaApp), wasm-run wasm-debug-run wasm-release-run)))
	$(if $(wildcard $(ZILLALIB_PATH)/Emscripten/ZillaAppLocalConfig.mk),$(info Platform: emscripten - Commands: emscripten emscripten-clean emscripten-debug emscripten-release emscripten-debug-clean emscripten-release-clean$(if $(ZillaApp), emscripten-run emscripten-debug-run emscripten-release-run)))
	$(if $(wildcard $(ZILLALIB_PATH)/NACL/ZillaAppLocalConfig.mk),$(info Platform: nacl       - Commands: nacl nacl-clean nacl-run nacl-debug nacl-release nacl-debug-clean nacl-release-clean nacl-debug-run nacl-release-run))
	$(if $(wildcard Android),$(info Platform: android    - Commands: android android-clean android-install android-uninstall android-sign android-debug android-release android-debug-clean android-release-clean android-debug-install android-release-install))
	$(if $(ISOSX),$(info Platform: osx        - Commands: osx osx-clean osx-cleanall osx-run osx-lldb osx-debug osx-release osx-debug-clean osx-release-clean osx-debug-cleanall osx-release-cleanall osx-debug-run osx-release-run))
	$(if $(ISOSX),$(info Platform: ios        - Commands: ios ios-clean ios-cleanall ios-run ios-simulator ios-phone ios-simulator-run ios-archive))
	$(ZLHELP_ECHOOTHER)$(info )
	$(info You can run "$(MAKE) {platform-name}-help" to get further setup instructions)$(info )
linux-help:helpheader
	$(info Linux Requirements:)
	$(info $( )    Building for linux requires an installed and running linux operating system (either 32 or 64-bit) and the following packages installed:)
	$(info $( )        gcc, make, g++, libgl1-mesa-dev, libasound2-dev)
	$(info $( )    By default, it will use the system machine (32 or 64-bit) for building)
	$(info $( )    If you have a cross compile environment set up, you can specify to build a specific machine target by supplying M32=1 or M64=1 as make parameter)
	$(info )
	$(info Linux Make Targets:)
	$(info $( )    linux       | linux-debug       | linux-release       | linux-releasedbg       -- Builds the $(if $(ZillaApp),game executable,static ZillaLib library))
	$(info $( )    linux-clean | linux-debug-clean | linux-release-clean | linux-releasedbg-clean -- Clean the build output directory)
	$(if $(ZillaApp),$(info $( )    linux-run   | linux-debug-run   | linux-release-run   | linux-releasedbg-run   -- Build and run the game))
	$(if $(ZillaApp),$(info $( )    linux-gdb   | linux-debug-gdb   | linux-release-gdb   | linux-releasedbg-gdb   -- Build and run the game with GDB))
	$(info )$(info $( )    (if no configuration is supplied in the make target name, the default (debug) configuration will be used))$(info )
wasm-help:helpheader
	$(info Wasm Requirements:)
	$(info $( )    A WebAssembly build environment, see $(ZILLALIB_PATH)/WebAssembly/README.md for further instructions.)
	$(info $( )    Building requires the configuration file "$(ZILLALIB_PATH)/WebAssembly/ZillaAppLocalConfig.mk" with the following definitions:)
	$(info $( )        LLVM_ROOT = $(if $(ISWIN),D:)/path/to/clang/e1.35.0_64bit             #path to root of LLVM/Clang (with clang$(if $(ISWIN),.exe) etc. in it))
	$(info $( )        SYSTEM_ROOT = $(if $(ISWIN),D:)/path/to/emscripten/system             #path to web platform system root (https://github.com/emscripten-core/emscripten/tree/incoming/system))
	$(info $( )        PYTHON = $(if $(ISWIN),D:)/path/to/python/python$(if $(ISWIN),.exe,    )                  #path to Python executable (only required if not in PATH))
	$(info $( )        WASMOPT = $(if $(ISWIN),D:)/path/to/binaryen/wasm-opt$(if $(ISWIN),.exe,    )             #[OPTIONAL] path to Wasm optimizer to create smaller .wasm files)
	$(info $( )        7ZIP = $(if $(ISWIN),D:)/path/to/7z$(if $(ISWIN),.exe,    )                               #[OPTIONAL] path to 7z$(if $(ISWIN),.exe) for better output gz compression)
	$(info $( )        BROWSER = $(if $(ISWIN),D:)/path/to/browser/browser$(if $(ISWIN),.exe,    )               #[OPTIONAL] path to a browser for run targets)
	$(info )
	$(info Wasm Make Targets:)
	$(info $( )    wasm       | wasm-debug       | wasm-release       -- Builds the $(if $(ZillaApp),game executable,static ZillaLib library))
	$(info $( )    wasm-clean | wasm-debug-clean | wasm-release-clean -- Clean the build output directory)
	$(if $(ZillaApp),$(info $( )    wasm-run   | wasm-debug-run   | wasm-release-run   -- Build and run the game))
	$(info )$(info $( )    (if no configuration is supplied in the make target name, the default (release) configuration will be used))$(info )
emscripten-help:helpheader
	$(info Emscripten Requirements:)
	$(info $( )    An Emscripten build environment, see the Emscripten homepage or $(ZILLALIB_PATH)/Emscripten/README.md for further instructions.)
	$(info $( )    Building requires the configuration file "$(ZILLALIB_PATH)/Emscripten/ZillaAppLocalConfig.mk" with the following definitions:)
	$(info $( )        EMSCRIPTEN_ROOT = $(if $(ISWIN),D:)/path/to/emscripten/1.35.0         #path to Emscripten root with emcc.py etc. in it)
	$(info $( )        LLVM_ROOT = $(if $(ISWIN),D:)/path/to/clang/e1.35.0_64bit             #path to LLVM/Clang for Emscripten root with clang++$(if $(ISWIN),.exe) etc. in it)
	$(info $( )        EMSCRIPTEN_NATIVE_OPTIMIZER = $(if $(ISWIN),D:)/path/to/optimizer$(if $(ISWIN),.exe,    ) #path to Emscripten optimizer executable)
	$(info $( )        NODE_JS = $(if $(ISWIN),D:)/path/to/node/node$(if $(ISWIN),.exe,    )                     #path to Node.js executable)
	$(info $( )        PYTHON = $(if $(ISWIN),D:)/path/to/python/python$(if $(ISWIN),.exe,    )                  #path to Python executable (only required if not in PATH))
#	$(info $( )        JAVA = $(if $(ISWIN),D:)/path/to/java/bin/java$(if $(ISWIN),.exe,    )                    #[OPTIONAL] path to Java executable for minifying output (ca. 15% less))
	$(info $( )        7ZIP = $(if $(ISWIN),D:)/path/to/7z$(if $(ISWIN),.exe,    )                               #[OPTIONAL] path to 7z$(if $(ISWIN),.exe) for better output gz compression)
	$(info $( )        BROWSER = $(if $(ISWIN),D:)/path/to/browser/browser$(if $(ISWIN),.exe,    )               #[OPTIONAL] path to a browser for run targets)
	$(info )
	$(info Emscripten Make Targets:)
	$(info $( )    emscripten       | emscripten-debug       | emscripten-release       -- Builds the $(if $(ZillaApp),game executable,static ZillaLib library))
	$(info $( )    emscripten-clean | emscripten-debug-clean | emscripten-release-clean -- Clean the build output directory)
	$(if $(ZillaApp),$(info $( )    emscripten-run   | emscripten-debug-run   | emscripten-release-run   -- Build and run the game))
	$(info )$(info $( )    (if no configuration is supplied in the make target name, the default (release) configuration will be used))$(info )
nacl-help:helpheader
	$(info NACL Requirements:)
	$(info $( )    A nacl build environment, see the Chrome Native Client homepage or $(ZILLALIB_PATH)/NACL/README.md for further instructions.)
	$(info $( )    Building requires the configuration file "$(ZILLALIB_PATH)/NACL/ZillaAppLocalConfig.mk" with the following definitions:)
	$(info $( )        NACL_SDK = $(if $(ISWIN),D:)/path/to/nacl_sdk/pepper_47       #path to NACL SDK version root with toolchain etc. in it)
	$(info $( )        PYTHON = $(if $(ISWIN),D:)/path/to/python/python$(if $(ISWIN),.exe,    )          #path to Python (only required if not in PATH))
	$(info $( )        7ZIP = $(if $(ISWIN),D:)/path/to/7z$(if $(ISWIN),.exe,    )                       #[OPTIONAL] path to 7z$(if $(ISWIN),.exe) for better output gz compression)
	$(info $( )        CHROME = $(if $(ISWIN),D:)/path/to/Chromium/chrome$(if $(ISWIN),.exe,    )        #[OPTIONAL] path to Chrome/Chromium browser for run targets)
	$(info )
	$(info NACL Make Targets:)
	$(info $( )    nacl       | nacl-debug       | nacl-release       -- Builds the $(if $(ZillaApp),game executable,static ZillaLib library))
	$(info $( )    nacl-clean | nacl-debug-clean | nacl-release-clean -- Clean the build output directory)
	$(if $(ZillaApp),$(info $( )    nacl-run   | nacl-debug-run   | nacl-release-run   -- Build and run the game))
	$(info )$(info $( )    (if no configuration is supplied in the make target name, the default (release) configuration will be used))$(info )
android-help:helpheader
	$(info Android Requirements:)
	$(info $( )    An Android build environment with both SDK and NDK, see the Android developers homepage or $(ZILLALIB_PATH)/Android/README.md for further instructions.)
	$(info $( )    Building requires the configuration file "$(ZILLALIB_PATH)/Android/ZillaAppLocalConfig.mk" with the following definitions:)
	$(info $( )        ANDROID_SDK = $(if $(ISWIN),D:)/path/to/android-sdk    #path to Android SDK)
	$(info $( )        ANDROID_NDK = $(if $(ISWIN),D:)/path/to/android-ndk    #path to Android NDK)
	$(info )
	$(info $( )    An installed Java developer kit distribution is required. If not installed globally as the default java environment, you can set the path to the)
	$(info $( )    JDK with the JAVA_HOME setting in the configuration file mentioned above (not containing the $(if $(ISWIN),/bin/java.exe part - i.e. "JAVA_HOME = D:/dev/Java/JDK81",/bin/java part - i.e. "JAVA_HOME = /home/user/JDK81")).)
	$(info )
	$(info Android Make Targets:)
	$(info $( )    android         | android-debug         | android-release         -- Builds the $(if $(ZillaApp),game APK file,static ZillaLib library))
	$(info $( )    android-clean   | android-debug-clean   | android-release-clean   -- Clean the build output directory)
	$(if $(ZillaApp),$(info $( )    android-install | android-debug-install | android-release-install -- Build and install the game on the connected default development device))
	$(if $(ZillaApp),$(info $( )    android-uninstall  -- Removes the installed game from the connected default development device))
	$(if $(ZillaApp),$(info $( )    android-sign       -- Build and sign the output release apk for uploading onto a store))
	$(if $(ZillaApp),$(info $( )                          The key file path and other settings will be asked during the process, or can be supplied as commandline parameters))
	$(info )$(info $( )    (if no configuration is supplied in the make target name, the default (release) configuration will be used))$(info )
osx-help:helpheader
	$(info OSX Requirements:)
	$(info $( )    Building for Mac OS X requires an installed and running Mac OS X operating system and Xcode installed.)
	$(info )
	$(info OSX Make Targets:)
	$(info $( )    osx          | osx-debug          | osx-release          -- Builds the $(if $(ZillaApp),game executable,static ZillaLib library))
	$(info $( )    osx-clean    | osx-debug-clean    | osx-release-clean    -- Clean the build output directory)
	$(if $(ZillaApp),$(info $( )    osx-cleanall | osx-debug-cleanall | osx-release-cleanall -- Clean the output directory including all dependencies))
	$(if $(ZillaApp),$(info $( )    osx-run      | osx-debug-run      | osx-release-run      -- Build and run the game))
	$(if $(ZillaApp),$(info $( )    osx-lldb     | osx-debug-lldb     | osx-release-lldb     -- Build and run the game with LLDB))
	$(info )$(info $( )    (if no configuration is supplied in the make target name, the default (debug) configuration will be used))$(info )
ios-help:helpheader
	$(info iOS Requirements:)
	$(info $( )    Building for iOS requires an installed and running Mac OS X operating system and Xcode installed.)
	$(info )
	$(info iOS Make Targets:)
	$(info $( )    ios | ios-simulator | ios-phone   -- Builds the $(if $(ZillaApp),game,static ZillaLib library) for testing on either simulator or phone targets)
	$(info $( )    ios-clean                         -- Clean the build output directory)
	$(if $(ZillaApp),$(info $( )    ios-cleanall                      -- Clean the output directory including all dependencies))
	$(if $(ZillaApp),$(info $( )    ios-run | ios-simulator-run       -- Build and run the game on the iOS simulator))
	$(if $(ZillaApp),$(info $( )    ios-list-simulators               -- Show a list of available iOS simulator devices))
	$(if $(ZillaApp),$(info $( )    ios-archive                       -- Build and sign the game archive for releasing))
	$(if $(ZillaApp),$(info $( )    (ios-archive is the only build target that compiles the release configuration, all other targets compile as debug)))
	$(if $(ZillaApp),$(info )$(info $( )    To specify a device to be used when running on the simulator, add parameter "IOSSIMULATOR=<device name>" (default is iPhone 5s).))
	$(info )$(info $( )    If no device target is supplied in the make target name, the default (simulator) device target will be used.)$(info )

#------------------------------------------------------------------------------------------------------

debug            : $(if $(ISLIN),linux-debug)$(if $(ISOSX),osx-debug)
release          : $(if $(ISLIN),linux-release)$(if $(ISOSX),osx-release)
releasedbg       : $(if $(ISLIN),linux-releasedbg)
clean            : $(if $(ISLIN),linux-clean)$(if $(ISOSX),osx-clean)
debug-clean      : $(if $(ISLIN),linux-debug-clean)$(if $(ISOSX),osx-debug-clean)
release-clean    : $(if $(ISLIN),linux-release-clean)$(if $(ISOSX),osx-release-clean)
releasedbg-clean : $(if $(ISLIN),linux-releasedbg-clean)

#------------------------------------------------------------------------------------------------------
ifdef ZillaApp
#------------------------------------------------------------------------------------------------------

run              : $(if $(ISLIN),linux-run)$(if $(ISOSX),osx-run)
debug-run        : $(if $(ISLIN),linux-debug-run)$(if $(ISOSX),osx-debug-run)
release-run      : $(if $(ISLIN),linux-release-run)$(if $(ISOSX),osx-release-run)
releasedbg-run   : $(if $(ISLIN),linux-releasedbg-run)
cleanall         : $(if $(ISOSX),osx-cleanall)
debug-cleanall   : $(if $(ISOSX),osx-debug-cleanall)
release-cleanall : $(if $(ISOSX),osx-release-cleanall)
gdb              : $(if $(ISLIN),linux-gdb)
lldb             : $(if $(ISOSX),osx-lldb)
debug-gdb        : $(if $(ISLIN),linux-debug-gdb)
debug-lldb       : $(if $(ISOSX),osx-debug-lldb)
release-gdb      : $(if $(ISLIN),linux-release-gdb)
release-lldb     : $(if $(ISOSX),osx-release-lldb)
releasedbg-gdb   : $(if $(ISLIN),linux-releasedbg-gdb)

#------------------------------------------------------------------------------------------------------

ZLPARAMS_MAKE  = $(strip $(strip $(if $(D),"D=$(D)")$(if $(W),$(foreach WW,$(W), -W "$(WW)")))$(if $(filter B,$(MAKEFLAGS)), -B))
ZLPARAMS_XCODE = $(if $(D),"CmdLinePreprocessorDefinitions=$(D)")

#------------------------------------------------------------------------------------------------------
.PHONY: linux linux-clean linux-run linux-gdb linux-debug linux-release linux-releasedbg linux-debug-clean linux-release-clean linux-releasedbg-clean linux-debug-run linux-release-run linux-releasedbg-run linux-debug-gdb linux-release-gdb linux-releasedbg-gdb
linux: linux-debug
linux-clean: linux-debug-clean
linux-run: linux-debug-run
linux-gdb: linux-debug-gdb
linux-release linux-release-clean linux-release-run linux-release-gdb: ZLLINUX_BUILD = BUILD=RELEASE
linux-releasedbg linux-releasedbg-clean linux-releasedbg-run linux-releasedbg-gdb: ZLLINUX_BUILD = BUILD=RELEASEDBG
ZLLINUX_CMD = @+"$(MAKE)" --no-print-directory -f "$(ZILLALIB_PATH)/Linux/ZillaLibLinux.mk" $(ZLLINUX_BUILD) "ZillaApp=$(ZillaApp)"
linux-debug linux-release linux-releasedbg:; $(ZLLINUX_CMD) $(ZLPARAMS_MAKE)
linux-debug-clean linux-release-clean linux-releasedbg-clean:; $(ZLLINUX_CMD) clean
linux-debug-run linux-release-run linux-releasedbg-run:; $(ZLLINUX_CMD) run
linux-debug-gdb linux-release-gdb linux-releasedbg-gdb:; $(ZLLINUX_CMD) gdb

#------------------------------------------------------------------------------------------------------

.PHONY: wasm wasm-clean wasm-run wasm-debug wasm-release wasm-debug-clean wasm-release-clean wasm-debug-run wasm-release-run
wasm: wasm-release
wasm-clean: wasm-release-clean
wasm-run: wasm-release-run
wasm-release wasm-release-clean wasm-release-run: ZLWASM_PARAMS = BUILD=RELEASE
ZLWASM_CMD = @+"$(MAKE)" --no-print-directory -f "$(ZILLALIB_PATH)/WebAssembly/ZillaLibWasm.mk" $(ZLWASM_PARAMS) "ZillaApp=$(ZillaApp)"
wasm-debug wasm-release:; $(ZLWASM_CMD) $(ZLPARAMS_MAKE)
wasm-debug-clean wasm-release-clean:; $(ZLWASM_CMD) clean
wasm-debug-run wasm-release-run:; $(ZLWASM_CMD) $(ZLPARAMS_MAKE) run

#------------------------------------------------------------------------------------------------------

.PHONY: emscripten emscripten-clean emscripten-run emscripten-debug emscripten-release emscripten-debug-clean emscripten-release-clean emscripten-debug-run emscripten-release-run
emscripten: emscripten-release
emscripten-clean: emscripten-release-clean
emscripten-run: emscripten-release-run
emscripten-release emscripten-release-clean emscripten-release-run: ZLEMSCRIPTEN_PARAMS = BUILD=RELEASE
ZLEMSCRIPTEN_CMD = @+"$(MAKE)" --no-print-directory -f "$(ZILLALIB_PATH)/Emscripten/ZillaLibEmscripten.mk" $(ZLEMSCRIPTEN_PARAMS) "ZillaApp=$(ZillaApp)"
emscripten-debug emscripten-release:; $(ZLEMSCRIPTEN_CMD) $(ZLPARAMS_MAKE)
emscripten-debug-clean emscripten-release-clean:; $(ZLEMSCRIPTEN_CMD) clean
emscripten-debug-run emscripten-release-run:; $(ZLEMSCRIPTEN_CMD) $(ZLPARAMS_MAKE) run

#------------------------------------------------------------------------------------------------------

.PHONY: nacl nacl-clean nacl-run nacl-debug nacl-release nacl-debug-clean nacl-release-clean nacl-debug-run nacl-release-run
nacl: nacl-release
nacl-clean: nacl-release-clean
nacl-run: nacl-release-run
nacl-release nacl-release-clean nacl-release-run: ZLNACL_PARAMS = BUILD=RELEASE
ZLNACL_CMD = @+"$(MAKE)" --no-print-directory -f "$(ZILLALIB_PATH)/NACL/ZillaLibNACL.mk" $(ZLNACL_PARAMS) "ZillaApp=$(ZillaApp)"
nacl-debug nacl-release:; $(ZLNACL_CMD) $(ZLPARAMS_MAKE)
nacl-debug-clean nacl-release-clean:; $(ZLNACL_CMD) clean
nacl-debug-run nacl-release-run:; $(ZLNACL_CMD) $(ZLPARAMS_MAKE) run

#------------------------------------------------------------------------------------------------------

.PHONY: android android-clean android-install android-uninstall android-sign android-debug android-release android-debug-clean android-release-clean android-debug-install android-release-install
android: android-release
android-clean: android-release-clean
android-install: android-release-install
android-debug android-debug-clean android-debug-install: ZLANDROID_DBG=1
android-release android-release-clean android-release-install android-sign: ZLANDROID_DBG=0
ZLANDROID_CMD = @+"$(MAKE)" --no-print-directory -f "$(ZILLALIB_PATH)/Android/ZillaLibAndroid.mk" ZLDEBUG=$(ZLANDROID_DBG) "ZillaApp=$(ZillaApp)"
android-debug android-release:; $(ZLANDROID_CMD) $(ZLPARAMS_MAKE)
android-debug-clean android-release-clean:; $(ZLANDROID_CMD) clean
android-debug-install android-release-install:; $(ZLANDROID_CMD) $(ZLPARAMS_MAKE) install
android-uninstall:; $(ZLANDROID_CMD) uninstall
android-sign:; $(ZLANDROID_CMD) sign $(if $(SIGN_OUTAPK),"SIGN_OUTAPK=$(SIGN_OUTAPK)") $(if $(SIGN_KEYSTORE),"SIGN_KEYSTORE=$(SIGN_KEYSTORE)") $(if $(SIGN_STOREPASS),"SIGN_STOREPASS=$(SIGN_STOREPASS)") $(if $(SIGN_KEYALIAS),"SIGN_KEYALIAS=$(SIGN_KEYALIAS)") $(if $(SIGN_KEYPASS),"SIGN_KEYPASS=$(SIGN_KEYPASS)")

#------------------------------------------------------------------------------------------------------

.PHONY: osx osx-clean osx-cleanall osx-run osx-lldb osx-debug osx-release osx-debug-clean osx-release-clean osx-debug-cleanall osx-release-cleanall osx-debug-run osx-release-run osx-debug-lldb osx-release-lldb
osx: osx-debug
osx-clean: osx-debug-clean
osx-cleanall: osx-debug-cleanall
osx-run: osx-debug-run
osx-lldb: osx-debug-lldb
osx-debug osx-debug-clean osx-debug-cleanall osx-debug-run osx-lldb: ZLOSX_CONFIG = Debug
osx-release osx-release-clean osx-release-cleanall osx-release-run: ZLOSX_CONFIG = Release
ZLOSX_CMD = @xcodebuild -configuration $(ZLOSX_CONFIG) -target "$(ZillaApp)" -project "$(ZillaApp)-OSX.xcodeproj" $(ZLPARAMS_XCODE)
osx-debug osx-release:; $(ZLOSX_CMD)
osx-debug-clean osx-release-clean:;rm -rf $(ZillaApp)-OSX.xcodeproj/$(ZLOSX_CONFIG)
osx-debug-cleanall osx-release-cleanall:; $(ZLOSX_CMD) clean
ZLOSX_RUN = "$(ZillaApp)-OSX.xcodeproj/$(ZLOSX_CONFIG)/$(ZillaApp).app/Contents/MacOS/$(ZillaApp)"
osx-debug-run:osx-debug; $(ZLOSX_RUN)
osx-release-run:osx-release; $(ZLOSX_RUN)
osx-debug-lldb:osx-debug;lldb $(ZLOSX_RUN)
osx-release-lldb:osx-release;lldb $(ZLOSX_RUN)

#------------------------------------------------------------------------------------------------------

.PHONY: ios ios-clean ios-cleanall ios-run ios-simulator ios-phone ios-simulator-run ios-archive ios-scheme
ios: ios-simulator
ios-run: ios-simulator-run
ios-simulator ios-simulator-run ios-phone: ZLIOS_CONFIG = Debug
ios-archive: ZLIOS_CONFIG = Release
ios-simulator ios-cleanall ios-simulator-run:ZLIOS_SNAME = $(if $(IOSSIMULATOR),$(IOSSIMULATOR),iPhone 5s)
ios-simulator ios-cleanall ios-simulator-run:ZLIOS_DEST = -destination 'platform=iOS Simulator,name=$(ZLIOS_SNAME)'
ZLIOS_CMD = @xcodebuild -project $(ZillaApp)-iOS.xcodeproj -scheme $(ZillaApp) $(ZLIOS_DEST) $(ZLPARAMS_XCODE) -configuration $(ZLIOS_CONFIG)
ios-simulator ios-phone:ios-scheme; $(ZLIOS_CMD)
ios-cleanall:ios-scheme; $(ZLIOS_CMD) Debug clean; $(ZLIOS_CMD) Release clean
ios-archive:ios-scheme; $(ZLIOS_CMD) archive
ios-clean:; rm -rf $(ZillaApp)-iOS.xcodeproj/Debug
ios-list-simulators:; @xcrun simctl list | grep SimDeviceType
ios-simulator-run:ios-simulator ios-scheme
	@echo "Running $(ZillaApp).app in simulator $(ZLIOS_SNAME) (using running Simulator if already started)"
	@open -a Simulator --args -CurrentDeviceUDID `xcrun simctl list | grep "  $(ZLIOS_SNAME)" | sed -E 's/.*([A-Z0-9-]{36}).*/\1/'`
	@while [ "`xcrun simctl list | grep Booted`" == "" ]; do sleep 1; done
	@xcrun simctl install booted "$(ZillaApp)-iOS.xcodeproj/Debug/$(ZillaApp).app"
	@xcrun simctl launch booted "`/usr/libexec/PlistBuddy -c 'Print CFBundleIdentifier' $(ZillaApp)-iOS.xcodeproj/Debug/$(ZillaApp).app/Info.plist`"
ios-scheme:ZLIOS_SCHEME = $(ZillaApp)-iOS.xcodeproj/xcshareddata/xcschemes/$(ZillaApp).xcscheme
ios-scheme:;@$(if $(or $(wildcard $(ZLIOS_SCHEME)),$(wildcard $(ZillaApp)-iOS.xcodeproj/xcuserdata/*/xcschemes/*.xcscheme)),,\
	mkdir -p $(dir $(ZLIOS_SCHEME));S=Scheme;A=BuildAction;B=Buildable;E=BuildActionEntries;F=BuildActionEntry;Z=$(ZillaApp);\
	echo "<$$S><$$A parallelize$${B}s='YES' buildImplicitDependencies='NO'><$$E><$$F>"\
	"<$${B}Reference $${B}Identifier='primary' BlueprintIdentifier='F01000' ReferencedContainer='container:$$Z-iOS.xcodeproj' $${B}Name='$$Z.app' BlueprintName='$$Z'/>"\
	"</$$F></$$E></$$A></$$S>">$(ZLIOS_SCHEME))

#------------------------------------------------------------------------------------------------------
else
#------------------------------------------------------------------------------------------------------

.PHONY: linux linux-clean linux-debug linux-release linux-releasedbg linux-debug-clean linux-release-clean linux-releasedbg-clean
linux: linux-debug
linux-clean: linux-debug-clean
linux-release linux-release-clean: ZLLINUX_BUILD = BUILD=RELEASE
linux-releasedbg linux-releasedbg-clean: ZLLINUX_BUILD = BUILD=RELEASEDBG
ZLLINUX_CMD = @+"$(MAKE)" --no-print-directory -f "$(ZILLALIB_PATH)/Linux/ZillaLibLinux.mk" $(ZLLINUX_BUILD)
linux-debug linux-release linux-releasedbg:; $(ZLLINUX_CMD) $(ZLPARAMS_MAKE)
linux-debug-clean linux-release-clean linux-releasedbg-clean:; $(ZLLINUX_CMD) clean

#------------------------------------------------------------------------------------------------------

.PHONY: wasm wasm-clean wasm-debug wasm-release wasm-debug-clean wasm-release-clean
wasm: wasm-release
wasm-clean: wasm-release-clean
wasm-release wasm-release-clean: ZLWASM_PARAMS = BUILD=RELEASE
ZLWASM_CMD = @+"$(MAKE)" --no-print-directory -f "$(ZILLALIB_PATH)/WebAssembly/ZillaLibWasm.mk" $(ZLWASM_PARAMS)
wasm-debug wasm-release:; $(ZLWASM_CMD) $(ZLPARAMS_MAKE)
wasm-debug-clean wasm-release-clean:; $(ZLWASM_CMD) clean

#------------------------------------------------------------------------------------------------------

.PHONY: emscripten emscripten-clean emscripten-debug emscripten-release emscripten-debug-clean emscripten-release-clean
emscripten: emscripten-release
emscripten-clean: emscripten-release-clean
emscripten-release emscripten-release-clean: ZLEMSCRIPTEN_PARAMS = BUILD=RELEASE
ZLEMSCRIPTEN_CMD = @+"$(MAKE)" --no-print-directory -f "$(ZILLALIB_PATH)/Emscripten/ZillaLibEmscripten.mk" $(ZLEMSCRIPTEN_PARAMS)
emscripten-debug emscripten-release:; $(ZLEMSCRIPTEN_CMD) $(ZLPARAMS_MAKE)
emscripten-debug-clean emscripten-release-clean:; $(ZLEMSCRIPTEN_CMD) clean

#------------------------------------------------------------------------------------------------------

.PHONY: nacl nacl-clean nacl-debug nacl-release nacl-debug-clean nacl-release-clean
nacl: nacl-release
nacl-clean: nacl-release-clean
nacl-release nacl-release-clean: ZLNACL_PARAMS = BUILD=RELEASE
ZLNACL_CMD = @+"$(MAKE)" --no-print-directory -f "$(ZILLALIB_PATH)/NACL/ZillaLibNACL.mk" $(ZLNACL_PARAMS)
nacl-debug nacl-release:; $(ZLNACL_CMD) $(ZLPARAMS_MAKE)
nacl-debug-clean nacl-release-clean:; $(ZLNACL_CMD) clean

#------------------------------------------------------------------------------------------------------

.PHONY: android android-clean android-debug android-release android-debug-clean android-release-clean
android: android-release
android-clean: android-release-clean
android-debug android-debug-clean: ZLANDROID_DBG=1
android-release android-release-clean: ZLANDROID_DBG=0
ZLANDROID_CMD = @+"$(MAKE)" --no-print-directory -f "$(ZILLALIB_PATH)/Android/ZillaLibAndroid.mk" ZLDEBUG=$(ZLANDROID_DBG)
android-debug android-release:; $(ZLANDROID_CMD)
android-debug-clean android-release-clean:; $(ZLANDROID_CMD) clean

#------------------------------------------------------------------------------------------------------

.PHONY: osx osx-clean osx-debug osx-release osx-debug-clean osx-release-clean
osx: osx-debug
osx-clean: osx-debug-clean
osx-debug osx-debug-clean: ZLOSX_CONFIG = Debug
osx-release osx-release-clean: ZLOSX_CONFIG = Release
ZLOSX_CMD = @xcodebuild -configuration $(ZLOSX_CONFIG) -target "ZillaLib" -project "ZillaLib-OSX.xcodeproj"
osx-debug osx-release:; $(ZLOSX_CMD)
osx-debug-clean osx-release-clean:; $(ZLOSX_CMD) clean

#------------------------------------------------------------------------------------------------------

.PHONY: ios ios-clean ios-simulator ios-phone ios-scheme
ios: ios-simulator
ios-simulator ios-phone: ZLIOS_CONFIG = Debug
ios-simulator ios-clean:ZLIOS_SNAME = $(if $(IOSSIMULATOR),$(IOSSIMULATOR),iPhone 5s)
ios-simulator ios-clean:ZLIOS_DEST = -destination 'platform=iOS Simulator,name=$(ZLIOS_SNAME)'
ZLIOS_CMD = @xcodebuild -project ZillaLib-iOS.xcodeproj -scheme ZillaLib $(ZLIOS_DEST) -configuration $(ZLIOS_CONFIG)
ios-simulator ios-phone:ios-scheme; $(ZLIOS_CMD)
ios-clean:ios-scheme; $(ZLIOS_CMD) Debug clean; $(ZLIOS_CMD) Release clean
ios-scheme:ZLIOS_SCHEME = ZillaLib-iOS.xcodeproj/xcshareddata/xcschemes/ZillaLib.xcscheme
ios-scheme:;$(if $(or $(wildcard $(ZLIOS_SCHEME)),$(wildcard ZillaLib-iOS.xcodeproj/xcuserdata/*/xcschemes/*.xcscheme)),,\
	mkdir -p $(dir $(ZLIOS_SCHEME));S=Scheme;A=BuildAction;B=Buildable;E=BuildActionEntries;F=BuildActionEntry;Z=ZillaLib;\
	echo "<$$S><$$A parallelize$${B}s='YES' buildImplicitDependencies='NO'><$$E><$$F>"\
	"<$${B}Reference $${B}Identifier='primary' BlueprintIdentifier='2177A2177A2177A2177A0022' ReferencedContainer='container:$$Z-iOS.xcodeproj' $${B}Name='$$Z.a' BlueprintName='$$Z'/>"\
	"</$$F></$$E></$$A></$$S>">$(ZLIOS_SCHEME))

#------------------------------------------------------------------------------------------------------
endif
#------------------------------------------------------------------------------------------------------

endif
