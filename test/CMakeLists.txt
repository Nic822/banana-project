find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

find_package(Ceres CONFIG REQUIRED)

add_executable(polyfit-test polyfit-test.cpp)
target_include_directories(polyfit-test PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(polyfit-test Ceres::ceres GTest::gtest_main)
gtest_discover_tests(polyfit-test)

add_executable(banana-lib-test banana-lib-test.cpp)
target_link_libraries(banana-lib-test banana-lib GTest::gtest_main)
gtest_discover_tests(banana-lib-test)

# the resources are used in the tests, so they need to be present in a folder where the test can access them
# with a known location.
file(COPY ${PROJECT_SOURCE_DIR}/resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
