#ifndef DZ_DAZZLE_H_
#define DZ_DAZZLE_H_

#include "dazzle/config.h"

typedef struct dz_kernel_t dz_kernel_t;

typedef void * (*dz_malloc_t)(dz_size_t _size, dz_userdata_t _ud);
typedef void * (*dz_realloc_t)(void * _ptr, dz_size_t _size, dz_userdata_t _ud);
typedef void (*dz_free_t)(void * _ptr, dz_userdata_t _ud);

dz_result_t dz_kernel_create( dz_kernel_t ** _kernel, dz_malloc_t _malloc, dz_realloc_t _realloc, dz_free_t _free, dz_userdata_t _ud );
void dz_kernel_destroy( dz_kernel_t * _kernel );


#endif
