ZillaLib on Android
===================

  * [Setup](#setup)
    * [Get a Java Development Kit (JDK)](#get-a-java-development-kit-jdk)
    * [Get the Android SDK](#get-the-android-sdk)
    * [Install the actual Platform SDK](#install-the-actual-platform-sdk)
    * [Get the Android NDK](#get-the-android-ndk)
    * [Setup a Debug Keystore](#setup-a-debug-keystore)
    * [Setup building ZillaLib](#setup-building-zillalib)
  * [Building](#building)
    * [Building from Visual Studio on Windows](#building-from-visual-studio-on-windows)
    * [Building anywhere else](#building-anywhere-else)
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
Download and extract the SDK Tools from the link below:  
http://developer.android.com/sdk/index.html#Other  
Again I suggest getting just the extractable archive (ZIP or TGZ) without installing something system wide. Just put it into a place where you have your development stuff like D:\dev\android\android-sdk-windows

### Install the actual Platform SDK
Open a command line to where you extracted the SDK. If you have the JDK system wide installed, you can skip step 1.
  1. Set the JAVA_HOME environment variable with the command, change the path to where you put the JDK, no double quotes needed  
     [on windows] `set JAVA_HOME=D:\dev\Java\JDK81`  
     [on Linux/OSX] `export JAVA_HOME=~/JDK81`
  2. Run the Android SDK Manager  
     [on Windows] `"SDK Manager.exe"`  
     [on Linux/OSX] `tools/android sdk`
  3. Click "Deselect All"
  4. Check the newest versions of "Tools/Android SDK Tools", "Tools/Android SDK Platform-tools", "Tools/Android SDK Build-tools"
  5. Check "Android 4.0.3 (API 15)/SDK Platform"  
     ZillaLib per default builds against API 15. As it is upwards compatible, no need for a newer (bigger) SDK.
  6. Install the selected packages

## Get the Android NDK
Next, we need the Android NDK (Native Development Kit). Get it from the link below:  
http://developer.android.com/ndk/downloads/index.html#download  
Extract it according to the instructions on the site, for example next to the SDK into D:\dev\android\android-ndk

The Android NDK has gotten ginormous over the years. If you're only looking to develop with ZillaLib for Android, feel free to remove stuff we don't need anyway (or don't extract these directories at all):
  - you can delete "docs"
  - under "platforms", you can delete everything but "platforms/android-9/arch-arm"
  - you can delete "samples"
  - under "sources", you can delete everything but "sources/cxx-stl/system"
  - you can delete "tests"
  - under "toolchains", you can delete everything but "toolchains/arm-linux-androideabi-4.9"

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

# Building

## Building from Visual Studio on Windows
Select "Android-Debug" (faster build, bigger output, outputs logging data, can reveal debugging information on a crash) or "Android-Release" (optimized build) from the build configuration selection drop down menu at the top. The platform selection (Win32/x64) is irrelevant for Android builds.

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

To get events on presses on the volume up/down keys, you can set the flag `ZL_DISPLAY_OVERRIDEANDROIDVOLUMEKEYS` when calling `ZL_Display::Init`. This prevents a user from changing the volume while in the game, but gives you more hardware buttons to use inside the game.
