#ifndef DZ_TIMELINE_INTERPOLATE_H_
#define DZ_TIMELINE_INTERPOLATE_H_

#include "dazzle/dazzle.h"

#include "timeline_key.h"

typedef struct dz_timeline_interpolate_t
{
    dz_timeline_interpolate_type_e type;

    float p0;
    float p1;

    const struct dz_timeline_key_t * key;

    dz_userdata_t ud;
} dz_timeline_interpolate_t;

#endif