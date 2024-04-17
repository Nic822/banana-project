find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

add_executable(banana-lib-test banana-lib-test.cpp)
target_link_libraries(banana-lib-test banana-lib GTest::gtest_main)
gtest_discover_tests(banana-lib-test)

# the resources are used in the tests, so they need to be present in a folder where the test can access them
# with a known location.
file(COPY ${PROJECT_SOURCE_DIR}/resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
