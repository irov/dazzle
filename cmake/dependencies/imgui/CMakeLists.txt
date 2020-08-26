CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

SET(IMGUI_SOURCE_DIR ${DAZZLE_THIRDPARTY_DIR}/imgui)

project(imgui)

add_library(${PROJECT_NAME} STATIC
    ${IMGUI_SOURCE_DIR}/imgui.cpp
    ${IMGUI_SOURCE_DIR}/imgui_draw.cpp
    ${IMGUI_SOURCE_DIR}/imgui_widgets.cpp
)

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(FILES ${IMGUI_SOURCE_DIR}/imgui.h ${IMGUI_SOURCE_DIR}/imgui_internal.h ${IMGUI_SOURCE_DIR}/imstb_textedit.h ${IMGUI_SOURCE_DIR}/imconfig.h DESTINATION include)