#include "memory.h"

//////////////////////////////////////////////////////////////////////////
void dz_memory_copy( void * _destination, const void * _source, dz_size_t _size )
{
    dz_uint8_t * destination = (dz_uint8_t *)_destination;
    const dz_uint8_t * source = (const dz_uint8_t *)_source;

    for( dz_size_t index = 0U; index != _size; ++index )
    {
        destination[index] = source[index];
    }
}
//////////////////////////////////////////////////////////////////////////
void dz_memory_zero( void * _destination, dz_size_t _size )
{
    dz_uint8_t * destination = (dz_uint8_t *)_destination;

    for( dz_size_t index = 0U; index != _size; ++index )
    {
        destination[index] = 0U;
    }
}
