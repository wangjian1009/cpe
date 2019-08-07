# {{{ cpe_pal
set(cpe_pal_base ${CMAKE_CURRENT_LIST_DIR}/../../pal)

set(cpe_pal_source
  ${cpe_pal_base}/src/pal_socket.c
  ${cpe_pal_base}/src/pal_error.c
  ${cpe_pal_base}/src/pal_string.c
  ${cpe_pal_base}/src/pal_shm.c
  ${cpe_pal_base}/src/pal_math.c
  ${cpe_pal_base}/src/pal_stdlib.c
  )

if (MSVC)
set(cpe_pal_source
  ${cpe_pal_source}
  ${cpe_pal_base}/src/msvc_time.c
  ${cpe_pal_base}/src/msvc_dlfcn.c
  ${cpe_pal_base}/src/win32_dirent.c
  )
endif ()

add_library(cpe_pal STATIC ${cpe_pal_source})

set_property(TARGET cpe_pal PROPERTY INCLUDE_DIRECTORIES
  ${cpe_pal_base}/include
  )
# }}}
# {{{ cpe_utils
set(cpe_utils_base ${CMAKE_CURRENT_LIST_DIR}/../../utils)

file(GLOB cpe_utils_source ${cpe_utils_base}/src/*.c)

add_library(cpe_utils STATIC ${cpe_utils_source})

set_property(TARGET cpe_utils PROPERTY INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_LIST_DIR}/../../pal/include
    ${cpe_utils_base}/include
  )
# }}}
# {{{ cpe_utils_sock
set(cpe_utils_sock_base ${CMAKE_CURRENT_LIST_DIR}/../../utils_sock)

file(GLOB cpe_utils_sock_source ${cpe_utils_sock_base}/src/*.c)

add_library(cpe_utils_sock STATIC ${cpe_utils_sock_source})

set_property(TARGET cpe_utils_sock PROPERTY INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_LIST_DIR}/../../pal/include
    ${CMAKE_CURRENT_LIST_DIR}/../../utils/include
    ${cpe_utils_sock_base}/include
  )
# }}}
# {{{ cpe_utils_json
set(cpe_utils_json_base ${CMAKE_CURRENT_LIST_DIR}/../../utils_json)

file(GLOB cpe_utils_json_source ${cpe_utils_json_base}/src/*.c)

add_library(cpe_utils_json STATIC ${cpe_utils_json_source})

set_property(TARGET cpe_utils_json PROPERTY INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_LIST_DIR}/../../pal/include
    ${CMAKE_CURRENT_LIST_DIR}/../../utils/include
    ${CMAKE_CURRENT_LIST_DIR}/../../depends/yajl/include
    ${cpe_utils_json_base}/include
  )
# }}}
# {{{ cpe_utils_yaml
set(cpe_utils_yaml_base ${CMAKE_CURRENT_LIST_DIR}/../../utils_yaml)

file(GLOB cpe_utils_yaml_source ${cpe_utils_yaml_base}/src/*.c)

add_library(cpe_utils_yaml STATIC ${cpe_utils_yaml_source})

set_property(TARGET cpe_utils_yaml PROPERTY INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_LIST_DIR}/../../pal/include
    ${CMAKE_CURRENT_LIST_DIR}/../../utils/include
    ${CMAKE_CURRENT_LIST_DIR}/../../depends/yaml/include
    ${cpe_utils_yaml_base}/include
  )
# }}}
