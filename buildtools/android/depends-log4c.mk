LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := log4c
LOCAL_CFLAGS += $(if $(filter 0,$(APKD)),,-DDEBUG=1)
LOCAL_CFLAGS += -DHAVE_CONFIG_H -DLOG4C_RCPATH=\"\" -Wno-unused

LOCAL_C_INCLUDES +=  $(LOCAL_PATH)/../../depends/log4c/include \
                     $(LOCAL_PATH)/../../depends/log4c/include/log4c \
                     $(LOCAL_PATH)/../../depends/log4c/src \
                     $(LOCAL_PATH)/../../depends/log4c/src/android \
                     $(LOCAL_PATH)/../../include

LOCAL_SRC_FILES += $(addprefix ../../depends/log4c/src/, \
                         rc.c \
                         init.c \
                         appender_type_stream.c \
                         appender_type_stream2.c \
                         appender_type_syslog.c \
                         appender_type_mmap.c \
                         layout_type_basic.c \
                         layout_type_dated.c \
                         layout_type_basic_r.c \
                         layout_type_dated_r.c \
                         version.c \
                         logging_event.c \
                         priority.c \
                         appender.c \
                         layout.c \
                         category.c \
                         appender_type_rollingfile.c \
                         rollingpolicy.c \
                         rollingpolicy_type_sizewin.c) \
                    $(addprefix ../../depends/log4c/src/sd/, \
                         stack.c \
                         list.c \
                         malloc.c \
                         factory.c \
                         hash.c \
                         sprintf.c \
                         test.c \
                         sd_xplatform.c \
                         error.c \
                         domnode.c \
                         domnode-xml.c \
                         domnode-xml-parser.c \
                         domnode-xml-scanner.c )

include $(BUILD_STATIC_LIBRARY)
