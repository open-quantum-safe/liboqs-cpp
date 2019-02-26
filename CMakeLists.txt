cmake_minimum_required(VERSION 3.2)
project(oqs_cpp)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories("include")
# modify as needed
set(LIBOQS_INCLUDE_DIR "/usr/local/include")
include_directories(SYSTEM "${LIBOQS_INCLUDE_DIR}")

# modify as needed
set(LIBOQS_LIB_DIR "/usr/local/lib")
link_directories("${LIBOQS_LIB_DIR}")

add_executable(oqs_cpp examples/sigs.cpp)
target_link_libraries(oqs_cpp -loqs)
