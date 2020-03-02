ORIGN_OS_NAME:=$(shell uname -s)

ifeq ($(ORIGN_OS_NAME),Linux)
ifeq ($(shell uname -p),x86_64)
OS_NAME:=linux64
else
OS_NAME:=linux32
endif
OS_RELEASE_NAME:=$(shell lsb_release -i -s)
endif

ifeq ($(ORIGN_OS_NAME),Darwin)
OS_NAME:=mac
OS_RELEASE_NAME:=darwin
endif

ifeq ($(filter CYGWIN%,$(ORIGN_OS_NAME)),$(ORIGN_OS_NAME))
ifeq ($(shell uname -m),x86_64)
OS_NAME:=cygwin64
else
OS_NAME:=cygwin
endif

OS_RELEASE_NAME:=$(OS_NAME)
endif

ifeq ($(OS_NAME),)
$(error unknown orign os name $(ORIGN_OS_NAME))
endif
