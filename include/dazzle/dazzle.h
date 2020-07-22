#ifndef DZ_DAZZLE_H_
#define DZ_DAZZLE_H_

#include "dazzle/config.h"

typedef struct dz_service_t dz_service_t;

typedef void * (*dz_malloc_t)(dz_size_t _size, dz_userdata_t _ud);
typedef void * (*dz_realloc_t)(void * _ptr, dz_size_t _size, dz_userdata_t _ud);
typedef void (*dz_free_t)(void * _ptr, dz_userdata_t _ud);
typedef float (*dz_cosf_t)(float _a, dz_userdata_t _ud);
typedef float (*dz_sinf_t)(float _a, dz_userdata_t _ud);
typedef float (*dz_randomize_t)(float _r, dz_userdata_t _ud);

typedef struct dz_service_providers_t
{
    dz_malloc_t f_malloc;
    dz_realloc_t f_realloc;
    dz_free_t f_free;
    dz_cosf_t f_cosf;
    dz_sinf_t f_sinf;
    dz_randomize_t f_randomize;
} dz_service_providers_t;

dz_result_t dz_service_create( dz_service_t ** _service, const dz_service_providers_t * _providers, dz_userdata_t _ud );
void dz_service_destroy( dz_service_t * _service );

void dz_service_get_providers( dz_service_t * _service, dz_service_providers_t * _providers );

typedef struct dz_timeline_key_t dz_timeline_key_t;

dz_result_t dz_timeline_key_create( dz_service_t * _service, dz_timeline_key_t ** _key, float _time, float _value, dz_userdata_t _ud );
void dz_timeline_key_destroy( dz_service_t * _service, dz_timeline_key_t * _key );

typedef enum dz_timeline_interpolate_type_e
{
    DZ_TIMELINE_INTERPOLATE_NONE,
    DZ_TIMELINE_INTERPOLATE_RANDOMIZE,
    DZ_TIMELINE_INTERPOLATE_BEZIER2
} dz_timeline_interpolate_type_e;

typedef struct dz_timeline_interpolate_t dz_timeline_interpolate_t;

dz_result_t dz_timeline_interpolate_create( dz_service_t * _service, dz_timeline_interpolate_t ** _interpolate, dz_timeline_interpolate_type_e _type, dz_userdata_t _ud );
void dz_timeline_interpolate_destroy( dz_service_t * _service, dz_timeline_interpolate_t * _interpolate );

typedef struct dz_affector_data_t dz_affector_data_t;

dz_result_t dz_affector_data_create( dz_service_t * _service, dz_affector_data_t ** _affector_data );
void dz_affector_data_destory( dz_service_t * _service, dz_affector_data_t * _affector_data );

typedef struct dz_emitter_data_t dz_emitter_data_t;

typedef enum dz_emitter_shape_type_e
{
    DZ_EMITTER_SHAPE_POINT,
    DZ_EMITTER_SHAPE_CIRCLE,
    DZ_EMITTER_SHAPE_LINE
} dz_emitter_shape_type_e;

dz_result_t dz_emitter_data_create( dz_service_t * _service, dz_emitter_data_t ** _data, dz_emitter_shape_type_e _type, dz_userdata_t _ud );
void dz_emitter_data_destroy( dz_service_t * _service, dz_emitter_data_t * _emitter_data );

typedef struct dz_emitter_t dz_emitter_t;

dz_result_t dz_emitter_create( dz_service_t * _service, const dz_emitter_data_t * _emitter_data, const dz_affector_data_t * _affector_data, dz_emitter_t ** _emitter );
void dz_emitter_destroy( dz_service_t * _service, dz_emitter_t * _emitter );

void dz_emitter_update( dz_service_t * _service, dz_emitter_t * _emitter, float _time );

#endif
