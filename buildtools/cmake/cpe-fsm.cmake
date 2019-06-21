set(cpe_fsm_base ${CMAKE_CURRENT_LIST_DIR}/../../fsm)

file(GLOB cpe_fsm_source ${cpe_fsm_base}/src/*.c)

add_library(cpe_fsm STATIC ${cpe_fsm_source})

set_property(TARGET cpe_fsm PROPERTY INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_LIST_DIR}/../../pal/include
    ${CMAKE_CURRENT_LIST_DIR}/../../cpe/include
    ${cpe_fsm_base}/include
    )
