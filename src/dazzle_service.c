#include "dazzle/dazzle.h"

#include "alloc.h"
#include "service.h"

//////////////////////////////////////////////////////////////////////////
void dz_service_create( dz_service_t ** _service, const dz_service_providers_t * _providers, dz_userdata_t _ud )
{
    dz_service_t * service = (dz_service_t *)(*_providers->f_malloc)(sizeof( dz_service_t ), _ud);

    service->providers = *_providers;
    service->ud = _ud;

    *_service = service;

}
//////////////////////////////////////////////////////////////////////////
void dz_service_destroy( const dz_service_t * _service )
{
    DZ_FREE( _service, _service );
}
//////////////////////////////////////////////////////////////////////////
void dz_service_get_providers( const dz_service_t * _service, dz_service_providers_t * _providers )
{
    *_providers = _service->providers;
}
