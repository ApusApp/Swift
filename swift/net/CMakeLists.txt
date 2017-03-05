cmake_minimum_required (VERSION 2.8.1)
cmake_policy (VERSION 2.8.1)

set (TARGET_NAME swift_net)

aux_source_directory (../base base_SRCS)
aux_source_directory (../base/experimental base_exp_SRCS)
aux_source_directory (. net_SRCS)
aux_source_directory (httpclient net_httpclient_SRCS)
add_library (${TARGET_NAME} ${base_SRCS} ${base_exp_SRCS} ${net_SRCS} ${net_httpclient_SRCS})
target_link_libraries (${TARGET_NAME} pthread glog gflags curl)
set_target_properties (${TARGET_NAME} PROPERTIES COMPILE_FLAGS "-std=c++0x -Wno-deprecated")

install(TARGETS ${TARGET_NAME} DESTINATION lib)