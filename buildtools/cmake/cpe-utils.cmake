set(cpe_utils_base ${CMAKE_CURRENT_LIST_DIR}/../..)

file(GLOB cpe_utils_source ${cpe_utils_base}/src/utils/*.c)

set(cpe_utils_source
  ${cpe_utils_source}
  ${cpe_utils_base}/src/pal/pal_socket.c
  ${cpe_utils_base}/src/pal/pal_error.c
  ${cpe_utils_base}/src/pal/pal_string.c
  ${cpe_utils_base}/src/pal/pal_shm.c
  ${cpe_utils_base}/src/pal/pal_math.c
  ${cpe_utils_base}/src/pal/pal_stdlib.c
  ${cpe_utils_base}/src/utils_sock/sock_utils.c
  ${cpe_utils_base}/src/utils_sock/getgateway.c
  ${cpe_utils_base}/src/utils_sock/getdnssvraddrs.c
  )

if (MSVC)
set(cpe_utils_source
  ${cpe_utils_source}
  ${cpe_utils_base}/src/pal/msvc_time.c
  ${cpe_utils_base}/src/pal/msvc_dlfcn.c
  ${cpe_utils_base}/src/pal/win32_dirent.c
  )
endif ()

add_library(cpe_utils STATIC ${cpe_utils_source})

set_property(TARGET cpe_utils PROPERTY INCLUDE_DIRECTORIES
  ${cpe_utils_base}/include
  )
