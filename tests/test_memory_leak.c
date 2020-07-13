#include "dazzle/dazzle.h"

#include <stdlib.h>

//////////////////////////////////////////////////////////////////////////
static void * dz_malloc( dz_size_t _size, void * _ud )
{
    void * p = malloc( _size + sizeof( dz_size_t ) );

    *(dz_size_t *)p = _size;

    *(dz_size_t *)_ud += _size;

    return (dz_size_t *)p + 1;
}
//////////////////////////////////////////////////////////////////////////
static void * dz_realloc( void * _ptr, dz_size_t _size, void * _ud )
{
    void * p = realloc( _ptr, _size + sizeof( dz_size_t ) );

    *(dz_size_t *)p = _size;

    *(dz_size_t *)_ud += _size;

    return (dz_size_t *)p + 1;
}
//////////////////////////////////////////////////////////////////////////
static void dz_free( void * _ptr, void * _ud )
{
    dz_size_t * p = (dz_size_t *)_ptr - 1;

    *(dz_size_t *)_ud -= *p;

    free( p );
}
//////////////////////////////////////////////////////////////////////////
int main( int argc, char ** argv )
{
    DZ_UNUSED( argc );
    DZ_UNUSED( argv );

    dz_size_t msz = 0;

    dz_kernel_t * kernel;
    if( dz_kernel_create( &kernel, &dz_malloc, &dz_realloc, &dz_free, &msz ) == DZ_FAILURE )
    {
        return EXIT_FAILURE;
    }

    dz_kernel_destroy( kernel );

    if( msz != 0 )
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}