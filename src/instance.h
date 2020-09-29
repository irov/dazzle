#ifndef DZ_INSTANCE_H_
#define DZ_INSTANCE_H_

#include "dazzle/dazzle.h"

#include "effect.h"
#include "particle.h"

typedef struct dz_instance_t
{
    const dz_effect_t * effect;

    uint32_t init_seed;
    uint32_t seed;

    dz_particle_t * partices;
    uint16_t partices_count;
    uint16_t partices_capacity;
    uint16_t particle_limit;

    dz_bool_t loop;
    dz_bool_t emit_pause;

    float time;
    float emitter_time;

    float x;
    float y;

    float angle;

    dz_userdata_t ud;
} dz_instance_t;

#endif