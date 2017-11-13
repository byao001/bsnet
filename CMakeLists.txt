cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 17)

project(bsnet)

add_subdirectory(src)
include_directories(src)

enable_testing()
add_subdirectory(test)

#add_test(AddrTest test/AddrTest)
#add_test(BufferTest test/BufferTest)

add_executable(demo main.cpp)
target_link_libraries(demo libbsnet)
