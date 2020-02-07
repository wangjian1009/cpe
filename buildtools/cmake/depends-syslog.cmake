set(syslog_base ${CMAKE_CURRENT_LIST_DIR}/../../depends/syslog-win32)

file(GLOB syslog_source ${syslog_base}/*.c)

if (MSVC)
elseif (CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_IS_GNUCC)
set(syslog_compile_options
  ${syslog_compile_options}
  -Wno-unused
  -Wno-implicit-function-declaration
  -Wno-int-conversion
  -Wno-logical-not-parentheses
  )
endif ()

add_library(syslog STATIC ${syslog_source})

set_property(TARGET syslog PROPERTY INCLUDE_DIRECTORIES
  ${syslog_base}
  )

set_property(TARGET syslog PROPERTY COMPILE_OPTIONS ${syslog_compile_options})
