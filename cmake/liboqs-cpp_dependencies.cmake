# liboqs-cpp additional dependencies Do not modify unless you know what you're
# doing

# Liboqs Path to liboqs include and lib, modify as needed
if(NOT WIN32)
  set(LIBOQS_INCLUDE_DIR
      "/usr/local/include"
      CACHE PATH "Path to liboqs include directory")
  set(LIBOQS_LIB_DIR
      "/usr/local/lib"
      CACHE PATH "Path to liboqs lib directory")
else()
  # Increase the stack size to 8MB on Windows
  if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:8388608")
  elseif(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang" OR ${CMAKE_CXX_COMPILER_ID}
                                                     STREQUAL "GNU")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--stack,8388608")
  endif()
  set(LIBOQS_INCLUDE_DIR
      ""
      CACHE PATH "Path to liboqs include directory")
  set(LIBOQS_LIB_DIR
      ""
      CACHE PATH "Path to liboqs lib directory")
endif()

if(LIBOQS_INCLUDE_DIR STREQUAL "")
  message(
    FATAL_ERROR
      "Please specify the path to the liboqs include directory\
    by setting the LIBOQS_INCLUDE_DIR CMake flag, i.e.
    cmake -DLIBOQS_INCLUDE_DIR=/path/to/liboqs/include")
elseif(NOT IS_DIRECTORY ${LIBOQS_INCLUDE_DIR})
  message(FATAL_ERROR "Invalid path to the liboqs include directory")
endif()

if(LIBOQS_LIB_DIR STREQUAL "")
  message(
    FATAL_ERROR
      "Please specify the path to the liboqs lib directory\
    by setting the LIBOQS_LIB_DIR CMake flag, i.e.
    cmake -DLIBOQS_LIB_DIR=/path/to/liboqs/lib")
elseif(NOT IS_DIRECTORY ${LIBOQS_LIB_DIR})
  message(FATAL_ERROR "Invalid path to the liboqs lib directory")
endif()

include_directories(SYSTEM "${LIBOQS_INCLUDE_DIR}")
link_directories("${LIBOQS_LIB_DIR}")

# Default build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE
      Release
      CACHE STRING "Choose the type of build, options are: \
         None Debug Release MinSizeRel RelWithDebInfo." FORCE)
endif()
