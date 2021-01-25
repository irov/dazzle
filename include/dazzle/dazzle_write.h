#ifndef DZ_DAZZLE_WRITE_H_
#define DZ_DAZZLE_WRITE_H_

#include "dazzle/config.h"
#include "dazzle/dazzle.h"

typedef dz_result_t( *dz_stream_write_t )(const void * _data, dz_size_t _size, dz_userdata_t _ud);

dz_result_t dz_header_write( dz_stream_write_t _write, dz_userdata_t _ud );
dz_result_t dz_effect_write( const dz_effect_t * _effect, dz_stream_write_t _write, dz_userdata_t _ud );

#endif
