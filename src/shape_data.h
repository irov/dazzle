#ifndef DZ_SHAPE_DATA_H_
#define DZ_SHAPE_DATA_H_

#include "dazzle/dazzle.h"

#include "timeline_key.h"

typedef struct dz_shape_data_t
{
    dz_shape_data_type_e type;

    const dz_timeline_key_t * timelines[__DZ_SHAPE_DATA_TIMELINE_MAX__];

    dz_userdata_t ud;
} dz_shape_data_t;

#endif