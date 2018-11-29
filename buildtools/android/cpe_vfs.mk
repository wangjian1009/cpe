LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := cpe_vfs

LOCAL_CFLAGS += $(if $(filter 0,$(APKD)),,-DDEBUG=1) \
                -Wno-unused -Wno-parentheses -Wall

LOCAL_C_INCLUDES +=  $(LOCAL_PATH)/../../include

LOCAL_SRC_FILES += $(patsubst $(LOCAL_PATH)/%,%,$(wildcard $(LOCAL_PATH)/../../src/vfs/*.c))

include $(BUILD_STATIC_LIBRARY)
