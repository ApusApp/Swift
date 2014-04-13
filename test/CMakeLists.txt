CMAKE_MINIMUM_REQUIRED (VERSION 2.8.1)
CMAKE_POLICY (VERSION 2.8.1)

#project name
PROJECT (test CXX)
SET (TARGET_NAME test)

if(NOT CMAKE_BUILD_TYPE)
  SET (CMAKE_BUILD_TYPE "Debug")
endif()

INCLUDE_DIRECTORIES (../)
link_directories ("../lib/")
add_executable(${TARGET_NAME} main.cpp)
ADD_DEFINITIONS ("-std=c++11 -Wno-deprecated")
target_link_libraries (${TARGET_NAME} swift)