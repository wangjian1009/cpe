set(zlog_base ${CMAKE_CURRENT_LIST_DIR}/../../depends/zlog)

file(GLOB zlog_source ${zlog_base}/src/*.c)

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

set_property(TARGET zlog PROPERTY INCLUDE_DIRECTORIES
  ${zlog_base}/src
  )

set_property(TARGET zlog PROPERTY COMPILE_OPTIONS ${zlog_compile_options})
