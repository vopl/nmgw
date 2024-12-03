cmake_minimum_required(VERSION 3.13)
project(foprox)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

########################################################################################
file(GLOB_RECURSE ASIO2_HEADERS
    asio2/3rd/**/*.hpp
    asio2/3rd/**/*.h
    asio2/include/**/*.hpp
    asio2/include/**/*.h)
add_library(asio2 STATIC ${ASIO2_HEADERS})
set_target_properties(asio2 PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(asio2 INTERFACE asio2/3rd asio2/include)
target_compile_definitions(asio2 INTERFACE -DASIO2_USE_SSL=1)

########################################################################################
include_directories(include)
file(GLOB_RECURSE LOGGER_HEADERS include/logger.hpp include/logger/*)
file(GLOB_RECURSE LOGGER_SOURCES src/logger.cpp src/logger/*)
add_library(logger STATIC ${LOGGER_HEADERS} ${LOGGER_SOURCES})

########################################################################################
add_executable(entrypoint src/entrypoint.cpp)
target_link_libraries(entrypoint asio2)

########################################################################################
add_executable(rendezvous src/rendezvous.cpp)
target_link_libraries(rendezvous logger asio2 -lssl -lcrypto)

########################################################################################
add_executable(endpoint src/endpoint.cpp)
target_link_libraries(endpoint asio2)
