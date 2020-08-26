CMAKE_MINIMUM_REQUIRED( VERSION 3.0 )

set(DAZZLE_DOWNLOADS_PATH "${DAZZLE_THIRDPARTY_DIR}" CACHE STRING "DAZZLE_DOWNLOADS_PATH")

if(DAZZLE_DOWNLOADS_SILENT)
    SET(DAZZLE_GIT_PROGRESS FALSE)
    SET(DAZZLE_DOWNLOAD_NO_PROGRESS 1)
    SET(DAZZLE_FILE_DOWNLOAD_SHOW_PROGRESS FALSE)
else()
    SET(DAZZLE_GIT_PROGRESS TRUE)
    SET(DAZZLE_DOWNLOAD_NO_PROGRESS 0)
    SET(DAZZLE_FILE_DOWNLOAD_SHOW_PROGRESS TRUE)
endif()

MESSAGE("DAZZLE_DOWNLOADS_PATH: " ${DAZZLE_DOWNLOADS_PATH})
MESSAGE("DAZZLE_DOWNLOADS_SILENT: " ${DAZZLE_DOWNLOADS_SILENT})

include(ExternalProject)

find_package(Git REQUIRED)

macro(DOWNLOAD_FILE NAME URL FILE)
    MESSAGE("Download ${NAME}: ${URL}")
    if(NOT EXISTS ${DAZZLE_DOWNLOADS_PATH}/${NAME}/${FILE})
        if(DAZZLE_FILE_DOWNLOAD_SHOW_PROGRESS)
            file(DOWNLOAD ${URL} ${DAZZLE_DOWNLOADS_PATH}/${NAME}/${FILE} SHOW_PROGRESS)
        else()
            file(DOWNLOAD ${URL} ${DAZZLE_DOWNLOADS_PATH}/${NAME}/${FILE})
        endif()
    endif()
endmacro()

macro(DOWNLOAD_URL NAME URL)
    MESSAGE("Download ${NAME}: ${URL}")
    ExternalProject_Add(${NAME}_download PREFIX ${NAME}
        SOURCE_DIR ${DAZZLE_THIRDPARTY_DIR}/${NAME}
        
        URL ${URL}
        DOWNLOAD_NO_PROGRESS ${DAZZLE_DOWNLOAD_NO_PROGRESS}

        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        UPDATE_COMMAND ""
        INSTALL_COMMAND ""
    )
    
    add_library(${NAME} STATIC IMPORTED)
endmacro()

macro(DOWNLOAD_URL_HASH NAME URL HASH_ALGO HASH)
    MESSAGE("Download ${NAME}: ${URL}")
    ExternalProject_Add(${NAME}_download PREFIX ${NAME}
        SOURCE_DIR ${DAZZLE_THIRDPARTY_DIR}/${NAME}
        
        URL ${URL}
        URL_HASH ${HASH_ALGO}=${HASH}
        DOWNLOAD_NO_PROGRESS ${DAZZLE_DOWNLOAD_NO_PROGRESS}

        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        UPDATE_COMMAND ""
        INSTALL_COMMAND ""
    )
    
    add_library(${NAME} STATIC IMPORTED)
endmacro()

macro(GIT_CLONE NAME REPOSITORY)
    set(TAG ${ARGN})
    
    list(LENGTH TAG EXIST_TAG)
    
    if(${EXIST_TAG} GREATER 0)
        MESSAGE("Git ${NAME}: ${REPOSITORY} [${TAG}]")
        ExternalProject_Add( "${NAME}_${TAG}_download" PREFIX ${NAME}
            SOURCE_DIR ${DAZZLE_THIRDPARTY_DIR}/${NAME}
            
            GIT_REPOSITORY ${REPOSITORY}
            GIT_TAG ${TAG}
            GIT_PROGRESS ${DAZZLE_GIT_PROGRESS}

            CONFIGURE_COMMAND ""
            BUILD_COMMAND ""
            UPDATE_COMMAND ""
            INSTALL_COMMAND ""
        )
    else()
        MESSAGE("Git ${NAME}: ${REPOSITORY}")
        
        ExternalProject_Add(${NAME}_download PREFIX ${NAME}
            SOURCE_DIR ${DAZZLE_THIRDPARTY_DIR}/${NAME}
            
            GIT_REPOSITORY ${REPOSITORY}
            GIT_PROGRESS ${DAZZLE_GIT_PROGRESS}

            CONFIGURE_COMMAND ""
            BUILD_COMMAND ""
            UPDATE_COMMAND ${GIT_EXECUTABLE} pull
            INSTALL_COMMAND ""
            )
    endif()
    
    add_library(${NAME} STATIC IMPORTED)
endmacro()