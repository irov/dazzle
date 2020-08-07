CMAKE_MINIMUM_REQUIRED(VERSION 3.2)

set(DAZZLE_RENDER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(DAZZLE_RENDER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

project(dazzle_render)

set(CMAKE_CXX_STANDARD 17)

if(APPLE)
    set(LLVM_ENABLE_CXX1Y ON CACHE BOOL "" FORCE)
    
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -stdlib=libc++" CACHE STRING "" FORCE)
elseif(ANDROID)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
elseif(UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
endif()

include_directories(${DAZZLE_RENDER_INCLUDE_DIR})
include_directories(${DAZZLE_RENDER_SOURCE_DIR})
include_directories(${DAZZLE_DEPENDENCIES_DIR})
MESSAGE("GLAD_INCLUDE_DIR: ${GLAD_INCLUDE_DIR}")
include_directories(${GLAD_INCLUDE_DIR})

if(MSVC)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(DAZZLE_RENDER_SOURCES
    ${DAZZLE_RENDER_INCLUDE_DIR}/render/render.h
    ${DAZZLE_RENDER_SOURCE_DIR}/render.cpp
)

add_library(${PROJECT_NAME} STATIC ${DAZZLE_RENDER_SOURCES})

target_link_libraries(${PROJECT_NAME}
    dazzle
    glad
)

add_dependencies(${PROJECT_NAME}
    glad_download
)

set_target_properties (${PROJECT_NAME} PROPERTIES
    FOLDER render
)