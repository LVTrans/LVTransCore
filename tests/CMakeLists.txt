find_package(GTest CONFIG REQUIRED)

set(TEST_SOURCES
  test_pipe.cpp
  test_multi_elements.cpp
)

add_executable(tests ${TEST_SOURCES})


target_compile_definitions(tests PRIVATE
    TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/test_data/"
)

target_link_libraries(tests
    PRIVATE
        lvtrans_core
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
)

target_compile_options(tests PRIVATE -Wall -Wextra -Wpedantic)

include(GoogleTest)
gtest_discover_tests(tests)
