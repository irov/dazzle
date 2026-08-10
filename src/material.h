#ifndef DZ_MATERIAL_H_
#define DZ_MATERIAL_H_

#include "dazzle/dazzle.h"

typedef struct dz_material_texture_t
{
    const dz_texture_t * texture;
    dz_float_t random_weight;
} dz_material_texture_t;

typedef struct dz_material_t
{
    dz_blend_type_e blend_type;

    dz_float_t r;
    dz_float_t g;
    dz_float_t b;
    dz_float_t a;

    dz_material_mode_e mode;

    const dz_atlas_t * atlas;

    dz_material_texture_t textures[64];
    dz_uint32_t textures_count;
    dz_float_t textures_time;
    dz_float_t textures_random_weight;

    dz_uint32_t texture_index;
    dz_uint32_t texture_count;

    dz_material_pass_desc_t passes[DZ_MATERIAL_PASS_MAX];
    dz_uint32_t pass_count;

    dz_userdata_t ud;
} dz_material_t;

#endif
