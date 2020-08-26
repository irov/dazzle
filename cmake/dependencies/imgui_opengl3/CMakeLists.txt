CMAKE_MINIMUM_REQUIRED(VERSION 3.2)

SET(IMGUI_SOURCE_DIR ${DAZZLE_THIRDPARTY_DIR}/imgui)

project(imgui_opengl3)

include_directories(${IMGUI_SOURCE_DIR})
include_directories(${DAZZLE_THIRDPARTY_DIR}/glad/include)

add_library(${PROJECT_NAME} STATIC
    ${IMGUI_SOURCE_DIR}/examples/imgui_impl_opengl3.cpp
)

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(FILES ${IMGUI_SOURCE_DIR}/examples/imgui_impl_opengl3.h DESTINATION include)
