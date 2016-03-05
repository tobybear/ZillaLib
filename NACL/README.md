ZillaLib on Native Client
=========================

# Setup

You can follow the guide on the NACL developers page on [how to set up the Native Client SDK](https://developer.chrome.com/native-client/sdk/download). Here are the main steps.

## Get Python 2.7

If you're running anything besides Windows, Python 2.7 should already be installed by your OS. You can check it by running "python -V" in a terminal.

On Windows, you can get the latest 2.7.x release from the [Python Downloads page](https://www.python.org/downloads/). You can get either x86-64 or x86 builds.  
If you don't want Python to be installed and registered system wide, you can manually extract the MSI file by running the following command in a command line prompt. You can optionally even remove the directories doc, include, libs, tcl and tools after extraction.
```sh
msiexec.exe /a python-2.7.11.amd64.msi /qb TARGETDIR="D:\dev\python"
```

## Get the NACL SDK

Download the SDK installer zip file from the [NACL Downloads page](https://developer.chrome.com/native-client/sdk/download) and extract it to a place where you have your development stuff like D:\dev\nacl_sdk

Open up a command line prompt/terminal and change the directory to where you extracted the SDK zip file. If you manually extracted python in the previous step, you need to make sure python is in your PATH environment variable by running "set PATH=%PATH%;D:\dev\python" (without quotes).  
Now run the following command to check the list of available NACL SDK versions.
```sh
naclsdk list
```

It is recommended to just get the latest stable version as noted by (stable) in the list. At the time of writing, it is pepper_47.
```sh
naclsdk install pepper_47
```

Because the SDK has gotten rather big over the years, feel free to remove stuff under the SDK version pepper_xx directory we don't need anyway
  - You can remove everything containing "arm", "i686", "mipsel" and "x86" in the file and directory name under /toolchain, /toolchain/win_pnacl and /toolchain/win_pnacl/bin
  - If you only want to build for ZillaLib and don't need the documentation parts, you can remove "examples", "getting_started" and (if available) "ports"

This removes the ability to build old NACL style (building separate for each processor and only deploying to the Chrome Web Store) and keeps the PNACL/LE32 compilation environement (building a portable binary which can be deployed anywhere).  
This keeps the SDK at about 250 MB by trimming 2 GB of these unused compilation environments.

## Setup building ZillaLib

Create a file inside your local ZillaLib directory under the NACL sub-directory called "ZillaAppLocalConfig.mk" with the following definitions:
```mk
NACL_SDK = D:/dev/nacl_sdk/pepper_47
PYTHON = D:/dev/python/python.exe
7ZIP = D:/dev/7z.exe
CHROME = C:/Programme/Chromium Portable/Application/chrome.exe
```

The setting "NACL_SDK" points to the SDK installed above. Make sure this is not the place where you extracted the SDK installer zip file, but the subdirectory containing the actual SDK version. The paths are with forward slashes, not back slashes, and are ending without a slash.

All other settings can be optional, depending on your environment.
  - PYTHON: Path to the executable of Python. Only required if Python was installed manually and not system wide.
  - 7ZIP: Path to the 7z executable that can be used to compress the output binary of a release build to a .gz file that can be uploaded to a webserver instead of the uncompressed file. If not specified, the weaker built-in compression of Python will be used.
  - CHROME: (Not required for building, only for running the game directly with the build scripts) Path to the executable of the Chrome/Chromium browser. If installed in a default path, this can be detected automatically.

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

In Visual Studio, select "NACL-Debug" (faster build, bigger output, outputs logging data) or "NACL-Release" (optimized build) from the build configuration selection drop down menu at the top. The platform selection (Win32/x64) is irrelevant for NACL builds.

Now you can press "Build Solution" under the "Build" menu and (if everything is setup correctly) get a PEXE file created under your project directory under Debug-nacl or Release-nacl.  
You can also select "Start Without Debugging" under the "Debug" menu to directly build and run the game in the Chrome or Chromium browser.

## Building anywhere else
Open a command line/terminal and change directory to your game project, then run the following command for further instructions:
```sh
make nacl-help
```
On Windows you can use the make binary that is distributed with ZillaLib, you can find it under the "Tools" sub-directory.

With the various make targets you can build debug and release targets and run the output directly.  
For example, running "make nacl" builds the release PEXE file, the NMF nacl manifest, the asset ZIP file and a sample HTML page under Release-nacl. Running "make nacl-run" builds and runs the game in the browser by running a temporary webserver.

# Deploying
Depending on the project option 'Embed Assets in Binary', your release build will consist of either one or two files.
With assets embedded, the only game file you need to upload is ZillaLibSamples_WithData.pexe.gz. Without, there will be the files ZillaLibSamples.pexe.gz and ZillaLibSamples_Files.dat.
If your webserver does not support sending gz compressed files, you can upload the non compressed file(s) directly.

Now for the actual HTML code that loads the game, the build process creates a sample HTML file which you can reference (or fully copy) to create your own game loading website.  
Some things to consider when adapting the loader:
  - Error messages can be customized/translated easily, they are near the top of the loader code
  - The url to the PEXE and the assets are near the top of the javascript part of the loader. The variable url_pexe needs to be a full url including protocol and domain name.
  - You can set a custom resolution which will override the display resolution set by the C++ code with `zl_module.postMessage('SZX'+'1000');` and `zl_module.postMessage('SZY'+'600');`
  - You can remove the logging code that uses `zl_log` in the sample code for release builds - they do not output logging anyway
  - Feel free to rename any object/variables that use the `zl_` prefix, it is only used and referenced inside the html, not the game code

# Possible issues
If your game doesn't show up properly, make sure to check your browsers debug console (usually F12 key). 
Changing certain project properties (name, asset settings) might invalidate the filename references in the generated test HTML file. 
To re-create the HTML file, just delete it inside the Debug-nacl/Release-nacl subdirectory and run the build process again.
