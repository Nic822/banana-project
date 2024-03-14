set(BANANA_HEADER_LIST "${PROJECT_SOURCE_DIR}/include/banana-lib/lib.hpp")

add_library(banana-lib lib.cpp ${BANANA_HEADER_LIST})

target_include_directories(banana-lib PUBLIC "${PROJECT_SOURCE_DIR}/include")
