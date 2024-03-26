cmake_minimum_required(VERSION 3.15)
project(standalone)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# If the liboqs-cpp installation path was non-standard, i.e., specified by
#
# cmake -B build -DCMAKE_INSTALL_PREFIX=/path/to/installed/liboqs-cpp
#
# then uncomment the following line and replace the installation path with yours

# set(CMAKE_PREFIX_PATH "/path/to/installed/liboqs-cpp")

find_package(liboqs-cpp REQUIRED)

add_executable(standalone src/main.cpp)
target_link_libraries(standalone PUBLIC liboqs-cpp oqs)
