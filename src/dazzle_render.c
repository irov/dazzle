#include "dazzle/dazzle.h"

#include "atlas.h"
#include "instance.h"
#include "material.h"
#include "math3d.h"
#include "texture.h"

#include <math.h>
#include <stdint.h>
#include <string.h>

#define DZ_RENDER_PARTICLE_MAX 10922U

typedef struct dz_render_order_t
{
    dz_uint16_t particle_index;
    dz_float_t key;
} dz_render_order_t;

//////////////////////////////////////////////////////////////////////////
static void __resolve_camera( const dz_instance_t * _instance, const dz_camera_state_t * _override, dz_camera_state_t * _camera )
{
    if( _override == DZ_NULLPTR )
    {
        dz_camera_state_from_profile( &_instance->effect->profile, 1.f, 1.f, _camera );
        return;
    }

    *_camera = *_override;
}
//////////////////////////////////////////////////////////////////////////
static void __particle_counts( const dz_effect_t * _effect, const dz_effect_layer_desc_t * _layer, dz_uint32_t * _vertices, dz_uint32_t * _indices )
{
    if( _layer->particle_mode == DZ_PARTICLE_MODE_MESH && _layer->mesh_id != DZ_RESOURCE_ID_NONE )
    {
        dz_mesh_desc_t mesh;
        dz_effect_get_mesh( _effect, _layer->mesh_id, &mesh );

        *_vertices = mesh.vertex_count;
        *_indices = mesh.index_count;
        return;
    }

    *_vertices = _layer->particle_mode == DZ_PARTICLE_MODE_MESH ? 8U : 4U;
    *_indices = _layer->particle_mode == DZ_PARTICLE_MODE_MESH ? 36U : 6U;
}
//////////////////////////////////////////////////////////////////////////
static dz_float_t __sort_key( const dz_particle_t * _particle, const dz_effect_layer_desc_t * _layer, const dz_camera_state_t * _camera )
{
    const dz_float_t depth = ( _particle->x - _camera->position.x ) * _camera->forward.x + ( _particle->y - _camera->position.y ) * _camera->forward.y +
                             ( _particle->z - _camera->position.z ) * _camera->forward.z;

    switch( _layer->sorting )
    {
    case DZ_PARTICLE_SORT_BIRTH_ASCENDING:
        return -(dz_float_t)_particle->birth_order;
    case DZ_PARTICLE_SORT_BIRTH_DESCENDING:
        return (dz_float_t)_particle->birth_order;
    case DZ_PARTICLE_SORT_CAMERA_NEAR:
        return -depth;
    case DZ_PARTICLE_SORT_CAMERA_FAR:
        return depth;
    case DZ_PARTICLE_SORT_NONE:
    default:
        return _layer->material->blend_type == DZ_BLEND_ADD ? (dz_float_t)_particle->birth_order : depth;
    }
}
//////////////////////////////////////////////////////////////////////////
static void __build_order( const dz_instance_t * _instance, const dz_camera_state_t * _camera, dz_render_order_t * _order )
{
    for( dz_uint16_t index = 0; index != _instance->partices_count; ++index )
    {
        const dz_particle_t * particle = _instance->partices + index;
        const dz_effect_layer_desc_t * layer = _instance->effect->layers + particle->layer_index;
        dz_render_order_t entry = { index, __sort_key( particle, layer, _camera ) };
        dz_uint16_t insert = index;

        while( insert != 0 && _order[insert - 1].key < entry.key )
        {
            _order[insert] = _order[insert - 1];
            --insert;
        }

        _order[insert] = entry;
    }
}
//////////////////////////////////////////////////////////////////////////
static void __compute_requirements( const dz_instance_t * _instance, const dz_camera_state_t * _camera, dz_render_requirements_t * _requirements, dz_render_order_t * _order )
{
    memset( _requirements, 0, sizeof( *_requirements ) );
    _requirements->vertex_semantics =
        DZ_VERTEX_SEMANTIC_POSITION | DZ_VERTEX_SEMANTIC_NORMAL | DZ_VERTEX_SEMANTIC_TANGENT | DZ_VERTEX_SEMANTIC_COLOR | DZ_VERTEX_SEMANTIC_UV0 | DZ_VERTEX_SEMANTIC_UV1;
    _requirements->index_type = DZ_INDEX_UINT16;

    if( _instance->partices_count == 0 )
    {
        return;
    }

    __build_order( _instance, _camera, _order );
    const dz_material_t * previous_material = DZ_NULLPTR;

    for( dz_uint16_t order_index = 0; order_index != _instance->partices_count; ++order_index )
    {
        const dz_particle_t * particle = _instance->partices + _order[order_index].particle_index;
        const dz_effect_layer_desc_t * layer = _instance->effect->layers + particle->layer_index;
        dz_uint32_t vertices;
        dz_uint32_t indices;
        __particle_counts( _instance->effect, layer, &vertices, &indices );

        _requirements->vertex_count += vertices;
        _requirements->index_count += indices;

        if( layer->material != previous_material )
        {
            _requirements->chunk_count += layer->material->pass_count;
            previous_material = layer->material;
        }
    }

    _requirements->index_type = _requirements->vertex_count > UINT16_MAX ? DZ_INDEX_UINT32 : DZ_INDEX_UINT16;
}
//////////////////////////////////////////////////////////////////////////
void dz_instance_prepare_render( const dz_instance_t * _instance, const dz_camera_state_t * _camera, dz_render_requirements_t * _requirements )
{
    dz_camera_state_t camera;
    __resolve_camera( _instance, _camera, &camera );

    dz_render_order_t order[DZ_RENDER_PARTICLE_MAX];
    __compute_requirements( _instance, &camera, _requirements, order );
}
//////////////////////////////////////////////////////////////////////////
static dz_bool_t __stream_fits( const dz_render_stream_t * _stream, dz_uint32_t _count, dz_size_t _element_size )
{
    if( _count == 0 )
    {
        return DZ_TRUE;
    }
    if( _stream->buffer == DZ_NULLPTR || _stream->stride < _element_size || _stream->offset > _stream->size )
    {
        return DZ_FALSE;
    }

    const dz_size_t available = _stream->size - _stream->offset;
    if( available < _element_size )
    {
        return DZ_FALSE;
    }
    return (dz_size_t)( _count - 1U ) <= ( available - _element_size ) / _stream->stride ? DZ_TRUE : DZ_FALSE;
}
//////////////////////////////////////////////////////////////////////////
static void __write_stream( const dz_render_stream_t * _stream, dz_uint32_t _index, const void * _value, dz_size_t _size )
{
    dz_uint8_t * target = (dz_uint8_t *)_stream->buffer + _stream->offset + _stream->stride * _index;
    memcpy( target, _value, _size );
}
//////////////////////////////////////////////////////////////////////////
static void __write_vertex( const dz_render_buffers_t * _buffers, dz_uint32_t _index, dz_vec3_t _position, dz_vec3_t _normal, dz_vec4_t _tangent, dz_vec4_t _color, dz_vec2_t _uv )
{
    __write_stream( &_buffers->positions, _index, &_position, sizeof( _position ) );
    __write_stream( &_buffers->normals, _index, &_normal, sizeof( _normal ) );
    __write_stream( &_buffers->tangents, _index, &_tangent, sizeof( _tangent ) );
    __write_stream( &_buffers->colors, _index, &_color, sizeof( _color ) );
    __write_stream( &_buffers->uv0, _index, &_uv, sizeof( _uv ) );
    __write_stream( &_buffers->uv1, _index, &_uv, sizeof( _uv ) );
}
//////////////////////////////////////////////////////////////////////////
static void __write_index( const dz_render_buffers_t * _buffers, dz_uint32_t _index, dz_uint32_t _value )
{
    if( _buffers->index_type == DZ_INDEX_UINT32 )
    {
        ( (dz_uint32_t *)_buffers->indices )[_index] = _value;
    }
    else
    {
        ( (dz_uint16_t *)_buffers->indices )[_index] = (dz_uint16_t)_value;
    }
}
//////////////////////////////////////////////////////////////////////////
static void __sprite_basis( const dz_particle_t * _particle, const dz_effect_layer_desc_t * _layer, const dz_camera_state_t * _camera, dz_vec3_t * _right, dz_vec3_t * _up )
{
    const dz_vec3_t camera_right = dz_math_normalize3( dz_math_cross3( _camera->forward, _camera->up ), dz_math_vec3( 1.f, 0.f, 0.f ) );
    const dz_vec3_t camera_up = dz_math_normalize3( _camera->up, dz_math_vec3( 0.f, 1.f, 0.f ) );

    switch( _layer->orientation )
    {
    case DZ_PARTICLE_ORIENTATION_CAMERA_AXIS:
        *_up = dz_math_normalize3( _layer->orientation_axis, camera_up );
        *_right = dz_math_normalize3( dz_math_cross3( _camera->forward, *_up ), camera_right );
        break;
    case DZ_PARTICLE_ORIENTATION_VELOCITY:
        *_up = dz_math_normalize3( dz_math_vec3( _particle->vx, _particle->vy, _particle->vz ), camera_up );
        *_right = dz_math_normalize3( dz_math_cross3( _camera->forward, *_up ), camera_right );
        break;
    case DZ_PARTICLE_ORIENTATION_WORLD:
        *_right = dz_math_quat_rotate3( _layer->rotation, dz_math_vec3( 1.f, 0.f, 0.f ) );
        *_up = dz_math_quat_rotate3( _layer->rotation, dz_math_vec3( 0.f, 1.f, 0.f ) );
        break;
    case DZ_PARTICLE_ORIENTATION_CAMERA:
    default:
        *_right = camera_right;
        *_up = camera_up;
        break;
    }

    const dz_float_t c = cosf( _particle->spin );
    const dz_float_t s = sinf( _particle->spin );
    const dz_vec3_t old_right = *_right;
    *_right = dz_math_add3( dz_math_mul3( old_right, c ), dz_math_mul3( *_up, s ) );
    *_up = dz_math_add3( dz_math_mul3( old_right, -s ), dz_math_mul3( *_up, c ) );
}
//////////////////////////////////////////////////////////////////////////
static void __fill_quad( const dz_particle_t * _particle, const dz_effect_layer_desc_t * _layer, const dz_camera_state_t * _camera, const dz_render_buffers_t * _buffers,
                         dz_uint32_t _vo, dz_uint32_t _io )
{
    dz_vec3_t right;
    dz_vec3_t up;
    __sprite_basis( _particle, _layer, _camera, &right, &up );
    dz_vec3_t center = dz_math_vec3( _particle->x, _particle->y, _particle->z );

    if( _layer->particle_mode == DZ_PARTICLE_MODE_SPRITE )
    {
        dz_float_t width = DZ_PARTICLE_SIZE * _particle->scale * _particle->aspect;
        dz_float_t height = DZ_PARTICLE_SIZE * _particle->scale;
        if( _particle->texture != DZ_NULLPTR )
        {
            width = _particle->texture->width * _particle->scale * _particle->aspect;
            height = _particle->texture->height * _particle->scale;
        }
        right = dz_math_mul3( right, width * 0.5f );
        up = dz_math_mul3( up, height * 0.5f );
    }
    else
    {
        dz_vec3_t start = dz_math_vec3( _particle->previous_x, _particle->previous_y, _particle->previous_z );
        if( _layer->particle_mode == DZ_PARTICLE_MODE_TRAIL )
        {
            const dz_float_t trail_age = DZ_MIN( DZ_MAX( _particle->time, 0.f ), _layer->trail_lifetime );
            start = dz_math_sub3( center, dz_math_mul3( dz_math_vec3( _particle->vx, _particle->vy, _particle->vz ), trail_age ) );
        }
        else if( _layer->particle_mode == DZ_PARTICLE_MODE_BEAM )
        {
            start = dz_math_vec3( _particle->birth_x, _particle->birth_y, _particle->birth_z );
        }
        dz_vec3_t segment = dz_math_sub3( center, start );
        if( dz_math_length3( segment ) < 0.000001f )
        {
            segment = dz_math_mul3( dz_math_normalize3( dz_math_vec3( _particle->vx, _particle->vy, _particle->vz ), up ), -_layer->trail_lifetime );
        }
        center = dz_math_mul3( dz_math_add3( start, center ), 0.5f );
        up = dz_math_mul3( segment, 0.5f );
        right = dz_math_mul3( dz_math_normalize3( dz_math_cross3( segment, _camera->forward ), right ), _layer->trail_width * _particle->scale * 0.5f );
    }

    const dz_vec3_t normal = dz_math_normalize3( dz_math_cross3( right, up ), dz_math_mul3( _camera->forward, -1.f ) );
    const dz_vec3_t tangent_direction = dz_math_normalize3( right, dz_math_vec3( 1.f, 0.f, 0.f ) );
    const dz_vec4_t tangent = { tangent_direction.x, tangent_direction.y, tangent_direction.z, 1.f };
    const dz_vec4_t color = { _particle->color_r, _particle->color_g, _particle->color_b, _particle->color_a };
    const dz_vec3_t positions[4] = { dz_math_sub3( dz_math_sub3( center, right ), up ), dz_math_add3( dz_math_sub3( center, up ), right ),
                                     dz_math_add3( dz_math_add3( center, right ), up ), dz_math_add3( dz_math_sub3( center, right ), up ) };
    const dz_vec2_t uvs[4] = { { 0.f, 0.f }, { 1.f, 0.f }, { 1.f, 1.f }, { 0.f, 1.f } };
    static const dz_uint8_t indices[6] = { 0, 1, 3, 3, 1, 2 };

    for( dz_uint32_t i = 0; i != 4; ++i )
    {
        __write_vertex( _buffers, _vo + i, positions[i], normal, tangent, color, uvs[i] );
    }
    for( dz_uint32_t i = 0; i != 6; ++i )
    {
        __write_index( _buffers, _io + i, _vo + indices[i] );
    }
}
//////////////////////////////////////////////////////////////////////////
static dz_vec3_t __rotate_spin( dz_vec3_t _value, dz_float_t _spin )
{
    const dz_float_t c = cosf( _spin );
    const dz_float_t s = sinf( _spin );
    return dz_math_vec3( _value.x * c - _value.y * s, _value.x * s + _value.y * c, _value.z );
}
//////////////////////////////////////////////////////////////////////////
static void __fill_mesh( const dz_effect_t * _effect, const dz_particle_t * _particle, const dz_effect_layer_desc_t * _layer, const dz_render_buffers_t * _buffers, dz_uint32_t _vo,
                         dz_uint32_t _io )
{
    if( _layer->mesh_id != DZ_RESOURCE_ID_NONE )
    {
        dz_mesh_desc_t mesh;
        if( dz_effect_get_mesh( _effect, _layer->mesh_id, &mesh ) == DZ_SUCCESSFUL )
        {
            const dz_vec3_t center = dz_math_vec3( _particle->x, _particle->y, _particle->z );
            const dz_vec4_t color = { _particle->color_r, _particle->color_g, _particle->color_b, _particle->color_a };
            const dz_vec3_t scale = { _layer->scale.x * _particle->scale * _particle->aspect, _layer->scale.y * _particle->scale, _layer->scale.z * _particle->scale };

            for( dz_uint32_t index = 0; index != mesh.vertex_count; ++index )
            {
                const dz_mesh_vertex_t * source = mesh.vertices + index;
                dz_vec3_t position = { source->position.x * scale.x, source->position.y * scale.y, source->position.z * scale.z };
                position = dz_math_quat_rotate3( _layer->rotation, __rotate_spin( position, _particle->spin ) );
                position = dz_math_add3( center, position );

                dz_vec3_t normal = { scale.x != 0.f ? source->normal.x / scale.x : source->normal.x, scale.y != 0.f ? source->normal.y / scale.y : source->normal.y,
                                     scale.z != 0.f ? source->normal.z / scale.z : source->normal.z };
                normal = dz_math_normalize3( dz_math_quat_rotate3( _layer->rotation, __rotate_spin( normal, _particle->spin ) ), dz_math_vec3( 0.f, 1.f, 0.f ) );

                dz_vec3_t tangent3 = dz_math_vec3( source->tangent.x, source->tangent.y, source->tangent.z );
                tangent3 = dz_math_normalize3( dz_math_quat_rotate3( _layer->rotation, __rotate_spin( tangent3, _particle->spin ) ), dz_math_vec3( 1.f, 0.f, 0.f ) );
                const dz_vec4_t tangent = { tangent3.x, tangent3.y, tangent3.z, source->tangent.w };
                __write_vertex( _buffers, _vo + index, position, normal, tangent, color, source->uv0 );
                __write_stream( &_buffers->uv1, _vo + index, &source->uv1, sizeof( source->uv1 ) );
            }

            for( dz_uint32_t index = 0; index != mesh.index_count; ++index )
            {
                __write_index( _buffers, _io + index, _vo + mesh.indices[index] );
            }
            return;
        }
    }

    const dz_float_t half = DZ_PARTICLE_SIZE * _particle->scale * 0.5f;
    const dz_vec3_t center = dz_math_vec3( _particle->x, _particle->y, _particle->z );
    const dz_vec4_t color = { _particle->color_r, _particle->color_g, _particle->color_b, _particle->color_a };

    for( dz_uint32_t i = 0; i != 8; ++i )
    {
        dz_vec3_t corner = dz_math_vec3( ( i & 1U ) ? half : -half, ( i & 2U ) ? half : -half, ( i & 4U ) ? half : -half );
        corner.x *= _layer->scale.x;
        corner.y *= _layer->scale.y;
        corner.z *= _layer->scale.z;
        corner = dz_math_quat_rotate3( _layer->rotation, corner );
        const dz_vec3_t normal = dz_math_normalize3( corner, dz_math_vec3( 0.f, 1.f, 0.f ) );
        const dz_vec4_t tangent = { 1.f, 0.f, 0.f, 1.f };
        const dz_vec2_t uv = { ( i & 1U ) ? 1.f : 0.f, ( i & 2U ) ? 1.f : 0.f };
        __write_vertex( _buffers, _vo + i, dz_math_add3( center, corner ), normal, tangent, color, uv );
    }

    static const dz_uint8_t indices[36] = { 0, 2, 1, 1, 2, 3, 4, 5, 6, 5, 7, 6, 0, 1, 4, 1, 5, 4, 2, 6, 3, 3, 6, 7, 0, 4, 2, 2, 4, 6, 1, 3, 5, 3, 7, 5 };
    for( dz_uint32_t i = 0; i != 36; ++i )
    {
        __write_index( _buffers, _io + i, _vo + indices[i] );
    }
}
//////////////////////////////////////////////////////////////////////////
static void __emit_group( const dz_material_t * _material, dz_uint32_t _vo, dz_uint32_t _vc, dz_uint32_t _io, dz_uint32_t _ic, dz_float_t _key, dz_render_chunk_t * _chunks,
                          dz_uint32_t * _count )
{
    for( dz_uint32_t pass = 0; pass != _material->pass_count; ++pass )
    {
        dz_render_chunk_t * chunk = _chunks + ( *_count )++;
        chunk->vertex_offset = _vo;
        chunk->vertex_count = _vc;
        chunk->index_offset = _io;
        chunk->index_count = _ic;
        chunk->material_pass = pass;
        chunk->sort_key = _key;
        chunk->primitive = DZ_PRIMITIVE_TRIANGLES;
        chunk->pass = _material->passes[pass];
        chunk->surface = _material->atlas != DZ_NULLPTR ? _material->atlas->surface : DZ_NULLPTR;
    }
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_instance_fill_render( const dz_instance_t * _instance, const dz_camera_state_t * _camera, const dz_render_buffers_t * _buffers, dz_render_chunk_t * _chunks,
                                     dz_uint32_t _chunk_capacity, dz_uint32_t * _chunk_count )
{
    *_chunk_count = 0;

    dz_camera_state_t camera;
    __resolve_camera( _instance, _camera, &camera );

    dz_render_order_t order[DZ_RENDER_PARTICLE_MAX];
    dz_render_requirements_t requirements;
    __compute_requirements( _instance, &camera, &requirements, order );

    const dz_size_t index_size = requirements.index_type == DZ_INDEX_UINT32 ? sizeof( dz_uint32_t ) : sizeof( dz_uint16_t );
    if( _chunk_capacity < requirements.chunk_count || ( requirements.chunk_count && _chunks == DZ_NULLPTR ) || _buffers->index_type != requirements.index_type ||
        __stream_fits( &_buffers->positions, requirements.vertex_count, sizeof( dz_vec3_t ) ) == DZ_FALSE ||
        __stream_fits( &_buffers->normals, requirements.vertex_count, sizeof( dz_vec3_t ) ) == DZ_FALSE ||
        __stream_fits( &_buffers->tangents, requirements.vertex_count, sizeof( dz_vec4_t ) ) == DZ_FALSE ||
        __stream_fits( &_buffers->colors, requirements.vertex_count, sizeof( dz_vec4_t ) ) == DZ_FALSE ||
        __stream_fits( &_buffers->uv0, requirements.vertex_count, sizeof( dz_vec2_t ) ) == DZ_FALSE ||
        __stream_fits( &_buffers->uv1, requirements.vertex_count, sizeof( dz_vec2_t ) ) == DZ_FALSE ||
        ( requirements.index_count && ( _buffers->indices == DZ_NULLPTR || requirements.index_count > _buffers->indices_size / index_size ) ) )
    {
        return DZ_FAILURE_BUFFER_TOO_SMALL;
    }

    dz_uint32_t vo = 0, io = 0, group_vo = 0, group_io = 0, chunk_count = 0;
    dz_float_t group_key = 0.f;
    const dz_material_t * group_material = DZ_NULLPTR;

    for( dz_uint16_t oi = 0; oi != _instance->partices_count; ++oi )
    {
        const dz_particle_t * particle = _instance->partices + order[oi].particle_index;
        const dz_effect_layer_desc_t * layer = _instance->effect->layers + particle->layer_index;

        if( group_material != DZ_NULLPTR && group_material != layer->material )
        {
            __emit_group( group_material, group_vo, vo - group_vo, group_io, io - group_io, group_key, _chunks, &chunk_count );
            group_vo = vo;
            group_io = io;
        }

        if( group_material != layer->material )
        {
            group_material = layer->material;
            group_key = order[oi].key;
        }

        if( layer->particle_mode == DZ_PARTICLE_MODE_MESH )
        {
            __fill_mesh( _instance->effect, particle, layer, _buffers, vo, io );
        }
        else
        {
            __fill_quad( particle, layer, &camera, _buffers, vo, io );
        }

        dz_uint32_t pvc, pic;
        __particle_counts( _instance->effect, layer, &pvc, &pic );
        vo += pvc;
        io += pic;
    }

    if( group_material != DZ_NULLPTR )
    {
        __emit_group( group_material, group_vo, vo - group_vo, group_io, io - group_io, group_key, _chunks, &chunk_count );
    }

    *_chunk_count = chunk_count;
    return DZ_SUCCESSFUL;
}
