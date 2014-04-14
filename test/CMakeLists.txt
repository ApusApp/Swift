cmake_minimum_required (VERSION 2.8.1)
cmake_policy (VERSION 2.8.1)

#project name
project (LinkedHashMap CXX)
set (TARGET_NAME LinkedHashMap)

if(NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE "Debug")
endif()

set(SRCS test_LinkedHashMap.cpp)
include_directories (../)
link_directories ("../lib/")
add_executable (${TARGET_NAME} ${SRCS})
add_definitions ("-std=c++11 -Wno-deprecated")
target_link_libraries (${TARGET_NAME} swift gtest pthread)