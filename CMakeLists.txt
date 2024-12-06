cmake_minimum_required(VERSION 3.13)
project(nmgw)

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
target_include_directories(asio2 SYSTEM BEFORE INTERFACE asio2/3rd asio2/include)
target_compile_definitions(asio2 INTERFACE -DASIO2_USE_SSL=1)

########################################################################################
include_directories(include)
file(GLOB_RECURSE LOGGER_HEADERS include/logger.hpp include/logger/*)
file(GLOB_RECURSE LOGGER_SOURCES src/logger.cpp src/logger/*)
add_library(logger STATIC ${LOGGER_HEADERS} ${LOGGER_SOURCES})

########################################################################################
add_executable(entrypoint src/entrypoint.cpp)
target_link_libraries(entrypoint PRIVATE logger asio2 -lssl -lcrypto)

########################################################################################
add_executable(rendezvous src/rendezvous.cpp)
target_link_libraries(rendezvous PRIVATE logger asio2 -lssl -lcrypto)

########################################################################################
find_package(Qt6 6.2 COMPONENTS Quick REQUIRED)
qt_policy(SET QTP0004 NEW)
qt_add_executable(endpoint
    src/endpoint.cpp
    src/endpoint/guiGate.hpp
    src/endpoint/guiGate.cpp
    src/endpoint/utils.hpp
    src/endpoint/utils.cpp
    src/endpoint/worker.hpp
    src/endpoint/worker.cpp
    src/endpoint/rvzClient.hpp
    src/endpoint/rvzClient.cpp
    src/endpoint/socks5/server.hpp
    src/endpoint/socks5/server.cpp
    src/endpoint/socks5/session.hpp
    src/endpoint/socks5/session.cpp
)
set_target_properties(endpoint PROPERTIES AUTOMOC On)
qt_add_qml_module(endpoint
    URI endpoint
    VERSION 1.0
    NO_RESOURCE_TARGET_PATH
    QML_FILES
        src/endpoint/gui.qml
    RESOURCES
        etc/ca.crt
        etc/client.crt
        etc/client.key
        etc/client.pswd
        etc/dh2048.pem
)
target_link_libraries(endpoint PRIVATE logger asio2 -lssl -lcrypto Qt6::Quick)
