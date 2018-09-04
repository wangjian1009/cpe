set(yajl_base ${CMAKE_CURRENT_LIST_DIR}/../../depends/yajl)

file(GLOB yajl_source ${yajl_base}/src/*.c)

add_library(yajl STATIC ${yajl_source})

set_property(TARGET yajl PROPERTY INCLUDE_DIRECTORIES
  ${yajl_base}/include
  )

set_property(TARGET yajl PROPERTY COMPILE_OPTIONS
  -Wno-implicit-function-declaration
  )
