ZillaLib on Linux
=================

  * [Setup](#setup)
  * [Building](#building)
  * [SDL2](#sdl2)
  * [OpenGL requirements](#opengl-requirements)

# Setup

Building for linux requires an installed and running linux operating system (either 32 or 64-bit) and the following packages installed:
  - gcc
  - make
  - g++
  - libgl1-mesa-dev
  - libasound2-dev

By default, it will use the system machine (32 or 64-bit) for building.  
If you have a cross compile environment set up, you can specify to build a specific machine target by supplying M32=1 or M64=1 as a parameter to the make command line.

# Building

Open a terminal and change the directory to your game project, then run the following command for further instructions:
```sh
make linux-help
```

With the various make targets you can build debug and release executables, run them normally or with the gdb command line debugger.
For example, running "make linux" builds the debug target or "make linux-release-run" builds and runs the release build.

# SDL2

ZillaLib comes with a minimal SDL2 distribution that normally gets statically linked to keep the dependencies at a minimum.

Alternatively it can be built against an external SDL2 shared library which can increase compatiblity with more linux variants.
To do so, create a file inside your local ZillaLib directory under the Linux sub-directory called "ZillaAppLocalConfig.mk" with the following definitions:
```mk
USE_EXTERNAL_SDL2 = 1
EXTERNAL_SDL2_INCLUDE = /path/to/SDL2/include
EXTERNAL_SDL2_SO = /path/to/libSDL2.so
```

# OpenGL requirements

ZillaLib by default requires at least OpenGL 2.1 and OpenGL shading language 1.20. You can check the version of OpenGL supported by your system by running `glxinfo | grep -i version`.
When runnin a Linux desktop on a virtual machine (or certain GPU/driver configurations) it is possible, that the system supports only a limited set of OpenGL features even though it reports a certain version.
So if your game starts up but only draws some unshaded/untextured areas or nothing at all, try to set ALWAYS_SOFTWARE with the following command before running the game:
```sh
export LIBGL_ALWAYS_SOFTWARE=1
```
