#include "dazzle/dazzle_write.h"

#include "material.h"
#include "atlas.h"
#include "texture.h"
#include "timeline_interpolate.h"
#include "timeline_key.h"
#include "shape.h"
#include "emitter.h"
#include "affector.h"
#include "effect.h"

//////////////////////////////////////////////////////////////////////////
#define DZ_WRITE(W, U, V) if( (*W)(&V, sizeof(V), U) == DZ_FAILURE ) return DZ_FAILURE
#define DZ_WRITEN(W, U, V, N) if( (*W)(V, sizeof(*V) * N, U) == DZ_FAILURE ) return DZ_FAILURE
//////////////////////////////////////////////////////////////////////////
static dz_result_t __write_bool( dz_bool_t _b, dz_stream_write_t _write, dz_userdata_t _ud )
{
    const dz_uint8_t v = (dz_uint8_t)_b;

    DZ_WRITE( _write, _ud, v );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
#define DZ_WRITEB(W, U, V) if( __write_bool(V, W, U) == DZ_FAILURE ) return DZ_FAILURE
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_header_write( dz_stream_write_t _write, dz_userdata_t _ud )
{
    const dz_uint32_t magic = dz_get_magic();

    DZ_WRITE( _write, _ud, magic );

    const dz_uint32_t version = dz_get_version();

    DZ_WRITE( _write, _ud, version );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __write_texture( const dz_texture_t * _texture, dz_float_t _random_weight, dz_stream_write_t _write, dz_userdata_t _ud )
{
    DZ_WRITEN( _write, _ud, _texture->u, 4 );
    DZ_WRITEN( _write, _ud, _texture->v, 4 );

    DZ_WRITE( _write, _ud, _texture->width );
    DZ_WRITE( _write, _ud, _texture->height );

    DZ_WRITE( _write, _ud, _texture->trim_offset_x );
    DZ_WRITE( _write, _ud, _texture->trim_offset_y );

    DZ_WRITE( _write, _ud, _texture->trim_width );
    DZ_WRITE( _write, _ud, _texture->trim_height );

    DZ_WRITE( _write, _ud, _random_weight );
    DZ_WRITE( _write, _ud, _texture->sequence_delay );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __write_atlas( const dz_atlas_t * _atlas, dz_stream_write_t _write, dz_userdata_t _ud )
{
    DZ_UNUSED( _atlas );
    DZ_UNUSED( _write );
    DZ_UNUSED( _ud );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __write_material( const dz_material_t * _material, dz_stream_write_t _write, dz_userdata_t _ud )
{
    DZ_WRITE( _write, _ud, _material->blend_type );

    DZ_WRITE( _write, _ud, _material->r );
    DZ_WRITE( _write, _ud, _material->g );
    DZ_WRITE( _write, _ud, _material->b );
    DZ_WRITE( _write, _ud, _material->a );

    DZ_WRITE( _write, _ud, _material->mode );
    DZ_WRITE( _write, _ud, _material->texture_index );
    DZ_WRITE( _write, _ud, _material->texture_count );
    DZ_WRITE( _write, _ud, _material->textures_count );

    for( dz_uint32_t index = 0; index != _material->textures_count; ++index )
    {
        const dz_material_texture_t * material_texture = &_material->textures[index];

        if( __write_texture( material_texture->texture, material_texture->random_weight, _write, _ud ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __write_timeline_key( const dz_timeline_key_t * _key, dz_stream_write_t _write, dz_userdata_t _ud );
//////////////////////////////////////////////////////////////////////////
static dz_result_t __write_timeline_interpolate( const dz_timeline_interpolate_t * _interpolate, dz_stream_write_t _write, dz_userdata_t _ud )
{
    DZ_WRITE( _write, _ud, _interpolate->type );

    DZ_WRITE( _write, _ud, _interpolate->p0 );
    DZ_WRITE( _write, _ud, _interpolate->p1 );

    if( _interpolate->key == DZ_NULLPTR )
    {
        DZ_WRITEB( _write, _ud, DZ_FALSE );
    }
    else
    {
        DZ_WRITEB( _write, _ud, DZ_TRUE );

        if( __write_timeline_key( _interpolate->key, _write, _ud ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __write_timeline_key( const dz_timeline_key_t * _key, dz_stream_write_t _write, dz_userdata_t _ud )
{
    DZ_WRITE( _write, _ud, _key->p );

    DZ_WRITE( _write, _ud, _key->type );

    DZ_WRITE( _write, _ud, _key->const_value );

    DZ_WRITE( _write, _ud, _key->randomize_min_value );
    DZ_WRITE( _write, _ud, _key->randomize_max_value );

    if( _key->interpolate == DZ_NULLPTR )
    {
        DZ_WRITEB( _write, _ud, DZ_FALSE );
    }
    else
    {
        DZ_WRITEB( _write, _ud, DZ_TRUE );

        if( __write_timeline_interpolate( _key->interpolate, _write, _ud ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __write_shape( const dz_shape_t * _shape, dz_stream_write_t _write, dz_userdata_t _ud )
{
    DZ_WRITE( _write, _ud, _shape->type );

    for( dz_uint32_t index = 0; index != __DZ_SHAPE_TIMELINE_MAX__; ++index )
    {
        const dz_timeline_key_t * timeline = _shape->timelines[index];

        if( timeline == DZ_NULLPTR )
        {
            DZ_WRITEB( _write, _ud, DZ_FALSE );

            continue;
        }

        DZ_WRITEB( _write, _ud, DZ_TRUE );

        if( __write_timeline_key( timeline, _write, _ud ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __write_emitter( const dz_emitter_t * _emitter, dz_stream_write_t _write, dz_userdata_t _ud )
{
    DZ_WRITE( _write, _ud, _emitter->life );

    for( dz_uint32_t index = 0; index != __DZ_EMITTER_TIMELINE_MAX__; ++index )
    {
        const dz_timeline_key_t * timeline = _emitter->timelines[index];

        if( timeline == DZ_NULLPTR )
        {
            DZ_WRITEB( _write, _ud, DZ_FALSE );

            continue;
        }

        DZ_WRITEB( _write, _ud, DZ_TRUE );

        if( __write_timeline_key( timeline, _write, _ud ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __write_affector( const dz_affector_t * _affector, dz_stream_write_t _write, dz_userdata_t _ud )
{
    for( dz_uint32_t index = 0; index != __DZ_AFFECTOR_TIMELINE_MAX__; ++index )
    {
        const dz_timeline_key_t * timeline = _affector->timelines[index];

        if( timeline == DZ_NULLPTR )
        {
            DZ_WRITEB( _write, _ud, DZ_FALSE );

            continue;
        }

        DZ_WRITEB( _write, _ud, DZ_TRUE );

        if( __write_timeline_key( timeline, _write, _ud ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __write_effect_layer( const dz_effect_layer_desc_t * _layer, dz_stream_write_t _write, dz_userdata_t _ud )
{
    if( __write_material( _layer->material, _write, _ud ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    if( __write_shape( _layer->shape, _write, _ud ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    if( __write_emitter( _layer->emitter, _write, _ud ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    if( __write_affector( _layer->affector, _write, _ud ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    DZ_WRITE( _write, _ud, _layer->x );
    DZ_WRITE( _write, _ud, _layer->y );
    DZ_WRITE( _write, _ud, _layer->angle );
    DZ_WRITE( _write, _ud, _layer->life );
    DZ_WRITE( _write, _ud, _layer->seed );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __write_effect_trigger( const dz_effect_trigger_desc_t * _trigger, dz_stream_write_t _write, dz_userdata_t _ud )
{
    DZ_WRITE( _write, _ud, _trigger->event_type );
    DZ_WRITE( _write, _ud, _trigger->source_layer_index );
    DZ_WRITE( _write, _ud, _trigger->target_layer_index );
    DZ_WRITE( _write, _ud, _trigger->time );
    DZ_WRITE( _write, _ud, _trigger->probability );
    DZ_WRITE( _write, _ud, _trigger->spawn_count_min );
    DZ_WRITE( _write, _ud, _trigger->spawn_count_max );
    DZ_WRITE( _write, _ud, _trigger->delay_min );
    DZ_WRITE( _write, _ud, _trigger->delay_max );
    DZ_WRITEB( _write, _ud, _trigger->inherit_position );
    DZ_WRITEB( _write, _ud, _trigger->inherit_angle );
    DZ_WRITEB( _write, _ud, _trigger->inherit_velocity );
    DZ_WRITE( _write, _ud, _trigger->offset_x );
    DZ_WRITE( _write, _ud, _trigger->offset_y );
    DZ_WRITE( _write, _ud, _trigger->angle_offset );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static const dz_atlas_t * __effect_find_atlas( const dz_effect_t * _effect )
{
    for( dz_uint32_t index = 0; index != _effect->layer_count; ++index )
    {
        const dz_material_t * material = _effect->layers[index].material;

        if( material != DZ_NULLPTR && material->atlas != DZ_NULLPTR )
        {
            return material->atlas;
        }
    }

    return DZ_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_effect_write( const dz_effect_t * _effect, dz_stream_write_t _write, dz_userdata_t _ud )
{
    const dz_atlas_t * atlas = _effect->atlas;

    if( atlas == DZ_NULLPTR )
    {
        atlas = __effect_find_atlas( _effect );
    }

    if( atlas == DZ_NULLPTR )
    {
        DZ_WRITEB( _write, _ud, DZ_FALSE );
    }
    else
    {
        DZ_WRITEB( _write, _ud, DZ_TRUE );

        if( __write_atlas( atlas, _write, _ud ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }
    }

    DZ_WRITE( _write, _ud, _effect->layer_count );

    for( dz_uint32_t index = 0; index != _effect->layer_count; ++index )
    {
        if( __write_effect_layer( _effect->layers + index, _write, _ud ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }
    }

    DZ_WRITE( _write, _ud, _effect->trigger_count );

    for( dz_uint32_t index = 0; index != _effect->trigger_count; ++index )
    {
        if( __write_effect_trigger( _effect->triggers + index, _write, _ud ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }
    }

    DZ_WRITE( _write, _ud, _effect->life );
    DZ_WRITE( _write, _ud, _effect->seed );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////