#include "evict/evict.hpp"

#include "dazzle/dazzle_aux.hpp"

#include <cstdio>
#include <cstring>
#include <vector>

//////////////////////////////////////////////////////////////////////////
typedef std::vector<dz_mesh_vertex_t> dz_evict_mesh_vertex_list_t;
typedef std::vector<dz_uint32_t> dz_evict_mesh_index_list_t;
//////////////////////////////////////////////////////////////////////////
static jpp::object __evict_texture_write( const dz_texture_t * _texture, dz_float_t _random_weight )
{
    jpp::object obj = jpp::make_object();

    dz_float_t u[4];
    dz_float_t v[4];
    dz_texture_get_uv( _texture, u, v );

    obj.set( "u", jpp::make_tuple( u[0], u[1], u[2], u[3] ) );
    obj.set( "v", jpp::make_tuple( v[0], v[1], v[2], v[3] ) );

    dz_float_t width = dz_texture_get_width( _texture );
    dz_float_t height = dz_texture_get_height( _texture );
    obj.set( "size", jpp::make_tuple( width, height ) );

    dz_float_t trim_offset_x;
    dz_float_t trim_offset_y;
    dz_texture_get_trim_offset( _texture, &trim_offset_x, &trim_offset_y );

    obj.set( "trim_offset", jpp::make_tuple( trim_offset_x, trim_offset_y ) );

    dz_float_t trime_width;
    dz_float_t trime_height;
    dz_texture_get_trim_size( _texture, &trime_width, &trime_height );

    obj.set( "trim_size", jpp::make_tuple( trime_width, trime_height ) );

    obj.set( "random_weight", _random_weight );

    dz_float_t sequence_delay = dz_texture_get_sequence_delay( _texture );

    obj.set( "sequence_delay", sequence_delay );

    return obj;
}
//////////////////////////////////////////////////////////////////////////
static jpp::object __evict_atlas_write( const dz_atlas_t * _atlas )
{
    jpp::object obj = jpp::make_object();

    DZ_UNUSED( _atlas );

    return obj;
}
//////////////////////////////////////////////////////////////////////////
static jpp::object __evict_material_write( const dz_material_t * _material )
{
    jpp::object obj = jpp::make_object();

    dz_blend_type_e blend_type = dz_material_get_blend( _material );

    const char * blend_type_str = dz_blend_type_stringize( blend_type );

    obj.set( "blend_type", blend_type_str );

    dz_float_t r;
    dz_float_t g;
    dz_float_t b;
    dz_float_t a;
    dz_material_get_color( _material, &r, &g, &b, &a );

    obj.set( "color", jpp::make_tuple( r, g, b, a ) );

    dz_material_mode_e mode = dz_material_get_mode( _material );

    const char * mode_str = dz_material_mode_stringize( mode );

    obj.set( "mode", mode_str );
    obj.set( "texture_index", dz_material_get_texture_index( _material ) );
    obj.set( "texture_count", dz_material_get_texture_count( _material ) );

    jpp::array array_textures = jpp::make_array();

    dz_uint32_t texture_count = dz_material_get_texture_slot_count( _material );

    for( dz_uint32_t index = 0; index != texture_count; ++index )
    {
        const dz_texture_t * texture;
        dz_material_get_texture( _material, index, &texture );

        dz_float_t random_weight = 1.f;
        dz_material_get_texture_random_weight( _material, index, &random_weight );

        jpp::object obj_texture = __evict_texture_write( texture, random_weight );

        array_textures.push_back( obj_texture );
    }

    obj.set( "textures", array_textures );

    jpp::array passes = jpp::make_array();
    const dz_uint32_t pass_count = dz_material_get_pass_count( _material );
    for( dz_uint32_t index = 0; index != pass_count; ++index )
    {
        dz_material_pass_desc_t pass;
        dz_material_get_pass( _material, index, &pass );
        jpp::object pass_data = jpp::make_object();
        pass_data.set( "technique", pass.technique_id );
        pass_data.set( "blend", (dz_uint32_t)pass.blend );
        pass_data.set( "depth_test", pass.depth_test == DZ_TRUE );
        pass_data.set( "depth_write", pass.depth_write == DZ_TRUE );
        pass_data.set( "depth_compare", (dz_uint32_t)pass.depth_compare );
        pass_data.set( "cull", (dz_uint32_t)pass.cull );
        pass_data.set( "color_mask", pass.color_mask );
        jpp::array uniforms = jpp::make_array();
        for( dz_uint32_t uniformIndex = 0; uniformIndex != pass.uniform_count; ++uniformIndex )
        {
            const dz_uniform_desc_t & uniform = pass.uniforms[uniformIndex];
            jpp::object uniformData = jpp::make_object();
            uniformData.set( "name", uniform.name );
            uniformData.set( "semantic", (dz_uint32_t)uniform.semantic );
            jpp::array values = jpp::make_array();
            for( dz_uint32_t value = 0; value != uniform.value_count; ++value )
            {
                values.push_back( uniform.values[value] );
            }
            uniformData.set( "values", values );
            uniforms.push_back( uniformData );
        }
        pass_data.set( "uniforms", uniforms );
        jpp::array bindings = jpp::make_array();
        for( dz_uint32_t bindingIndex = 0; bindingIndex != pass.texture_binding_count; ++bindingIndex )
        {
            const dz_texture_binding_desc_t & binding = pass.texture_bindings[bindingIndex];
            jpp::object bindingData = jpp::make_object();
            bindingData.set( "uniform", binding.uniform_name );
            bindingData.set( "texture_slot", binding.texture_slot );
            bindingData.set( "min_filter", (dz_uint32_t)binding.min_filter );
            bindingData.set( "mag_filter", (dz_uint32_t)binding.mag_filter );
            bindingData.set( "wrap_u", (dz_uint32_t)binding.wrap_u );
            bindingData.set( "wrap_v", (dz_uint32_t)binding.wrap_v );
            bindings.push_back( bindingData );
        }
        pass_data.set( "texture_bindings", bindings );
        passes.push_back( pass_data );
    }
    obj.set( "passes", passes );

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

    dz_float_t p0;
    dz_float_t p1;
    dz_timeline_interpolate_get_bezier2( _interpolate, &p0, &p1 );

    jpp::object obj_bezier2 = jpp::make_object();

    obj_bezier2.set( "p0", p0 );
    obj_bezier2.set( "p1", p1 );

    obj.set( "bezier2", obj_bezier2 );

    dz_float_t out_tangent;
    dz_float_t in_tangent;
    dz_timeline_interpolate_get_hermite( _interpolate, &out_tangent, &in_tangent );
    obj.set( "hermite", jpp::make_tuple( out_tangent, in_tangent ) );

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

    dz_float_t p = dz_timeline_key_get_p( _key );

    obj.set( "p", p );

    dz_float_t const_value;
    dz_timeline_key_get_const_value( _key, &const_value );

    obj.set( "const_value", const_value );

    dz_float_t randomize_min;
    dz_float_t randomize_max;
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

    dz_transform_t transform;
    dz_shape_get_transform( _shape, &transform );
    dz_vec3_t dimensions;
    dz_shape_get_dimensions( _shape, &dimensions );
    obj.set( "position", jpp::make_tuple( transform.position.x, transform.position.y, transform.position.z ) );
    obj.set( "rotation", jpp::make_tuple( transform.rotation.x, transform.rotation.y, transform.rotation.z, transform.rotation.w ) );
    obj.set( "scale", jpp::make_tuple( transform.scale.x, transform.scale.y, transform.scale.z ) );
    obj.set( "dimensions", jpp::make_tuple( dimensions.x, dimensions.y, dimensions.z ) );
    obj.set( "mesh_id", dz_shape_get_mesh_id( _shape ) );

    jpp::object obj_timeline = jpp::make_object();

    for( dz_uint32_t index = 0; index != __DZ_SHAPE_TIMELINE_MAX__; ++index )
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

    dz_float_t life = dz_emitter_get_life( _emitter );

    obj.set( "life", life );

    jpp::object obj_timeline = jpp::make_object();

    for( dz_uint32_t index = 0; index != __DZ_EMITTER_TIMELINE_MAX__; ++index )
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

    for( dz_uint32_t index = 0; index != __DZ_AFFECTOR_TIMELINE_MAX__; ++index )
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
static const char * __evict_effect_event_type_stringize( dz_effect_event_type_e _type )
{
    switch( _type )
    {
    case DZ_EFFECT_EVENT_EFFECT_START:
        return "effect_start";
    case DZ_EFFECT_EVENT_TIME:
        return "time";
    case DZ_EFFECT_EVENT_LAYER_EMIT_COMPLETE:
        return "layer_emit_complete";
    case DZ_EFFECT_EVENT_LAYER_PARTICLE_COMPLETE:
        return "layer_particle_complete";
    case DZ_EFFECT_EVENT_PARTICLE_DEATH:
        return "particle_death";
    case DZ_EFFECT_EVENT_CUSTOM:
        return "custom";
    case __DZ_EFFECT_EVENT_MAX__:
    default:
        break;
    }

    return "effect_start";
}
//////////////////////////////////////////////////////////////////////////
static dz_effect_event_type_e __evict_effect_event_type_load( const char * _type )
{
    if( strcmp( _type, "effect_start" ) == 0 )
    {
        return DZ_EFFECT_EVENT_EFFECT_START;
    }
    else if( strcmp( _type, "time" ) == 0 )
    {
        return DZ_EFFECT_EVENT_TIME;
    }
    else if( strcmp( _type, "layer_emit_complete" ) == 0 )
    {
        return DZ_EFFECT_EVENT_LAYER_EMIT_COMPLETE;
    }
    else if( strcmp( _type, "layer_particle_complete" ) == 0 )
    {
        return DZ_EFFECT_EVENT_LAYER_PARTICLE_COMPLETE;
    }
    else if( strcmp( _type, "particle_death" ) == 0 )
    {
        return DZ_EFFECT_EVENT_PARTICLE_DEATH;
    }
    else if( strcmp( _type, "custom" ) == 0 )
    {
        return DZ_EFFECT_EVENT_CUSTOM;
    }

    return __DZ_EFFECT_EVENT_MAX__;
}
//////////////////////////////////////////////////////////////////////////
static jpp::object __evict_effect_layer_write( const dz_effect_layer_desc_t * _layer )
{
    jpp::object obj = jpp::make_object();

    obj.set( "material", __evict_material_write( _layer->material ) );
    obj.set( "shape", __evict_shape_write( _layer->shape ) );
    obj.set( "emitter", __evict_emitter_write( _layer->emitter ) );
    obj.set( "affector", __evict_affector_write( _layer->affector ) );
    obj.set( "position", jpp::make_tuple( _layer->x, _layer->y, _layer->z ) );
    obj.set( "angle", _layer->angle );
    obj.set( "rotation", jpp::make_tuple( _layer->rotation.x, _layer->rotation.y, _layer->rotation.z, _layer->rotation.w ) );
    obj.set( "scale", jpp::make_tuple( _layer->scale.x, _layer->scale.y, _layer->scale.z ) );
    obj.set( "particle_mode", (dz_uint32_t)_layer->particle_mode );
    obj.set( "orientation", (dz_uint32_t)_layer->orientation );
    obj.set( "sorting", (dz_uint32_t)_layer->sorting );
    obj.set( "orientation_axis", jpp::make_tuple( _layer->orientation_axis.x, _layer->orientation_axis.y, _layer->orientation_axis.z ) );
    obj.set( "mesh_id", _layer->mesh_id );
    obj.set( "trail_width", _layer->trail_width );
    obj.set( "trail_lifetime", _layer->trail_lifetime );
    obj.set( "life", _layer->life );
    obj.set( "seed", _layer->seed );

    return obj;
}
//////////////////////////////////////////////////////////////////////////
static jpp::object __evict_effect_trigger_write( const dz_effect_trigger_desc_t * _trigger )
{
    jpp::object obj = jpp::make_object();

    obj.set( "event", __evict_effect_event_type_stringize( _trigger->event_type ) );
    obj.set( "source_layer", _trigger->source_layer_index );
    obj.set( "target_layer", _trigger->target_layer_index );
    obj.set( "time", _trigger->time );
    obj.set( "probability", _trigger->probability );
    obj.set( "spawn_count", jpp::make_tuple( _trigger->spawn_count_min, _trigger->spawn_count_max ) );
    obj.set( "delay", jpp::make_tuple( _trigger->delay_min, _trigger->delay_max ) );
    obj.set( "inherit_position", _trigger->inherit_position == DZ_TRUE );
    obj.set( "inherit_angle", _trigger->inherit_angle == DZ_TRUE );
    obj.set( "inherit_velocity", _trigger->inherit_velocity == DZ_TRUE );
    obj.set( "offset", jpp::make_tuple( _trigger->offset_x, _trigger->offset_y ) );
    obj.set( "angle_offset", _trigger->angle_offset );

    return obj;
}
//////////////////////////////////////////////////////////////////////////
static const dz_atlas_t * __evict_effect_find_atlas( const dz_effect_t * _effect )
{
    const dz_uint32_t layer_count = dz_effect_get_layer_count( _effect );

    for( dz_uint32_t index = 0; index != layer_count; ++index )
    {
        dz_effect_layer_desc_t layer;
        dz_effect_get_layer( _effect, index, &layer );

        const dz_atlas_t * atlas = dz_material_get_atlas( layer.material );
        if( atlas != DZ_NULLPTR )
        {
            return atlas;
        }
    }

    return DZ_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
jpp::object dz_evict_write( const dz_effect_t * _effect )
{
    jpp::object obj = jpp::make_object();

    dz_float_t life = dz_effect_get_life( _effect );

    obj.set( "life", life );

    dz_uint32_t seed = dz_effect_get_seed( _effect );

    obj.set( "seed", seed );

    dz_project_profile_t profile;
    dz_effect_get_project_profile( _effect, &profile );
    jpp::object camera = jpp::make_object();
    camera.set( "projection", profile.projection == DZ_PROJECTION_ORTHOGRAPHIC ? "orthographic" : "perspective" );
    camera.set( "position", jpp::make_tuple( profile.position.x, profile.position.y, profile.position.z ) );
    camera.set( "forward", jpp::make_tuple( profile.forward.x, profile.forward.y, profile.forward.z ) );
    camera.set( "up", jpp::make_tuple( profile.up.x, profile.up.y, profile.up.z ) );
    camera.set( "field_of_view", profile.field_of_view );
    camera.set( "orthographic_height", profile.orthographic_height );
    camera.set( "near", profile.near_plane );
    camera.set( "far", profile.far_plane );
    obj.set( "camera", camera );

    jpp::array meshes = jpp::make_array();
    const dz_uint32_t mesh_count = dz_effect_get_mesh_count( _effect );
    for( dz_uint32_t mesh_index = 0; mesh_index != mesh_count; ++mesh_index )
    {
        dz_mesh_desc_t mesh;
        dz_effect_get_mesh_at( _effect, mesh_index, &mesh );
        jpp::object mesh_data = jpp::make_object();
        mesh_data.set( "id", mesh.id );
        jpp::array vertices = jpp::make_array();
        for( dz_uint32_t vertex_index = 0; vertex_index != mesh.vertex_count; ++vertex_index )
        {
            const dz_mesh_vertex_t & vertex = mesh.vertices[vertex_index];
            vertices.push_back( jpp::make_tuple( vertex.position.x, vertex.position.y, vertex.position.z, vertex.normal.x, vertex.normal.y, vertex.normal.z, vertex.tangent.x,
                                                 vertex.tangent.y, vertex.tangent.z, vertex.tangent.w, vertex.uv0.x, vertex.uv0.y, vertex.uv1.x, vertex.uv1.y ) );
        }
        jpp::array indices = jpp::make_array();
        for( dz_uint32_t index = 0; index != mesh.index_count; ++index )
        {
            indices.push_back( mesh.indices[index] );
        }
        mesh_data.set( "vertices", vertices );
        mesh_data.set( "indices", indices );
        meshes.push_back( mesh_data );
    }
    obj.set( "meshes", meshes );

    const dz_atlas_t * atlas = dz_effect_get_atlas( _effect );

    if( atlas == DZ_NULLPTR )
    {
        atlas = __evict_effect_find_atlas( _effect );
    }
    if( atlas != DZ_NULLPTR )
    {
        obj.set( "atlas", __evict_atlas_write( atlas ) );
    }

    jpp::array layers = jpp::make_array();

    const dz_uint32_t layer_count = dz_effect_get_layer_count( _effect );

    for( dz_uint32_t index = 0; index != layer_count; ++index )
    {
        dz_effect_layer_desc_t layer;
        dz_effect_get_layer( _effect, index, &layer );

        layers.push_back( __evict_effect_layer_write( &layer ) );
    }

    obj.set( "layers", layers );

    jpp::array triggers = jpp::make_array();

    const dz_uint32_t trigger_count = dz_effect_get_trigger_count( _effect );

    for( dz_uint32_t index = 0; index != trigger_count; ++index )
    {
        dz_effect_trigger_desc_t trigger;
        dz_effect_get_trigger( _effect, index, &trigger );

        triggers.push_back( __evict_effect_trigger_write( &trigger ) );
    }

    obj.set( "triggers", triggers );

    jpp::array physics = jpp::make_array();
    const dz_uint32_t physics_count = dz_effect_get_physics_object_count( _effect );
    for( dz_uint32_t index = 0; index != physics_count; ++index )
    {
        dz_physics_object_desc_t object;
        dz_effect_get_physics_object( _effect, index, &object );
        jpp::object value = jpp::make_object();
        value.set( "id", object.id );
        value.set( "mesh_id", object.mesh_id );
        value.set( "type", (dz_uint32_t)object.type );
        value.set( "position", jpp::make_tuple( object.transform.position.x, object.transform.position.y, object.transform.position.z ) );
        value.set( "rotation", jpp::make_tuple( object.transform.rotation.x, object.transform.rotation.y, object.transform.rotation.z, object.transform.rotation.w ) );
        value.set( "scale", jpp::make_tuple( object.transform.scale.x, object.transform.scale.y, object.transform.scale.z ) );
        value.set( "direction", jpp::make_tuple( object.direction.x, object.direction.y, object.direction.z ) );
        value.set( "half_extents", jpp::make_tuple( object.half_extents.x, object.half_extents.y, object.half_extents.z ) );
        value.set( "radius", object.radius );
        value.set( "strength", object.strength );
        value.set( "falloff", object.falloff );
        value.set( "turbulence", object.turbulence );
        value.set( "restitution", object.restitution );
        value.set( "friction", object.friction );
        value.set( "response", (dz_uint32_t)object.response );
        physics.push_back( value );
    }
    obj.set( "physics", physics );

    return obj;
}
//////////////////////////////////////////////////////////////////////////
static void __evict_texture_load( dz_service_t * _service, dz_texture_t ** _texture, dz_float_t * const _random_weight, const jpp::object & _data )
{
    dz_texture_t * texture;
    dz_texture_create( _service, &texture, DZ_NULLPTR );

    jpp::array j_u = _data["u"];
    jpp::array j_v = _data["v"];

    dz_float_t u[4] = {j_u[0], j_u[1], j_u[2], j_u[3]};
    dz_float_t v[4] = {j_v[0], j_v[1], j_v[2], j_v[3]};

    dz_texture_set_uv( texture, u, v );

    jpp::array j_size = _data["size"];

    dz_float_t width = j_size[0];
    dz_float_t height = j_size[1];

    dz_texture_set_width( texture, width );
    dz_texture_set_height( texture, height );

    jpp::array j_trim_offset = _data["trim_offset"];

    dz_float_t trim_offset_x = j_trim_offset[0];
    dz_float_t trim_offset_y = j_trim_offset[1];

    dz_texture_set_trim_offset( texture, trim_offset_x, trim_offset_y );

    jpp::array j_trim_size = _data["trim_size"];

    dz_float_t trime_width = j_trim_size[0];
    dz_float_t trime_height = j_trim_size[1];

    dz_texture_set_trim_size( texture, trime_width, trime_height );

    dz_float_t random_weight = _data["random_weight"];

    *_random_weight = DZ_MAX( random_weight, 0.f );

    dz_float_t sequence_delay = _data["sequence_delay"];

    dz_texture_set_sequence_delay( texture, sequence_delay );

    *_texture = texture;

}
//////////////////////////////////////////////////////////////////////////
static void __evict_atlas_load( dz_service_t * _service, dz_atlas_t ** _atlas, const jpp::object & _data )
{
    DZ_UNUSED( _data );

    dz_atlas_t * atlas;
    dz_atlas_create( _service, &atlas, DZ_NULLPTR, DZ_NULLPTR );

    *_atlas = atlas;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __evict_material_load( dz_service_t * _service, dz_material_t ** _material, const dz_atlas_t * _atlas, const jpp::object & _data )
{
    dz_material_t * material;
    dz_material_create( _service, &material, DZ_NULLPTR );

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

    jpp::array j_color = _data["color"];

    dz_material_set_color( material, j_color[0], j_color[1], j_color[2], j_color[3] );

    const char * j_mode = _data["mode"];

    dz_material_mode_e mode;
    if( strcmp( j_mode, "solid" ) == 0 )
    {
        mode = DZ_MATERIAL_MODE_SOLID;
    }
    else if( strcmp( j_mode, "texture" ) == 0 )
    {
        mode = DZ_MATERIAL_MODE_TEXTURE;
    }
    else if( strcmp( j_mode, "sequence" ) == 0 )
    {
        mode = DZ_MATERIAL_MODE_SEQUENCE;
    }
    else
    {
        return DZ_FAILURE;
    }

    dz_material_set_mode( material, mode );
    dz_material_set_atlas( material, _atlas );
    dz_material_set_texture_index( material, _data.get( "texture_index", 0 ) );
    dz_material_set_texture_count( material, _data.get( "texture_count", 1 ) );

    jpp::array array_textures = _data.get( "textures", jpp::make_array() );

    for( const jpp::object & texture_data : array_textures )
    {
        dz_texture_t * texture;
        dz_float_t random_weight;
        __evict_texture_load( _service, &texture, &random_weight, texture_data );

        if( dz_material_add_texture( material, texture ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        const dz_uint32_t texture_index = dz_material_get_texture_slot_count( material ) - 1;
        if( dz_material_set_texture_random_weight( material, texture_index, random_weight ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }
    }

    jpp::array passes = _data.get( "passes", jpp::make_array() );
    dz_uint32_t pass_index = 0;
    for( const jpp::object & pass_data : passes )
    {
        dz_material_pass_desc_t pass;
        memset( &pass, 0, sizeof( pass ) );
        const char * technique = pass_data["technique"];
        snprintf( pass.technique_id, sizeof( pass.technique_id ), "%s", technique );
        pass.blend = (dz_blend_type_e)pass_data.get( "blend", (dz_uint32_t)DZ_BLEND_NORMAL );
        pass.depth_test = pass_data.get( "depth_test", false ) ? DZ_TRUE : DZ_FALSE;
        pass.depth_write = pass_data.get( "depth_write", false ) ? DZ_TRUE : DZ_FALSE;
        pass.depth_compare = (dz_depth_compare_e)pass_data.get( "depth_compare", (dz_uint32_t)DZ_DEPTH_LESS_EQUAL );
        pass.cull = (dz_cull_mode_e)pass_data.get( "cull", (dz_uint32_t)DZ_CULL_NONE );
        pass.color_mask = (dz_uint8_t)pass_data.get( "color_mask", 0x0f );

        jpp::array uniforms = pass_data.get( "uniforms", jpp::make_array() );
        pass.uniform_count = (dz_uint32_t)uniforms.size();
        for( dz_uint32_t uniformIndex = 0; uniformIndex != pass.uniform_count; ++uniformIndex )
        {
            jpp::object uniformData = uniforms[uniformIndex];
            dz_uniform_desc_t & uniform = pass.uniforms[uniformIndex];
            snprintf( uniform.name, sizeof( uniform.name ), "%s", (const char *)uniformData["name"] );
            uniform.semantic = (dz_uniform_semantic_e)uniformData.get( "semantic", (dz_uint32_t)DZ_UNIFORM_CUSTOM );
            jpp::array values = uniformData.get( "values", jpp::make_array() );
            uniform.value_count = (dz_uint32_t)values.size();
            for( dz_uint32_t value = 0; value != uniform.value_count; ++value )
            {
                uniform.values[value] = values[value];
            }
        }

        jpp::array bindings = pass_data.get( "texture_bindings", jpp::make_array() );
        pass.texture_binding_count = (dz_uint32_t)bindings.size();
        for( dz_uint32_t bindingIndex = 0; bindingIndex != pass.texture_binding_count; ++bindingIndex )
        {
            jpp::object bindingData = bindings[bindingIndex];
            dz_texture_binding_desc_t & binding = pass.texture_bindings[bindingIndex];
            snprintf( binding.uniform_name, sizeof( binding.uniform_name ), "%s", (const char *)bindingData["uniform"] );
            binding.texture_slot = bindingData.get( "texture_slot", 0U );
            binding.min_filter = (dz_sampler_filter_e)bindingData.get( "min_filter", (dz_uint32_t)DZ_SAMPLER_LINEAR );
            binding.mag_filter = (dz_sampler_filter_e)bindingData.get( "mag_filter", (dz_uint32_t)DZ_SAMPLER_LINEAR );
            binding.wrap_u = (dz_sampler_wrap_e)bindingData.get( "wrap_u", (dz_uint32_t)DZ_SAMPLER_CLAMP );
            binding.wrap_v = (dz_sampler_wrap_e)bindingData.get( "wrap_v", (dz_uint32_t)DZ_SAMPLER_CLAMP );
        }

        if( pass_index == 0 )
        {
            dz_material_set_pass( material, 0, &pass );
        }
        else
        {
            dz_material_add_pass( material, &pass, DZ_NULLPTR );
        }
        ++pass_index;
    }

    *_material = material;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_timeline_interpolate_type_e __load_timeline_interpolate_type( const char * _type )
{
    if( strcmp( _type, "step" ) == 0 )
    {
        return DZ_TIMELINE_INTERPOLATE_STEP;
    }
    else if( strcmp( _type, "linear" ) == 0 )
    {
        return DZ_TIMELINE_INTERPOLATE_LINEAR;
    }
    else if( strcmp( _type, "bezier2" ) == 0 )
    {
        return DZ_TIMELINE_INTERPOLATE_BEZIER2;
    }
    else if( strcmp( _type, "hermite" ) == 0 )
    {
        return DZ_TIMELINE_INTERPOLATE_HERMITE;
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
    dz_timeline_interpolate_create( _service, &interpolate, timeline_interpolate_type, DZ_NULLPTR );

    jpp::object j_bezier2 = _data["bezier2"];

    dz_float_t p0 = j_bezier2["p0"];
    dz_float_t p1 = j_bezier2["p1"];

    dz_timeline_interpolate_set_bezier2( interpolate, p0, p1 );

    jpp::array hermite = _data.get( "hermite", jpp::make_tuple( 0.f, 0.f ) );
    dz_timeline_interpolate_set_hermite( interpolate, hermite[0], hermite[1] );

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
    dz_float_t p = _data["p"];

    const char * j_key_type = _data["type"];

    dz_timeline_key_type_e timeline_key_type = __load_timeline_key_type( j_key_type );

    if( timeline_key_type == __DZ_TIMELINE_KEY_MAX__ )
    {
        return DZ_FAILURE;
    }

    dz_timeline_key_t * key;
    dz_timeline_key_create( _service, &key, p, timeline_key_type, DZ_NULLPTR );

    dz_float_t const_value = _data["const_value"];
    dz_timeline_key_set_const_value( key, const_value );

    dz_float_t randomize_min = _data["randomize_min"];
    dz_float_t randomize_max = _data["randomize_max"];

    dz_timeline_key_set_randomize_min_max( key, randomize_min, randomize_max );

    jpp::object j_interpolate;
    if( _data.exist( "interpolate", &j_interpolate ) == true )
    {
        dz_timeline_interpolate_t * interpolate;
        if( __evict_timeline_interpolate_load( _service, &interpolate, j_interpolate ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        dz_timeline_key_set_interpolate( key, interpolate );
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
    else if( strcmp( _type, "sphere" ) == 0 )
    {
        return DZ_SHAPE_SPHERE;
    }
    else if( strcmp( _type, "box" ) == 0 )
    {
        return DZ_SHAPE_BOX;
    }
    else if( strcmp( _type, "cone" ) == 0 )
    {
        return DZ_SHAPE_CONE;
    }
    else if( strcmp( _type, "cylinder" ) == 0 )
    {
        return DZ_SHAPE_CYLINDER;
    }
    else if( strcmp( _type, "mesh_surface" ) == 0 )
    {
        return DZ_SHAPE_MESH_SURFACE;
    }
    else if( strcmp( _type, "mesh_volume" ) == 0 )
    {
        return DZ_SHAPE_MESH_VOLUME;
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
    dz_shape_create( _service, &shape, shape_type, DZ_NULLPTR );

    dz_transform_t transform;
    jpp::array position = _data.get( "position", jpp::make_tuple( 0.f, 0.f, 0.f ) );
    jpp::array rotation = _data.get( "rotation", jpp::make_tuple( 0.f, 0.f, 0.f, 1.f ) );
    jpp::array scale = _data.get( "scale", jpp::make_tuple( 1.f, 1.f, 1.f ) );
    transform.position.x = position[0];
    transform.position.y = position[1];
    transform.position.z = position[2];
    transform.rotation.x = rotation[0];
    transform.rotation.y = rotation[1];
    transform.rotation.z = rotation[2];
    transform.rotation.w = rotation[3];
    transform.scale.x = scale[0];
    transform.scale.y = scale[1];
    transform.scale.z = scale[2];
    dz_shape_set_transform( shape, &transform );
    jpp::array dimensions_data = _data.get( "dimensions", jpp::make_tuple( 1.f, 1.f, 1.f ) );
    dz_vec3_t dimensions = { dimensions_data[0], dimensions_data[1], dimensions_data[2] };
    dz_shape_set_dimensions( shape, &dimensions );
    dz_shape_set_mesh_id( shape, _data.get( "mesh_id", DZ_RESOURCE_ID_NONE ) );

    jpp::object j_timeline = _data["timeline"];

    for( dz_uint32_t index = 0; index != __DZ_SHAPE_TIMELINE_MAX__; ++index )
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
    dz_emitter_create( _service, &emitter, DZ_NULLPTR );

    dz_float_t life = _data["life"];

    dz_emitter_set_life( emitter, life );

    jpp::object j_timeline = _data["timeline"];

    for( dz_uint32_t index = 0; index != __DZ_EMITTER_TIMELINE_MAX__; ++index )
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
    dz_affector_create( _service, &affector, DZ_NULLPTR );

    jpp::object j_timeline = _data["timeline"];

    for( dz_uint32_t index = 0; index != __DZ_AFFECTOR_TIMELINE_MAX__; ++index )
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
static dz_result_t __evict_effect_layer_load( dz_service_t * _service, dz_effect_layer_desc_t * const _layer, const dz_atlas_t * _atlas, const jpp::object & _data )
{
    jpp::object j_material = _data["material"];

    dz_material_t * material;
    if( __evict_material_load( _service, &material, _atlas, j_material ) == DZ_FAILURE )
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

    _layer->material = material;
    _layer->shape = shape;
    _layer->emitter = emitter;
    _layer->affector = affector;

    jpp::array j_position = _data.get( "position", jpp::make_tuple( 0.f, 0.f, 0.f ) );
    _layer->x = j_position[0];
    _layer->y = j_position[1];
    _layer->z = j_position[2];
    _layer->angle = _data.get( "angle", 0.f );
    jpp::array rotation = _data.get( "rotation", jpp::make_tuple( 0.f, 0.f, 0.f, 1.f ) );
    _layer->rotation.x = rotation[0];
    _layer->rotation.y = rotation[1];
    _layer->rotation.z = rotation[2];
    _layer->rotation.w = rotation[3];
    jpp::array scale = _data.get( "scale", jpp::make_tuple( 1.f, 1.f, 1.f ) );
    _layer->scale.x = scale[0];
    _layer->scale.y = scale[1];
    _layer->scale.z = scale[2];
    _layer->particle_mode = (dz_particle_mode_e)_data.get( "particle_mode", (dz_uint32_t)DZ_PARTICLE_MODE_SPRITE );
    _layer->orientation = (dz_particle_orientation_e)_data.get( "orientation", (dz_uint32_t)DZ_PARTICLE_ORIENTATION_CAMERA );
    _layer->sorting = (dz_particle_sort_e)_data.get( "sorting", (dz_uint32_t)DZ_PARTICLE_SORT_NONE );
    jpp::array orientation_axis = _data.get( "orientation_axis", jpp::make_tuple( 0.f, 1.f, 0.f ) );
    _layer->orientation_axis.x = orientation_axis[0];
    _layer->orientation_axis.y = orientation_axis[1];
    _layer->orientation_axis.z = orientation_axis[2];
    _layer->mesh_id = _data.get( "mesh_id", DZ_EFFECT_LAYER_NONE );
    _layer->trail_width = _data.get( "trail_width", 1.f );
    _layer->trail_lifetime = _data.get( "trail_lifetime", 0.5f );
    _layer->life = _data.get( "life", 0.f );
    _layer->seed = _data.get( "seed", 0 );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __evict_effect_trigger_load( dz_effect_trigger_desc_t * const _trigger, const jpp::object & _data )
{
    const char * event_type = _data["event"];

    _trigger->event_type = __evict_effect_event_type_load( event_type );

    if( _trigger->event_type == __DZ_EFFECT_EVENT_MAX__ )
    {
        return DZ_FAILURE;
    }

    _trigger->source_layer_index = _data.get( "source_layer", DZ_EFFECT_LAYER_NONE );
    _trigger->target_layer_index = _data["target_layer"];
    _trigger->time = _data.get( "time", 0.f );
    _trigger->probability = _data.get( "probability", 1.f );

    jpp::array spawn_count = _data.get( "spawn_count", jpp::make_tuple( 1, 1 ) );
    _trigger->spawn_count_min = spawn_count[0];
    _trigger->spawn_count_max = spawn_count[1];

    jpp::array delay = _data.get( "delay", jpp::make_tuple( 0.f, 0.f ) );
    _trigger->delay_min = delay[0];
    _trigger->delay_max = delay[1];

    _trigger->inherit_position = _data.get( "inherit_position", false ) == true ? DZ_TRUE : DZ_FALSE;
    _trigger->inherit_angle = _data.get( "inherit_angle", false ) == true ? DZ_TRUE : DZ_FALSE;
    _trigger->inherit_velocity = _data.get( "inherit_velocity", false ) == true ? DZ_TRUE : DZ_FALSE;

    jpp::array offset = _data.get( "offset", jpp::make_tuple( 0.f, 0.f ) );
    _trigger->offset_x = offset[0];
    _trigger->offset_y = offset[1];
    _trigger->angle_offset = _data.get( "angle_offset", 0.f );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_evict_load( dz_service_t * _service, dz_effect_t ** _effect, const jpp::object & _data )
{
    dz_atlas_t * atlas = DZ_NULLPTR;

    jpp::object j_atlas;
    if( _data.exist( "atlas", &j_atlas ) == true )
    {
        __evict_atlas_load( _service, &atlas, j_atlas );
    }

    dz_float_t life = _data.get( "life", 0.f );

    dz_uint32_t seed = _data.get( "seed", 0 );

    jpp::object camera;
    if( _data.exist( "camera", &camera ) == false )
    {
        return DZ_FAILURE_INVALID_VERSION;
    }

    const char * projection = camera["projection"];
    dz_project_profile_t profile;
    if( strcmp( projection, "orthographic" ) == 0 )
    {
        dz_project_profile_default( &profile, DZ_PROJECTION_ORTHOGRAPHIC );
    }
    else if( strcmp( projection, "perspective" ) == 0 )
    {
        dz_project_profile_default( &profile, DZ_PROJECTION_PERSPECTIVE );
    }
    else
    {
        return DZ_FAILURE_INVALID_DATA;
    }

    jpp::array camera_position = camera["position"];
    jpp::array camera_forward = camera["forward"];
    jpp::array camera_up = camera["up"];
    profile.position.x = camera_position[0];
    profile.position.y = camera_position[1];
    profile.position.z = camera_position[2];
    profile.forward.x = camera_forward[0];
    profile.forward.y = camera_forward[1];
    profile.forward.z = camera_forward[2];
    profile.up.x = camera_up[0];
    profile.up.y = camera_up[1];
    profile.up.z = camera_up[2];
    profile.field_of_view = camera.get( "field_of_view", profile.field_of_view );
    profile.orthographic_height = camera.get( "orthographic_height", profile.orthographic_height );
    profile.near_plane = camera.get( "near", profile.near_plane );
    profile.far_plane = camera.get( "far", profile.far_plane );

    dz_effect_t * effect;
    dz_effect_create_with_profile( _service, &effect, &profile, life, seed, DZ_NULLPTR );

    dz_effect_set_atlas( effect, atlas );

    jpp::array j_meshes = _data.get( "meshes", jpp::make_array() );
    for( const jpp::object & mesh_data : j_meshes )
    {
        jpp::array vertex_data = mesh_data["vertices"];
        jpp::array index_data = mesh_data["indices"];
        if( vertex_data.size() < 3 || index_data.size() < 3 || index_data.size() % 3 != 0 )
        {
            dz_effect_destroy( _service, effect );
            return DZ_FAILURE_INVALID_DATA;
        }

        dz_evict_mesh_vertex_list_t vertices( vertex_data.size() );
        dz_evict_mesh_index_list_t indices( index_data.size() );
        for( jpp::array::size_type index = 0; index != vertex_data.size(); ++index )
        {
            jpp::array values = vertex_data[index];
            if( values.size() != 14 )
            {
                dz_effect_destroy( _service, effect );
                return DZ_FAILURE_INVALID_DATA;
            }
            dz_mesh_vertex_t & vertex = vertices[index];
            vertex.position = { (dz_float_t)values[0], (dz_float_t)values[1], (dz_float_t)values[2] };
            vertex.normal = { (dz_float_t)values[3], (dz_float_t)values[4], (dz_float_t)values[5] };
            vertex.tangent = { (dz_float_t)values[6], (dz_float_t)values[7], (dz_float_t)values[8], (dz_float_t)values[9] };
            vertex.uv0 = { (dz_float_t)values[10], (dz_float_t)values[11] };
            vertex.uv1 = { (dz_float_t)values[12], (dz_float_t)values[13] };
        }
        for( jpp::array::size_type index = 0; index != index_data.size(); ++index )
        {
            indices[index] = (dz_uint32_t)index_data[index];
        }

        dz_mesh_desc_t mesh;
        memset( &mesh, 0, sizeof( mesh ) );
        mesh.id = mesh_data["id"];
        mesh.vertices = vertices.data();
        mesh.vertex_count = (dz_uint32_t)vertices.size();
        mesh.indices = indices.data();
        mesh.index_count = (dz_uint32_t)indices.size();
        dz_effect_add_mesh( _service, effect, &mesh );
    }

    jpp::array j_layers = _data["layers"];

    for( const jpp::object & j_layer : j_layers )
    {
        dz_effect_layer_desc_t layer;
        dz_effect_layer_desc_default( &layer );
        if( __evict_effect_layer_load( _service, &layer, atlas, j_layer ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        dz_effect_add_layer( effect, &layer, DZ_NULLPTR );
    }

    jpp::array j_triggers = _data.get( "triggers", jpp::make_array() );

    for( const jpp::object & j_trigger : j_triggers )
    {
        dz_effect_trigger_desc_t trigger;
        if( __evict_effect_trigger_load( &trigger, j_trigger ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        dz_effect_add_trigger( effect, &trigger, DZ_NULLPTR );
    }

    jpp::array physics = _data.get( "physics", jpp::make_array() );
    for( const jpp::object & value : physics )
    {
        dz_physics_object_desc_t object;
        memset( &object, 0, sizeof( object ) );
        object.id = value["id"];
        object.mesh_id = value.get( "mesh_id", DZ_RESOURCE_ID_NONE );
        object.type = (dz_physics_object_type_e)(dz_uint32_t)value["type"];
        jpp::array position = value.get( "position", jpp::make_tuple( 0.f, 0.f, 0.f ) );
        jpp::array rotation = value.get( "rotation", jpp::make_tuple( 0.f, 0.f, 0.f, 1.f ) );
        jpp::array scale = value.get( "scale", jpp::make_tuple( 1.f, 1.f, 1.f ) );
        jpp::array direction = value.get( "direction", jpp::make_tuple( 0.f, 0.f, 0.f ) );
        jpp::array half_extents = value.get( "half_extents", jpp::make_tuple( 0.f, 0.f, 0.f ) );
        object.transform.position = { (dz_float_t)position[0], (dz_float_t)position[1], (dz_float_t)position[2] };
        object.transform.rotation = { (dz_float_t)rotation[0], (dz_float_t)rotation[1], (dz_float_t)rotation[2], (dz_float_t)rotation[3] };
        object.transform.scale = { (dz_float_t)scale[0], (dz_float_t)scale[1], (dz_float_t)scale[2] };
        object.direction = { (dz_float_t)direction[0], (dz_float_t)direction[1], (dz_float_t)direction[2] };
        object.half_extents = { (dz_float_t)half_extents[0], (dz_float_t)half_extents[1], (dz_float_t)half_extents[2] };
        object.radius = value.get( "radius", 0.f );
        object.strength = value.get( "strength", 0.f );
        object.falloff = value.get( "falloff", 0.f );
        object.turbulence = value.get( "turbulence", 0.f );
        object.restitution = value.get( "restitution", 0.f );
        object.friction = value.get( "friction", 0.f );
        object.response = (dz_collision_response_e)value.get( "response", (dz_uint32_t)DZ_COLLISION_BOUNCE );
        dz_effect_add_physics_object( effect, &object, DZ_NULLPTR );
    }

    *_effect = effect;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
