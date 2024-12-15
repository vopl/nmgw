cmake_minimum_required(VERSION 3.13)
project(nmgw VERSION 0.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE_UC)
add_compile_definitions(-DBUILD_TYPE_${BUILD_TYPE_UC}=1)
list(APPEND CMAKE_BUILD_RPATH "\$ORIGIN" "\$ORIGIN/../lib")


########################################################################################
find_package(Qt6 6.4 COMPONENTS Core Quick REQUIRED)
# qt_policy(SET QTP0004 NEW)

########################################################################################
add_subdirectory(modules/asio2)
add_subdirectory(modules/logger)
add_subdirectory(modules/common)
include(modules/common/cmake/common.cmake)

add_subdirectory(modules/gate)
add_subdirectory(modules/entry)
if (NOT ANDROID)
    add_subdirectory(modules/rendezvous)
endif()
