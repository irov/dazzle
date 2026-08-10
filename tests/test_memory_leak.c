#include "test_support.h"

int main( void )
{
    dz_test_memory_t memory = { 0, 0, 0 };
    dz_service_t * service;
    dz_test_service_create( &service, &memory );
    dz_service_destroy( service );
    DZ_TEST_CHECK( memory.allocated == 0 );
    return EXIT_SUCCESS;
}
