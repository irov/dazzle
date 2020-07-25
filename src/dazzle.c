#include "dazzle/dazzle.h"

#include "alloc.h"

//////////////////////////////////////////////////////////////////////////
typedef struct dz_service_t
{
    dz_service_providers_t providers;
    dz_userdata_t ud;
} dz_service_t;
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
typedef struct dz_timeline_interpolate_t
{
    dz_timeline_interpolate_type_e type;

    const struct dz_timeline_key_t * key;

    dz_userdata_t ud;
} dz_timeline_interpolate_t;
//////////////////////////////////////////////////////////////////////////
typedef struct dz_timeline_interpolate_linear_t
{
    dz_timeline_interpolate_t base;

} dz_timeline_interpolate_linear_t;
//////////////////////////////////////////////////////////////////////////
typedef struct dz_timeline_interpolate_bezier2_t
{
    dz_timeline_interpolate_t base;

    float p0;
    float p1;
} dz_timeline_interpolate_bezier2_t;
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
    default:
        {
            return DZ_FAILURE;
        }break;
    }

    interpolate->type = _type;
    interpolate->ud = _ud;

    *_interpolate = interpolate;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_timeline_interpolate_destroy( dz_service_t * _service, dz_timeline_interpolate_t * _interpolate )
{
    if( _interpolate->key != DZ_NULLPTR )
    {
        dz_timeline_key_destroy( _service, _interpolate->key );
    }

    DZ_FREE( _service, _interpolate );
}
//////////////////////////////////////////////////////////////////////////
typedef struct dz_timeline_key_t
{
    float time;
    float inv_time;

    dz_timeline_key_type_e type;

    dz_timeline_interpolate_t * interpolate;

    dz_userdata_t ud;
} dz_timeline_key_t;
//////////////////////////////////////////////////////////////////////////
typedef struct dz_timeline_key_const_t
{
    dz_timeline_key_t base;

    float value;
} dz_timeline_key_const_t;
//////////////////////////////////////////////////////////////////////////
typedef struct dz_timeline_key_randomize_t
{
    dz_timeline_key_t base;

    float value_min;
    float value_max;
} dz_timeline_key_randomize_t;
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_timeline_key_create( dz_service_t * _service, dz_timeline_key_t ** _key, float _time, dz_timeline_key_type_e _type, dz_userdata_t _ud )
{
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
    default:
        {
            return DZ_FAILURE;
        }break;
    }

    key->time = _time;
    key->inv_time = 1.f / _time;
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
typedef struct dz_affector_data_t
{
    const dz_timeline_key_t * timelines[__DZ_AFFECTOR_DATA_TIMELINE_MAX__];
} dz_affector_data_t;
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_affector_data_create( dz_service_t * _service, dz_affector_data_t ** _affector_data )
{
    dz_affector_data_t * affector_data = DZ_NEW( _service, dz_affector_data_t );

    for( uint32_t index = 0; index != __DZ_AFFECTOR_DATA_TIMELINE_MAX__; ++index )
    {
        affector_data->timelines[index] = DZ_NULLPTR;
    }

    *_affector_data = affector_data;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_affector_data_destroy( dz_service_t * _service, dz_affector_data_t * _affector_data )
{
    for( uint32_t index = 0; index != __DZ_AFFECTOR_DATA_TIMELINE_MAX__; ++index )
    {
        const dz_timeline_key_t * timeline = _affector_data->timelines[index];
        dz_timeline_key_destroy( _service, timeline );
    }

    DZ_FREE( _service, _affector_data );
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
typedef struct dz_emitter_data_t
{
    dz_emitter_shape_type_e shape_type;

    float life;

    const dz_timeline_key_t * spawn_delay;
    const dz_timeline_key_t * spawn_count;

    dz_userdata_t ud;
} dz_emitter_data_t;
//////////////////////////////////////////////////////////////////////////
typedef struct dz_emitter_data_point_t
{
    dz_emitter_data_t base;

} dz_emitter_data_point_t;
//////////////////////////////////////////////////////////////////////////
typedef struct dz_emitter_data_circle_t
{
    dz_emitter_data_t base;

    const dz_timeline_key_t * radius;
} dz_emitter_data_circle_t;
//////////////////////////////////////////////////////////////////////////
typedef struct dz_emitter_data_line_t
{
    dz_emitter_data_t base;

    const dz_timeline_key_t * bx;
    const dz_timeline_key_t * by;

    const dz_timeline_key_t * ex;
    const dz_timeline_key_t * ey;
} dz_emitter_data_line_t;
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_emitter_data_create( dz_service_t * _service, dz_emitter_data_t ** _emitter_data, dz_emitter_shape_type_e _type, dz_userdata_t _ud )
{
    dz_emitter_data_t * emitter_data;

    switch( _type )
    {
    case DZ_EMITTER_SHAPE_POINT:
        {
            emitter_data = DZ_NEWV( _service, dz_emitter_data_t, dz_emitter_data_point_t );
        }break;
    case DZ_EMITTER_SHAPE_CIRCLE:
        {
            emitter_data = DZ_NEWV( _service, dz_emitter_data_t, dz_emitter_data_circle_t );
        }break;
    case DZ_EMITTER_SHAPE_LINE:
        {
            emitter_data = DZ_NEWV( _service, dz_emitter_data_t, dz_emitter_data_line_t );
        }break;
    default:
        return DZ_FAILURE;
    }

    emitter_data->shape_type = _type;
    emitter_data->spawn_delay = DZ_NULLPTR;
    emitter_data->spawn_count = DZ_NULLPTR;
    emitter_data->life = 0.f;

    emitter_data->ud = _ud;

    *_emitter_data = emitter_data;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_emitter_data_destroy( dz_service_t * _service, dz_emitter_data_t * _emitter_data )
{
    dz_timeline_key_destroy( _service, _emitter_data->spawn_delay );
    dz_timeline_key_destroy( _service, _emitter_data->spawn_count );

    DZ_FREE( _service, _emitter_data );
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
void dz_emitter_data_set_timeline_key_spawn_delay( dz_emitter_data_t * _emitter_data, const dz_timeline_key_t * _timeline )
{
    _emitter_data->spawn_delay = _timeline;
}
//////////////////////////////////////////////////////////////////////////
const dz_timeline_key_t * dz_emitter_data_get_timeline_key_spawn_delay( const dz_emitter_data_t * _emitter_data )
{
    return _emitter_data->spawn_delay;
}
//////////////////////////////////////////////////////////////////////////
void dz_emitter_data_set_timeline_key_spawn_count( dz_emitter_data_t * _emitter_data, const dz_timeline_key_t * _timeline )
{
    _emitter_data->spawn_count = _timeline;
}
//////////////////////////////////////////////////////////////////////////
const dz_timeline_key_t * dz_emitter_data_get_timeline_key_spawn_count( const dz_emitter_data_t * _emitter_data )
{
    return _emitter_data->spawn_count;
}
//////////////////////////////////////////////////////////////////////////
typedef struct dz_timeline_value_t
{
    const dz_timeline_key_t * begin;
    const dz_timeline_key_t * current;

    float time;
} dz_timeline_value_t;
//////////////////////////////////////////////////////////////////////////
static dz_timeline_value_t * __new_timeline_value( dz_service_t * _service, const dz_timeline_key_t * _key )
{
    dz_timeline_value_t * value = DZ_NEW( _service, dz_timeline_value_t );

    value->begin = _key;
    value->current = _key;
    value->time = 0.f;

    return value;
}
//////////////////////////////////////////////////////////////////////////
static uint16_t __get_rand0( uint32_t _seed )
{
    uint32_t value = (_seed * 1103515245U) + 12345U;

    return value & 0xffff;
}
//////////////////////////////////////////////////////////////////////////
static float __get_randf0( uint32_t _seed )
{
    uint16_t value = __get_rand0( _seed );

    const float inv_65535 = 1.f / 65535.f;

    float valuef = (float)value * inv_65535;

    return valuef;
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
    default:
        break;
    };

    return 0.f;
}
//////////////////////////////////////////////////////////////////////////
static float __get_timeline_value( float _t, dz_timeline_value_t * _value, float _time )
{
    for( const dz_timeline_key_t * current = _value->current; current != DZ_NULLPTR && current->interpolate != DZ_NULLPTR; current = current->interpolate->key )
    {
        float dt = _time - _value->time;

        if( current->time < dt )
        {
            float current_value = __get_timeline_key_value( _t, current );

            const dz_timeline_key_t * next = current->interpolate->key;

            float next_value = __get_timeline_key_value( _t, next );

            float t = (_time - _value->time) * current->inv_time;

            float value = current_value + (next_value - current_value) * t;

            return value;
        }

        _value->time += current->time;

        _value->current = _value->current->interpolate->key;
    }

    const dz_timeline_key_t * last = _value->current;

    float last_value = __get_timeline_key_value( _t, last );

    return last_value;
}
//////////////////////////////////////////////////////////////////////////
static float __get_timeline_value_seed( uint32_t * _seed, dz_timeline_value_t * _value, float _time )
{
    float t = __get_rand( _seed );

    float value = __get_timeline_value( t, _value, _time );

    return value;
}
//////////////////////////////////////////////////////////////////////////
typedef enum dz_particle_seed_e
{
    DZ_PARTICLE_SEED_MOVE_SPEED = 0,
    DZ_PARTICLE_SEED_MOVE_ACCELERATE,

    DZ_PARTICLE_SEED_ROTATE_SPEED,
    DZ_PARTICLE_SEED_ROTATE_ACCELERATE,

    DZ_PARTICLE_SEED_SPIN_SPEED,
    DZ_PARTICLE_SEED_SPIN_ACCELERATE,

    DZ_PARTICLE_SEED_SIZE,

    DZ_PARTICLE_SEED_TRANSPARENT,
    DZ_PARTICLE_SEED_COLOR_R,
    DZ_PARTICLE_SEED_COLOR_G,
    DZ_PARTICLE_SEED_COLOR_B,

    __DZ_PARTICLE_SEED_MAX__
} dz_particle_seed_e;
//////////////////////////////////////////////////////////////////////////
typedef struct dz_particle_t
{
    float rands[__DZ_PARTICLE_SEED_MAX__];

    float life;
    float time;

    float x;
    float y;

    float angle;
    float spin;

    float sx;
    float sy;

    float move_speed;
    float move_accelerate;
    float move_accelerate_aux;

    float rotate_speed;
    float rotate_accelerate;
    float rotate_accelerate_aux;

    float spin_speed;
    float spin_accelerate;
    float spin_accelerate_aux;

    float size;

    float transparent;
    float color_r;
    float color_g;
    float color_b;
} dz_particle_t;
//////////////////////////////////////////////////////////////////////////
typedef struct dz_emitter_t
{
    const dz_emitter_data_t * emitter_data;
    const dz_affector_data_t * affector_data;

    uint32_t seed;

    dz_particle_t * partices;
    dz_size_t partices_count;
    dz_size_t partices_capacity;

    float time;
    float emitter_time;

    dz_timeline_value_t * spawn_delay;
    dz_timeline_value_t * spawn_count;

    dz_timeline_value_t * affector_values[__DZ_AFFECTOR_DATA_TIMELINE_MAX__];
} dz_emitter_t;
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_emitter_create( dz_service_t * _service, const dz_emitter_data_t * _emitter_data, const dz_affector_data_t * _affector_data, uint32_t _seed, dz_emitter_t ** _emitter )
{
    dz_emitter_t * emitter = DZ_NEW( _service, dz_emitter_t );

    emitter->emitter_data = _emitter_data;
    emitter->affector_data = _affector_data;

    emitter->seed = _seed;

    emitter->partices = DZ_NULLPTR;
    emitter->partices_count = 0;
    emitter->partices_capacity = 0;

    emitter->time = 0.f;
    emitter->emitter_time = 0.f;

    emitter->spawn_delay = __new_timeline_value( _service, emitter->emitter_data->spawn_delay );
    emitter->spawn_count = __new_timeline_value( _service, emitter->emitter_data->spawn_count );

    for( uint32_t index = 0; index != __DZ_AFFECTOR_DATA_TIMELINE_MAX__; ++index )
    {
        const dz_timeline_key_t * timeline = emitter->affector_data->timelines[index];

        emitter->affector_values[index] = __new_timeline_value( _service, timeline );
    }

    *_emitter = emitter;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_emitter_destroy( dz_service_t * _service, dz_emitter_t * _emitter )
{
    DZ_FREE( _service, _emitter );
}
//////////////////////////////////////////////////////////////////////////
static void __particle_update( dz_service_t * _service, dz_emitter_t * _emitter, dz_particle_t * _p, float _time )
{
    _p->move_speed = __get_timeline_value( _p->rands[DZ_PARTICLE_SEED_MOVE_SPEED], _emitter->affector_values[DZ_AFFECTOR_DATA_TIMELINE_MOVE_SPEED], _time );
    _p->move_accelerate = __get_timeline_value( _p->rands[DZ_PARTICLE_SEED_MOVE_ACCELERATE], _emitter->affector_values[DZ_AFFECTOR_DATA_TIMELINE_MOVE_ACCELERATE], _time );

    _p->rotate_speed = __get_timeline_value( _p->rands[DZ_PARTICLE_SEED_ROTATE_SPEED], _emitter->affector_values[DZ_AFFECTOR_DATA_TIMELINE_ROTATE_SPEED], _time );
    _p->rotate_accelerate = __get_timeline_value( _p->rands[DZ_PARTICLE_SEED_MOVE_ACCELERATE], _emitter->affector_values[DZ_AFFECTOR_DATA_TIMELINE_ROTATE_ACCELERATE], _time );

    _p->spin_speed = __get_timeline_value( _p->rands[DZ_PARTICLE_SEED_SPIN_SPEED], _emitter->affector_values[DZ_AFFECTOR_DATA_TIMELINE_SPIN_SPEED], _time );
    _p->spin_accelerate = __get_timeline_value( _p->rands[DZ_PARTICLE_SEED_SPIN_ACCELERATE], _emitter->affector_values[DZ_AFFECTOR_DATA_TIMELINE_SPIN_ACCELERATE], _time );

    _p->size = __get_timeline_value( _p->rands[DZ_PARTICLE_SEED_SIZE], _emitter->affector_values[DZ_AFFECTOR_DATA_TIMELINE_SIZE], _time );

    _p->transparent = __get_timeline_value( _p->rands[DZ_PARTICLE_SEED_TRANSPARENT], _emitter->affector_values[DZ_AFFECTOR_DATA_TIMELINE_TRANSPARENT], _time );
    _p->color_r = __get_timeline_value( _p->rands[DZ_PARTICLE_SEED_COLOR_R], _emitter->affector_values[DZ_AFFECTOR_DATA_TIMELINE_COLOR_R], _time );
    _p->color_g = __get_timeline_value( _p->rands[DZ_PARTICLE_SEED_COLOR_G], _emitter->affector_values[DZ_AFFECTOR_DATA_TIMELINE_COLOR_G], _time );
    _p->color_b = __get_timeline_value( _p->rands[DZ_PARTICLE_SEED_COLOR_B], _emitter->affector_values[DZ_AFFECTOR_DATA_TIMELINE_COLOR_B], _time );

    _p->rotate_accelerate_aux += _p->rotate_accelerate * _time;

    _p->angle += _p->rotate_speed + _p->rotate_accelerate_aux;

    _p->spin_accelerate_aux += _p->spin_accelerate * _time;

    float dx = _service->providers.f_cosf( _p->angle, _service->ud );
    float dy = _service->providers.f_sinf( _p->angle, _service->ud );

    _p->move_accelerate_aux += _p->move_accelerate * _time;

    _p->x += dx * (_p->move_speed + _p->move_accelerate_aux);
    _p->y += dy * (_p->move_speed + _p->move_accelerate_aux);

    float sx = _service->providers.f_cosf( _p->angle + _p->spin, _service->ud );
    float sy = _service->providers.f_sinf( _p->angle + _p->spin, _service->ud );

    _p->sx = sx;
    _p->sy = sy;
}
//////////////////////////////////////////////////////////////////////////
static void __emitter_spawn( dz_service_t * _service, dz_emitter_t * _emitter, float _life, float _time )
{
    if( _emitter->partices_count >= _emitter->partices_capacity )
    {
        if( _emitter->partices_capacity == 0 )
        {
            _emitter->partices_capacity = 16;
        }
        else
        {
            _emitter->partices_capacity >>= 1;
        }

        _emitter->partices = DZ_REALLOCN( _service, _emitter->partices, dz_particle_t, _emitter->partices_capacity );
    }

    dz_particle_t * p = _emitter->partices + _emitter->partices_count++;

    for( uint32_t index = 0; index != __DZ_PARTICLE_SEED_MAX__; ++index )
    {
        p->rands[index] = __get_randf( &_emitter->seed );
    }

    p->life = _life;
    p->time = _time;

    p->x = 0.f;
    p->y = 0.f;

    p->angle = 0.f;
    p->spin = 0.f;

    p->sx = 1.f;
    p->sy = 0.f;

    p->move_accelerate_aux = 0.f;
    p->rotate_accelerate_aux = 0.f;

    __particle_update( _service, _emitter, p, _time );
}
//////////////////////////////////////////////////////////////////////////
void dz_emitter_update( dz_service_t * _service, dz_emitter_t * _emitter, float _time )
{
    _emitter->time += _time;

    dz_particle_t * p = _emitter->partices;
    dz_particle_t * p_end = _emitter->partices + _emitter->partices_count;
    for( ; p != p_end; ++p )
    {
        __particle_update( _service, _emitter, p, _time );
    }

    for( ;;)
    {
        float delay = __get_timeline_value_seed( &_emitter->seed, _emitter->spawn_delay, _emitter->emitter_time );

        if( _emitter->time - _emitter->emitter_time < delay )
        {
            break;
        }

        float spawn_time = _emitter->emitter_time + delay;

        float count = __get_timeline_value_seed( &_emitter->seed, _emitter->spawn_count, spawn_time );

        while( count > 0.f )
        {
            float life = __get_timeline_value_seed( &_emitter->seed, _emitter->affector_values[DZ_AFFECTOR_DATA_TIMELINE_LIFE], spawn_time );
            float chance_extra_life = __get_timeline_value_seed( &_emitter->seed, _emitter->affector_values[DZ_AFFECTOR_DATA_TIMELINE_CHANCE_EXTRA_LIFE], spawn_time );

            if( __get_randf( &_emitter->seed ) <= chance_extra_life )
            {
                float extra_life = __get_timeline_value_seed( &_emitter->seed, _emitter->affector_values[DZ_AFFECTOR_DATA_TIMELINE_EXTRA_LIFE], spawn_time );

                life += extra_life;
            }

            float ptime = _emitter->time - spawn_time;

            if( life > ptime )
            {
                __emitter_spawn( _service, _emitter, life, _emitter->time - spawn_time );
            }

            count -= 1.f;
        }

        _emitter->emitter_time += delay;
    }
}
//////////////////////////////////////////////////////////////////////////
static void __particle_compute_positions( const dz_particle_t * _p, uint16_t _iterator, dz_emitter_mesh_t * _mesh )
{
    const float hs = _p->size * 0.5f;

    const float ux = _p->sx * hs;
    const float uy = _p->sy * hs;

    const float vx = -_p->sy * hs;
    const float vy = _p->sx * hs;

    float * p0 = (float *)((uint8_t *)(_mesh->position_buffer) + _mesh->position_stride * (_iterator * 4 + 0));

    p0[0] = _p->x - ux + vx;
    p0[1] = _p->y - uy + vy;

    float * p1 = (float *)((uint8_t *)(_mesh->position_buffer) + _mesh->position_stride * (_iterator * 4 + 1));

    p1[0] = _p->x + ux + vx;
    p1[1] = _p->y + uy + vy;

    float * p2 = (float *)((uint8_t *)(_mesh->position_buffer) + _mesh->position_stride * (_iterator * 4 + 2));

    p2[0] = _p->x + ux - vx;
    p2[1] = _p->y + uy - vy;

    float * p3 = (float *)((uint8_t *)(_mesh->position_buffer) + _mesh->position_stride * (_iterator * 4 + 3));

    p3[0] = _p->x - ux - vx;
    p3[1] = _p->y - uy - vy;
}
//////////////////////////////////////////////////////////////////////////
static void __particle_compute_colors( const dz_particle_t * _p, uint16_t _iterator, dz_emitter_mesh_t * _mesh )
{
    const uint8_t r8 = (uint8_t)(_mesh->r * _p->color_r * 255.5f);
    const uint8_t g8 = (uint8_t)(_mesh->g * _p->color_g * 255.5f);
    const uint8_t b8 = (uint8_t)(_mesh->b * _p->color_b * 255.5f);
    const uint8_t a8 = (uint8_t)(_mesh->transparent * _p->transparent * 255.5f);

    const uint32_t color = (a8 << 24) | (r8 << 16) | (g8 << 8) | (b8 << 0);

    uint32_t * c0 = (uint32_t *)((uint8_t *)(_mesh->color_buffer) + _mesh->color_stride * (_iterator * 4 + 0));

    c0[0] = color;

    uint32_t * c1 = (uint32_t *)((uint8_t *)(_mesh->color_buffer) + _mesh->color_stride * (_iterator * 4 + 1));

    c1[0] = color;

    uint32_t * c2 = (uint32_t *)((uint8_t *)(_mesh->color_buffer) + _mesh->color_stride * (_iterator * 4 + 2));

    c2[0] = color;

    uint32_t * c3 = (uint32_t *)((uint8_t *)(_mesh->color_buffer) + _mesh->color_stride * (_iterator * 4 + 3));

    c3[0] = color;
}
//////////////////////////////////////////////////////////////////////////
static void __particle_compute_uvs( const dz_particle_t * _p, uint16_t _iterator, dz_emitter_mesh_t * _mesh )
{
    DZ_UNUSED( _p );

    float * uv0 = (float *)((uint8_t *)(_mesh->uv_buffer) + _mesh->uv_stride * (_iterator * 4 + 0));

    uv0[0] = 0.f;
    uv0[1] = 0.f;

    float * uv1 = (float *)((uint8_t *)(_mesh->uv_buffer) + _mesh->uv_stride * (_iterator * 4 + 1));

    uv1[0] = 1.f;
    uv1[1] = 0.f;

    float * uv2 = (float *)((uint8_t *)(_mesh->uv_buffer) + _mesh->uv_stride * (_iterator * 4 + 2));

    uv2[0] = 1.f;
    uv2[1] = 1.f;

    float * uv3 = (float *)((uint8_t *)(_mesh->uv_buffer) + _mesh->uv_stride * (_iterator * 4 + 3));

    uv3[0] = 0.f;
    uv3[1] = 1.f;
}
//////////////////////////////////////////////////////////////////////////
static void __particle_compute_index( uint16_t _iterator, dz_emitter_mesh_t * _mesh )
{
    uint16_t * index_buffer = (uint16_t *)(_mesh->index_buffer) + _iterator * 6;
    index_buffer[0] = _iterator + 0;
    index_buffer[1] = _iterator + 1;
    index_buffer[2] = _iterator + 3;
    index_buffer[3] = _iterator + 3;
    index_buffer[4] = _iterator + 1;
    index_buffer[5] = _iterator + 2;
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

    dz_emitter_mesh_chunk_t * chunk = _chunks + 0;
    chunk->offset = 0;
    chunk->size = particle_iterator * 6;

    *_count = 1;
}
//////////////////////////////////////////////////////////////////////////