#ifndef DZ_EMITTER_H_
#define DZ_EMITTER_H_

#include "dazzle/dazzle.h"

#include "shape_data.h"
#include "emitter_data.h"
#include "affector_data.h"
#include "particle.h"

typedef struct dz_emitter_t
{
    const dz_material_t * material;
    const dz_shape_data_t * shape_data;
    const dz_emitter_data_t * emitter_data;
    const dz_affector_data_t * affector_data;

    uint32_t init_seed;
    uint32_t seed;

    dz_particle_t * partices;
    dz_size_t partices_count;
    dz_size_t partices_capacity;

    float life;

    float time;
    float emitter_time;

    dz_userdata_t ud;
} dz_emitter_t;

#endif