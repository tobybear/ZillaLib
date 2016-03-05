ZillaLib on Emscripten
======================

  * [Setup](#setup)
    * [The official installer](#the-official-installer)
    * [Installing just the required parts](#installing-just-the-required-parts)
      * [On Windows](#on-windows)
      * [On Linux/Mac OS X](#on-linuxmac-os-x)
    * [Building the Emscripten Optimizer](#building-the-emscripten-optimizer)
    * [Setup building ZillaLib](#setup-building-zillalib)
  * [Building](#building)
    * [Building from Visual Studio on Windows](#building-from-visual-studio-on-windows)
    * [Building anywhere else](#building-anywhere-else)
  * [Deploying](#deploying)

# Setup
There are two ways of installing Emscripten.  
Either installing it with the official installer and guide, which will place a bunch of configuration, temporary build and cache files into your system user directory.
Or by installing just the necessary parts required for building ZillaLib (or other projects using a similar independent build script) and by keeping the configuration and caches inside the actual build folder.

## The official installer
This is recommended if you plan on using Emscripten for other things besides ZillaLib related projects.

Follow the install guide from the link below to get emscripten, clang, node, python and set up the required configuration.  
http://kripken.github.io/emscripten-site/docs/getting_started/

When done, you should be set up with Emscripten compiling at least its hello_world.cpp example with -O3 optimizations.

## Installing just the required parts

### On Windows
You can get the Emscripten SDK Offline Installer package from the [Emscripten Downloads page](http://kripken.github.io/emscripten-site/docs/getting_started/downloads.html).  
If the download is slow, you can download it via http instead of https or use a tool that uses multiple connection to speed up downloads.  
Instead of running the installer, just open it directly with 7-Zip.

You only need to extract the folders clang, emscripten, node and optionally Python (if you don't have Python 2.7 somewhere on your system already).

For Node, you can leave out everything but "node.exe".  
For Emscripten, you don't need to extract the directories "tests", "site", "docs", "media" and "cmake".

### On Linux/Mac OS X
You need a compiler toolchain (gcc or clang on Linux or the included clang from Xcode on OS X), Node.js and Python 2.7 (should already be installed on Mac OS X and most Linux distributions, confirm by running python -V in a terminal).
Node.js can be downloaded as a single file binary from its [download page](https://nodejs.org/en/download/stable/).

Next you will need the portable Emscripten SDK for Linux and OS X from the [Emscripten Downloads page](http://kripken.github.io/emscripten-site/docs/getting_started/downloads.html).

Open a terminal to where you extracted the portable SDK, and run the following commands:

```sh
..\python\python.exe emsdk update
..\python\python.exe emsdk install latest
```

This should install Emscripten and its custom llvm/clang suite. If there are errors, please refer to the guide on the Emscripten download page.

## Building the Emscripten Optimizer
Emscripten comes with C++ source code of its optimizer which needs to be built locally. The source can be found under [emscripten_sdk_root/emscripten/x.y.z/tools/optimizer](https://github.com/kripken/emscripten/tree/master/tools/optimizer).

On Windows you can use the Visual Studio project "[Emscripten-Optimizer.vcxproj](Emscripten-Optimizer.vcxproj)" found in the same directory as this README.md file. Just copy it into the directory of the optimizer, open it with Visual Studio and build it to create "optimizer.exe".

On Linux and Mac OS X you can run the command below in a terminal in the directory of the optimizer to build the binary "optimizer". 
```sh
g++ -std=c++0x -O3 optimizer.cpp optimizer-main.cpp parser.cpp simple_ast.cpp -o optimizer
```

## Setup building ZillaLib
Create a file inside your local ZillaLib directory under the Emscripten sub-directory called "ZillaAppLocalConfig.mk" with the following definitions:
```mk
EMSCRIPTEN_ROOT = D:/dev/emscripten_sdk/emscripten/1.35.0
LLVM_ROOT = D:/dev/emscripten_sdk/clang/e1.35.0_64bit
EMSCRIPTEN_NATIVE_OPTIMIZER = D:/dev/emscripten_sdk/emscripten/1.35.0/tools/optimizer/optimizer.exe
NODE_JS = D:/dev/emscripten_sdk/node/4.1.1_64bit/bin/node.exe
PYTHON = D:/dev/python/python.exe
7ZIP = D:/dev/7z.exe
BROWSER = D:/dev/chromium/chrome.exe
```

The paths are set with forward slashes on all operating systems, not back slashes, and are ending without a slash.

The required settings are:  
  - EMSCRIPTEN_ROOT: Root of the Emscripten suite containing emcc.py etc.
  - LLVM_ROOT: Path to LLVM/Clang for Emscripten, with clang executables
  - EMSCRIPTEN_NATIVE_OPTIMIZER: Path to the Emscripten optimizer executable built in the step above

These settings are for required programs but can be skipped if these are found in system wide PATH environment variable:
  - NODE_JS: Path to Node.js executable
  - PYTHON: Path to Python executable

The rest of the settings are optional depending on your environment and requirements:
  - 7ZIP: Path to the 7z executable that can be used to compress the output binary of a release build to a .gz file that can be uploaded to a webserver instead of the uncompressed file. If not specified, the weaker built-in compression of Python will be used.
  - BROWSER: Path to the executable of a web browser. If not set, will use the system default. This is not required for building, only for running the game directly with the build scripts.

# Building

## Building from Visual Studio on Windows
First we need to tell Visual Studio where Python is located. To do so, follow these steps:
  1. Open up your ZillaLib game project with Visual Studio
  2. Go to the Property Manager with the "VIEW" menu, "Other Windows", then selecting "Property Manager"
  3. In the Property Manager screen, open up "ZillaLib" -> "Emscripten-Debug | Win32" -> "Microsoft.Cpp.Win32.user" (double click on it)
  4. Switch to the "User Macro" sub-category of "Common Properties"
  5. Click "Add Macro" and enter `PythonDir` as the name
  6. For the value, enter the full path to the directory where python.exe is located (without specifying python.exe but with a backslash at the end, i.e. `D:\dev\python\`)
  7. You don't need to check "Set this macro as an environment variable in the build environment"
  8. Press both "OK" buttons to confirm and save the user macro

In Visual Studio, select "Emscripten-Debug" (faster build, bigger output, outputs logging data) or "Emscripten-Release" (optimized and minified build) from the build configuration selection drop down menu at the top. The platform selection (Win32/x64) is irrelevant for Emscripten builds.

Now you can press "Build Solution" under the "Build" menu and (if everything is setup correctly) get a JS file created under your project directory under Debug-emscripten or Release-emscripten.  
You can also select "Start Without Debugging" under the "Debug" menu to directly build and run the game in a browser.

## Building anywhere else
Open a command line/terminal and change directory to your game project, then run the following command for further instructions:
```sh
make emscripten-help
```
On Windows you can use make.exe that is distributed with ZillaLib, you can find it under the "Tools" sub-directory.

With the various make targets you can build debug and release targets and run the output directly.  
For example, running "make emscripten" builds the release JS file, the the asset JS file and a sample HTML page under Release-emscripten. Running "make emscripten-run" builds and opens the game in the browser.

# Deploying
Depending on the project option 'Embed Assets in Binary', your release build will consist of either one or two files.
With assets embedded, the only game file you need to upload YOURGAME_WithData.js.gz. Without, there will be the files YOURGAME.js.gz and YOURGAME_Files.js.gz.
If your webserver does not support sending gz compressed files, you can upload the js file(s) directly.

Now for the actual HTML code that loads the game, the build process creates a sample HTML file which you can reference (or fully copy) to create your own game loading website.  
Some things to consider when adapting the loader:
  - The javascript file(s) is/are loaded with a basic `<script>` tag at the bottom
  - You can set a custom resolution which will override the display resolution set by the C++ code by enabling the commented out lines `requestWidth:` and `requestHeight:` in the javascript section
  - You can remove the logging code that uses `zl_log` in the sample code for release builds - they do not output logging anyway
  - Feel free to rename any object/variables that use the `zl_` prefix, it is only used and referenced inside the html, not the game code

# Possible issues
If your game doesn't show up properly, make sure to check your browsers debug console (usually F12 key). 
Changing certain project properties (name, asset settings) might invalidate the filename references in the generated test HTML file. 
To re-create the HTML file, just delete it inside the Debug-emscripten or Release-emscripten subdirectory and re-run the build process.
