#ifndef DZ_ALLOC_H_
#define DZ_ALLOC_H_

#define DZ_NEW(k, t) ((t*)(*k->providers.f_malloc)(sizeof(t), k->ud))
#define DZ_NEWV(k, t, v) ((t*)(*k->providers.f_malloc)(sizeof(v), k->ud))
#define DZ_REALLOC(k, p, t, s) ((t*)(*k->providers.f_realloc)((p), sizeof(t) * s, k->ud))
#define DZ_FREE(k, p) (*k->providers.f_free)((p), k->ud)

#endif