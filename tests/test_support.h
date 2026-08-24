#ifndef DAZZLE_TEST_SUPPORT_H_
#define DAZZLE_TEST_SUPPORT_H_

#include "dazzle/dazzle.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct dz_test_memory_t
{
    dz_size_t allocated;
    dz_size_t fail_after;
    dz_size_t allocation_count;
} dz_test_memory_t;

static void * dz_test_malloc( dz_size_t _size, dz_userdata_t _ud )
{
    dz_test_memory_t * memory = (dz_test_memory_t *)_ud;
    dz_size_t * allocation = (dz_size_t *)malloc( _size + sizeof( dz_size_t ) );
    allocation[0] = _size;
    memory->allocated += _size;
    return allocation + 1;
}

static void * dz_test_realloc( void * _ptr, dz_size_t _size, dz_userdata_t _ud )
{
    dz_test_memory_t * memory = (dz_test_memory_t *)_ud;
    dz_size_t old_size = 0;
    dz_size_t * old_allocation = DZ_NULLPTR;
    if( _ptr != DZ_NULLPTR )
    {
        old_allocation = (dz_size_t *)_ptr - 1;
        old_size = old_allocation[0];
    }
    dz_size_t * allocation = (dz_size_t *)realloc( old_allocation, _size + sizeof( dz_size_t ) );
    allocation[0] = _size;
    memory->allocated = memory->allocated - old_size + _size;
    return allocation + 1;
}

static void dz_test_free( const void * _ptr, dz_userdata_t _ud )
{
    if( _ptr == DZ_NULLPTR )
    {
        return;
    }
    dz_test_memory_t * memory = (dz_test_memory_t *)_ud;
    dz_size_t * allocation = (dz_size_t *)_ptr - 1;
    memory->allocated -= allocation[0];
    free( allocation );
}

static dz_float_t dz_test_sqrtf( dz_float_t _value, dz_userdata_t _ud )
{
    DZ_UNUSED( _ud );
    return sqrtf( _value );
}
static dz_float_t dz_test_cosf( dz_float_t _value, dz_userdata_t _ud )
{
    DZ_UNUSED( _ud );
    return cosf( _value );
}
static dz_float_t dz_test_sinf( dz_float_t _value, dz_userdata_t _ud )
{
    DZ_UNUSED( _ud );
    return sinf( _value );
}
static dz_float_t dz_test_atan2f( dz_float_t _y, dz_float_t _x, dz_userdata_t _ud )
{
    DZ_UNUSED( _ud );
    return atan2f( _y, _x );
}
static dz_float_t dz_test_asinf( dz_float_t _value, dz_userdata_t _ud )
{
    DZ_UNUSED( _ud );
    return asinf( _value );
}
static dz_float_t dz_test_tanf( dz_float_t _value, dz_userdata_t _ud )
{
    DZ_UNUSED( _ud );
    return tanf( _value );
}

static void dz_test_service_create( dz_service_t ** _service, dz_test_memory_t * _memory )
{
    dz_service_providers_t providers;
    providers.f_malloc = &dz_test_malloc;
    providers.f_realloc = &dz_test_realloc;
    providers.f_free = &dz_test_free;
    providers.f_sqrtf = &dz_test_sqrtf;
    providers.f_cosf = &dz_test_cosf;
    providers.f_sinf = &dz_test_sinf;
    providers.f_atan2f = &dz_test_atan2f;
    providers.f_asinf = &dz_test_asinf;
    providers.f_tanf = &dz_test_tanf;
    dz_service_create( _service, &providers, _memory );
}

#define DZ_TEST_CHECK( expression )                                                                                                                                                \
    do                                                                                                                                                                             \
    {                                                                                                                                                                              \
        if( !( expression ) )                                                                                                                                                      \
        {                                                                                                                                                                          \
            fprintf( stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #expression );                                                                                       \
            return EXIT_FAILURE;                                                                                                                                                   \
        }                                                                                                                                                                          \
    } while( 0 )

#endif
