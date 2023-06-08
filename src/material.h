#ifndef DZ_MATERIAL_H_
#define DZ_MATERIAL_H_

#include "dazzle/dazzle.h"

typedef struct dz_material_t
{
    dz_blend_type_e blend_type;

    dz_float_t r;
    dz_float_t g;
    dz_float_t b;
    dz_float_t a;

    dz_material_mode_e mode;

    const dz_atlas_t * atlas;

    dz_userdata_t ud;
} dz_material_t;

#endif