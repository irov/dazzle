cmake_minimum_required(VERSION 3.16)

project(dazzle VERSION 5.0.0 LANGUAGES C)

option(DAZZLE_INSTALL "Install Dazzle libraries and headers" OFF)
option(DAZZLE_BUILD_TESTS "Build Dazzle tests" OFF)
option(DAZZLE_BUILD_EDITOR "Build the Dazzle editor" OFF)
option(DAZZLE_BUILD_EXAMPLE "Build the Dazzle example" OFF)
option(DAZZLE_WARNINGS_AS_ERRORS "Treat Dazzle compiler warnings as errors" ON)
option(DAZZLE_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

# Keep the historical switches accepted by existing build scripts.
if(DEFINED DAZZLE_TESTS)
    set(DAZZLE_BUILD_TESTS "${DAZZLE_TESTS}")
endif()
if(DEFINED DAZZLE_EDITOR_BUILD)
    set(DAZZLE_BUILD_EDITOR "${DAZZLE_EDITOR_BUILD}")
endif()
if(DEFINED DAZZLE_EXAMPLE_BUILD)
    set(DAZZLE_BUILD_EXAMPLE "${DAZZLE_EXAMPLE_BUILD}")
endif()

function(dazzle_configure_target target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
        if(DAZZLE_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif()
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
        if(DAZZLE_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()

    if(DAZZLE_ENABLE_SANITIZERS)
        if(MSVC)
            message(FATAL_ERROR "DAZZLE_ENABLE_SANITIZERS currently requires Clang or GCC")
        endif()

        target_compile_options(${target} PUBLIC -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(${target} PUBLIC -fsanitize=address,undefined)
    endif()
endfunction()

set(DAZZLE_PUBLIC_HEADERS
    include/dazzle/config.h
    include/dazzle/dazzle.h
    include/dazzle/dazzle.hpp
)

set(DAZZLE_PRIVATE_HEADERS
    src/affector.h
    src/alloc.h
    src/atlas.h
    src/constant65535.h
    src/effect.h
    src/emitter.h
    src/instance.h
    src/material.h
    src/memory.h
    src/math_provider.h
    src/math3d.h
    src/particle.h
    src/service.h
    src/shape.h
    src/texture.h
    src/timeline_interpolate.h
    src/timeline_key.h
    src/timeline_limits.h
)

add_library(dazzle STATIC
    src/dazzle.c
    src/dazzle_affector.c
    src/dazzle_atlas.c
    src/dazzle_effect.c
    src/dazzle_emitter.c
    src/dazzle_instance.c
    src/dazzle_material.c
    src/dazzle_math.c
    src/dazzle_render.c
    src/dazzle_service.c
    src/dazzle_shape.c
    src/dazzle_texture.c
    src/dazzle_timeline.c
    src/memory.c
    ${DAZZLE_PUBLIC_HEADERS}
    ${DAZZLE_PRIVATE_HEADERS}
)
add_library(dazzle::dazzle ALIAS dazzle)
set_target_properties(dazzle PROPERTIES EXPORT_NAME dazzle)
target_compile_features(dazzle PUBLIC c_std_99)
target_include_directories(dazzle
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
dazzle_configure_target(dazzle)

add_library(dazzle_aux STATIC
    src/dazzle_aux.c
    include/dazzle/dazzle_aux.h
    include/dazzle/dazzle_aux.hpp
)
add_library(dazzle::aux ALIAS dazzle_aux)
set_target_properties(dazzle_aux PROPERTIES EXPORT_NAME aux)
target_compile_features(dazzle_aux PUBLIC c_std_99)
target_include_directories(dazzle_aux PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(dazzle_aux PUBLIC dazzle)
dazzle_configure_target(dazzle_aux)

add_library(dazzle_read STATIC
    src/dazzle_read.c
    include/dazzle/dazzle_read.h
    include/dazzle/dazzle_read.hpp
)
add_library(dazzle::read ALIAS dazzle_read)
set_target_properties(dazzle_read PROPERTIES EXPORT_NAME read)
target_compile_features(dazzle_read PUBLIC c_std_99)
target_include_directories(dazzle_read
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(dazzle_read PUBLIC dazzle)
dazzle_configure_target(dazzle_read)

add_library(dazzle_write STATIC
    src/dazzle_write.c
    include/dazzle/dazzle_write.h
    include/dazzle/dazzle_write.hpp
)
add_library(dazzle::write ALIAS dazzle_write)
set_target_properties(dazzle_write PROPERTIES EXPORT_NAME write)
target_compile_features(dazzle_write PUBLIC c_std_99)
target_include_directories(dazzle_write
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(dazzle_write PUBLIC dazzle)
dazzle_configure_target(dazzle_write)

if(DAZZLE_BUILD_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif()

if(DAZZLE_BUILD_EDITOR OR DAZZLE_BUILD_EXAMPLE)
    enable_language(CXX)
    if(APPLE)
        enable_language(OBJC)
    endif()

    set(DAZZLE_REPOSITORY "${CMAKE_CURRENT_SOURCE_DIR}")
    set(DAZZLE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
    set(DAZZLE_DEPENDENCIES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dependencies")
    set(DAZZLE_THIRDPARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dependencies")
    set(DAZZLE_THIRDPARTY_CONFIG_DIR "${CMAKE_CURRENT_BINARY_DIR}/configuration")
    set(DAZZLE_TOOLS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tools")
    set(CMAKE_TEMP_DIR "${CMAKE_CURRENT_BINARY_DIR}/thirdparty-build")

    set(DAZZLE_DEPENDENCE_GLAD ON CACHE BOOL "" FORCE)
    set(DAZZLE_DEPENDENCE_GLFW3 ON CACHE BOOL "" FORCE)
    set(DAZZLE_DEPENDENCE_GRAPHICS OFF CACHE BOOL "" FORCE)
    if(DAZZLE_BUILD_EDITOR)
        set(DAZZLE_DEPENDENCE_ZLIB ON CACHE BOOL "" FORCE)
        set(DAZZLE_DEPENDENCE_JANSSON ON CACHE BOOL "" FORCE)
        set(DAZZLE_DEPENDENCE_JPP ON CACHE BOOL "" FORCE)
        set(DAZZLE_DEPENDENCE_IMGUI ON CACHE BOOL "" FORCE)
        set(DAZZLE_DEPENDENCE_NATIVEFILEDIALOG ON CACHE BOOL "" FORCE)
    else()
        set(DAZZLE_DEPENDENCE_ZLIB OFF CACHE BOOL "" FORCE)
        set(DAZZLE_DEPENDENCE_JANSSON OFF CACHE BOOL "" FORCE)
        set(DAZZLE_DEPENDENCE_JPP OFF CACHE BOOL "" FORCE)
        set(DAZZLE_DEPENDENCE_IMGUI OFF CACHE BOOL "" FORCE)
        set(DAZZLE_DEPENDENCE_NATIVEFILEDIALOG OFF CACHE BOOL "" FORCE)
    endif()

    if(APPLE AND NOT COMMAND ADD_APPLE_FRAMEWORK)
        function(ADD_APPLE_FRAMEWORK target framework)
            find_library(DAZZLE_FRAMEWORK_${framework} ${framework} REQUIRED)
            target_link_libraries(${target} "${DAZZLE_FRAMEWORK_${framework}}")
        endfunction()
    endif()

    add_subdirectory(cmake/dependencies ${CMAKE_CURRENT_BINARY_DIR}/thirdparty)
    set(DAZZLE_EDITOR_BUILD "${DAZZLE_BUILD_EDITOR}")
    set(DAZZLE_EXAMPLE_BUILD "${DAZZLE_BUILD_EXAMPLE}")
    add_subdirectory(tools)
endif()

if(DAZZLE_INSTALL)
    include(GNUInstallDirs)

    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(
        TARGETS dazzle dazzle_aux dazzle_read dazzle_write
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
endif()
