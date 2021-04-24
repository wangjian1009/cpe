set(cpe_utils_json_base ${CMAKE_CURRENT_LIST_DIR}/../../utils_json)

file(GLOB cpe_utils_json_source ${cpe_utils_json_base}/src/*.c)

add_library(cpe_utils_json STATIC ${cpe_utils_json_source})

set_property(TARGET cpe_utils_json PROPERTY INCLUDE_DIRECTORIES
    ${cpe_pal_base}/include
    ${cpe_utils_base}/include
    ${yajl_base}/include
    ${cpe_utils_json_base}/include
    )

target_link_libraries(cpe_utils_json INTERFACE yajl cpe_utils)
  
