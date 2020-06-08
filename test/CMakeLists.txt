cmake_minimum_required(VERSION 3.8)

add_executable(result_test "result_test.cpp")

target_include_directories(result_test PUBLIC
    ${PROJECT_SOURCE_DIR}/inc)

target_link_libraries(result_test PUBLIC gtest_main)

add_test(NAME result_test COMMAND result_test)

