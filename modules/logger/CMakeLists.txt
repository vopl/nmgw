
########################################################################################
include_directories(include)
file(GLOB_RECURSE HDR include/*)
file(GLOB_RECURSE SRC src/*)
add_library(logger STATIC ${HDR} ${SRC})
target_include_directories(logger INTERFACE include)

