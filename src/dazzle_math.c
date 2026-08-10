#include "dazzle/dazzle.h"

#include "math3d.h"

#include <math.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////
dz_vec3_t dz_math_vec3( dz_float_t _x, dz_float_t _y, dz_float_t _z )
{
    dz_vec3_t value = { _x, _y, _z };
    return value;
}
//////////////////////////////////////////////////////////////////////////
dz_vec3_t dz_math_add3( dz_vec3_t _a, dz_vec3_t _b )
{
    return dz_math_vec3( _a.x + _b.x, _a.y + _b.y, _a.z + _b.z );
}
//////////////////////////////////////////////////////////////////////////
dz_vec3_t dz_math_sub3( dz_vec3_t _a, dz_vec3_t _b )
{
    return dz_math_vec3( _a.x - _b.x, _a.y - _b.y, _a.z - _b.z );
}
//////////////////////////////////////////////////////////////////////////
dz_vec3_t dz_math_mul3( dz_vec3_t _v, dz_float_t _scalar )
{
    return dz_math_vec3( _v.x * _scalar, _v.y * _scalar, _v.z * _scalar );
}
//////////////////////////////////////////////////////////////////////////
dz_float_t dz_math_dot3( dz_vec3_t _a, dz_vec3_t _b )
{
    return _a.x * _b.x + _a.y * _b.y + _a.z * _b.z;
}
//////////////////////////////////////////////////////////////////////////
dz_vec3_t dz_math_cross3( dz_vec3_t _a, dz_vec3_t _b )
{
    return dz_math_vec3( _a.y * _b.z - _a.z * _b.y, _a.z * _b.x - _a.x * _b.z, _a.x * _b.y - _a.y * _b.x );
}
//////////////////////////////////////////////////////////////////////////
dz_float_t dz_math_length3( dz_vec3_t _v )
{
    return sqrtf( dz_math_dot3( _v, _v ) );
}
//////////////////////////////////////////////////////////////////////////
dz_vec3_t dz_math_normalize3( dz_vec3_t _v, dz_vec3_t _fallback )
{
    const dz_float_t length = dz_math_length3( _v );

    if( length <= 0.000001f || isfinite( length ) == 0 )
    {
        return _fallback;
    }

    return dz_math_mul3( _v, 1.f / length );
}
//////////////////////////////////////////////////////////////////////////
dz_quat_t dz_math_quat_identity( void )
{
    dz_quat_t value = { 0.f, 0.f, 0.f, 1.f };
    return value;
}
//////////////////////////////////////////////////////////////////////////
dz_quat_t dz_math_quat_normalize( dz_quat_t _q )
{
    const dz_float_t length = sqrtf( _q.x * _q.x + _q.y * _q.y + _q.z * _q.z + _q.w * _q.w );

    if( length <= 0.000001f || isfinite( length ) == 0 )
    {
        return dz_math_quat_identity();
    }

    const dz_float_t inverse = 1.f / length;
    dz_quat_t value = { _q.x * inverse, _q.y * inverse, _q.z * inverse, _q.w * inverse };
    return value;
}
//////////////////////////////////////////////////////////////////////////
void dz_quat_to_euler_xyz_degrees( const dz_quat_t * _quat, dz_vec3_t * _euler )
{
    const dz_quat_t q = dz_math_quat_normalize( *_quat );
    const dz_float_t sinr = 2.f * ( q.w * q.x + q.y * q.z );
    const dz_float_t cosr = 1.f - 2.f * ( q.x * q.x + q.y * q.y );
    const dz_float_t sinp = 2.f * ( q.w * q.y - q.z * q.x );
    const dz_float_t siny = 2.f * ( q.w * q.z + q.x * q.y );
    const dz_float_t cosy = 1.f - 2.f * ( q.y * q.y + q.z * q.z );
    const dz_float_t radians_to_degrees = 180.f / DZ_PI;

    _euler->x = atan2f( sinr, cosr ) * radians_to_degrees;
    _euler->y = ( fabsf( sinp ) >= 1.f ? copysignf( DZ_PI * 0.5f, sinp ) : asinf( sinp ) ) * radians_to_degrees;
    _euler->z = atan2f( siny, cosy ) * radians_to_degrees;
}
//////////////////////////////////////////////////////////////////////////
void dz_quat_from_euler_xyz_degrees( const dz_vec3_t * _euler, dz_quat_t * _quat )
{
    const dz_float_t degrees_to_half_radians = DZ_PI / 360.f;
    const dz_float_t cr = cosf( _euler->x * degrees_to_half_radians );
    const dz_float_t sr = sinf( _euler->x * degrees_to_half_radians );
    const dz_float_t cp = cosf( _euler->y * degrees_to_half_radians );
    const dz_float_t sp = sinf( _euler->y * degrees_to_half_radians );
    const dz_float_t cy = cosf( _euler->z * degrees_to_half_radians );
    const dz_float_t sy = sinf( _euler->z * degrees_to_half_radians );

    _quat->w = cr * cp * cy + sr * sp * sy;
    _quat->x = sr * cp * cy - cr * sp * sy;
    _quat->y = cr * sp * cy + sr * cp * sy;
    _quat->z = cr * cp * sy - sr * sp * cy;
}
//////////////////////////////////////////////////////////////////////////
dz_vec3_t dz_math_quat_rotate3( dz_quat_t _q, dz_vec3_t _v )
{
    _q = dz_math_quat_normalize( _q );

    const dz_vec3_t qv = { _q.x, _q.y, _q.z };
    const dz_vec3_t twice_cross = dz_math_mul3( dz_math_cross3( qv, _v ), 2.f );

    return dz_math_add3( _v, dz_math_add3( dz_math_mul3( twice_cross, _q.w ), dz_math_cross3( qv, twice_cross ) ) );
}
//////////////////////////////////////////////////////////////////////////
dz_vec3_t dz_math_transform_point( const dz_transform_t * _transform, dz_vec3_t _point )
{
    dz_vec3_t scaled = { _point.x * _transform->scale.x, _point.y * _transform->scale.y, _point.z * _transform->scale.z };

    return dz_math_add3( dz_math_quat_rotate3( _transform->rotation, scaled ), _transform->position );
}
//////////////////////////////////////////////////////////////////////////
dz_bool_t dz_math_is_finite3( dz_vec3_t _v )
{
    return isfinite( _v.x ) != 0 && isfinite( _v.y ) != 0 && isfinite( _v.z ) != 0 ? DZ_TRUE : DZ_FALSE;
}
//////////////////////////////////////////////////////////////////////////
void dz_mat4_identity( dz_mat4_t * _matrix )
{
    memset( _matrix->m, 0, sizeof( _matrix->m ) );
    _matrix->m[0] = 1.f;
    _matrix->m[5] = 1.f;
    _matrix->m[10] = 1.f;
    _matrix->m[15] = 1.f;
}
//////////////////////////////////////////////////////////////////////////
void dz_project_profile_default( dz_project_profile_t * _profile, dz_projection_type_e _projection )
{
    _profile->projection = _projection;
    _profile->position = dz_math_vec3( 0.f, 0.f, 10.f );
    _profile->forward = dz_math_vec3( 0.f, 0.f, -1.f );
    _profile->up = dz_math_vec3( 0.f, 1.f, 0.f );
    _profile->field_of_view = 60.f;
    _profile->orthographic_height = 10.f;
    _profile->near_plane = 0.01f;
    _profile->far_plane = 1000.f;
}
//////////////////////////////////////////////////////////////////////////
void dz_camera_state_from_profile( const dz_project_profile_t * _profile, dz_float_t _viewport_width, dz_float_t _viewport_height, dz_camera_state_t * _camera )
{
    _camera->projection = _profile->projection;
    _camera->position = _profile->position;
    _camera->forward = _profile->forward;
    _camera->up = _profile->up;
    _camera->field_of_view = _profile->field_of_view;
    _camera->orthographic_height = _profile->orthographic_height;
    _camera->near_plane = _profile->near_plane;
    _camera->far_plane = _profile->far_plane;
    _camera->viewport_width = _viewport_width;
    _camera->viewport_height = _viewport_height;

}
//////////////////////////////////////////////////////////////////////////
void dz_camera_compute_view( const dz_camera_state_t * _camera, dz_mat4_t * _view )
{
    const dz_vec3_t forward = dz_math_normalize3( _camera->forward, dz_math_vec3( 0.f, 0.f, -1.f ) );
    const dz_vec3_t right = dz_math_normalize3( dz_math_cross3( forward, _camera->up ), dz_math_vec3( 1.f, 0.f, 0.f ) );
    const dz_vec3_t up = dz_math_cross3( right, forward );

    dz_mat4_identity( _view );
    _view->m[0] = right.x;
    _view->m[4] = right.y;
    _view->m[8] = right.z;
    _view->m[1] = up.x;
    _view->m[5] = up.y;
    _view->m[9] = up.z;
    _view->m[2] = -forward.x;
    _view->m[6] = -forward.y;
    _view->m[10] = -forward.z;
    _view->m[12] = -dz_math_dot3( right, _camera->position );
    _view->m[13] = -dz_math_dot3( up, _camera->position );
    _view->m[14] = dz_math_dot3( forward, _camera->position );

}
//////////////////////////////////////////////////////////////////////////
void dz_camera_compute_projection( const dz_camera_state_t * _camera, dz_mat4_t * _projection )
{
    const dz_float_t aspect = _camera->viewport_width / _camera->viewport_height;
    memset( _projection->m, 0, sizeof( _projection->m ) );

    if( _camera->projection == DZ_PROJECTION_PERSPECTIVE )
    {
        const dz_float_t radians = _camera->field_of_view * ( DZ_PI / 180.f );
        const dz_float_t tangent = tanf( radians * 0.5f );
        const dz_float_t factor = 1.f / tangent;
        _projection->m[0] = factor / aspect;
        _projection->m[5] = factor;
        _projection->m[10] = ( _camera->far_plane + _camera->near_plane ) / ( _camera->near_plane - _camera->far_plane );
        _projection->m[11] = -1.f;
        _projection->m[14] = ( 2.f * _camera->far_plane * _camera->near_plane ) / ( _camera->near_plane - _camera->far_plane );
    }
    else
    {
        const dz_float_t height = _camera->orthographic_height;
        const dz_float_t width = height * aspect;
        _projection->m[0] = 2.f / width;
        _projection->m[5] = 2.f / height;
        _projection->m[10] = -2.f / ( _camera->far_plane - _camera->near_plane );
        _projection->m[14] = -( _camera->far_plane + _camera->near_plane ) / ( _camera->far_plane - _camera->near_plane );
        _projection->m[15] = 1.f;
    }
}
//////////////////////////////////////////////////////////////////////////
static dz_mat4_t __mat4_multiply( const dz_mat4_t * _a, const dz_mat4_t * _b )
{
    dz_mat4_t result;
    for( dz_uint32_t column = 0; column != 4U; ++column )
    {
        for( dz_uint32_t row = 0; row != 4U; ++row )
        {
            result.m[column * 4U + row] = _a->m[0U * 4U + row] * _b->m[column * 4U + 0U] + _a->m[1U * 4U + row] * _b->m[column * 4U + 1U] +
                                          _a->m[2U * 4U + row] * _b->m[column * 4U + 2U] + _a->m[3U * 4U + row] * _b->m[column * 4U + 3U];
        }
    }
    return result;
}
//////////////////////////////////////////////////////////////////////////
void dz_camera_test_aabb( const dz_camera_state_t * _camera, const dz_aabb_t * _aabb, dz_bool_t * _visible )
{
    dz_mat4_t view;
    dz_mat4_t projection;
    dz_camera_compute_view( _camera, &view );
    dz_camera_compute_projection( _camera, &projection );
    const dz_mat4_t view_projection = __mat4_multiply( &projection, &view );

    dz_uint32_t outside_left = 0U, outside_right = 0U, outside_bottom = 0U, outside_top = 0U, outside_near = 0U, outside_far = 0U;
    for( dz_uint32_t corner = 0; corner != 8U; ++corner )
    {
        const dz_float_t x = ( corner & 1U ) != 0U ? _aabb->maximum.x : _aabb->minimum.x;
        const dz_float_t y = ( corner & 2U ) != 0U ? _aabb->maximum.y : _aabb->minimum.y;
        const dz_float_t z = ( corner & 4U ) != 0U ? _aabb->maximum.z : _aabb->minimum.z;
        const dz_float_t clip_x = view_projection.m[0] * x + view_projection.m[4] * y + view_projection.m[8] * z + view_projection.m[12];
        const dz_float_t clip_y = view_projection.m[1] * x + view_projection.m[5] * y + view_projection.m[9] * z + view_projection.m[13];
        const dz_float_t clip_z = view_projection.m[2] * x + view_projection.m[6] * y + view_projection.m[10] * z + view_projection.m[14];
        const dz_float_t clip_w = view_projection.m[3] * x + view_projection.m[7] * y + view_projection.m[11] * z + view_projection.m[15];
        outside_left += clip_x < -clip_w ? 1U : 0U;
        outside_right += clip_x > clip_w ? 1U : 0U;
        outside_bottom += clip_y < -clip_w ? 1U : 0U;
        outside_top += clip_y > clip_w ? 1U : 0U;
        outside_near += clip_z < -clip_w ? 1U : 0U;
        outside_far += clip_z > clip_w ? 1U : 0U;
    }

    *_visible = outside_left == 8U || outside_right == 8U || outside_bottom == 8U || outside_top == 8U || outside_near == 8U || outside_far == 8U ? DZ_FALSE : DZ_TRUE;
}
