

ADD_EXECUTABLE(gtest_basic gtest_basic.cpp gtest/gtest-all.cc)
TARGET_LINK_LIBRARIES(gtest_basic pthread)
include_directories(.)
ADD_TEST(GTest_Basic gtest_basic)

