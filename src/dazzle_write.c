#include "dazzle/dazzle_write.h"

#include "material.h"
#include "memory.h"
#include "atlas.h"
#include "texture.h"
#include "timeline_interpolate.h"
#include "timeline_key.h"
#include "shape.h"
#include "emitter.h"
#include "affector.h"
#include "effect.h"

//////////////////////////////////////////////////////////////////////////
dz_result_t dz_header_write( dz_stream_write_t _write, dz_userdata_t _ud )
{
    DZ_UNUSED( _write );
    DZ_UNUSED( _ud );
    return DZ_FAILURE_UNSUPPORTED;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_effect_write( const dz_effect_t * _effect, dz_stream_write_t _write, dz_userdata_t _ud )
{
    DZ_UNUSED( _effect );
    DZ_UNUSED( _write );
    DZ_UNUSED( _ud );
    return DZ_FAILURE_UNSUPPORTED;
}
//////////////////////////////////////////////////////////////////////////

typedef struct dz_writer_t
{
    dz_uint8_t * data;
    dz_size_t capacity;
    dz_size_t size;
    dz_result_t result;
} dz_writer_t;

static void __write_bytes( dz_writer_t * _writer, const void * _data, dz_size_t _size )
{
    if( _writer->result != DZ_SUCCESSFUL )
    {
        return;
    }

    if( _writer->data != DZ_NULLPTR )
    {
        if( _writer->size + _size > _writer->capacity )
        {
            _writer->result = DZ_FAILURE_BUFFER_TOO_SMALL;
            return;
        }
        dz_memory_copy( _writer->data + _writer->size, _data, _size );
    }

    _writer->size += _size;
}

static void __write_u32( dz_writer_t * _writer, dz_uint32_t _value )
{
    const dz_uint8_t bytes[4] = { (dz_uint8_t)_value, (dz_uint8_t)( _value >> 8 ), (dz_uint8_t)( _value >> 16 ), (dz_uint8_t)( _value >> 24 ) };
    __write_bytes( _writer, bytes, sizeof( bytes ) );
}

static void __write_f32( dz_writer_t * _writer, dz_float_t _value )
{
    dz_uint32_t bits;
    dz_memory_copy( &bits, &_value, sizeof( bits ) );
    __write_u32( _writer, bits );
}

static void __write_vec3( dz_writer_t * _writer, dz_vec3_t _value )
{
    __write_f32( _writer, _value.x );
    __write_f32( _writer, _value.y );
    __write_f32( _writer, _value.z );
}

static void __write_quat( dz_writer_t * _writer, dz_quat_t _value )
{
    __write_f32( _writer, _value.x );
    __write_f32( _writer, _value.y );
    __write_f32( _writer, _value.z );
    __write_f32( _writer, _value.w );
}

static void __write_transform( dz_writer_t * _writer, const dz_transform_t * _transform )
{
    __write_vec3( _writer, _transform->position );
    __write_quat( _writer, _transform->rotation );
    __write_vec3( _writer, _transform->scale );
}

static dz_uint32_t __write_timeline_count( const dz_timeline_key_t * _key )
{
    dz_uint32_t count = 0;
    while( _key != DZ_NULLPTR )
    {
        ++count;
        _key = _key->interpolate != DZ_NULLPTR ? _key->interpolate->key : DZ_NULLPTR;
    }
    return count;
}

static void __write_timeline( dz_writer_t * _writer, const dz_timeline_key_t * _key )
{
    const dz_uint32_t count = __write_timeline_count( _key );
    __write_u32( _writer, count );

    for( dz_uint32_t index = 0; index != count; ++index )
    {
        const dz_timeline_interpolate_t * interpolate = _key->interpolate;
        __write_f32( _writer, _key->p );
        __write_u32( _writer, (dz_uint32_t)_key->type );
        __write_f32( _writer, _key->const_value );
        __write_f32( _writer, _key->randomize_min_value );
        __write_f32( _writer, _key->randomize_max_value );
        __write_u32( _writer, interpolate != DZ_NULLPTR ? 1U : 0U );
        if( interpolate != DZ_NULLPTR )
        {
            __write_u32( _writer, (dz_uint32_t)interpolate->type );
            __write_f32( _writer, interpolate->p0 );
            __write_f32( _writer, interpolate->p1 );
            __write_f32( _writer, interpolate->out_tangent );
            __write_f32( _writer, interpolate->in_tangent );
        }
        _key = interpolate != DZ_NULLPTR ? interpolate->key : DZ_NULLPTR;
    }
}

static void __write_texture( dz_writer_t * _writer, const dz_material_texture_t * _texture )
{
    for( dz_uint32_t i = 0; i != 4; ++i )
    {
        __write_f32( _writer, _texture->texture->u[i] );
    }
    for( dz_uint32_t i = 0; i != 4; ++i )
    {
        __write_f32( _writer, _texture->texture->v[i] );
    }
    __write_f32( _writer, _texture->texture->width );
    __write_f32( _writer, _texture->texture->height );
    __write_f32( _writer, _texture->texture->trim_offset_x );
    __write_f32( _writer, _texture->texture->trim_offset_y );
    __write_f32( _writer, _texture->texture->trim_width );
    __write_f32( _writer, _texture->texture->trim_height );
    __write_f32( _writer, _texture->random_weight );
    __write_f32( _writer, _texture->texture->sequence_delay );
}

static void __write_material( dz_writer_t * _writer, const dz_material_t * _material )
{
    __write_u32( _writer, (dz_uint32_t)_material->blend_type );
    __write_f32( _writer, _material->r );
    __write_f32( _writer, _material->g );
    __write_f32( _writer, _material->b );
    __write_f32( _writer, _material->a );
    __write_u32( _writer, (dz_uint32_t)_material->mode );
    __write_u32( _writer, _material->texture_index );
    __write_u32( _writer, _material->texture_count );
    __write_u32( _writer, _material->textures_count );
    for( dz_uint32_t i = 0; i != _material->textures_count; ++i )
    {
        __write_texture( _writer, _material->textures + i );
    }

    __write_u32( _writer, _material->pass_count );
    for( dz_uint32_t i = 0; i != _material->pass_count; ++i )
    {
        const dz_material_pass_desc_t * pass = _material->passes + i;
        __write_bytes( _writer, pass->technique_id, DZ_TECHNIQUE_ID_MAX );
        __write_u32( _writer, (dz_uint32_t)pass->blend );
        __write_u32( _writer, (dz_uint32_t)pass->depth_test );
        __write_u32( _writer, (dz_uint32_t)pass->depth_write );
        __write_u32( _writer, (dz_uint32_t)pass->depth_compare );
        __write_u32( _writer, (dz_uint32_t)pass->cull );
        __write_u32( _writer, pass->color_mask );
        __write_u32( _writer, pass->uniform_count );
        for( dz_uint32_t uniform_index = 0; uniform_index != pass->uniform_count; ++uniform_index )
        {
            const dz_uniform_desc_t * uniform = pass->uniforms + uniform_index;
            __write_bytes( _writer, uniform->name, DZ_UNIFORM_NAME_MAX );
            __write_u32( _writer, (dz_uint32_t)uniform->semantic );
            __write_u32( _writer, uniform->value_count );
            for( dz_uint32_t value = 0; value != uniform->value_count; ++value )
            {
                __write_f32( _writer, uniform->values[value] );
            }
        }
        __write_u32( _writer, pass->texture_binding_count );
        for( dz_uint32_t binding_index = 0; binding_index != pass->texture_binding_count; ++binding_index )
        {
            const dz_texture_binding_desc_t * binding = pass->texture_bindings + binding_index;
            __write_bytes( _writer, binding->uniform_name, DZ_UNIFORM_NAME_MAX );
            __write_u32( _writer, binding->texture_slot );
            __write_u32( _writer, (dz_uint32_t)binding->min_filter );
            __write_u32( _writer, (dz_uint32_t)binding->mag_filter );
            __write_u32( _writer, (dz_uint32_t)binding->wrap_u );
            __write_u32( _writer, (dz_uint32_t)binding->wrap_v );
        }
    }
}

static void __write_shape( dz_writer_t * _writer, const dz_shape_t * _shape )
{
    __write_u32( _writer, (dz_uint32_t)_shape->type );
    __write_transform( _writer, &_shape->transform );
    __write_vec3( _writer, _shape->dimensions );
    __write_u32( _writer, _shape->mesh_id );

    __write_u32( _writer, (dz_uint32_t)_shape->has_emitter_texture_desc );
    __write_u32( _writer, _shape->emitter_texture_desc.alpha_threshold );
    __write_u32( _writer, _shape->emitter_texture_desc.rgb_threshold );
    __write_u32( _writer, _shape->emitter_texture_desc.strata );
    __write_f32( _writer, _shape->emitter_texture_desc.sample_scale );
    __write_u32( _writer, (dz_uint32_t)_shape->emitter_texture_desc.boundary );
    __write_u32( _writer, (dz_uint32_t)_shape->emitter_texture_desc.compile );

    for( dz_uint32_t i = 0; i != __DZ_SHAPE_TIMELINE_MAX__; ++i )
    {
        __write_timeline( _writer, _shape->timelines[i] );
    }

    __write_u32( _writer, _shape->triangle_count );
    if( _shape->triangle_count != 0 )
    {
        for( dz_uint32_t i = 0; i != _shape->triangle_count * 6U; ++i )
        {
            __write_f32( _writer, _shape->triangles[i] );
        }
    }

    __write_u32( _writer, (dz_uint32_t)_shape->mask_uses_bits );
    __write_u32( _writer, _shape->mask_source.pitch );
    __write_u32( _writer, _shape->mask_source.width );
    __write_u32( _writer, _shape->mask_source.height );
    __write_u32( _writer, _shape->mask_source.channel_count );
    __write_u32( _writer, _shape->mask_source.alpha_channel );
    __write_u32( _writer, _shape->mask_source.alpha_threshold );
    __write_f32( _writer, _shape->mask_scale );
    __write_u32( _writer, _shape->mask_bits_pitch );

    const void * mask_buffer = _shape->mask_uses_bits == DZ_TRUE ? _shape->mask_bits : _shape->mask_source.buffer;
    const dz_uint32_t mask_pitch = _shape->mask_uses_bits == DZ_TRUE ? _shape->mask_bits_pitch : _shape->mask_source.pitch;
    const dz_size_t mask_size = mask_buffer != DZ_NULLPTR ? (dz_size_t)mask_pitch * _shape->mask_source.height : 0U;
    __write_u32( _writer, (dz_uint32_t)mask_size );
    if( mask_size != 0 )
    {
        __write_bytes( _writer, mask_buffer, mask_size );
    }
}

static void __write_emitter( dz_writer_t * _writer, const dz_emitter_t * _emitter )
{
    __write_f32( _writer, _emitter->life );
    for( dz_uint32_t i = 0; i != __DZ_EMITTER_TIMELINE_MAX__; ++i )
    {
        __write_timeline( _writer, _emitter->timelines[i] );
    }
}

static void __write_affector( dz_writer_t * _writer, const dz_affector_t * _affector )
{
    for( dz_uint32_t i = 0; i != __DZ_AFFECTOR_TIMELINE_MAX__; ++i )
    {
        __write_timeline( _writer, _affector->timelines[i] );
    }
}

static void __write_layer( dz_writer_t * _writer, const dz_effect_layer_desc_t * _layer )
{
    __write_material( _writer, _layer->material );
    __write_shape( _writer, _layer->shape );
    __write_emitter( _writer, _layer->emitter );
    __write_affector( _writer, _layer->affector );
    __write_f32( _writer, _layer->x );
    __write_f32( _writer, _layer->y );
    __write_f32( _writer, _layer->z );
    __write_f32( _writer, _layer->angle );
    __write_quat( _writer, _layer->rotation );
    __write_vec3( _writer, _layer->scale );
    __write_u32( _writer, (dz_uint32_t)_layer->particle_mode );
    __write_u32( _writer, (dz_uint32_t)_layer->orientation );
    __write_u32( _writer, (dz_uint32_t)_layer->sorting );
    __write_vec3( _writer, _layer->orientation_axis );
    __write_u32( _writer, _layer->mesh_id );
    __write_f32( _writer, _layer->trail_width );
    __write_f32( _writer, _layer->trail_lifetime );
    __write_f32( _writer, _layer->life );
    __write_u32( _writer, _layer->seed );
}

static void __write_trigger( dz_writer_t * _writer, const dz_effect_trigger_desc_t * _trigger )
{
    __write_u32( _writer, (dz_uint32_t)_trigger->event_type );
    __write_u32( _writer, _trigger->source_layer_index );
    __write_u32( _writer, _trigger->target_layer_index );
    __write_f32( _writer, _trigger->time );
    __write_f32( _writer, _trigger->probability );
    __write_u32( _writer, _trigger->spawn_count_min );
    __write_u32( _writer, _trigger->spawn_count_max );
    __write_f32( _writer, _trigger->delay_min );
    __write_f32( _writer, _trigger->delay_max );
    __write_u32( _writer, (dz_uint32_t)_trigger->inherit_position );
    __write_u32( _writer, (dz_uint32_t)_trigger->inherit_angle );
    __write_u32( _writer, (dz_uint32_t)_trigger->inherit_velocity );
    __write_f32( _writer, _trigger->offset_x );
    __write_f32( _writer, _trigger->offset_y );
    __write_f32( _writer, _trigger->angle_offset );
}

static void __write_mesh( dz_writer_t * _writer, const dz_mesh_desc_t * _mesh )
{
    __write_u32( _writer, _mesh->id );
    __write_u32( _writer, _mesh->vertex_count );
    __write_u32( _writer, _mesh->index_count );
    for( dz_uint32_t index = 0; index != _mesh->vertex_count; ++index )
    {
        const dz_mesh_vertex_t * vertex = _mesh->vertices + index;
        __write_vec3( _writer, vertex->position );
        __write_vec3( _writer, vertex->normal );
        __write_f32( _writer, vertex->tangent.x );
        __write_f32( _writer, vertex->tangent.y );
        __write_f32( _writer, vertex->tangent.z );
        __write_f32( _writer, vertex->tangent.w );
        __write_f32( _writer, vertex->uv0.x );
        __write_f32( _writer, vertex->uv0.y );
        __write_f32( _writer, vertex->uv1.x );
        __write_f32( _writer, vertex->uv1.y );
    }
    for( dz_uint32_t index = 0; index != _mesh->index_count; ++index )
    {
        __write_u32( _writer, _mesh->indices[index] );
    }
}

static void __write_physics( dz_writer_t * _writer, const dz_physics_object_desc_t * _object )
{
    __write_u32( _writer, _object->id );
    __write_u32( _writer, _object->mesh_id );
    __write_u32( _writer, (dz_uint32_t)_object->type );
    __write_transform( _writer, &_object->transform );
    __write_vec3( _writer, _object->direction );
    __write_vec3( _writer, _object->half_extents );
    __write_f32( _writer, _object->radius );
    __write_f32( _writer, _object->strength );
    __write_f32( _writer, _object->falloff );
    __write_f32( _writer, _object->turbulence );
    __write_f32( _writer, _object->restitution );
    __write_f32( _writer, _object->friction );
    __write_u32( _writer, (dz_uint32_t)_object->response );
}

static void __write_effect_payload( dz_writer_t * _writer, const dz_effect_t * _effect )
{
    __write_u32( _writer, (dz_uint32_t)_effect->profile.projection );
    __write_vec3( _writer, _effect->profile.position );
    __write_vec3( _writer, _effect->profile.forward );
    __write_vec3( _writer, _effect->profile.up );
    __write_f32( _writer, _effect->profile.field_of_view );
    __write_f32( _writer, _effect->profile.orthographic_height );
    __write_f32( _writer, _effect->profile.near_plane );
    __write_f32( _writer, _effect->profile.far_plane );
    __write_f32( _writer, _effect->life );
    __write_u32( _writer, _effect->seed );
    __write_u32( _writer, _effect->atlas != DZ_NULLPTR ? 1U : 0U );
    __write_u32( _writer, _effect->mesh_count );
    for( dz_uint32_t i = 0; i != _effect->mesh_count; ++i )
    {
        __write_mesh( _writer, _effect->meshes + i );
    }
    __write_u32( _writer, _effect->layer_count );
    for( dz_uint32_t i = 0; i != _effect->layer_count; ++i )
    {
        __write_layer( _writer, _effect->layers + i );
    }
    __write_u32( _writer, _effect->trigger_count );
    for( dz_uint32_t i = 0; i != _effect->trigger_count; ++i )
    {
        __write_trigger( _writer, _effect->triggers + i );
    }
    __write_u32( _writer, _effect->physics_object_count );
    for( dz_uint32_t i = 0; i != _effect->physics_object_count; ++i )
    {
        __write_physics( _writer, _effect->physics_objects + i );
    }
}

dz_result_t dz_effect_write_memory( const dz_effect_t * _effect, void * _buffer, dz_size_t _capacity, dz_size_t * _written )
{
    *_written = 0;

    dz_writer_t measure = { DZ_NULLPTR, SIZE_MAX, 0, DZ_SUCCESSFUL };
    __write_effect_payload( &measure, _effect );

    const dz_size_t payload_size = measure.size + 8U;
    const dz_size_t total_size = payload_size + 32U;
    *_written = total_size;
    if( _buffer == DZ_NULLPTR || _capacity < total_size )
    {
        return DZ_FAILURE_BUFFER_TOO_SMALL;
    }

    dz_writer_t writer = { (dz_uint8_t *)_buffer, _capacity, 0, DZ_SUCCESSFUL };
    __write_u32( &writer, dz_get_magic() );
    __write_u32( &writer, dz_get_version() );
    __write_u32( &writer, 0x01020304U );
    __write_u32( &writer, 32U );
    __write_u32( &writer, (dz_uint32_t)payload_size );
    __write_u32( &writer, 0U );
    __write_u32( &writer, 1U );
    __write_u32( &writer, 0U );
    __write_u32( &writer, 'E' + ( 'F' << 8 ) + ( 'C' << 16 ) + ( 'T' << 24 ) );
    __write_u32( &writer, (dz_uint32_t)measure.size );
    __write_effect_payload( &writer, _effect );
    if( writer.result != DZ_SUCCESSFUL )
    {
        return writer.result;
    }

    return DZ_SUCCESSFUL;
}
