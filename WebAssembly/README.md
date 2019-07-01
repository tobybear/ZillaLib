ZillaLib on WebAssembly
=======================

  * [Setup](#setup)
    * [Getting LLVM](#getting-llvm)
    * [Getting System Libraries](#getting-system-libraries)
    * [Getting Python](#getting-python)
    * [Getting wasm-opt](#getting-wasm-opt) (optional)
    * [Setup building with ZillaLib](#setup-building-with-zillalib)
  * [Building](#building)
    * [Building from Visual Studio on Windows](#building-from-visual-studio-on-windows)
    * [Building anywhere else](#building-anywhere-else)
  * [Deploying](#deploying)

# Setup
For getting builds going, we need 4 things that all are easy to get and require minimal setup.
 - [clang and wasm-ld from LLVM](#getting-llvm)
 - [libc/libcxx sources prepared for Wasm](#getting-system-libraries)
 - [any version of Python](#getting-python)
 - [wasm-opt for smaller output files](#getting-wasm-opt) (optional)

## Getting LLVM
We need only clang and wasm-ld from LLVM 8.0.0 or newer which is available on [the official LLVM releases page](https://releases.llvm.org/download.html).  
On Windows it's much simpler to use 7zip to just extract `clang.exe` and `wasm-ld.exe` instead of installing the whole suite.

## Getting System Libraries
The system libraries are maintained in the [Emscripten project](https://github.com/emscripten-core/emscripten/tree/incoming/system).  
Just download the [GitHub archive](https://github.com/emscripten-core/emscripten/archive/incoming.zip) and extract only the `System` directory from it.

## Getting Python
If you already have Python (any version) on your system, you're good to go.  
Otherwise if you're on Windows, there's a simple portable ZIP [here](https://s3.amazonaws.com/mozilla-games/emscripten/packages/python_2.7.5.3_64bit.zip).

## Getting wasm-opt
Using the tool wasm-opt from Binaryen is optional, but it does provide a 15% size reduction of the generated .wasm files.  
Binary releases are available on the [Binaryen project page](https://github.com/WebAssembly/binaryen/releases).  
Feel free to extract only `wasm-opt.exe` and ignore the rest.

## Setup building with ZillaLib
Create a file inside your local ZillaLib directory under the WebAssembly sub-directory called `ZillaAppLocalConfig.mk` with the following definitions:
```mk
LLVM_ROOT = D:/dev/wasm/llvm
SYSTEM_ROOT = D:/dev/wasm/system
PYTHON = D:/dev/python/python.exe
WASMOPT = D:/dev/wasm/wasm-opt.exe
7ZIP = D:/dev/7z.exe
BROWSER = D:/dev/chromium/chrome.exe
```

The paths are set with forward slashes on all operating systems, not back slashes, and directories are ending without a slash.

The required settings are:  
  - LLVM_ROOT: Path to LLVM with clang and wasm-ld executables
  - SYSTEM_ROOT: Path to system with sub-directories `lib` and `include`

These settings are for required programs but can be skipped if these are found in the system wide PATH environment variable:
  - PYTHON: Path to Python executable

The rest of the settings are optional depending on your environment and requirements:
  - WASMOPT: Path to the wasm-opt tool which can be used to reduce size of release builds
  - 7ZIP: Path to the 7z executable that can be used to compress the output binary of a release build to a .gz file that can be uploaded to a webserver instead of the uncompressed file. If not specified, the weaker built-in compression of Python will be used.
  - BROWSER: Path to the executable of a web browser. If not set, will use the system default. This is not required for building, only for running the game directly with the build scripts.

# Building

## Building from Visual Studio on Windows
In Visual Studio with your game project open, select "WebAssembly-Debug" (faster build, bigger output, outputs logging data) or "WebAssembly-Release" (optimized and minified build) from the build configuration selection drop down menu at the top. The platform selection (Win32/x64) is irrelevant for WebAssembly builds.

Now you can press "Build Solution" under the "Build" menu and (if everything is setup correctly) get a JS file created under your project directory under Debug-wasm or Release-wasm.  
You can also select "Start Without Debugging" under the "Debug" menu to directly build and run the game in the browser.

## Building anywhere else
Open a command line/terminal and change directory to your game project, then run the following command for further instructions:
```sh
make wasm-help
```
On Windows you can use make.exe that is distributed with ZillaLib, you can find it under the "Tools" sub-directory.

With the various make targets you can build debug and release targets and run the output directly.  
For example, running `make wasm` builds the release JS file and a sample HTML page under Release-wasm. Running `make wasm-run` builds and opens the game in the browser.

To speed up compilation, the -j parameter can be passed to specify the number of parallel processes to use for building. For example `make wasm -j 4` uses a maximum of 4 processes.

# Deploying
Depending on the project option 'Embed Assets in Binary', your release build will consist of either one or two files.
With assets embedded, the only game file you need to upload is YOURGAME.js.gz. Without, there will be 2 files YOURGAME.js.gz and YOURGAME_Files.js.gz.
If your webserver does not support sending gz compressed files, you can upload the .js file(s) directly.

Now for the actual HTML code that loads the game, the build process creates a sample HTML file which you can reference (or fully copy) to create your own game loading website.  
Some things to consider when adapting the loader:
  - The javascript file(s) is/are loaded with a basic `<script>` tag at the bottom
  - You can set a custom resolution which will override the display resolution set by the C++ code by enabling the commented out lines `requestWidth:` and `requestHeight:` in the javascript section
  - You can remove the logging code that uses `zl_log` in the sample code for release builds - they do not output logging anyway

# Possible issues
If your game doesn't show up properly, make sure to check your browsers debug console (usually F12 key). 
Changing certain project properties (name, asset settings) might invalidate the filename references in the generated test HTML file. 
To re-create the HTML file, just delete it inside the Debug-wasm or Release-wasm subdirectory and re-run the build process.
