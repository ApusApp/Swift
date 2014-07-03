cmake_minimum_required (VERSION 2.8.1)
cmake_policy (VERSION 2.8.1)

set (TARGET_NAME swift_base)

aux_source_directory (. base_SRCS)
add_library (${TARGET_NAME} ${base_SRCS})
target_link_libraries (${TARGET_NAME} pthread glog gflags)
set_target_properties (${TARGET_NAME} PROPERTIES COMPILE_FLAGS "-std=c++0x -Wno-deprecated")

install(TARGETS ${TARGET_NAME} DESTINATION lib)