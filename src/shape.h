#ifndef DZ_SHAPE_H_
#define DZ_SHAPE_H_

#include "dazzle/dazzle.h"

#include "timeline_key.h"

typedef struct dz_shape_mask_boundary_point_t
{
    dz_uint32_t x;
    dz_uint32_t y;
} dz_shape_mask_boundary_point_t;

typedef struct dz_shape_t
{
    dz_shape_type_e type;

    const dz_timeline_key_t * timelines[__DZ_SHAPE_TIMELINE_MAX__];

    const dz_float_t * triangles;
    dz_uint32_t triangle_count;
    dz_bool_t owns_triangles;

    dz_shape_mask_source_t mask_source;
    dz_uint8_t * mask_bits;
    dz_uint32_t mask_bits_pitch;
    dz_bool_t mask_uses_bits;
    dz_float_t mask_scale;
    dz_bool_t owns_mask_source;
    dz_shape_mask_boundary_point_t * mask_boundary_points;
    dz_uint32_t mask_boundary_point_count;
    dz_uint32_t * mask_boundary_offsets;
    dz_uint32_t mask_boundary_strata_count;

    dz_emitter_texture_desc_t emitter_texture_desc;
    dz_bool_t has_emitter_texture_desc;

    dz_transform_t transform;
    dz_vec3_t dimensions;
    dz_uint32_t mesh_id;

    dz_userdata_t ud;
} dz_shape_t;

#endif
