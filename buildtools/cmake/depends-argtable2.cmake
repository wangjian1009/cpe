set(argtable2_base ${CMAKE_CURRENT_LIST_DIR}/../../depends/argtable2)

set(argtable2_source
  ${argtable2_base}/src/arg_date.c
  ${argtable2_base}/src/arg_dbl.c
  ${argtable2_base}/src/arg_end.c
  ${argtable2_base}/src/arg_file.c
  ${argtable2_base}/src/arg_int.c
  ${argtable2_base}/src/arg_lit.c
  ${argtable2_base}/src/arg_rem.c
  ${argtable2_base}/src/arg_str.c
  ${argtable2_base}/src/argtable2.c
  ${argtable2_base}/src/getopt1.c
  ${argtable2_base}/src/getopt.c
  )

if (NOT MSVC)
set(argtable2_source
  ${argtable2_source}
  ${argtable2_base}/src/arg_rex.c
  )
endif ()

add_library(argtable2 STATIC ${argtable2_source})

set_property(TARGET argtable2 PROPERTY INCLUDE_DIRECTORIES
  ${argtable2_base}/include
  ${argtable2_base}/src/${OS_NAME}
  )

set_property(TARGET argtable2 PROPERTY COMPILE_DEFINITIONS
  HAVE_CONFIG_H
  )

if (GCC)
set_property(TARGET argtable2 PROPERTY COMPILE_OPTIONS
  -Wno-implicit-function-declaration
  )
endif ()
