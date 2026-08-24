#include "dazzle/dazzle.h"

#include "alloc.h"
#include "math3d.h"
#include "memory.h"
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

    dz_memory_zero( &shape->mask_source, sizeof( shape->mask_source ) );
    shape->mask_bits = DZ_NULLPTR;
    shape->mask_bits_pitch = 0U;
    shape->mask_uses_bits = DZ_FALSE;
    shape->mask_scale = 1.f;
    shape->owns_mask_source = DZ_FALSE;
    shape->mask_boundary_points = DZ_NULLPTR;
    shape->mask_boundary_point_count = 0U;
    shape->mask_boundary_offsets = DZ_NULLPTR;
    shape->mask_boundary_strata_count = 0U;
    dz_emitter_texture_desc_default( &shape->emitter_texture_desc );
    shape->has_emitter_texture_desc = DZ_FALSE;

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

    if( _shape->owns_mask_source == DZ_TRUE )
    {
        DZ_FREE( _service, _shape->mask_source.buffer );
    }

    if( _shape->mask_bits != DZ_NULLPTR )
    {
        DZ_FREE( _service, _shape->mask_bits );
    }

    if( _shape->mask_boundary_points != DZ_NULLPTR )
    {
        DZ_FREE( _service, _shape->mask_boundary_points );
    }

    if( _shape->mask_boundary_offsets != DZ_NULLPTR )
    {
        DZ_FREE( _service, _shape->mask_boundary_offsets );
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
void dz_shape_set_transform( const dz_service_t * _service, dz_shape_t * _shape, const dz_transform_t * _transform )
{
    _shape->transform = *_transform;
    _shape->transform.rotation = dz_math_quat_normalize( _service, _transform->rotation );
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
void dz_emitter_texture_desc_default( dz_emitter_texture_desc_t * const _desc )
{
    _desc->alpha_threshold = 0U;
    _desc->rgb_threshold = 255U;
    _desc->strata = 192U;
    _desc->sample_scale = 1.f;
    _desc->boundary = DZ_FALSE;
    _desc->compile = DZ_TRUE;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_shape_set_emitter_texture_desc( dz_shape_t * const _shape, const dz_emitter_texture_desc_t * _desc )
{
#if defined( DZ_DEBUG )
    if( _shape == DZ_NULLPTR || _desc == DZ_NULLPTR || _shape->type != DZ_SHAPE_MASK || _desc->alpha_threshold > 255U || _desc->rgb_threshold > 255U ||
        _desc->sample_scale <= 0.f || (_desc->boundary == DZ_TRUE && _desc->strata == 0U) )
    {
        return DZ_FAILURE_INVALID_ARGUMENT;
    }
#endif

    _shape->emitter_texture_desc = *_desc;
    _shape->has_emitter_texture_desc = DZ_TRUE;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_bool_t dz_shape_get_emitter_texture_desc( const dz_shape_t * _shape, dz_emitter_texture_desc_t * const _desc )
{
#if defined( DZ_DEBUG )
    if( _shape == DZ_NULLPTR )
    {
        return DZ_FALSE;
    }
#endif

    if( _shape->has_emitter_texture_desc == DZ_FALSE )
    {
        return DZ_FALSE;
    }

    if( _desc != DZ_NULLPTR )
    {
        *_desc = _shape->emitter_texture_desc;
    }

    return DZ_TRUE;
}
//////////////////////////////////////////////////////////////////////////
void dz_shape_clear_emitter_texture_desc( dz_shape_t * const _shape )
{
#if defined( DZ_DEBUG )
    if( _shape == DZ_NULLPTR )
    {
        return;
    }
#endif

    dz_emitter_texture_desc_default( &_shape->emitter_texture_desc );
    _shape->has_emitter_texture_desc = DZ_FALSE;
}
//////////////////////////////////////////////////////////////////////////
#if defined( DZ_DEBUG )
static dz_result_t __shape_validate_mask_source( const dz_shape_mask_source_t * _source )
{
    if( _source == DZ_NULLPTR || _source->buffer == DZ_NULLPTR || _source->width == 0U || _source->height == 0U || _source->channel_count == 0U ||
        _source->alpha_channel >= _source->channel_count || _source->pitch / _source->channel_count < _source->width || _source->alpha_threshold > 255U )
    {
        return DZ_FAILURE_INVALID_ARGUMENT;
    }

    return DZ_SUCCESSFUL;
}
#endif
//////////////////////////////////////////////////////////////////////////
void dz_shape_clear_mask( const dz_service_t * _service, dz_shape_t * const _shape )
{
    if( _shape->owns_mask_source == DZ_TRUE )
    {
        DZ_FREE( _service, _shape->mask_source.buffer );
    }

    if( _shape->mask_bits != DZ_NULLPTR )
    {
        DZ_FREE( _service, _shape->mask_bits );
    }

    dz_memory_zero( &_shape->mask_source, sizeof( _shape->mask_source ) );
    _shape->mask_bits = DZ_NULLPTR;
    _shape->mask_bits_pitch = 0U;
    _shape->mask_uses_bits = DZ_FALSE;
    _shape->owns_mask_source = DZ_FALSE;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_shape_set_mask_source( const dz_service_t * _service, dz_shape_t * const _shape, const dz_shape_mask_source_t * _source )
{
#if defined( DZ_DEBUG )
    if( _service == DZ_NULLPTR || _shape == DZ_NULLPTR )
    {
        return DZ_FAILURE_INVALID_ARGUMENT;
    }

    const dz_result_t validation = __shape_validate_mask_source( _source );

    if( validation != DZ_SUCCESSFUL )
    {
        return validation;
    }
#endif

    dz_shape_clear_mask( _service, _shape );

    _shape->mask_source = *_source;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_shape_get_mask_source( const dz_shape_t * _shape, dz_shape_mask_source_t * const _source )
{
    *_source = _shape->mask_source;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_shape_build_mask( const dz_service_t * _service, dz_shape_t * const _shape, const dz_shape_mask_source_t * _source )
{
#if defined( DZ_DEBUG )
    if( _service == DZ_NULLPTR || _shape == DZ_NULLPTR )
    {
        return DZ_FAILURE_INVALID_ARGUMENT;
    }

    const dz_result_t validation = __shape_validate_mask_source( _source );

    if( validation != DZ_SUCCESSFUL )
    {
        return validation;
    }
#endif

    const dz_uint32_t bits_pitch = (_source->width + 7U) / 8U;
    const dz_size_t bits_size = (dz_size_t)bits_pitch * _source->height;
    dz_uint8_t * bits = DZ_REALLOCN( _service, DZ_NULLPTR, dz_uint8_t, bits_size );

    if( bits == DZ_NULLPTR )
    {
        return DZ_FAILURE;
    }

    dz_memory_zero( bits, bits_size );

    for( dz_uint32_t y = 0U; y != _source->height; ++y )
    {
        const dz_uint8_t * source_row = (const dz_uint8_t *)_source->buffer + (dz_size_t)y * _source->pitch;
        dz_uint8_t * bits_row = bits + (dz_size_t)y * bits_pitch;

        for( dz_uint32_t x = 0U; x != _source->width; ++x )
        {
            const dz_uint8_t alpha = source_row[(dz_size_t)x * _source->channel_count + _source->alpha_channel];

            if( (dz_uint32_t)alpha <= _source->alpha_threshold )
            {
                continue;
            }

            bits_row[x >> 3U] |= (dz_uint8_t)(1U << (x & 7U));
        }
    }

    dz_shape_clear_mask( _service, _shape );

    _shape->mask_source = *_source;
    _shape->mask_source.buffer = DZ_NULLPTR;
    _shape->mask_bits = bits;
    _shape->mask_bits_pitch = bits_pitch;
    _shape->mask_uses_bits = DZ_TRUE;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_shape_get_mask_bits( const dz_shape_t * _shape, const void ** _buffer, dz_uint32_t * const _pitch )
{
    *_buffer = _shape->mask_uses_bits == DZ_TRUE ? _shape->mask_bits : DZ_NULLPTR;
    *_pitch = _shape->mask_uses_bits == DZ_TRUE ? _shape->mask_bits_pitch : 0U;
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
static dz_bool_t __shape_rgba_is_boundary( const dz_uint8_t * _rgba, dz_uint32_t _pitch, dz_uint32_t _width, dz_uint32_t _height, dz_uint32_t _x, dz_uint32_t _y, dz_uint32_t _alpha_threshold, dz_uint32_t _rgb_threshold )
{
    const dz_uint8_t * pixel = _rgba + (dz_size_t)_y * _pitch + (dz_size_t)_x * 4U;

    if( (dz_uint32_t)pixel[3] <= _alpha_threshold )
    {
        return DZ_FALSE;
    }

    static const dz_int32_t neighbor_x[4] = {-1, 1, 0, 0};
    static const dz_int32_t neighbor_y[4] = {0, 0, -1, 1};

    for( dz_uint32_t index = 0U; index != 4U; ++index )
    {
        const dz_int32_t nx = (dz_int32_t)_x + neighbor_x[index];
        const dz_int32_t ny = (dz_int32_t)_y + neighbor_y[index];

        if( nx < 0 || ny < 0 || nx >= (dz_int32_t)_width || ny >= (dz_int32_t)_height )
        {
            return DZ_TRUE;
        }

        const dz_uint8_t * neighbor = _rgba + (dz_size_t)ny * _pitch + (dz_size_t)nx * 4U;

        if( (dz_uint32_t)neighbor[3] <= _alpha_threshold )
        {
            return DZ_TRUE;
        }

        const dz_uint32_t difference_r = pixel[0] > neighbor[0] ? (dz_uint32_t)(pixel[0] - neighbor[0]) : (dz_uint32_t)(neighbor[0] - pixel[0]);
        const dz_uint32_t difference_g = pixel[1] > neighbor[1] ? (dz_uint32_t)(pixel[1] - neighbor[1]) : (dz_uint32_t)(neighbor[1] - pixel[1]);
        const dz_uint32_t difference_b = pixel[2] > neighbor[2] ? (dz_uint32_t)(pixel[2] - neighbor[2]) : (dz_uint32_t)(neighbor[2] - pixel[2]);
        const dz_uint32_t difference = DZ_MAX( difference_r, DZ_MAX( difference_g, difference_b ) );

        if( difference > _rgb_threshold )
        {
            return DZ_TRUE;
        }
    }

    return DZ_FALSE;
}
//////////////////////////////////////////////////////////////////////////
void dz_shape_clear_mask_boundary( const dz_service_t * _service, dz_shape_t * const _shape )
{
    if( _shape->mask_boundary_points != DZ_NULLPTR )
    {
        DZ_FREE( _service, _shape->mask_boundary_points );
        _shape->mask_boundary_points = DZ_NULLPTR;
    }

    if( _shape->mask_boundary_offsets != DZ_NULLPTR )
    {
        DZ_FREE( _service, _shape->mask_boundary_offsets );
        _shape->mask_boundary_offsets = DZ_NULLPTR;
    }

    _shape->mask_boundary_point_count = 0U;
    _shape->mask_boundary_strata_count = 0U;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_shape_build_rgba_boundary( const dz_service_t * _service, dz_shape_t * const _shape, const void * _rgba, dz_uint32_t _pitch, dz_uint32_t _width, dz_uint32_t _height, dz_uint32_t _alpha_threshold, dz_uint32_t _rgb_threshold, dz_uint32_t _strata )
{
#if defined( DZ_DEBUG )
    if( _service == DZ_NULLPTR || _shape == DZ_NULLPTR || _rgba == DZ_NULLPTR || _width == 0U || _height == 0U || _pitch / 4U < _width )
    {
        return DZ_FAILURE_INVALID_ARGUMENT;
    }
#endif

    dz_shape_clear_mask_boundary( _service, _shape );

    const dz_uint8_t * rgba = (const dz_uint8_t *)_rgba;
    dz_uint32_t boundary_point_count = 0U;

    for( dz_uint32_t y = 0U; y != _height; ++y )
    {
        for( dz_uint32_t x = 0U; x != _width; ++x )
        {
            if( __shape_rgba_is_boundary( rgba, _pitch, _width, _height, x, y, _alpha_threshold, _rgb_threshold ) == DZ_TRUE )
            {
                ++boundary_point_count;
            }
        }
    }

    if( boundary_point_count == 0U )
    {
        return DZ_FAILURE_INVALID_DATA;
    }

    const dz_uint32_t requested_strata = DZ_MAX( 1U, DZ_MIN( _strata, boundary_point_count ) );
    dz_uint32_t columns = 1U;
    const dz_uint64_t target = (dz_uint64_t)requested_strata * _width;

    while( columns < _width && (dz_uint64_t)(columns + 1U) * (columns + 1U) * _height <= target )
    {
        ++columns;
    }

    dz_uint32_t rows = (requested_strata + columns - 1U) / columns;
    rows = DZ_MAX( 1U, DZ_MIN( rows, _height ) );
    columns = DZ_MAX( 1U, DZ_MIN( columns, _width ) );

    dz_shape_mask_boundary_point_t * points = DZ_REALLOCN( _service, DZ_NULLPTR, dz_shape_mask_boundary_point_t, boundary_point_count );
    dz_uint32_t * offsets = DZ_REALLOCN( _service, DZ_NULLPTR, dz_uint32_t, columns * rows + 1U );

    if( points == DZ_NULLPTR || offsets == DZ_NULLPTR )
    {
        if( points != DZ_NULLPTR )
        {
            DZ_FREE( _service, points );
        }

        if( offsets != DZ_NULLPTR )
        {
            DZ_FREE( _service, offsets );
        }

        return DZ_FAILURE;
    }

    dz_uint32_t point_index = 0U;
    dz_uint32_t stratum_index = 0U;

    for( dz_uint32_t row = 0U; row != rows; ++row )
    {
        const dz_uint32_t y_begin = (dz_uint32_t)(((dz_uint64_t)row * _height) / rows);
        const dz_uint32_t y_end = (dz_uint32_t)(((dz_uint64_t)(row + 1U) * _height) / rows);

        for( dz_uint32_t column = 0U; column != columns; ++column )
        {
            const dz_uint32_t x_begin = (dz_uint32_t)(((dz_uint64_t)column * _width) / columns);
            const dz_uint32_t x_end = (dz_uint32_t)(((dz_uint64_t)(column + 1U) * _width) / columns);
            const dz_uint32_t stratum_begin = point_index;

            for( dz_uint32_t y = y_begin; y != y_end; ++y )
            {
                for( dz_uint32_t x = x_begin; x != x_end; ++x )
                {
                    if( __shape_rgba_is_boundary( rgba, _pitch, _width, _height, x, y, _alpha_threshold, _rgb_threshold ) == DZ_FALSE )
                    {
                        continue;
                    }

                    points[point_index].x = x;
                    points[point_index].y = y;
                    ++point_index;
                }
            }

            if( point_index != stratum_begin )
            {
                offsets[stratum_index++] = stratum_begin;
            }
        }
    }

    offsets[stratum_index] = point_index;
    _shape->mask_boundary_points = points;
    _shape->mask_boundary_point_count = point_index;
    _shape->mask_boundary_offsets = offsets;
    _shape->mask_boundary_strata_count = stratum_index;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_uint32_t dz_shape_get_mask_boundary_point_count( const dz_shape_t * _shape )
{
    return _shape->mask_boundary_point_count;
}
//////////////////////////////////////////////////////////////////////////
dz_uint32_t dz_shape_get_mask_boundary_strata_count( const dz_shape_t * _shape )
{
    return _shape->mask_boundary_strata_count;
}
