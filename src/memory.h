#ifndef DZ_MEMORY_H_
#define DZ_MEMORY_H_

#include "dazzle/config.h"

void dz_memory_copy( void * _destination, const void * _source, dz_size_t _size );
void dz_memory_zero( void * _destination, dz_size_t _size );

#endif
