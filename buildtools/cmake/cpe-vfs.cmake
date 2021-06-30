set(cpe_vfs_base ${CMAKE_CURRENT_LIST_DIR}/../../vfs)

file(GLOB cpe_vfs_source ${cpe_vfs_base}/src/*.c)

add_library(cpe_vfs STATIC ${cpe_vfs_source})

set_property(TARGET cpe_vfs PROPERTY INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_LIST_DIR}/../../pal/include
    ${CMAKE_CURRENT_LIST_DIR}/../../utils/include
    ${cpe_vfs_base}/include
  )

target_link_libraries(cpe_vfs INTERFACE cpe_utils)
