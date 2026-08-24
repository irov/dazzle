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
        const dz_shape_mask_source_t source = { mask, 2U, 2U, 2U, 1U, 0U, 1U };
        dz_shape_build_mask( _service, shape, &source );
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
    DZ_TEST_CHECK( dz_instance_fill_render( _service, instance, &camera, &buffers, chunks, requirements.chunk_count, &chunk_count ) == DZ_SUCCESSFUL );
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
    DZ_TEST_CHECK( dz_instance_fill_render( _service, instance, &camera, &buffers, chunks, requirements.chunk_count, &chunk_count ) == DZ_SUCCESSFUL );

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

static int verify_beam_birth_anchor( const dz_service_t * _service )
{
    dz_project_profile_t profile;
    dz_project_profile_default( &profile, DZ_PROJECTION_ORTHOGRAPHIC );

    dz_effect_t * effect = DZ_NULLPTR;
    dz_effect_create_with_profile( _service, &effect, &profile, 2.f, 404U, DZ_NULLPTR );

    dz_material_t * material = DZ_NULLPTR;
    dz_shape_t * shape = DZ_NULLPTR;
    dz_emitter_t * emitter = DZ_NULLPTR;
    dz_affector_t * affector = DZ_NULLPTR;
    dz_material_create( _service, &material, DZ_NULLPTR );
    dz_shape_create( _service, &shape, DZ_SHAPE_SEGMENT, DZ_NULLPTR );
    dz_emitter_create( _service, &emitter, DZ_NULLPTR );
    dz_affector_create( _service, &affector, DZ_NULLPTR );

    dz_texture_t * texture = DZ_NULLPTR;
    dz_texture_create( _service, &texture, DZ_NULLPTR );
    const dz_float_t texture_u[4] = {0.2f, 0.7f, 0.7f, 0.2f};
    const dz_float_t texture_v[4] = {0.3f, 0.3f, 0.8f, 0.8f};
    dz_texture_set_uv( texture, texture_u, texture_v );
    dz_texture_set_width( texture, 1.f );
    dz_texture_set_height( texture, 1.f );
    DZ_TEST_CHECK( dz_material_add_texture( material, texture ) == DZ_SUCCESSFUL );
    dz_material_set_mode( material, DZ_MATERIAL_MODE_TEXTURE );

    dz_timeline_key_t * angle_min = DZ_NULLPTR;
    dz_timeline_key_create( _service, &angle_min, 0.f, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR );
    dz_timeline_key_set_const_value( angle_min, 0.f );
    dz_shape_set_timeline( shape, DZ_SHAPE_SEGMENT_ANGLE_MIN, angle_min );

    dz_timeline_key_t * angle_max = DZ_NULLPTR;
    dz_timeline_key_create( _service, &angle_max, 0.f, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR );
    dz_timeline_key_set_const_value( angle_max, 0.f );
    dz_shape_set_timeline( shape, DZ_SHAPE_SEGMENT_ANGLE_MAX, angle_max );

    dz_timeline_key_t * spawn_delay = DZ_NULLPTR;
    dz_timeline_key_create( _service, &spawn_delay, 0.f, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR );
    dz_timeline_key_set_const_value( spawn_delay, 0.001f );
    dz_emitter_set_timeline( emitter, DZ_EMITTER_SPAWN_DELAY, spawn_delay );

    dz_timeline_key_t * spawn_count = DZ_NULLPTR;
    dz_timeline_key_create( _service, &spawn_count, 0.f, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR );
    dz_timeline_key_set_const_value( spawn_count, 1.f );
    dz_emitter_set_timeline( emitter, DZ_EMITTER_SPAWN_COUNT, spawn_count );

    dz_timeline_key_t * particle_life = DZ_NULLPTR;
    dz_timeline_key_create( _service, &particle_life, 0.f, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR );
    dz_timeline_key_set_const_value( particle_life, 1.5f );
    dz_affector_set_timeline( affector, DZ_AFFECTOR_TIMELINE_LIFE, particle_life );

    dz_timeline_key_t * move_speed = DZ_NULLPTR;
    dz_timeline_key_create( _service, &move_speed, 0.f, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR );
    dz_timeline_key_set_const_value( move_speed, 4.f );
    dz_affector_set_timeline( affector, DZ_AFFECTOR_TIMELINE_MOVE_SPEED, move_speed );

    dz_effect_layer_desc_t layer;
    dz_effect_layer_desc_default( &layer );
    layer.material = material;
    layer.shape = shape;
    layer.emitter = emitter;
    layer.affector = affector;
    layer.life = 0.0015f;
    layer.particle_mode = DZ_PARTICLE_MODE_BEAM;
    layer.trail_width = 0.2f;

    dz_uint32_t layer_index;
    dz_effect_add_layer( effect, &layer, &layer_index );

    dz_effect_trigger_desc_t trigger;
    memset( &trigger, 0, sizeof( trigger ) );
    trigger.event_type = DZ_EFFECT_EVENT_EFFECT_START;
    trigger.source_layer_index = DZ_EFFECT_LAYER_NONE;
    trigger.target_layer_index = layer_index;
    trigger.probability = 1.f;
    trigger.spawn_count_min = 1U;
    trigger.spawn_count_max = 1U;
    dz_effect_add_trigger( effect, &trigger, DZ_NULLPTR );

    dz_instance_t * instance = DZ_NULLPTR;
    dz_instance_create( _service, &instance, effect, DZ_NULLPTR );
    DZ_TEST_CHECK( dz_instance_update( _service, instance, 0.25f ) == DZ_SUCCESSFUL );
    DZ_TEST_CHECK( dz_instance_get_particle_count( instance ) == 1U );

    dz_render_requirements_t requirements;
    dz_instance_prepare_render( instance, DZ_NULLPTR, &requirements );
    DZ_TEST_CHECK( requirements.vertex_count == 4U && requirements.index_count == 6U && requirements.chunk_count == 1U );

    dz_vec3_t positions[4];
    dz_vec3_t normals[4];
    dz_vec4_t tangents[4];
    dz_vec4_t colors[4];
    dz_vec2_t uv0[4];
    dz_vec2_t uv1[4];
    dz_uint16_t indices[6];
    dz_render_chunk_t chunks[1];
    dz_render_buffers_t buffers;
    memset( &buffers, 0, sizeof( buffers ) );
#define SET_BEAM_STREAM( name, pointer, type )                                                                                                                                     \
    buffers.name.buffer = pointer;                                                                                                                                                \
    buffers.name.size = sizeof( type ) * 4U;                                                                                                                                      \
    buffers.name.stride = sizeof( type )
    SET_BEAM_STREAM( positions, positions, dz_vec3_t );
    SET_BEAM_STREAM( normals, normals, dz_vec3_t );
    SET_BEAM_STREAM( tangents, tangents, dz_vec4_t );
    SET_BEAM_STREAM( colors, colors, dz_vec4_t );
    SET_BEAM_STREAM( uv0, uv0, dz_vec2_t );
    SET_BEAM_STREAM( uv1, uv1, dz_vec2_t );
#undef SET_BEAM_STREAM
    buffers.indices = indices;
    buffers.indices_size = sizeof( indices );
    buffers.index_type = DZ_INDEX_UINT16;

    dz_uint32_t chunk_count = 0U;
    DZ_TEST_CHECK( dz_instance_fill_render( _service, instance, DZ_NULLPTR, &buffers, chunks, 1U, &chunk_count ) == DZ_SUCCESSFUL );
    DZ_TEST_CHECK( chunk_count == 1U );
    DZ_TEST_CHECK( uv0[0].x == 0.2f && uv0[0].y == 0.3f );
    DZ_TEST_CHECK( uv0[1].x == 0.7f && uv0[1].y == 0.3f );
    DZ_TEST_CHECK( uv0[2].x == 0.7f && uv0[2].y == 0.8f );
    DZ_TEST_CHECK( uv0[3].x == 0.2f && uv0[3].y == 0.8f );

    dz_float_t minimum_x = positions[0].x;
    dz_float_t maximum_x = positions[0].x;
    for( dz_uint32_t index = 1U; index != 4U; ++index )
    {
        minimum_x = DZ_MIN( minimum_x, positions[index].x );
        maximum_x = DZ_MAX( maximum_x, positions[index].x );
    }
    DZ_TEST_CHECK( minimum_x < 0.01f );
    DZ_TEST_CHECK( maximum_x > 0.9f );

    const dz_path_point_t path_points[4] = {
        {{0.f, 0.f, 0.f}, 0.5f, 0.1f, {1.f, 0.f, 0.f, 1.f}, DZ_PATH_POINT_FLAG_BREAK | DZ_PATH_POINT_FLAG_COLOR},
        {{1.f, 1.f, 0.f}, 1.f, 0.5f, {0.f, 0.f, 0.f, 0.f}, DZ_PATH_POINT_FLAG_NONE},
        {{2.f, 0.f, 0.f}, 0.75f, 0.2f, {0.f, 1.f, 0.f, 1.f}, DZ_PATH_POINT_FLAG_BREAK | DZ_PATH_POINT_FLAG_COLOR},
        {{3.f, 1.f, 0.f}, 1.5f, 1.f, {0.f, 0.f, 0.f, 0.f}, DZ_PATH_POINT_FLAG_NONE}
    };
    DZ_TEST_CHECK( dz_instance_set_path_points( _service, instance, path_points, 4U ) == DZ_SUCCESSFUL );
    DZ_TEST_CHECK( dz_instance_get_path_point_count( instance ) == 4U );
    dz_instance_prepare_render( instance, DZ_NULLPTR, &requirements );
    DZ_TEST_CHECK( requirements.vertex_count == 8U && requirements.index_count == 12U && requirements.chunk_count == 1U );

    dz_vec3_t path_positions[16];
    dz_vec3_t path_normals[16];
    dz_vec4_t path_tangents[16];
    dz_vec4_t path_colors[16];
    dz_vec2_t path_uv0[16];
    dz_vec2_t path_uv1[16];
    dz_uint16_t path_indices[24];
    memset( &buffers, 0, sizeof( buffers ) );
#define SET_PATH_STREAM( name, pointer, type )                                                                                                                                     \
    buffers.name.buffer = pointer;                                                                                                                                                \
    buffers.name.size = sizeof( pointer );                                                                                                                                        \
    buffers.name.stride = sizeof( type )
    SET_PATH_STREAM( positions, path_positions, dz_vec3_t );
    SET_PATH_STREAM( normals, path_normals, dz_vec3_t );
    SET_PATH_STREAM( tangents, path_tangents, dz_vec4_t );
    SET_PATH_STREAM( colors, path_colors, dz_vec4_t );
    SET_PATH_STREAM( uv0, path_uv0, dz_vec2_t );
    SET_PATH_STREAM( uv1, path_uv1, dz_vec2_t );
#undef SET_PATH_STREAM
    buffers.indices = path_indices;
    buffers.indices_size = sizeof( path_indices );
    buffers.index_type = DZ_INDEX_UINT16;
    chunk_count = 0U;
    DZ_TEST_CHECK( dz_instance_fill_render( _service, instance, DZ_NULLPTR, &buffers, chunks, 1U, &chunk_count ) == DZ_SUCCESSFUL );
    DZ_TEST_CHECK( chunk_count == 1U );
    DZ_TEST_CHECK( path_uv0[0].x == 0.2f && path_uv0[1].x == 0.7f );
    DZ_TEST_CHECK( path_uv0[0].y == 0.3f && path_uv0[2].y == 0.8f && path_uv0[4].y == 0.3f && path_uv0[6].y == 0.8f );
    DZ_TEST_CHECK( path_colors[0].w < path_colors[2].w && path_colors[4].w < path_colors[6].w );
    DZ_TEST_CHECK( path_colors[0].x == 1.f && path_colors[0].y == 0.f && path_colors[4].x == 0.f && path_colors[4].y == 1.f );
    DZ_TEST_CHECK( path_colors[2].x == 1.f && path_colors[2].y == 1.f && path_colors[2].z == 1.f );
    DZ_TEST_CHECK( path_colors[6].x == 1.f && path_colors[6].y == 1.f && path_colors[6].z == 1.f );
    DZ_TEST_CHECK( path_indices[0] == 0U && path_indices[2] == 2U && path_indices[6] == 4U && path_indices[8] == 6U );
    DZ_TEST_CHECK( fabsf( (path_positions[0].x + path_positions[1].x) * 0.5f - path_points[0].position.x ) < 0.0001f );
    DZ_TEST_CHECK( fabsf( (path_positions[2].y + path_positions[3].y) * 0.5f - path_points[1].position.y ) < 0.0001f );

    dz_aabb_t path_aabb;
    dz_instance_get_aabb( _service, instance, &path_aabb );
    DZ_TEST_CHECK( path_aabb.valid == DZ_TRUE && path_aabb.minimum.x < 0.f && path_aabb.maximum.x > 3.f && path_aabb.maximum.y > 1.f );

    layer.particle_mode = DZ_PARTICLE_MODE_PATH;
    dz_effect_set_layer( effect, layer_index, &layer );
    dz_instance_prepare_render( instance, DZ_NULLPTR, &requirements );
    DZ_TEST_CHECK( requirements.vertex_count == 16U && requirements.index_count == 24U && requirements.chunk_count == 1U );
    chunk_count = 0U;
    DZ_TEST_CHECK( dz_instance_fill_render( _service, instance, DZ_NULLPTR, &buffers, chunks, 1U, &chunk_count ) == DZ_SUCCESSFUL );
    DZ_TEST_CHECK( chunk_count == 1U );
    DZ_TEST_CHECK( path_colors[0].w < path_colors[4].w && path_colors[8].w < path_colors[12].w );

    dz_instance_clear_path_points( instance );
    DZ_TEST_CHECK( dz_instance_get_path_point_count( instance ) == 0U );

    dz_instance_destroy( _service, instance );
    dz_effect_destroy( _service, effect );
    return EXIT_SUCCESS;
}

static int verify_mask_threshold_sampling( const dz_service_t * _service )
{
    dz_project_profile_t profile;
    dz_project_profile_default( &profile, DZ_PROJECTION_ORTHOGRAPHIC );

    dz_effect_t * effect = DZ_NULLPTR;
    dz_effect_create_with_profile( _service, &effect, &profile, 1.f, 808U, DZ_NULLPTR );

    dz_material_t * material = DZ_NULLPTR;
    dz_shape_t * shape = DZ_NULLPTR;
    dz_emitter_t * emitter = DZ_NULLPTR;
    dz_affector_t * affector = DZ_NULLPTR;
    dz_material_create( _service, &material, DZ_NULLPTR );
    dz_shape_create( _service, &shape, DZ_SHAPE_MASK, DZ_NULLPTR );
    dz_emitter_create( _service, &emitter, DZ_NULLPTR );
    dz_affector_create( _service, &affector, DZ_NULLPTR );

    static const dz_uint8_t rgba_mask[5U * 4U] = {
        255U, 64U, 32U, 0U,
        255U, 64U, 32U, 64U,
        255U, 64U, 32U, 128U,
        255U, 64U, 32U, 192U,
        255U, 64U, 32U, 255U
    };
    const dz_shape_mask_source_t compiled_source = { rgba_mask, 5U * 4U, 5U, 1U, 4U, 3U, 64U };
    dz_emitter_texture_desc_t emitter_texture_desc;
    dz_emitter_texture_desc_default( &emitter_texture_desc );
    emitter_texture_desc.alpha_threshold = 64U;
    DZ_TEST_CHECK( dz_shape_set_emitter_texture_desc( shape, &emitter_texture_desc ) == DZ_SUCCESSFUL );

    dz_timeline_key_t * spawn_count = DZ_NULLPTR;
    dz_timeline_key_create( _service, &spawn_count, 0.f, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR );
    dz_timeline_key_set_const_value( spawn_count, 4096.f );
    dz_emitter_set_timeline( emitter, DZ_EMITTER_SPAWN_COUNT, spawn_count );

    dz_timeline_key_t * move_speed = DZ_NULLPTR;
    dz_timeline_key_create( _service, &move_speed, 0.f, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR );
    dz_timeline_key_set_const_value( move_speed, 0.f );
    dz_affector_set_timeline( affector, DZ_AFFECTOR_TIMELINE_MOVE_SPEED, move_speed );

    dz_effect_layer_desc_t layer;
    dz_effect_layer_desc_default( &layer );
    layer.material = material;
    layer.shape = shape;
    layer.emitter = emitter;
    layer.affector = affector;
    layer.life = 0.11f;

    dz_uint32_t layer_index;
    dz_effect_add_layer( effect, &layer, &layer_index );
    DZ_TEST_CHECK( dz_effect_requires_emitter_texture( effect ) == DZ_TRUE );
    DZ_TEST_CHECK( dz_effect_set_emitter_texture( _service, effect, &compiled_source ) == DZ_SUCCESSFUL );

    const void * compiled_bits = DZ_NULLPTR;
    dz_uint32_t compiled_pitch = 0U;
    dz_shape_get_mask_bits( shape, &compiled_bits, &compiled_pitch );
    DZ_TEST_CHECK( compiled_bits != DZ_NULLPTR && compiled_pitch == 1U );
    DZ_TEST_CHECK( ((const dz_uint8_t *)compiled_bits)[0] == 0x1CU );

    dz_effect_trigger_desc_t trigger;
    memset( &trigger, 0, sizeof( trigger ) );
    trigger.event_type = DZ_EFFECT_EVENT_EFFECT_START;
    trigger.source_layer_index = DZ_EFFECT_LAYER_NONE;
    trigger.target_layer_index = layer_index;
    trigger.probability = 1.f;
    trigger.spawn_count_min = 1U;
    trigger.spawn_count_max = 1U;
    dz_effect_add_trigger( effect, &trigger, DZ_NULLPTR );

    dz_instance_t * threshold_instance = DZ_NULLPTR;
    dz_instance_create( _service, &threshold_instance, effect, DZ_NULLPTR );
    dz_instance_set_particle_limit( threshold_instance, 4096U );
    DZ_TEST_CHECK( dz_instance_update( _service, threshold_instance, 0.3f ) == DZ_SUCCESSFUL );

    const dz_uint16_t threshold_particle_count = dz_instance_get_particle_count( threshold_instance );
    DZ_TEST_CHECK( threshold_particle_count > 2200U && threshold_particle_count < 2700U );

    dz_uint32_t threshold_counts[5] = {0U, 0U, 0U, 0U, 0U};

    for( dz_uint16_t index = 0U; index != threshold_particle_count; ++index )
    {
        dz_particle_state_t state;
        dz_instance_get_particle_state( threshold_instance, index, &state );

        DZ_TEST_CHECK( state.position.x >= 0.f && state.position.x < 5.f );
        DZ_TEST_CHECK( state.position.y >= 0.f && state.position.y < 1.f );
        ++threshold_counts[(dz_uint32_t)state.position.x];
    }

    DZ_TEST_CHECK( threshold_counts[0] == 0U && threshold_counts[1] == 0U );
    DZ_TEST_CHECK( threshold_counts[2] > 600U && threshold_counts[3] > 600U && threshold_counts[4] > 600U );
    dz_instance_destroy( _service, threshold_instance );

    const dz_uint32_t large_mask_width = 256U;
    const dz_uint32_t large_mask_height = 192U;
    const dz_size_t large_mask_size = (dz_size_t)large_mask_width * large_mask_height * 2U;
    dz_uint8_t * large_mask = (dz_uint8_t *)malloc( large_mask_size );
    DZ_TEST_CHECK( large_mask != DZ_NULLPTR );

    for( dz_size_t index = 0U; index != (dz_size_t)large_mask_width * large_mask_height; ++index )
    {
        large_mask[index * 2U + 0U] = 255U;
        large_mask[index * 2U + 1U] = 255U;
    }

    const dz_shape_mask_source_t external_source = { large_mask, large_mask_width * 2U, large_mask_width, large_mask_height, 2U, 1U, 254U };
    emitter_texture_desc.alpha_threshold = 254U;
    emitter_texture_desc.boundary = DZ_FALSE;
    emitter_texture_desc.compile = DZ_FALSE;
    DZ_TEST_CHECK( dz_shape_set_emitter_texture_desc( shape, &emitter_texture_desc ) == DZ_SUCCESSFUL );
    DZ_TEST_CHECK( dz_effect_set_emitter_texture( _service, effect, &external_source ) == DZ_SUCCESSFUL );
    dz_timeline_key_set_const_value( spawn_count, 256.f );

    dz_instance_t * accepted_instance = DZ_NULLPTR;
    dz_instance_create( _service, &accepted_instance, effect, DZ_NULLPTR );
    dz_instance_set_particle_limit( accepted_instance, 512U );
    DZ_TEST_CHECK( dz_instance_update( _service, accepted_instance, 0.3f ) == DZ_SUCCESSFUL );
    DZ_TEST_CHECK( dz_instance_get_particle_count( accepted_instance ) == 256U );
    dz_instance_destroy( _service, accepted_instance );

    for( dz_size_t index = 0U; index != (dz_size_t)large_mask_width * large_mask_height; ++index )
    {
        large_mask[index * 2U + 0U] = 255U;
        large_mask[index * 2U + 1U] = 0U;
    }
    dz_instance_t * rejected_instance = DZ_NULLPTR;
    dz_instance_create( _service, &rejected_instance, effect, DZ_NULLPTR );
    dz_instance_set_particle_limit( rejected_instance, 512U );
    DZ_TEST_CHECK( dz_instance_update( _service, rejected_instance, 0.3f ) == DZ_SUCCESSFUL );
    DZ_TEST_CHECK( dz_instance_get_particle_count( rejected_instance ) == 0U );
    dz_instance_destroy( _service, rejected_instance );
    free( large_mask );

    dz_uint8_t rgba[7U * 7U * 4U];
    memset( rgba, 0, sizeof( rgba ) );

    for( dz_uint32_t y = 1U; y <= 5U; ++y )
    {
        for( dz_uint32_t x = 1U; x <= 5U; ++x )
        {
            dz_uint8_t * pixel = rgba + (y * 7U + x) * 4U;
            pixel[0] = x < 3U ? 20U : 180U;
            pixel[1] = 40U;
            pixel[2] = 60U;
            pixel[3] = 255U;
        }
    }

    emitter_texture_desc.alpha_threshold = 8U;
    emitter_texture_desc.rgb_threshold = 32U;
    emitter_texture_desc.strata = 4U;
    emitter_texture_desc.boundary = DZ_TRUE;
    DZ_TEST_CHECK( dz_shape_set_emitter_texture_desc( shape, &emitter_texture_desc ) == DZ_SUCCESSFUL );
    const dz_shape_mask_source_t boundary_source = { rgba, 7U * 4U, 7U, 7U, 4U, 3U, 0U };
    DZ_TEST_CHECK( dz_effect_set_emitter_texture( _service, effect, &boundary_source ) == DZ_SUCCESSFUL );
    DZ_TEST_CHECK( dz_shape_get_mask_boundary_point_count( shape ) == 22U );
    DZ_TEST_CHECK( dz_shape_get_mask_boundary_strata_count( shape ) == 4U );

    dz_instance_t * boundary_instance = DZ_NULLPTR;
    dz_instance_create( _service, &boundary_instance, effect, DZ_NULLPTR );
    dz_instance_set_particle_limit( boundary_instance, 512U );
    DZ_TEST_CHECK( dz_instance_update( _service, boundary_instance, 0.3f ) == DZ_SUCCESSFUL );
    DZ_TEST_CHECK( dz_instance_get_particle_count( boundary_instance ) == 256U );

    dz_uint32_t occupied_strata = 0U;
    dz_bool_t strata_seen[4] = {DZ_FALSE, DZ_FALSE, DZ_FALSE, DZ_FALSE};

    for( dz_uint16_t index = 0U; index != dz_instance_get_particle_count( boundary_instance ); ++index )
    {
        dz_particle_state_t state;
        dz_instance_get_particle_state( boundary_instance, index, &state );
        const dz_uint32_t x = (dz_uint32_t)state.position.x;
        const dz_uint32_t y = (dz_uint32_t)state.position.y;
        const dz_bool_t alpha_edge = x == 1U || x == 5U || y == 1U || y == 5U;
        const dz_bool_t rgb_edge = (x == 2U || x == 3U) && y >= 2U && y <= 4U;
        DZ_TEST_CHECK( alpha_edge == DZ_TRUE || rgb_edge == DZ_TRUE );

        const dz_uint32_t column = x < 3U ? 0U : 1U;
        const dz_uint32_t row = y < 3U ? 0U : 1U;
        strata_seen[row * 2U + column] = DZ_TRUE;
    }

    for( dz_uint32_t index = 0U; index != 4U; ++index )
    {
        occupied_strata += strata_seen[index] == DZ_TRUE ? 1U : 0U;
    }

    DZ_TEST_CHECK( occupied_strata == 4U );
    dz_instance_destroy( _service, boundary_instance );
    dz_shape_clear_mask_boundary( _service, shape );
    DZ_TEST_CHECK( dz_shape_get_mask_boundary_point_count( shape ) == 0U );
    DZ_TEST_CHECK( dz_shape_get_mask_boundary_strata_count( shape ) == 0U );

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
    DZ_TEST_CHECK( verify_beam_birth_anchor( service ) == EXIT_SUCCESS );
    DZ_TEST_CHECK( verify_mask_threshold_sampling( service ) == EXIT_SUCCESS );
    dz_service_destroy( service );
    DZ_TEST_CHECK( memory.allocated == 0U );
    return EXIT_SUCCESS;
}
