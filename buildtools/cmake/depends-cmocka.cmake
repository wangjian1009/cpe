set(cmocka_base ${CMAKE_CURRENT_LIST_DIR}/../../depends/cmocka-1.1.5)

set(cmocka_definitions HAVE_INTTYPES_H HAVE_SIGNAL_H HAVE_STRINGS_H)

file(GLOB cmocka_source ${cmocka_base}/src/*.c)

add_library(cmocka STATIC ${cmocka_source})

set_property(TARGET cmocka PROPERTY COMPILE_DEFINITIONS ${cmocka_definitions})

set_property(TARGET cmocka PROPERTY INCLUDE_DIRECTORIES
  ${cmocka_base}/include
  )

set_property(TARGET cmocka PROPERTY COMPILE_OPTIONS ${cmocka_compile_options})
