include_guard(GLOBAL)

include(ExternalProject)

ExternalProject_Add(jansson_download PREFIX jansson
        GIT_REPOSITORY https://github.com/akheron/jansson.git
		GIT_TAG "v2.13.1"
		GIT_PROGRESS TRUE
            
        UPDATE_COMMAND ""
      
        CMAKE_ARGS 
            -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
            -DUSE_URANDOM=OFF
            -DUSE_WINDOWS_CRYPTOAPI=OFF
            -DJANSSON_STATIC_CRT=ON
            -DJANSSON_EXAMPLES=ON
            -DJANSSON_BUILD_DOCS=OFF
            -DJANSSON_WITHOUT_TESTS=ON
            -DJANSSON_INSTALL=ON
    )

ExternalProject_Get_Property(jansson_download INSTALL_DIR)

set(JANSSON_INCLUDE_DIR ${INSTALL_DIR}/include)
set(JANSSON_LIBRARY_DIR ${INSTALL_DIR}/lib)

set(JANSSON_INCLUDE_DIR ${JANSSON_INCLUDE_DIR} CACHE STRING "")
set(JANSSON_LIBRARY_DIR ${JANSSON_LIBRARY_DIR} CACHE STRING "")

add_library(jansson STATIC IMPORTED)

set_target_properties(jansson PROPERTIES IMPORTED_LOCATION ${JANSSON_LIBRARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}jansson${CMAKE_STATIC_LIBRARY_SUFFIX})