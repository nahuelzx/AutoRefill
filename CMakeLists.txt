cmake_minimum_required(VERSION 3.16)
project(AutoRefill LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(AutoRefill main.cpp)

if (MSVC)
    target_compile_options(AutoRefill PRIVATE /W44)
    target_link_libraries(AutoRefill PRIVATE user32)
else()
    target_compile_options(AutoRefill PRIVATE -Wall -Wextra -Wpedantic)
    target_link_libraries(AutoRefill PRIVATE user32)
endif()
