cmake_minimum_required (VERSION 2.8.1)
cmake_policy (VERSION 2.8.1)

set (TARGET_NAME swift_base_test)

aux_source_directory (. SRCS)
add_executable (${TARGET_NAME} ${SRCS})
add_definitions ("-std=c++0x -Wno-deprecated")
target_link_libraries (${TARGET_NAME} swift_base gtest pthread)