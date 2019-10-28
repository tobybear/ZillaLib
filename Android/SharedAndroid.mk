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

ZILLALIB_DIR := $(dir $(call my-dir))

ifneq ($(words $(ZILLALIB_DIR)),1)
  $(error ZILLALIB_DIR is set to "$(ZILLALIB_DIR)" which contains spaces. Paths with spaces are not supported for Android builds at the moment.)
endif
ifeq ($(ZillaApp),)
  $(error ZillaApp parameter was not set, please pass it along to ndk-build with a parameter "ZillaApp=SHORTAPPNAME")
endif

include $(CLEAR_VARS)
LOCAL_MODULE    := $(ZillaApp)
LOCAL_PATH      := .
LOCAL_SRC_FILES := $(wildcard *.cpp *.c)

-include sources.mk
LOCAL_SRC_FILES += $(foreach F, $(ZL_ADD_SRC_FILES), $(wildcard $(F)))

LOCAL_C_INCLUDES       := $(ZILLALIB_DIR)Include
LOCAL_CFLAGS           := -fno-exceptions -fno-non-call-exceptions -fno-rtti
LOCAL_CPPFLAGS         := -std=gnu++11
LOCAL_CPP_EXTENSION    := .cpp
LOCAL_LDLIBS           := -lGLESv2 -lz -lOpenSLES -lEGL -landroid

NDK_APP_OPTIM := $(if $(filter true,$(APP_DEBUGGABLE)),debug,$(NDK_APP_OPTIM))
ifeq ($(NDK_APP_OPTIM),debug)
	ZILLALIB_OUT  := $(ZILLALIB_DIR)Android/build-debug/$(TARGET_ARCH_ABI)
	LOCAL_CFLAGS  += -g -funwind-tables -DZILLALOG
	LOCAL_CFLAGS  += -ffunction-sections -fdata-sections -fvisibility=hidden
	LOCAL_LDLIBS  += -llog
else
	ZILLALIB_OUT  := $(ZILLALIB_DIR)Android/build/$(TARGET_ARCH_ABI)
	LOCAL_CFLAGS  += -O$(RELEASE_OPTIMIZATION)
	LOCAL_CFLAGS  += -ffunction-sections -fdata-sections -fvisibility=hidden
	LOCAL_CFLAGS  += -fno-stack-protector
	LOCAL_CFLAGS  += -fomit-frame-pointer
	LOCAL_CFLAGS  += -fno-unwind-tables
	LOCAL_CFLAGS  += -fno-asynchronous-unwind-tables
	LOCAL_CFLAGS  += -fno-math-errno
	LOCAL_CFLAGS  += -fmerge-all-constants
	LOCAL_CFLAGS  += -fno-ident
	LOCAL_LDFLAGS := -Wl,--gc-sections
	LOCAL_LDFLAGS += -Wl,-z,norelro
	LOCAL_LDFLAGS += -Wl,-z,lazy
	LOCAL_LDFLAGS += -Wl,--hash-style=sysv
	LOCAL_LDFLAGS += -Wl,--build-id=none
	LOCAL_LDFLAGS += -Wl,--spare-dynamic-tags=0
	LOCAL_LDFLAGS += -Wl,--sort-section=name
	LOCAL_LDFLAGS += -Wl,-O,3
	LOCAL_LDFLAGS += -fuse-ld=gold -Wl,--icf=safe -Wl,--icf-iterations=5
	#LOCAL_LDFLAGS += -Wl,-Map,$(TARGET_OUT)/lib$(ZillaApp).map -Wl,--cref -Wl,--demangle
endif

#add defines from the make command line (e.g. D=MACRO=VALUE)
LOCAL_CFLAGS += $(subst \\\, ,$(foreach F,$(subst \ ,\\\,$(D)),"-D$(F)"))

#if we're being called with B flag (always-make), build the libraries only if they don't exist at all, otherwise just link the existing files
AUTO_REBUILD_LIBS := $(if $(filter -B,$(MAKEFLAGS)),$(if $(wildcard $(ZILLALIB_OUT)/libZillaLib.a),NO,YES),YES)

ifeq ($(AUTO_REBUILD_LIBS),YES)
	LOCAL_STATIC_LIBRARIES := ZillaLib
else
	link-whole-archives += $(ZILLALIB_OUT)/libZillaLib.a
endif

include $(BUILD_SHARED_LIBRARY)

ifeq ($(AUTO_REBUILD_LIBS),YES)
	include $(ZILLALIB_DIR)Android/Android.mk
endif
