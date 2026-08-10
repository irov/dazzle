#include "test_support.h"

#include "dazzle/dazzle_read.h"
#include "dazzle/dazzle_write.h"

#include <string.h>

static dz_result_t legacy_write_callback( const void * _data, dz_size_t _size, dz_userdata_t _ud )
{
    DZ_UNUSED( _data );
    DZ_UNUSED( _size );
    DZ_UNUSED( _ud );
    return DZ_SUCCESSFUL;
}

static dz_result_t legacy_read_callback( void * _data, dz_size_t _size, dz_userdata_t _ud )
{
    DZ_UNUSED( _data );
    DZ_UNUSED( _size );
    DZ_UNUSED( _ud );
    return DZ_SUCCESSFUL;
}

static dz_result_t failing_read_callback( void * _data, dz_size_t _size, dz_userdata_t _ud )
{
    DZ_UNUSED( _data );
    DZ_UNUSED( _size );
    DZ_UNUSED( _ud );
    return DZ_FAILURE;
}

typedef struct test_read_stream_t
{
    const dz_uint8_t * data;
    dz_size_t position;
} test_read_stream_t;

static dz_result_t memory_read_callback( void * _data, dz_size_t _size, dz_userdata_t _ud )
{
    test_read_stream_t * stream = (test_read_stream_t *)_ud;
    memcpy( _data, stream->data + stream->position, _size );
    stream->position += _size;
    return DZ_SUCCESSFUL;
}

static dz_result_t read_effect_bytes( const dz_service_t * _service, dz_effect_t ** _effect, const void * _data, dz_size_t _size )
{
    DZ_UNUSED( _size );
    test_read_stream_t stream = { (const dz_uint8_t *)_data, 0 };
    return dz_effect_read( _service, _effect, &memory_read_callback, &stream );
}

static dz_result_t make_serialized_effect( const dz_service_t * service, dz_effect_t ** effect )
{
    dz_project_profile_t profile;
    dz_project_profile_default( &profile, DZ_PROJECTION_PERSPECTIVE );
    dz_effect_create_with_profile( service, effect, &profile, 3.f, 1234U, DZ_NULLPTR );

    const dz_mesh_vertex_t mesh_vertices[4] = { { { 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, { 1.f, 0.f, 0.f, 1.f }, { 0.f, 0.f }, { 0.f, 0.f } },
                                                { { 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, { 1.f, 0.f, 0.f, 1.f }, { 1.f, 0.f }, { 1.f, 0.f } },
                                                { { 0.f, 1.f, 0.f }, { 0.f, 1.f, 0.f }, { 1.f, 0.f, 0.f, 1.f }, { 0.f, 1.f }, { 0.f, 1.f } },
                                                { { 0.f, 0.f, 1.f }, { 0.f, 1.f, 0.f }, { 1.f, 0.f, 0.f, 1.f }, { 1.f, 1.f }, { 1.f, 1.f } } };
    const dz_uint32_t mesh_indices[12] = { 0, 2, 1, 0, 1, 3, 0, 3, 2, 1, 2, 3 };
    dz_mesh_desc_t mesh;
    memset( &mesh, 0, sizeof( mesh ) );
    mesh.id = 7U;
    mesh.vertices = mesh_vertices;
    mesh.vertex_count = 4U;
    mesh.indices = mesh_indices;
    mesh.index_count = 12U;
    dz_effect_add_mesh( service, *effect, &mesh );

    dz_material_t * material;
    dz_shape_t * shape;
    dz_emitter_t * emitter;
    dz_affector_t * affector;
    dz_material_create( service, &material, DZ_NULLPTR );
    dz_shape_create( service, &shape, DZ_SHAPE_BOX, DZ_NULLPTR );
    dz_emitter_create( service, &emitter, DZ_NULLPTR );
    dz_affector_create( service, &affector, DZ_NULLPTR );

    dz_material_pass_desc_t pass;
    memset( &pass, 0, sizeof( pass ) );
    memcpy( pass.technique_id, "test.second", sizeof( "test.second" ) );
    pass.blend = DZ_BLEND_ADD;
    pass.depth_test = DZ_TRUE;
    pass.depth_compare = DZ_DEPTH_LESS_EQUAL;
    pass.color_mask = 0x0f;
    pass.uniform_count = 2U;
    memcpy( pass.uniforms[0].name, "uTint", sizeof( "uTint" ) );
    pass.uniforms[0].semantic = DZ_UNIFORM_CUSTOM;
    pass.uniforms[0].value_count = 4U;
    pass.uniforms[0].values[0] = 1.f;
    pass.uniforms[0].values[1] = 0.5f;
    pass.uniforms[0].values[2] = 0.25f;
    pass.uniforms[0].values[3] = 1.f;
    memcpy( pass.uniforms[1].name, "uProjectTime", sizeof( "uProjectTime" ) );
    pass.uniforms[1].semantic = DZ_UNIFORM_TIME;
    pass.texture_binding_count = 1U;
    memcpy( pass.texture_bindings[0].uniform_name, "uTextureRGB", sizeof( "uTextureRGB" ) );
    pass.texture_bindings[0].min_filter = pass.texture_bindings[0].mag_filter = DZ_SAMPLER_LINEAR;
    pass.texture_bindings[0].wrap_u = pass.texture_bindings[0].wrap_v = DZ_SAMPLER_CLAMP;
    dz_material_add_pass( material, &pass, DZ_NULLPTR );

    dz_timeline_key_t * key0;
    dz_timeline_key_t * key1;
    dz_timeline_interpolate_t * interpolation;
    dz_timeline_key_create( service, &key0, 0.f, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR );
    dz_timeline_key_create( service, &key1, 1.f, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR );
    dz_timeline_interpolate_create( service, &interpolation, DZ_TIMELINE_INTERPOLATE_HERMITE, DZ_NULLPTR );
    dz_timeline_key_set_const_value( key0, 1.f );
    dz_timeline_key_set_const_value( key1, 2.f );
    dz_timeline_interpolate_set_key( interpolation, key1 );
    dz_timeline_interpolate_set_hermite( interpolation, 0.5f, -0.25f );
    dz_timeline_key_set_interpolate( key0, interpolation );
    dz_affector_set_timeline( affector, DZ_AFFECTOR_TIMELINE_SCALE, key0 );

    dz_effect_layer_desc_t layer;
    dz_effect_layer_desc_default( &layer );
    layer.material = material;
    layer.shape = shape;
    layer.emitter = emitter;
    layer.affector = affector;
    layer.life = 3.f;
    layer.z = 4.f;
    layer.particle_mode = DZ_PARTICLE_MODE_MESH;
    layer.mesh_id = 7U;
    layer.orientation = DZ_PARTICLE_ORIENTATION_WORLD;
    dz_effect_add_layer( *effect, &layer, DZ_NULLPTR );

    dz_physics_object_desc_t object;
    memset( &object, 0, sizeof( object ) );
    object.id = 42U;
    object.mesh_id = 7U;
    object.type = DZ_PHYSICS_MESH;
    object.transform.rotation.w = 1.f;
    object.transform.scale.x = object.transform.scale.y = object.transform.scale.z = 1.f;
    object.radius = 2.f;
    object.restitution = 0.5f;
    object.friction = 0.2f;
    object.response = DZ_COLLISION_BOUNCE;
    dz_effect_add_physics_object( *effect, &object, DZ_NULLPTR );

    return DZ_SUCCESSFUL;
}

int main( void )
{
    dz_test_memory_t memory = { 0, 0, 0 };
    dz_service_t * service;
    dz_test_service_create( &service, &memory );

    dz_effect_t * source;
    DZ_TEST_CHECK( make_serialized_effect( service, &source ) == DZ_SUCCESSFUL );

    dz_effect_t * unchanged = source;
    DZ_TEST_CHECK( dz_effect_read( service, &unchanged, &failing_read_callback, DZ_NULLPTR ) == DZ_FAILURE );
    DZ_TEST_CHECK( unchanged == source );

    dz_effect_read_status_e legacy_status = DZ_EFFECT_LOAD_STATUS_SUCCESSFUL;
    DZ_TEST_CHECK( dz_header_write( &legacy_write_callback, DZ_NULLPTR ) == DZ_FAILURE_UNSUPPORTED );
    DZ_TEST_CHECK( dz_effect_write( source, &legacy_write_callback, DZ_NULLPTR ) == DZ_FAILURE_UNSUPPORTED );
    DZ_TEST_CHECK( dz_header_read( &legacy_read_callback, DZ_NULLPTR, &legacy_status ) == DZ_FAILURE_UNSUPPORTED );
    DZ_TEST_CHECK( legacy_status == DZ_EFFECT_LOAD_STATUS_INVALID_VERSION );
    dz_size_t size = 0;
    DZ_TEST_CHECK( dz_effect_write_memory( source, DZ_NULLPTR, 0, &size ) == DZ_FAILURE_BUFFER_TOO_SMALL && size > 40U );
    dz_uint8_t * data = (dz_uint8_t *)malloc( size );
    DZ_TEST_CHECK( dz_effect_write_memory( source, data, size, &size ) == DZ_SUCCESSFUL );

    dz_effect_t * roundtrip;
    DZ_TEST_CHECK( read_effect_bytes( service, &roundtrip, data, size ) == DZ_SUCCESSFUL );
    DZ_TEST_CHECK( dz_effect_get_layer_count( roundtrip ) == 1U );
    dz_project_profile_t loaded_profile;
    dz_effect_get_project_profile( roundtrip, &loaded_profile );
    DZ_TEST_CHECK( loaded_profile.projection == DZ_PROJECTION_PERSPECTIVE );
    dz_effect_layer_desc_t loaded_layer;
    dz_effect_get_layer( roundtrip, 0, &loaded_layer );
    DZ_TEST_CHECK( loaded_layer.z == 4.f && loaded_layer.particle_mode == DZ_PARTICLE_MODE_MESH );
    DZ_TEST_CHECK( loaded_layer.mesh_id == 7U && dz_effect_get_mesh_count( roundtrip ) == 1U );
    dz_mesh_desc_t loaded_mesh;
    DZ_TEST_CHECK( dz_effect_get_mesh( roundtrip, 7U, &loaded_mesh ) == DZ_SUCCESSFUL && loaded_mesh.vertex_count == 4U && loaded_mesh.index_count == 12U );
    DZ_TEST_CHECK( dz_material_get_pass_count( loaded_layer.material ) == 2U );
    dz_material_pass_desc_t loaded_pass;
    dz_material_get_pass( loaded_layer.material, 1U, &loaded_pass );
    DZ_TEST_CHECK( loaded_pass.uniform_count == 2U && loaded_pass.uniforms[0].value_count == 4U && loaded_pass.uniforms[1].semantic == DZ_UNIFORM_TIME );
    DZ_TEST_CHECK( loaded_pass.texture_binding_count == 1U && loaded_pass.texture_bindings[0].mag_filter == DZ_SAMPLER_LINEAR );
    DZ_TEST_CHECK( dz_effect_get_physics_object_count( roundtrip ) == 1U );
    dz_effect_destroy( service, roundtrip );

    free( data );
    dz_effect_destroy( service, source );
    dz_service_destroy( service );
    DZ_TEST_CHECK( memory.allocated == 0 );
    return EXIT_SUCCESS;
}
