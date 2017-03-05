cmake_minimum_required (VERSION 2.8.1)
cmake_policy (VERSION 2.8.1)

set (TARGET_NAME swiftclient_test)

include_directories(../.. Inc)
include_directories(../../.. swift_Inc)
aux_source_directory (. SRCS)
aux_source_directory (../../swiftclient swiftclient_SRCS)
add_executable (${TARGET_NAME} ${SRCS} ${Inc} ${swiftclient_SRCS} ${swift_Inc})
add_definitions ("-std=c++0x -Wno-deprecated -D_GLIBCXX_USE_NANOSLEEP")
target_link_libraries (${TARGET_NAME} swift_net pthread gflags gtest)