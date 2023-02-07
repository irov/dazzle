CMAKE_MINIMUM_REQUIRED(VERSION 3.2)

set(DAZZLE_RENDER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(DAZZLE_RENDER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

project(dazzle_render C)

include_directories(${DAZZLE_INCLUDE_DIR})
include_directories(${DAZZLE_RENDER_INCLUDE_DIR})
include_directories(${DAZZLE_RENDER_SOURCE_DIR})
include_directories(${DAZZLE_DEPENDENCIES_DIR})
include_directories(${DAZZLE_THIRDPARTY_DIR}/glad_gl_core_33/include)

set(DAZZLE_RENDER_SOURCES
    ${DAZZLE_RENDER_INCLUDE_DIR}/render/render.h
    ${DAZZLE_RENDER_INCLUDE_DIR}/render/render.hpp
    ${DAZZLE_RENDER_SOURCE_DIR}/render.c
)

add_library(${PROJECT_NAME} STATIC ${DAZZLE_RENDER_SOURCES})

target_link_libraries(${PROJECT_NAME} dazzle)
target_link_libraries(${PROJECT_NAME} ${DAZZLE_THIRDPARTY_LIB_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}glad_gl_core_33${CMAKE_STATIC_LIBRARY_SUFFIX})

set_target_properties (${PROJECT_NAME} PROPERTIES
    FOLDER render
)