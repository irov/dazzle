#ifndef DZ_MALLOC_H_
#define DZ_MALLOC_H_

#define DZ_NEW(k, t) ((t*)(*k->f_malloc)(sizeof(t), k->ud))
#define DZ_REALLOC(k, p, t, s) ((t*)(*k->f_realloc)((p), sizeof(t) * s, k->ud))
#define DZ_FREE(k, p) (*k->f_free)((p), k->ud)

#endif