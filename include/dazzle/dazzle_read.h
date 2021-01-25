#ifndef DZ_DAZZLE_READ_H_
#define DZ_DAZZLE_READ_H_

#include "dazzle/config.h"
#include "dazzle/dazzle.h"

typedef dz_result_t( *dz_stream_read_t )(void * const _data, dz_size_t _size, dz_userdata_t _ud);

typedef enum dz_effect_read_status_e
{
    DZ_EFFECT_LOAD_STATUS_SUCCESSFUL = 0x00000000,
    DZ_EFFECT_LOAD_STATUS_INVALID_MAGIC = 0x00000001,
    DZ_EFFECT_LOAD_STATUS_INVALID_VERSION = 0x00000002,
} dz_effect_read_status_e;

dz_result_t dz_header_read( dz_stream_read_t _load, dz_userdata_t _ud, dz_effect_read_status_e * const _status );
dz_result_t dz_effect_read( const dz_service_t * _service, dz_effect_t ** _effect, dz_stream_read_t _read, dz_userdata_t _ud );

#endif
