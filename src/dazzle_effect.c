#include "dazzle/dazzle.h"

#include "affector.h"
#include "alloc.h"
#include "atlas.h"
#include "effect.h"
#include "emitter.h"
#include "material.h"
#include "math3d.h"
#include "shape.h"
#include "texture.h"

#include <string.h>

//////////////////////////////////////////////////////////////////////////
void dz_effect_create( const dz_service_t * _service, dz_effect_t ** _effect, dz_float_t _life, dz_uint32_t _seed, dz_userdata_t _ud )
{
    dz_project_profile_t profile;
    dz_project_profile_default( &profile, DZ_PROJECTION_ORTHOGRAPHIC );
    dz_effect_create_with_profile( _service, _effect, &profile, _life, _seed, _ud );
}
//////////////////////////////////////////////////////////////////////////
void dz_effect_create_with_profile( const dz_service_t * _service, dz_effect_t ** _effect, const dz_project_profile_t * _profile, dz_float_t _life, dz_uint32_t _seed,
                                    dz_userdata_t _ud )
{
    dz_effect_t * effect = DZ_NEW( _service, dz_effect_t );

    effect->profile = *_profile;

    effect->atlas = DZ_NULLPTR;

    effect->layer_count = 0;
    effect->trigger_count = 0;
    effect->mesh_count = 0;
    effect->physics_object_count = 0;

    effect->life = _life;
    effect->seed = _seed;

    effect->ud = _ud;

    *_effect = effect;

}
//////////////////////////////////////////////////////////////////////////
void dz_effect_get_project_profile( const dz_effect_t * _effect, dz_project_profile_t * _profile )
{
    *_profile = _effect->profile;
}
//////////////////////////////////////////////////////////////////////////
void dz_effect_set_camera_defaults( dz_effect_t * _effect, const dz_project_profile_t * _profile )
{
    _effect->profile = *_profile;
}
//////////////////////////////////////////////////////////////////////////
void dz_effect_layer_desc_default( dz_effect_layer_desc_t * _layer )
{
    memset( _layer, 0, sizeof( *_layer ) );
    _layer->rotation = dz_math_quat_identity();
    _layer->scale = dz_math_vec3( 1.f, 1.f, 1.f );
    _layer->particle_mode = DZ_PARTICLE_MODE_SPRITE;
    _layer->orientation = DZ_PARTICLE_ORIENTATION_CAMERA;
    _layer->sorting = DZ_PARTICLE_SORT_NONE;
    _layer->orientation_axis = dz_math_vec3( 0.f, 1.f, 0.f );
    _layer->mesh_id = DZ_RESOURCE_ID_NONE;
    _layer->trail_width = 1.f;
    _layer->trail_lifetime = 0.5f;
}
//////////////////////////////////////////////////////////////////////////
void dz_effect_destroy( const dz_service_t * _service, const dz_effect_t * _effect )
{
    const dz_material_t * materials[DZ_EFFECT_LAYER_MAX];
    const dz_shape_t * shapes[DZ_EFFECT_LAYER_MAX];
    const dz_emitter_t * emitters[DZ_EFFECT_LAYER_MAX];
    const dz_affector_t * affectors[DZ_EFFECT_LAYER_MAX];
    const dz_atlas_t * atlases[DZ_EFFECT_LAYER_MAX + 1];
    const dz_texture_t * textures[DZ_EFFECT_LAYER_MAX * 64];
    dz_uint32_t material_count = 0, shape_count = 0, emitter_count = 0, affector_count = 0, atlas_count = 0, texture_count = 0;

#define DZ_COLLECT_UNIQUE( array, count, value )                                                                                                                                   \
    do                                                                                                                                                                             \
    {                                                                                                                                                                              \
        dz_bool_t found_ = DZ_FALSE;                                                                                                                                               \
        for( dz_uint32_t unique_index_ = 0; unique_index_ != ( count ); ++unique_index_ )                                                                                          \
        {                                                                                                                                                                          \
            if( ( array )[unique_index_] == ( value ) )                                                                                                                            \
            {                                                                                                                                                                      \
                found_ = DZ_TRUE;                                                                                                                                                  \
                break;                                                                                                                                                             \
            }                                                                                                                                                                      \
        }                                                                                                                                                                          \
        if( found_ == DZ_FALSE && ( value ) != DZ_NULLPTR )                                                                                                                        \
        {                                                                                                                                                                          \
            ( array )[( count )++] = ( value );                                                                                                                                    \
        }                                                                                                                                                                          \
    } while( 0 )

    if( _effect->atlas != DZ_NULLPTR )
    {
        atlases[atlas_count++] = _effect->atlas;
    }

    for( dz_uint32_t layer_index = 0; layer_index != _effect->layer_count; ++layer_index )
    {
        const dz_effect_layer_desc_t * layer = _effect->layers + layer_index;
        DZ_COLLECT_UNIQUE( materials, material_count, layer->material );
        DZ_COLLECT_UNIQUE( shapes, shape_count, layer->shape );
        DZ_COLLECT_UNIQUE( emitters, emitter_count, layer->emitter );
        DZ_COLLECT_UNIQUE( affectors, affector_count, layer->affector );
    }

    for( dz_uint32_t material_index = 0; material_index != material_count; ++material_index )
    {
        const dz_material_t * material = materials[material_index];
        DZ_COLLECT_UNIQUE( atlases, atlas_count, material->atlas );
        for( dz_uint32_t texture_index = 0; texture_index != material->textures_count; ++texture_index )
        {
            DZ_COLLECT_UNIQUE( textures, texture_count, material->textures[texture_index].texture );
        }
    }

#undef DZ_COLLECT_UNIQUE

    for( dz_uint32_t index = 0; index != _effect->mesh_count; ++index )
    {
        DZ_FREE( _service, _effect->meshes[index].vertices );
        DZ_FREE( _service, _effect->meshes[index].indices );
        DZ_FREE( _service, _effect->mesh_bvhs[index].nodes );
        DZ_FREE( _service, _effect->mesh_bvhs[index].triangles );
    }

    for( dz_uint32_t index = 0; index != texture_count; ++index )
    {
        dz_texture_destroy( _service, textures[index] );
    }

    for( dz_uint32_t index = 0; index != material_count; ++index )
    {
        dz_material_destroy( _service, materials[index] );
    }

    for( dz_uint32_t index = 0; index != shape_count; ++index )
    {
        dz_shape_destroy( _service, shapes[index] );
    }

    for( dz_uint32_t index = 0; index != emitter_count; ++index )
    {
        dz_emitter_destroy( _service, emitters[index] );
    }

    for( dz_uint32_t index = 0; index != affector_count; ++index )
    {
        dz_affector_destroy( _service, affectors[index] );
    }

    for( dz_uint32_t index = 0; index != atlas_count; ++index )
    {
        dz_atlas_destroy( _service, atlases[index] );
    }

    DZ_FREE( _service, _effect );
}
//////////////////////////////////////////////////////////////////////////
void dz_effect_set_ud( dz_effect_t * const _effect, dz_userdata_t _ud )
{
    _effect->ud = _ud;
}
//////////////////////////////////////////////////////////////////////////
dz_userdata_t dz_effect_get_ud( const dz_effect_t * _effect )
{
    return _effect->ud;
}
//////////////////////////////////////////////////////////////////////////
void dz_effect_set_atlas( dz_effect_t * const _effect, const dz_atlas_t * _atlas )
{
    _effect->atlas = _atlas;
}
//////////////////////////////////////////////////////////////////////////
const dz_atlas_t * dz_effect_get_atlas( const dz_effect_t * _effect )
{
    return _effect->atlas;
}
//////////////////////////////////////////////////////////////////////////
dz_uint32_t dz_effect_find_mesh_index( const dz_effect_t * _effect, dz_uint32_t _id )
{
    for( dz_uint32_t index = 0; index != _effect->mesh_count; ++index )
    {
        if( _effect->meshes[index].id == _id )
        {
            return index;
        }
    }

    return DZ_RESOURCE_ID_NONE;
}
//////////////////////////////////////////////////////////////////////////
const dz_mesh_desc_t * dz_effect_find_mesh( const dz_effect_t * _effect, dz_uint32_t _id )
{
    const dz_uint32_t index = dz_effect_find_mesh_index( _effect, _id );
    return index != DZ_RESOURCE_ID_NONE ? _effect->meshes + index : DZ_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
dz_uint32_t dz_effect_get_mesh_count( const dz_effect_t * _effect )
{
    return _effect->mesh_count;
}
//////////////////////////////////////////////////////////////////////////
static void __mesh_triangle_bounds( const dz_mesh_desc_t * _mesh, dz_uint32_t _triangle, dz_aabb_t * _bounds, dz_vec3_t * _centroid )
{
    const dz_uint32_t offset = _triangle * 3U;
    const dz_vec3_t a = _mesh->vertices[_mesh->indices[offset + 0U]].position;
    const dz_vec3_t b = _mesh->vertices[_mesh->indices[offset + 1U]].position;
    const dz_vec3_t c = _mesh->vertices[_mesh->indices[offset + 2U]].position;

    _bounds->valid = DZ_TRUE;
    _bounds->minimum = dz_math_vec3( DZ_MIN( a.x, DZ_MIN( b.x, c.x ) ), DZ_MIN( a.y, DZ_MIN( b.y, c.y ) ), DZ_MIN( a.z, DZ_MIN( b.z, c.z ) ) );
    _bounds->maximum = dz_math_vec3( DZ_MAX( a.x, DZ_MAX( b.x, c.x ) ), DZ_MAX( a.y, DZ_MAX( b.y, c.y ) ), DZ_MAX( a.z, DZ_MAX( b.z, c.z ) ) );
    *_centroid = dz_math_mul3( dz_math_add3( dz_math_add3( a, b ), c ), 1.f / 3.f );
}
//////////////////////////////////////////////////////////////////////////
static void __mesh_bounds_include( dz_aabb_t * _bounds, const dz_aabb_t * _include )
{
    if( _bounds->valid == DZ_FALSE )
    {
        *_bounds = *_include;
        return;
    }

    _bounds->minimum.x = DZ_MIN( _bounds->minimum.x, _include->minimum.x );
    _bounds->minimum.y = DZ_MIN( _bounds->minimum.y, _include->minimum.y );
    _bounds->minimum.z = DZ_MIN( _bounds->minimum.z, _include->minimum.z );
    _bounds->maximum.x = DZ_MAX( _bounds->maximum.x, _include->maximum.x );
    _bounds->maximum.y = DZ_MAX( _bounds->maximum.y, _include->maximum.y );
    _bounds->maximum.z = DZ_MAX( _bounds->maximum.z, _include->maximum.z );
}
//////////////////////////////////////////////////////////////////////////
static dz_float_t __mesh_centroid_axis( const dz_mesh_desc_t * _mesh, dz_uint32_t _triangle, dz_uint32_t _axis )
{
    dz_aabb_t bounds;
    dz_vec3_t centroid;
    __mesh_triangle_bounds( _mesh, _triangle, &bounds, &centroid );
    return _axis == 0U ? centroid.x : ( _axis == 1U ? centroid.y : centroid.z );
}
//////////////////////////////////////////////////////////////////////////
static dz_uint32_t __mesh_bvh_build_node( const dz_mesh_desc_t * _mesh, dz_mesh_bvh_t * _bvh, dz_uint32_t _first, dz_uint32_t _count )
{
    const dz_uint32_t node_index = _bvh->node_count++;
    dz_mesh_bvh_node_t * node = _bvh->nodes + node_index;
    node->bounds.valid = DZ_FALSE;
    node->first = _first;
    node->count = _count;
    node->left = DZ_RESOURCE_ID_NONE;
    node->right = DZ_RESOURCE_ID_NONE;

    dz_vec3_t centroid_min = dz_math_vec3( DZ_FLT_MAX, DZ_FLT_MAX, DZ_FLT_MAX );
    dz_vec3_t centroid_max = dz_math_vec3( -DZ_FLT_MAX, -DZ_FLT_MAX, -DZ_FLT_MAX );
    for( dz_uint32_t offset = 0; offset != _count; ++offset )
    {
        dz_aabb_t triangle_bounds;
        dz_vec3_t centroid;
        __mesh_triangle_bounds( _mesh, _bvh->triangles[_first + offset], &triangle_bounds, &centroid );
        __mesh_bounds_include( &node->bounds, &triangle_bounds );
        centroid_min.x = DZ_MIN( centroid_min.x, centroid.x );
        centroid_min.y = DZ_MIN( centroid_min.y, centroid.y );
        centroid_min.z = DZ_MIN( centroid_min.z, centroid.z );
        centroid_max.x = DZ_MAX( centroid_max.x, centroid.x );
        centroid_max.y = DZ_MAX( centroid_max.y, centroid.y );
        centroid_max.z = DZ_MAX( centroid_max.z, centroid.z );
    }

    if( _count <= 8U )
    {
        return node_index;
    }

    const dz_vec3_t extent = dz_math_sub3( centroid_max, centroid_min );
    dz_uint32_t axis = 0U;
    if( extent.y > extent.x )
    {
        axis = 1U;
    }
    if( ( axis == 0U ? extent.x : extent.y ) < extent.z )
    {
        axis = 2U;
    }
    const dz_float_t split = 0.5f * ( ( axis == 0U ? centroid_min.x : ( axis == 1U ? centroid_min.y : centroid_min.z ) ) +
                                      ( axis == 0U ? centroid_max.x : ( axis == 1U ? centroid_max.y : centroid_max.z ) ) );

    dz_uint32_t begin = _first;
    dz_uint32_t end = _first + _count;
    while( begin < end )
    {
        if( __mesh_centroid_axis( _mesh, _bvh->triangles[begin], axis ) < split )
        {
            ++begin;
        }
        else
        {
            --end;
            const dz_uint32_t swap = _bvh->triangles[begin];
            _bvh->triangles[begin] = _bvh->triangles[end];
            _bvh->triangles[end] = swap;
        }
    }

    dz_uint32_t left_count = begin - _first;
    if( left_count == 0U || left_count == _count )
    {
        left_count = _count / 2U;
    }

    node->count = 0U;
    node->left = __mesh_bvh_build_node( _mesh, _bvh, _first, left_count );
    node->right = __mesh_bvh_build_node( _mesh, _bvh, _first + left_count, _count - left_count );
    return node_index;
}
//////////////////////////////////////////////////////////////////////////
static void __mesh_bvh_build( const dz_service_t * _service, const dz_mesh_desc_t * _mesh, dz_mesh_bvh_t * _bvh )
{
    const dz_uint32_t triangle_count = _mesh->index_count / 3U;
    _bvh->nodes = DZ_REALLOCN( _service, DZ_NULLPTR, dz_mesh_bvh_node_t, triangle_count * 2U - 1U );
    _bvh->triangles = DZ_REALLOCN( _service, DZ_NULLPTR, dz_uint32_t, triangle_count );

    for( dz_uint32_t triangle = 0; triangle != triangle_count; ++triangle )
    {
        _bvh->triangles[triangle] = triangle;
    }

    _bvh->node_count = 0U;
    (void)__mesh_bvh_build_node( _mesh, _bvh, 0U, triangle_count );
}
//////////////////////////////////////////////////////////////////////////
void dz_effect_add_mesh( const dz_service_t * _service, dz_effect_t * _effect, const dz_mesh_desc_t * _mesh )
{
    dz_mesh_vertex_t * vertices = DZ_REALLOCN( _service, DZ_NULLPTR, dz_mesh_vertex_t, _mesh->vertex_count );
    dz_uint32_t * indices = DZ_REALLOCN( _service, DZ_NULLPTR, dz_uint32_t, _mesh->index_count );

    memcpy( vertices, _mesh->vertices, sizeof( *vertices ) * _mesh->vertex_count );
    memcpy( indices, _mesh->indices, sizeof( *indices ) * _mesh->index_count );

    dz_mesh_desc_t resource = *_mesh;
    resource.vertices = vertices;
    resource.indices = indices;
    resource.bounds.valid = DZ_TRUE;
    resource.bounds.minimum = resource.bounds.maximum = vertices[0].position;
    for( dz_uint32_t index = 1; index != resource.vertex_count; ++index )
    {
        const dz_vec3_t p = vertices[index].position;
        resource.bounds.minimum.x = DZ_MIN( resource.bounds.minimum.x, p.x );
        resource.bounds.minimum.y = DZ_MIN( resource.bounds.minimum.y, p.y );
        resource.bounds.minimum.z = DZ_MIN( resource.bounds.minimum.z, p.z );
        resource.bounds.maximum.x = DZ_MAX( resource.bounds.maximum.x, p.x );
        resource.bounds.maximum.y = DZ_MAX( resource.bounds.maximum.y, p.y );
        resource.bounds.maximum.z = DZ_MAX( resource.bounds.maximum.z, p.z );
    }

    dz_mesh_bvh_t bvh = { DZ_NULLPTR, 0U, DZ_NULLPTR };
    __mesh_bvh_build( _service, &resource, &bvh );

    _effect->meshes[_effect->mesh_count] = resource;
    _effect->mesh_bvhs[_effect->mesh_count] = bvh;
    ++_effect->mesh_count;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_effect_get_mesh( const dz_effect_t * _effect, dz_uint32_t _id, dz_mesh_desc_t * _mesh )
{
    const dz_mesh_desc_t * resource = dz_effect_find_mesh( _effect, _id );
    if( resource == DZ_NULLPTR )
    {
        return DZ_FAILURE_INVALID_DATA;
    }

    *_mesh = *resource;
    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_effect_get_mesh_at( const dz_effect_t * _effect, dz_uint32_t _index, dz_mesh_desc_t * _mesh )
{
    *_mesh = _effect->meshes[_index];
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_effect_remove_mesh( const dz_service_t * _service, dz_effect_t * _effect, dz_uint32_t _id )
{
    for( dz_uint32_t index = 0; index != _effect->mesh_count; ++index )
    {
        if( _effect->meshes[index].id != _id )
        {
            continue;
        }

        DZ_FREE( _service, _effect->meshes[index].vertices );
        DZ_FREE( _service, _effect->meshes[index].indices );
        DZ_FREE( _service, _effect->mesh_bvhs[index].nodes );
        DZ_FREE( _service, _effect->mesh_bvhs[index].triangles );
        for( dz_uint32_t move = index + 1U; move != _effect->mesh_count; ++move )
        {
            _effect->meshes[move - 1U] = _effect->meshes[move];
            _effect->mesh_bvhs[move - 1U] = _effect->mesh_bvhs[move];
        }
        --_effect->mesh_count;
        return DZ_SUCCESSFUL;
    }

    return DZ_FAILURE_INVALID_DATA;
}
//////////////////////////////////////////////////////////////////////////
dz_uint32_t dz_effect_get_layer_count( const dz_effect_t * _effect )
{
    return _effect->layer_count;
}
//////////////////////////////////////////////////////////////////////////
void dz_effect_add_layer( dz_effect_t * const _effect, const dz_effect_layer_desc_t * _layer, dz_uint32_t * const _index )
{
    const dz_uint32_t index = _effect->layer_count++;

    _effect->layers[index] = *_layer;

    if( _index != DZ_NULLPTR )
    {
        *_index = index;
    }
}
//////////////////////////////////////////////////////////////////////////
void dz_effect_remove_layer( dz_effect_t * const _effect, dz_uint32_t _index, dz_effect_layer_desc_t * const _layer )
{
    if( _layer != DZ_NULLPTR )
    {
        *_layer = _effect->layers[_index];
    }

    for( dz_uint32_t index = _index + 1; index != _effect->layer_count; ++index )
    {
        _effect->layers[index - 1] = _effect->layers[index];
    }

    --_effect->layer_count;

    dz_uint32_t trigger_write = 0;
    for( dz_uint32_t index = 0; index != _effect->trigger_count; ++index )
    {
        dz_effect_trigger_desc_t trigger = _effect->triggers[index];

        if( trigger.target_layer_index == _index || trigger.source_layer_index == _index )
        {
            continue;
        }

        if( trigger.target_layer_index > _index && trigger.target_layer_index != DZ_EFFECT_LAYER_NONE )
        {
            --trigger.target_layer_index;
        }

        if( trigger.source_layer_index > _index && trigger.source_layer_index != DZ_EFFECT_LAYER_NONE )
        {
            --trigger.source_layer_index;
        }

        _effect->triggers[trigger_write++] = trigger;
    }

    _effect->trigger_count = trigger_write;

}
//////////////////////////////////////////////////////////////////////////
void dz_effect_set_layer( dz_effect_t * const _effect, dz_uint32_t _index, const dz_effect_layer_desc_t * _layer )
{
    _effect->layers[_index] = *_layer;
}
//////////////////////////////////////////////////////////////////////////
void dz_effect_get_layer( const dz_effect_t * _effect, dz_uint32_t _index, dz_effect_layer_desc_t * const _layer )
{
    *_layer = _effect->layers[_index];
}
//////////////////////////////////////////////////////////////////////////
dz_uint32_t dz_effect_get_trigger_count( const dz_effect_t * _effect )
{
    return _effect->trigger_count;
}
//////////////////////////////////////////////////////////////////////////
void dz_effect_add_trigger( dz_effect_t * const _effect, const dz_effect_trigger_desc_t * _trigger, dz_uint32_t * const _index )
{
    const dz_uint32_t index = _effect->trigger_count++;

    _effect->triggers[index] = *_trigger;

    if( _index != DZ_NULLPTR )
    {
        *_index = index;
    }
}
//////////////////////////////////////////////////////////////////////////
void dz_effect_remove_trigger( dz_effect_t * const _effect, dz_uint32_t _index, dz_effect_trigger_desc_t * const _trigger )
{
    if( _trigger != DZ_NULLPTR )
    {
        *_trigger = _effect->triggers[_index];
    }

    for( dz_uint32_t index = _index + 1; index != _effect->trigger_count; ++index )
    {
        _effect->triggers[index - 1] = _effect->triggers[index];
    }

    --_effect->trigger_count;

}
//////////////////////////////////////////////////////////////////////////
void dz_effect_set_trigger( dz_effect_t * const _effect, dz_uint32_t _index, const dz_effect_trigger_desc_t * _trigger )
{
    _effect->triggers[_index] = *_trigger;
}
//////////////////////////////////////////////////////////////////////////
void dz_effect_get_trigger( const dz_effect_t * _effect, dz_uint32_t _index, dz_effect_trigger_desc_t * const _trigger )
{
    *_trigger = _effect->triggers[_index];
}
//////////////////////////////////////////////////////////////////////////
void dz_effect_set_life( dz_effect_t * const _effect, dz_float_t _life )
{
    _effect->life = _life;
}
//////////////////////////////////////////////////////////////////////////
dz_float_t dz_effect_get_life( const dz_effect_t * _effect )
{
    return _effect->life;
}
//////////////////////////////////////////////////////////////////////////
void dz_effect_set_seed( dz_effect_t * const _effect, dz_uint32_t _seed )
{
    _effect->seed = _seed;
}
//////////////////////////////////////////////////////////////////////////
dz_uint32_t dz_effect_get_seed( const dz_effect_t * _effect )
{
    return _effect->seed;
}
//////////////////////////////////////////////////////////////////////////
dz_uint32_t dz_effect_get_physics_object_count( const dz_effect_t * _effect )
{
    return _effect->physics_object_count;
}
//////////////////////////////////////////////////////////////////////////
void dz_effect_add_physics_object( dz_effect_t * _effect, const dz_physics_object_desc_t * _object, dz_uint32_t * _index )
{
    const dz_uint32_t index = _effect->physics_object_count++;
    _effect->physics_objects[index] = *_object;
    _effect->physics_objects[index].transform.rotation = dz_math_quat_normalize( _object->transform.rotation );

    if( _index != DZ_NULLPTR )
    {
        *_index = index;
    }

}
//////////////////////////////////////////////////////////////////////////
void dz_effect_get_physics_object( const dz_effect_t * _effect, dz_uint32_t _index, dz_physics_object_desc_t * _object )
{
    *_object = _effect->physics_objects[_index];
}
//////////////////////////////////////////////////////////////////////////
void dz_effect_set_physics_object( dz_effect_t * _effect, dz_uint32_t _index, const dz_physics_object_desc_t * _object )
{
    _effect->physics_objects[_index] = *_object;
    _effect->physics_objects[_index].transform.rotation = dz_math_quat_normalize( _object->transform.rotation );
}
//////////////////////////////////////////////////////////////////////////
void dz_effect_remove_physics_object( dz_effect_t * _effect, dz_uint32_t _index )
{
    for( dz_uint32_t move = _index + 1U; move != _effect->physics_object_count; ++move )
    {
        _effect->physics_objects[move - 1U] = _effect->physics_objects[move];
    }

    --_effect->physics_object_count;
}
