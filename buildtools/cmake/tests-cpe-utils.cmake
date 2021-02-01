file(GLOB test_cpe_utils_source ${cpe_utils_base}/tests/*.c)

add_executable(tests_cpe_utils ${test_cpe_utils_source})

set_property(TARGET tests_cpe_utils PROPERTY INCLUDE_DIRECTORIES
  ${cmocka_base}/include
  ${cpe_pal_base}/include
  ${cpe_utils_base}/include
  ${cpe_utils_base}/src
  ${cpe_utils_base}/test_utils
  ${cpe_utils_base}/tests
  )

set(tests_cpe_utils_libraries
  test_utils_cpe_utils
  cpe_utils cpe_utils cpe_pal cmocka)

if (OS_NAME STREQUAL linux32 OR OS_NAME STREQUAL linux64)
  set(tests_cpe_utils_libraries ${tests_cpe_utils_libraries} m)
endif()

set_property(TARGET tests_cpe_utils PROPERTY LINK_LIBRARIES ${tests_cpe_utils_libraries})

add_test(NAME cpe-utils COMMAND tests_cpe_utils)
