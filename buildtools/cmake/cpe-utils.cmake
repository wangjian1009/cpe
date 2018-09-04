set(cpe_utils_base ${CMAKE_CURRENT_LIST_DIR}/../..)

include_directories(
  ${cpe_utils_base}/include
  )

file(GLOB cpe_utils_source ${cpe_utils_base}/src/utils/*.c)

set(cpe_utils_source
  ${cpe_utils_source}
  ${cpe_utils_base}/src/pal/pal_socket.c
  ${cpe_utils_base}/src/pal/pal_error.c
  ${cpe_utils_base}/src/utils_sock/sock_utils.c
  ${cpe_utils_base}/src/utils_sock/getgateway.c
  ${cpe_utils_base}/src/utils_sock/getdnssvraddrs.c
  )

add_library(cpe_utils STATIC ${cpe_utils_source})
