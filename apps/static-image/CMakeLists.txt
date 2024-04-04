add_executable(banana-app-static main.cpp)

target_link_libraries(banana-app-static
        PRIVATE banana-lib
)
