CMAKE_MINIMUM_REQUIRED(VERSION 3.2)

set(DAZZLE_EVICT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(DAZZLE_EVICT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

project(dazzle_evict)

include_directories(${DAZZLE_INCLUDE_DIR})
include_directories(${DAZZLE_EVICT_INCLUDE_DIR})
include_directories(${DAZZLE_EVICT_SOURCE_DIR})
include_directories(${DAZZLE_DEPENDENCIES_DIR})
include_directories(${DAZZLE_THIRDPARTY_DIR}/glad/include)
include_directories(${DAZZLE_THIRDPARTY_DIR}/jpp/include)

set(DAZZLE_EVICT_SOURCES
    ${DAZZLE_EVICT_INCLUDE_DIR}/evict/evict.hpp
    ${DAZZLE_EVICT_SOURCE_DIR}/evict.cpp
)

add_library(${PROJECT_NAME} STATIC ${DAZZLE_EVICT_SOURCES})

target_link_libraries(${PROJECT_NAME} dazzle)
target_link_libraries(${PROJECT_NAME} dazzle_aux)

target_link_libraries(${PROJECT_NAME} ${DAZZLE_THIRDPARTY_LIB_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}jansson${CMAKE_STATIC_LIBRARY_SUFFIX})
target_link_libraries(${PROJECT_NAME} ${DAZZLE_THIRDPARTY_LIB_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}jpp${CMAKE_STATIC_LIBRARY_SUFFIX})

set_target_properties (${PROJECT_NAME} PROPERTIES
    FOLDER evict
)