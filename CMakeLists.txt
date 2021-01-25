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

PROJECT(dazzle_aux C)

set(DAZZLE_AUX_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dazzle_aux.c
)

set(DAZZLE_AUX_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/dazzle/dazzle_aux.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/dazzle/dazzle_aux.hpp
)

include_directories(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(${PROJECT_NAME} STATIC ${DAZZLE_AUX_SOURCES} ${DAZZLE_AUX_HEADERS})

if(DAZZLE_INSTALL)
    install(DIRECTORY include
        DESTINATION .
        FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h")

    install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib)
endif()

PROJECT(dazzle_read C)

set(DAZZLE_READ_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dazzle_read.c
)

set(DAZZLE_READ_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/dazzle/dazzle_read.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/dazzle/dazzle_read.hpp
)

include_directories(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(${PROJECT_NAME} STATIC ${DAZZLE_READ_SOURCES} ${DAZZLE_READ_HEADERS})

if(DAZZLE_INSTALL)
    install(DIRECTORY include
        DESTINATION .
        FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h")

    install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib)
endif()

PROJECT(dazzle_write C)

set(DAZZLE_WRITE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dazzle_write.c
)

set(DAZZLE_WRITE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/dazzle/dazzle_write.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/dazzle/dazzle_write.hpp
)

include_directories(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(${PROJECT_NAME} STATIC ${DAZZLE_WRITE_SOURCES} ${DAZZLE_WRITE_HEADERS})

if(DAZZLE_INSTALL)
    install(DIRECTORY include
        DESTINATION .
        FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h")

    install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib)
endif()