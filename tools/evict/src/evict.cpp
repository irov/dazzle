#include "evict/evict.hpp"

#include "dazzle/dazzle_aux.hpp"

//////////////////////////////////////////////////////////////////////////
static jpp::object __evict_texture_write( const dz_texture_t * _texture )
{
    jpp::object obj = jpp::make_object();

    float u[4];
    float v[4];
    dz_texture_get_uv( _texture, u, v );

    obj.set( "u", jpp::make_tuple( u[0], u[1], u[2], u[3] ) );
    obj.set( "v", jpp::make_tuple( v[0], v[1], v[2], v[3] ) );

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

    jpp::object obj_bezier2 = jpp::make_object();

    obj_bezier2.set( "p0", p0 );
    obj_bezier2.set( "p1", p1 );

    obj.set( "bezier2", obj_bezier2 );

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

        if( key == DZ_NULLPTR )
        {
            continue;
        }

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

        if( key == DZ_NULLPTR )
        {
            continue;
        }

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

        if( key == DZ_NULLPTR )
        {
            continue;
        }

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
static dz_result_t __evict_texture_load( dz_service_t * _service, dz_texture_t ** _texture, const jpp::object & _data )
{
    dz_texture_t * texture;
    if( dz_texture_create( _service, &texture, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    jpp::object j_u = _data["u"];
    jpp::object j_v = _data["v"];

    float u[4] = {j_u[0], j_u[1], j_u[2], j_u[3]};
    float v[4] = {j_v[0], j_v[1], j_v[2], j_v[3]};

    dz_texture_set_uv( texture, u, v );

    *_texture = texture;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __evict_atlas_load( dz_service_t * _service, dz_atlas_t ** _atlas, const jpp::object & _data )
{
    dz_atlas_t * atlas;
    if( dz_atlas_create( _service, &atlas, DZ_NULLPTR, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    jpp::array array_data = _data["textures"];

    for( const jpp::object & texture_data : array_data )
    {
        dz_texture_t * texture;
        if( __evict_texture_load( _service, &texture, texture_data ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        dz_atlas_add_texture( atlas, texture );
    }

    *_atlas = atlas;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __evict_material_load( dz_service_t * _service, dz_material_t ** _material, const jpp::object & _data )
{
    dz_material_t * material;
    if( dz_material_create( _service, &material, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    const char * j_blend_type = _data["blend_type"];

    dz_blend_type_e blend_type;
    if( strcmp( j_blend_type, "normal" ) == 0 )
    {
        blend_type = DZ_BLEND_NORMAL;
    }
    else if( strcmp( j_blend_type, "add" ) == 0 )
    {
        blend_type = DZ_BLEND_ADD;
    }
    else if( strcmp( j_blend_type, "multiply" ) == 0 )
    {
        blend_type = DZ_BLEND_MULTIPLY;
    }
    else if( strcmp( j_blend_type, "screen" ) == 0 )
    {
        blend_type = DZ_BLEND_SCREEN;
    }
    else
    {
        return DZ_FAILURE;
    }

    dz_material_set_blend( material, blend_type );

    jpp::object j_color = _data["color"];

    dz_material_set_color( material, j_color[0], j_color[1], j_color[2], j_color[3] );

    jpp::object j_atlas = _data["atlas"];

    dz_atlas_t * atlas;
    if( __evict_atlas_load( _service, &atlas, j_atlas ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    dz_material_set_atlas( material, atlas );

    *_material = material;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_timeline_interpolate_type_e __load_timeline_interpolate_type( const char * _type )
{
    if( strcmp( _type, "linear" ) == 0 )
    {
        return DZ_TIMELINE_INTERPOLATE_LINEAR;
    }
    else if( strcmp( _type, "bezier2" ) == 0 )
    {
        return DZ_TIMELINE_INTERPOLATE_BEZIER2;
    }

    return __DZ_TIMELINE_INTERPOLATE_MAX__;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __evict_timeline_key_load( dz_service_t * _service, dz_timeline_key_t ** _key, const jpp::object & _data );
//////////////////////////////////////////////////////////////////////////
static dz_result_t __evict_timeline_interpolate_load( dz_service_t * _service, dz_timeline_interpolate_t ** _interpolate, const jpp::object & _data )
{
    const char * j_interpolate_type = _data["type"];

    dz_timeline_interpolate_type_e timeline_interpolate_type = __load_timeline_interpolate_type( j_interpolate_type );

    dz_timeline_interpolate_t * interpolate;
    if( dz_timeline_interpolate_create( _service, &interpolate, timeline_interpolate_type, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    float p0 = _data["bezier2"]["p0"];
    float p1 = _data["bezier2"]["p1"];

    dz_timeline_interpolate_set_bezier2( interpolate, p0, p1 );

    jpp::object j_key;
    if( _data.exist( "key", &j_key ) == true )
    {
        dz_timeline_key_t * key;
        if( __evict_timeline_key_load( _service, &key, j_key ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        dz_timeline_interpolate_set_key( interpolate, key );
    }

    *_interpolate = interpolate;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_timeline_key_type_e __load_timeline_key_type( const char * _type )
{
    if( strcmp( _type, "const" ) == 0 )
    {
        return DZ_TIMELINE_KEY_CONST;
    }
    else if( strcmp( _type, "randomize" ) == 0 )
    {
        return DZ_TIMELINE_KEY_RANDOMIZE;
    }

    return __DZ_TIMELINE_KEY_MAX__;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __evict_timeline_key_load( dz_service_t * _service, dz_timeline_key_t ** _key, const jpp::object & _data )
{
    float p = _data["p"];

    const char * j_key_type = _data["type"];

    dz_timeline_key_type_e timeline_key_type = __load_timeline_key_type( j_key_type );

    if( timeline_key_type == __DZ_TIMELINE_KEY_MAX__ )
    {
        return DZ_FAILURE;
    }

    dz_timeline_key_t * key;
    if( dz_timeline_key_create( _service, &key, p, timeline_key_type, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    float const_value = _data["const_value"];
    dz_timeline_key_set_const_value( key, const_value );

    float randomize_min = _data["randomize_min"];
    float randomize_max = _data["randomize_max"];

    dz_timeline_key_set_randomize_min_max( key, randomize_min, randomize_max );

    jpp::object j_interpolate;
    if( _data.exist( "interpolate", &j_interpolate ) == true )
    {
        dz_timeline_interpolate_t * interpolate;
        if( __evict_timeline_interpolate_load( _service, &interpolate, j_interpolate ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        if( dz_timeline_key_set_interpolate( key, interpolate ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }
    }

    *_key = key;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_shape_type_e __load_shape_type( const char * _type )
{
    if( strcmp( _type, "point" ) == 0 )
    {
        return DZ_SHAPE_POINT;
    }
    else if( strcmp( _type, "segment" ) == 0 )
    {
        return DZ_SHAPE_SEGMENT;
    }
    else if( strcmp( _type, "circle" ) == 0 )
    {
        return DZ_SHAPE_CIRCLE;
    }
    else if( strcmp( _type, "line" ) == 0 )
    {
        return DZ_SHAPE_LINE;
    }
    else if( strcmp( _type, "rect" ) == 0 )
    {
        return DZ_SHAPE_RECT;
    }
    else if( strcmp( _type, "polygon" ) == 0 )
    {
        return DZ_SHAPE_POLYGON;
    }
    else if( strcmp( _type, "mask" ) == 0 )
    {
        return DZ_SHAPE_MASK;
    }

    return __DZ_SHAPE_MAX__;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __evict_shape_load( dz_service_t * _service, dz_shape_t ** _shape, const jpp::object & _data )
{
    const char * j_shape_type = _data["type"];

    dz_shape_type_e shape_type = __load_shape_type( j_shape_type );

    if( shape_type == __DZ_SHAPE_MAX__ )
    {
        return DZ_FAILURE;
    }

    dz_shape_t * shape;
    if( dz_shape_create( _service, &shape, shape_type, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    jpp::object j_timeline = _data["timeline"];

    for( uint32_t index = 0; index != __DZ_SHAPE_TIMELINE_MAX__; ++index )
    {
        dz_shape_timeline_type_e timeline_type = (dz_shape_timeline_type_e)index;

        const char * timeline_type_str = dz_shape_timeline_type_stringize( timeline_type );

        jpp::object j_timeline_key;
        if( j_timeline.exist( timeline_type_str, &j_timeline_key ) == false )
        {
            continue;
        }

        dz_timeline_key_t * timeline_key;
        if( __evict_timeline_key_load( _service, &timeline_key, j_timeline_key ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        dz_shape_set_timeline( shape, timeline_type, timeline_key );
    }

    *_shape = shape;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __evict_emitter_load( dz_service_t * _service, dz_emitter_t ** _emitter, const jpp::object & _data )
{
    dz_emitter_t * emitter;
    if( dz_emitter_create( _service, &emitter, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    float life = _data["life"];

    dz_emitter_set_life( emitter, life );

    jpp::object j_timeline = _data["timeline"];

    for( uint32_t index = 0; index != __DZ_EMITTER_TIMELINE_MAX__; ++index )
    {
        dz_emitter_timeline_type_e timeline_type = (dz_emitter_timeline_type_e)index;

        const char * timeline_type_str = dz_emitter_timeline_type_stringize( timeline_type );

        jpp::object j_timeline_key;
        if( j_timeline.exist( timeline_type_str, &j_timeline_key ) == false )
        {
            continue;
        }

        dz_timeline_key_t * key;
        if( __evict_timeline_key_load( _service, &key, j_timeline_key ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        dz_emitter_set_timeline( emitter, timeline_type, key );
    }

    *_emitter = emitter;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t __evict_affector_load( dz_service_t * _service, dz_affector_t ** _affector, const jpp::object & _data )
{
    dz_affector_t * affector;
    if( dz_affector_create( _service, &affector, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    jpp::object j_timeline = _data["timeline"];

    for( uint32_t index = 0; index != __DZ_AFFECTOR_TIMELINE_MAX__; ++index )
    {
        dz_affector_timeline_type_e timeline_type = (dz_affector_timeline_type_e)index;

        const char * timeline_type_str = dz_affector_timeline_type_stringize( timeline_type );

        jpp::object j_timeline_key;
        if( j_timeline.exist( timeline_type_str, &j_timeline_key ) == false )
        {
            continue;
        }

        dz_timeline_key_t * key;
        if( __evict_timeline_key_load( _service, &key, j_timeline_key ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        dz_affector_set_timeline( affector, timeline_type, key );
    }

    *_affector = affector;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_evict_load( dz_service_t * _service, dz_effect_t ** _effect, const jpp::object & _data )
{
    jpp::object j_material = _data["material"];

    dz_material_t * material;
    if( __evict_material_load( _service, &material, j_material ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    jpp::object j_shape = _data["shape"];

    dz_shape_t * shape;
    if( __evict_shape_load( _service, &shape, j_shape ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    jpp::object j_emitter = _data["emitter"];

    dz_emitter_t * emitter;
    if( __evict_emitter_load( _service, &emitter, j_emitter ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    jpp::object j_affector = _data["affector"];

    dz_affector_t * affector;
    if( __evict_affector_load( _service, &affector, j_affector ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    float life = _data.get( "life", 0.f );

    dz_effect_t * effect;
    if( dz_effect_create( _service, &effect, material, shape, emitter, affector, life, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    *_effect = effect;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////