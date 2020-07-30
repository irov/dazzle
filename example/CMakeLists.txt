CMAKE_MINIMUM_REQUIRED(VERSION 3.2)

set(DAZZLE_EXAMPLE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(DAZZLE_EXAMPLE_CONTENT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/content)

project(dazzle_example)

set(CMAKE_CXX_STANDARD 17)

if(APPLE)
    set(LLVM_ENABLE_CXX1Y ON CACHE BOOL "" FORCE)
    
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -stdlib=libc++" CACHE STRING "" FORCE)
elseif(ANDROID)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
elseif(UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
endif()

include_directories(${DAZZLE_EXAMPLE_SOURCE_DIR})
include_directories(${DAZZLE_DEPENDENCIES_DIR})
include_directories(${GLFW_INCLUDE_DIR})
include_directories(${GLAD_INCLUDE_DIR})

if(MSVC)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(DAZZLE_EXAMPLE_SOURCES
    ${DAZZLE_EXAMPLE_SOURCE_DIR}/main.cpp
    
    ${DAZZLE_EXAMPLE_SOURCE_DIR}/opengl.h
    ${DAZZLE_EXAMPLE_SOURCE_DIR}/opengl.cpp
)

ADD_DEFINITIONS(-DDAZZLE_EXAMPLE_CONTENT_DIR="${DAZZLE_EXAMPLE_CONTENT_DIR}")

add_executable(${PROJECT_NAME} ${DAZZLE_EXAMPLE_SOURCES})

message("UNIX: ${UNIX}")
message("APPLE: ${APPLE}")
message("ANDROID: ${ANDROID}")

target_link_libraries(${PROJECT_NAME}
    dazzle
    glad
    glfw
)

add_dependencies(${PROJECT_NAME}
    glad_download
    glfw_download
)

if(UNIX AND NOT ANDROID AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} m)
    target_link_libraries(${PROJECT_NAME} c)
    target_link_libraries(${PROJECT_NAME} dl)
    target_link_libraries(${PROJECT_NAME} X11)
    target_link_libraries(${PROJECT_NAME} Xxf86vm)
    target_link_libraries(${PROJECT_NAME} Xrandr)
    target_link_libraries(${PROJECT_NAME} rt)
    target_link_libraries(${PROJECT_NAME} GL)
    target_link_libraries(${PROJECT_NAME} pthread)
endif()

if(APPLE)
    ADD_APPLE_FRAMEWORK(${PROJECT_NAME} Cocoa)
    ADD_APPLE_FRAMEWORK(${PROJECT_NAME} IOKit)
    ADD_APPLE_FRAMEWORK(${PROJECT_NAME} CoreVideo)
    ADD_APPLE_FRAMEWORK(${PROJECT_NAME} OpenAL)
endif()

set_target_properties (${PROJECT_NAME} PROPERTIES
    FOLDER example
)