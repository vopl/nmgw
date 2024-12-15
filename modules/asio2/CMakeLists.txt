
########################################################################################
file(GLOB_RECURSE HDR
    repo/3rd/**/*.hpp
    repo/3rd/**/*.h
    repo/include/**/*.hpp
    repo/include/**/*.h)
add_library(asio2 STATIC ${HDR})
set_target_properties(asio2 PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(asio2 SYSTEM BEFORE INTERFACE repo/3rd repo/include)
target_compile_definitions(asio2 INTERFACE -DASIO2_USE_SSL=1)
