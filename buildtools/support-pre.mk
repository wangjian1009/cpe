my-dir=$(patsubst %/,%,$(dir $(word $(words $(MAKEFILE_LIST)), $(MAKEFILE_LIST))))
proj-dir=$(patsubst %/,%,$(dir $(word 1, $(MAKEFILE_LIST))))

OUTPUT_PATH?=$(call my-dir)/../build
auto-build-dirs+=$(OUTPUT_PATH)

ORIGN_OS_NAME:=$(shell uname -s)

ifeq ($(ORIGN_OS_NAME),Linux)
ifeq ($(shell uname -p),x86_64)
OS_NAME:=linux64
else
OS_NAME:=linux32
endif
endif

ifeq ($(ORIGN_OS_NAME),Darwin)
OS_NAME:=mac
endif

ifeq ($(filter CYGWIN%,$(ORIGN_OS_NAME)),$(ORIGN_OS_NAME))
ifeq ($(shell uname -m),x86_64)
OS_NAME:=cygwin64
else
OS_NAME:=cygwin
endif
endif

ifeq ($(OS_NAME),)
$(error unknown orign os name $(ORIGN_OS_NAME))
endif

#$(call def_c_rule,product,f)
define def_c_rule

$(patsubst $($1_base)/%.c,$(OUTPUT_PATH)/obj/$1/%.o,$2): $2
	$(if $V,,@echo "$1: compiling $(notdir $$@)" &&)CC $(if $D,-ggdb) $($1_cpp_flags) $($1_c_flags) -c -o $$@ $$^

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
	$(if $V,,@echo "$1: linking $(notdir $$@)" &&)CC $(if $D,-ggdb) $($1_ld_flags) -o $$@ $$^

auto-build-dirs+=$(sort $(dir $(patsubst $($1_base)/%.c,$(OUTPUT_PATH)/obj/$1/%.o,$($1_src)) $($1_output)))

endef
