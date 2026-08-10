#ifndef DZ_SHAPE_H_
#define DZ_SHAPE_H_

#include "dazzle/dazzle.h"

#include "timeline_key.h"

typedef struct dz_shape_t
{
    dz_shape_type_e type;

    const dz_timeline_key_t * timelines[__DZ_SHAPE_TIMELINE_MAX__];

    const dz_float_t * triangles;
    dz_uint32_t triangle_count;
    dz_bool_t owns_triangles;

    const void * mask_buffer;
    dz_uint32_t mask_bites;
    dz_uint32_t mask_pitch;
    dz_uint32_t mask_width;
    dz_uint32_t mask_height;
    dz_uint32_t mask_threshold;
    dz_float_t mask_scale;
    dz_bool_t owns_mask;

    dz_transform_t transform;
    dz_vec3_t dimensions;
    dz_uint32_t mesh_id;

    dz_userdata_t ud;
} dz_shape_t;

#endif
