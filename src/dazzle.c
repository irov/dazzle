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
    dz_timeline_key_t * scale;
    dz_timeline_key_t * transparent;
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
    affector_data->scale = DZ_NULLPTR;
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
    DZ_FREE( _service, _affector_data->scale );
    DZ_FREE( _service, _affector_data->transparent );

    DZ_FREE( _service, _affector_data );
}
//////////////////////////////////////////////////////////////////////////
typedef struct dz_emitter_data_t
{
    dz_emitter_shape_type_e shape_type;

    dz_timeline_key_t * life;
    dz_timeline_key_t * delay;

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
typedef struct dz_emitter_t
{
    const dz_emitter_data_t * emitter_data;
    const dz_affector_data_t * affector_data;

    float time;
} dz_emitter_t;
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_emitter_create( dz_service_t * _service, const dz_emitter_data_t * _emitter_data, dz_emitter_t ** _emitter )
{
    dz_emitter_t * emitter = DZ_NEW( _service, dz_emitter_t );

    emitter->emitter_data = _emitter_data;
    emitter->time = 0.f;

    *_emitter = emitter;

    return DZ_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
void dz_emitter_destroy( dz_service_t * _service, dz_emitter_t * _emitter )
{
    DZ_FREE( _service, _emitter );
}
//////////////////////////////////////////////////////////////////////////
void dz_emitter_update( dz_service_t * _service, dz_emitter_t * _emitter, float _time )
{
    DZ_UNUSED( _service );

    _emitter->time += _time;
}
//////////////////////////////////////////////////////////////////////////