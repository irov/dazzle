#ifndef DZ_CONFIG_H_
#define DZ_CONFIG_H_

#include <stdint.h>
#include <stddef.h>

typedef enum dz_result_e
{
    DZ_SUCCESSFUL,
    DZ_FAILURE
} dz_result_e;

typedef enum dz_boolean_e
{
    DZ_FALSE,
    DZ_TRUE
} dz_boolean_e;

typedef dz_result_e dz_result_t;
typedef uint32_t dz_bool_t;
typedef int8_t dz_int8_t;
typedef uint8_t dz_uint8_t;
typedef int16_t dz_int16_t;
typedef uint16_t dz_uint16_t;
typedef int32_t dz_int32_t;
typedef uint32_t dz_uint32_t;
typedef size_t dz_size_t;
typedef void * dz_userdata_t;

#define DZ_PI_HALF (1.5707963267948966192313216916398f)
#define DZ_PI (3.1415926535897932384626433832795f)
#define DZ_PI2 (6.283185307179586476925286766559f)
#define DZ_PI2N (-6.283185307179586476925286766559f)

#define DZ_FLT_MIN (-3.402823466e+38F)
#define DZ_FLT_MAX (3.402823466e+38F)

#ifndef DZ_PARTICLE_SIZE
#define DZ_PARTICLE_SIZE (16.f)
#endif

#ifndef DZ_RAND_BASE_SEED
#define DZ_RAND_BASE_SEED 1103515245U
#endif

#ifndef DZ_RAND_BASE_PROBE
#define DZ_RAND_BASE_PROBE 12345U
#endif

#ifndef DZ_RAND_FUNCTION
#define DZ_RAND_FUNCTION(X) (((X) * DZ_RAND_BASE_SEED) + DZ_RAND_BASE_PROBE)
#endif

#ifndef NDEBUG
#define DZ_DEBUG
#endif

#define DZ_TODO

#ifndef DZ_UNUSED
#define DZ_UNUSED(X) ((void)(X))
#endif

#define DZ_MAX(X, Y) ((X) > (Y) ? (X) : (Y))
#define DZ_MIN(X, Y) ((X) < (Y) ? (X) : (Y))

#define DZ_NULLPTR (0)

#endif