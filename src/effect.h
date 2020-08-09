#ifndef DZ_EFFECT_H_
#define DZ_EFFECT_H_

#include "dazzle/dazzle.h"

#include "shape.h"
#include "emitter.h"
#include "affector.h"
#include "particle.h"

typedef struct dz_effect_t
{
    const dz_material_t * material;
    const dz_shape_t * shape;
    const dz_emitter_t * emitter;
    const dz_affector_t * affector;

    uint32_t init_seed;
    uint32_t seed;

    dz_particle_t * partices;
    uint32_t partices_count;
    uint32_t partices_capacity;
    uint32_t particle_limit;

    dz_bool_t loop;
    dz_bool_t spawn_pause;

    float life;

    float time;
    float emitter_time;

    float x;
    float y;

    float angle;

    dz_userdata_t ud;
} dz_effect_t;

#endif