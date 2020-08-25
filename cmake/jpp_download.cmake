include_guard(GLOBAL)

include(ExternalProject)

MESSAGE("JANSSON_INCLUDE_DIR: ${JANSSON_INCLUDE_DIR}")

ExternalProject_Add(jpp_download PREFIX jpp
        GIT_REPOSITORY https://github.com/irov/jpp.git
		GIT_PROGRESS TRUE
            
        UPDATE_COMMAND ""
      
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
            -DJPP_EXTERNAL_BUILD=ON
            -DJPP_JANSSON_INCLUDE_PATH=${JANSSON_INCLUDE_DIR}
            -DJPP_TEST=OFF
            -DJPP_INSTALL=ON
    )

ExternalProject_Get_Property(jpp_download INSTALL_DIR)
set(JPP_INCLUDE_DIR ${INSTALL_DIR}/include)
set(JPP_LIBRARY_DIR ${INSTALL_DIR}/lib)

set(JPP_INCLUDE_DIR ${JPP_INCLUDE_DIR} CACHE STRING "")
set(JPP_LIBRARY_DIR ${JPP_LIBRARY_DIR} CACHE STRING "")

add_library(jpp STATIC IMPORTED)

add_dependencies(jpp jansson)

set_target_properties(jpp PROPERTIES IMPORTED_LOCATION ${JPP_LIBRARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}jpp${CMAKE_STATIC_LIBRARY_SUFFIX})