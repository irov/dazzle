CMAKE_MINIMUM_REQUIRED(VERSION 3.2)

set(DAZZLE_EVICT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(DAZZLE_EVICT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

project(dazzle_evict C)

set(CMAKE_C_STANDARD 11)

if(${CMAKE_C_COMPILER_ID} STREQUAL Clang)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")
elseif(${CMAKE_C_COMPILER_ID} STREQUAL AppleClang)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")
elseif(${CMAKE_C_COMPILER_ID} STREQUAL GNU)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")
elseif(${CMAKE_C_COMPILER_ID} STREQUAL Intel)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")
elseif(${CMAKE_C_COMPILER_ID} STREQUAL MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Wall /WX")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd5045")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4668")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4710")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4738")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4820")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4711")
endif()

if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /DWIN32 /D_WINDOWS")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /D_DEBUG /ZI /Ob0 /Od /RTC1")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /DNDEBUG /O2 /Oi /Ot /GL")
    
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /D_CRT_SECURE_NO_WARNINGS")
endif()

include_directories(${DAZZLE_EVICT_INCLUDE_DIR})
include_directories(${DAZZLE_EVICT_SOURCE_DIR})
include_directories(${DAZZLE_DEPENDENCIES_DIR})
include_directories(${GLAD_INCLUDE_DIR})
include_directories(${JPP_INCLUDE_DIR})

if(MSVC)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(DAZZLE_EVICT_SOURCES
    ${DAZZLE_EVICT_INCLUDE_DIR}/evict/evict.hpp
    ${DAZZLE_EVICT_SOURCE_DIR}/evict.cpp
)

add_library(${PROJECT_NAME} STATIC ${DAZZLE_EVICT_SOURCES})

target_link_libraries(${PROJECT_NAME}
    dazzle
    jansson
    jpp
)

add_dependencies(${PROJECT_NAME}
    jansson_download
    jpp_download
)

set_target_properties (${PROJECT_NAME} PROPERTIES
    FOLDER evict
)