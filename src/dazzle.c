#include "dazzle/dazzle.h"

#include "malloc.h"

typedef struct dz_kernel_t
{
    dz_malloc_t f_malloc;
    dz_realloc_t f_realloc;
    dz_free_t f_free;

    dz_userdata_t ud;
} dz_kernel_t;
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_kernel_create( dz_kernel_t ** _kernel, dz_malloc_t _malloc, dz_realloc_t _realloc, dz_free_t _free, dz_userdata_t _ud )
{
    dz_kernel_t * k = (dz_kernel_t *)(*_malloc)(sizeof( dz_kernel_t ), _ud);

    k->f_malloc = _malloc;
    k->f_realloc = _realloc;
    k->f_free = _free;

    k->ud = _ud;

    *_kernel = k;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_kernel_destroy( dz_kernel_t * _kernel )
{
    DZ_FREE( _kernel, _kernel );
}