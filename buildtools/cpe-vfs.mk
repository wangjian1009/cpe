cpe_vfs_base:=$(call my-dir)/..
cpe_vfs_output:=$(OUTPUT_PATH)/lib/libcpe_vfs.a
cpe_vfs_cpp_flags:=-I$(cpe_vfs_base)/include
cpe_vfs_src:=$(wildcard $(cpe_vfs_base)/src/vfs/*.c)
$(eval $(call def_library,cpe_vfs))
