cmake_minimum_required(VERSION 3.27)
project(
    banana_project
    DESCRIPTION "A tool to measure some data from photos of bananas"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest) # this automatically enables testing as well

add_subdirectory(src)
add_subdirectory(apps)
add_subdirectory(test)
