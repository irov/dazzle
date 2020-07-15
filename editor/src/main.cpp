#include "editor.h"

#include "dazzle/dazzle.hpp"

int main( int argc, char ** argv )
{
    DZ_UNUSED( argc );
    DZ_UNUSED( argv );

    editor editorApp = editor();

    return editorApp.run();
}