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

    const dz_texture_t * texture;

    dz_userdata_t ud;
} dz_material_t;

#endif