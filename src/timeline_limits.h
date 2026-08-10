#ifndef DZ_TIMELINE_LIMITS_H_
#define DZ_TIMELINE_LIMITS_H_

#include "dazzle/dazzle.h"

typedef struct dz_timeline_limits_t
{
    dz_timeline_limit_status_e status;

    dz_float_t min_value;
    dz_float_t max_value;
    dz_float_t default_value;
    dz_float_t factor_value;
} dz_timeline_limits_t;

#endif
