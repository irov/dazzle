cmake_minimum_required(VERSION 2.4.4)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)

project(minizip C)

include(CheckTypeSize)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckCSourceCompiles)

check_include_file(sys/types.h HAVE_SYS_TYPES_H)
check_include_file(stdint.h    HAVE_STDINT_H)
check_include_file(stddef.h    HAVE_STDDEF_H)

set(CMAKE_REQUIRED_DEFINITIONS -D_LARGEFILE64_SOURCE=1)

if(HAVE_SYS_TYPES_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_SYS_TYPES_H)
endif()
if(HAVE_STDINT_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_STDINT_H)
endif()
if(HAVE_STDDEF_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_STDDEF_H)
endif()
check_type_size(off64_t OFF64_T)
if(HAVE_OFF64_T)
   add_definitions(-D_LARGEFILE64_SOURCE=1)
endif()

set(CMAKE_REQUIRED_DEFINITIONS)

check_function_exists(fseeko HAVE_FSEEKO)
if(NOT HAVE_FSEEKO)
    add_definitions(-DNO_FSEEKO)
endif()

check_include_file(unistd.h Z_HAVE_UNISTD_H)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
endif()

#============================================================================
# zlib
#============================================================================

include_directories(${ZLIB_INCLUDE_DIR})
MESSAGE("ZLIB_INCLUDE_DIR: ${ZLIB_INCLUDE_DIR}")

set(MINIZIP_SOURCES
    ${DAZZLE_THIRDPARTY_DIR}/zlib/contrib/minizip/zip.h
    ${DAZZLE_THIRDPARTY_DIR}/zlib/contrib/minizip/zip.c
    ${DAZZLE_THIRDPARTY_DIR}/zlib/contrib/minizip/ioapi.c
)

add_library(${PROJECT_NAME} STATIC ${MINIZIP_SOURCES})