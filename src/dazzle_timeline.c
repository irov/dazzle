#include "dazzle/dazzle.h"

#include "alloc.h"
#include "timeline_interpolate.h"
#include "timeline_key.h"

//////////////////////////////////////////////////////////////////////////
void dz_timeline_interpolate_create( const dz_service_t * _service, dz_timeline_interpolate_t ** _interpolate, dz_timeline_interpolate_type_e _type, dz_userdata_t _ud )
{
    dz_timeline_interpolate_t * interpolate = DZ_NEW( _service, dz_timeline_interpolate_t );

    interpolate->type = _type;
    interpolate->key = DZ_NULLPTR;
    interpolate->ud = _ud;

    interpolate->p0 = 0.f;
    interpolate->p1 = 0.f;
    interpolate->out_tangent = 0.f;
    interpolate->in_tangent = 0.f;

    *_interpolate = interpolate;

}
//////////////////////////////////////////////////////////////////////////
void dz_timeline_interpolate_destroy( const dz_service_t * _service, const dz_timeline_interpolate_t * _interpolate )
{
    if( _interpolate->key != DZ_NULLPTR )
    {
        dz_timeline_key_destroy( _service, _interpolate->key );
    }

    DZ_FREE( _service, _interpolate );
}
//////////////////////////////////////////////////////////////////////////
void dz_timeline_interpolate_set_ud( dz_timeline_interpolate_t * const _interpolate, dz_userdata_t _ud )
{
    _interpolate->ud = _ud;
}
//////////////////////////////////////////////////////////////////////////
dz_userdata_t dz_timeline_interpolate_get_ud( const dz_timeline_interpolate_t * _interpolate )
{
    return _interpolate->ud;
}
//////////////////////////////////////////////////////////////////////////
void dz_timeline_interpolate_set_type( dz_timeline_interpolate_t * const _interpolate, dz_timeline_interpolate_type_e _type )
{
    _interpolate->type = _type;
}
//////////////////////////////////////////////////////////////////////////
dz_timeline_interpolate_type_e dz_timeline_interpolate_get_type( const dz_timeline_interpolate_t * _interpolate )
{
    return _interpolate->type;
}
//////////////////////////////////////////////////////////////////////////
void dz_timeline_interpolate_set_bezier2( dz_timeline_interpolate_t * const _interpolate, dz_float_t _p0, dz_float_t _p1 )
{
    _interpolate->p0 = _p0;
    _interpolate->p1 = _p1;
}
//////////////////////////////////////////////////////////////////////////
void dz_timeline_interpolate_get_bezier2( const dz_timeline_interpolate_t * _interpolate, dz_float_t * const _p0, dz_float_t * const _p1 )
{
    *_p0 = _interpolate->p0;
    *_p1 = _interpolate->p1;
}
//////////////////////////////////////////////////////////////////////////
void dz_timeline_interpolate_set_hermite( dz_timeline_interpolate_t * const _interpolate, dz_float_t _out_tangent, dz_float_t _in_tangent )
{
    _interpolate->out_tangent = _out_tangent;
    _interpolate->in_tangent = _in_tangent;
}
//////////////////////////////////////////////////////////////////////////
void dz_timeline_interpolate_get_hermite( const dz_timeline_interpolate_t * _interpolate, dz_float_t * const _out_tangent, dz_float_t * const _in_tangent )
{
    *_out_tangent = _interpolate->out_tangent;
    *_in_tangent = _interpolate->in_tangent;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_timeline_key_create( const dz_service_t * _service, dz_timeline_key_t ** _key, dz_float_t _p, dz_timeline_key_type_e _type, dz_userdata_t _ud )
{
#ifdef DZ_DEBUG
    if( _p < 0.f || _p > 1.f )
    {
        return DZ_FAILURE;
    }
#endif

    dz_timeline_key_t * key = DZ_NEW( _service, dz_timeline_key_t );

    key->p = _p;
    key->d_inv = 0.f;
    key->type = _type;
    key->interpolate = DZ_NULLPTR;
    key->ud = _ud;
    key->const_value = 0.f;
    key->randomize_min_value = 0.f;
    key->randomize_max_value = 0.f;

    *_key = key;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_timeline_key_destroy( const dz_service_t * _service, const dz_timeline_key_t * _key )
{
    if( _key->interpolate != DZ_NULLPTR )
    {
        dz_timeline_interpolate_destroy( _service, _key->interpolate );
    }

    DZ_FREE( _service, _key );
}
//////////////////////////////////////////////////////////////////////////
void dz_timeline_key_set_ud( dz_timeline_key_t * const _key, dz_userdata_t _ud )
{
    _key->ud = _ud;
}
//////////////////////////////////////////////////////////////////////////
dz_userdata_t dz_timeline_key_get_ud( const dz_timeline_key_t * _key )
{
    return _key->ud;
}
//////////////////////////////////////////////////////////////////////////
void dz_timeline_key_set_type( dz_timeline_key_t * const _key, dz_timeline_key_type_e _type )
{
    _key->type = _type;
}
//////////////////////////////////////////////////////////////////////////
dz_timeline_key_type_e dz_timeline_key_get_type( const dz_timeline_key_t * _key )
{
    return _key->type;
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
void dz_timeline_interpolate_set_key( dz_timeline_interpolate_t * const _interpolate, dz_timeline_key_t * const _key )
{
    _interpolate->key = _key;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_timeline_key_set_interpolate( dz_timeline_key_t * const _key, dz_timeline_interpolate_t * const _interpolate )
{
#ifdef DZ_DEBUG
    if( _interpolate != DZ_NULLPTR &&
        _interpolate->key == DZ_NULLPTR )
    {
        return DZ_FAILURE;
    }
#endif

    _key->interpolate = _interpolate;

    if( _interpolate != DZ_NULLPTR )
    {
        _key->d_inv = 1.f / (_interpolate->key->p - _key->p);
    }
    else
    {
        _key->d_inv = 0.f;
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_timeline_key_set_p( dz_timeline_key_t * const _key, dz_float_t _p )
{
    _key->p = _p;
}
//////////////////////////////////////////////////////////////////////////
dz_float_t dz_timeline_key_get_p( const dz_timeline_key_t * _key )
{
    return _key->p;
}
//////////////////////////////////////////////////////////////////////////
void dz_timeline_key_set_const_value( dz_timeline_key_t * const _key, dz_float_t _value )
{
    _key->const_value = _value;
}
//////////////////////////////////////////////////////////////////////////
void dz_timeline_key_get_const_value( const dz_timeline_key_t * _key, dz_float_t * const _value )
{
    *_value = _key->const_value;
}
//////////////////////////////////////////////////////////////////////////
void dz_timeline_key_set_randomize_min_max( dz_timeline_key_t * const _key, dz_float_t _min, dz_float_t _max )
{
    _key->randomize_min_value = _min;
    _key->randomize_max_value = _max;
}
//////////////////////////////////////////////////////////////////////////
void dz_timeline_key_get_randomize_min_max( const dz_timeline_key_t * _key, dz_float_t * const _min, dz_float_t * const _max )
{
    *_min = _key->randomize_min_value;
    *_max = _key->randomize_max_value;
}
