#ifndef DZ_MATH3D_H_
#define DZ_MATH3D_H_

#include "dazzle/dazzle.h"

dz_vec3_t dz_math_vec3( dz_float_t _x, dz_float_t _y, dz_float_t _z );
dz_vec3_t dz_math_add3( dz_vec3_t _a, dz_vec3_t _b );
dz_vec3_t dz_math_sub3( dz_vec3_t _a, dz_vec3_t _b );
dz_vec3_t dz_math_mul3( dz_vec3_t _v, dz_float_t _scalar );
dz_float_t dz_math_dot3( dz_vec3_t _a, dz_vec3_t _b );
dz_vec3_t dz_math_cross3( dz_vec3_t _a, dz_vec3_t _b );
dz_float_t dz_math_length3( dz_vec3_t _v );
dz_vec3_t dz_math_normalize3( dz_vec3_t _v, dz_vec3_t _fallback );
dz_quat_t dz_math_quat_identity( void );
dz_quat_t dz_math_quat_normalize( dz_quat_t _q );
dz_vec3_t dz_math_quat_rotate3( dz_quat_t _q, dz_vec3_t _v );
dz_vec3_t dz_math_transform_point( const dz_transform_t * _transform, dz_vec3_t _point );
dz_bool_t dz_math_is_finite3( dz_vec3_t _v );

#endif
