#include "dazzle/dazzle.h"

#include "affector.h"
#include "alloc.h"
#include "timeline_key.h"
#include "timeline_limits.h"

//////////////////////////////////////////////////////////////////////////
static const dz_timeline_limits_t affector_timeline_limits[__DZ_AFFECTOR_TIMELINE_MAX__] = {
    { DZ_TIMELINE_LIMIT_MAX, 0.f, DZ_FLT_MAX, 5.f, 10.f },             // DZ_AFFECTOR_TIMELINE_LIFE
    { DZ_TIMELINE_LIMIT_MAX, 0.f, DZ_FLT_MAX, 50.f, 100.f },           // DZ_AFFECTOR_TIMELINE_MOVE_SPEED
    { DZ_TIMELINE_LIMIT_MINMAX, DZ_FLT_MIN, DZ_FLT_MAX, 0.f, 100.f },  // DZ_AFFECTOR_TIMELINE_MOVE_ACCELERATE
    { DZ_TIMELINE_LIMIT_MINMAX, DZ_FLT_MIN, DZ_FLT_MAX, 0.f, DZ_PI2 }, // DZ_AFFECTOR_TIMELINE_ROTATE_SPEED
    { DZ_TIMELINE_LIMIT_MINMAX, DZ_FLT_MIN, DZ_FLT_MAX, 0.f, DZ_PI2 }, // DZ_AFFECTOR_TIMELINE_ROTATE_ACCELERATE
    { DZ_TIMELINE_LIMIT_MINMAX, DZ_FLT_MIN, DZ_FLT_MAX, 0.f, DZ_PI2 }, // DZ_AFFECTOR_TIMELINE_SPIN_SPEED
    { DZ_TIMELINE_LIMIT_MINMAX, DZ_FLT_MIN, DZ_FLT_MAX, 0.f, DZ_PI2 }, // DZ_AFFECTOR_TIMELINE_SPIN_ACCELERATE
    { DZ_TIMELINE_LIMIT_MINMAX, DZ_FLT_MIN, DZ_FLT_MAX, 0.f, 1.f },    // DZ_AFFECTOR_TIMELINE_STRAFE_SPEED
    { DZ_TIMELINE_LIMIT_MINMAX, DZ_FLT_MIN, DZ_FLT_MAX, 0.f, DZ_PI2 }, // DZ_AFFECTOR_TIMELINE_STRAFE_FRENQUENCE
    { DZ_TIMELINE_LIMIT_MINMAX, DZ_FLT_MIN, DZ_FLT_MAX, 0.f, 10.f },   // DZ_AFFECTOR_TIMELINE_STRAFE_SIZE
    { DZ_TIMELINE_LIMIT_MINMAX, DZ_PI2N, DZ_PI2, 0.f, 0.f },           // DZ_AFFECTOR_TIMELINE_STRAFE_SHIFT
    { DZ_TIMELINE_LIMIT_MAX, 0.f, DZ_FLT_MAX, 1.f, 2.f },              // DZ_AFFECTOR_TIMELINE_SCALE
    { DZ_TIMELINE_LIMIT_MINMAX, DZ_FLT_MIN, DZ_FLT_MAX, 1.f, 5.f },    // DZ_AFFECTOR_TIMELINE_ASPECT
    { DZ_TIMELINE_LIMIT_NORMAL, 0.f, 1.f, 1.f, 1.f },                  // DZ_AFFECTOR_TIMELINE_COLOR_R
    { DZ_TIMELINE_LIMIT_NORMAL, 0.f, 1.f, 1.f, 1.f },                  // DZ_AFFECTOR_TIMELINE_COLOR_G
    { DZ_TIMELINE_LIMIT_NORMAL, 0.f, 1.f, 1.f, 1.f },                  // DZ_AFFECTOR_TIMELINE_COLOR_B
    { DZ_TIMELINE_LIMIT_NORMAL, 0.f, 1.f, 1.f, 1.f },                  // DZ_AFFECTOR_TIMELINE_COLOR_A
    { DZ_TIMELINE_LIMIT_MINMAX, -1.f, 1.f, 0.f, 1.f },                 // DZ_AFFECTOR_TIMELINE_DIRECTION_Z
    { DZ_TIMELINE_LIMIT_MINMAX, DZ_FLT_MIN, DZ_FLT_MAX, 0.f, 100.f },  // DZ_AFFECTOR_TIMELINE_GRAVITY_X
    { DZ_TIMELINE_LIMIT_MINMAX, DZ_FLT_MIN, DZ_FLT_MAX, 0.f, 100.f },  // DZ_AFFECTOR_TIMELINE_GRAVITY_Y
    { DZ_TIMELINE_LIMIT_MINMAX, DZ_FLT_MIN, DZ_FLT_MAX, 0.f, 100.f },  // DZ_AFFECTOR_TIMELINE_GRAVITY_Z
    { DZ_TIMELINE_LIMIT_MAX, 0.f, DZ_FLT_MAX, 0.f, 10.f }              // DZ_AFFECTOR_TIMELINE_DRAG
};
//////////////////////////////////////////////////////////////////////////
void dz_affector_timeline_get_limit( dz_affector_timeline_type_e _timeline, dz_timeline_limit_status_e * const _status, dz_float_t * const _min, dz_float_t * const _max, dz_float_t * const _default, dz_float_t * const _factor )
{
    const dz_timeline_limits_t * limit = affector_timeline_limits + _timeline;

    *_status = limit->status;
    *_min = limit->min_value;
    *_max = limit->max_value;
    *_default = limit->default_value;
    *_factor = limit->factor_value;
}
//////////////////////////////////////////////////////////////////////////
dz_float_t dz_affector_get_particle_size( void )
{
    return DZ_PARTICLE_SIZE;
}
//////////////////////////////////////////////////////////////////////////
void dz_affector_create( const dz_service_t * _service, dz_affector_t ** _affector, dz_userdata_t _ud )
{
    dz_affector_t * affector = DZ_NEW( _service, dz_affector_t );

    for( dz_uint32_t index = 0; index != __DZ_AFFECTOR_TIMELINE_MAX__; ++index )
    {
        affector->timelines[index] = DZ_NULLPTR;
    }

    affector->ud = _ud;

    *_affector = affector;

}
//////////////////////////////////////////////////////////////////////////
void dz_affector_destroy( const dz_service_t * _service, const dz_affector_t * _affector )
{
    for( dz_uint32_t index = 0; index != __DZ_AFFECTOR_TIMELINE_MAX__; ++index )
    {
        const dz_timeline_key_t * timeline = _affector->timelines[index];

        if( timeline == DZ_NULLPTR )
        {
            continue;
        }

        dz_timeline_key_destroy( _service, timeline );
    }

    DZ_FREE( _service, _affector );
}
//////////////////////////////////////////////////////////////////////////
dz_userdata_t dz_affector_get_ud( const dz_affector_t * _affector )
{
    return _affector->ud;
}
//////////////////////////////////////////////////////////////////////////
void dz_affector_set_timeline( dz_affector_t * const _affector, dz_affector_timeline_type_e _type, const dz_timeline_key_t * _timeline )
{
    _affector->timelines[_type] = _timeline;
}
//////////////////////////////////////////////////////////////////////////
const dz_timeline_key_t * dz_affector_get_timeline( const dz_affector_t * _affector, dz_affector_timeline_type_e _type )
{
    const dz_timeline_key_t * timeline = _affector->timelines[_type];

    return timeline;
}
