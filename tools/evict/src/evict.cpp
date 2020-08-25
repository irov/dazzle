#include "evict/evict.hpp"

//////////////////////////////////////////////////////////////////////////
static jpp::object __evict_texture_write( const dz_texture_t * _texture )
{
    (void)_texture;

    jpp::object obj = jpp::make_object();

    jpp::array a = jpp::make_tuple( 1.f, 2.f, 3.f, 4.f );

    //obj.set( "u0" )

    //float u[4];
    //float v[4];

    //float trim_offset_x;
    //float trim_offset_y;

    //float trim_width;
    //float trim_height;

    //float random_weight;
    //float sequence_delay;

    return obj;
}
//////////////////////////////////////////////////////////////////////////
static jpp::object __evict_atlas_write( const dz_atlas_t * _atlas )
{
    jpp::object obj = jpp::make_object();

    jpp::array array_textures = jpp::make_array();

    uint32_t texture_count = dz_atlas_get_texture_count( _atlas );

    for( uint32_t index = 0; index != texture_count; ++index )
    {
        const dz_texture_t * texture;
        dz_atlas_get_texture( _atlas, index, &texture );

        jpp::object obj_texture = __evict_texture_write( texture );

        array_textures.push_back( obj_texture );
    }

    obj.set( "textures", array_textures );

    return obj;
}
//////////////////////////////////////////////////////////////////////////
static jpp::object __evict_material_write( const dz_material_t * _material )
{
    jpp::object obj = jpp::make_object();

    dz_blend_type_e blend_type = dz_material_get_blend( _material );

    const char * blend_type_str = dz_blend_type_stringize( blend_type );

    obj.set( "blend_type", blend_type_str );

    float r;
    float g;
    float b;
    float a;
    dz_material_get_color( _material, &r, &g, &b, &a );

    obj.set( "color_r", r );
    obj.set( "color_g", g );
    obj.set( "color_b", b );
    obj.set( "color_a", a );

    const dz_atlas_t * atlas = dz_material_get_atlas( _material );

    jpp::object obj_atlas = __evict_atlas_write( atlas );

    obj.set( "atlas", obj_atlas );

    return obj;
}
//////////////////////////////////////////////////////////////////////////
jpp::object dz_evict_write( const dz_effect_t * _effect )
{
    jpp::object obj = jpp::make_object();

    const dz_material_t * material = dz_effect_get_material( _effect );

    jpp::object obj_material = __evict_material_write( material );

    obj.set( "material", obj_material );

    return obj;
}
//////////////////////////////////////////////////////////////////////////
void dz_evict_create( dz_service_t * _service, dz_effect_t ** _effect, const jpp::object & _data )
{
    (void)_service;
    (void)_effect;
    (void)_data;
}
//////////////////////////////////////////////////////////////////////////