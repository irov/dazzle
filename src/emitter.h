#ifndef DZ_EMITTER_H_
#define DZ_EMITTER_H_

#include "dazzle/dazzle.h"

#include "shape_data.h"
#include "emitter_data.h"
#include "affector_data.h"
#include "particle.h"
#include "timeline_value.h"

typedef struct dz_emitter_t
{
    const dz_shape_data_t * shape_data;
    const dz_emitter_data_t * emitter_data;
    const dz_affector_data_t * affector_data;

    uint32_t init_seed;
    uint32_t seed;

    dz_particle_t * partices;
    dz_size_t partices_count;
    dz_size_t partices_capacity;

    float time;
    float emitter_time;

    dz_timeline_value_t * shape_values[__DZ_EMITTER_DATA_TIMELINE_MAX__];
    dz_timeline_value_t * emitter_values[__DZ_EMITTER_DATA_TIMELINE_MAX__];
    dz_timeline_value_t * affector_values[__DZ_AFFECTOR_DATA_TIMELINE_MAX__];
} dz_emitter_t;

#endif