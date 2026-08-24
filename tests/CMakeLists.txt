function(dazzle_add_test name)
    add_executable(dazzle_${name}_test ${ARGN})

    if(name STREQUAL "serialization")
        target_link_libraries(dazzle_${name}_test PRIVATE dazzle_read dazzle_write)
    else()
        target_link_libraries(dazzle_${name}_test PRIVATE dazzle)
    endif()

    target_compile_features(dazzle_${name}_test PRIVATE c_std_99)

    dazzle_configure_target(dazzle_${name}_test)

    if(UNIX)
        target_link_libraries(dazzle_${name}_test PRIVATE m)
    endif()

    add_test(NAME dazzle_${name} COMMAND dazzle_${name}_test)
endfunction()

dazzle_add_test(memory test_memory_leak.c)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_runtime.c")
    dazzle_add_test(runtime test_runtime.c)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_serialization.c")
    dazzle_add_test(serialization test_serialization.c)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_geometry.c")
    dazzle_add_test(geometry test_geometry.c)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_physics.c")
    dazzle_add_test(physics test_physics.c)
endif()
