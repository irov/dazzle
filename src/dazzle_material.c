#include "dazzle/dazzle.h"

#include "alloc.h"
#include "material.h"
#include "memory.h"
#include "texture.h"

//////////////////////////////////////////////////////////////////////////
dz_material_mode_e dz_material_get_default_mode( void )
{
    return DZ_MATERIAL_MODE_SOLID;
}
//////////////////////////////////////////////////////////////////////////
dz_blend_type_e dz_material_get_default_blend( void )
{
    return DZ_BLEND_NORMAL;
}
//////////////////////////////////////////////////////////////////////////
void dz_material_create( const dz_service_t * _service, dz_material_t ** _material, dz_userdata_t _ud )
{
    dz_material_t * material = DZ_NEW( _service, dz_material_t );

    material->blend_type = dz_material_get_default_blend();

    material->r = 1.f;
    material->g = 1.f;
    material->b = 1.f;
    material->a = 1.f;

    material->mode = dz_material_get_default_mode();
    material->atlas = DZ_NULLPTR;
    material->textures_count = 0;
    material->textures_time = 0.f;
    material->textures_random_weight = 0.f;
    material->texture_index = 0;
    material->texture_count = 1;

    material->pass_count = 1;
    dz_memory_zero( material->passes, sizeof( material->passes ) );
    dz_memory_copy( material->passes[0].technique_id, "dazzle.textured", sizeof( "dazzle.textured" ) );
    material->passes[0].blend = DZ_BLEND_NORMAL;
    material->passes[0].depth_test = DZ_FALSE;
    material->passes[0].depth_write = DZ_FALSE;
    material->passes[0].depth_compare = DZ_DEPTH_LESS_EQUAL;
    material->passes[0].cull = DZ_CULL_NONE;
    material->passes[0].color_mask = 0x0f;

    material->ud = _ud;

    *_material = material;

}
//////////////////////////////////////////////////////////////////////////
void dz_material_destroy( const dz_service_t * _service, const dz_material_t * _material )
{
    DZ_FREE( _service, _material );
}
//////////////////////////////////////////////////////////////////////////
void dz_material_set_ud( dz_material_t * const _material, dz_userdata_t _ud )
{
    _material->ud = _ud;
}
//////////////////////////////////////////////////////////////////////////
dz_userdata_t dz_material_get_ud( const dz_material_t * _material )
{
    return _material->ud;
}
//////////////////////////////////////////////////////////////////////////
void dz_material_set_blend( dz_material_t * const _material, dz_blend_type_e _blend )
{
    _material->blend_type = _blend;
}
//////////////////////////////////////////////////////////////////////////
dz_blend_type_e dz_material_get_blend( const dz_material_t * _material )
{
    return _material->blend_type;
}
//////////////////////////////////////////////////////////////////////////
void dz_material_set_color( dz_material_t * const _material, dz_float_t _r, dz_float_t _g, dz_float_t _b, dz_float_t _a )
{
    _material->r = _r;
    _material->g = _g;
    _material->b = _b;
    _material->a = _a;
}
//////////////////////////////////////////////////////////////////////////
void dz_material_get_color( const dz_material_t * _material, dz_float_t * const _r, dz_float_t * const _g, dz_float_t * const _b, dz_float_t * const _a )
{
    *_r = _material->r;
    *_g = _material->g;
    *_b = _material->b;
    *_a = _material->a;
}
//////////////////////////////////////////////////////////////////////////
void dz_material_set_atlas( dz_material_t * const _material, const dz_atlas_t * _atlas )
{
    _material->atlas = _atlas;
}
//////////////////////////////////////////////////////////////////////////
const dz_atlas_t * dz_material_get_atlas( const dz_material_t * _material )
{
    return _material->atlas;
}
//////////////////////////////////////////////////////////////////////////
static void __material_update_textures_random_weight( dz_material_t * const _material )
{
    dz_float_t textures_random_weight = 0.f;

    for( dz_uint32_t index = 0; index != _material->textures_count; ++index )
    {
        textures_random_weight += DZ_MAX( _material->textures[index].random_weight, 0.f );
    }

    _material->textures_random_weight = textures_random_weight;
}
//////////////////////////////////////////////////////////////////////////
dz_uint32_t dz_material_get_texture_slot_count( const dz_material_t * _material )
{
    return _material->textures_count;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_material_add_texture( dz_material_t * const _material, const dz_texture_t * _texture )
{
    if( _material->textures_count >= 64 )
    {
        return DZ_FAILURE;
    }

    dz_material_texture_t * material_texture = &_material->textures[_material->textures_count++];
    material_texture->texture = _texture;
    material_texture->random_weight = 1.f;

    _material->textures_time += _texture->sequence_delay;
    __material_update_textures_random_weight( _material );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_material_pop_texture( dz_material_t * const _material, const dz_texture_t ** _texture )
{
    if( _material->textures_count == 0 )
    {
        return DZ_FAILURE;
    }

    const dz_texture_t * texture = _material->textures[_material->textures_count - 1].texture;

    *_texture = texture;

    _material->textures[_material->textures_count - 1].texture = DZ_NULLPTR;
    _material->textures[_material->textures_count - 1].random_weight = 0.f;

    _material->textures_count--;

    _material->textures_time -= texture->sequence_delay;
    __material_update_textures_random_weight( _material );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_material_get_texture( const dz_material_t * _material, dz_uint32_t _index, const dz_texture_t ** _texture )
{
    if( _index >= _material->textures_count )
    {
        return DZ_FAILURE;
    }

    const dz_texture_t * texture = _material->textures[_index].texture;

    *_texture = texture;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_material_set_texture_random_weight( dz_material_t * const _material, dz_uint32_t _index, dz_float_t _weight )
{
    if( _index >= _material->textures_count )
    {
        return DZ_FAILURE;
    }

    dz_material_texture_t * material_texture = &_material->textures[_index];
    const dz_float_t weight = DZ_MAX( _weight, 0.f );

    material_texture->random_weight = weight;

    __material_update_textures_random_weight( _material );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_material_get_texture_random_weight( const dz_material_t * _material, dz_uint32_t _index, dz_float_t * const _weight )
{
    if( _index >= _material->textures_count )
    {
        return DZ_FAILURE;
    }

    *_weight = _material->textures[_index].random_weight;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_material_set_texture_index( dz_material_t * const _material, dz_uint32_t _index )
{
    _material->texture_index = _index;
}
//////////////////////////////////////////////////////////////////////////
dz_uint32_t dz_material_get_texture_index( const dz_material_t * _material )
{
    return _material->texture_index;
}
//////////////////////////////////////////////////////////////////////////
void dz_material_set_texture_count( dz_material_t * const _material, dz_uint32_t _count )
{
    _material->texture_count = _count;
}
//////////////////////////////////////////////////////////////////////////
dz_uint32_t dz_material_get_texture_count( const dz_material_t * _material )
{
    return _material->texture_count;
}
//////////////////////////////////////////////////////////////////////////
void dz_material_set_mode( dz_material_t * const _material, dz_material_mode_e _mode )
{
    _material->mode = _mode;
}
//////////////////////////////////////////////////////////////////////////
dz_material_mode_e dz_material_get_mode( const dz_material_t * _material )
{
    return _material->mode;
}
//////////////////////////////////////////////////////////////////////////
dz_uint32_t dz_material_get_pass_count( const dz_material_t * _material )
{
    return _material->pass_count;
}
//////////////////////////////////////////////////////////////////////////
void dz_material_add_pass( dz_material_t * _material, const dz_material_pass_desc_t * _pass, dz_uint32_t * _index )
{
    const dz_uint32_t index = _material->pass_count++;
    _material->passes[index] = *_pass;
    _material->passes[index].technique_id[DZ_TECHNIQUE_ID_MAX - 1] = '\0';

    if( _index != DZ_NULLPTR )
    {
        *_index = index;
    }

}
//////////////////////////////////////////////////////////////////////////
void dz_material_remove_pass( dz_material_t * _material, dz_uint32_t _index )
{
    for( dz_uint32_t move = _index + 1U; move != _material->pass_count; ++move )
    {
        _material->passes[move - 1U] = _material->passes[move];
    }

    --_material->pass_count;
}
//////////////////////////////////////////////////////////////////////////
void dz_material_set_pass( dz_material_t * _material, dz_uint32_t _index, const dz_material_pass_desc_t * _pass )
{
    _material->passes[_index] = *_pass;
    _material->passes[_index].technique_id[DZ_TECHNIQUE_ID_MAX - 1] = '\0';
}
//////////////////////////////////////////////////////////////////////////
void dz_material_get_pass( const dz_material_t * _material, dz_uint32_t _index, dz_material_pass_desc_t * _pass )
{
    *_pass = _material->passes[_index];
}
