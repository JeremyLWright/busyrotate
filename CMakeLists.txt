cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_FLAGS "-Wall -Werror -O3 -std=gnu++0x")

include_directories(.)
add_library(rotateutils STATIC rotateutils.cpp)
add_executable(busyrotate busyrotate.cpp)
target_link_libraries(busyrotate rotateutils)
add_executable(dirdiff dirdiff.cpp)
target_link_libraries(dirdiff rotateutils)

include(ExternalProject)
# Add gtest
ExternalProject_Add(
    googletest
    SVN_REPOSITORY http://googletest.googlecode.com/svn/trunk/
    TIMEOUT 10
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} 
    # Disable install step
    INSTALL_COMMAND ""
)
# Specify include dir
ExternalProject_Get_Property(googletest source_dir)
include_directories(${source_dir}/include)
ExternalProject_Get_Property(googletest binary_dir)
link_directories(${binary_dir})

file(GLOB test_srcs "test/*.cpp")
add_executable(tests ${test_srcs})
add_dependencies(tests googletest)
target_link_libraries(tests
    gtest_main
    gtest
    pthread)

enable_testing()
add_test(busyrotate_tests tests)

