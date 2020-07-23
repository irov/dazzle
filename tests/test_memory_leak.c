#include "dazzle/dazzle.h"

#include <stdlib.h>
#include <math.h>

//////////////////////////////////////////////////////////////////////////
static void * dz_malloc( dz_size_t _size, dz_userdata_t _ud )
{
    void * p = malloc( _size + sizeof( dz_size_t ) );

    *(dz_size_t *)p = _size;

    *(dz_size_t *)_ud += _size;

    return (dz_size_t *)p + 1;
}
//////////////////////////////////////////////////////////////////////////
static void * dz_realloc( void * _ptr, dz_size_t _size, dz_userdata_t _ud )
{
    void * p = realloc( _ptr, _size + sizeof( dz_size_t ) );

    *(dz_size_t *)p = _size;

    *(dz_size_t *)_ud += _size;

    return (dz_size_t *)p + 1;
}
//////////////////////////////////////////////////////////////////////////
static void dz_free( void * _ptr, dz_userdata_t _ud )
{
    dz_size_t * p = (dz_size_t *)_ptr - 1;

    *(dz_size_t *)_ud -= *p;

    free( p );
}
//////////////////////////////////////////////////////////////////////////
static float dz_cosf( float _a, dz_userdata_t _ud )
{
    DZ_UNUSED( _ud );

    float value = cosf( _a );

    return value;
}
//////////////////////////////////////////////////////////////////////////
static float dz_sinf( float _a, dz_userdata_t _ud )
{
    DZ_UNUSED( _ud );

    float value = sinf( _a );

    return value;
}
//////////////////////////////////////////////////////////////////////////
int main( int argc, char ** argv )
{
    DZ_UNUSED( argc );
    DZ_UNUSED( argv );

    dz_size_t msz = 0;

    dz_service_providers_t providers;
    providers.f_malloc = &dz_malloc;
    providers.f_realloc = &dz_realloc;
    providers.f_free = &dz_free;
    providers.f_cosf = &dz_cosf;
    providers.f_sinf = &dz_sinf;

    dz_service_t * kernel;
    if( dz_service_create( &kernel, &providers, &msz ) == DZ_FAILURE )
    {
        return EXIT_FAILURE;
    }

    dz_service_destroy( kernel );

    if( msz != 0 )
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}