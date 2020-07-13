include(ExternalProject)

ExternalProject_Add(imgui_download PREFIX imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
		GIT_TAG "v1.75"
		GIT_PROGRESS TRUE
            
        UPDATE_COMMAND ${CMAKE_COMMAND} -E copy 
			${CMAKE_CURRENT_SOURCE_DIR}/cmake/CMakeLists_imgui.txt
			${CMAKE_CURRENT_BINARY_DIR}/imgui/src/imgui_download/CMakeLists.txt
      
        CMAKE_ARGS 
            -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
            -DGLFW_INCLUDE_DIR=${GLFW_INCLUDE_DIR}
            -DGLAD_INCLUDE_DIR=${GLAD_INCLUDE_DIR}
    )
    
ExternalProject_Add_StepDependencies(imgui_download build glfw_download glad_download)

ExternalProject_Get_Property(imgui_download INSTALL_DIR)
set(IMGUI_INCLUDE_DIR ${INSTALL_DIR}/include)
set(IMGUI_LIBRARY_DIR ${INSTALL_DIR}/lib)

add_library(imgui STATIC IMPORTED)

set_target_properties(imgui PROPERTIES IMPORTED_LOCATION ${IMGUI_LIBRARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}imgui${CMAKE_STATIC_LIBRARY_SUFFIX})

add_library(imgui_glfw STATIC IMPORTED)

set_target_properties(imgui_glfw PROPERTIES IMPORTED_LOCATION ${IMGUI_LIBRARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}imgui_glfw${CMAKE_STATIC_LIBRARY_SUFFIX})

add_library(imgui_opengl3 STATIC IMPORTED)

set_target_properties(imgui_opengl3 PROPERTIES IMPORTED_LOCATION ${IMGUI_LIBRARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}imgui_opengl3${CMAKE_STATIC_LIBRARY_SUFFIX})