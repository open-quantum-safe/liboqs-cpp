include_directories(SYSTEM ${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
aux_source_directory(./ TEST_FILES)
add_executable(oqs_cpp_testing ${TEST_FILES})
target_link_libraries(oqs_cpp_testing gmock oqs)
