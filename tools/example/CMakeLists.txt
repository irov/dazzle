CMAKE_MINIMUM_REQUIRED(VERSION 3.2)

set(DAZZLE_EXAMPLE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(DAZZLE_EXAMPLE_CONTENT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/content)

project(dazzle_example)

include_directories(${DAZZLE_INCLUDE_DIR})
include_directories(${DAZZLE_EXAMPLE_SOURCE_DIR})
include_directories(${DAZZLE_DEPENDENCIES_DIR})
include_directories(${DAZZLE_THIRDPARTY_DIR}/glfw3/include)
include_directories(${DAZZLE_THIRDPARTY_DIR}/glad/include)
include_directories(${DAZZLE_TOOLS_DIR}/render/include)

set(DAZZLE_EXAMPLE_SOURCES
    ${DAZZLE_EXAMPLE_SOURCE_DIR}/main.cpp
)

ADD_DEFINITIONS(-DDAZZLE_EXAMPLE_CONTENT_DIR="${DAZZLE_EXAMPLE_CONTENT_DIR}")

add_executable(${PROJECT_NAME} ${DAZZLE_EXAMPLE_SOURCES})

target_link_libraries(${PROJECT_NAME} dazzle)
target_link_libraries(${PROJECT_NAME} dazzle_render)

target_link_libraries(${PROJECT_NAME} ${DAZZLE_THIRDPARTY_LIB_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}glad${CMAKE_STATIC_LIBRARY_SUFFIX})
target_link_libraries(${PROJECT_NAME} ${DAZZLE_THIRDPARTY_LIB_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}glfw3${CMAKE_STATIC_LIBRARY_SUFFIX})

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