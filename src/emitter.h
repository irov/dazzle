#ifndef DZ_EMITTER_H_
#define DZ_EMITTER_H_

#include "dazzle/dazzle.h"

#include "timeline_key.h"

typedef struct dz_emitter_t
{
    float life;

    const dz_timeline_key_t * timelines[__DZ_EMITTER_TIMELINE_MAX__];

    dz_userdata_t ud;
} dz_emitter_t;

#endif