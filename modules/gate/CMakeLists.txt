
########################################################################################
include_directories(include)
file(GLOB_RECURSE HDR include/*)
file(GLOB_RECURSE SRC src/*)
qt_add_executable(gate ${HDR} ${SRC})
set_target_properties(gate PROPERTIES
    AUTOMOC On
    QT_ANDROID_MIN_SDK_VERSION 24
    QT_ANDROID_TARGET_SDK_VERSION 35
)

set_source_files_properties(${CMAKE_SOURCE_DIR}/etc/ca.crt PROPERTIES QT_RESOURCE_ALIAS etc/ca.crt)
set_source_files_properties(${CMAKE_SOURCE_DIR}/etc/client.crt PROPERTIES QT_RESOURCE_ALIAS etc/client.crt)
set_source_files_properties(${CMAKE_SOURCE_DIR}/etc/client.key PROPERTIES QT_RESOURCE_ALIAS etc/client.key)
set_source_files_properties(${CMAKE_SOURCE_DIR}/etc/client.pswd PROPERTIES QT_RESOURCE_ALIAS etc/client.pswd)
set_source_files_properties(${CMAKE_SOURCE_DIR}/etc/dh2048.pem PROPERTIES QT_RESOURCE_ALIAS etc/dh2048.pem)

qt_add_qml_module(gate
    URI org.nmgw.gate
    VERSION 1.0
    NO_RESOURCE_TARGET_PATH
    QML_FILES
        src/gate/gui.qml
    RESOURCES
        ${CMAKE_SOURCE_DIR}/etc/ca.crt
        ${CMAKE_SOURCE_DIR}/etc/client.crt
        ${CMAKE_SOURCE_DIR}/etc/client.key
        ${CMAKE_SOURCE_DIR}/etc/client.pswd
        ${CMAKE_SOURCE_DIR}/etc/dh2048.pem
)
target_link_libraries(gate PRIVATE common logger asio2 Qt6::Quick)
add_openssl_libraries(gate)
collect_bin(gate)
