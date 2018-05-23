LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := cpe_utils

LOCAL_CFLAGS += $(if $(filter 0,$(APKD)),,-DDEBUG=1) \
                -Wno-unused -Wno-parentheses -Wall

LOCAL_C_INCLUDES +=  $(LOCAL_PATH)/../../../include

LOCAL_SRC_FILES += $(addprefix $(LOCAL_PATH)/../../../src/pal/,\
                       pal_socket.c \
                   ) \
                   $(addprefix $(LOCAL_PATH)/../../../src/utils/,\
                       memory.c \
                       error.c \
                       string_utils.c \
                       hex_utils.c \
                       time_utils.c \
                       buffer.c buffer_pos.c buffer_trunk.c \
                       fcvt.c \
                       stream.c \
                       stream_printf.c \
                       array.c \
                       hash.c \
                       hash_standalone.c \
                       hash_algo.c \
                       base64.c \
                       ringbuffer.c \
                       file_file.c \
                       file_wrapper.c \
                    ) \
                    $(addprefix $(LOCAL_PATH)/../../../src/utils_sock/,\
                       sock_utils.c \
                    )

include $(BUILD_STATIC_LIBRARY)
