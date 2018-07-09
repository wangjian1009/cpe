my-dir=$(patsubst %/,%,$(dir $(word $(words $(MAKEFILE_LIST)), $(MAKEFILE_LIST))))
proj-dir=$(patsubst %/,%,$(dir $(word 1, $(MAKEFILE_LIST))))

OUTPUT_PATH?=$(call my-dir)/../build
auto-build-dirs+=$(OUTPUT_PATH)

ifneq ($(OPENWRT_ROOT),)

ORIGN_OS_NAME=Linux
OS_NAME=linux32

OPENWRT_TOOLCHAIN:=$(word 1,$(wildcard $(OPENWRT_ROOT)/staging_dir/toolchain-*))
$(if $(OPENWRT_TOOLCHAIN),,$(warning OpenWRT $(OPENWRT_ROOT) no toolchain found))

openwrt_search_tool=$(foreach a,mipsel arm,$(wildcard $(OPENWRT_TOOLCHAIN)/bin/$a-openwrt-linux-$1))

CC:=$(call openwrt_search_tool,gcc)
$(if $(CC),,$(warning OpenWRT toolchain $(OPENWRT_TOOLCHAIN) no gcc found))

AR:=$(call openwrt_search_tool,ar)
$(if $(AR),,$(warning OpenWRT toolchain $(OPENWRT_TOOLCHAIN) no ar found))

CC:=STAGING_DIR=$(OPENWRT_ROOT)/staging_dir $(CC)
AR:=STAGING_DIR=$(OPENWRT_ROOT)/staging_dir $(AR)

CPPFLAGS+=-DCPE_OS_LINUX=1

else #OPENWRT_ROOT

ORIGN_OS_NAME:=$(shell uname -s)

ifeq ($(ORIGN_OS_NAME),Linux)
ifeq ($(shell uname -p),x86_64)
OS_NAME:=linux64
CPPFLAGS+=-DCPE_OS_LINUX=1
else
OS_NAME:=linux32
CPPFLAGS+=-DCPE_OS_LINUX=1
endif
endif

ifeq ($(ORIGN_OS_NAME),Darwin)
OS_NAME:=mac
CPPFLAGS+=-DCPE_OS_MAC=1
endif

ifeq ($(filter CYGWIN%,$(ORIGN_OS_NAME)),$(ORIGN_OS_NAME))
ifeq ($(shell uname -m),x86_64)
OS_NAME:=cygwin64
CPPFLAGS+=-DCPE_OS_CYGWIN=1
else
OS_NAME:=cygwin
CPPFLAGS+=-DCPE_OS_CYGWIN=1
endif
endif

endif #OPENWRT

ifeq ($(OS_NAME),)
$(error unknown orign os name $(ORIGN_OS_NAME))
endif

#$(call def_c_rule,product,f)
define def_c_rule

$(patsubst $($1_base)/%.c,$(OUTPUT_PATH)/obj/$1/%.o,$2): $2
	$(if $V,,@echo "$1: compiling $(notdir $$@)" &&)$(CC) $(CPPFLAGS) $(CFLAGS) $(if $D,-ggdb) $($1_cpp_flags) $($1_c_flags) -c -o $$@ $$^

endef

#$(call def_executable,product)
define def_library

.PHONY: $1

$1: $($1_output)

$(foreach f,$($1_src),$(call def_c_rule,$1,$f))

$($1_output): $(patsubst $($1_base)/%.c,$(OUTPUT_PATH)/obj/$1/%.o,$($1_src))
	$(if $V,,@echo "$1: archiving $(basename $$@)" &&)$(AR) rcs $$@ $$^

auto-build-dirs+=$(sort $(dir $(patsubst $($1_base)/%.c,$(OUTPUT_PATH)/obj/$1/%.o,$($1_src)) $($1_output)))

endef

#$(call def_executable,product)
define def_executable

.PHONY: $1

$1: $($1_output)

$(foreach f,$($1_src),$(call def_c_rule,$1,$f))

$($1_output): $(patsubst $($1_base)/%.c,$(OUTPUT_PATH)/obj/$1/%.o,$($1_src)) $(foreach d,$($1_depends),$($d_output))
	$(if $V,,@echo "$1: linking $(notdir $$@)" &&)$(CC) $(if $D,-ggdb) $($1_ld_flags) -o $$@ $$^

auto-build-dirs+=$(sort $(dir $(patsubst $($1_base)/%.c,$(OUTPUT_PATH)/obj/$1/%.o,$($1_src)) $($1_output)))

endef
