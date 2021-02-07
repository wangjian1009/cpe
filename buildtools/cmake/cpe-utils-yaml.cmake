set(cpe_utils_yaml_base ${CMAKE_CURRENT_LIST_DIR}/../../utils_yaml)

file(GLOB cpe_utils_yaml_source ${cpe_utils_yaml_base}/src/*.c)

add_library(cpe_utils_yaml STATIC ${cpe_utils_yaml_source})

set_property(TARGET cpe_utils_yaml PROPERTY INCLUDE_DIRECTORIES
    ${cpe_pal_base}/include
    ${cpe_utils_base}/include
    ${yaml_base}/include
    ${cpe_utils_yaml_base}/include
  )

target_link_libraries(cpe_utils_yaml INTERFACE yaml cpe_utils)
