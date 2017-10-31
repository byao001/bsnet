cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 11)

project(Network)
add_subdirectory(src)
include_directories(src)

enable_testing()
add_subdirectory(test)

add_test(NAME MyTest COMMAND Test)

add_executable(demo main.cpp)
target_link_libraries(demo Coro)