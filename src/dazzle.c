#include "dazzle/dazzle.h"

#include "service.h"
#include "timeline_key.h"
#include "timeline_interpolate.h"
#include "affector_data.h"
#include "emitter_data.h"
#include "particle.h"
#include "texture.h"
#include "material.h"
#include "emitter.h"

#include "alloc.h"
#include "math.h"

//////////////////////////////////////////////////////////////////////////
dz_result_t dz_service_create( dz_service_t ** _service, const dz_service_providers_t * _providers, dz_userdata_t _ud )
{
    dz_service_t * service = (dz_service_t *)(*_providers->f_malloc)(sizeof( dz_service_t ), _ud);

    service->providers = *_providers;
    service->ud = _ud;

    *_service = service;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_service_destroy( dz_service_t * _service )
{
    DZ_FREE( _service, _service );
}
//////////////////////////////////////////////////////////////////////////
void dz_service_get_providers( dz_service_t * _service, dz_service_providers_t * _providers )
{
    *_providers = _service->providers;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_texture_create( dz_service_t * _service, dz_texture_t ** _texture, dz_userdata_t _ud )
{
    dz_texture_t * texture = DZ_NEW( _service, dz_texture_t );

    texture->u[0] = 0.f;
    texture->v[0] = 0.f;
    texture->u[1] = 1.f;
    texture->v[1] = 0.f;
    texture->u[2] = 1.f;
    texture->v[2] = 1.f;
    texture->u[3] = 1.f;
    texture->v[3] = 0.f;

    texture->ud = _ud;

    *_texture = texture;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_texture_destroy( dz_service_t * _service, const dz_texture_t * _texture )
{
    DZ_FREE( _service, _texture );
}
//////////////////////////////////////////////////////////////////////////
dz_userdata_t dz_texture_get_ud( const dz_texture_t * _texture )
{
    return _texture->ud;
}
//////////////////////////////////////////////////////////////////////////
void dz_texture_set_uv( dz_texture_t * _texture, const float * _u, const float * _v )
{
    _texture->u[0] = _u[0];
    _texture->v[0] = _v[0];
    _texture->u[1] = _u[1];
    _texture->v[1] = _v[1];
    _texture->u[2] = _u[2];
    _texture->v[2] = _v[2];
    _texture->u[3] = _u[3];
    _texture->v[3] = _v[3];
}
//////////////////////////////////////////////////////////////////////////
void dz_texture_get_uv( const dz_texture_t * _texture, float * _u, float * _v )
{
    _u[0] = _texture->u[0];
    _v[0] = _texture->v[0];
    _u[1] = _texture->u[1];
    _v[1] = _texture->v[1];
    _u[2] = _texture->u[2];
    _v[2] = _texture->v[2];
    _u[3] = _texture->u[3];
    _v[3] = _texture->v[3];
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_material_create( dz_service_t * _service, dz_material_t ** _material, dz_userdata_t _ud )
{
    dz_material_t * material = DZ_NEW( _service, dz_material_t );

    material->blend_type = DZ_BLEND_NORNAL;

    material->r = 1.f;
    material->g = 1.f;
    material->b = 1.f;
    material->a = 1.f;

    material->texture = DZ_NULLPTR;

    material->ud = _ud;

    *_material = material;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_material_destroy( dz_service_t * _service, const dz_material_t * _material )
{
    DZ_FREE( _service, _material );
}
//////////////////////////////////////////////////////////////////////////
dz_userdata_t dz_material_get_ud( const dz_material_t * _material )
{
    return _material->ud;
}
//////////////////////////////////////////////////////////////////////////
void dz_material_set_blend( dz_material_t * _material, dz_blend_type_e _blend )
{
    _material->blend_type = _blend;
}
//////////////////////////////////////////////////////////////////////////
dz_blend_type_e dz_material_get_blend( const dz_material_t * _material )
{
    return _material->blend_type;
}
//////////////////////////////////////////////////////////////////////////
void dz_material_set_color( dz_material_t * _material, float _r, float _g, float _b, float _a )
{
    _material->r = _r;
    _material->g = _g;
    _material->b = _b;
    _material->a = _a;
}
//////////////////////////////////////////////////////////////////////////
void dz_material_get_color( const dz_material_t * _material, float * _r, float * _g, float * _b, float * _a )
{
    *_r = _material->r;
    *_g = _material->g;
    *_b = _material->b;
    *_a = _material->a;
}
//////////////////////////////////////////////////////////////////////////
void dz_material_set_texture( dz_material_t * _material, const dz_texture_t * _texture )
{
    _material->texture = _texture;
}
//////////////////////////////////////////////////////////////////////////
const dz_texture_t * dz_material_get_texture( dz_material_t * _material )
{
    return _material->texture;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_timeline_interpolate_create( dz_service_t * _service, dz_timeline_interpolate_t ** _interpolate, dz_timeline_interpolate_type_e _type, dz_userdata_t _ud )
{
    dz_timeline_interpolate_t * interpolate;

    switch( _type )
    {
    case DZ_TIMELINE_INTERPOLATE_LINEAR:
        {
            interpolate = DZ_NEWV( _service, dz_timeline_interpolate_t, dz_timeline_interpolate_linear_t );
        }break;
    case DZ_TIMELINE_INTERPOLATE_BEZIER2:
        {
            interpolate = DZ_NEWV( _service, dz_timeline_interpolate_t, dz_timeline_interpolate_bezier2_t );
        }break;
    case __DZ_TIMELINE_INTERPOLATE_MAX__:
    default:
        {
            return DZ_FAILURE;
        }break;
    }

    interpolate->type = _type;
    interpolate->key = DZ_NULLPTR;
    interpolate->ud = _ud;

    *_interpolate = interpolate;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_timeline_interpolate_destroy( dz_service_t * _service, const dz_timeline_interpolate_t * _interpolate )
{
    if( _interpolate->key != DZ_NULLPTR )
    {
        dz_timeline_key_destroy( _service, _interpolate->key );
    }

    DZ_FREE( _service, _interpolate );
}
//////////////////////////////////////////////////////////////////////////
dz_userdata_t dz_timeline_interpolate_get_ud( const dz_timeline_interpolate_t * _key )
{
    return _key->ud;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_timeline_key_create( dz_service_t * _service, dz_timeline_key_t ** _key, float _p, dz_timeline_key_type_e _type, dz_userdata_t _ud )
{
#ifdef DZ_DEBUG
    if( _p < 0.f || _p > 1.f )
    {
        return DZ_FAILURE;
    }
#endif

    dz_timeline_key_t * key;

    switch( _type )
    {
    case DZ_TIMELINE_KEY_CONST:
        {
            key = DZ_NEWV( _service, dz_timeline_key_t, dz_timeline_key_const_t );
        }break;
    case DZ_TIMELINE_KEY_RANDOMIZE:
        {
            key = DZ_NEWV( _service, dz_timeline_key_t, dz_timeline_key_const_t );
        }break;
    case __DZ_TIMELINE_KEY_MAX__:
    default:
        {
            return DZ_FAILURE;
        }break;
    }

    key->p = _p;
    key->type = _type;
    key->interpolate = DZ_NULLPTR;
    key->ud = _ud;

    *_key = key;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_timeline_key_destroy( dz_service_t * _service, const dz_timeline_key_t * _key )
{
    if( _key->interpolate != DZ_NULLPTR )
    {
        dz_timeline_interpolate_destroy( _service, _key->interpolate );
    }

    DZ_FREE( _service, _key );
}
//////////////////////////////////////////////////////////////////////////
dz_userdata_t dz_timeline_key_get_ud( const dz_timeline_key_t * _key )
{
    return _key->ud;
}
//////////////////////////////////////////////////////////////////////////
const dz_timeline_key_t * dz_timeline_interpolate_get_key( const dz_timeline_interpolate_t * _interpolate )
{
    const dz_timeline_key_t * key = _interpolate->key;

    return key;
}
//////////////////////////////////////////////////////////////////////////
const dz_timeline_interpolate_t * dz_timeline_key_get_interpolate( const dz_timeline_key_t * _key )
{
    const dz_timeline_interpolate_t * interpolate = _key->interpolate;

    return interpolate;
}
//////////////////////////////////////////////////////////////////////////
void dz_timeline_key_set_interpolate( dz_timeline_key_t * _key0, dz_timeline_interpolate_t * _interpolate, dz_timeline_key_t * _key1 )
{
    _key0->interpolate = _interpolate;
    _interpolate->key = _key1;
}
//////////////////////////////////////////////////////////////////////////
float dz_timeline_key_get_p( const dz_timeline_key_t * _key )
{
    float p = _key->p;

    return p;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_timeline_key_const_set_value( dz_timeline_key_t * _key, float _value )
{
    dz_timeline_key_const_t * key_const = (dz_timeline_key_const_t *)_key;

    key_const->value = _value;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_timeline_key_const_get_value( const dz_timeline_key_t * _key, float * _value )
{
    const dz_timeline_key_const_t * key_const = (const dz_timeline_key_const_t *)_key;

    *_value = key_const->value;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_timeline_key_randomize_set_min_max( dz_timeline_key_t * _key, float _min, float _max )
{
    dz_timeline_key_randomize_t * key_randomize = (dz_timeline_key_randomize_t *)_key;

    key_randomize->value_min = _min;
    key_randomize->value_max = _max;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_timeline_key_randomize_get_min_max( const dz_timeline_key_t * _key, float * _min, float * _max )
{
    const dz_timeline_key_randomize_t * key_randomize = (const dz_timeline_key_randomize_t *)_key;

    *_min = key_randomize->value_min;
    *_max = key_randomize->value_max;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_affector_data_create( dz_service_t * _service, dz_affector_data_t ** _affector_data, dz_userdata_t _ud )
{
    dz_affector_data_t * affector_data = DZ_NEW( _service, dz_affector_data_t );

    for( uint32_t index = 0; index != __DZ_AFFECTOR_DATA_TIMELINE_MAX__; ++index )
    {
        affector_data->timelines[index] = DZ_NULLPTR;
    }

    affector_data->ud = _ud;

    *_affector_data = affector_data;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_affector_data_destroy( dz_service_t * _service, const dz_affector_data_t * _affector_data )
{
    for( uint32_t index = 0; index != __DZ_AFFECTOR_DATA_TIMELINE_MAX__; ++index )
    {
        const dz_timeline_key_t * timeline = _affector_data->timelines[index];
        dz_timeline_key_destroy( _service, timeline );
    }

    DZ_FREE( _service, _affector_data );
}
//////////////////////////////////////////////////////////////////////////
dz_userdata_t dz_affector_data_get_ud( const dz_affector_data_t * _affector_data )
{
    return _affector_data->ud;
}
//////////////////////////////////////////////////////////////////////////
typedef struct dz_timeline_limits_t
{
    float min_value;
    float max_value;
} dz_timeline_limits_t;
//////////////////////////////////////////////////////////////////////////
void get_timeline_limits( dz_affector_data_timeline_type_e _timeline, float * _min, float * _max )
{
    const dz_timeline_limits_t limits[__DZ_AFFECTOR_DATA_TIMELINE_MAX__] = {
        {0.f, DZ_FLT_MAX}, //DZ_AFFECTOR_DATA_TIMELINE_LIFE
        {0.f, DZ_FLT_MAX}, //DZ_AFFECTOR_DATA_TIMELINE_MOVE_SPEED
        {DZ_FLT_MIN, DZ_FLT_MAX}, //DZ_AFFECTOR_DATA_TIMELINE_MOVE_ACCELERATE
        {DZ_FLT_MIN, DZ_FLT_MAX}, //DZ_AFFECTOR_DATA_TIMELINE_ROTATE_SPEED
        {DZ_FLT_MIN, DZ_FLT_MAX}, //DZ_AFFECTOR_DATA_TIMELINE_ROTATE_ACCELERATE
        {DZ_FLT_MIN, DZ_FLT_MAX}, //DZ_AFFECTOR_DATA_TIMELINE_SPIN_SPEED
        {DZ_FLT_MIN, DZ_FLT_MAX}, //DZ_AFFECTOR_DATA_TIMELINE_SPIN_ACCELERATE
        {DZ_FLT_MIN, DZ_FLT_MAX}, //DZ_AFFECTOR_DATA_TIMELINE_STRAFE_SPEED
        {DZ_FLT_MIN, DZ_FLT_MAX}, //DZ_AFFECTOR_DATA_TIMELINE_STRAFE_FRENQUENCE
        {DZ_FLT_MIN, DZ_FLT_MAX}, //DZ_AFFECTOR_DATA_TIMELINE_STRAFE_SIZE
        {DZ_PI2N, DZ_PI2}, //DZ_AFFECTOR_DATA_TIMELINE_STRAFE_SHIFT
        {0.f, DZ_FLT_MAX}, //DZ_AFFECTOR_DATA_TIMELINE_SIZE
        {0.f, 1.f}, //DZ_AFFECTOR_DATA_TIMELINE_COLOR_R
        {0.f, 1.f}, //DZ_AFFECTOR_DATA_TIMELINE_COLOR_G
        {0.f, 1.f}, //DZ_AFFECTOR_DATA_TIMELINE_COLOR_B
        {0.f, 1.f}, //DZ_AFFECTOR_DATA_TIMELINE_COLOR_A
    };

    const dz_timeline_limits_t * limit = limits + _timeline;

    *_min = limit->min_value;
    *_max = limit->max_value;
}
//////////////////////////////////////////////////////////////////////////
void dz_affector_data_set_timeline( dz_affector_data_t * _affector_data, dz_affector_data_timeline_type_e _type, const dz_timeline_key_t * _timeline )
{
    _affector_data->timelines[_type] = _timeline;
}
//////////////////////////////////////////////////////////////////////////
const dz_timeline_key_t * dz_affector_data_get_timeline( const dz_affector_data_t * _affector_data, dz_affector_data_timeline_type_e _type )
{
    const dz_timeline_key_t * timeline = _affector_data->timelines[_type];

    return timeline;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_shape_data_create( dz_service_t * _service, dz_shape_data_t ** _shape_data, dz_shape_data_type_e _type, dz_userdata_t _ud )
{
    dz_shape_data_t * shape = DZ_NEW( _service, dz_shape_data_t );

    shape->type = _type;

    for( uint32_t index = 0; index != __DZ_SHAPE_DATA_TIMELINE_MAX__; ++index )
    {
        shape->timelines[index] = DZ_NULLPTR;
    }

    shape->triangles = DZ_NULLPTR;
    shape->triangle_count = 0;

    shape->mask_buffer = DZ_NULLPTR;
    shape->mask_bites = 0;
    shape->mask_pitch = 0;
    shape->mask_width = 0;
    shape->mask_height = 0;
    shape->mask_threshold = 0;
    shape->mask_scale = 1.f;


    shape->ud = _ud;

    *_shape_data = shape;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_shape_data_destroy( dz_service_t * _service, const dz_shape_data_t * _shape_data )
{
    for( uint32_t index = 0; index != __DZ_SHAPE_DATA_TIMELINE_MAX__; ++index )
    {
        const dz_timeline_key_t * timeline = _shape_data->timelines[index];

        if( timeline == DZ_NULLPTR )
        {
            continue;
        }

        dz_timeline_key_destroy( _service, timeline );
    }

    DZ_FREE( _service, _shape_data );
}
//////////////////////////////////////////////////////////////////////////
dz_userdata_t dz_shape_data_get_ud( const dz_shape_data_t * _shape_data )
{
    return _shape_data->ud;
}
//////////////////////////////////////////////////////////////////////////
void dz_shape_data_set_timeline( dz_shape_data_t * _shape, dz_shape_data_timeline_type_e _type, const dz_timeline_key_t * _timeline )
{
    _shape->timelines[_type] = _timeline;
}
//////////////////////////////////////////////////////////////////////////
const dz_timeline_key_t * dz_shape_data_get_timeline( const dz_shape_data_t * _shape, dz_shape_data_timeline_type_e _type )
{
    const dz_timeline_key_t * timeline = _shape->timelines[_type];

    return timeline;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_shape_data_set_polygon( dz_shape_data_t * _shape, const float * _triangles, uint32_t _count )
{
    _shape->triangles = _triangles;
    _shape->triangle_count = _count;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_shape_data_get_polygon( const dz_shape_data_t * _shape, const float ** _triangles, uint32_t * _count )
{
    *_triangles = _shape->triangles;
    *_count = _shape->triangle_count;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_shape_data_set_mask( dz_shape_data_t * _shape, const void * _buffer, uint32_t _bites, uint32_t _pitch, uint32_t _width, uint32_t _height )
{
#ifdef DZ_DEBUG
    if( _bites != 1 && _bites != 2 && _bites != 4 )
    {
        return DZ_FAILURE;
    }
#endif

    _shape->mask_buffer = _buffer;
    _shape->mask_bites = _bites;
    _shape->mask_pitch = _pitch;
    _shape->mask_width = _width;
    _shape->mask_height = _height;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_shape_data_get_mask( const dz_shape_data_t * _shape, const void ** _buffer, uint32_t * _bites, uint32_t * _pitch, uint32_t * _width, uint32_t * _height )
{
    *_buffer = _shape->mask_buffer;
    *_bites = _shape->mask_bites;
    *_pitch = _shape->mask_pitch;
    *_width = _shape->mask_width;
    *_height = _shape->mask_height;
}
//////////////////////////////////////////////////////////////////////////
void dz_shape_data_set_mask_scale( dz_shape_data_t * _shape, float _scale )
{
    _shape->mask_scale = _scale;
}
//////////////////////////////////////////////////////////////////////////
float dz_shape_data_get_mask_scale( const dz_shape_data_t * _shape )
{
    return _shape->mask_scale;
}
//////////////////////////////////////////////////////////////////////////
void dz_shape_data_set_mask_threshold( dz_shape_data_t * _shape, uint32_t _threshold )
{
    _shape->mask_threshold = _threshold;
}
//////////////////////////////////////////////////////////////////////////
uint32_t dz_shape_data_get_mask_threshold( const dz_shape_data_t * _shape )
{
    return _shape->mask_threshold;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_emitter_data_create( dz_service_t * _service, dz_emitter_data_t ** _emitter_data, dz_userdata_t _ud )
{
    dz_emitter_data_t * emitter_data = DZ_NEW( _service, dz_emitter_data_t );

    emitter_data->life = 0.f;

    for( uint32_t index = 0; index != __DZ_EMITTER_DATA_TIMELINE_MAX__; ++index )
    {
        emitter_data->timelines[index] = DZ_NULLPTR;
    }

    emitter_data->ud = _ud;

    *_emitter_data = emitter_data;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_emitter_data_destroy( dz_service_t * _service, const dz_emitter_data_t * _emitter_data )
{
    for( uint32_t index = 0; index != __DZ_EMITTER_DATA_TIMELINE_MAX__; ++index )
    {
        const dz_timeline_key_t * timeline = _emitter_data->timelines[index];

        if( timeline == DZ_NULLPTR )
        {
            continue;
        }

        dz_timeline_key_destroy( _service, timeline );
    }

    DZ_FREE( _service, _emitter_data );
}
//////////////////////////////////////////////////////////////////////////
dz_userdata_t dz_emitter_data_get_ud( const dz_emitter_data_t * _emitter_data )
{
    return _emitter_data->ud;
}
//////////////////////////////////////////////////////////////////////////
void dz_emitter_data_set_life( dz_emitter_data_t * _emitter_data, float _life )
{
    _emitter_data->life = _life;
}
//////////////////////////////////////////////////////////////////////////
float dz_emitter_data_get_life( const dz_emitter_data_t * _emitter_data )
{
    return _emitter_data->life;
}
//////////////////////////////////////////////////////////////////////////
void dz_emitter_data_set_timeline( dz_emitter_data_t * _emitter_data, dz_emitter_data_timeline_type_e _type, const dz_timeline_key_t * _timeline )
{
    _emitter_data->timelines[_type] = _timeline;
}
//////////////////////////////////////////////////////////////////////////
const dz_timeline_key_t * dz_emitter_data_get_timeline( const dz_emitter_data_t * _emitter_data, dz_emitter_data_timeline_type_e _type )
{
    const dz_timeline_key_t * timeline = _emitter_data->timelines[_type];

    return timeline;
}
//////////////////////////////////////////////////////////////////////////
static uint16_t __get_rand( uint32_t * _seed )
{
    uint32_t value = (*_seed * 1103515245U) + 12345U;

    *_seed = value;

    return value & 0xffff;
}
//////////////////////////////////////////////////////////////////////////
static float __get_randf( uint32_t * _seed )
{
    uint16_t value = __get_rand( _seed );

    const float inv_65535 = 1.f / 65535.f;

    float valuef = (float)value * inv_65535;

    return valuef;
}
//////////////////////////////////////////////////////////////////////////
static float __get_randf2( uint32_t * _seed, float _min, float _max )
{
    uint16_t value = __get_rand( _seed );

    const float inv_65535 = 1.f / 65535.f;

    float valuef = (float)value * inv_65535;

    return _min + (_max - _min) * valuef;
}
//////////////////////////////////////////////////////////////////////////
static float __get_timeline_key_value( float _t, const dz_timeline_key_t * _key )
{
    switch( _key->type )
    {
    case DZ_TIMELINE_KEY_CONST:
        {
            const dz_timeline_key_const_t * key_const = (const dz_timeline_key_const_t *)_key;

            float value = key_const->value;

            return value;
        }break;
    case DZ_TIMELINE_KEY_RANDOMIZE:
        {
            const dz_timeline_key_randomize_t * key_randomize = (const dz_timeline_key_randomize_t *)_key;

            float value = key_randomize->value_min + (key_randomize->value_max - key_randomize->value_min) * _t;

            return value;
        }break;
    case __DZ_TIMELINE_KEY_MAX__:
    default:
        break;
    };

    return 0.f;
}
//////////////////////////////////////////////////////////////////////////
static float __get_timeline_value( float _t, const dz_timeline_key_t * _key, float _life, float _time )
{
    for( ; _key->interpolate != DZ_NULLPTR && _key->interpolate->key->p * _life < _time; _key = _key->interpolate->key );

    float current_value = __get_timeline_key_value( _t, _key );

    if( _key->interpolate == DZ_NULLPTR ||
        _key->p * _life > _time )
    {
        return current_value;
    }

    const dz_timeline_key_t * next = _key->interpolate->key;

    float next_value = __get_timeline_key_value( _t, next );

    float t = (_time - _key->p * _life) / (next->p * _life - _key->p * _life);

    float value = current_value + (next_value - current_value) * t;

    return value;    
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_emitter_create( dz_service_t * _service, dz_emitter_t ** _emitter, const dz_material_t * _material, const dz_shape_data_t * _shape_data, const dz_emitter_data_t * _emitter_data, const dz_affector_data_t * _affector_data, uint32_t _seed, float _life, dz_userdata_t _ud )
{
    dz_emitter_t * emitter = DZ_NEW( _service, dz_emitter_t );

    emitter->material = _material;
    emitter->shape_data = _shape_data;
    emitter->emitter_data = _emitter_data;
    emitter->affector_data = _affector_data;

    emitter->init_seed = _seed;
    emitter->seed = _seed;

    emitter->partices = DZ_NULLPTR;
    emitter->partices_count = 0;
    emitter->partices_capacity = 0;
    emitter->particle_limit = ~1U;

    emitter->life = _life;

    emitter->time = 0.f;
    emitter->emitter_time = 0.f;

    emitter->ud = _ud;

    *_emitter = emitter;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_emitter_destroy( dz_service_t * _service, const dz_emitter_t * _emitter )
{
    DZ_FREE( _service, _emitter );
}
//////////////////////////////////////////////////////////////////////////
dz_userdata_t dz_emitter_get_ud( const dz_emitter_t * _emitter )
{
    return _emitter->ud;
}
//////////////////////////////////////////////////////////////////////////
uint32_t dz_emitter_get_seed( const dz_emitter_t * _emitter )
{
    return _emitter->init_seed;
}
//////////////////////////////////////////////////////////////////////////
void dz_emitter_set_particle_limit( dz_emitter_t * _emitter, uint32_t _limit )
{
    _emitter->particle_limit = _limit;
}
//////////////////////////////////////////////////////////////////////////
uint32_t dz_emitter_get_particle_limit( const dz_emitter_t * _emitter )
{
    return _emitter->particle_limit;
}
//////////////////////////////////////////////////////////////////////////
static float __get_affector_value_rands( dz_particle_t * _p, dz_emitter_t * _emitter, dz_affector_data_timeline_type_e _type, float _default )
{
    const dz_timeline_key_t * timeline_key = _emitter->affector_data->timelines[_type];

    if( timeline_key == DZ_NULLPTR )
    {
        return _default;
    }

    float time = _p->time;

    float value = __get_timeline_value( _p->rands[_type], timeline_key, _p->life, time );

    return value;
}
//////////////////////////////////////////////////////////////////////////
static void __particle_update( dz_service_t * _service, dz_emitter_t * _emitter, dz_particle_t * _p, float _time )
{
    _p->time += _time;

    float move_speed = __get_affector_value_rands( _p, _emitter, DZ_AFFECTOR_DATA_TIMELINE_MOVE_SPEED, 1.f );
    float move_accelerate = __get_affector_value_rands( _p, _emitter, DZ_AFFECTOR_DATA_TIMELINE_MOVE_ACCELERATE, 0.f );

    float rotate_speed = __get_affector_value_rands( _p, _emitter, DZ_AFFECTOR_DATA_TIMELINE_ROTATE_SPEED, 0.f );
    float rotate_accelerate = __get_affector_value_rands( _p, _emitter, DZ_AFFECTOR_DATA_TIMELINE_ROTATE_ACCELERATE, 0.f );

    float spin_speed = __get_affector_value_rands( _p, _emitter, DZ_AFFECTOR_DATA_TIMELINE_SPIN_SPEED, 0.f );
    float spin_accelerate = __get_affector_value_rands( _p, _emitter, DZ_AFFECTOR_DATA_TIMELINE_SPIN_ACCELERATE, 0.f );

    _p->size = __get_affector_value_rands( _p, _emitter, DZ_AFFECTOR_DATA_TIMELINE_SIZE, 1.f );

    _p->color_r = __get_affector_value_rands( _p, _emitter, DZ_AFFECTOR_DATA_TIMELINE_COLOR_R, 1.f );
    _p->color_g = __get_affector_value_rands( _p, _emitter, DZ_AFFECTOR_DATA_TIMELINE_COLOR_G, 1.f );
    _p->color_b = __get_affector_value_rands( _p, _emitter, DZ_AFFECTOR_DATA_TIMELINE_COLOR_B, 1.f );
    _p->color_a = __get_affector_value_rands( _p, _emitter, DZ_AFFECTOR_DATA_TIMELINE_COLOR_A, 1.f );

    _p->rotate_accelerate_aux += rotate_accelerate * _time * _time;
    _p->angle += rotate_speed * _time + _p->rotate_accelerate_aux;

    _p->spin_accelerate_aux += spin_accelerate * _time * _time;
    _p->spin += spin_speed * _time + _p->spin_accelerate_aux;

    float dx = DZ_COSF( _service, _p->angle );
    float dy = DZ_SINF( _service, _p->angle );

    _p->move_accelerate_aux += move_accelerate * _time * _time;

    float strafe_size = __get_affector_value_rands( _p, _emitter, DZ_AFFECTOR_DATA_TIMELINE_STRAFE_SIZE, 0.f );

    if( strafe_size != 0.f )
    {
        float strafe_speed = __get_affector_value_rands( _p, _emitter, DZ_AFFECTOR_DATA_TIMELINE_STRAFE_SPEED, 0.f );
        float strafe_frenquence = __get_affector_value_rands( _p, _emitter, DZ_AFFECTOR_DATA_TIMELINE_STRAFE_FRENQUENCE, 0.f );

        float strafe_shift = _p->rands[DZ_AFFECTOR_DATA_TIMELINE_STRAFE_SHIFT];

        float strafex = -dy * DZ_COSF( _service, strafe_shift * DZ_PI + strafe_frenquence * _p->time ) * strafe_size * strafe_speed * _time;
        float strafey = dx * DZ_SINF( _service, strafe_shift * DZ_PI + strafe_frenquence * _p->time ) * strafe_size * strafe_speed * _time;

        _p->x += strafex;
        _p->y += strafey;
    }

    float movex = dx * (move_speed * _time + _p->move_accelerate_aux);
    float movey = dy * (move_speed * _time + _p->move_accelerate_aux);

    _p->x += movex;
    _p->y += movey;

    float sx = DZ_COSF( _service, _p->angle + _p->spin );
    float sy = DZ_SINF( _service, _p->angle + _p->spin );

    _p->sx = sx;
    _p->sy = sy;
}
//////////////////////////////////////////////////////////////////////////
static float __get_timeline_value_seed( uint32_t * _seed, const dz_timeline_key_t * _timeline, float _life, float _time )
{
    float t = __get_randf( _seed );

    float value = __get_timeline_value( t, _timeline, _life, _time );

    return value;
}
//////////////////////////////////////////////////////////////////////////
static float __get_shape_value_seed( dz_emitter_t * _emitter, dz_affector_data_timeline_type_e _type, float _life, float _time, float _default )
{
    const dz_timeline_key_t * timeline_key = _emitter->shape_data->timelines[_type];

    if( timeline_key == DZ_NULLPTR )
    {
        return _default;
    }

    float value = __get_timeline_value_seed( &_emitter->seed, timeline_key, _life, _time );

    return value;
}
//////////////////////////////////////////////////////////////////////////
static float __get_emitter_value_seed( dz_emitter_t * _emitter, dz_emitter_data_timeline_type_e _type, float _life, float _time, float _default )
{
    const dz_timeline_key_t * timeline_key = _emitter->emitter_data->timelines[_type];

    if( timeline_key == DZ_NULLPTR )
    {
        return _default;
    }

    float value = __get_timeline_value_seed( &_emitter->seed, timeline_key, _life, _time );

    return value;
}
//////////////////////////////////////////////////////////////////////////
static float __get_affector_value_seed( dz_emitter_t * _emitter, dz_affector_data_timeline_type_e _type, float _life, float _time, float _default )
{
    const dz_timeline_key_t * timeline_key = _emitter->affector_data->timelines[_type];

    if( timeline_key == DZ_NULLPTR )
    {
        return _default;
    }

    float value = __get_timeline_value_seed( &_emitter->seed, timeline_key, _life, _time );

    return value;
}
//////////////////////////////////////////////////////////////////////////
static float __calc_triangle_area( float ax, float ay, float bx, float by, float cx, float cy )
{
    float area =  (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by)) * 0.5f;

    if( area < 0.f )
    {
        return -area;
    }

    return area;
}
//////////////////////////////////////////////////////////////////////////
static uint32_t __calc_mask_threshold_value_count( const void * _buffer, uint32_t _pitch, uint32_t _bites, uint32_t _width, uint32_t _height, uint32_t _threshold )
{
    uint32_t threshold_value_count = 0;

    const void * buffer_iterator = _buffer;

    for( uint32_t h = 0; h != _height; ++h )
    {
        for( uint32_t w = 0; w != _width; ++w )
        {
            uint32_t value = 0;

            switch( _bites )
            {
            case 1:
                {
                    const uint8_t * point = (const uint8_t *)buffer_iterator + w;

                    value = (uint32_t)*point;
                }break;
            case 2:
                {
                    const uint16_t * point = (const uint16_t *)buffer_iterator + w;

                    value = (uint32_t)*point;
                }break;
            case 4:
                {
                    const uint32_t * point = (const uint32_t *)buffer_iterator + w;

                    value = (uint32_t)*point;
                }break;
            default:
                break;
            }

            if( value <= _threshold )
            {
                continue;
            }

            ++threshold_value_count;
        }

        buffer_iterator = (const uint8_t *)buffer_iterator + _pitch;
    }

    return threshold_value_count;
}
//////////////////////////////////////////////////////////////////////////
static void __get_mask_threshold_value( const void * _buffer, uint32_t _pitch, uint32_t _bites, uint32_t _width, uint32_t _height, uint32_t _threshold, uint32_t _index, uint32_t * _x, uint32_t * _y )
{
    const void * buffer_iterator = _buffer;

    for( uint32_t h = 0; h != _height; ++h )
    {
        for( uint32_t w = 0; w != _width; ++w )
        {
            uint32_t mask_value = 0;

            switch( _bites )
            {
            case 1:
                {
                    const uint8_t * mask_point = (const uint8_t *)buffer_iterator + w;

                    mask_value = (uint32_t)*mask_point;
                }break;
            case 2:
                {
                    const uint16_t * mask_point = (const uint16_t *)buffer_iterator + w;

                    mask_value = (uint32_t)*mask_point;
                }break;
            case 4:
                {
                    const uint32_t * mask_point = (const uint32_t *)buffer_iterator + w;

                    mask_value = (uint32_t)*mask_point;
                }break;
            default:
                break;
            }

            if( mask_value <= _threshold )
            {
                continue;
            }

            if( _index-- == 0 )
            {
                *_x = w;
                *_y = h;

                return;
            }
        }

        buffer_iterator = (const uint8_t *)buffer_iterator + _pitch;
    }
}
//////////////////////////////////////////////////////////////////////////
static void __emitter_spawn_particle( dz_service_t * _service, dz_emitter_t * _emitter, float _life, float _spawn_time )
{
    float time = _emitter->time - _spawn_time;

    if( _emitter->partices_count >= _emitter->partices_capacity )
    {
        if( _emitter->partices_capacity == 0 )
        {
            _emitter->partices_capacity = 16;
        }
        else
        {
            _emitter->partices_capacity *= 2;
        }

        _emitter->partices = DZ_REALLOCN( _service, _emitter->partices, dz_particle_t, _emitter->partices_capacity );
    }

    dz_particle_t * p = _emitter->partices + _emitter->partices_count++;

    for( uint32_t index = 0; index != __DZ_AFFECTOR_DATA_TIMELINE_MAX__; ++index )
    {
        p->rands[index] = __get_randf( &_emitter->seed );
    }

    p->life = _life;
    p->time = 0.f;

    p->move_accelerate_aux = 0.f;
    p->rotate_accelerate_aux = 0.f;
    p->spin_accelerate_aux = 0.f;

    float spin = __get_emitter_value_seed( _emitter, DZ_EMITTER_DATA_SPAWN_SPIN, _life, _spawn_time, 0.f );

    dz_shape_data_type_e shape_type = _emitter->shape_data->type;

    switch( shape_type )
    {
    case DZ_SHAPE_DATA_POINT:
        {
            p->x = 0.f;
            p->y = 0.f;

            p->angle = __get_randf( &_emitter->seed ) * DZ_PI2;
        }break;
    case DZ_SHAPE_DATA_SEGMENT:
        {
            p->x = 0.f;
            p->y = 0.f;

            float angle_min = __get_shape_value_seed( _emitter, DZ_SHAPE_DATA_SEGMENT_ANGLE_MIN, _life, _spawn_time, -DZ_PI * 0.25f );
            float angle_max = __get_shape_value_seed( _emitter, DZ_SHAPE_DATA_SEGMENT_ANGLE_MAX, _life, _spawn_time, DZ_PI * 0.25f );

            p->angle = __get_randf2( &_emitter->seed, angle_min, angle_max );
        }break;
    case DZ_SHAPE_DATA_CIRCLE:
        {
            float radius_min = __get_shape_value_seed( _emitter, DZ_SHAPE_DATA_CIRCLE_RADIUS_MIN, _life, _spawn_time, 1.f );
            float radius_max = __get_shape_value_seed( _emitter, DZ_SHAPE_DATA_CIRCLE_RADIUS_MAX, _life, _spawn_time, 1.f );

            float angle = __get_randf( &_emitter->seed ) * DZ_PI2;
            float radius = radius_min + DZ_SQRTF( _service, __get_randf( &_emitter->seed ) ) * (radius_max - radius_min);

            float rx = radius * DZ_COSF( _service, angle );
            float ry = radius * DZ_SINF( _service, angle );

            p->x = rx;
            p->y = ry;

            float angle_min = __get_shape_value_seed( _emitter, DZ_SHAPE_DATA_CIRCLE_ANGLE_MIN, _life, _spawn_time, -DZ_PI * 0.05f );
            float angle_max = __get_shape_value_seed( _emitter, DZ_SHAPE_DATA_CIRCLE_ANGLE_MAX, _life, _spawn_time, DZ_PI * 0.05f );

            p->angle = angle + __get_randf2( &_emitter->seed, angle_min, angle_max );
        }break;
    case DZ_SHAPE_DATA_LINE:
        {
            float angle = __get_shape_value_seed( _emitter, DZ_SHAPE_DATA_LINE_ANGLE, _life, _spawn_time, 0.f );

            float dx = DZ_COSF( _service, angle );
            float dy = DZ_SINF( _service, angle );

            float size = __get_shape_value_seed( _emitter, DZ_SHAPE_DATA_LINE_SIZE, _life, _spawn_time, 0.f );

            float l = (__get_randf( &_emitter->seed ) - 0.5f) * size;

            p->x = -dy * l;
            p->y = dx * l;

            p->angle = angle;
        }break;
    case DZ_SHAPE_DATA_RECT:
        {
            //float width_min = __get_shape_value_seed( _emitter, DZ_SHAPE_DATA_RECT_WIDTH_MIN, _spawn_time, 0.f );
            float width_max = __get_shape_value_seed( _emitter, DZ_SHAPE_DATA_RECT_WIDTH_MAX, _life, _spawn_time, 1.f );
            //float height_min = __get_shape_value_seed( _emitter, DZ_SHAPE_DATA_RECT_HEIGHT_MIN, _spawn_time, 0.f );
            float height_max = __get_shape_value_seed( _emitter, DZ_SHAPE_DATA_RECT_HEIGHT_MAX, _life, _spawn_time, 1.f );

            float x = (__get_randf( &_emitter->seed ) - 0.5f) * width_max;
            float y = (__get_randf( &_emitter->seed ) - 0.5f) * height_max;

            p->x = x;
            p->y = y;

            float angle = __get_randf( &_emitter->seed ) * DZ_PI2;

            p->angle = angle;
        }break;
    case DZ_SHAPE_DATA_POLYGON:
        {
            float total_area = 0.f;
            float areas[1024];

            const float * triangles = _emitter->shape_data->triangles;
            uint32_t triangle_count = _emitter->shape_data->triangle_count;

            for( uint32_t index = 0; index != triangle_count; ++index )
            {
                float ax = triangles[index * 6 + 0];
                float ay = triangles[index * 6 + 1];
                float bx = triangles[index * 6 + 2];
                float by = triangles[index * 6 + 3];
                float cx = triangles[index * 6 + 4];
                float cy = triangles[index * 6 + 5];

                float area = __calc_triangle_area( ax, ay, bx, by, cx, cy );

                total_area += area;

                areas[index] = total_area;
            }

            float triangle_rand = __get_randf( &_emitter->seed );

            float triangle_find_area = triangle_rand * total_area;

            uint32_t triangle_found_index = 0;

            for( uint32_t index = 0; index != triangle_count; ++index )
            {
                float area = areas[index];

                if( area < triangle_find_area )
                {
                    continue;
                }

                triangle_found_index = index;

                break;
            }

            float rax = triangles[triangle_found_index * 6 + 0];
            float ray = triangles[triangle_found_index * 6 + 1];
            float rbx = triangles[triangle_found_index * 6 + 2];
            float rby = triangles[triangle_found_index * 6 + 3];
            float rcx = triangles[triangle_found_index * 6 + 4];
            float rcy = triangles[triangle_found_index * 6 + 5];

            float r1 = __get_randf( &_emitter->seed );
            float r2 = __get_randf( &_emitter->seed );

            float qr1 = DZ_SQRTF( _service, r1 );

            float tx = (1.f - qr1) * rax + (qr1 * (1.f - r2)) * rbx + (qr1 * r2) * rcx;
            float ty = (1.f - qr1) * ray + (qr1 * (1.f - r2)) * rby + (qr1 * r2) * rcy;

            p->x = tx;
            p->y = ty;

            float angle = __get_randf( &_emitter->seed ) * DZ_PI2;

            p->angle = angle;
        }break;
    case DZ_SHAPE_DATA_MASK:
        {
            const void * mask_buffer = _emitter->shape_data->mask_buffer;
            uint32_t mask_bites = _emitter->shape_data->mask_bites;
            uint32_t mask_pitch = _emitter->shape_data->mask_pitch;
            uint32_t mask_width = _emitter->shape_data->mask_width;
            uint32_t mask_height = _emitter->shape_data->mask_height;
            uint32_t mask_threshold = _emitter->shape_data->mask_threshold;
            float mask_scale = _emitter->shape_data->mask_scale;

            uint32_t threshold_value_count = __calc_mask_threshold_value_count( mask_buffer, mask_pitch, mask_bites, mask_width, mask_height, mask_threshold );

            float r = __get_randf( &_emitter->seed );

            uint32_t threshold_value_index = (uint32_t)(r * (threshold_value_count - 1) + 0.5f);

            uint32_t w_found = 0;
            uint32_t h_found = 0;
            __get_mask_threshold_value( mask_buffer, mask_pitch, mask_bites, mask_width, mask_height, mask_threshold, threshold_value_index, &w_found, &h_found );

            p->x = w_found * mask_scale;
            p->y = h_found * mask_scale;

            float angle = __get_randf( &_emitter->seed ) * DZ_PI2;

            p->angle = angle;
        }break;
    case __DZ_SHAPE_DATA_MAX__:
    default:
        break;
    }

    p->spin = (__get_randf( &_emitter->seed ) * 2.f - 1.f) * spin;

    float sx = DZ_COSF( _service, p->spin );
    float sy = DZ_SINF( _service, p->spin );

    p->sx = sx;
    p->sy = sy;

    __particle_update( _service, _emitter, p, time );
}
//////////////////////////////////////////////////////////////////////////
void dz_emitter_update( dz_service_t * _service, dz_emitter_t * _emitter, float _time )
{
    _emitter->time += _time;

    dz_particle_t * p = _emitter->partices;
    dz_particle_t * p_end = _emitter->partices + _emitter->partices_count;
    while( p != p_end )
    {
        if( p->time + _time < p->life )
        {
            __particle_update( _service, _emitter, p, _time );
            ++p;
        }
        else
        {
            *p = *(--p_end);
            --_emitter->partices_count;
        }        
    }

    for( ;;)
    {
        if( _emitter->life > 0.f && _emitter->emitter_time > _emitter->life )
        {
            break;
        }
        else if( _emitter->life < 0.f )
        {
            for( ; _emitter->emitter_time > _emitter->life; _emitter->emitter_time -= _emitter->life );
        }

        float delay = __get_emitter_value_seed( _emitter, DZ_EMITTER_DATA_SPAWN_DELAY, _emitter->life, _emitter->emitter_time, 1.f );

        if( _emitter->time - _emitter->emitter_time < delay )
        {
            break;
        }

        float spawn_time = _emitter->emitter_time + delay;

        float count = __get_emitter_value_seed( _emitter, DZ_EMITTER_DATA_SPAWN_COUNT, _emitter->life, spawn_time, 1.f );

        while( count > 0.f )
        {
            float life = __get_affector_value_seed( _emitter, DZ_AFFECTOR_DATA_TIMELINE_LIFE, _emitter->life, spawn_time, 1.f );

            float ptime = _emitter->time - spawn_time;

            if( life > ptime && _emitter->partices_count < _emitter->particle_limit )
            {
                __emitter_spawn_particle( _service, _emitter, life, spawn_time );
            }

            count -= 1.f;
        }

        _emitter->emitter_time += delay;
    }
}
//////////////////////////////////////////////////////////////////////////
dz_emitter_state_e dz_emitter_get_state( const dz_emitter_t * _emitter )
{
    dz_emitter_state_e state = DZ_EMITTER_PROCESS;

    if( _emitter->life > 0.f && _emitter->emitter_time > _emitter->life )
    {
        state |= DZ_EMITTER_EMIT_COMPLETE;
    }

    if( state & DZ_EMITTER_EMIT_COMPLETE && _emitter->partices_count == 0U )
    {
        state |= DZ_EMITTER_PARTICLE_COMPLETE;
    }

    return state;
}
//////////////////////////////////////////////////////////////////////////
static void __particle_compute_positions( const dz_particle_t * _p, uint16_t _iterator, dz_emitter_mesh_t * _mesh )
{
    const float hs = _p->size * 0.5f;

    const float hsx = _p->sx * hs;
    const float hsy = _p->sy * hs;

    const float ux = hsx;
    const float uy = hsy;

    const float vx = -hsy;
    const float vy = hsx;

    float * p0 = (float *)((uint8_t *)(_mesh->position_buffer) + _mesh->position_offset + _mesh->position_stride * (_iterator * 4 + 0));

    p0[0] = _p->x - ux + vx;
    p0[1] = _p->y - uy + vy;

    float * p1 = (float *)((uint8_t *)(_mesh->position_buffer) + _mesh->position_offset + _mesh->position_stride * (_iterator * 4 + 1));

    p1[0] = _p->x + ux + vx;
    p1[1] = _p->y + uy + vy;

    float * p2 = (float *)((uint8_t *)(_mesh->position_buffer) + _mesh->position_offset + _mesh->position_stride * (_iterator * 4 + 2));

    p2[0] = _p->x + ux - vx;
    p2[1] = _p->y + uy - vy;

    float * p3 = (float *)((uint8_t *)(_mesh->position_buffer) + _mesh->position_offset + _mesh->position_stride * (_iterator * 4 + 3));

    p3[0] = _p->x - ux - vx;
    p3[1] = _p->y - uy - vy;
}
//////////////////////////////////////////////////////////////////////////
static void __particle_compute_colors( const dz_particle_t * _p, uint16_t _iterator, dz_emitter_mesh_t * _mesh )
{
    const uint8_t r8 = (uint8_t)(_mesh->r * _p->color_r * 255.5f);
    const uint8_t g8 = (uint8_t)(_mesh->g * _p->color_g * 255.5f);
    const uint8_t b8 = (uint8_t)(_mesh->b * _p->color_b * 255.5f);
    const uint8_t a8 = (uint8_t)(_mesh->a * _p->color_a * 255.5f);

    const uint32_t color = (a8 << 24) | (r8 << 16) | (g8 << 8) | (b8 << 0);

    uint32_t * c0 = (uint32_t *)((uint8_t *)(_mesh->color_buffer) + _mesh->color_offset + _mesh->color_stride * (_iterator * 4 + 0));

    c0[0] = color;

    uint32_t * c1 = (uint32_t *)((uint8_t *)(_mesh->color_buffer) + _mesh->color_offset + _mesh->color_stride * (_iterator * 4 + 1));

    c1[0] = color;

    uint32_t * c2 = (uint32_t *)((uint8_t *)(_mesh->color_buffer) + _mesh->color_offset + _mesh->color_stride * (_iterator * 4 + 2));

    c2[0] = color;

    uint32_t * c3 = (uint32_t *)((uint8_t *)(_mesh->color_buffer) + _mesh->color_offset + _mesh->color_stride * (_iterator * 4 + 3));

    c3[0] = color;
}
//////////////////////////////////////////////////////////////////////////
static void __particle_compute_uvs( const dz_particle_t * _p, uint16_t _iterator, dz_emitter_mesh_t * _mesh )
{
    DZ_UNUSED( _p );

    float * uv0 = (float *)((uint8_t *)(_mesh->uv_buffer) + _mesh->uv_offset + _mesh->uv_stride * (_iterator * 4 + 0));

    uv0[0] = 0.f;
    uv0[1] = 0.f;

    float * uv1 = (float *)((uint8_t *)(_mesh->uv_buffer) + _mesh->uv_offset + _mesh->uv_stride * (_iterator * 4 + 1));

    uv1[0] = 1.f;
    uv1[1] = 0.f;

    float * uv2 = (float *)((uint8_t *)(_mesh->uv_buffer) + _mesh->uv_offset + _mesh->uv_stride * (_iterator * 4 + 2));

    uv2[0] = 1.f;
    uv2[1] = 1.f;

    float * uv3 = (float *)((uint8_t *)(_mesh->uv_buffer) + _mesh->uv_offset + _mesh->uv_stride * (_iterator * 4 + 3));

    uv3[0] = 0.f;
    uv3[1] = 1.f;
}
//////////////////////////////////////////////////////////////////////////
static void __particle_compute_index( uint16_t _iterator, dz_emitter_mesh_t * _mesh )
{
    uint16_t * index_buffer = (uint16_t *)(_mesh->index_buffer) + _iterator * 6;
    index_buffer[0] = _iterator * 4 + 0;
    index_buffer[1] = _iterator * 4 + 1;
    index_buffer[2] = _iterator * 4 + 3;
    index_buffer[3] = _iterator * 4 + 3;
    index_buffer[4] = _iterator * 4 + 1;
    index_buffer[5] = _iterator * 4 + 2;
}
//////////////////////////////////////////////////////////////////////////
void dz_emitter_compute_mesh( const dz_emitter_t * _emitter, dz_emitter_mesh_t * _mesh, dz_emitter_mesh_chunk_t * _chunks, uint32_t _capacity, uint32_t * _count )
{
    DZ_UNUSED( _capacity );

    uint16_t particle_iterator = 0;

    const dz_particle_t * p = _emitter->partices;
    const dz_particle_t * p_end = _emitter->partices + _emitter->partices_count;
    for( ; p != p_end; ++p )
    {
        __particle_compute_positions( p, particle_iterator, _mesh );
        __particle_compute_colors( p, particle_iterator, _mesh );
        __particle_compute_uvs( p, particle_iterator, _mesh );
        __particle_compute_index( particle_iterator, _mesh );

        ++particle_iterator;
    }

    if( particle_iterator == 0 )
    {
        *_count = 0;

        return;
    }

    dz_emitter_mesh_chunk_t * chunk = _chunks + 0;
    chunk->offset = 0;
    chunk->vertex_size = particle_iterator * 4;
    chunk->index_size = particle_iterator * 6;

    chunk->blend_type = DZ_BLEND_NORNAL;
    chunk->texture = DZ_NULLPTR;

    *_count = 1;
}
//////////////////////////////////////////////////////////////////////////