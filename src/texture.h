#ifndef DZ_TEXTURE_H_
#define DZ_TEXTURE_H_

#include "dazzle/dazzle.h"

typedef struct dz_texture_t
{
    dz_float_t u[4];
    dz_float_t v[4];

    dz_float_t width;
    dz_float_t height;

    dz_float_t trim_offset_x;
    dz_float_t trim_offset_y;

    dz_float_t trim_width;
    dz_float_t trim_height;

    dz_float_t random_weight;
    dz_float_t sequence_delay;

    dz_userdata_t ud;
} dz_texture_t;

#endif