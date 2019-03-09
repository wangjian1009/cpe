LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := cpe_fsm

LOCAL_CFLAGS += $(if $(filter 0,$(APKD)),,-DDEBUG=1)

LOCAL_C_INCLUDES +=  $(LOCAL_PATH)/../../include

LOCAL_SRC_FILES += $(patsubst $(LOCAL_PATH)/%,%,$(wildcard $(LOCAL_PATH)/../../src/fsm/*.c))

include $(BUILD_STATIC_LIBRARY)
