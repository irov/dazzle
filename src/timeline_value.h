#ifndef DZ_TIMELINE_VALUE_H_
#define DZ_TIMELINE_VALUE_H_

#include "dazzle/dazzle.h"

#include "timeline_key.h"

typedef struct dz_timeline_value_t
{
    const dz_timeline_key_t * begin;
    const dz_timeline_key_t * current;

    float time;
} dz_timeline_value_t;

#endif