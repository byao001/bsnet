set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost COMPONENTS unit_test_framework REQUIRED)

if(Boost_FOUND)
  include_directories(${TEST_SOURCE_DIR}/src, ${Boost_INCLUDE_DIRS})
  add_executable(AddrTest test_addr.cpp)
  target_link_libraries(AddrTest Coro ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
  add_executable(BufferTest test_buffer.cpp)
  target_link_libraries(BufferTest Coro ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
  add_definitions(-DBOOST_TEST_DYN_LINK)
endif()
  

