#include "editor.h"

#include "dazzle/dazzle.hpp"

int main( int argc, char ** argv )
{
    editor editorApp = editor();

    if( editorApp.run( argc, argv ) == DZ_FAILURE )
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}