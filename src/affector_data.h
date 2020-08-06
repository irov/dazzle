#ifndef DZ_AFFECTOR_DATA_H_
#define DZ_AFFECTOR_DATA_H_

#include "dazzle/dazzle.h"

#include "timeline_key.h"

typedef struct dz_affector_data_t
{
    const dz_timeline_key_t * timelines[__DZ_AFFECTOR_DATA_TIMELINE_MAX__];

    dz_userdata_t ud;
} dz_affector_data_t;

#endif