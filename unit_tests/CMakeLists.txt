cmake_minimum_required(VERSION 3.2)
project(oqs_cpp_testing)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# path to liboqs include and lib, modify as needed
set(LIBOQS_INCLUDE_DIR "/usr/local/include")
set(LIBOQS_LIB_DIR "/usr/local/lib")

include_directories("../include")
include_directories(SYSTEM "${LIBOQS_INCLUDE_DIR}")
link_directories("${LIBOQS_LIB_DIR}")

# Google Test
add_subdirectory(lib/gtest-1.8.0)

# enable all warnings only after compiling Google Test
set(ADDITIONAL_FLAGS "-pedantic -Wall -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ADDITIONAL_FLAGS}")

# unit tests
add_subdirectory(tests)

# default build type
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
            "Choose the type of build, options are: \
         None Debug Release MinSizeRel RelWithDebInfo."
            FORCE)
endif ()
