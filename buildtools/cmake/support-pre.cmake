message(STATUS "CMAKE_VERSION=${CMAKE_VERSION}")
message(STATUS "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}")
message(STATUS "CMAKE_C_COMPILER_ID=${CMAKE_C_COMPILER_ID}")

if (CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "GNU")
  if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND SANITIZE)
    add_compile_options(-fsanitize=${SANITIZE})
    add_link_options("-fsanitize=${SANITIZE}")
    message(STATUS "optional:-fsanitize=${SANITIZE}")
  endif ()

  if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND GCOV)
    add_compile_options(-ftest-coverage)
    add_link_options("-fprofile-arcs")
    message(STATUS "optional:-fprofile-arcs -ftest-coverage")
  endif ()

  if (GPROF)
    add_compile_options(-pg)
    add_link_options("-pg")
    message(STATUS "optional:-pg")
  endif()
  
  add_compile_options(-fno-omit-frame-pointer)
  message(STATUS "optional:-fno-omit-frame-pointer")

  add_compile_options(-g2 -ggdb)
  message(STATUS "optional:-g2 -ggdb")

endif()

if (BUILD_TESTS)
  add_definitions(-DCPE_UNIT_TEST=1)
endif()

if (MINGW)
  set(OS_NAME mingw)
  add_definitions(-DCPE_OS_MINGW=1)
elseif (WIN32)
  set(GUI_TYPE WIN32)
  
  if (MSVC)
    add_definitions(-DCPE_OS_WIN=1)
    set(OS_NAME vc)
  else()
    add_definitions(-DCPE_OS_MINGW=1)
    set(OS_NAME mingw)
  endif()
elseif (ANDROID)
  set(OS_NAME android)
  add_definitions(-DCPE_OS_ANDROID=1)
elseif (APPLE)
  if (SDK_NAME STREQUAL "iphoneos" OR SDK_NAME STREQUAL "iphonesimulator")
    add_definitions(-DCPE_OS_IOS=1)
    set(OS_NAME ios)
  else()
    add_definitions(-DCPE_OS_MAC=1)
    set(OS_NAME mac)
  endif()
elseif (CYGWIN)
  add_definitions(-DCPE_OS_CYGWIN=1)
  if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    set(OS_NAME cygwin64)
  else ()
    set(OS_NAME cygwin)
  endif ()
else()
  add_definitions(-DCPE_OS_LINUX=1)
  if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    set(OS_NAME linux64)
  else ()
    set(OS_NAME linux32)
  endif ()
endif()

message(STATUS "OS_NAME=${OS_NAME}")

if (MSVC)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4819 /wd4103 /wd4101 /wd4715 /wd4996 /wd4018")
endif ()

if (DUMP)
get_cmake_property(_variableNames VARIABLES)
foreach (_variableName ${_variableNames})
    message(STATUS ${_variableName} = ${${_variableName}})
endforeach()
endif ()
