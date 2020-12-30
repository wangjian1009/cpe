file(GLOB test_utils_cpe_utils_source ${cpe_utils_base}/test_utils/*.c)

add_library(test_utils_cpe_utils STATIC ${test_utils_cpe_utils_source})

set_property(TARGET test_utils_cpe_utils PROPERTY INCLUDE_DIRECTORIES
  ${cmocka_base}/include
  ${cpe_pal_base}/include
  ${cpe_utils_base}/include
  )
