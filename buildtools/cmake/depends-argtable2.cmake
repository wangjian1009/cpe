project(argtable2)

set(argtable2_base ${CMAKE_CURRENT_LIST_DIR}/../../depends/argtable2)

include_directories(
  ${argtable2_base}/include
  )

add_definitions(
  -Wno-implicit-function-declaration
  )

file(GLOB argtable2_source ${argtable2_base}/src/*.c)

add_library(argtable2 STATIC ${argtable2_source})
