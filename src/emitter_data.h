#ifndef DZ_EMITTER_DATA_H_
#define DZ_EMITTER_DATA_H_

#include "dazzle/dazzle.h"

#include "timeline_key.h"

typedef struct dz_emitter_data_t
{
    float life;    

    const dz_timeline_key_t * timelines[__DZ_EMITTER_DATA_TIMELINE_MAX__];

    dz_userdata_t ud;
} dz_emitter_data_t;

#endif