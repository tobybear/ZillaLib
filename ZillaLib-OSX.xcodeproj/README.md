ZillaLib on Mac OS X
====================

  * [Setup](#setup)
  * [Building](#building)
  * [Building from a terminal](#building-from-a-terminal)

# Setup

You need a running Mac OS X system (on real hardware or in a virtual machine) with the Apple development suite Xcode installed.  
Only the newest Xcode version (requiring the latest OS X version) can be installed from inside the Mac App Store but you can get any old version from the [Apple Developer Download site](https://developer.apple.com/downloads/).  
See [Xcode on Wikipedia](https://en.wikipedia.org/wiki/Xcode#Version_comparison_table) for which version runs on which OS.  

ZillaLib should build with fairly old Xcode versions (3.2) but 5.1.1 or higher is recommended.

Projects for ZillaLib have defaults set to build to run on Mac OS X 10.5 and higher.

# Building

Open your game projects OSX Xcode project file and build it. It will automatically build the library alongside the game on the first time compiling. From within Xcode you can directly play and debug the game.

Make sure to have the correct scheme of your game set before building/running with the scheme selector on the top of the Xcode window.

Its easiest to hide the library scheme from the scheme selection in the scheme manager. To do this click the Product menu, Scheme, and then "Manage Schemes...". In there, disable the Show flag of the ZillaLib scheme. This only needs to be done once per OS X user and is kept across projects.

# Building from a terminal

Open a terminal and change the directory to your game project, then run the following command for further instructions:
```sh
make osx-help
```

With the various make targets you can build debug and release executables, run them normally or with the lldb command line debugger.
