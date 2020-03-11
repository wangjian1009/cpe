set(cpe_zip_base ${CMAKE_CURRENT_LIST_DIR}/../../zip)

file(GLOB cpe_zip_source ${cpe_zip_base}/src/*.c)

add_library(cpe_zip STATIC ${cpe_zip_source})

set_property(TARGET cpe_zip PROPERTY INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_LIST_DIR}/../../pal/include
    ${CMAKE_CURRENT_LIST_DIR}/../../utils/include
    ${CMAKE_CURRENT_LIST_DIR}/../../vfs/include
    ${cpe_zip_base}/include
  )
