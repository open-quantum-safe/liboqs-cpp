cmake_minimum_required(VERSION 3.2)
project(oqs_cpp)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# enable all warnings for GNU gcc and Clang/AppleClang
if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang" OR ${CMAKE_CXX_COMPILER_ID}
        STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra -Weffc++")
endif ()

# add all examples
aux_source_directory(examples EXAMPLE_FILES)

# path to liboqs include and lib, modify as needed
set(LIBOQS_INCLUDE_DIR "" CACHE PATH "Path to liboqs include folder")
set(LIBOQS_LIB_DIR "" CACHE PATH "Path to liboqs lib folder")

if (LIBOQS_INCLUDE_DIR STREQUAL "")
    message(FATAL_ERROR "Please specify the path to the liboqs include directory by\
    setting the LIBOQS_INCLUDE_DIR cmake flag, i.e.
    cmake -DLIBOQS_INCLUDE_DIR=/path/to/liboqs/include")
elseif (NOT IS_DIRECTORY ${LIBOQS_INCLUDE_DIR})
    message(FATAL_ERROR "Invalid path to the liboqs include directory")
endif ()

if (LIBOQS_LIB_DIR STREQUAL "")
    message(FATAL_ERROR "Please specify the path to the liboqs lib directory by\
    setting the LIBOQS_LIB_DIR cmake flag, i.e.
    cmake -DLIBOQS_LIB_DIR=/path/to/liboqs/include")
elseif (NOT IS_DIRECTORY ${LIBOQS_LIB_DIR})
    message(FATAL_ERROR "Invalid path to the liboqs lib directory")
endif ()

include_directories("include")
include_directories(SYSTEM "${LIBOQS_INCLUDE_DIR}")
link_directories("${LIBOQS_LIB_DIR}")

# default build type
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
            "Choose the type of build, options are: \
         None Debug Release MinSizeRel RelWithDebInfo."
            FORCE)
endif ()

# build all executables in ${EXAMPLE_FILES}
foreach (FILE ${EXAMPLE_FILES})
    get_filename_component(TARGET_NAME ${FILE} NAME_WE)
    add_executable(${TARGET_NAME} ${FILE})
    target_link_libraries(${TARGET_NAME} oqs)
endforeach ()
