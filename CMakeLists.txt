cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 14)

project(bsnet)

add_subdirectory(src)
include_directories(src)

enable_testing()
add_subdirectory(test)

add_test(AddrTest test/testaddr)
add_test(BufferTest test/testbuffer)
add_test(EventTest test/testevent)
add_test(TcpTest test/testtcp)
add_test(RegistrationTest test/testregistration)
add_test(TokenTest test/testtoken)

add_executable(demo main.cpp)
target_link_libraries(demo libbsnet)
