set(argtable2_base ${CMAKE_CURRENT_LIST_DIR}/../../depends/argtable2)

file(GLOB argtable2_source ${argtable2_base}/src/*.c)

add_library(argtable2 STATIC ${argtable2_source})

set_property(TARGET argtable2 PROPERTY INCLUDE_DIRECTORIES
  ${argtable2_base}/include
  )

set_property(TARGET argtable2 PROPERTY COMPILE_OPTIONS
  -Wno-implicit-function-declaration
  )
