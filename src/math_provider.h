#ifndef DZ_MATH_PROVIDER_H_
#define DZ_MATH_PROVIDER_H_

#define DZ_SQRTF( k, v ) ( ( *( k )->providers.f_sqrtf )( ( v ), ( k )->ud ) )
#define DZ_COSF( k, v ) ( ( *( k )->providers.f_cosf )( ( v ), ( k )->ud ) )
#define DZ_SINF( k, v ) ( ( *( k )->providers.f_sinf )( ( v ), ( k )->ud ) )
#define DZ_ATAN2F( k, y, x ) ( ( *( k )->providers.f_atan2f )( ( y ), ( x ), ( k )->ud ) )
#define DZ_ASINF( k, v ) ( ( *( k )->providers.f_asinf )( ( v ), ( k )->ud ) )
#define DZ_TANF( k, v ) ( ( *( k )->providers.f_tanf )( ( v ), ( k )->ud ) )

#endif
