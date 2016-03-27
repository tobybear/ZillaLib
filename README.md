ZillaLib Game SDK
=================

  * [Welcome to ZillaLib](#welcome-to-zillalib)
  * [Games using ZillaLib](#games-using-zillalib)
  * [Sample Code](#sample-code)
  * [Tutorials](#tutorials)
  * [ZillaLib Features](#zillalib-features)
  * [License](#license)
  * [Setting Up a Project](#setting-up-a-project)
  * [Platforms and Setup](#platforms-and-setup)
    * [Windows (32 and 64 bit)](#windows-32-and-64-bit)
    * [Android](#android)
    * [iOS (iPhone/iPad/iPod)](#ios-iphoneipadipod)
    * [Chrome Portable Native Client](#chrome-portable-native-client)
    * [Emscripten (HTML5)](#emscripten-html5)
    * [Linux](#linux)
    * [Max OS X](#max-os-x)
  * [Asset File Handling](#asset-file-handling)

## Welcome to ZillaLib

ZillaLib is a 2D game creation framework that runs on pretty much every open platform out there. Windows, Linux, Mac OS X, Android, iOS, Chrome Native Client and Emscripten (HTML5).

It's a sleek C++ library that compiles to all platforms with zero change in the game code. Truly write once, run everywhere. Of course it is still possible to deviate parts easily where needed. For instance for multitouch or touchscreen keyboard inputs.

The library itself compiles quickly, has zero external dependencies, and links statically adding only around 250kb to the binary size of the game. Still it comes with plenty of features.

## Games using ZillaLib

See [Games.md](Games.md) for a list. These can be played directly in the web browser or downloaded, and come with full source code under the same license as ZillaLib.

## Sample Code

See the [ZillaLibSamples](//github.com/schellingb/ZillaLibSamples) repository for short and easy to understand sample code for almost all features.

## Tutorials

See the [tutorials listing](https://zillalib.github.io/tutorials/) on the ZillaLib website.

## ZillaLib Features
Here's a list of features provided by the ZillaLib.

### Code
- Game code implementation is 100% C++
- All platforms get compiled to natively, linked statically
- No touching of Java, Objective-C, Javascript, C# or anything required
- Exact same code that debugs on PC runs everywhere else
- Easy to set up, no dependencies

### Platforms
- [Windows (32 and 64 bit)](#windows-32-and-64-bit)
- [Android](#android)
- [iOS (iPhone/iPad/iPod)](#ios-iphoneipadipod)
- [Chrome Portable Native Client](#chrome-portable-native-client)
- [Emscripten (HTML5)](#emscripten-html5)
- [Linux](#linux)
- [Max OS X](#max-os-x)

### Rendering
- Drawing of simple 2D geometry (lines, circles, polygons, etc)
- Surface texture loading (PNG/JPEG)
- Drawing of surfaces scaled/rotated
- Buffered batch rendering
- Repeated texture rendering
- Tiled textures for tile maps or animations
- Font rendering
- Bitmap fonts and TTF fonts
- Text alignment vertical/horizontal/baseline, automatic line breaks
- Particle system
- Custom vertex/fragment shader support

### Audio
- Audio sample loading and playing (OGG)
- Music streaming and looping
- Hooks for custom processing
- Built in chip tune like synthesizer
- Samples with arbitrary playback speed

### Animation
- Tiled textures for animation
- Particle system
- Transition manager for timer/event based animation
- Bone based physics for 2D ragdoll animations

### Math
- 2D Math library (Vectors, Rects, Rotating Bounding Boxes, etc.)
- Collision test functions for common interactions (Circle/Line/Boxes tests and sweeps)
- Easing functions
- Random Number Generators

### Networking
- UDP based server/client system (based on ENET)
- Super light weight NAT punch (easily P2P connect, even from mobile to PC)
- HTTP requests for all platforms including POST data
- HTTP responses can be loaded as regular files (including textures/audio)

### Game
- Scene manager for splitting game parts
- Built in transition system for animating scene switch
- Event system for input, resizing
- Multitouch input support where available
- Mobile device motion sensors and joystick support

### System
- Key/Value configuration storage for all platforms
- Thread support
- Open URL in default system browser
- Base64 encoding/decoding

## License

ZillaLib is available under the [zlib license](http://www.gzip.org/zlib/zlib_license.html).

## Setting Up a Project

First download this project from GitHub with the 'Download ZIP' button or by cloning the repository.

The library itself compiles automatically with the first project being built, so lets start right away!

By far the easiest way to get started is to use the interactive [ZillaLib Project Generator](https://zillalib.github.io/project-generator/).
Just enter a project name and select the target project files to generate.  
If you have placed ZillaLib in a directory named dfferent than "ZillaLib" or if you want to place your game project in a directory that is not next to ZillaLib's, you have to specify the relative path in the field 'Path to ZillaLib'. For instance, if you have ZillaLib in "D:\dev\libs\ZillaLib-master" and you plan to put your game project in "D:\dev\proj\MySuperGame", then specify "../../libs/ZillaLib-master" as the path to ZillaLib. Always specify the relative path. It makes it easy to build various platforms and even from different virtual machines in the same place.

When finished, click "Create and Download Project Archive (ZIP)" and everything will automagically be prepared for you. Extract the project files in the planned location. There's even some sample code included to start up right away. Check the specific platform from the [list below](#platforms-and-setup) you want to build for on how to set it up and how to actually get your project up and running.

Alternatively you could copy the sample project and modify the paths and names in it. But that would be quite a hassle. An offline project generator might be added some day.

## Platforms and Setup

### Windows (32 and 64 bit)
Compiling and debugging is done with Visual Studio. All versions are supported (VC6 until VS2015). If you don't have it installed yet, get either [2013 Community Edition](https://go.microsoft.com/fwlink/?LinkId=532495&clcid=0x409) or [2015 Community Edition](https://go.microsoft.com/fwlink/?LinkId=691978&clcid=0x409). They're both free and fully featured. For 2013, none of the optional features are required. For 2015, we need a customized installation containing both "Common Tools for Visual C++ 2015" and "Windows XP Support for C++" under the "Programming Languages\Visual C++" features.

Once installed, open your GameProject-vs.sln solution file. On your first start-up, it should have your game project selected as the start up project in the Solution Explorer (project name is bold). It should also default to the build configuration "Debug" and the build platform "Win32" on the top of the Visual Studio window. This is fine for now, you can just build and run the sample code with the menu function "Debug / Start Without Debugging".

### Android
See [README.md under Android](Android) for how to setup and build for Android.

### iOS (iPhone/iPad/iPod)
See [README.md under ZillaLib-iOS.xcodeproj](ZillaLib-iOS.xcodeproj) for how to setup and build for iOS.

### Chrome Portable Native Client
See [README.md under NACL](NACL) for how to setup and build for Native Client.

### Emscripten (HTML5)
See [README.md under Emscripten](Emscripten) for how to setup and build for Emscripten.

### Linux
See [README.md under Linux](Linux) for how to setup and build for Linux.

### Max OS X
See [README.md under ZillaLib-OSX.xcodeproj](ZillaLib-OSX.xcodeproj) for how to setup and build for Mac OS X.

## Asset File Handling

ZillaLib has various modes of packaging asset files with the game depending mainly on the target platform, but also on build configuration (debug or release) and on the project settings.

### How files are loaded

Files are loaded through the ZL_File class. ZL_File can reference actual files on the file system, data in memory, or files inside containers. Containers themselves are a ZL_File, too. When loading files, paths are given relative from the game project directory and are written with forward slashes.

There is a global file container ZL_File::DefaultReadFileContainer which will be used automatically for reading files. Depending on the platform and build configuration, this default container will be automatically set on startup. Generally similar platform types have similar asset handling.

### Desktop (Windows, Linux, Mac OS X)

Debug builds always load asset files directly. It is recommended to call the LoadReleaseDesktopDataBundle method at the beginning of Load of your ZL_Application subclass. LoadReleaseDesktopDataBundle takes an optional file name as parameter. It should name a zip-file (does not need a .zip ending) which will be used as the default read file container in release builds.  
Alternatively, if your project was set-up with the 'Embed Assets in Binary' option, the optional parameter can be left empty. In that case, the executable binary of the game itself will be loaded as default container. A project with that option enabled will automatically package all assets during the release build process into the output binary just for that.  
LoadReleaseDesktopDataBundle does nothing on debug builds as well as non-desktop builds, so it does not need a platform check with if or #if.

### Mobile (Android, iOS)

Mobile app packages have a fixed way of packaging and loading files. Therefore all build configurations behave the same and no separate actions are required. On Android, the applications APK package is being loaded as the default read file container on startup. On iOS applications are installed and extracted by the operating system so files can be loaded with no file container.

### Web (NACL, Emscripten)

The build process for web targets packages all assets into a single file for distribution. A ZIP file named GAME_Files.dat on NACL and a GAME_Files.js named file for Emscripten.  
If your project was set-up with the 'Embed Assets in Binary' option, the asset file will be merged into the main game executable file so distribution becomes a single file.  
Because there is no traditional file access on the web, the entire asset file will be loaded into memory on startup and ZL_File instances then read from that memory.
