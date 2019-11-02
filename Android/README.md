ZillaLib on Android
===================

  * [Setup](#setup)
    * [Get a Java Development Kit (JDK)](#get-a-java-development-kit-jdk)
    * [Get the Android SDK](#get-the-android-sdk)
    * [Install the actual Platform SDK](#install-the-actual-platform-sdk)
    * [Slim down the SDK](#slim-down-the-sdk)
    * [Get the Android NDK](#get-the-android-ndk)
    * [Slim down the NDK](#slim-down-the-ndk)
    * [Setup a Debug Keystore](#setup-a-debug-keystore)
    * [Setup building ZillaLib](#setup-building-zillalib)
  * [Building](#building)
    * [Building from Visual Studio on Windows](#building-from-visual-studio-on-windows)
    * [Building anywhere else](#building-anywhere-else)
    * [Build ABIs](#build-abis)
    * [Manifest Version Codes](#manifest-version-codes)
    * [Signing a build for release](#signing-a-build-for-release)
  * [Notes on developing for Android](#notes-on-developing-for-android)

# Setup

## Get a Java Development Kit (JDK)
On Windows, I suggest getting a JDK that does not integrate into the OS, annoying you with adware like popups and unwanted browser plugins. You can get that by extracting 'jdk-8u***-windows-x64.exe' (or jdk-8u***-windows-i586.exe if you're running a 32-bit OS) from the link below with 7-Zip:  
http://www.oracle.com/technetwork/java/javase/downloads/jdk8-downloads-2133151.html
  1. Download jdk-8u***-windows-****.exe and open it with 7-Zip, not by running it
  2. Browse with 7-Zip to /.rsrc/1033/JAVA_CAB10/111/tools.zip
  3. Extract bin and lib from tools.zip/jre/ into a directory named JDK81
  4. Extract bin and lib from tools.zip/ into the same place (this will overwrite some stuff from step 1 from jre) (lib/missioncontrol and lib/visualvm can be skipped or deleted afterwards)
  5. Open a command line to the extracted files and run the following command  
     `for /r %x in (*.pack) do .\bin\unpack200 -r "%x" "%~dx%~px%~nx.jar"`

## Get the Android SDK
Download and extract the SDK Tools for your platform from the link below:  
https://developer.android.com/studio#command-tools  
Extract it into a place where you have your development stuff like D:\dev\android\android-sdk  

### Install the actual Platform SDK
Open a command line to where you extracted the SDK. If you have the JDK system wide installed, you can skip step 1.
  1. Set the JAVA_HOME environment variable with the command, change the path to where you put the JDK, no double quotes needed  
     [on Windows] `set JAVA_HOME=D:\dev\Java\JDK81`  
     [on Linux/OSX] `export JAVA_HOME=~/JDK81`
  2. Verify you can launch the Android SDK Manager  
     [on Windows] `tools\bin\sdkmanager.bat --list`  
     [on Linux/OSX] `tools/bin/sdkmanager --list`
  3. After available packages were listed, install the ones we need woth  
     [on Windows] `tools\bin\sdkmanager.bat "build-tools;29.0.2" "platform-tools" "platforms;android-29"`  
     [on Linux/OSX] `tools/bin/sdkmanager "build-tools;29.0.2" "platform-tools" "platforms;android-29"`

### Slim down the SDK
If you're only looking to develop with ZillaLib for Android, feel free to remove stuff from the SDK we don't need anyway.
Removing these reduces an SDK installation from 408MB (15k files) to 111MB (200 files).
  - In /build-tools/29.0.2 remove: `libclang_android.dll`, `libLLVM_android.dll`, `renderscript`
  - In /platforms/android-29 remove: `data`, `optional`, `skins`, `templates`
  - In /platform-tools remove: `systrace`
  - In /tools/lib remove: `x86`, `x86_64`, `monitor-x86`, `monitor-x86_64`, `jython-standalone-2.5.3.jar`, `intellij-core-26.0.0-dev.jar`

## Get the Android NDK
Next, we need the Android NDK (Native Development Kit). Get it from the link below:  
http://developer.android.com/ndk/downloads/index.html#download  
Extract it according to the instructions on the site, for example next to the SDK into D:\dev\android\android-ndk

### Slim down the NDK
The Android NDK has gotten ginormous and extremely bloated over the years. If you're only looking to develop with ZillaLib for Android, feel free to remove stuff we don't need anyway.
Removing these reduces an NDK installation from 2717MB (16k files) to 745MB (2k files).
This was tested with NDK r20 and might not work the same way in future releases.
  - In the root directory remove:
    - /meta
    - /python-packages
    - /platforms
    - /shader-tools
    - /simpleperf
    - /sysroot
    - /wrap.sh
  - Now create these directories (empty with nothing inside):
    - /platforms/android-16/arch-arm
    - /platforms/android-16/arch-x86
    - /platforms/android-21/arch-arm64
    - /platforms/android-21/arch-x86_64
  - These sub-directories can be removed as well:
    - /prebuilt/windows-x86_64/include
    - /prebuilt/windows-x86_64/lib/python2.7/lib-tk
    - /prebuilt/windows-x86_64/lib/python2.7/lib2to3
    - /prebuilt/windows-x86_64/lib/python2.7/logging
    - /prebuilt/windows-x86_64/lib/python2.7/multiprocessing
    - /prebuilt/windows-x86_64/lib/python2.7/unittest
    - /prebuilt/windows-x86_64/lib/python2.7/wsgiref
    - /prebuilt/windows-x86_64/share
    - /sources/android/cpufeatures
    - /sources/android/native_app_glue
    - /sources/android/ndk_helper
    - /sources/android/renderscript
    - /sources/cxx-stl/llvm-libc++/test
    - /sources/cxx-stl/llvm-libc++/utils
    - /sources/cxx-stl/llvm-libc++abi/cmake
    - /sources/cxx-stl/llvm-libc++abi/lib
    - /sources/cxx-stl/llvm-libc++abi/src
    - /sources/cxx-stl/llvm-libc++abi/test
    - /sources/cxx-stl/llvm-libc++abi/www
    - /sources/cxx-stl/system
    - /sources/third_party
    - /toolchains/aarch64-linux-android-4.9
    - /toolchains/arm-linux-androideabi-4.9
    - /toolchains/x86-4.9
    - /toolchains/x86_64-4.9
    - /toolchains/renderscript
    - /toolchains/llvm/prebuilt/windows-x86_64/aarch64-linux-android/lib
    - /toolchains/llvm/prebuilt/windows-x86_64/arm-linux-androideabi/lib/ldscripts
    - /toolchains/llvm/prebuilt/windows-x86_64/arm-linux-androideabi/lib/thumb
    - /toolchains/llvm/prebuilt/windows-x86_64/i686-linux-android/lib/ldscripts
    - /toolchains/llvm/prebuilt/windows-x86_64/x86_64-linux-android/lib
    - /toolchains/llvm/prebuilt/windows-x86_64/include
    - /toolchains/llvm/prebuilt/windows-x86_64/lib/bfd-plugins
    - /toolchains/llvm/prebuilt/windows-x86_64/lib64/clang/8.0.7/bin
    - /toolchains/llvm/prebuilt/windows-x86_64/lib64/clang/8.0.7/lib
    - /toolchains/llvm/prebuilt/windows-x86_64/lib64/clang/8.0.7/share
    - /toolchains/llvm/prebuilt/windows-x86_64/lib64/cmake
    - /toolchains/llvm/prebuilt/windows-x86_64/libexec
    - /toolchains/llvm/prebuilt/windows-x86_64/share
    - /toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/include/c++
    - /toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/lib/aarch64-linux-android/* (everything BUT KEEP 21)
    - /toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/lib/arm-linux-androideabi/* (everything BUT KEEP 16, libandroid_support.a, libunwind.a)
    - /toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/lib/i686-linux-android/* (everything BUT KEEP 16, libandroid_support.a)
    - /toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/lib/x86_64-linux-android/* (everything BUT KEEP 21)

## Setup a Debug Keystore
As Android application packages are always encrypted, you need a dummy keystore even for debug builds. Luckily it's super easy to create one. Just run the following commands in the command line/terminal. Replace the JAVA_HOME part with the directory to the JDK if you have a JDK that is not installed system wide (meaning without a global JAVA_HOME environment variable set).
- On Windows:
```sh
mkdir "%USERPROFILE%/.android"
"%JAVA_HOME%/bin/keytool" -genkey -v -keystore "%USERPROFILE%/.android/debug.keystore" -alias androiddebugkey -storepass android -keypass android -keyalg RSA -validity 14000
```
- Other:
```sh
md ~/.android
"$JAVA_HOME/bin/keytool" -genkey -v -keystore ~/.android/debug.keystore -alias androiddebugkey -storepass android -keypass android -keyalg RSA -validity 14000
```
You can answer all the identification fields with the default (just press enter) because it's not relevant for the debug keystore. Make sure to write "yes" when being asked if the input is correct.

## Setup building ZillaLib
Create a file inside your local ZillaLib directory under the Android sub-directory called "ZillaAppLocalConfig.mk" with the following two definitions:
```mk
ANDROID_SDK = d:/dev/android/android-sdk-windows
ANDROID_NDK = d:/dev/android/android-ndk
```
The setting "ANDROID_SDK" points to the location of the SDK and "ANDROID_NDK" is the location of the NDK. The paths are with forward slashes, not back slashes, and are ending without a slash.

If from step 1 you have a JDK that is not installed system wide (meaning without a global JAVA_HOME environment variable set), you also need to add the following line containing the path to the JDK root:
```mk
JAVA_HOME = d:/dev/Java/JDK81
```

Optionally you can specify the compile optimization level that will be used for release builds.  
Possible values are (see [clang documentation](https://clang.llvm.org/docs/CommandGuide/clang.html#code-generation-options) for more):
- s = optimize size (default)
- z = smaller size, lower performance
- 2 = larger size, better performance
```mk
RELEASE_OPTIMIZATION = z
```

# Building

## Building from Visual Studio on Windows
Select "Android-Debug" (faster build, bigger output, outputs logging data, can reveal debugging information on a crash) or "Android-Release" (optimized build) from the build configuration selection drop down menu at the top. The platform selection (Win32/x64) in Visual Studio is irrelevant for Android builds. How to set the actual build target platforms is explained under [Build ABIs](#build-abis).

Now you can press "Build Solution" under the "Build" menu and (if everything is setup correctly) get an APK created under your project directory under Android/bin/.  
You can also select "Start Without Debugging" under the "Debug" menu to directly install and run the game on an Android device. Make sure your Android device is connected via [ADB (Android Debug Bridge)](http://developer.android.com/tools/help/adb.html) (either over USB or Wi-Fi).

## Building anywhere else
Open a command line/terminal and change directory to your game project, then run the following command for further instructions:
```sh
make android-help
```
On Windows you can use the make binary that is distributed with ZillaLib, you can find it under the "Tools" sub-directory.

With the various make targets you can build debug and release APK files, install and uninstall it from a connected Android device, or signing the output APK for releasing onto an app store.  
For example, running "make android" builds the release APK file. Running "make android-install" builds, installs and runs the game on an Android device. Make sure your Android device is connected via [ADB (Android Debug Bridge)](http://developer.android.com/tools/help/adb.html) (either over USB or Wi-Fi).

## Build ABIs
The target ABIs to build for are defined in the file /GameProject/Android/jni/Application.mk.

```mk
# This selects build targets for release builds
# Debug builds only build the first listed target
# Choose from armeabi-v7a arm64-v8a x86 x86_64 
APP_ABI := armeabi-v7a arm64-v8a
```

The defaults set by the [project generator](https://zillalib.github.io/project-generator/) are the two ARM variations `armeabi-v7a` and `arm64-v8a`. Additionally you can enable `x86` and `x86_64` to build for the rarer devices with intel based CPUs. X86 builds are also used for the emulator so if you're testing in the emulator make sure to move that ABI first in the list to make use of it in debug builds.

## Manifest Version Codes
The build system automatically generates different version codes for each built ABI APK file.

The last digit of the version code will indicate which ABI it is for (with 64-bit ABIs having a higher number, as required by the Android specs).

The version code set in AndroidManifest.xml will be used as a base, and the ABI indicator will be attached by the build process ("100" becomes "1001", "1002", ...).

## Signing a build for release
First you need a keystore for your publishing release. You can use the same keystore for multiple apps/games or have separate keystores for each of them. You need to make sure to keep the keystore files (and the passwords for them) or otherwise you won't be able to re-publish an app/game (for updates!).  
Creating a keystore is the same procedure as was used to [Setup a Debug Keystore](#setup-a-debug-keystore). This time just use a proper storepass, alias and keypass given in the commandline. And also feel free to give more information regarding your identification.

Then run the make script as explained under [Building anywhere else](#building-anywhere-else). Use the following command:
```sh
make android-sign SIGN_KEYSTORE=<...> SIGN_STOREPASS=<...> SIGN_KEYALIAS=<...> SIGN_KEYPASS=<...> SIGN_OUTAPK=<...>
```

The listed parameters are as follows:
  - `SIGN_KEYSTORE`: Path to the keystore file used to sign the APK (given to keytool during store creation with -keystore)
  - `SIGN_STOREPASS`: Password for the keystore file (given to keytool during store creation with -storepass)
  - `SIGN_KEYALIAS`: Name of the alias as stored in the keystore (given to keytool during store creation with -alias)
  - `SIGN_KEYPASS`: Password of the key (given to keytool during store creation with -keypass)
  - `SIGN_OUTAPK`: Name of the signed output APK file, if omitted, it will be saved into the Android sub-directory of your project

Your completed fully signed releasable APK file will then be generated.

# Notes on developing for Android
The Android back key will be sent as an ESC keyboard press event. Make sure to handle it properly for instance by quitting while on the title screen of your game.

To get events on presses on the volume up/down keys, you can set the flag `ZL_DISPLAY_ANDROID_OVERRIDEVOLUMEKEYS` when calling `ZL_Display::Init`. This prevents a user from changing the volume while in the game, but gives you more hardware buttons to use inside the game.

To always show the navigation bar and giving up some of the available screen space you can set the flag `ZL_DISPLAY_ANDROID_SHOWNAVIGATIONBAR` when calling `ZL_Display::Init`.
Without it the user is required to do a swipe gesture from the edge to make it visible and being able to access the back button.
