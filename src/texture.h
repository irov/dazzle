#ifndef DZ_TEXTURE_H_
#define DZ_TEXTURE_H_

#include "dazzle/dazzle.h"

typedef struct dz_texture_t
{
    float u[4];
    float v[4];

    float width;
    float height;

    float trim_offset_x;
    float trim_offset_y;

    float trim_width;
    float trim_height;

    float random_weight;
    float sequence_delay;

    dz_userdata_t ud;
} dz_texture_t;

#endif