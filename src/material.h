#ifndef DZ_MATERIAL_H_
#define DZ_MATERIAL_H_

#include "dazzle/dazzle.h"

typedef struct dz_material_t
{
    dz_blend_type_e blend_type;

    float r;
    float g;
    float b;
    float a;

    dz_material_mode_e mode;

    const dz_atlas_t * atlas;

    dz_userdata_t ud;
} dz_material_t;

#endif