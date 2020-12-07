#ifndef DZ_PARTICLE_H_
#define DZ_PARTICLE_H_

#include "dazzle/dazzle.h"

typedef struct dz_particle_t
{
    float rands[__DZ_AFFECTOR_TIMELINE_MAX__];

    float life;
    float time;

    float x;
    float y;

    float angle;
    float spin;

    float sx;
    float sy;

    float move_accelerate_aux;
    float rotate_accelerate_aux;
    float spin_accelerate_aux;

    float size;

    float color_r;
    float color_g;
    float color_b;
    float color_a;

    float born_color_r;
    float born_color_g;
    float born_color_b;
    float born_color_a;

    const dz_texture_t * texture;
} dz_particle_t;

#endif