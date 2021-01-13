include(GoogleTest)
set(TARGET_NAME "oqs_cpp_testing")

#### Link Google Test dynamically if using MSVC
if (MSVC)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif ()

add_subdirectory(lib/googletest-release-1.10.0)

aux_source_directory(tests TEST_FILES)

add_executable(${TARGET_NAME} ${TEST_FILES})
target_link_libraries(${TARGET_NAME} gmock oqs)

gtest_discover_tests(${TARGET_NAME})
