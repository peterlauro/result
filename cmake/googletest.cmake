macro(fetch_googletest download_module_path download_root)
    set(GOOGLETEST_DOWNLOAD_ROOT ${download_root})
    configure_file(
        ${download_module_path}/googletest-download.cmake
        ${download_root}/CMakeLists.txt
        @ONLY
        )
    unset(GOOGLETEST_DOWNLOAD_ROOT)

    execute_process(
        COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${download_root})
    if(result)
        message(FATAL_ERROR "CMake step for googletest failed: ${result}")
    endif()
    execute_process(
        COMMAND "${CMAKE_COMMAND}" --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${download_root})
    if(result)
        message(FATAL_ERROR "Build step for googletest failed: ${result}")
    endif()

    # Prevent overriding the parent project's compiler/linker
    # settings on Windows
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    # adds the targers: gtest, gtest_main, gmock, gmock_main
    add_subdirectory(
        ${download_root}/googletest-src
        ${download_root}/googletest-build
        EXCLUDE_FROM_ALL)
endmacro()
