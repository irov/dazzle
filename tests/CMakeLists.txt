cmake_minimum_required(VERSION 3.0)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

macro(ADD_DAZZLE_TEST testname)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
    
    ADD_EXECUTABLE(test_${testname} test_${testname}.c)
    TARGET_LINK_LIBRARIES(test_${testname} dazzle)
    
    if(UNIX AND NOT ANDROID AND NOT APPLE)
        target_link_libraries(test_${testname} m)
        target_link_libraries(test_${testname} c)
        target_link_libraries(test_${testname} dl)
    endif()

    set_target_properties (test_${testname} PROPERTIES
        FOLDER tests
    )

    ADD_TEST(NAME ${testname} COMMAND ${DAZZLE_ROOT_DIR}/bin/${CMAKE_GENERATOR}/test_${testname} ${DAZZLE_ROOT_DIR}/bin/${CMAKE_GENERATOR})
endmacro()

ADD_DAZZLE_TEST(memory_leak)