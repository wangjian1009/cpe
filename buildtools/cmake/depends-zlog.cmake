set(zlog_base ${CMAKE_CURRENT_LIST_DIR}/../../depends/zlog)

file(GLOB zlog_source ${zlog_base}/src/*.c)

set(zlog_include_directories ${zlog_base})
  
if (OS_NAME STREQUAL "mingw")
  set(zlog_include_directories ${zlog_include_directories}
    ${CMAKE_CURRENT_LIST_DIR}/../../depends/syslog-win32
    ${CMAKE_CURRENT_LIST_DIR}/../../depends/unixem/include
    )
  set(zlog_definitions ${zlog_definitions}
    _POSIX_THREAD_SAFE_FUNCTIONS
    O_SYNC=0)
endif()

if (MSVC)
elseif (CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_IS_GNUCC)
set(zlog_compile_options
  ${zlog_compile_options}
  -Wno-unused
  -Wno-implicit-function-declaration
  -Wno-int-conversion
  -Wno-logical-not-parentheses
  )
endif ()

add_library(zlog STATIC ${zlog_source})
set_property(TARGET zlog PROPERTY COMPILE_DEFINITIONS ${zlog_definitions})
set_property(TARGET zlog PROPERTY INCLUDE_DIRECTORIES ${zlog_include_directories})
set_property(TARGET zlog PROPERTY COMPILE_OPTIONS ${zlog_compile_options})

if (OS_NAME STREQUAL "mingw")
  target_link_libraries(zlog INTERFACE syslog)
endif()
