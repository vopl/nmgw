
########################################################################################
include_directories(include)
file(GLOB_RECURSE HDR include/*)
file(GLOB_RECURSE SRC src/*)
add_executable(rendezvous ${HDR} ${SRC})
target_link_libraries(rendezvous PRIVATE common logger asio2)
add_openssl_libraries(rendezvous)
collect_bin(rendezvous)