include_guard(GLOBAL)

include(ExternalProject)

ExternalProject_Add(nativefiledialog_download PREFIX nativefiledialog
        GIT_REPOSITORY https://github.com/mlabbe/nativefiledialog.git
		GIT_PROGRESS TRUE
            
        UPDATE_COMMAND ${CMAKE_COMMAND} -E copy 
			${DAZZLE_ROOT_DIR}/cmake/CMakeLists_nativefiledialog.txt
			${CMAKE_CURRENT_BINARY_DIR}/nativefiledialog/src/nativefiledialog_download/CMakeLists.txt
      
        CMAKE_ARGS 
            -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>			
    )

ExternalProject_Get_Property(nativefiledialog_download INSTALL_DIR)
set(NATIVEFILEDIALOG_INCLUDE_DIR ${INSTALL_DIR}/include)
set(NATIVEFILEDIALOG_LIBRARY_DIR ${INSTALL_DIR}/lib)

set(NATIVEFILEDIALOG_INCLUDE_DIR ${NATIVEFILEDIALOG_INCLUDE_DIR} CACHE STRING "NATIVEFILEDIALOG_INCLUDE_DIR")
set(NATIVEFILEDIALOG_LIBRARY_DIR ${NATIVEFILEDIALOG_LIBRARY_DIR} CACHE STRING "NATIVEFILEDIALOG_LIBRARY_DIR")

add_library(nativefiledialog STATIC IMPORTED)

set_target_properties(nativefiledialog PROPERTIES IMPORTED_LOCATION ${NATIVEFILEDIALOG_LIBRARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}nativefiledialog${CMAKE_STATIC_LIBRARY_SUFFIX})