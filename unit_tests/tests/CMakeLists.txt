INCLUDE_DIRECTORIES(SYSTEM ${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
ADD_EXECUTABLE(oqs_cpp_testing
        kems.cpp
        sigs.cpp
        testing_main.cpp)
TARGET_LINK_LIBRARIES(oqs_cpp_testing gmock oqs)
