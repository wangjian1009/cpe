cpe_utils_base:=$(call my-dir)/..
cpe_utils_output:=$(OUTPUT_PATH)/lib/libcpe_utils.a
cpe_utils_cpp_flags:=-I$(cpe_utils_base)/include
cpe_utils_src:=$(addprefix $(cpe_utils_base)/src/pal/,\
                    pal_socket.c \
                    pal_error.c \
                ) \
                $(wildcard $(cpe_utils_base)/src/utils/*.c) \
                 $(addprefix $(cpe_utils_base)/src/utils_sock/,\
                     sock_utils.c \
                 )
$(eval $(call def_library,cpe_utils))
