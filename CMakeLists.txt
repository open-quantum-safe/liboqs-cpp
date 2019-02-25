cmake_minimum_required(VERSION 3.2)
project(oqs_cpp)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(LIBOQS_INCLUDE_DIR "$ENV{HOME}/liboqs/include")
set(LIBOQS_LIB_DIR "$ENV{HOME}/liboqs/.libs")
include_directories(SYSTEM "${LIBOQS_INCLUDE_DIR}")
include_directories("include")
link_directories("${LIBOQS_LIB_DIR}")

add_executable(oqs_cpp examples/example.cpp)
target_link_libraries(oqs_cpp -loqs)
