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

ZILLALIB_ANDROID := $(call my-dir)
ZILLALIB_ROOT := $(if $(filter .,$(ZILLALIB_ANDROID)),../,$(dir $(ZILLALIB_ANDROID)))

include $(CLEAR_VARS)

LOCAL_MODULE := ZillaLib
LOCAL_PATH := $(ZILLALIB_ROOT)Source

LOCAL_SRC_FILES := *.cpp enet/*.c libtess2/*.c stb/*.cpp
LOCAL_SRC_FILES := $(foreach F,$(LOCAL_SRC_FILES),$(addprefix $(if $(subst ./,,$(dir $(F))),$(dir $(F)),),$(notdir $(wildcard $(LOCAL_PATH)/$(F)))))

# setting calculation intensive modules to compile in full arm mode (not thumb arm)
LOCAL_SRC_FILES := $(subst ZL_Particles.cpp,ZL_Particles.cpp.arm,$(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(subst ZL_Surface.cpp,ZL_Surface.cpp.arm,$(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(subst ZL_Audio.cpp,ZL_Audio.cpp.arm,$(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(subst ZL_SynthImc.cpp,ZL_SynthImc.cpp.arm,$(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(subst ZL_Display3D.cpp,ZL_Display3D.cpp.arm,$(LOCAL_SRC_FILES))

LOCAL_C_INCLUDES    := $(ZILLALIB_ROOT)Include
LOCAL_CFLAGS        := -fno-exceptions -fno-non-call-exceptions -fno-rtti
LOCAL_CPPFLAGS      := -std=gnu++11
LOCAL_CPP_EXTENSION := .cpp

NDK_APP_OPTIM := $(if $(filter true,$(APP_DEBUGGABLE)),debug,$(NDK_APP_OPTIM))
ifeq ($(NDK_APP_OPTIM),debug)
	ZILLALIB_OUT := $(ZILLALIB_ANDROID)/build-debug/$(TARGET_ARCH_ABI)
	LOCAL_CFLAGS += -g -funwind-tables -DZILLALOG
	LOCAL_CFLAGS += -ffunction-sections -fdata-sections -fvisibility=hidden
else
	ZILLALIB_OUT := $(ZILLALIB_ANDROID)/build/$(TARGET_ARCH_ABI)
	LOCAL_CFLAGS += -O$(RELEASE_OPTIMIZATION)
	LOCAL_CFLAGS += -ffunction-sections -fdata-sections -fvisibility=hidden
	LOCAL_CFLAGS += -fno-stack-protector
	LOCAL_CFLAGS += -fomit-frame-pointer
	LOCAL_CFLAGS += -fno-unwind-tables
	LOCAL_CFLAGS += -fno-asynchronous-unwind-tables
	LOCAL_CFLAGS += -fno-math-errno
	LOCAL_CFLAGS += -fmerge-all-constants
	LOCAL_CFLAGS += -fno-ident
endif

TARGET_OUT.temp  := $(TARGET_OUT)
TARGET_OBJS.temp := $(TARGET_OBJS)
TARGET_OUT  := $(ZILLALIB_OUT)
TARGET_OBJS := $(ZILLALIB_OUT)
include $(BUILD_STATIC_LIBRARY)
TARGET_OUT  := $(TARGET_OUT.temp)
TARGET_OBJS := $(TARGET_OBJS.temp)
