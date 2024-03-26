cmake_minimum_required(VERSION 3.15)
set(LIBOQS_CPP_VERSION_NUM 0.10.0)
set(LIBOQS_CPP_VERSION_STR "${LIBOQS_CPP_VERSION_NUM}")
project(
  liboqs-cpp
  VERSION ${LIBOQS_CPP_VERSION_NUM}
  LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
enable_testing()

# liboqs-cpp version number
add_definitions(-DLIBOQS_CPP_VERSION="${LIBOQS_CPP_VERSION_STR}")

# Guard against in-source builds (snippet from Eigen's CMakeLists.txt)
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please instruct CMake to use an\
      out-of-source build, e.g.,
      cmake -B build && cmake --build build
You may need to remove CMakeCache.txt.")
endif()

add_library(liboqs-cpp INTERFACE)
target_include_directories(
  liboqs-cpp INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
                       $<INSTALL_INTERFACE:include/>)
target_compile_definitions(
  liboqs-cpp INTERFACE -DLIBOQS_CPP_VERSION_NUM=${LIBOQS_CPP_VERSION_NUM})
target_compile_definitions(
  liboqs-cpp INTERFACE -DLIBOQS_CPP_VERSION_STR="${LIBOQS_CPP_VERSION_STR}")
target_compile_definitions(
  liboqs-cpp INTERFACE -DLIBOQS_CPP_VERSION="${LIBOQS_CPP_VERSION_STR}")

# Dependencies
include(cmake/liboqs-cpp_dependencies.cmake)

# Unit testing
add_subdirectory(${CMAKE_SOURCE_DIR}/unit_tests/ EXCLUDE_FROM_ALL SYSTEM)

# Enable all warnings for GNU gcc and Clang/AppleClang
if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang" OR ${CMAKE_CXX_COMPILER_ID}
                                               STREQUAL "GNU")
  add_compile_options("-pedantic" "-Wall" "-Wextra" "-Weffc++")
  if(${SANITIZE})
    if(NOT (${CMAKE_CXX_COMPILER_ID} MATCHES "AppleClang"))
      list(APPEND SANITIZE_OPTIONS -fsanitize=undefined)
      add_compile_options("${SANITIZE_OPTIONS}")
      set(CMAKE_EXE_LINKER_FLAGS
          "${CMAKE_EXE_LINKER_FLAGS} ${SANITIZE_OPTIONS}")
    endif()
  endif()
endif()

# Examples
include(cmake/examples.cmake)
# END LOCAL stuff

include_directories(SYSTEM "${LIBOQS_INCLUDE_DIR}")
link_directories("${LIBOQS_LIB_DIR}")

# Installation
set(LIBOQS_CPP_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include/${PROJECT_NAME}")
install(DIRECTORY include/ DESTINATION ${LIBOQS_CPP_INSTALL_DIR})
install(TARGETS liboqs-cpp EXPORT liboqs-cpp_targets)
install(EXPORT liboqs-cpp_targets DESTINATION "lib/cmake/${PROJECT_NAME}")
include(CMakePackageConfigHelpers)
configure_package_config_file(
  "cmake/liboqs-cppConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/liboqs-cppConfig.cmake"
  INSTALL_DESTINATION "lib/cmake/${PROJECT_NAME}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/liboqs-cppConfig.cmake"
        DESTINATION "lib/cmake/${PROJECT_NAME}")
install(FILES "${CMAKE_SOURCE_DIR}/cmake/liboqs-cpp_dependencies.cmake"
        DESTINATION "lib/cmake/${PROJECT_NAME}")

# Uninstall
# https://gitlab.kitware.com/cmake/community/-/wikis/FAQ#can-i-do-make-uninstall-with-cmake
# UNIX/Linux: sudo cmake --build build --target uninstall Windows:    cmake
# --build build --target uninstall
if(NOT TARGET uninstall)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/liboqs-cpp_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" IMMEDIATE @ONLY)
  if(NOT MSVC)
    add_custom_target(
      uninstall
      COMMAND ${CMAKE_COMMAND} -P
              "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
      COMMAND ${CMAKE_COMMAND} -E remove_directory
              "${CMAKE_INSTALL_PREFIX}/lib/cmake/${PROJECT_NAME}"
      COMMAND ${CMAKE_COMMAND} -E remove_directory "${LIBOQS_CPP_INSTALL_DIR}"
      COMMENT "Uninstall liboqs-cpp")
  else()
    add_custom_target(
      uninstall
      COMMAND ${CMAKE_COMMAND} -P
              "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
      COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_INSTALL_PREFIX}"
      COMMENT "Uninstall liboqs-cpp")
  endif()
endif()
