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

    float const_value;

    float randomize_min_value;
    float randomize_max_value;
} dz_timeline_key_t;

#endif