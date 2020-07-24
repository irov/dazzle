#include "dazzle/dazzle.hpp"

#include <stdlib.h>
#include <math.h>

//////////////////////////////////////////////////////////////////////////
static void * dz_malloc( dz_size_t _size, dz_userdata_t _ud )
{
    DZ_UNUSED( _ud );

    void * p = malloc( _size );

    return p;
}
//////////////////////////////////////////////////////////////////////////
static void * dz_realloc( void * _ptr, dz_size_t _size, dz_userdata_t _ud )
{
    DZ_UNUSED( _ud );

    void * p = realloc( _ptr, _size );

    return p;
}
//////////////////////////////////////////////////////////////////////////
static void dz_free( const void * _ptr, dz_userdata_t _ud )
{
    DZ_UNUSED( _ud );

    free( (void *)_ptr );
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
static dz_result_t __set_affector_timeline( dz_service_t * _service, dz_affector_data_t * _affector_data, dz_affector_data_timeline_type_e _type, float _value )
{
    dz_timeline_key_t * timeline;
    if( dz_timeline_key_create( _service, &timeline, 0.f, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    if( dz_timeline_key_const_set_value( timeline, _value ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    dz_affector_data_set_timeline( _affector_data, _type, timeline );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
int main( int argc, char ** argv )
{
    DZ_UNUSED( argc );
    DZ_UNUSED( argv );

    dz_service_providers_t providers;
    providers.f_malloc = &dz_malloc;
    providers.f_realloc = &dz_realloc;
    providers.f_free = &dz_free;
    providers.f_cosf = &dz_cosf;
    providers.f_sinf = &dz_sinf;

    dz_service_t * service;
    if( dz_service_create( &service, &providers, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return EXIT_FAILURE;
    }

    dz_emitter_data_t * emitter_data;
    if( dz_emitter_data_create( service, &emitter_data, DZ_EMITTER_SHAPE_POINT, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return EXIT_FAILURE;
    }

    dz_emitter_data_set_life( emitter_data, 1000.f );

    dz_timeline_key_t * emitter_spawn_delay;
    if( dz_timeline_key_create( service, &emitter_spawn_delay, 0.f, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return EXIT_FAILURE;
    }

    dz_timeline_key_const_set_value( emitter_spawn_delay, 50.f );

    dz_emitter_data_set_timeline_key_spawn_delay( emitter_data, emitter_spawn_delay );

    dz_timeline_key_t * emitter_spawn_count;
    if( dz_timeline_key_create( service, &emitter_spawn_count, 0.f, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return EXIT_FAILURE;
    }

    dz_timeline_key_const_set_value( emitter_spawn_count, 1.f );

    dz_emitter_data_set_timeline_key_spawn_count( emitter_data, emitter_spawn_count );

    dz_affector_data_t * affector_data;
    if( dz_affector_data_create( service, &affector_data ) == DZ_FAILURE )
    {
        return EXIT_FAILURE;
    }

    if( __set_affector_timeline( service, affector_data, DZ_AFFECTOR_DATA_TIMELINE_LIFE, 300.f ) == DZ_FAILURE )
    {
        return EXIT_FAILURE;
    }

    dz_emitter_data_destroy( service, emitter_data );
    dz_service_destroy( service );

    return EXIT_SUCCESS;
}