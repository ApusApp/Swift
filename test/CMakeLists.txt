cmake_minimum_required (VERSION 2.8.1)
cmake_policy (VERSION 2.8.1)

#project name
project (test CXX)
set (TARGET_NAME test)

if(NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE "Debug")
endif()

include_directories (../)
link_directories ("../lib/")
add_executable (${TARGET_NAME} main.cpp)
add_definitions ("-std=c++11 -Wno-deprecated")
target_link_libraries (${TARGET_NAME} swift gtest pthread)