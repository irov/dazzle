#ifndef DZ_PARTICLE_H_
#define DZ_PARTICLE_H_

#include "dazzle/dazzle.h"

typedef struct dz_particle_t
{
    dz_float_t rands[__DZ_AFFECTOR_TIMELINE_MAX__];

    dz_float_t life;
    dz_float_t time;

    dz_float_t x;
    dz_float_t y;

    dz_float_t angle;
    dz_float_t spin;

    dz_float_t sx;
    dz_float_t sy;

    dz_float_t move_accelerate_aux;
    dz_float_t rotate_accelerate_aux;
    dz_float_t spin_accelerate_aux;

    dz_float_t scale;
    dz_float_t aspect;

    dz_float_t color_r;
    dz_float_t color_g;
    dz_float_t color_b;
    dz_float_t color_a;

    dz_float_t born_color_r;
    dz_float_t born_color_g;
    dz_float_t born_color_b;
    dz_float_t born_color_a;

    const dz_texture_t * texture;
} dz_particle_t;

#endif