#ifndef DZ_EFFECT_H_
#define DZ_EFFECT_H_

#include "dazzle/dazzle.h"

#include "shape.h"
#include "emitter.h"
#include "affector.h"
#include "particle.h"

typedef struct dz_effect_t
{
    const dz_atlas_t * atlas;

    dz_effect_layer_desc_t layers[DZ_EFFECT_LAYER_MAX];
    dz_uint32_t layer_count;

    dz_effect_trigger_desc_t triggers[DZ_EFFECT_TRIGGER_MAX];
    dz_uint32_t trigger_count;

    dz_uint32_t seed;

    dz_float_t life;

    dz_userdata_t ud;
} dz_effect_t;

#endif