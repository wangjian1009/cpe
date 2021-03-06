# -*- origami-fold-style: triple-braces -*-

# {{{ cpe_pal
set(cpe_pal_base ${CMAKE_CURRENT_LIST_DIR}/../../pal)

file(GLOB cpe_pal_source ${cpe_pal_base}/src/pal_*.c)

if (OS_NAME STREQUAL "vc" OR OS_NAME STREQUAL "mingw")
  file(GLOB cpe_pal_source_win32 ${cpe_pal_base}/src/win32_*.c)
  file(GLOB cpe_pal_source_msvc ${cpe_pal_base}/src/msvc_*.c)
  list(APPEND cpe_pal_source ${cpe_pal_source_win32} ${cpe_pal_source_msvc})
endif ()

add_library(cpe_pal STATIC ${cpe_pal_source})

set_property(TARGET cpe_pal PROPERTY INCLUDE_DIRECTORIES
  ${cpe_pal_base}/include
  )

if (OS_NAME STREQUAL "mingw")
  target_link_libraries(cpe_pal INTERFACE unixem ws2_32)
endif()

# }}}
# {{{ cpe_utils
set(cpe_utils_base ${CMAKE_CURRENT_LIST_DIR}/../../utils)

file(GLOB cpe_utils_source ${cpe_utils_base}/src/*.c)

add_library(cpe_utils STATIC ${cpe_utils_source})

set_property(TARGET cpe_utils PROPERTY INCLUDE_DIRECTORIES
    ${cpe_pal_base}/include
    ${cpe_utils_base}/include
    )

add_dependencies(cpe_utils cpe_pal)

set(cpe_utils_libraries cpe_pal)

if (OS_NAME MATCHES "linux")
  set(cpe_utils_libraries ${cpe_utils_libraries} m)
endif()

target_link_libraries(cpe_utils INTERFACE ${cpe_utils_libraries})
  
# }}}
# {{{ cpe_utils_sock
set(cpe_utils_sock_base ${CMAKE_CURRENT_LIST_DIR}/../../utils_sock)

file(GLOB cpe_utils_sock_source ${cpe_utils_sock_base}/src/*.c)

add_library(cpe_utils_sock STATIC ${cpe_utils_sock_source})

set_property(TARGET cpe_utils_sock PROPERTY INCLUDE_DIRECTORIES
    ${cpe_pal_base}/include
    ${cpe_utils_base}/include
    ${cpe_utils_sock_base}/include
    )

set(cpe_utils_sock_libraries cpe_utils)

if(WIN32)
  list(APPEND cpe_utils_sock_libraries IPHLPAPI)
endif()

if (OS_NAME MATCHES "mac")
  list(APPEND cpe_utils_sock_libraries resolv)
endif()

target_link_libraries(cpe_utils_sock INTERFACE ${cpe_utils_sock_libraries})

# }}}
