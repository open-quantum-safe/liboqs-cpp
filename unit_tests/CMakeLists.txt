include(GoogleTest)
set(TARGET_NAME "unit_tests")

#### Link Google Test dynamically if using MSVC
if (MSVC)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif ()

add_subdirectory(libs/googletest-release-1.12.1 EXCLUDE_FROM_ALL SYSTEM)

aux_source_directory(tests TEST_FILES)

add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL tests/main.cpp)
#### Build all tests in ${TEST_FILES}
foreach (FILE ${TEST_FILES})
    target_sources(${TARGET_NAME} PUBLIC ${FILE})
endforeach ()

target_link_libraries(${TARGET_NAME} gmock oqs)

gtest_discover_tests(${TARGET_NAME})
