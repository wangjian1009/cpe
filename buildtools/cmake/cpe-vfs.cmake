set(cpe_vfs_base ${CMAKE_CURRENT_LIST_DIR}/../..)

include_directories(
  ${cpe_vfs_base}/include
  )

file(GLOB cpe_vfs_source ${cpe_vfs_base}/src/vfs/*.c)

add_library(cpe_vfs STATIC ${cpe_vfs_source})
