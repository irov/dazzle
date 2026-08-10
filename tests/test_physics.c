#include "test_support.h"

#include <string.h>

static dz_result_t make_effect( const dz_service_t * _service, dz_effect_t ** _effect )
{
    dz_project_profile_t profile;
    dz_project_profile_default( &profile, DZ_PROJECTION_PERSPECTIVE );
    dz_effect_create_with_profile( _service, _effect, &profile, 1.f, 4242U, DZ_NULLPTR );

    dz_material_t * material;
    dz_shape_t * shape;
    dz_emitter_t * emitter;
    dz_affector_t * affector;
    dz_material_create( _service, &material, DZ_NULLPTR );
    dz_shape_create( _service, &shape, DZ_SHAPE_POINT, DZ_NULLPTR );
    dz_emitter_create( _service, &emitter, DZ_NULLPTR );
    dz_affector_create( _service, &affector, DZ_NULLPTR );

    dz_timeline_key_t * spawn_count = DZ_NULLPTR;
    dz_timeline_key_create( _service, &spawn_count, 0.f, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR );
    dz_timeline_key_set_const_value( spawn_count, 1.f );
    dz_emitter_set_timeline( emitter, DZ_EMITTER_SPAWN_COUNT, spawn_count );

    dz_effect_layer_desc_t layer;
    dz_effect_layer_desc_default( &layer );
    layer.material = material;
    layer.shape = shape;
    layer.emitter = emitter;
    layer.affector = affector;
    layer.life = 0.11f;
    dz_uint32_t layer_index;
    dz_effect_add_layer( *_effect, &layer, &layer_index );

    dz_effect_trigger_desc_t trigger;
    memset( &trigger, 0, sizeof( trigger ) );
    trigger.event_type = DZ_EFFECT_EVENT_EFFECT_START;
    trigger.source_layer_index = DZ_EFFECT_LAYER_NONE;
    trigger.target_layer_index = layer_index;
    trigger.probability = 1.f;
    trigger.spawn_count_min = 1U;
    trigger.spawn_count_max = 1U;
    dz_effect_add_trigger( *_effect, &trigger, DZ_NULLPTR );

    return DZ_SUCCESSFUL;
}

static dz_result_t simulate_one( const dz_service_t * _service, const dz_effect_t * _effect, dz_float_t _time, dz_particle_state_t * _state, dz_uint16_t * _count )
{
    dz_instance_t * instance = DZ_NULLPTR;
    dz_instance_create( _service, &instance, _effect, DZ_NULLPTR );
    dz_result_t result = dz_instance_update( _service, instance, _time );
    if( result == DZ_SUCCESSFUL )
    {
        *_count = dz_instance_get_particle_count( instance );
        if( *_count != 0U && _state != DZ_NULLPTR )
        {
            dz_instance_get_particle_state( instance, 0U, _state );
        }
    }
    dz_instance_destroy( _service, instance );
    return result;
}

static dz_physics_object_desc_t default_object( dz_uint32_t _id, dz_physics_object_type_e _type )
{
    dz_physics_object_desc_t object;
    memset( &object, 0, sizeof( object ) );
    object.id = _id;
    object.mesh_id = DZ_RESOURCE_ID_NONE;
    object.type = _type;
    object.transform.rotation.w = 1.f;
    object.transform.scale.x = object.transform.scale.y = object.transform.scale.z = 1.f;
    object.response = DZ_COLLISION_BOUNCE;
    object.restitution = 1.f;
    return object;
}

static dz_vec3_t normalize3( dz_vec3_t _value )
{
    const dz_float_t length = sqrtf( _value.x * _value.x + _value.y * _value.y + _value.z * _value.z );
    if( length > 0.f )
    {
        _value.x /= length;
        _value.y /= length;
        _value.z /= length;
    }
    return _value;
}

static dz_float_t dot3( dz_vec3_t _a, dz_vec3_t _b )
{
    return _a.x * _b.x + _a.y * _b.y + _a.z * _b.z;
}

int main( void )
{
    dz_test_memory_t memory = { 0, 0, 0 };
    dz_service_t * service = DZ_NULLPTR;
    dz_test_service_create( &service, &memory );
    dz_effect_t * effect = DZ_NULLPTR;
    DZ_TEST_CHECK( make_effect( service, &effect ) == DZ_SUCCESSFUL );

    dz_particle_state_t baseline;
    dz_uint16_t count = 0;
    DZ_TEST_CHECK( simulate_one( service, effect, 0.3f, &baseline, &count ) == DZ_SUCCESSFUL );
    DZ_TEST_CHECK( count != 0U );
    const dz_uint16_t baseline_count = count;

    dz_physics_object_desc_t field = default_object( 1U, DZ_PHYSICS_GRAVITY );
    field.direction.z = -1.f;
    field.strength = 20.f;
    dz_effect_add_physics_object( effect, &field, DZ_NULLPTR );
    dz_particle_state_t state;
    DZ_TEST_CHECK( simulate_one( service, effect, 0.3f, &state, &count ) == DZ_SUCCESSFUL && state.velocity.z < baseline.velocity.z );

    field.type = DZ_PHYSICS_WIND;
    field.direction.x = 1.f;
    field.direction.z = 0.f;
    field.strength = 15.f;
    field.turbulence = 3.f;
    dz_effect_set_physics_object( effect, 0U, &field );
    DZ_TEST_CHECK( simulate_one( service, effect, 0.3f, &state, &count ) == DZ_SUCCESSFUL && state.velocity.x != baseline.velocity.x );

    field.type = DZ_PHYSICS_MAGNET;
    field.transform.position.z = 10.f;
    field.strength = 30.f;
    field.falloff = 0.1f;
    dz_effect_set_physics_object( effect, 0U, &field );
    DZ_TEST_CHECK( simulate_one( service, effect, 0.3f, &state, &count ) == DZ_SUCCESSFUL && state.velocity.z > baseline.velocity.z );
    dz_effect_remove_physics_object( effect, 0U );

    const dz_vec3_t direction = normalize3( baseline.velocity );
    const dz_vec3_t obstacle_position = { direction.x * 5.f, direction.y * 5.f, direction.z * 5.f };
    const dz_vec3_t opposing_normal = { -direction.x, -direction.y, -direction.z };

    dz_physics_object_desc_t obstacle = default_object( 2U, DZ_PHYSICS_PLANE );
    obstacle.transform.position = obstacle_position;
    obstacle.direction = opposing_normal;
    dz_effect_add_physics_object( effect, &obstacle, DZ_NULLPTR );
    DZ_TEST_CHECK( simulate_one( service, effect, 0.3f, &state, &count ) == DZ_SUCCESSFUL && count == baseline_count );
    DZ_TEST_CHECK( dot3( state.velocity, opposing_normal ) > 0.f );

    obstacle.type = DZ_PHYSICS_SPHERE;
    obstacle.radius = 1.f;
    obstacle.response = DZ_COLLISION_SLIDE;
    obstacle.friction = 0.f;
    dz_effect_set_physics_object( effect, 0U, &obstacle );
    DZ_TEST_CHECK( simulate_one( service, effect, 0.3f, &state, &count ) == DZ_SUCCESSFUL && count == baseline_count );
    DZ_TEST_CHECK( isfinite( state.position.x ) && isfinite( state.position.y ) && isfinite( state.position.z ) );

    obstacle.type = DZ_PHYSICS_BOX;
    obstacle.half_extents.x = obstacle.half_extents.y = obstacle.half_extents.z = 1.f;
    obstacle.response = DZ_COLLISION_BOUNCE;
    dz_effect_set_physics_object( effect, 0U, &obstacle );
    DZ_TEST_CHECK( simulate_one( service, effect, 0.3f, &state, &count ) == DZ_SUCCESSFUL && count == baseline_count );
    DZ_TEST_CHECK( dot3( state.velocity, direction ) < 0.f );

    dz_vec3_t tangent = normalize3( (dz_vec3_t){ -direction.y, direction.x, 0.f } );
    if( dot3( tangent, tangent ) < 0.5f )
    {
        tangent = (dz_vec3_t){ 1.f, 0.f, 0.f };
    }
    dz_vec3_t bitangent = normalize3( (dz_vec3_t){ opposing_normal.y * tangent.z - opposing_normal.z * tangent.y, opposing_normal.z * tangent.x - opposing_normal.x * tangent.z,
                                                   opposing_normal.x * tangent.y - opposing_normal.y * tangent.x } );
    dz_mesh_vertex_t mesh_vertices[3];
    memset( mesh_vertices, 0, sizeof( mesh_vertices ) );
    const dz_vec3_t corners[3] = { { obstacle_position.x + tangent.x * 20.f + bitangent.x * 20.f, obstacle_position.y + tangent.y * 20.f + bitangent.y * 20.f,
                                     obstacle_position.z + tangent.z * 20.f + bitangent.z * 20.f },
                                   { obstacle_position.x - tangent.x * 20.f + bitangent.x * 20.f, obstacle_position.y - tangent.y * 20.f + bitangent.y * 20.f,
                                     obstacle_position.z - tangent.z * 20.f + bitangent.z * 20.f },
                                   { obstacle_position.x - bitangent.x * 20.f, obstacle_position.y - bitangent.y * 20.f, obstacle_position.z - bitangent.z * 20.f } };
    for( dz_uint32_t index = 0; index != 3U; ++index )
    {
        mesh_vertices[index].position = corners[index];
        mesh_vertices[index].normal = opposing_normal;
        mesh_vertices[index].tangent.x = tangent.x;
        mesh_vertices[index].tangent.y = tangent.y;
        mesh_vertices[index].tangent.z = tangent.z;
        mesh_vertices[index].tangent.w = 1.f;
    }
    dz_uint32_t mesh_indices[27];
    for( dz_uint32_t triangle = 0; triangle != 9U; ++triangle )
    {
        mesh_indices[triangle * 3U + 0U] = 0U;
        mesh_indices[triangle * 3U + 1U] = 1U;
        mesh_indices[triangle * 3U + 2U] = 2U;
    }
    dz_mesh_desc_t mesh;
    memset( &mesh, 0, sizeof( mesh ) );
    mesh.id = 77U;
    mesh.vertices = mesh_vertices;
    mesh.vertex_count = 3U;
    mesh.indices = mesh_indices;
    mesh.index_count = 27U;
    dz_effect_add_mesh( service, effect, &mesh );
    obstacle.type = DZ_PHYSICS_MESH;
    obstacle.mesh_id = 77U;
    obstacle.transform.position = (dz_vec3_t){ 0.f, 0.f, 0.f };
    obstacle.response = DZ_COLLISION_KILL;
    dz_effect_set_physics_object( effect, 0U, &obstacle );
    DZ_TEST_CHECK( simulate_one( service, effect, 0.3f, &state, &count ) == DZ_SUCCESSFUL && count == 0U );

    dz_instance_t * transform_instance = DZ_NULLPTR;
    dz_instance_create( service, &transform_instance, effect, DZ_NULLPTR );
    dz_transform_t moved = obstacle.transform;
    moved.position.z = 100.f;
    DZ_TEST_CHECK( dz_instance_set_physics_transform( transform_instance, obstacle.id, &moved ) == DZ_SUCCESSFUL );
    DZ_TEST_CHECK( dz_instance_set_physics_transform( transform_instance, 999U, &moved ) == DZ_FAILURE_INVALID_DATA );
    dz_instance_destroy( service, transform_instance );

    dz_effect_destroy( service, effect );
    dz_service_destroy( service );
    DZ_TEST_CHECK( memory.allocated == 0U );
    return EXIT_SUCCESS;
}
