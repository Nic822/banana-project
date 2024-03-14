find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

# TODO: rename to something more appropriate when adding the first real tests!
add_executable(example-test example-test.cpp)
target_link_libraries(example-test banana-lib GTest::gtest_main)
gtest_discover_tests(example-test)
