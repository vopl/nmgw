
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
        set(dstDir "${CMAKE_SOURCE_DIR}/bin")
        get_target_property(srcDir ${target} BINARY_DIR)
        set(srcDir "${srcDir}/android-build")
        add_custom_target(${target}-collect_bin ALL
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${srcDir}/${file}" "${dstDir}/${file}"
            DEPENDS ${target}_make_apk
            COMMENT "Collect APK ${srcDir}/${file} -> ${dstDir}/${file}")
    else()
        # add_custom_target(${target}-collect_bin ALL
        #     COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${target}> "${CMAKE_CURRENT_LIST_DIR}/bin"
        #     DEPENDS ${target})
    endif()
endfunction()
