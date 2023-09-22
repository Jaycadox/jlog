cmake_minimum_required(VERSION 3.23)
project(jlog)

set(CMAKE_CXX_STANDARD 23)
option(BUILD_TESTS "Build tests" ON)

add_subdirectory(vendor/fmt)

add_library(jlog src/library.cpp tests/test_1.cpp src/os.cpp include/jlog/os.h src/string_help.cpp include/jlog/string_help.h src/tap.cpp include/jlog/tap.h)
target_link_libraries(jlog PUBLIC fmt)
target_include_directories(jlog PUBLIC
        include
        ./vendor/fmt/include
        )

if(BUILD_TESTS)
    add_executable(test tests/test_1.cpp)
    target_link_libraries(test PRIVATE jlog)
endif()