#include "test_support.h"

#include <stdint.h>
#include <string.h>

static uint64_t hash_bytes( uint64_t _hash, const void * _data, dz_size_t _size )
{
    const dz_uint8_t * bytes = (const dz_uint8_t *)_data;
    for( dz_size_t index = 0; index != _size; ++index )
    {
        _hash ^= bytes[index];
        _hash *= UINT64_C( 1099511628211 );
    }
    return _hash;
}

static void add_tetrahedron( const dz_service_t * _service, dz_effect_t * _effect, dz_uint32_t _id )
{
    const dz_mesh_vertex_t vertices[4] = { { { 0.f, 0.f, 0.f }, { 0.f, 0.f, -1.f }, { 1.f, 0.f, 0.f, 1.f }, { 0.f, 0.f }, { 0.f, 0.f } },
                                           { { 1.f, 0.f, 0.f }, { 0.f, 0.f, -1.f }, { 1.f, 0.f, 0.f, 1.f }, { 1.f, 0.f }, { 1.f, 0.f } },
                                           { { 0.f, 1.f, 0.f }, { 0.f, 0.f, -1.f }, { 1.f, 0.f, 0.f, 1.f }, { 0.f, 1.f }, { 0.f, 1.f } },
                                           { { 0.f, 0.f, 1.f }, { 0.f, 1.f, 0.f }, { 1.f, 0.f, 0.f, 1.f }, { 1.f, 1.f }, { 1.f, 1.f } } };
    const dz_uint32_t indices[12] = { 0, 2, 1, 0, 1, 3, 0, 3, 2, 1, 2, 3 };
    dz_mesh_desc_t mesh;
    memset( &mesh, 0, sizeof( mesh ) );
    mesh.id = _id;
    mesh.vertices = vertices;
    mesh.vertex_count = 4U;
    mesh.indices = indices;
    mesh.index_count = 12U;
    dz_effect_add_mesh( _service, _effect, &mesh );
}

static dz_result_t add_layer( const dz_service_t * _service, dz_effect_t * _effect, dz_shape_type_e _shape_type, dz_particle_mode_e _mode, dz_uint32_t _mesh_id )
{
    dz_material_t * material = DZ_NULLPTR;
    dz_shape_t * shape = DZ_NULLPTR;
    dz_emitter_t * emitter = DZ_NULLPTR;
    dz_affector_t * affector = DZ_NULLPTR;
    dz_material_create( _service, &material, DZ_NULLPTR );
    dz_shape_create( _service, &shape, _shape_type, DZ_NULLPTR );
    dz_emitter_create( _service, &emitter, DZ_NULLPTR );
    dz_affector_create( _service, &affector, DZ_NULLPTR );

    if( _shape_type == DZ_SHAPE_POLYGON )
    {
        static const dz_float_t polygon[6] = { -1.f, -1.f, 1.f, -1.f, 0.f, 1.f };
        dz_shape_set_polygon( shape, polygon, 1U );
    }
    else if( _shape_type == DZ_SHAPE_MASK )
    {
        static const dz_uint8_t mask[4] = { 0U, 255U, 255U, 255U };
        dz_shape_set_mask( shape, mask, 1U, 2U, 2U, 2U );
        dz_shape_set_mask_threshold( shape, 1U );
    }
    else if( _shape_type == DZ_SHAPE_MESH_SURFACE || _shape_type == DZ_SHAPE_MESH_VOLUME )
    {
        dz_shape_set_mesh_id( shape, _mesh_id );
    }
    dz_effect_layer_desc_t layer;
    dz_effect_layer_desc_default( &layer );
    layer.material = material;
    layer.shape = shape;
    layer.emitter = emitter;
    layer.affector = affector;
    layer.life = 2.f;
    layer.particle_mode = _mode;
    layer.trail_width = 0.25f;
    if( _mode == DZ_PARTICLE_MODE_MESH )
    {
        layer.mesh_id = _mesh_id;
    }
    dz_uint32_t layer_index;
    dz_effect_add_layer( _effect, &layer, &layer_index );

    dz_effect_trigger_desc_t trigger;
    memset( &trigger, 0, sizeof( trigger ) );
    trigger.event_type = DZ_EFFECT_EVENT_EFFECT_START;
    trigger.source_layer_index = DZ_EFFECT_LAYER_NONE;
    trigger.target_layer_index = layer_index;
    trigger.probability = 1.f;
    trigger.spawn_count_min = 1U;
    trigger.spawn_count_max = 1U;
    dz_effect_add_trigger( _effect, &trigger, DZ_NULLPTR );

    return DZ_SUCCESSFUL;
}

static int verify_mixed_geometry( const dz_service_t * _service )
{
    dz_project_profile_t profile;
    dz_project_profile_default( &profile, DZ_PROJECTION_PERSPECTIVE );
    dz_effect_t * effect = DZ_NULLPTR;
    dz_effect_create_with_profile( _service, &effect, &profile, 2.f, 101U, DZ_NULLPTR );
    add_tetrahedron( _service, effect, 7U );

    for( dz_uint32_t shape = 0; shape != __DZ_SHAPE_MAX__; ++shape )
    {
        const dz_particle_mode_e mode = (dz_particle_mode_e)( shape % __DZ_PARTICLE_MODE_MAX__ );
        DZ_TEST_CHECK( add_layer( _service, effect, (dz_shape_type_e)shape, mode, 7U ) == DZ_SUCCESSFUL );
    }

    dz_instance_t * instance = DZ_NULLPTR;
    dz_instance_create( _service, &instance, effect, DZ_NULLPTR );
    DZ_TEST_CHECK( dz_instance_update( _service, instance, 0.25f ) == DZ_SUCCESSFUL );
    DZ_TEST_CHECK( dz_instance_get_particle_count( instance ) >= __DZ_SHAPE_MAX__ );

    dz_camera_state_t camera;
    dz_camera_state_from_profile( &profile, 800.f, 600.f, &camera );
    dz_render_requirements_t requirements;
    dz_instance_prepare_render( instance, &camera, &requirements );
    DZ_TEST_CHECK( requirements.vertex_count > 0U && requirements.index_count > 0U && requirements.chunk_count > 0U );

    dz_vec3_t * positions = (dz_vec3_t *)calloc( requirements.vertex_count, sizeof( dz_vec3_t ) );
    dz_vec3_t * normals = (dz_vec3_t *)calloc( requirements.vertex_count, sizeof( dz_vec3_t ) );
    dz_vec4_t * tangents = (dz_vec4_t *)calloc( requirements.vertex_count, sizeof( dz_vec4_t ) );
    dz_vec4_t * colors = (dz_vec4_t *)calloc( requirements.vertex_count, sizeof( dz_vec4_t ) );
    dz_vec2_t * uv0 = (dz_vec2_t *)calloc( requirements.vertex_count, sizeof( dz_vec2_t ) );
    dz_vec2_t * uv1 = (dz_vec2_t *)calloc( requirements.vertex_count, sizeof( dz_vec2_t ) );
    const dz_size_t index_size = requirements.index_type == DZ_INDEX_UINT32 ? sizeof( dz_uint32_t ) : sizeof( dz_uint16_t );
    void * indices = calloc( requirements.index_count, index_size );
    dz_render_chunk_t * chunks = (dz_render_chunk_t *)calloc( requirements.chunk_count, sizeof( dz_render_chunk_t ) );

    dz_render_buffers_t buffers;
    memset( &buffers, 0, sizeof( buffers ) );
#define SET_STREAM( name, pointer, type )                                                                                                                                          \
    buffers.name.buffer = pointer;                                                                                                                                                 \
    buffers.name.size = sizeof( type ) * requirements.vertex_count;                                                                                                                \
    buffers.name.stride = sizeof( type )
    SET_STREAM( positions, positions, dz_vec3_t );
    SET_STREAM( normals, normals, dz_vec3_t );
    SET_STREAM( tangents, tangents, dz_vec4_t );
    SET_STREAM( colors, colors, dz_vec4_t );
    SET_STREAM( uv0, uv0, dz_vec2_t );
    SET_STREAM( uv1, uv1, dz_vec2_t );
#undef SET_STREAM
    buffers.indices = indices;
    buffers.indices_size = index_size * requirements.index_count;
    buffers.index_type = requirements.index_type;
    dz_uint32_t chunk_count = 0;
    DZ_TEST_CHECK( dz_instance_fill_render( instance, &camera, &buffers, chunks, requirements.chunk_count, &chunk_count ) == DZ_SUCCESSFUL );
    DZ_TEST_CHECK( chunk_count == requirements.chunk_count );
    for( dz_uint32_t index = 0; index != requirements.vertex_count; ++index )
    {
        const dz_float_t normal_length = sqrtf( normals[index].x * normals[index].x + normals[index].y * normals[index].y + normals[index].z * normals[index].z );
        const dz_float_t tangent_length = sqrtf( tangents[index].x * tangents[index].x + tangents[index].y * tangents[index].y + tangents[index].z * tangents[index].z );
        DZ_TEST_CHECK( isfinite( positions[index].x ) && isfinite( positions[index].y ) && isfinite( positions[index].z ) );
        DZ_TEST_CHECK( normal_length > 0.5f && tangent_length > 0.5f );
    }

    uint64_t first_hash = UINT64_C( 1469598103934665603 );
    first_hash = hash_bytes( first_hash, positions, sizeof( *positions ) * requirements.vertex_count );
    first_hash = hash_bytes( first_hash, normals, sizeof( *normals ) * requirements.vertex_count );
    first_hash = hash_bytes( first_hash, tangents, sizeof( *tangents ) * requirements.vertex_count );
    first_hash = hash_bytes( first_hash, colors, sizeof( *colors ) * requirements.vertex_count );
    first_hash = hash_bytes( first_hash, uv0, sizeof( *uv0 ) * requirements.vertex_count );
    first_hash = hash_bytes( first_hash, uv1, sizeof( *uv1 ) * requirements.vertex_count );
    first_hash = hash_bytes( first_hash, indices, index_size * requirements.index_count );
    first_hash = hash_bytes( first_hash, chunks, sizeof( *chunks ) * requirements.chunk_count );

    memset( positions, 0, sizeof( *positions ) * requirements.vertex_count );
    memset( normals, 0, sizeof( *normals ) * requirements.vertex_count );
    memset( tangents, 0, sizeof( *tangents ) * requirements.vertex_count );
    memset( colors, 0, sizeof( *colors ) * requirements.vertex_count );
    memset( uv0, 0, sizeof( *uv0 ) * requirements.vertex_count );
    memset( uv1, 0, sizeof( *uv1 ) * requirements.vertex_count );
    memset( indices, 0, index_size * requirements.index_count );
    memset( chunks, 0, sizeof( *chunks ) * requirements.chunk_count );
    DZ_TEST_CHECK( dz_instance_fill_render( instance, &camera, &buffers, chunks, requirements.chunk_count, &chunk_count ) == DZ_SUCCESSFUL );

    uint64_t second_hash = UINT64_C( 1469598103934665603 );
    second_hash = hash_bytes( second_hash, positions, sizeof( *positions ) * requirements.vertex_count );
    second_hash = hash_bytes( second_hash, normals, sizeof( *normals ) * requirements.vertex_count );
    second_hash = hash_bytes( second_hash, tangents, sizeof( *tangents ) * requirements.vertex_count );
    second_hash = hash_bytes( second_hash, colors, sizeof( *colors ) * requirements.vertex_count );
    second_hash = hash_bytes( second_hash, uv0, sizeof( *uv0 ) * requirements.vertex_count );
    second_hash = hash_bytes( second_hash, uv1, sizeof( *uv1 ) * requirements.vertex_count );
    second_hash = hash_bytes( second_hash, indices, index_size * requirements.index_count );
    second_hash = hash_bytes( second_hash, chunks, sizeof( *chunks ) * requirements.chunk_count );
    DZ_TEST_CHECK( first_hash == second_hash );

    free( chunks );
    free( indices );
    free( uv1 );
    free( uv0 );
    free( colors );
    free( tangents );
    free( normals );
    free( positions );
    dz_instance_destroy( _service, instance );
    dz_effect_destroy( _service, effect );
    return EXIT_SUCCESS;
}

static int verify_index32( const dz_service_t * _service )
{
    const dz_uint32_t vertex_count = 65536U;
    dz_mesh_vertex_t * vertices = (dz_mesh_vertex_t *)calloc( vertex_count, sizeof( dz_mesh_vertex_t ) );
    for( dz_uint32_t index = 0; index != vertex_count; ++index )
    {
        vertices[index].position.x = (dz_float_t)index * 0.001f;
        vertices[index].normal.y = 1.f;
        vertices[index].tangent.x = 1.f;
        vertices[index].tangent.w = 1.f;
    }
    const dz_uint32_t indices[3] = { 0U, 65534U, 65535U };

    dz_project_profile_t profile;
    dz_project_profile_default( &profile, DZ_PROJECTION_ORTHOGRAPHIC );
    dz_effect_t * effect = DZ_NULLPTR;
    dz_effect_create_with_profile( _service, &effect, &profile, 1.f, 9U, DZ_NULLPTR );
    dz_mesh_desc_t mesh;
    memset( &mesh, 0, sizeof( mesh ) );
    mesh.id = 99U;
    mesh.vertices = vertices;
    mesh.vertex_count = vertex_count;
    mesh.indices = indices;
    mesh.index_count = 3U;
    dz_effect_add_mesh( _service, effect, &mesh );
    free( vertices );
    DZ_TEST_CHECK( add_layer( _service, effect, DZ_SHAPE_POINT, DZ_PARTICLE_MODE_MESH, 99U ) == DZ_SUCCESSFUL );
    dz_instance_t * instance = DZ_NULLPTR;
    dz_instance_create( _service, &instance, effect, DZ_NULLPTR );
    DZ_TEST_CHECK( dz_instance_update( _service, instance, 0.11f ) == DZ_SUCCESSFUL );
    dz_render_requirements_t requirements;
    dz_instance_prepare_render( instance, DZ_NULLPTR, &requirements );
    DZ_TEST_CHECK( requirements.vertex_count >= vertex_count && requirements.index_type == DZ_INDEX_UINT32 );
    dz_instance_destroy( _service, instance );
    dz_effect_destroy( _service, effect );
    return EXIT_SUCCESS;
}

int main( void )
{
    dz_test_memory_t memory = { 0, 0, 0 };
    dz_service_t * service = DZ_NULLPTR;
    dz_test_service_create( &service, &memory );
    DZ_TEST_CHECK( verify_mixed_geometry( service ) == EXIT_SUCCESS );
    DZ_TEST_CHECK( verify_index32( service ) == EXIT_SUCCESS );
    dz_service_destroy( service );
    DZ_TEST_CHECK( memory.allocated == 0U );
    return EXIT_SUCCESS;
}
