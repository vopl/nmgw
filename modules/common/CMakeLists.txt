
########################################################################################
include_directories(include)
file(GLOB_RECURSE HDR include/*)
file(GLOB_RECURSE SRC src/*)
file(GLOB_RECURSE CMK cmake/*)
add_library(common STATIC ${HDR} ${SRC} ${CMK})
target_include_directories(common INTERFACE include)
target_link_libraries(common PRIVATE asio2 logger)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(common)
add_openssl_libraries(common)

