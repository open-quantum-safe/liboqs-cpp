cmake_minimum_required(VERSION 3.2)
project(oqs_cpp)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(ADDITIONAL_FLAGS "-pedantic -Wall -Wextra -Weffc++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ADDITIONAL_FLAGS}")

# path to liboqs include and lib, modify as needed
set(LIBOQS_INCLUDE_DIR "/usr/local/include")
set(LIBOQS_LIB_DIR "/usr/local/lib")

include_directories("include")
include_directories(SYSTEM "${LIBOQS_INCLUDE_DIR}")
link_directories("${LIBOQS_LIB_DIR}")



add_executable(oqs_cpp_sig examples/sigs.cpp)
add_executable(oqs_cpp_kem examples/kems.cpp)
target_link_libraries(oqs_cpp_sig oqs)
target_link_libraries(oqs_cpp_kem oqs)
