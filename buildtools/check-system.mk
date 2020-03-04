ORIGN_OS_NAME:=$(shell uname -s)

ifeq ($(ORIGN_OS_NAME),Linux)
ifeq ($(shell uname -p),x86_64)
OS_NAME:=linux64
OS_ARCH:=amd64
else
OS_NAME:=linux32
OS_ARCH:=386
endif
OS_RELEASE_NAME:=$(shell lsb_release -i -s | tr '[:upper:]' '[:lower:]')
endif

ifeq ($(ORIGN_OS_NAME),Darwin)
OS_NAME:=mac
OS_RELEASE_NAME:=darwin
OS_ARCH:=amd64
endif

ifeq ($(filter CYGWIN%,$(ORIGN_OS_NAME)),$(ORIGN_OS_NAME))
ifeq ($(shell uname -m),x86_64)
OS_NAME:=cygwin64
OS_ARCH:=amd64
else
OS_NAME:=cygwin
OS_ARCH:=386
endif
OS_RELEASE_NAME:=cygwin
endif

ifeq ($(filter MSYS_%,$(ORIGN_OS_NAME)),$(ORIGN_OS_NAME))
ifeq ($(shell uname -m),x86_64)
OS_NAME:=mingw64
OS_ARCH:=amd64
else
OS_NAME:=mingw
OS_ARCH:=386
endif
OS_RELEASE_NAME:=mingw
endif

ifeq ($(OS_NAME),)
$(error unknown orign os name $(ORIGN_OS_NAME))
endif
