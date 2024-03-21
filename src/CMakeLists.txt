set(BANANA_HEADER_LIST "${PROJECT_SOURCE_DIR}/include/banana-lib/lib.hpp")

find_package(OpenCV CONFIG REQUIRED)

add_library(banana-lib lib.cpp ${BANANA_HEADER_LIST})

target_include_directories(
        banana-lib
        PUBLIC "${PROJECT_SOURCE_DIR}/include"
        PUBLIC "${OpenCV_INCLUDE_DIRS}"
)

target_link_libraries(banana-lib
        PUBLIC ${OpenCV_LIBS}
)
