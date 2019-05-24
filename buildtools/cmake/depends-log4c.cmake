set(log4c_base ${CMAKE_CURRENT_LIST_DIR}/../../depends/log4c)

set(log4c_source
  ${log4c_base}/src/rc.c
  ${log4c_base}/src/init.c
  ${log4c_base}/src/appender_type_stream.c
  ${log4c_base}/src/appender_type_stream2.c
  ${log4c_base}/src/layout_type_basic.c
  ${log4c_base}/src/layout_type_dated.c
  ${log4c_base}/src/layout_type_basic_r.c
  ${log4c_base}/src/layout_type_dated_r.c
  ${log4c_base}/src/version.c
  ${log4c_base}/src/logging_event.c
  ${log4c_base}/src/priority.c
  ${log4c_base}/src/appender.c
  ${log4c_base}/src/layout.c
  ${log4c_base}/src/category.c
  ${log4c_base}/src/appender_type_rollingfile.c
  ${log4c_base}/src/rollingpolicy.c
  ${log4c_base}/src/rollingpolicy_type_sizewin.c
  ${log4c_base}/src/sd/stack.c
  ${log4c_base}/src/sd/list.c
  ${log4c_base}/src/sd/malloc.c
  ${log4c_base}/src/sd/factory.c
  ${log4c_base}/src/sd/hash.c
  ${log4c_base}/src/sd/sprintf.c
  ${log4c_base}/src/sd/test.c
  ${log4c_base}/src/sd/sd_xplatform.c
  ${log4c_base}/src/sd/error.c
  ${log4c_base}/src/sd/domnode.c
  ${log4c_base}/src/sd/domnode-xml.c
  ${log4c_base}/src/sd/domnode-xml-parser.c
  ${log4c_base}/src/sd/domnode-xml-scanner.c
  )

if (OS_NAME MATCHES "mac" OR OS_NAME MATCHES "linux" OR OS_NAME MATCHES "android")
set(log4c_source
  ${log4c_source}
  ${log4c_base}/src/appender_type_syslog.c
  ${log4c_base}/src/appender_type_mmap.c
  )
endif ()

set(log4c_compile_options
  -DHAVE_CONFIG_H
  -DLOG4C_RCPATH=\"\"
  )

if (MSVC)
elseif (CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_IS_GNUCC)
set(log4c_compile_options
  ${log4c_compile_options}
  -Wno-unused
  -Wno-deprecated-declarations
  )
endif ()

add_library(log4c STATIC ${log4c_source})

set_property(TARGET log4c PROPERTY INCLUDE_DIRECTORIES
  ${log4c_base}/include
  ${log4c_base}/include/log4c
  ${log4c_base}/src
  ${log4c_base}/src/${OS_NAME}
  ${CMAKE_CURRENT_LIST_DIR}/../include
  )

set_property(TARGET log4c PROPERTY COMPILE_OPTIONS ${log4c_compile_options})
