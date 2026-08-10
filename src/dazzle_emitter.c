#include "dazzle/dazzle.h"

#include "alloc.h"
#include "emitter.h"
#include "timeline_key.h"
#include "timeline_limits.h"

//////////////////////////////////////////////////////////////////////////
void dz_emitter_create( const dz_service_t * _service, dz_emitter_t ** _emitter, dz_userdata_t _ud )
{
    dz_emitter_t * emitter = DZ_NEW( _service, dz_emitter_t );

    emitter->life = 0.f;

    for( dz_uint32_t index = 0; index != __DZ_EMITTER_TIMELINE_MAX__; ++index )
    {
        emitter->timelines[index] = DZ_NULLPTR;
    }

    emitter->ud = _ud;

    *_emitter = emitter;

}
//////////////////////////////////////////////////////////////////////////
void dz_emitter_destroy( const dz_service_t * _service, const dz_emitter_t * _emitter )
{
    for( dz_uint32_t index = 0; index != __DZ_EMITTER_TIMELINE_MAX__; ++index )
    {
        const dz_timeline_key_t * timeline = _emitter->timelines[index];

        if( timeline == DZ_NULLPTR )
        {
            continue;
        }

        dz_timeline_key_destroy( _service, timeline );
    }

    DZ_FREE( _service, _emitter );
}
//////////////////////////////////////////////////////////////////////////
void dz_emitter_set_ud( dz_emitter_t * const _emitter, dz_userdata_t _ud )
{
    _emitter->ud = _ud;
}
//////////////////////////////////////////////////////////////////////////
dz_userdata_t dz_emitter_get_ud( const dz_emitter_t * _emitter )
{
    return _emitter->ud;
}
//////////////////////////////////////////////////////////////////////////
void dz_emitter_set_life( dz_emitter_t * const _emitter, dz_float_t _life )
{
    _emitter->life = _life;
}
//////////////////////////////////////////////////////////////////////////
dz_float_t dz_emitter_get_life( const dz_emitter_t * _emitter )
{
    return _emitter->life;
}
//////////////////////////////////////////////////////////////////////////
void dz_emitter_set_timeline( dz_emitter_t * _emitter, dz_emitter_timeline_type_e _type, const dz_timeline_key_t * _timeline )
{
    _emitter->timelines[_type] = _timeline;
}
//////////////////////////////////////////////////////////////////////////
const dz_timeline_key_t * dz_emitter_get_timeline( const dz_emitter_t * _emitter, dz_emitter_timeline_type_e _type )
{
    const dz_timeline_key_t * timeline = _emitter->timelines[_type];

    return timeline;
}
//////////////////////////////////////////////////////////////////////////
static const dz_timeline_limits_t emitter_timeline_limits[__DZ_EMITTER_TIMELINE_MAX__] = {
    { DZ_TIMELINE_LIMIT_MAX, 0.0009765625f, DZ_FLT_MAX, 0.1f, 1.f },   // DZ_EMITTER_SPAWN_DELAY
    { DZ_TIMELINE_LIMIT_MAX, 0.f, DZ_FLT_MAX, 5.f, 10.f },             // DZ_EMITTER_SPAWN_COUNT
    { DZ_TIMELINE_LIMIT_MAX, 0.f, DZ_FLT_MAX, 0.f, 10.f },             // DZ_EMITTER_SPAWN_SPIN_MIN
    { DZ_TIMELINE_LIMIT_MAX, 0.f, DZ_FLT_MAX, 1.f, 10.f },             // DZ_EMITTER_SPAWN_SPIN_MAX
    { DZ_TIMELINE_LIMIT_NORMAL, -DZ_PI_HALF, DZ_PI_HALF, 0.f, DZ_PI }, // DZ_EMITTER_SPAWN_ELEVATION_MIN
    { DZ_TIMELINE_LIMIT_NORMAL, -DZ_PI_HALF, DZ_PI_HALF, 0.f, DZ_PI }, // DZ_EMITTER_SPAWN_ELEVATION_MAX
};
//////////////////////////////////////////////////////////////////////////
void dz_emitter_timeline_get_limit( dz_emitter_timeline_type_e _timeline, dz_timeline_limit_status_e * const _status, dz_float_t * const _min, dz_float_t * const _max, dz_float_t * const _default, dz_float_t * const _factor )
{
    const dz_timeline_limits_t * limit = emitter_timeline_limits + _timeline;

    *_status = limit->status;
    *_min = limit->min_value;
    *_max = limit->max_value;
    *_default = limit->default_value;
    *_factor = limit->factor_value;
}
