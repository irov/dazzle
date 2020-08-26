CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

PROJECT(solution_win32)

INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../base_template.cmake)
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../win32_template.cmake)

SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${DAZZLE_THIRDPARTY_LIB_OUT})
SET(CMAKE_TEMP_DIR ${CMAKE_BINARY_DIR}/dependencies)

ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/../../ ${CMAKE_TEMP_DIR}/dazzle)

if(DAZZLE_TESTS)
    include(CTest)
    enable_testing()
    
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../tests ${CMAKE_TEMP_DIR}/tests)
else()
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../tools ${CMAKE_TEMP_DIR}/tools)

    if(DAZZLE_TESTS_IN_SOLUTIONS)
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../tests ${CMAKE_TEMP_DIR}/tests)
    endif()
endif()