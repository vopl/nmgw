cmake_minimum_required(VERSION 3.13)
project(nmgw VERSION 0.0.0)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE_UC)
add_compile_definitions(-DBUILD_TYPE_${BUILD_TYPE_UC}=1)
list(APPEND CMAKE_BUILD_RPATH "\$ORIGIN" "\$ORIGIN/../lib")

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
function(add_openssl_libraries target)
    if (ANDROID)
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            set(ssl_root_path ${CMAKE_ANDROID_NDK}/../../android_openssl/no-asm)
        else()
            set(ssl_root_path ${CMAKE_ANDROID_NDK}/../../android_openssl)
        endif()

        set(ssl_include_path ${ssl_root_path}/ssl_3/include)
        set(ssl_lib_path ${ssl_root_path}/ssl_3/${CMAKE_ANDROID_ARCH_ABI})
        set(ssl_lib
            ${ssl_lib_path}/libcrypto_3.so
            ${ssl_lib_path}/libssl_3.so)

        target_include_directories(${target} PRIVATE ${ssl_include_path})
        target_link_libraries(${target} PRIVATE ${ssl_lib})

        set_target_properties(${target} PROPERTIES QT_ANDROID_EXTRA_LIBS "${ssl_lib}")
    else()
        target_link_libraries(${target} PRIVATE -lssl -lcrypto)
    endif()
endfunction()

########################################################################################
function(collect_bin target)
    if (ANDROID)
        set(file "${target}.apk")
        set(dstDir "${CMAKE_CURRENT_LIST_DIR}/bin")
        get_target_property(srcDir ${target} BINARY_DIR)
        set(srcDir "${srcDir}/android-build")
        add_custom_target(${target}-collect_bin ALL
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${srcDir}/${file}" "${dstDir}/${file}"
            DEPENDS "${srcDir}/${file}")
    else()
        add_custom_target(${target}-collect_bin ALL
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${target}> "${CMAKE_CURRENT_LIST_DIR}/bin"
            DEPENDS ${target})
    endif()
endfunction()

########################################################################################
find_package(Qt6 6.4 COMPONENTS Core Quick REQUIRED)
# qt_policy(SET QTP0004 NEW)

########################################################################################
include_directories(include)
file(GLOB_RECURSE LOGGER_HEADERS include/logger.hpp include/logger/*)
file(GLOB_RECURSE LOGGER_SOURCES src/logger.cpp src/logger/*)
add_library(logger STATIC ${LOGGER_HEADERS} ${LOGGER_SOURCES})

########################################################################################
qt_add_executable(entry
    src/utils.hpp
    src/utils.cpp
    src/common.hpp
    src/common.cpp
    src/entry.cpp
    src/entry/guiTalk.hpp
    src/entry/guiTalk.cpp
    src/entry/rvzClient.hpp
    src/entry/rvzClient.cpp
    src/entry/socks5/server.hpp
    src/entry/socks5/server.cpp
    src/entry/socks5/session.hpp
    src/entry/socks5/session.cpp
)
set_target_properties(entry PROPERTIES
    AUTOMOC On
    QT_ANDROID_MIN_SDK_VERSION 26
    QT_ANDROID_TARGET_SDK_VERSION 26
)
qt_add_qml_module(entry
    URI entry
    VERSION 1.0
    NO_RESOURCE_TARGET_PATH
    QML_FILES
        src/entry/gui.qml
    RESOURCES
        etc/ca.crt
        etc/client.crt
        etc/client.key
        etc/client.pswd
        etc/dh2048.pem
)
target_link_libraries(entry PRIVATE logger asio2 Qt6::Quick)
add_openssl_libraries(entry)
collect_bin(entry)

########################################################################################
if (NOT ANDROID)
    add_executable(rendezvous
        src/utils.hpp
        src/utils.cpp
        src/common.hpp
        src/common.cpp
        src/rendezvous.cpp
        src/rendezvous/server.hpp
        src/rendezvous/server.cpp
    )
    target_link_libraries(rendezvous PRIVATE logger asio2 pthread)
    add_openssl_libraries(rendezvous)
    collect_bin(rendezvous)
endif()

########################################################################################
qt_add_executable(gate
    src/utils.hpp
    src/utils.cpp
    src/common.hpp
    src/common.cpp
    src/gate.cpp
    src/gate/guiTalk.hpp
    src/gate/guiTalk.cpp
    src/gate/rvzClient.hpp
    src/gate/rvzClient.cpp
    src/gate/socks5/server.hpp
    src/gate/socks5/server.cpp
)
set_target_properties(gate PROPERTIES
    AUTOMOC On
    QT_ANDROID_MIN_SDK_VERSION 26
    QT_ANDROID_TARGET_SDK_VERSION 26
)
qt_add_qml_module(gate
    URI gate
    VERSION 1.0
    NO_RESOURCE_TARGET_PATH
    QML_FILES
        src/gate/gui.qml
    RESOURCES
        etc/ca.crt
        etc/client.crt
        etc/client.key
        etc/client.pswd
        etc/dh2048.pem
)
target_link_libraries(gate PRIVATE logger asio2 Qt6::Quick)
add_openssl_libraries(gate)
collect_bin(gate)

