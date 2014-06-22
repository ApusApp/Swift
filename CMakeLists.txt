cmake_minimum_required (VERSION 2.8.1)
cmake_policy (VERSION 2.8.1)

project (swift CXX)

if(NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE "Debug")
endif()

set (CMAKE_CXX_COMPILER "g++")
set (CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb -D__STDC_FORMAT_MACROS -fprofile-arcs -ftest-coverage -fPIC")
set (CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O2 -finline-limit=1000 -DNDEBUG -D__STDC_FORMAT_MACROS")

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set (LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

string (TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)

include_directories (.)

add_subdirectory (swift/base)
#add_subdirectory (swift/disruptor)

add_subdirectory (test/base)
add_subdirectory (test/disruptor)
