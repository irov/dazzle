CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

set(DAZZLE_EDITOR_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

project(dazzle_editor)

include_directories(${DAZZLE_INCLUDE_DIR})
include_directories(${DAZZLE_EDITOR_SOURCE_DIR})
include_directories(${DAZZLE_DEPENDENCIES_DIR})
include_directories(${DAZZLE_THIRDPARTY_CONFIG_DIR}/zlib)
include_directories(${DAZZLE_THIRDPARTY_DIR}/zlib)
include_directories(${DAZZLE_THIRDPARTY_DIR}/zlib/contrib/minizip)
include_directories(${DAZZLE_THIRDPARTY_DIR}/glfw3/include)
include_directories(${DAZZLE_THIRDPARTY_DIR}/glad_gl_core_33/include)
include_directories(${DAZZLE_THIRDPARTY_DIR}/imgui)
include_directories(${DAZZLE_THIRDPARTY_DIR}/imgui/backends)
include_directories(${DAZZLE_THIRDPARTY_DIR}/nativefiledialog/src/include)
include_directories(${DAZZLE_THIRDPARTY_DIR}/jpp/include)

include_directories(${DAZZLE_TOOLS_DIR}/render/include)
include_directories(${DAZZLE_TOOLS_DIR}/evict/include)

set(DAZZLE_EDITOR_SOURCES
    ${DAZZLE_EDITOR_SOURCE_DIR}/main.cpp
    ${DAZZLE_EDITOR_SOURCE_DIR}/editor.cpp
    ${DAZZLE_EDITOR_SOURCE_DIR}/editor.h
)

add_executable(${PROJECT_NAME} ${DAZZLE_EDITOR_SOURCES})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

target_link_libraries(${PROJECT_NAME} dazzle)
target_link_libraries(${PROJECT_NAME} dazzle_render)
target_link_libraries(${PROJECT_NAME} dazzle_evict)
target_link_libraries(${PROJECT_NAME} dazzle_aux)
target_link_libraries(${PROJECT_NAME} dazzle_write)
target_link_libraries(${PROJECT_NAME} dazzle_read)

target_link_libraries(${PROJECT_NAME} zlibstatic minizip glad_gl_core_33 glfw imgui imgui_glfw imgui_opengl3 nativefiledialog jansson jpp)

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

if(COMMAND dazzle_configure_target)
    dazzle_configure_target(${PROJECT_NAME})
endif()

if(APPLE)
    ADD_APPLE_FRAMEWORK(${PROJECT_NAME} Cocoa)
    ADD_APPLE_FRAMEWORK(${PROJECT_NAME} IOKit)
    ADD_APPLE_FRAMEWORK(${PROJECT_NAME} CoreVideo)
    ADD_APPLE_FRAMEWORK(${PROJECT_NAME} OpenAL)
endif()

set_target_properties (${PROJECT_NAME} PROPERTIES
    FOLDER editor
)
