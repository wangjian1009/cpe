yajl_base:=$(call my-dir)/../depends/yajl
yajl_output:=$(OUTPUT_PATH)/lib/libyajl.a
yajl_cpp_flags:=-I$(yajl_base)/include
yajl_c_flags:=-Wno-implicit-function-declaration -Wno-enum-conversion
yajl_src:=$(wildcard $(yajl_base)/src/*.c)
$(eval $(call def_library,yajl))

