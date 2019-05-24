set(cpe_fsm_base ${CMAKE_CURRENT_LIST_DIR}/../..)

file(GLOB cpe_fsm_source ${cpe_fsm_base}/src/fsm/*.c)

add_library(cpe_fsm STATIC ${cpe_fsm_source})

set_property(TARGET cpe_fsm PROPERTY INCLUDE_DIRECTORIES
  ${cpe_fsm_base}/include
  )
