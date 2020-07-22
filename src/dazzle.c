#include "dazzle/dazzle.h"

#include "alloc.h"
#include "list.h"

//////////////////////////////////////////////////////////////////////////
typedef struct dz_service_t
{
    dz_service_providers_t providers;
    dz_userdata_t ud;
} dz_service_t;
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_service_create( dz_service_t ** _service, const dz_service_providers_t * _providers, dz_userdata_t _ud )
{
    dz_service_t * k = (dz_service_t *)(*_providers->f_malloc)(sizeof( dz_service_t ), _ud);

    k->providers = *_providers;
    k->ud = _ud;

    *_service = k;

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

    struct dz_timeline_key_t * key;

    dz_userdata_t ud;
} dz_timeline_interpolate_t;
//////////////////////////////////////////////////////////////////////////
typedef struct dz_timeline_interpolate_none_t
{
    dz_timeline_interpolate_t base;

} dz_timeline_interpolate_none_t;
//////////////////////////////////////////////////////////////////////////
typedef struct dz_timeline_interpolate_randomize_t
{
    dz_timeline_interpolate_t base;

} dz_timeline_interpolate_randomize_t;
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
    dz_timeline_interpolate_t * interpolate = DZ_NEW( _service, dz_timeline_interpolate_t );

    interpolate->type = _type;
    interpolate->ud = _ud;

    *_interpolate = interpolate;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_timeline_interpolate_destroy( dz_service_t * _service, dz_timeline_interpolate_t * _interpolate )
{
    DZ_FREE( _service, _interpolate );
}
//////////////////////////////////////////////////////////////////////////
typedef struct dz_timeline_key_t
{
    float time;
    float value;

    dz_timeline_interpolate_t * interpolate;

    dz_userdata_t ud;
} dz_timeline_key_t;
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_timeline_key_create( dz_service_t * _service, dz_timeline_key_t ** _key, float _time, float _value, dz_userdata_t _ud )
{
    dz_timeline_key_t * key = DZ_NEW( _service, dz_timeline_key_t );

    key->time = _time;
    key->value = _value;
    key->ud = _ud;

    *_key = key;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_timeline_key_destroy( dz_service_t * _service, dz_timeline_key_t * _key )
{
    DZ_FREE( _service, _key );
}
//////////////////////////////////////////////////////////////////////////
typedef struct dz_affector_data_t
{
    dz_timeline_key_t * life;
    dz_timeline_key_t * chance_extra_life;
    dz_timeline_key_t * extra_life;
    dz_timeline_key_t * move_speed;
    dz_timeline_key_t * move_accelerate;
    dz_timeline_key_t * rotate_speed;
    dz_timeline_key_t * rotate_accelerate;
    dz_timeline_key_t * size;
    dz_timeline_key_t * transparent;
    dz_timeline_key_t * color_r;
    dz_timeline_key_t * color_g;
    dz_timeline_key_t * color_b;
} dz_affector_data_t;
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_affector_data_create( dz_service_t * _service, dz_affector_data_t ** _affector_data )
{
    dz_affector_data_t * affector_data = DZ_NEW( _service, dz_affector_data_t );

    affector_data->life = DZ_NULLPTR;
    affector_data->chance_extra_life = DZ_NULLPTR;
    affector_data->extra_life = DZ_NULLPTR;
    affector_data->move_speed = DZ_NULLPTR;
    affector_data->move_accelerate = DZ_NULLPTR;
    affector_data->rotate_speed = DZ_NULLPTR;
    affector_data->rotate_accelerate = DZ_NULLPTR;
    affector_data->size = DZ_NULLPTR;
    affector_data->transparent = DZ_NULLPTR;

    *_affector_data = affector_data;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_affector_data_destory( dz_service_t * _service, dz_affector_data_t * _affector_data )
{
    DZ_FREE( _service, _affector_data->life );
    DZ_FREE( _service, _affector_data->chance_extra_life );
    DZ_FREE( _service, _affector_data->extra_life );
    DZ_FREE( _service, _affector_data->move_speed );
    DZ_FREE( _service, _affector_data->move_accelerate );
    DZ_FREE( _service, _affector_data->rotate_speed );
    DZ_FREE( _service, _affector_data->rotate_accelerate );
    DZ_FREE( _service, _affector_data->size );
    DZ_FREE( _service, _affector_data->transparent );

    DZ_FREE( _service, _affector_data );
}
//////////////////////////////////////////////////////////////////////////
typedef struct dz_emitter_data_t
{
    dz_emitter_shape_type_e shape_type;

    float life;

    dz_timeline_key_t * spawn_delay;
    dz_timeline_key_t * spawn_count;

    dz_userdata_t ud;
} dz_emitter_data_t;
//////////////////////////////////////////////////////////////////////////
typedef struct dz_emitter_data_point_t
{
    dz_emitter_data_t base;
    
    dz_timeline_key_t * x;
    dz_timeline_key_t * y;
} dz_emitter_data_point_t;
//////////////////////////////////////////////////////////////////////////
typedef struct dz_emitter_data_circle_t
{
    dz_emitter_data_t base;

    dz_timeline_key_t * x;
    dz_timeline_key_t * y;

    dz_timeline_key_t * r;
} dz_emitter_data_circle_t;
//////////////////////////////////////////////////////////////////////////
typedef struct dz_emitter_data_line_t
{
    dz_emitter_data_t base;

    dz_timeline_key_t * bx;
    dz_timeline_key_t * by;

    dz_timeline_key_t * ex;
    dz_timeline_key_t * ey;
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
    DZ_FREE( _service, _emitter_data );
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
static float __get_timeline_value( dz_timeline_value_t * _value, float _time )
{
    for( const dz_timeline_key_t * current = _value->current; current != DZ_NULLPTR && current->interpolate != DZ_NULLPTR; current = current->interpolate->key )
    {
        float dt = _time - _value->time;

        if( current->time < dt )
        {
            return current->value;
        }

        _value->time += current->time;

        _value->current = _value->current->interpolate->key;
    }

    return _value->current->value;

}
//////////////////////////////////////////////////////////////////////////
typedef struct dz_particle_t
{
    float life;
    float time;    

    float x;
    float y;
    
    float angle;

    float move_speed;
    float move_accelerate;
    float move_accelerate_aux;

    float rotate_speed;
    float rotate_accelerate;
    float rotate_accelerate_aux;

    float size;

    float transparent;
    float color_r;
    float color_g;
    float color_b;

    struct dz_particle_t * next;
    struct dz_particle_t * prev;
} dz_particle_t;
//////////////////////////////////////////////////////////////////////////
typedef struct dz_emitter_t
{
    const dz_emitter_data_t * emitter_data;
    const dz_affector_data_t * affector_data;

    dz_particle_t * partices;

    float time;
    float emitter_time;

    dz_timeline_value_t * spawn_delay;
    dz_timeline_value_t * spawn_count;

    dz_timeline_value_t * life;
    dz_timeline_value_t * chance_extra_life;
    dz_timeline_value_t * extra_life;

    dz_timeline_value_t * move_speed;
    dz_timeline_value_t * move_accelerate;
    dz_timeline_value_t * rotate_speed;
    dz_timeline_value_t * rotate_accelerate;
    dz_timeline_value_t * size;
    dz_timeline_value_t * transparent;
    dz_timeline_value_t * color_r;
    dz_timeline_value_t * color_g;
    dz_timeline_value_t * color_b;
} dz_emitter_t;
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_emitter_create( dz_service_t * _service, const dz_emitter_data_t * _emitter_data, const dz_affector_data_t * _affector_data, dz_emitter_t ** _emitter )
{
    dz_emitter_t * emitter = DZ_NEW( _service, dz_emitter_t );

    emitter->emitter_data = _emitter_data;
    emitter->affector_data = _affector_data;

    emitter->partices = DZ_NULLPTR;

    emitter->time = 0.f;
    emitter->emitter_time = 0.f;

    emitter->spawn_delay = __new_timeline_value( _service, emitter->emitter_data->spawn_delay );
    emitter->spawn_count = __new_timeline_value( _service, emitter->emitter_data->spawn_count );

    emitter->life = __new_timeline_value( _service, emitter->affector_data->life );
    emitter->chance_extra_life = __new_timeline_value( _service, emitter->affector_data->chance_extra_life );
    emitter->extra_life = __new_timeline_value( _service, emitter->affector_data->extra_life );
    emitter->move_speed = __new_timeline_value( _service, emitter->affector_data->move_speed );
    emitter->move_accelerate = __new_timeline_value( _service, emitter->affector_data->move_accelerate );
    emitter->rotate_speed = __new_timeline_value( _service, emitter->affector_data->rotate_speed );
    emitter->rotate_accelerate = __new_timeline_value( _service, emitter->affector_data->rotate_accelerate );
    emitter->size = __new_timeline_value( _service, emitter->affector_data->size );
    emitter->transparent = __new_timeline_value( _service, emitter->affector_data->transparent );
    emitter->color_r = __new_timeline_value( _service, emitter->affector_data->color_r );
    emitter->color_g = __new_timeline_value( _service, emitter->affector_data->color_g );
    emitter->color_b = __new_timeline_value( _service, emitter->affector_data->color_b );

    *_emitter = emitter;

    return DZ_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
void dz_emitter_destroy( dz_service_t * _service, dz_emitter_t * _emitter )
{
    DZ_FREE( _service, _emitter );
}
//////////////////////////////////////////////////////////////////////////
static float __get_emitter_value( dz_emitter_t * _emitter, dz_timeline_value_t * _value )
{
    float value = __get_timeline_value( _value, _emitter->time );

    return value;
}
//////////////////////////////////////////////////////////////////////////
static void __particle_update( dz_service_t * _service, dz_emitter_t * _emitter, dz_particle_t * _particle, float _time )
{
    _particle->move_speed = __get_timeline_value( _emitter->move_speed, _time );
    _particle->move_accelerate = __get_timeline_value( _emitter->move_accelerate, _time );

    _particle->rotate_speed = __get_timeline_value( _emitter->rotate_speed, _time );
    _particle->rotate_accelerate = __get_timeline_value( _emitter->rotate_accelerate, _time );
    _particle->rotate_accelerate_aux = 0.f;

    _particle->size = __get_timeline_value( _emitter->size, _time );

    _particle->transparent = __get_timeline_value( _emitter->transparent, _time );
    _particle->color_r = __get_timeline_value( _emitter->color_r, _time );
    _particle->color_g = __get_timeline_value( _emitter->color_g, _time );
    _particle->color_b = __get_timeline_value( _emitter->color_b, _time );

    _particle->rotate_accelerate_aux += _particle->rotate_accelerate * _time;

    _particle->angle += _particle->rotate_speed + _particle->rotate_accelerate_aux;

    float dx = _service->providers.f_cosf( _particle->angle, _service->ud );
    float dy = _service->providers.f_sinf( _particle->angle, _service->ud );

    _particle->move_accelerate_aux += _particle->move_accelerate * _time;

    _particle->x += dx * (_particle->move_speed + _particle->move_accelerate_aux);
    _particle->y += dy * (_particle->move_speed + _particle->move_accelerate_aux);
}
//////////////////////////////////////////////////////////////////////////
static void __emitter_spawn( dz_service_t * _service, dz_emitter_t * _emitter, float _life, float _time )
{
    dz_particle_t * p = DZ_NEW( _service, dz_particle_t );

    p->life = _life;
    p->time = _time;

    p->x = 0.f;
    p->y = 0.f;

    p->angle = 0.f;

    p->move_accelerate_aux = 0.f;
    p->rotate_accelerate_aux = 0.f;

    __particle_update( _service, _emitter, p, _time );

    DZ_LIST_PUSHBACK( _emitter->partices, p );
}
//////////////////////////////////////////////////////////////////////////
void dz_emitter_update( dz_service_t * _service, dz_emitter_t * _emitter, float _time )
{
    DZ_UNUSED( _service );

    _emitter->time += _time;

    DZ_LIST_FOREACH( dz_particle_t, _emitter->partices, p )
    {
        __particle_update( _service, _emitter, p, _time );
    }

    for( ;;)
    {
        float delay = __get_timeline_value( _emitter->spawn_delay, _emitter->emitter_time );

        if( _emitter->emitter_time + delay < _emitter->time )
        {
            break;
        }

        float spawn_time = _emitter->emitter_time + delay;

        float count = __get_timeline_value( _emitter->spawn_count, spawn_time );

        while( count > 0.f )
        {
            float life = __get_timeline_value( _emitter->life, spawn_time );
            float chance_extra_life = __get_timeline_value( _emitter->chance_extra_life, spawn_time );

            if( _service->providers.f_randomize( 1.f, _service->ud ) <= chance_extra_life )
            {
                float extra_life = __get_timeline_value( _emitter->extra_life, spawn_time );

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