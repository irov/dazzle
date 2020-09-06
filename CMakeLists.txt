CMAKE_MINIMUM_REQUIRED(VERSION 3.2)

PROJECT(dazzle C)

OPTION(DAZZLE_INSTALL "DAZZLE_INSTALL" OFF)

set(DAZZLE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/affector.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/alloc.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/atlas.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dazzle.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/effect.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/emitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/material.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/math.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/instance.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/particle.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/service.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shape.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/texture.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/timeline_interpolate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/timeline_key.h
)

set(DAZZLE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/dazzle/config.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/dazzle/dazzle.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/dazzle/dazzle.hpp
)

include_directories(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(${PROJECT_NAME} STATIC ${DAZZLE_SOURCES} ${DAZZLE_HEADERS})

if(DAZZLE_INSTALL)
    install(DIRECTORY include
        DESTINATION .
        FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h")

    install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib)
endif()