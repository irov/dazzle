#ifndef DZ_DAZZLE_H_
#define DZ_DAZZLE_H_

#include "dazzle/config.h"

typedef struct dz_service_t dz_service_t;

typedef void * (*dz_malloc_t)(dz_size_t _size, dz_userdata_t _ud);
typedef void * (*dz_realloc_t)(void * _ptr, dz_size_t _size, dz_userdata_t _ud);
typedef void (*dz_free_t)(void * _ptr, dz_userdata_t _ud);
typedef float (*dz_randomize_t)(float _r, dz_userdata_t _ud);

typedef struct dz_service_providers_t
{
    dz_malloc_t f_malloc;
    dz_realloc_t f_realloc;
    dz_free_t f_free;
    dz_randomize_t f_randomize;
} dz_service_providers_t;

dz_result_t dz_service_create( dz_service_t ** _service, const dz_service_providers_t * _providers, dz_userdata_t _ud );
void dz_service_destroy( dz_service_t * _service );

typedef struct dz_emitter_data_t dz_emitter_data_t;

typedef enum dz_emitter_shape_type_e
{
    DZ_EMITTER_SHAPE_POINT,
    DZ_EMITTER_SHAPE_CIRCLE,
    DZ_EMITTER_SHAPE_LINE
} dz_emitter_shape_type_e;

dz_result_t dz_emitter_data_create( dz_service_t * _service, dz_emitter_data_t ** _data, dz_emitter_shape_type_e _type, dz_userdata_t _ud );
void dz_emitter_data_destroy( dz_service_t * _service, dz_emitter_data_t * _emitter_data );


#endif
