find_package(Boost COMPONENTS unit_test_framework REQUIRED)
include_directories(${TEST_SOURCE_DIR}/src
                    ${Boost_INCLUDE_DIRS}
                    )
add_definitions(-DBOOST_TEST_DYN_LINK)
include_directories(../src)

add_executable(Test test_addr.cpp test_buffer.cpp)
target_link_libraries(Test Coro ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
