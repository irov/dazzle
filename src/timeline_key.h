#ifndef DZ_TIMELINE_KEY_H_
#define DZ_TIMELINE_KEY_H_

#include "dazzle/dazzle.h"

#include "timeline_interpolate.h"

typedef struct dz_timeline_key_t
{
    float p;

    dz_timeline_key_type_e type;

    const dz_timeline_interpolate_t * interpolate;

    dz_userdata_t ud;
} dz_timeline_key_t;

typedef struct dz_timeline_key_const_t
{
    dz_timeline_key_t base;

    float value;
} dz_timeline_key_const_t;

typedef struct dz_timeline_key_randomize_t
{
    dz_timeline_key_t base;

    float value_min;
    float value_max;
} dz_timeline_key_randomize_t;

#endif