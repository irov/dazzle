#include "dazzle/dazzle_read.h"

#include "affector.h"
#include "alloc.h"
#include "atlas.h"
#include "effect.h"
#include "emitter.h"
#include "material.h"
#include "memory.h"
#include "service.h"
#include "shape.h"
#include "texture.h"
#include "timeline_interpolate.h"
#include "timeline_key.h"

//////////////////////////////////////////////////////////////////////////
dz_result_t dz_header_read( dz_stream_read_t _read, dz_userdata_t _ud, dz_effect_read_status_e * const _status )
{
    DZ_UNUSED( _read );
    DZ_UNUSED( _ud );
    *_status = DZ_EFFECT_LOAD_STATUS_INVALID_VERSION;
    return DZ_FAILURE_UNSUPPORTED;
}
//////////////////////////////////////////////////////////////////////////
typedef struct dz_reader_t
{
    dz_stream_read_t read;
    dz_userdata_t ud;
    dz_result_t result;
} dz_reader_t;

static dz_uint32_t __load_u32( const dz_uint8_t * _data )
{
    return (dz_uint32_t)_data[0] | ( (dz_uint32_t)_data[1] << 8 ) | ( (dz_uint32_t)_data[2] << 16 ) | ( (dz_uint32_t)_data[3] << 24 );
}

static void __read_bytes( dz_reader_t * _reader, void * _data, dz_size_t _size )
{
    if( _reader->result != DZ_SUCCESSFUL )
    {
        return;
    }
    _reader->result = ( *_reader->read )( _data, _size, _reader->ud );
}

static dz_uint32_t __read_u32( dz_reader_t * _reader )
{
    dz_uint8_t bytes[4] = {0};
    __read_bytes( _reader, bytes, sizeof( bytes ) );
    return __load_u32( bytes );
}

static dz_float_t __read_f32( dz_reader_t * _reader )
{
    const dz_uint32_t bits = __read_u32( _reader );
    dz_float_t value;
    dz_memory_copy( &value, &bits, sizeof( value ) );
    return value;
}

static dz_vec3_t __read_vec3( dz_reader_t * _reader )
{
    dz_vec3_t value;
    value.x = __read_f32( _reader );
    value.y = __read_f32( _reader );
    value.z = __read_f32( _reader );
    return value;
}

static dz_quat_t __read_quat( dz_reader_t * _reader )
{
    dz_quat_t value;
    value.x = __read_f32( _reader );
    value.y = __read_f32( _reader );
    value.z = __read_f32( _reader );
    value.w = __read_f32( _reader );
    return value;
}

static dz_transform_t __read_transform( dz_reader_t * _reader )
{
    dz_transform_t transform;
    transform.position = __read_vec3( _reader );
    transform.rotation = __read_quat( _reader );
    transform.scale = __read_vec3( _reader );
    return transform;
}

static void __read_timeline( const dz_service_t * _service, dz_reader_t * _reader, dz_timeline_key_t ** _timeline )
{
    *_timeline = DZ_NULLPTR;
    const dz_uint32_t count = __read_u32( _reader );

    dz_timeline_key_t * root = DZ_NULLPTR;
    dz_timeline_interpolate_t * previous_interpolate = DZ_NULLPTR;

    for( dz_uint32_t index = 0; index != count; ++index )
    {
        const dz_float_t p = __read_f32( _reader );
        const dz_uint32_t key_type = __read_u32( _reader );
        const dz_float_t const_value = __read_f32( _reader );
        const dz_float_t random_min = __read_f32( _reader );
        const dz_float_t random_max = __read_f32( _reader );
        const dz_uint32_t has_interpolate = __read_u32( _reader );
        const dz_uint32_t interpolate_type = has_interpolate ? __read_u32( _reader ) : 0U;
        const dz_float_t p0 = has_interpolate ? __read_f32( _reader ) : 0.f;
        const dz_float_t p1 = has_interpolate ? __read_f32( _reader ) : 0.f;
        const dz_float_t out_tangent = has_interpolate ? __read_f32( _reader ) : 0.f;
        const dz_float_t in_tangent = has_interpolate ? __read_f32( _reader ) : 0.f;

        dz_timeline_key_t * key;
        dz_timeline_key_create( _service, &key, p, (dz_timeline_key_type_e)key_type, DZ_NULLPTR );
        key->const_value = const_value;
        key->randomize_min_value = random_min;
        key->randomize_max_value = random_max;

        if( root == DZ_NULLPTR )
        {
            root = key;
        }
        if( previous_interpolate != DZ_NULLPTR )
        {
            previous_interpolate->key = key;
        }

        previous_interpolate = DZ_NULLPTR;
        if( has_interpolate )
        {
            dz_timeline_interpolate_create( _service, &previous_interpolate, (dz_timeline_interpolate_type_e)interpolate_type, DZ_NULLPTR );
            previous_interpolate->p0 = p0;
            previous_interpolate->p1 = p1;
            previous_interpolate->out_tangent = out_tangent;
            previous_interpolate->in_tangent = in_tangent;
            key->interpolate = previous_interpolate;
        }
    }

    *_timeline = root;
}

static void __read_texture( const dz_service_t * _service, dz_reader_t * _reader, dz_texture_t ** _texture, dz_float_t * _weight )
{
    dz_texture_create( _service, _texture, DZ_NULLPTR );
    for( dz_uint32_t i = 0; i != 4; ++i )
    {
        ( *_texture )->u[i] = __read_f32( _reader );
    }
    for( dz_uint32_t i = 0; i != 4; ++i )
    {
        ( *_texture )->v[i] = __read_f32( _reader );
    }
    ( *_texture )->width = __read_f32( _reader );
    ( *_texture )->height = __read_f32( _reader );
    ( *_texture )->trim_offset_x = __read_f32( _reader );
    ( *_texture )->trim_offset_y = __read_f32( _reader );
    ( *_texture )->trim_width = __read_f32( _reader );
    ( *_texture )->trim_height = __read_f32( _reader );
    *_weight = __read_f32( _reader );
    ( *_texture )->sequence_delay = __read_f32( _reader );
}

static void __read_material( const dz_service_t * _service, dz_reader_t * _reader, const dz_atlas_t * _atlas, dz_material_t ** _material )
{
    dz_material_create( _service, _material, DZ_NULLPTR );

    ( *_material )->blend_type = (dz_blend_type_e)__read_u32( _reader );
    ( *_material )->r = __read_f32( _reader );
    ( *_material )->g = __read_f32( _reader );
    ( *_material )->b = __read_f32( _reader );
    ( *_material )->a = __read_f32( _reader );
    ( *_material )->mode = (dz_material_mode_e)__read_u32( _reader );
    ( *_material )->texture_index = __read_u32( _reader );
    ( *_material )->texture_count = __read_u32( _reader );
    const dz_uint32_t texture_count = __read_u32( _reader );
    for( dz_uint32_t i = 0; i != texture_count; ++i )
    {
        dz_texture_t * texture;
        dz_float_t weight;
        __read_texture( _service, _reader, &texture, &weight );
        ( *_material )->textures[( *_material )->textures_count].texture = texture;
        ( *_material )->textures[( *_material )->textures_count].random_weight = weight;
        ++( *_material )->textures_count;
    }

    const dz_uint32_t pass_count = __read_u32( _reader );
    ( *_material )->pass_count = 0;
    for( dz_uint32_t i = 0; i != pass_count; ++i )
    {
        dz_material_pass_desc_t pass;
        dz_memory_zero( &pass, sizeof( pass ) );
        __read_bytes( _reader, pass.technique_id, DZ_TECHNIQUE_ID_MAX );
        pass.blend = (dz_blend_type_e)__read_u32( _reader );
        pass.depth_test = (dz_bool_t)__read_u32( _reader );
        pass.depth_write = (dz_bool_t)__read_u32( _reader );
        pass.depth_compare = (dz_depth_compare_e)__read_u32( _reader );
        pass.cull = (dz_cull_mode_e)__read_u32( _reader );
        pass.color_mask = (dz_uint8_t)__read_u32( _reader );
        pass.uniform_count = __read_u32( _reader );
        for( dz_uint32_t uniform_index = 0; uniform_index != pass.uniform_count; ++uniform_index )
        {
            dz_uniform_desc_t * uniform = pass.uniforms + uniform_index;
            __read_bytes( _reader, uniform->name, DZ_UNIFORM_NAME_MAX );
            uniform->semantic = (dz_uniform_semantic_e)__read_u32( _reader );
            uniform->value_count = __read_u32( _reader );
            for( dz_uint32_t value = 0; value != uniform->value_count; ++value )
            {
                uniform->values[value] = __read_f32( _reader );
            }
        }
        pass.texture_binding_count = __read_u32( _reader );
        for( dz_uint32_t binding_index = 0; binding_index != pass.texture_binding_count; ++binding_index )
        {
            dz_texture_binding_desc_t * binding = pass.texture_bindings + binding_index;
            __read_bytes( _reader, binding->uniform_name, DZ_UNIFORM_NAME_MAX );
            binding->texture_slot = __read_u32( _reader );
            binding->min_filter = (dz_sampler_filter_e)__read_u32( _reader );
            binding->mag_filter = (dz_sampler_filter_e)__read_u32( _reader );
            binding->wrap_u = (dz_sampler_wrap_e)__read_u32( _reader );
            binding->wrap_v = (dz_sampler_wrap_e)__read_u32( _reader );
        }
        dz_material_add_pass( *_material, &pass, DZ_NULLPTR );
    }
    ( *_material )->atlas = _atlas;
}

static void __read_shape( const dz_service_t * _service, dz_reader_t * _reader, dz_shape_t ** _shape )
{
    const dz_uint32_t type = __read_u32( _reader );
    dz_shape_create( _service, _shape, (dz_shape_type_e)type, DZ_NULLPTR );
    ( *_shape )->transform = __read_transform( _reader );
    ( *_shape )->dimensions = __read_vec3( _reader );
    ( *_shape )->mesh_id = __read_u32( _reader );
    ( *_shape )->has_emitter_texture_desc = (dz_bool_t)__read_u32( _reader );
    ( *_shape )->emitter_texture_desc.alpha_threshold = __read_u32( _reader );
    ( *_shape )->emitter_texture_desc.rgb_threshold = __read_u32( _reader );
    ( *_shape )->emitter_texture_desc.strata = __read_u32( _reader );
    ( *_shape )->emitter_texture_desc.sample_scale = __read_f32( _reader );
    ( *_shape )->emitter_texture_desc.boundary = (dz_bool_t)__read_u32( _reader );
    ( *_shape )->emitter_texture_desc.compile = (dz_bool_t)__read_u32( _reader );

    for( dz_uint32_t i = 0; i != __DZ_SHAPE_TIMELINE_MAX__; ++i )
    {
        dz_timeline_key_t * timeline;
        __read_timeline( _service, _reader, &timeline );
        ( *_shape )->timelines[i] = timeline;
    }

    const dz_uint32_t triangle_count = __read_u32( _reader );
    if( triangle_count != 0 )
    {
        dz_float_t * triangles = DZ_REALLOCN( _service, DZ_NULLPTR, dz_float_t, triangle_count * 6U );
        ( *_shape )->triangles = triangles;
        ( *_shape )->triangle_count = triangle_count;
        ( *_shape )->owns_triangles = DZ_TRUE;
        for( dz_uint32_t i = 0; i != triangle_count * 6U; ++i )
        {
            triangles[i] = __read_f32( _reader );
        }
    }

    ( *_shape )->mask_uses_bits = (dz_bool_t)__read_u32( _reader );
    ( *_shape )->mask_source.pitch = __read_u32( _reader );
    ( *_shape )->mask_source.width = __read_u32( _reader );
    ( *_shape )->mask_source.height = __read_u32( _reader );
    ( *_shape )->mask_source.channel_count = __read_u32( _reader );
    ( *_shape )->mask_source.alpha_channel = __read_u32( _reader );
    ( *_shape )->mask_source.alpha_threshold = __read_u32( _reader );
    ( *_shape )->mask_scale = __read_f32( _reader );
    ( *_shape )->mask_bits_pitch = __read_u32( _reader );
    const dz_uint32_t mask_size = __read_u32( _reader );
    if( mask_size != 0 )
    {
        void * mask = DZ_REALLOCN( _service, DZ_NULLPTR, dz_uint8_t, mask_size );

        if( ( *_shape )->mask_uses_bits == DZ_TRUE )
        {
            ( *_shape )->mask_bits = mask;
        }
        else
        {
            ( *_shape )->mask_source.buffer = mask;
            ( *_shape )->owns_mask_source = DZ_TRUE;
        }

        __read_bytes( _reader, mask, mask_size );
    }
}

static dz_result_t __read_emitter( const dz_service_t * _service, dz_reader_t * _reader, dz_emitter_t ** _emitter )
{
    dz_emitter_create( _service, _emitter, DZ_NULLPTR );
    ( *_emitter )->life = __read_f32( _reader );
    for( dz_uint32_t i = 0; i != __DZ_EMITTER_TIMELINE_MAX__; ++i )
    {
        dz_timeline_key_t * timeline;
        __read_timeline( _service, _reader, &timeline );
        ( *_emitter )->timelines[i] = timeline;
    }
    return _reader->result;
}

static dz_result_t __read_affector( const dz_service_t * _service, dz_reader_t * _reader, dz_affector_t ** _affector )
{
    dz_affector_create( _service, _affector, DZ_NULLPTR );
    for( dz_uint32_t i = 0; i != __DZ_AFFECTOR_TIMELINE_MAX__; ++i )
    {
        dz_timeline_key_t * timeline;
        __read_timeline( _service, _reader, &timeline );
        ( *_affector )->timelines[i] = timeline;
    }
    return _reader->result;
}

static void __read_layer( const dz_service_t * _service, dz_reader_t * _reader, const dz_atlas_t * _atlas, dz_effect_layer_desc_t * _layer )
{
    dz_effect_layer_desc_default( _layer );
    __read_material( _service, _reader, _atlas, (dz_material_t **)&_layer->material );
    __read_shape( _service, _reader, (dz_shape_t **)&_layer->shape );
    __read_emitter( _service, _reader, (dz_emitter_t **)&_layer->emitter );
    __read_affector( _service, _reader, (dz_affector_t **)&_layer->affector );

    _layer->x = __read_f32( _reader );
    _layer->y = __read_f32( _reader );
    _layer->z = __read_f32( _reader );
    _layer->angle = __read_f32( _reader );
    _layer->rotation = __read_quat( _reader );
    _layer->scale = __read_vec3( _reader );
    _layer->particle_mode = (dz_particle_mode_e)__read_u32( _reader );
    _layer->orientation = (dz_particle_orientation_e)__read_u32( _reader );
    _layer->sorting = (dz_particle_sort_e)__read_u32( _reader );
    _layer->orientation_axis = __read_vec3( _reader );
    _layer->mesh_id = __read_u32( _reader );
    _layer->trail_width = __read_f32( _reader );
    _layer->trail_lifetime = __read_f32( _reader );
    _layer->life = __read_f32( _reader );
    _layer->seed = __read_u32( _reader );
}

static dz_result_t __read_trigger( dz_reader_t * _reader, dz_effect_trigger_desc_t * _trigger )
{
    _trigger->event_type = (dz_effect_event_type_e)__read_u32( _reader );
    _trigger->source_layer_index = __read_u32( _reader );
    _trigger->target_layer_index = __read_u32( _reader );
    _trigger->time = __read_f32( _reader );
    _trigger->probability = __read_f32( _reader );
    _trigger->spawn_count_min = __read_u32( _reader );
    _trigger->spawn_count_max = __read_u32( _reader );
    _trigger->delay_min = __read_f32( _reader );
    _trigger->delay_max = __read_f32( _reader );
    _trigger->inherit_position = (dz_bool_t)__read_u32( _reader );
    _trigger->inherit_angle = (dz_bool_t)__read_u32( _reader );
    _trigger->inherit_velocity = (dz_bool_t)__read_u32( _reader );
    _trigger->offset_x = __read_f32( _reader );
    _trigger->offset_y = __read_f32( _reader );
    _trigger->angle_offset = __read_f32( _reader );
    return _reader->result;
}

static void __read_mesh( const dz_service_t * _service, dz_reader_t * _reader, dz_effect_t * _effect )
{
    dz_mesh_desc_t mesh;
    dz_memory_zero( &mesh, sizeof( mesh ) );
    mesh.id = __read_u32( _reader );
    mesh.vertex_count = __read_u32( _reader );
    mesh.index_count = __read_u32( _reader );
    dz_mesh_vertex_t * vertices = DZ_REALLOCN( _service, DZ_NULLPTR, dz_mesh_vertex_t, mesh.vertex_count );
    dz_uint32_t * indices = DZ_REALLOCN( _service, DZ_NULLPTR, dz_uint32_t, mesh.index_count );

    for( dz_uint32_t index = 0; index != mesh.vertex_count; ++index )
    {
        dz_mesh_vertex_t * vertex = vertices + index;
        vertex->position = __read_vec3( _reader );
        vertex->normal = __read_vec3( _reader );
        vertex->tangent.x = __read_f32( _reader );
        vertex->tangent.y = __read_f32( _reader );
        vertex->tangent.z = __read_f32( _reader );
        vertex->tangent.w = __read_f32( _reader );
        vertex->uv0.x = __read_f32( _reader );
        vertex->uv0.y = __read_f32( _reader );
        vertex->uv1.x = __read_f32( _reader );
        vertex->uv1.y = __read_f32( _reader );
    }
    for( dz_uint32_t index = 0; index != mesh.index_count; ++index )
    {
        indices[index] = __read_u32( _reader );
    }

    mesh.vertices = vertices;
    mesh.indices = indices;
    dz_effect_add_mesh( _service, _effect, &mesh );
    DZ_FREE( _service, indices );
    DZ_FREE( _service, vertices );
}

static dz_result_t __read_physics( dz_reader_t * _reader, dz_physics_object_desc_t * _object )
{
    _object->id = __read_u32( _reader );
    _object->mesh_id = __read_u32( _reader );
    _object->type = (dz_physics_object_type_e)__read_u32( _reader );
    _object->transform = __read_transform( _reader );
    _object->direction = __read_vec3( _reader );
    _object->half_extents = __read_vec3( _reader );
    _object->radius = __read_f32( _reader );
    _object->strength = __read_f32( _reader );
    _object->falloff = __read_f32( _reader );
    _object->turbulence = __read_f32( _reader );
    _object->restitution = __read_f32( _reader );
    _object->friction = __read_f32( _reader );
    _object->response = (dz_collision_response_e)__read_u32( _reader );
    return _reader->result;
}

static dz_result_t __read_decode_effect( const dz_service_t * _service, dz_effect_t ** _effect, dz_stream_read_t _read, dz_userdata_t _ud )
{
    dz_reader_t reader = { _read, _ud, DZ_SUCCESSFUL };
    __read_u32( &reader );
    __read_u32( &reader );

    dz_project_profile_t profile;
    profile.projection = (dz_projection_type_e)__read_u32( &reader );
    profile.position = __read_vec3( &reader );
    profile.forward = __read_vec3( &reader );
    profile.up = __read_vec3( &reader );
    profile.field_of_view = __read_f32( &reader );
    profile.orthographic_height = __read_f32( &reader );
    profile.near_plane = __read_f32( &reader );
    profile.far_plane = __read_f32( &reader );
    const dz_float_t life = __read_f32( &reader );
    const dz_uint32_t seed = __read_u32( &reader );
    const dz_uint32_t has_atlas = __read_u32( &reader );

    dz_effect_t * effect;
    dz_effect_create_with_profile( _service, &effect, &profile, life, seed, DZ_NULLPTR );
    if( has_atlas )
    {
        dz_atlas_t * atlas;
        dz_atlas_create( _service, &atlas, DZ_NULLPTR, DZ_NULLPTR );
        effect->atlas = atlas;
    }

    const dz_uint32_t mesh_count = __read_u32( &reader );
    for( dz_uint32_t i = 0; i != mesh_count; ++i )
    {
        __read_mesh( _service, &reader, effect );
    }

    const dz_uint32_t layer_count = __read_u32( &reader );
    for( dz_uint32_t i = 0; i != layer_count; ++i )
    {
        dz_effect_layer_desc_t layer;
        __read_layer( _service, &reader, effect->atlas, &layer );
        dz_effect_add_layer( effect, &layer, DZ_NULLPTR );
    }

    const dz_uint32_t trigger_count = __read_u32( &reader );
    for( dz_uint32_t i = 0; i != trigger_count; ++i )
    {
        dz_effect_trigger_desc_t trigger;
        __read_trigger( &reader, &trigger );
        dz_effect_add_trigger( effect, &trigger, DZ_NULLPTR );
    }

    const dz_uint32_t physics_count = __read_u32( &reader );
    for( dz_uint32_t i = 0; i != physics_count; ++i )
    {
        dz_physics_object_desc_t object;
        __read_physics( &reader, &object );
        dz_effect_add_physics_object( _service, effect, &object, DZ_NULLPTR );
    }

    *_effect = effect;
    return reader.result;
}

dz_result_t dz_effect_read( const dz_service_t * _service, dz_effect_t ** _effect, dz_stream_read_t _read, dz_userdata_t _ud )
{
    dz_uint8_t header[32];
    dz_result_t result = ( *_read )( header, sizeof( header ), _ud );
    if( result != DZ_SUCCESSFUL )
    {
        return result;
    }

    const dz_uint32_t magic = __load_u32( header );
    const dz_uint32_t version = __load_u32( header + 4U );

    if( magic != dz_get_magic() )
    {
        return DZ_FAILURE_INVALID_DATA;
    }

    if( version != dz_get_version() )
    {
        return DZ_FAILURE_INVALID_VERSION;
    }

    return __read_decode_effect( _service, _effect, _read, _ud );
}
