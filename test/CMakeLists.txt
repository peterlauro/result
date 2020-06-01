enable_testing()

add_executable(result_test "result_test.cpp")

target_include_directories(result_test PUBLIC
    ${PROJECT_SOURCE_DIR}/doctest
    ${PROJECT_SOURCE_DIR}/inc)

add_test(NAME result_test COMMAND result_test)

