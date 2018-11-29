LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := yajl
LOCAL_CFLAGS += $(if $(filter 0,$(APKD)),,-DDEBUG=1)
LOCAL_CFLAGS +=  -Wno-unused -Wno-parentheses -Wno-unused-variable -Wno-enum-conversion
LOCAL_C_INCLUDES +=  $(LOCAL_PATH)/../../depends/yajl/include
LOCAL_SRC_FILES += $(patsubst $(LOCAL_PATH)/%,%,$(wildcard $(LOCAL_PATH)/../../depends/yajl/src/*.c))

include $(BUILD_STATIC_LIBRARY)
