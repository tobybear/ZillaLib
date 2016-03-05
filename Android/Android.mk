#
#  ZillaLib
#  Copyright (C) 2010-2016 Bernhard Schelling
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

LOCAL_CPP_EXTENSION := .cpp
LOCAL_C_INCLUDES    := $(ZILLALIB_ANDROID)/stlport/stlport $(ZILLALIB_ROOT)Include
LOCAL_CFLAGS        := -ffunction-sections -fdata-sections -fno-exceptions -fno-non-call-exceptions -fno-rtti

NDK_APP_OPTIM := $(if $(filter true,$(APP_DEBUGGABLE)),debug,$(NDK_APP_OPTIM))
ifeq ($(NDK_APP_OPTIM),debug)
	LOCAL_CFLAGS += -g -funwind-tables -DZILLALOG
	ZILLALIB_OUT := $(ZILLALIB_ANDROID)/build-debug/$(TARGET_ARCH_ABI)
else
	LOCAL_CFLAGS += -fvisibility=hidden
	ZILLALIB_OUT := $(ZILLALIB_ANDROID)/build/$(TARGET_ARCH_ABI)
endif

TARGET_OUT.temp  := $(TARGET_OUT)
TARGET_OBJS.temp := $(TARGET_OBJS)
TARGET_OUT  := $(ZILLALIB_OUT)
TARGET_OBJS := $(ZILLALIB_OUT)
include $(BUILD_STATIC_LIBRARY)
TARGET_OUT  := $(TARGET_OUT.temp)
TARGET_OBJS := $(TARGET_OBJS.temp)
