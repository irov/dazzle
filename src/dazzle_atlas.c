#include "dazzle/dazzle.h"

#include "alloc.h"
#include "atlas.h"

//////////////////////////////////////////////////////////////////////////
void dz_atlas_create( const dz_service_t * _service, dz_atlas_t ** _atlas, dz_userdata_t _surface, dz_userdata_t _ud )
{
    dz_atlas_t * atlas = DZ_NEW( _service, dz_atlas_t );

    atlas->surface = _surface;
    atlas->ud = _ud;

    *_atlas = atlas;

}
//////////////////////////////////////////////////////////////////////////
void dz_atlas_destroy( const dz_service_t * _service, const dz_atlas_t * _atlas )
{
    DZ_FREE( _service, _atlas );
}
//////////////////////////////////////////////////////////////////////////
void dz_atlas_set_surface( dz_atlas_t * const _atlas, dz_userdata_t _surface )
{
    _atlas->surface = _surface;
}
//////////////////////////////////////////////////////////////////////////
dz_userdata_t dz_atlas_get_surface( const dz_atlas_t * _atlas )
{
    return _atlas->surface;
}
//////////////////////////////////////////////////////////////////////////
void dz_atlas_set_ud( dz_atlas_t * const _atlas, dz_userdata_t _ud )
{
    _atlas->ud = _ud;
}
//////////////////////////////////////////////////////////////////////////
dz_userdata_t dz_atlas_get_ud( const dz_atlas_t * _atlas )
{
    return _atlas->ud;
}
