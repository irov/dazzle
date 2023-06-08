#ifndef DZ_INSTANCE_H_
#define DZ_INSTANCE_H_

#include "dazzle/dazzle.h"

#include "effect.h"
#include "particle.h"

typedef struct dz_instance_t
{
    const dz_effect_t * effect;

    dz_uint32_t init_seed;
    dz_uint32_t seed;

    dz_particle_t * partices;
    dz_uint16_t partices_count;
    dz_uint16_t partices_capacity;
    dz_uint16_t particle_limit;

    dz_bool_t loop;
    dz_bool_t emit_pause;

    dz_float_t time;
    dz_float_t emitter_time;

    dz_float_t x;
    dz_float_t y;

    dz_float_t angle;

    dz_float_t r;
    dz_float_t g;
    dz_float_t b;
    dz_float_t a;

    dz_userdata_t ud;
} dz_instance_t;

#endif