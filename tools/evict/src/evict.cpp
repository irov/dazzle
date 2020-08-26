#include "evict/evict.hpp"

//////////////////////////////////////////////////////////////////////////
static jpp::object __evict_texture_write( const dz_texture_t * _texture )
{
    (void)_texture;

    jpp::object obj = jpp::make_object();

    float u[4];
    float v[4];
    dz_texture_get_uv( _texture, u, v );

    obj.set( "u", jpp::make_tuple( u[0], u[1], u[2], u[3] ) );
    obj.set( "v", jpp::make_tuple( v[0], v[1], v[2], v[3] ) );

    float trim_offset_x;
    float trim_offset_y;
    dz_texture_get_trim_offset( _texture, &trim_offset_x, &trim_offset_y );

    obj.set( "trim_offset", jpp::make_tuple( trim_offset_x, trim_offset_y ) );

    float trime_width;
    float trime_height;
    dz_texture_get_trim_size( _texture, &trime_width, &trime_height );

    obj.set( "trim_size", jpp::make_tuple( trime_width, trime_height ) );

    float random_weight = dz_texture_get_random_weight( _texture );

    obj.set( "random_weight", random_weight );

    float sequence_delay = dz_texture_get_sequence_delay( _texture );

    obj.set( "sequence_delay", sequence_delay );

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

    obj.set( "color", jpp::make_tuple( r, g, b, a ) );

    const dz_atlas_t * atlas = dz_material_get_atlas( _material );

    jpp::object obj_atlas = __evict_atlas_write( atlas );

    obj.set( "atlas", obj_atlas );

    return obj;
}
//////////////////////////////////////////////////////////////////////////
static jpp::object __evict_timeline_key_write( const dz_timeline_key_t * _key );
//////////////////////////////////////////////////////////////////////////
static jpp::object __evict_timeline_interpolate_write( const dz_timeline_interpolate_t * _interpolate )
{
    jpp::object obj = jpp::make_object();

    dz_timeline_interpolate_type_e interpolate_type = dz_timeline_interpolate_get_type( _interpolate );

    const char * interpolate_type_str = dz_timeline_interpolate_type_stringize( interpolate_type );

    obj.set( "type", interpolate_type_str );

    float p0;
    float p1;
    dz_timeline_interpolate_get_bezier2( _interpolate, &p0, &p1 );

    obj.set( "bezier2", jpp::make_tuple( p0, p1 ) );

    const dz_timeline_key_t * key = dz_timeline_interpolate_get_key( _interpolate );

    if( key != DZ_NULLPTR )
    {
        jpp::object obj_key = __evict_timeline_key_write( key );

        obj.set( "key", obj_key );
    }

    return obj;
}
//////////////////////////////////////////////////////////////////////////
static jpp::object __evict_timeline_key_write( const dz_timeline_key_t * _key )
{
    jpp::object obj = jpp::make_object();

    dz_timeline_key_type_e key_type = dz_timeline_key_get_type( _key );

    const char * key_type_str = dz_timeline_key_type_stringize( key_type );

    obj.set( "type", key_type_str );

    float p = dz_timeline_key_get_p( _key );

    obj.set( "p", p );

    float const_value;
    dz_timeline_key_get_const_value( _key, &const_value );

    obj.set( "const_value", const_value );

    float randomize_min;
    float randomize_max;
    dz_timeline_key_get_randomize_min_max( _key, &randomize_min, &randomize_max );

    obj.set( "randomize_min", randomize_min );
    obj.set( "randomize_max", randomize_max );

    const dz_timeline_interpolate_t * interpolate = dz_timeline_key_get_interpolate( _key );

    if( interpolate != DZ_NULLPTR )
    {
        jpp::object obj_interpolate = __evict_timeline_interpolate_write( interpolate );

        obj.set( "interpolate", obj_interpolate );
    }

    return obj;
}
//////////////////////////////////////////////////////////////////////////
static jpp::object __evict_shape_write( const dz_shape_t * _shape )
{
    jpp::object obj = jpp::make_object();

    dz_shape_type_e shape_type = dz_shape_get_type( _shape );

    const char * shape_type_str = dz_shape_type_stringize( shape_type );

    obj.set( "type", shape_type_str );

    jpp::object obj_timeline = jpp::make_object();

    for( uint32_t index = 0; index != __DZ_SHAPE_TIMELINE_MAX__; ++index )
    {
        dz_shape_timeline_type_e timeline_type = (dz_shape_timeline_type_e)index;

        const dz_timeline_key_t * key = dz_shape_get_timeline( _shape, timeline_type );

        jpp::object obj_key = __evict_timeline_key_write( key );

        const char * timeline_type_str = dz_shape_timeline_type_stringize( timeline_type );

        obj_timeline.set( timeline_type_str, obj_key );
    }

    obj.set( "timeline", obj_timeline );

    return obj;
}
//////////////////////////////////////////////////////////////////////////
static jpp::object __evict_emitter_write( const dz_emitter_t * _emitter )
{
    jpp::object obj = jpp::make_object();

    float life = dz_emitter_get_life( _emitter );

    obj.set( "life", life );

    jpp::object obj_timeline = jpp::make_object();

    for( uint32_t index = 0; index != __DZ_EMITTER_TIMELINE_MAX__; ++index )
    {
        dz_emitter_timeline_type_e timeline_type = (dz_emitter_timeline_type_e)index;

        const dz_timeline_key_t * key = dz_emitter_get_timeline( _emitter, timeline_type );

        jpp::object obj_key = __evict_timeline_key_write( key );

        const char * timeline_type_str = dz_emitter_timeline_type_stringize( timeline_type );

        obj_timeline.set( timeline_type_str, obj_key );
    }

    obj.set( "timeline", obj_timeline );

    return obj;
}
//////////////////////////////////////////////////////////////////////////
static jpp::object __evict_affector_write( const dz_affector_t * _affector )
{
    jpp::object obj = jpp::make_object();

    jpp::object obj_timeline = jpp::make_object();

    for( uint32_t index = 0; index != __DZ_AFFECTOR_TIMELINE_MAX__; ++index )
    {
        dz_affector_timeline_type_e timeline_type = (dz_affector_timeline_type_e)index;

        const dz_timeline_key_t * key = dz_affector_get_timeline( _affector, timeline_type );

        jpp::object obj_key = __evict_timeline_key_write( key );

        const char * timeline_type_str = dz_affector_timeline_type_stringize( timeline_type );

        obj_timeline.set( timeline_type_str, obj_key );
    }

    obj.set( "timeline", obj_timeline );

    return obj;
}
//////////////////////////////////////////////////////////////////////////
jpp::object dz_evict_write( const dz_effect_t * _effect )
{
    jpp::object obj = jpp::make_object();

    float life = dz_effect_get_life( _effect );

    obj.set( "life", life );

    dz_bool_t loop = dz_effect_get_loop( _effect );

    obj.set( "loop", loop );

    uint32_t seed = dz_effect_get_seed( _effect );

    obj.set( "seed", seed );

    uint32_t particle_limit = dz_effect_get_particle_limit( _effect );

    obj.set( "particle_limit", particle_limit );

    float x;
    float y;
    dz_effect_get_position( _effect, &x, &y );

    obj.set( "position", jpp::make_tuple( x, y ) );

    float rotate = dz_effect_get_rotate( _effect );

    obj.set( "rotate", rotate );

    const dz_material_t * material = dz_effect_get_material( _effect );

    jpp::object obj_material = __evict_material_write( material );

    obj.set( "material", obj_material );

    const dz_shape_t * shape = dz_effect_get_shape( _effect );

    jpp::object obj_shape = __evict_shape_write( shape );

    obj.set( "shape", obj_shape );

    const dz_emitter_t * emitter = dz_effect_get_emitter( _effect );

    jpp::object obj_emitter = __evict_emitter_write( emitter );

    obj.set( "emitter", obj_emitter );

    const dz_affector_t * affector = dz_effect_get_affector( _effect );

    jpp::object obj_affector = __evict_affector_write( affector );

    obj.set( "affector", obj_affector );

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