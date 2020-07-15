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
typedef struct dz_timeline_data_t
{
    float time;
} dz_timeline_data_t;
//////////////////////////////////////////////////////////////////////////
typedef struct dz_emitter_data_t
{
    dz_emitter_shape_type_e shape_type;

    dz_timeline_data_t * delay_min;
    dz_timeline_data_t * delay_max;

    dz_userdata_t ud;
} dz_emitter_data_t;
//////////////////////////////////////////////////////////////////////////
typedef struct dz_emitter_data_point_t
{
    dz_emitter_data_t data;
    
    dz_timeline_data_t * x;
    dz_timeline_data_t * y;
} dz_emitter_data_point_t;
//////////////////////////////////////////////////////////////////////////
typedef struct dz_emitter_data_circle_t
{
    dz_emitter_data_t data;

    dz_timeline_data_t * x;
    dz_timeline_data_t * y;

    dz_timeline_data_t * r;
} dz_emitter_data_circle_t;
//////////////////////////////////////////////////////////////////////////
typedef struct dz_emitter_data_line_t
{
    dz_emitter_data_t data;

    dz_timeline_data_t * bx;
    dz_timeline_data_t * by;

    dz_timeline_data_t * ex;
    dz_timeline_data_t * ey;
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