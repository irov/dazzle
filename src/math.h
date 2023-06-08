#ifndef DZ_MATH_H_
#define DZ_MATH_H_

#include "dazzle/config.h"

#define DZ_SQRTF(k, v) ((*(k)->providers.f_sqrtf)((v), (k)->ud))
#define DZ_COSF(k, v) ((*(k)->providers.f_cosf)((v), (k)->ud))
#define DZ_SINF(k, v) ((*(k)->providers.f_sinf)((v), (k)->ud))

#endif