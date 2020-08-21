if(NOT DEFINED ENV{MINGW64_ROOT})
    message(FATAL_ERROR "Required ENV variable MINGW64_ROOT does not exists")
else()
    SET(MINGW64_ROOT $ENV{MINGW64_ROOT})
endif()

set(MINGW_ROOT ${MINGW64_ROOT})

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

list(APPEND CMAKE_PREFIX_PATH
    ${MINGW_ROOT}/x86_64-w64-mingw32
    ${MINGW_ROOT}
)

SET(CMAKE_ASM_MASM_COMPILER    "uasm")
SET(CMAKE_C_COMPILER           "clang")
SET(CMAKE_CXX_COMPILER         "clang++")

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -femulated-tls -fuse-ld=lld -DWIN32 -D_WIN64 -m64")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -femulated-tls -fuse-ld=lld -DWIN32 -D_WIN64 -m64")

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -fno-strict-aliasing -fomit-frame-pointer -std=c11 -O0 -D_DEBUG")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fno-strict-aliasing -fomit-frame-pointer -std=c11 -O3 -DNDEBUG")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -fno-strict-aliasing -fomit-frame-pointer -std=c++17 -O0 -D_DEBUG -UMBCS -D_UNICODE -DUNICODE")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-strict-aliasing -fomit-frame-pointer -std=c++17 -O3 -DNDEBUG -UMBCS -D_UNICODE -DUNICODE")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")