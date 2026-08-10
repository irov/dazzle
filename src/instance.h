#ifndef DZ_INSTANCE_H_
#define DZ_INSTANCE_H_

#include "dazzle/dazzle.h"

#include "effect.h"
#include "particle.h"

typedef struct dz_effect_emitter_instance_t
{
    dz_uint32_t layer_index;
    dz_uint32_t seed;

    dz_float_t time;
    dz_float_t emitter_time;

    dz_float_t x;
    dz_float_t y;
    dz_float_t z;

    dz_float_t angle;
    dz_transform_t transform;

    dz_float_t sx;
    dz_float_t sy;

    dz_bool_t active;
} dz_effect_emitter_instance_t;

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
    dz_bool_t started;
    dz_bool_t stopped;
    dz_bool_t paused;

    dz_float_t time;
    dz_float_t fixed_step;
    double fixed_step_accumulator;

    dz_effect_emitter_instance_t emitter_instances[DZ_EFFECT_EMITTER_INSTANCE_MAX];
    dz_uint32_t emitter_instance_count;

    dz_float_t x;
    dz_float_t y;
    dz_float_t z;

    dz_float_t angle;
    dz_transform_t transform;

    dz_uint32_t birth_order;
    dz_transform_t physics_transforms[DZ_EFFECT_PHYSICS_OBJECT_MAX];

    dz_float_t r;
    dz_float_t g;
    dz_float_t b;
    dz_float_t a;

    dz_userdata_t ud;
} dz_instance_t;

#endif
