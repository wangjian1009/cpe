cpe_utils_base:=$(call my-dir)/..
cpe_utils_output:=$(OUTPUT_PATH)/lib/libcpe_utils.a
cpe_utils_cpp_flags:=-I$(cpe_utils_base)/include
cpe_utils_src:=$(addprefix $(cpe_utils_base)/src/pal/,\
                    pal_socket.c \
                ) \
                $(addprefix $(cpe_utils_base)/src/utils/,\
                    memory.c \
                    error.c \
                    string_utils.c \
                    hex_utils.c \
                    time_utils.c \
                    buffer.c buffer_pos.c buffer_trunk.c \
                    fcvt.c \
                    stream.c \
                    stream_printf.c \
                    hash.c \
                    hash_standalone.c \
                    hash_algo.c \
                    base64.c \
                    array.c \
                    ringbuffer.c \
                    file_file.c \
                    file_wrapper.c \
                 ) \
                 $(addprefix $(cpe_utils_base)/src/utils_sock/,\
                     sock_utils.c \
                 )
$(eval $(call def_library,cpe_utils))
