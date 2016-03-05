ZillaLib on iOS
===============

  * [Setup](#setup)
  * [Building](#building)
  * [Building from a terminal](#building-from-a-terminal)
  * [Targeting WatchOS or TVOS](#targeting-watchos-or-tvos)

# Setup

You need a running Mac OS X system (on real hardware or in a virtual machine) with the Apple development suite Xcode installed.  
Only the newest Xcode version (requiring the latest OS X version) can be installed from inside the Mac App Store but you can get any old version from the [Apple Developer Download site](https://developer.apple.com/downloads/).  
See [Xcode on Wikipedia](https://en.wikipedia.org/wiki/Xcode#Version_comparison_table) for which version runs on which OS.  

ZillaLib should build with fairly old Xcode versions (3.2) but 5.1.1 or higher is recommended. The newer the version the easier it is to test latest devices and iOS versions (but it might actually be harder to test older iOS versions). Also the process of submitting a game onto the iOS App Store might be easier with newer Xcode versions.

Projects for ZillaLib have defaults set to build to run on iOS 4.3 and higher.

# Building

Open your game projects iOS Xcode project file and build it. It will automatically build the library alongside the game on the first time compiling. From within Xcode you can directly play and debug the game on the iOS Simulator or on a connected iOS device.

Make sure to have the correct scheme of your game and the intended target set before building/running with the scheme selector on the top of the Xcode window.

Its easiest to hide the library scheme from the scheme selection in the scheme manager. To do this click the Product menu, Scheme, and then "Manage Schemes...". In there, disable the Show flag of the ZillaLib scheme. This only needs to be done once per OS X user and is kept across projects.

# Building from a terminal

Open a terminal and change the directory to your game project, then run the following command for further instructions:
```sh
make ios-help
```

With the various make targets you can build for simulator and device targets, run the simulator with a given device type, and making an app archive for releasing onto the App Store.  
Unfortunately testing and deploying onto an actual connected device is not possible with the Xcode tools. For that you need to open up the project in Xcode itself.

# Targeting WatchOS or TVOS

If you want to deploy on non iPhone/iPad devices (WatchOS, TVOS), you need to enable building with embedded bitcode by removing the following declarations in the "project.pbxproj" files from both your game and ZillaLib itself.

 - Under the release build configuration, remove "-ffunction-sections", "-fdata-sections" from OTHER_CFLAGS
 - Remove the declaration ENABLE_BITCODE = NO

You might also need to change IPHONEOS_DEPLOYMENT_TARGET from 4.3 to 6.0.
