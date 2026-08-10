#include "dazzle/dazzle.h"

#include "alloc.h"
#include "math3d.h"
#include "shape.h"
#include "timeline_key.h"
#include "timeline_limits.h"

//////////////////////////////////////////////////////////////////////////
void dz_shape_create( const dz_service_t * _service, dz_shape_t ** _shape, dz_shape_type_e _type, dz_userdata_t _ud )
{
    dz_shape_t * shape = DZ_NEW( _service, dz_shape_t );

    shape->type = _type;

    for( dz_uint32_t index = 0; index != __DZ_SHAPE_TIMELINE_MAX__; ++index )
    {
        shape->timelines[index] = DZ_NULLPTR;
    }

    shape->triangles = DZ_NULLPTR;
    shape->triangle_count = 0;
    shape->owns_triangles = DZ_FALSE;

    shape->mask_buffer = DZ_NULLPTR;
    shape->mask_bites = 0;
    shape->mask_pitch = 0;
    shape->mask_width = 0;
    shape->mask_height = 0;
    shape->mask_threshold = 0;
    shape->mask_scale = 1.f;
    shape->owns_mask = DZ_FALSE;

    shape->transform.position = dz_math_vec3( 0.f, 0.f, 0.f );
    shape->transform.rotation = dz_math_quat_identity();
    shape->transform.scale = dz_math_vec3( 1.f, 1.f, 1.f );
    shape->dimensions = dz_math_vec3( 1.f, 1.f, 1.f );
    shape->mesh_id = DZ_RESOURCE_ID_NONE;

    shape->ud = _ud;

    *_shape = shape;

}
//////////////////////////////////////////////////////////////////////////
void dz_shape_destroy( const dz_service_t * _service, const dz_shape_t * _shape )
{
    for( dz_uint32_t index = 0; index != __DZ_SHAPE_TIMELINE_MAX__; ++index )
    {
        const dz_timeline_key_t * timeline = _shape->timelines[index];

        if( timeline == DZ_NULLPTR )
        {
            continue;
        }

        dz_timeline_key_destroy( _service, timeline );
    }

    if( _shape->owns_triangles == DZ_TRUE )
    {
        DZ_FREE( _service, _shape->triangles );
    }

    if( _shape->owns_mask == DZ_TRUE )
    {
        DZ_FREE( _service, _shape->mask_buffer );
    }

    DZ_FREE( _service, _shape );
}
//////////////////////////////////////////////////////////////////////////
void dz_shape_set_ud( dz_shape_t * const _shape, dz_userdata_t _ud )
{
    _shape->ud = _ud;
}
//////////////////////////////////////////////////////////////////////////
dz_userdata_t dz_shape_get_ud( const dz_shape_t * _shape )
{
    return _shape->ud;
}
//////////////////////////////////////////////////////////////////////////
void dz_shape_set_type( dz_shape_t * const _shape, dz_shape_type_e _type )
{
    _shape->type = _type;
}
//////////////////////////////////////////////////////////////////////////
dz_shape_type_e dz_shape_get_type( const dz_shape_t * _shape )
{
    return _shape->type;
}
//////////////////////////////////////////////////////////////////////////
void dz_shape_set_transform( dz_shape_t * _shape, const dz_transform_t * _transform )
{
    _shape->transform = *_transform;
    _shape->transform.rotation = dz_math_quat_normalize( _transform->rotation );
}
//////////////////////////////////////////////////////////////////////////
void dz_shape_get_transform( const dz_shape_t * _shape, dz_transform_t * _transform )
{
    *_transform = _shape->transform;
}
//////////////////////////////////////////////////////////////////////////
void dz_shape_set_dimensions( dz_shape_t * _shape, const dz_vec3_t * _dimensions )
{
    _shape->dimensions = *_dimensions;
}
//////////////////////////////////////////////////////////////////////////
void dz_shape_get_dimensions( const dz_shape_t * _shape, dz_vec3_t * _dimensions )
{
    *_dimensions = _shape->dimensions;
}
//////////////////////////////////////////////////////////////////////////
void dz_shape_set_mesh_id( dz_shape_t * _shape, dz_uint32_t _mesh_id )
{
    _shape->mesh_id = _mesh_id;
}
//////////////////////////////////////////////////////////////////////////
dz_uint32_t dz_shape_get_mesh_id( const dz_shape_t * _shape )
{
    return _shape->mesh_id;
}
//////////////////////////////////////////////////////////////////////////
void dz_shape_set_timeline( dz_shape_t * const _shape, dz_shape_timeline_type_e _type, const dz_timeline_key_t * _timeline )
{
    _shape->timelines[_type] = _timeline;
}
//////////////////////////////////////////////////////////////////////////
const dz_timeline_key_t * dz_shape_get_timeline( const dz_shape_t * _shape, dz_shape_timeline_type_e _type )
{
    const dz_timeline_key_t * timeline = _shape->timelines[_type];

    return timeline;
}
//////////////////////////////////////////////////////////////////////////
static const dz_timeline_limits_t shape_timeline_limits[__DZ_SHAPE_TIMELINE_MAX__] = {
    { DZ_TIMELINE_LIMIT_NORMAL, -DZ_PI2, DZ_PI2, -DZ_PI * 0.25f, DZ_PI2 }, // DZ_SHAPE_SEGMENT_ANGLE_MIN
    { DZ_TIMELINE_LIMIT_NORMAL, -DZ_PI2, DZ_PI2, DZ_PI * 0.25f, DZ_PI2 },  // DZ_SHAPE_SEGMENT_ANGLE_MAX
    { DZ_TIMELINE_LIMIT_MAX, 0.f, DZ_FLT_MAX, 0.f, 100.f },                // DZ_SHAPE_CIRCLE_RADIUS_MIN
    { DZ_TIMELINE_LIMIT_MAX, 0.f, DZ_FLT_MAX, 1.f, 100.f },                // DZ_SHAPE_CIRCLE_RADIUS_MAX
    { DZ_TIMELINE_LIMIT_NORMAL, -DZ_PI2, DZ_PI2, -DZ_PI * 0.05f, DZ_PI2 }, // DZ_SHAPE_CIRCLE_ANGLE_MIN
    { DZ_TIMELINE_LIMIT_NORMAL, -DZ_PI2, DZ_PI2, DZ_PI * 0.05f, DZ_PI2 },  // DZ_SHAPE_CIRCLE_ANGLE_MAX
    { DZ_TIMELINE_LIMIT_NORMAL, -DZ_PI2, DZ_PI2, 0.f, DZ_PI2 },            // DZ_SHAPE_LINE_ANGLE
    { DZ_TIMELINE_LIMIT_MAX, 0.f, DZ_FLT_MAX, 1.f, 100.f },                // DZ_SHAPE_LINE_SIZE
    { DZ_TIMELINE_LIMIT_MINMAX, DZ_FLT_MIN, DZ_FLT_MAX, 0.f, 100.f },      // DZ_SHAPE_LINE_OFFSET
    { DZ_TIMELINE_LIMIT_MAX, 0.f, DZ_FLT_MAX, 0.f, 100.f },                // DZ_SHAPE_RECT_WIDTH_MIN
    { DZ_TIMELINE_LIMIT_MAX, 0.f, DZ_FLT_MAX, 1.f, 100.f },                // DZ_SHAPE_RECT_WIDTH_MAX
    { DZ_TIMELINE_LIMIT_MAX, 0.f, DZ_FLT_MAX, 0.f, 100.f },                // DZ_SHAPE_RECT_HEIGHT_MIN
    { DZ_TIMELINE_LIMIT_MAX, 0.f, DZ_FLT_MAX, 1.f, 100.f },                // DZ_SHAPE_RECT_HEIGHT_MAX
    { DZ_TIMELINE_LIMIT_MAX, 0.f, DZ_FLT_MAX, 0.f, 100.f },                // DZ_SHAPE_SPHERE_RADIUS_MIN
    { DZ_TIMELINE_LIMIT_MAX, 0.f, DZ_FLT_MAX, 1.f, 100.f },                // DZ_SHAPE_SPHERE_RADIUS_MAX
    { DZ_TIMELINE_LIMIT_MAX, 0.f, DZ_FLT_MAX, 1.f, 100.f },                // DZ_SHAPE_BOX_WIDTH
    { DZ_TIMELINE_LIMIT_MAX, 0.f, DZ_FLT_MAX, 1.f, 100.f },                // DZ_SHAPE_BOX_HEIGHT
    { DZ_TIMELINE_LIMIT_MAX, 0.f, DZ_FLT_MAX, 1.f, 100.f },                // DZ_SHAPE_BOX_DEPTH
    { DZ_TIMELINE_LIMIT_MAX, 0.f, DZ_FLT_MAX, 1.f, 100.f },                // DZ_SHAPE_CONE_RADIUS
    { DZ_TIMELINE_LIMIT_MAX, 0.f, DZ_FLT_MAX, 1.f, 100.f },                // DZ_SHAPE_CONE_HEIGHT
    { DZ_TIMELINE_LIMIT_MAX, 0.f, DZ_FLT_MAX, 1.f, 100.f },                // DZ_SHAPE_CYLINDER_RADIUS
    { DZ_TIMELINE_LIMIT_MAX, 0.f, DZ_FLT_MAX, 1.f, 100.f },                // DZ_SHAPE_CYLINDER_HEIGHT
};
//////////////////////////////////////////////////////////////////////////
void dz_shape_timeline_get_limit( dz_shape_timeline_type_e _timeline, dz_timeline_limit_status_e * const _status, dz_float_t * const _min, dz_float_t * const _max, dz_float_t * const _default, dz_float_t * const _factor )
{
    const dz_timeline_limits_t * limit = shape_timeline_limits + _timeline;

    *_status = limit->status;
    *_min = limit->min_value;
    *_max = limit->max_value;
    *_default = limit->default_value;
    *_factor = limit->factor_value;
}
//////////////////////////////////////////////////////////////////////////
void dz_shape_set_polygon( dz_shape_t * const _shape, const dz_float_t * _triangles, dz_uint32_t _count )
{
    _shape->triangles = _triangles;
    _shape->triangle_count = _count;

}
//////////////////////////////////////////////////////////////////////////
void dz_shape_get_polygon( const dz_shape_t * _shape, const dz_float_t ** _triangles, dz_uint32_t * _count )
{
    *_triangles = _shape->triangles;
    *_count = _shape->triangle_count;
}
//////////////////////////////////////////////////////////////////////////
void dz_shape_set_mask( dz_shape_t * const _shape, const void * _buffer, dz_uint32_t _bites, dz_uint32_t _pitch, dz_uint32_t _width, dz_uint32_t _height )
{
    _shape->mask_buffer = _buffer;
    _shape->mask_bites = _bites;
    _shape->mask_pitch = _pitch;
    _shape->mask_width = _width;
    _shape->mask_height = _height;

}
//////////////////////////////////////////////////////////////////////////
void dz_shape_get_mask( const dz_shape_t * _shape, const void ** _buffer, dz_uint32_t * const _bites, dz_uint32_t * const _pitch, dz_uint32_t * const _width, dz_uint32_t * const _height )
{
    *_buffer = _shape->mask_buffer;
    *_bites = _shape->mask_bites;
    *_pitch = _shape->mask_pitch;
    *_width = _shape->mask_width;
    *_height = _shape->mask_height;
}
//////////////////////////////////////////////////////////////////////////
void dz_shape_set_mask_scale( dz_shape_t * const _shape, dz_float_t _scale )
{
    _shape->mask_scale = _scale;
}
//////////////////////////////////////////////////////////////////////////
dz_float_t dz_shape_get_mask_scale( const dz_shape_t * _shape )
{
    return _shape->mask_scale;
}
//////////////////////////////////////////////////////////////////////////
void dz_shape_set_mask_threshold( dz_shape_t * const _shape, dz_uint32_t _threshold )
{
    _shape->mask_threshold = _threshold;
}
//////////////////////////////////////////////////////////////////////////
dz_uint32_t dz_shape_get_mask_threshold( const dz_shape_t * _shape )
{
    return _shape->mask_threshold;
}
