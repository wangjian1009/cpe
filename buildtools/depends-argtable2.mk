argtable2_base:=$(call my-dir)/../depends/argtable2
argtable2_output:=$(OUTPUT_PATH)/lib/libargtable2.a
argtable2_cpp_flags:=-I$(argtable2_base)/include -I$(argtable2_base)/src/$(OS_NAME) -DHAVE_CONFIG_H
argtable2_c_flags:=-Wno-implicit-function-declaration
argtable2_src:=$(wildcard $(argtable2_base)/src/*.c)
$(eval $(call def_library,argtable2))
