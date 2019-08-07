set(yaml_base ${CMAKE_CURRENT_LIST_DIR}/../../depends/yaml)

file(GLOB yaml_source ${yaml_base}/src/*.c)

if (MSVC)
elseif (CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_IS_GNUCC)
set(yaml_compile_options
  -Wno-implicit-function-declaration
  -Wno-enum-conversion
  )
endif ()

set(yaml_definitions ${yaml_definitions}
  YAML_VERSION_MAJOR=0
  YAML_VERSION_MINOR=1
  YAML_VERSION_PATCH=4
  YAML_VERSION_STRING="0.1.4"
  )

add_library(yaml STATIC ${yaml_source})

set_property(TARGET yaml PROPERTY COMPILE_DEFINITIONS ${yaml_definitions})

set_property(TARGET yaml PROPERTY INCLUDE_DIRECTORIES
  ${yaml_base}/include
  )

set_property(TARGET yaml PROPERTY COMPILE_OPTIONS ${yaml_compile_options})
