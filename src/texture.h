#ifndef DZ_TEXTURE_H_
#define DZ_TEXTURE_H_

#include "dazzle/dazzle.h"

typedef struct dz_texture_t
{
    float u[4];
    float v[4];

    dz_userdata_t ud;
} dz_texture_t;

#endif