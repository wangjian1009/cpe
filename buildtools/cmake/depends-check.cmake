set(check_base ${CMAKE_CURRENT_LIST_DIR}/../../depends/check)

set(HAVE_FORK 1)
set(ENABLE_SUBUNIT 1)
set(CHECK_MAJOR_VERSION 0)
set(CHECK_MINOR_VERSION 13)
set(CHECK_MICRO_VERSION 0)

configure_file(
    ${check_base}/src/check.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/check/check.h)
configure_file(
    ${check_base}/cmake/check_stdint.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/check/check_stdint.h)

file(GLOB check_source ${check_base}/src/*.c)

set(check_source ${check_source}
    ${CMAKE_CURRENT_BINARY_DIR}/check/check.h
    ${CMAKE_CURRENT_BINARY_DIR}/check/check_stdint.h
    )

if (MSVC)
elseif (CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_IS_GNUCC)
set(check_compile_options
  -Wno-implicit-function-declaration
  -Wno-enum-conversion
  -Wno-tautological-constant-out-of-range-compare
  -Wno-invalid-noreturn
  )
endif ()

add_library(check STATIC ${check_source})

set(check_definitions ${check_definitions}
    HAVE_SNPRINTF=1
    HAVE_VSNPRINTF=1)

if (APPLE)
    set(check_definitions ${check_definitions} timer_t=int STRUCT_ITIMERSPEC_DEFINITION_MISSING HAVE_MKSTEMP=1)
endif()    
#set(check_definitions ${check_definitions} HAVE_LIBRT=1)

set_property(TARGET check PROPERTY COMPILE_DEFINITIONS ${check_definitions})

set_property(TARGET check PROPERTY INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_BINARY_DIR}/check
    ${check_base}/include
  )

set_property(TARGET check PROPERTY COMPILE_OPTIONS ${check_compile_options})
