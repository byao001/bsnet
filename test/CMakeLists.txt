# We need thread support
find_package(Threads REQUIRED)

# Enable ExternalProject CMake module
include(ExternalProject)

# Download and install GoogleTest
ExternalProject_Add(
        gtest
        URL https://github.com/google/googletest/archive/master.zip
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gtest
        # Disable install step
        INSTALL_COMMAND ""
)

# Get GTest source and binary directories from CMake project
ExternalProject_Get_Property(gtest source_dir binary_dir)

# Create a libgtest target to be used as a dependency by test programs
add_library(libgtest IMPORTED STATIC GLOBAL)
add_dependencies(libgtest gtest)

# Set libgtest properties
set_target_properties(libgtest PROPERTIES
        "IMPORTED_LOCATION" "${binary_dir}/googlemock/gtest/libgtest.a"
        "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
        )

# Create a libgmock target to be used as a dependency by test programs
add_library(libgmock IMPORTED STATIC GLOBAL)
add_dependencies(libgmock gtest)

# Set libgmock properties
set_target_properties(libgmock PROPERTIES
        "IMPORTED_LOCATION" "${binary_dir}/googlemock/libgmock.a"
        "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
        )

# I couldn't make it work with INTERFACE_INCLUDE_DIRECTORIES
include_directories("${source_dir}/googletest/include"
        "${source_dir}/googlemock/include")


add_executable(testaddr test_addr.cpp main.cpp)
target_link_libraries(testaddr
        libbsnet
        libgtest
        libgmock
        )
install(TARGETS testaddr DESTINATION bin)

add_executable(testbuffer test_buffer.cpp main.cpp)
target_link_libraries(testbuffer
        libbsnet
        libgtest
        libgmock
        )
install(TARGETS testbuffer DESTINATION bin)

add_executable(testevent test_event.cpp main.cpp)
target_link_libraries(testevent
        libbsnet
        libgtest
        libgmock
        )
install(TARGETS testevent DESTINATION bin)

add_executable(testtcp test_tcp.cpp main.cpp)
target_link_libraries(testtcp
        libbsnet
        libgtest
        libgmock
        )
install(TARGETS testtcp DESTINATION bin)

add_executable(testregistration test_registration.cpp main.cpp)
target_link_libraries(testregistration
libbsnet
libgtest
libgmock)
install(TARGETS testregistration DESTINATION bin)