CMAKE_MINIMUM_REQUIRED (VERSION 2.8.1)
CMAKE_POLICY (VERSION 2.8.1)

aux_source_directory (base base_SRCS)

ADD_DEFINITIONS ("-std=c++11 -D__STDC_FORMAT_MACROS -Wno-deprecated")
add_library (swift STATIC ${base_SRCS})