#ifndef DZ_LIST_H_
#define DZ_LIST_H_

#include "dazzle/config.h"

#define DZ_LIST_FOREACH(t, list, e) for( t * e = list; e != DZ_NULLPTR; e = e->next )
#define DZ_LIST_PUSHBACK(list, e) {if( list == DZ_NULLPTR ){list = e; list->prev = e;}else{e->prev = list->prev; e->next = DZ_NULLPTR; list->prev->next = e; list->prev = e;}}
#define DZ_LIST_BACK(list) (list->prev)
#define DZ_LIST_DESTROY(c, t, list) {for( t * __l = list; __l != DZ_NULLPTR; ){ t * __r = __l; __l = __l->next; GP_FREE( c, __r ); } list = DZ_NULLPTR;}

#endif