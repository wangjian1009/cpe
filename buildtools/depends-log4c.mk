log4c_base:=$(call my-dir)/../depends/log4c
log4c_output:=$(OUTPUT_PATH)/lib/liblog4c.a
log4c_cpp_flags:=-I$(log4c_base)/include -I$(log4c_base)/include/log4c -I$(log4c_base)/src -I$(log4c_base)/src/$(OS_NAME)
log4c_c_flags:=-DHAVE_CONFIG_H -DLOG4C_RCPATH=\"\" -Wno-unused
log4c_src:=$(addprefix $(log4c_base)/src/, \
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
                    $(addprefix $(log4c_base)/src/sd/, \
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
$(eval $(call def_library,log4c))
