set(unixem_base ${CMAKE_CURRENT_LIST_DIR}/../../depends/unixem)

file(GLOB unixem_source ${unixem_base}/src/*.c ${unixem_base}/src/internal/*.c)

if (MSVC)
elseif (CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_IS_GNUCC)
set(unixem_compile_options
  ${unixem_compile_options}
  -Wno-unused
  -Wno-implicit-function-declaration
  -Wno-int-conversion
  -Wno-logical-not-parentheses
  )
endif ()

add_library(unixem STATIC ${unixem_source})

set_property(TARGET unixem PROPERTY INCLUDE_DIRECTORIES
  ${unixem_base}/include
  )

set_property(TARGET unixem PROPERTY COMPILE_OPTIONS ${unixem_compile_options})
