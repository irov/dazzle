#ifndef DZ_TIMELINE_KEY_H_
#define DZ_TIMELINE_KEY_H_

#include "dazzle/dazzle.h"

#include "timeline_interpolate.h"

typedef struct dz_timeline_key_t
{
    float p;
    float d_inv;

    dz_timeline_key_type_e type;

    float const_value;

    float randomize_min_value;
    float randomize_max_value;

    const dz_timeline_interpolate_t * interpolate;

    dz_userdata_t ud;
} dz_timeline_key_t;

#endif