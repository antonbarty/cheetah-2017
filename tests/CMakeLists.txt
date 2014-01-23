

ADD_EXECUTABLE(gtest_basic gtest_basic.cpp gtest/gtest-all.cc)
TARGET_LINK_LIBRARIES(gtest_basic pthread)
include_directories(.)
ADD_TEST(GTest_Basic gtest_basic)
FIND_PACKAGE(PythonInterp)
ADD_TEST(Basic ${PYTHON_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/psana_basic.py" "${CMAKE_CURRENT_SOURCE_DIR}")
ADD_TEST(Basic_Memcheck ${PYTHON_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/psana_basic_valgrind.py" "${CMAKE_CURRENT_SOURCE_DIR}")

set_property(TEST Basic_Memcheck APPEND PROPERTY DEPENDS Basic)