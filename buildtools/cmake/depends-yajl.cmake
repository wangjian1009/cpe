project(yajl)

set(yajl_base ${CMAKE_CURRENT_LIST_DIR}/../../depends/yajl)

include_directories(
  ${yajl_base}/include
  )

add_definitions("-Wno-implicit-function-declaration")

file(GLOB yajl_source ${yajl_base}/src/*.c)

add_library(yajl STATIC ${yajl_source})
