#ifndef DZ_ATLAS_H_
#define DZ_ATLAS_H_

#include "dazzle/dazzle.h"

typedef struct dz_atlas_t
{
    const dz_texture_t * textures[64];
    uint32_t texture_count;

    float textures_time;

    dz_userdata_t surface;

    dz_userdata_t ud;
} dz_atlas_t;

#endif