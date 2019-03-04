cmake_minimum_required(VERSION 3.2)
project(oqs_cpp)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(ADDITIONAL_FLAGS "-pedantic -Wall -Wextra -Weffc++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ADDITIONAL_FLAGS}")

# add all examples
aux_source_directory(examples EXAMPLES)

# path to liboqs include and lib, modify as needed
set(LIBOQS_INCLUDE_DIR "/usr/local/include")
set(LIBOQS_LIB_DIR "/usr/local/lib")

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

# build all executables in ${EXAMPLES}
foreach (FILE ${EXAMPLES})
    get_filename_component(TARGET_NAME ${FILE} NAME_WE)
    add_executable(${TARGET_NAME} ${FILE})
    target_link_libraries(${TARGET_NAME} oqs)
endforeach ()
