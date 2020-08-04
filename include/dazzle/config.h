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
typedef uint8_t dz_uint8_t;
typedef uint16_t dz_uint16_t;
typedef uint32_t dz_uint32_t;
typedef size_t dz_size_t;
typedef void * dz_userdata_t;

#define DZ_PI (3.1415926535897932384626433832795f)
#define DZ_PI2 (6.283185307179586476925286766559f)

#ifndef NDEBUG
#define DZ_DEBUG
#endif

#ifndef DZ_UNUSED
#define DZ_UNUSED(X) ((void)(X))
#endif

#define DZ_NULLPTR (0)

#endif