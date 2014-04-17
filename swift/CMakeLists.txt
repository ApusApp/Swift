CMAKE_MINIMUM_REQUIRED (VERSION 2.8.1)
CMAKE_POLICY (VERSION 2.8.1)

aux_source_directory (base base_SRCS)
aux_source_directory (disruptor disruptor_SRCS)

add_definitions ("-std=c++0x -D__STDC_FORMAT_MACROS -Wno-deprecated")
add_library (swift STATIC ${base_SRCS} ${disruptor_SRCS})