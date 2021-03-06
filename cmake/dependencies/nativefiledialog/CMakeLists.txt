CMAKE_MINIMUM_REQUIRED(VERSION 3.2)

project(nativefiledialog)

set(INCLUDE_DIR ${DAZZLE_THIRDPARTY_DIR}/nativefiledialog/src/include)
set(SOURCE_DIR ${DAZZLE_THIRDPARTY_DIR}/nativefiledialog/src)

include_directories(${INCLUDE_DIR})

if(WIN32)
	add_library(${PROJECT_NAME} STATIC ${SOURCE_DIR}/nfd_common.c ${SOURCE_DIR}/nfd_win.cpp)
endif()

if(UNIX AND NOT APPLE)
	FIND_PACKAGE(PkgConfig REQUIRED)
	PKG_CHECK_MODULES(GTK3 REQUIRED gtk+-3.0)
	
	INCLUDE_DIRECTORIES(${GTK3_INCLUDE_DIRS})
	LINK_DIRECTORIES(${GTK3_LIBRARY_DIRS})
    
    ADD_DEFINITIONS(${GTK3_CFLAGS_OTHER})

	add_library(${PROJECT_NAME} STATIC ${SOURCE_DIR}/nfd_common.c ${SOURCE_DIR}/nfd_gtk.c)
	target_link_libraries(${PROJECT_NAME} ${GTK3_LIBRARIES})
endif()

if(APPLE)
	add_library(${PROJECT_NAME} STATIC ${SOURCE_DIR}/nfd_common.c ${SOURCE_DIR}/nfd_cocoa.m)
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(FILES ${INCLUDE_DIR}/nfd.h DESTINATION include)