#pragma once

#include "dazzle/dazzle.hpp"

#include "jpp/jpp.hpp"

jpp::object dz_evict_write( const dz_effect_t * _effect );
void dz_evict_create( dz_service_t * _service, dz_effect_t ** _effect, const jpp::object & _data );