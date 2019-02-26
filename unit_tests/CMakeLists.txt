cmake_minimum_required(VERSION 3.2)
project(oqs_cpp_testing)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ADDITIONAL_FLAGS}")

# path to liboqs include and lib, modify as needed
set(LIBOQS_INCLUDE_DIR "/usr/local/include")
set(LIBOQS_LIB_DIR "/usr/local/lib")

include_directories("../include")
include_directories(SYSTEM "${LIBOQS_INCLUDE_DIR}")
link_directories("${LIBOQS_LIB_DIR}")

#### GoogleTest
add_subdirectory(lib/gtest-1.8.0)

#### Unit tests
add_subdirectory(tests)

#### Enable all warnings after compiling gtest
set(ADDITIONAL_FLAGS "-pedantic -Wall -Wextra -Weffc++")
