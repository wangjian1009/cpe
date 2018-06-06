LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := argtable2
LOCAL_CFLAGS += $(if $(filter 0,$(APKD)),,-DDEBUG=1) -DHAVE_CONFIG_H
LOCAL_CFLAGS +=  -Wno-unused -Wno-parentheses -Wno-unused-variable
LOCAL_C_INCLUDES +=  $(LOCAL_PATH)/../../depends/argtable2/include \
                     $(LOCAL_PATH)/../../depends/argtable2/src/android
LOCAL_SRC_FILES +=  $(wildcard $(LOCAL_PATH)/../../depends/argtable2/src/*.c)

include $(BUILD_STATIC_LIBRARY)
