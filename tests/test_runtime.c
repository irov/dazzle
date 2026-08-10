#include "test_support.h"

#include <math.h>
#include <string.h>

static dz_result_t make_effect( const dz_service_t * service, dz_projection_type_e projection, dz_particle_mode_e mode, dz_effect_t ** effect )
{
    dz_project_profile_t profile;
    dz_project_profile_default( &profile, projection );
    dz_effect_create_with_profile( service, effect, &profile, 2.f, 777U, DZ_NULLPTR );

    dz_material_t * material;
    dz_shape_t * shape;
    dz_emitter_t * emitter;
    dz_affector_t * affector;
    dz_material_create( service, &material, DZ_NULLPTR );
    dz_shape_create( service, &shape, DZ_SHAPE_SPHERE, DZ_NULLPTR );
    dz_emitter_create( service, &emitter, DZ_NULLPTR );
    dz_affector_create( service, &affector, DZ_NULLPTR );

    dz_material_set_mode( material, DZ_MATERIAL_MODE_SOLID );
    dz_timeline_key_t * elevation;
    dz_timeline_key_create( service, &elevation, 0.f, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR );
    dz_timeline_key_set_const_value( elevation, 0.35f );
    dz_emitter_set_timeline( emitter, DZ_EMITTER_SPAWN_ELEVATION_MIN, elevation );

    dz_effect_layer_desc_t layer;
    dz_effect_layer_desc_default( &layer );
    layer.material = material;
    layer.shape = shape;
    layer.emitter = emitter;
    layer.affector = affector;
    layer.life = 2.f;
    layer.seed = 11U;
    layer.particle_mode = mode;
    layer.sorting = DZ_PARTICLE_SORT_CAMERA_FAR;
    dz_uint32_t layer_index;
    dz_effect_add_layer( *effect, &layer, &layer_index );

    dz_effect_trigger_desc_t trigger;
    memset( &trigger, 0, sizeof( trigger ) );
    trigger.event_type = DZ_EFFECT_EVENT_EFFECT_START;
    trigger.source_layer_index = DZ_EFFECT_LAYER_NONE;
    trigger.target_layer_index = layer_index;
    trigger.probability = 1.f;
    trigger.spawn_count_min = 1U;
    trigger.spawn_count_max = 1U;
    dz_effect_add_trigger( *effect, &trigger, DZ_NULLPTR );

    dz_physics_object_desc_t gravity;
    memset( &gravity, 0, sizeof( gravity ) );
    gravity.id = 19U;
    gravity.type = DZ_PHYSICS_GRAVITY;
    gravity.transform.rotation.w = 1.f;
    gravity.transform.scale.x = gravity.transform.scale.y = gravity.transform.scale.z = 1.f;
    gravity.direction.y = -1.f;
    gravity.strength = 9.8f;
    gravity.response = DZ_COLLISION_BOUNCE;
    dz_effect_add_physics_object( *effect, &gravity, DZ_NULLPTR );

    return DZ_SUCCESSFUL;
}

static int compare_states( const dz_instance_t * a, const dz_instance_t * b )
{
    if( dz_instance_get_particle_count( a ) != dz_instance_get_particle_count( b ) )
    {
        fprintf( stderr, "particle count mismatch: %u != %u, time %.9g != %.9g\n", (unsigned)dz_instance_get_particle_count( a ), (unsigned)dz_instance_get_particle_count( b ),
                 dz_instance_get_time( a ), dz_instance_get_time( b ) );
        return 0;
    }
    for( dz_uint16_t i = 0; i != dz_instance_get_particle_count( a ); ++i )
    {
        dz_particle_state_t sa, sb;
        dz_instance_get_particle_state( a, i, &sa );
        dz_instance_get_particle_state( b, i, &sb );
        if( memcmp( &sa, &sb, sizeof( sa ) ) != 0 )
        {
            fprintf( stderr, "particle %u mismatch at time %.9g != %.9g: pos %.9g %.9g %.9g vs %.9g %.9g %.9g\n", (unsigned)i, dz_instance_get_time( a ), dz_instance_get_time( b ),
                     sa.position.x, sa.position.y, sa.position.z, sb.position.x, sb.position.y, sb.position.z );
            return 0;
        }
    }
    return 1;
}

int main( void )
{
    dz_test_memory_t memory = { 0, 0, 0 };
    dz_service_t * service;
    dz_test_service_create( &service, &memory );

    dz_project_profile_t ortho_profile, perspective_profile;
    dz_project_profile_default( &ortho_profile, DZ_PROJECTION_ORTHOGRAPHIC );
    dz_project_profile_default( &perspective_profile, DZ_PROJECTION_PERSPECTIVE );

    const dz_vec3_t expected_euler = { 23.f, -31.f, 47.f };
    dz_quat_t rotation;
    dz_quat_from_euler_xyz_degrees( &expected_euler, &rotation );
    dz_vec3_t actual_euler;
    dz_quat_to_euler_xyz_degrees( &rotation, &actual_euler );
    DZ_TEST_CHECK( fabsf( actual_euler.x - expected_euler.x ) < 0.0001f );
    DZ_TEST_CHECK( fabsf( actual_euler.y - expected_euler.y ) < 0.0001f );
    DZ_TEST_CHECK( fabsf( actual_euler.z - expected_euler.z ) < 0.0001f );

    dz_effect_t * ortho_effect;
    dz_effect_t * perspective_effect;
    DZ_TEST_CHECK( make_effect( service, DZ_PROJECTION_ORTHOGRAPHIC, DZ_PARTICLE_MODE_TRAIL, &ortho_effect ) == DZ_SUCCESSFUL );
    DZ_TEST_CHECK( make_effect( service, DZ_PROJECTION_PERSPECTIVE, DZ_PARTICLE_MODE_TRAIL, &perspective_effect ) == DZ_SUCCESSFUL );

    dz_instance_t * ortho_instance;
    dz_instance_t * perspective_instance;
    dz_instance_create( service, &ortho_instance, ortho_effect, DZ_NULLPTR );
    dz_instance_create( service, &perspective_instance, perspective_effect, DZ_NULLPTR );
    DZ_TEST_CHECK( dz_instance_update( service, ortho_instance, 0.5f ) == DZ_SUCCESSFUL );
    DZ_TEST_CHECK( dz_instance_update( service, perspective_instance, 0.5f ) == DZ_SUCCESSFUL );
    DZ_TEST_CHECK( compare_states( ortho_instance, perspective_instance ) != 0 );
    DZ_TEST_CHECK( dz_instance_get_particle_count( ortho_instance ) != 0 );

    dz_instance_restart( ortho_instance );
    DZ_TEST_CHECK( dz_instance_update( service, ortho_instance, 0.2f ) == DZ_SUCCESSFUL );
    DZ_TEST_CHECK( dz_instance_update( service, ortho_instance, 0.3f ) == DZ_SUCCESSFUL );
    DZ_TEST_CHECK( compare_states( ortho_instance, perspective_instance ) != 0 );

    const dz_float_t stopped_time = dz_instance_get_time( ortho_instance );
    const dz_uint16_t stopped_count = dz_instance_get_particle_count( ortho_instance );
    dz_instance_stop( ortho_instance );
    DZ_TEST_CHECK( dz_instance_update( service, ortho_instance, 0.5f ) == DZ_SUCCESSFUL );
    DZ_TEST_CHECK( dz_instance_get_time( ortho_instance ) == stopped_time && dz_instance_get_particle_count( ortho_instance ) == stopped_count );

    DZ_TEST_CHECK( dz_instance_seek( service, ortho_instance, 0.5f, dz_instance_get_fixed_step( perspective_instance ) ) == DZ_SUCCESSFUL );
    DZ_TEST_CHECK( compare_states( ortho_instance, perspective_instance ) != 0 );
    const dz_float_t pause_time = dz_instance_get_time( ortho_instance );
    dz_instance_pause( ortho_instance );
    DZ_TEST_CHECK( dz_instance_is_paused( ortho_instance ) == DZ_TRUE );
    dz_particle_state_t paused_state;
    dz_instance_get_particle_state( ortho_instance, 0U, &paused_state );
    DZ_TEST_CHECK( dz_instance_update( service, ortho_instance, 0.1f ) == DZ_SUCCESSFUL && dz_instance_get_time( ortho_instance ) == pause_time );
    dz_particle_state_t paused_state_after;
    dz_instance_get_particle_state( ortho_instance, 0U, &paused_state_after );
    DZ_TEST_CHECK( memcmp( &paused_state, &paused_state_after, sizeof( paused_state ) ) == 0 );
    dz_instance_resume( ortho_instance );
    DZ_TEST_CHECK( dz_instance_is_paused( ortho_instance ) == DZ_FALSE );
    DZ_TEST_CHECK( dz_instance_update( service, ortho_instance, 0.1f ) == DZ_SUCCESSFUL && dz_instance_get_time( ortho_instance ) > pause_time );

    dz_camera_state_t ortho_camera, perspective_camera;
    dz_camera_state_from_profile( &ortho_profile, 1280.f, 720.f, &ortho_camera );
    dz_camera_state_from_profile( &perspective_profile, 1280.f, 720.f, &perspective_camera );
    dz_render_requirements_t requirements;
    dz_instance_prepare_render( ortho_instance, &perspective_camera, &requirements );
    dz_instance_prepare_render( ortho_instance, &ortho_camera, &requirements );
    DZ_TEST_CHECK( requirements.vertex_count != 0 && requirements.index_count != 0 && requirements.chunk_count != 0 );

    dz_render_buffers_t empty_buffers;
    memset( &empty_buffers, 0, sizeof( empty_buffers ) );
    empty_buffers.index_type = requirements.index_type;
    dz_uint32_t chunk_count;
    DZ_TEST_CHECK( dz_instance_fill_render( ortho_instance, &ortho_camera, &empty_buffers, DZ_NULLPTR, 0, &chunk_count ) == DZ_FAILURE_BUFFER_TOO_SMALL );

    dz_aabb_t bounds;
    dz_instance_get_aabb( ortho_instance, &bounds );
    DZ_TEST_CHECK( bounds.valid == DZ_TRUE );
    dz_bool_t visible = DZ_FALSE;
    dz_camera_test_aabb( &ortho_camera, &bounds, &visible );
    DZ_TEST_CHECK( visible == DZ_TRUE );
    dz_aabb_t distant = bounds;
    distant.minimum.x += 100000.f;
    distant.maximum.x += 100000.f;
    dz_camera_test_aabb( &ortho_camera, &distant, &visible );
    DZ_TEST_CHECK( visible == DZ_FALSE );

    dz_instance_destroy( service, ortho_instance );
    dz_instance_destroy( service, perspective_instance );
    dz_effect_destroy( service, ortho_effect );
    dz_effect_destroy( service, perspective_effect );
    dz_service_destroy( service );
    DZ_TEST_CHECK( memory.allocated == 0 );
    return EXIT_SUCCESS;
}
