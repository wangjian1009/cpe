LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := cpe_utils

LOCAL_CFLAGS += $(if $(filter 0,$(APKD)),,-DDEBUG=1) \
                -Wno-unused -Wno-parentheses -Wall

LOCAL_C_INCLUDES +=  $(LOCAL_PATH)/../../include

LOCAL_SRC_FILES += $(addprefix $(LOCAL_PATH)/../../src/pal/,\
                       pal_socket.c \
                   ) \
                   $(wildcard $(LOCAL_PATH)/../../src/utils/*.c) \
                    $(addprefix $(LOCAL_PATH)/../../src/utils_sock/,\
                       sock_utils.c \
                       getgateway.c \
                       getdnssvraddrs.c \
                    )

include $(BUILD_STATIC_LIBRARY)
