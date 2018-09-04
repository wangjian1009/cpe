#ORIGN_OS_NAME=Linux
#OS_NAME=linux32

#CPPFLAGS+=-DCPE_OS_LINUX=1

#ORIGN_OS_NAME:=$(shell uname -s)

#OS_NAME:=linux64
#CPPFLAGS+=-DCPE_OS_LINUX=1

#OS_NAME:=linux32
#CPPFLAGS+=-DCPE_OS_LINUX=1

#OS_NAME:=mac
#CPPFLAGS+=-DCPE_OS_MAC=1

#OS_NAME:=cygwin64
#OS_NAME:=cygwin

set(OS_NAME mac)

add_definitions(
  -DCPE_OS_MAC=1
  )
