cpe_fsm_base:=$(call my-dir)/..
cpe_fsm_output:=$(OUTPUT_PATH)/lib/libcpe_fsm.a
cpe_fsm_cpp_flags:=-I$(cpe_fsm_base)/include
cpe_fsm_src:=$(wildcard $(cpe_fsm_base)/src/fsm/*.c)
$(eval $(call def_library,cpe_fsm))
