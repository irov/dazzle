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

    dz_uint32_t seed;

    dz_float_t life;

    dz_userdata_t ud;
} dz_effect_t;

#endif