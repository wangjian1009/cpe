file(GLOB test_utils_cpe_vfs_source ${cpe_vfs_base}/test_utils/*.c)

add_library(test_utils_cpe_vfs STATIC ${test_utils_cpe_vfs_source})

set_property(TARGET test_utils_cpe_vfs PROPERTY INCLUDE_DIRECTORIES
  ${cmocka_base}/include
  ${cpe_pal_base}/include
  ${cpe_utils_base}/include
  ${cpe_utils_base}/test_utils
  ${cpe_vfs_base}/include
  )

target_link_libraries(test_utils_cpe_vfs INTERFACE test_utils_cpe_utils cpe_vfs)
