cmake_minimum_required (VERSION 2.8.1)
cmake_policy (VERSION 2.8.1)

set (TARGET_NAME disruptor_test)

list (APPEND CMAKE_CXX_FLAGS "-pthread -std=c++0x ${CMAKE_CXX_FLAGS}")
#add_definitions ("-std=c++0x")
add_executable(${TARGET_NAME} test.cpp)
#target_link_libraries(${TARGET_NAME} pthread)

