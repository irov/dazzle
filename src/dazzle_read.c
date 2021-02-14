#include "dazzle/dazzle_read.h"

#include "texture.h"
#include "atlas.h"
#include "material.h"
#include "timeline_interpolate.h"
#include "timeline_key.h"
#include "shape.h"
#include "emitter.h"
#include "affector.h"

//////////////////////////////////////////////////////////////////////////
#define DZ_READ(L, U, V) if( (*L)(&V, sizeof(V), U) == DZ_FAILURE ) return DZ_FAILURE
#define DZ_READN(L, U, V, N) if( (*L)(V, sizeof(*V) * N, U) == DZ_FAILURE ) return DZ_FAILURE
//////////////////////////////////////////////////////////////////////////
static dz_result_t __read_bool( dz_bool_t * _b, dz_stream_read_t _read, dz_userdata_t _ud )
{
    uint8_t v;
    DZ_READ( _read, _ud, v );

    *_b = (dz_bool_t)v;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
#define DZ_READB(L, U, V) if( __read_bool(&V, L, U) == DZ_FAILURE ) return DZ_FAILURE;
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_header_read( dz_stream_read_t _read, dz_userdata_t _ud, dz_effect_read_status_e * const _status )
{
    uint32_t read_magic;
    DZ_READ( _read, _ud, read_magic );

    uint32_t magic = dz_get_magic();

    if( read_magic != magic )
    {
        *_status = DZ_EFFECT_LOAD_STATUS_INVALID_MAGIC;

        return DZ_FAILURE;
    }

    uint32_t read_version;
    DZ_READ( _read, _ud, read_version );

    const uint32_t version = dz_get_version();

    if( read_version != version )
    {
        *_status = DZ_EFFECT_LOAD_STATUS_INVALID_VERSION;

        return DZ_FAILURE;
    }

    *_status = DZ_EFFECT_LOAD_STATUS_SUCCESSFUL;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __read_texture( const dz_service_t * _service, dz_texture_t ** _texture, dz_stream_read_t _read, dz_userdata_t _ud )
{
    dz_texture_t * texture;
    if( dz_texture_create( _service, &texture, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    DZ_READN( _read, _ud, texture->u, 4 );
    DZ_READN( _read, _ud, texture->v, 4 );

    *_texture = texture;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __read_atlas( const dz_service_t * _service, dz_atlas_t ** _atlas, dz_stream_read_t _read, dz_userdata_t _ud )
{
    dz_atlas_t * atlas;
    if( dz_atlas_create( _service, &atlas, DZ_NULLPTR, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    DZ_READ( _read, _ud, atlas->texture_count );

    for( uint32_t index = 0; index != atlas->texture_count; ++index )
    {
        dz_texture_t * texture;
        if( __read_texture( _service, &texture, _read, _ud ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        atlas->textures[index] = texture;
    }

    *_atlas = atlas;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __read_material( const dz_service_t * _service, dz_material_t ** _material, dz_stream_read_t _read, dz_userdata_t _ud )
{
    dz_material_t * material;
    if( dz_material_create( _service, &material, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    DZ_READ( _read, _ud, material->blend_type );

    DZ_READ( _read, _ud, material->r );
    DZ_READ( _read, _ud, material->g );
    DZ_READ( _read, _ud, material->b );
    DZ_READ( _read, _ud, material->a );

    DZ_READ( _read, _ud, material->mode );

    dz_bool_t has_atlas;
    DZ_READB( _read, _ud, has_atlas );

    if( has_atlas == DZ_TRUE )
    {
        dz_atlas_t * atlas;
        if( __read_atlas( _service, &atlas, _read, _ud ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        material->atlas = atlas;
    }

    *_material = material;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __read_timeline_key( const dz_service_t * _service, dz_timeline_key_t ** _key, dz_stream_read_t _read, dz_userdata_t _ud );
//////////////////////////////////////////////////////////////////////////
static dz_result_t __read_timeline_interpolate( const dz_service_t * _service, dz_timeline_interpolate_t ** _interpolate, dz_stream_read_t _read, dz_userdata_t _ud )
{
    dz_timeline_interpolate_type_e type;
    DZ_READ( _read, _ud, type );

    dz_timeline_interpolate_t * interpolate;
    if( dz_timeline_interpolate_create( _service, &interpolate, type, _ud ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    DZ_READ( _read, _ud, interpolate->p0 );
    DZ_READ( _read, _ud, interpolate->p1 );

    dz_bool_t has_timeline_key;
    DZ_READB( _read, _ud, has_timeline_key );

    if( has_timeline_key == DZ_TRUE )
    {
        dz_timeline_key_t * key;
        if( __read_timeline_key( _service, &key, _read, _ud ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        interpolate->key = key;
    }

    *_interpolate = interpolate;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __read_timeline_key( const dz_service_t * _service, dz_timeline_key_t ** _key, dz_stream_read_t _read, dz_userdata_t _ud )
{
    float p;
    DZ_READ( _read, _ud, p );

    dz_timeline_key_type_e type;
    DZ_READ( _read, _ud, type );

    dz_timeline_key_t * key;
    if( dz_timeline_key_create( _service, &key, p, type, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    DZ_READ( _read, _ud, key->const_value );

    DZ_READ( _read, _ud, key->randomize_min_value );
    DZ_READ( _read, _ud, key->randomize_max_value );

    dz_bool_t has_interpolate;
    DZ_READB( _read, _ud, has_interpolate );

    if( has_interpolate == DZ_TRUE )
    {
        dz_timeline_interpolate_t * interpolate;
        if( __read_timeline_interpolate( _service, &interpolate, _read, _ud ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        key->interpolate = interpolate;
    }

    *_key = key;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __read_shape( const dz_service_t * _service, dz_shape_t ** _shape, dz_stream_read_t _read, dz_userdata_t _ud )
{
    dz_shape_type_e type;
    DZ_READ( _read, _ud, type );

    dz_shape_t * shape;
    if( dz_shape_create( _service, &shape, type, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    for( uint32_t index = 0; index != __DZ_SHAPE_TIMELINE_MAX__; ++index )
    {
        uint8_t exist;
        DZ_READ( _read, _ud, exist );

        if( exist == DZ_FALSE )
        {
            continue;
        }

        dz_timeline_key_t * timeline;
        if( __read_timeline_key( _service, &timeline, _read, _ud ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        shape->timelines[index] = timeline;
    }

    *_shape = shape;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __read_emitter( const dz_service_t * _service, dz_emitter_t ** _emitter, dz_stream_read_t _read, dz_userdata_t _ud )
{
    dz_emitter_t * emitter;
    if( dz_emitter_create( _service, &emitter, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    DZ_READ( _read, _ud, emitter->life );

    for( uint32_t index = 0; index != __DZ_EMITTER_TIMELINE_MAX__; ++index )
    {
        uint8_t exist;
        DZ_READ( _read, _ud, exist );

        if( exist == DZ_FALSE )
        {
            continue;
        }

        dz_timeline_key_t * key;
        if( __read_timeline_key( _service, &key, _read, _ud ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        emitter->timelines[index] = key;
    }

    *_emitter = emitter;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __read_affector( const dz_service_t * _service, dz_affector_t ** _affector, dz_stream_read_t _read, dz_userdata_t _ud )
{
    dz_affector_t * affector;
    if( dz_affector_create( _service, &affector, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    for( uint32_t index = 0; index != __DZ_AFFECTOR_TIMELINE_MAX__; ++index )
    {
        uint8_t exist;
        DZ_READ( _read, _ud, exist );

        if( exist == DZ_FALSE )
        {
            continue;
        }

        dz_timeline_key_t * key;
        if( __read_timeline_key( _service, &key, _read, _ud ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        affector->timelines[index] = key;
    }

    *_affector = affector;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_effect_read( const dz_service_t * _service, dz_effect_t ** _effect, dz_stream_read_t _read, dz_userdata_t _ud )
{
    dz_material_t * material;
    if( __read_material( _service, &material, _read, _ud ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    dz_shape_t * shape;
    if( __read_shape( _service, &shape, _read, _ud ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    dz_emitter_t * emitter;
    if( __read_emitter( _service, &emitter, _read, _ud ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    dz_affector_t * affector;
    if( __read_affector( _service, &affector, _read, _ud ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    float life;
    DZ_READ( _read, _ud, life );

    dz_effect_t * effect;
    if( dz_effect_create( _service, &effect, material, shape, emitter, affector, life, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    *_effect = effect;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////