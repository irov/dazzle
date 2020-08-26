CMAKE_MINIMUM_REQUIRED(VERSION 3.2)

if(DAZZLE_EDITOR_BUILD OR DAZZLE_EXAMPLE_BUILD)
    add_subdirectory(evict)
    add_subdirectory(render)
endif()

if(DAZZLE_EDITOR_BUILD)    
    add_subdirectory(editor)
endif()

if(DAZZLE_EXAMPLE_BUILD)
    add_subdirectory(example)
endif()