CMAKE_MINIMUM_REQUIRED(VERSION 3.2)

if(DAZZLE_EDITOR_BUILD OR DAZZLE_EXAMPLE_BUILD)
    include(${DAZZLE_CMAKE_DIR}/macro.cmake)
    include(${DAZZLE_CMAKE_DIR}/glad_download.cmake)
    include(${DAZZLE_CMAKE_DIR}/stb_libs_download.cmake)    
    include(${DAZZLE_CMAKE_DIR}/glfw_download.cmake)
    include(${DAZZLE_CMAKE_DIR}/imgui_download.cmake)
    include(${DAZZLE_CMAKE_DIR}/jansson_download.cmake)
    include(${DAZZLE_CMAKE_DIR}/jpp_download.cmake)

    add_subdirectory(evict)
    add_subdirectory(render)
endif()

if(DAZZLE_EDITOR_BUILD)
    include(${DAZZLE_CMAKE_DIR}/nativefiledialog_download.cmake)
    
    add_subdirectory(editor)
endif()

if(DAZZLE_EXAMPLE_BUILD)
    add_subdirectory(example)
endif()