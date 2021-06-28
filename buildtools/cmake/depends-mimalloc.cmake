set(mimalloc_base ${CMAKE_CURRENT_LIST_DIR}/../../depends/mimalloc)

file(GLOB mimalloc_source ${mimalloc_base}/src/*.c)

option(MI_SECURE            "Use full security mitigations (like guard pages, allocation randomization, double-free mitigation, and free-list corruption detection)" OFF)
option(MI_DEBUG_FULL        "Use full internal heap invariant checking in DEBUG mode (expensive)" OFF)
option(MI_PADDING           "Enable padding to detect heap block overflow (used only in DEBUG mode)" ON)
option(MI_OVERRIDE          "Override the standard malloc interface (e.g. define entry points for malloc() etc)" ON)
option(MI_XMALLOC           "Enable abort() call on memory allocation failure by default" OFF)
option(MI_SHOW_ERRORS       "Show error and warning messages by default (only enabled by default in DEBUG mode)" OFF)
option(MI_USE_CXX           "Use the C++ compiler to compile the library (instead of the C compiler)" OFF)
option(MI_SEE_ASM           "Generate assembly files" OFF)
option(MI_INTERPOSE         "Use interpose to override standard malloc on macOS" OFF)
option(MI_OSX_ZONE          "Use malloc zone to override standard malloc on macOS" ON) 
option(MI_LOCAL_DYNAMIC_TLS "Use slightly slower, dlopen-compatible TLS mechanism (Unix)" OFF)
option(MI_BUILD_SHARED      "Build shared library" ON)
option(MI_BUILD_STATIC      "Build static library" ON)
option(MI_BUILD_OBJECT      "Build object library" ON)
option(MI_BUILD_TESTS       "Build test executables" ON)
option(MI_DEBUG_TSAN        "Build with thread sanitizer (needs clang)" OFF)
option(MI_DEBUG_UBSAN       "Build with undefined-behavior sanitizer (needs clang++)" OFF)
option(MI_CHECK_FULL        "Use full internal invariant checking in DEBUG mode (deprecated, use MI_DEBUG_FULL instead)" OFF)
option(MI_INSTALL_TOPLEVEL  "Install directly into $CMAKE_INSTALL_PREFIX instead of PREFIX/lib/mimalloc-version" OFF)

include("${mimalloc_base}/cmake/mimalloc-config-version.cmake")

set(mi_source
    ${mimalloc_base}/src/stats.c
    ${mimalloc_base}/src/random.c
    ${mimalloc_base}/src/os.c
    ${mimalloc_base}/src/bitmap.c
    ${mimalloc_base}/src/arena.c
    ${mimalloc_base}/src/region.c
    ${mimalloc_base}/src/segment.c
    ${mimalloc_base}/src/page.c
    ${mimalloc_base}/src/alloc.c
    ${mimalloc_base}/src/alloc-aligned.c
    ${mimalloc_base}/src/alloc-posix.c
    ${mimalloc_base}/src/heap.c
    ${mimalloc_base}/src/options.c
    ${mimalloc_base}/src/init.c)

# -----------------------------------------------------------------------------
# Process options
# -----------------------------------------------------------------------------

if(CMAKE_C_COMPILER_ID MATCHES "MSVC|Intel")
  set(MI_USE_CXX "ON")
endif()

if(MI_OVERRIDE)
  message(STATUS "Override standard malloc (MI_OVERRIDE=ON)")
  if(APPLE)
    if(MI_OSX_ZONE)
      # use zone's on macOS
      message(STATUS "  Use malloc zone to override malloc (MI_OSX_ZONE=ON)")
      list(APPEND mi_sources src/alloc-override-osx.c)
      list(APPEND mi_defines MI_OSX_ZONE=1)      
    endif()
    if(MI_INTERPOSE)
      # use interpose on macOS
      message(STATUS "  Use interpose to override malloc (MI_INTERPOSE=ON)")
      message(STATUS "  WARNING: interpose does not seem to work reliably on the M1; use -DMI_OSX_ZONE=ON instead")
      list(APPEND mi_defines MI_INTERPOSE)
    endif()
  endif()
endif()

if(MI_SECURE)
  message(STATUS "Set full secure build (MI_SECURE=ON)")
  list(APPEND mi_defines MI_SECURE=4)
endif()

if(MI_SEE_ASM)
  message(STATUS "Generate assembly listings (MI_SEE_ASM=ON)")
  list(APPEND mi_cflags -save-temps)
endif()

if(MI_CHECK_FULL)
  message(STATUS "The MI_CHECK_FULL option is deprecated, use MI_DEBUG_FULL instead")
  set(MI_DEBUG_FULL "ON")
endif()

if(MI_DEBUG_FULL)
  message(STATUS "Set debug level to full internal invariant checking (MI_DEBUG_FULL=ON)")
  list(APPEND mi_defines MI_DEBUG=3)   # full invariant checking
endif()

if(NOT MI_PADDING)
  message(STATUS "Disable padding of heap blocks in debug mode (MI_PADDING=OFF)")
  list(APPEND mi_defines MI_PADDING=0)
endif()

if(MI_XMALLOC)
  message(STATUS "Enable abort() calls on memory allocation failure (MI_XMALLOC=ON)")
  list(APPEND mi_defines MI_XMALLOC=1)
endif()

if(MI_SHOW_ERRORS)
  message(STATUS "Enable printing of error and warning messages by default (MI_SHOW_ERRORS=ON)")
  list(APPEND mi_defines MI_SHOW_ERRORS=1)
endif()

if(MI_DEBUG_TSAN)
  if(CMAKE_C_COMPILER_ID MATCHES "Clang")
    message(STATUS "Build with thread sanitizer (MI_DEBUG_TSAN=ON)")
    list(APPEND mi_defines MI_TSAN=1)
    list(APPEND mi_cflags -fsanitize=thread -g -O1)
    list(APPEND CMAKE_EXE_LINKER_FLAGS -fsanitize=thread)
  else()
    message(WARNING "Can only use thread sanitizer with clang (MI_DEBUG_TSAN=ON but ignored)")    
  endif()  
endif()

if(MI_DEBUG_UBSAN)
  if(CMAKE_BUILD_TYPE MATCHES "Debug")    
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      message(STATUS "Build with undefined-behavior sanitizer (MI_DEBUG_UBSAN=ON)")
      list(APPEND mi_cflags -fsanitize=undefined -g)
      list(APPEND CMAKE_EXE_LINKER_FLAGS -fsanitize=undefined)
      if (NOT MI_USE_CXX)
        message(STATUS "(switch to use C++ due to MI_DEBUG_UBSAN)")
        set(MI_USE_CXX "ON")
      endif()
    else()
      message(WARNING "Can only use undefined-behavior sanitizer with clang++ (MI_DEBUG_UBSAN=ON but ignored)")    
    endif()  
  else()
    message(WARNING "Can only use thread sanitizer with a debug build (CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})")    
  endif()
endif()

if(MI_USE_CXX)
  message(STATUS "Use the C++ compiler to compile (MI_USE_CXX=ON)")
  set_source_files_properties(${mi_sources} PROPERTIES LANGUAGE CXX )
  set_source_files_properties(src/static.c test/test-api.c test/test-stress PROPERTIES LANGUAGE CXX )
  if(CMAKE_CXX_COMPILER_ID MATCHES "AppleClang|Clang")
    list(APPEND mi_cflags -Wno-deprecated)
  endif()
  if(CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    list(APPEND mi_cflags -Kc++)
  endif()
endif()

# Compiler flags
if(CMAKE_C_COMPILER_ID MATCHES "AppleClang|Clang|GNU")
  list(APPEND mi_cflags -Wall -Wextra -Wno-unknown-pragmas -fvisibility=hidden)
  if(CMAKE_C_COMPILER_ID MATCHES "GNU")
    list(APPEND mi_cflags -Wno-invalid-memory-model)
  endif()
  if(CMAKE_C_COMPILER_ID MATCHES "AppleClang|Clang")
    list(APPEND mi_cflags -Wpedantic -Wno-static-in-inline)
  endif()
endif()

if(CMAKE_C_COMPILER_ID MATCHES "Intel")
  list(APPEND mi_cflags -Wall -fvisibility=hidden)
endif()

if(CMAKE_C_COMPILER_ID MATCHES "AppleClang|Clang|GNU|Intel" AND NOT CMAKE_SYSTEM_NAME MATCHES "Haiku")
  if(MI_LOCAL_DYNAMIC_TLS)
    list(APPEND mi_cflags -ftls-model=local-dynamic)
  else()
    list(APPEND mi_cflags -ftls-model=initial-exec)
  endif()
endif()

if (MSVC AND MSVC_VERSION GREATER_EQUAL 1914)
  list(APPEND mi_cflags /Zc:__cplusplus)
endif()

# extra needed libraries
if(WIN32)
  list(APPEND mi_libraries psapi shell32 user32 advapi32 bcrypt)
else()
  if(NOT ${CMAKE_C_COMPILER} MATCHES "android")
    list(APPEND mi_libraries pthread)
    find_library(LIBRT rt)
    if(LIBRT)
      list(APPEND mi_libraries ${LIBRT})
    endif()
  endif()
endif()

# static library
add_library(mimalloc STATIC ${mi_source})
set_property(TARGET mimalloc PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET mimalloc PROPERTY C_STANDARD 11)
set_property(TARGET mimalloc PROPERTY CXX_STANDARD 17)
target_compile_definitions(mimalloc PRIVATE ${mi_defines} MI_STATIC_LIB)
target_compile_options(mimalloc PRIVATE ${mi_cflags})
target_link_libraries(mimalloc PUBLIC ${mi_libraries})
target_include_directories(mimalloc PUBLIC
  $<BUILD_INTERFACE:${mimalloc_base}/include>
  $<INSTALL_INTERFACE:${mi_install_incdir}>
  )

# set_property(TARGET mimalloc PROPERTY INCLUDE_DIRECTORIES
#   ${mimalloc_base}/include
#   )
