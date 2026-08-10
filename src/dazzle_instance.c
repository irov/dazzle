#include "dazzle/dazzle.h"

#include "alloc.h"
#include "atlas.h"
#include "constant65535.h"
#include "effect.h"
#include "instance.h"
#include "material.h"
#include "math3d.h"
#include "math_provider.h"
#include "particle.h"
#include "shape.h"
#include "texture.h"

#include <math.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////
static dz_float_t __get_affector_timeline_default( dz_affector_timeline_type_e _timeline )
{
    dz_timeline_limit_status_e status;
    dz_float_t min_value;
    dz_float_t max_value;
    dz_float_t default_value;
    dz_float_t factor_value;
    dz_affector_timeline_get_limit( _timeline, &status, &min_value, &max_value, &default_value, &factor_value );
    return default_value;
}
//////////////////////////////////////////////////////////////////////////
static dz_float_t __get_shape_timeline_default( dz_shape_timeline_type_e _timeline )
{
    dz_timeline_limit_status_e status;
    dz_float_t min_value;
    dz_float_t max_value;
    dz_float_t default_value;
    dz_float_t factor_value;
    dz_shape_timeline_get_limit( _timeline, &status, &min_value, &max_value, &default_value, &factor_value );
    return default_value;
}
//////////////////////////////////////////////////////////////////////////
static dz_float_t __get_emitter_timeline_default( dz_emitter_timeline_type_e _timeline )
{
    dz_timeline_limit_status_e status;
    dz_float_t min_value;
    dz_float_t max_value;
    dz_float_t default_value;
    dz_float_t factor_value;
    dz_emitter_timeline_get_limit( _timeline, &status, &min_value, &max_value, &default_value, &factor_value );
    return default_value;
}
//////////////////////////////////////////////////////////////////////////
static dz_uint16_t __get_rand( dz_uint32_t * _seed )
{
    const dz_uint32_t value = DZ_RAND_FUNCTION( *_seed );

    *_seed = value;

    return value & 0xffff;
}
//////////////////////////////////////////////////////////////////////////
static dz_float_t __get_randf( dz_uint32_t * _seed )
{
    const dz_uint16_t value = __get_rand( _seed );

    const dz_float_t valuef = uint16_2_inv_float[value];

    return valuef;
}
//////////////////////////////////////////////////////////////////////////
static dz_float_t __get_randf2( dz_uint32_t * _seed, dz_float_t _min, dz_float_t _max )
{
    const dz_uint16_t value = __get_rand( _seed );

    const dz_float_t valuef = uint16_2_inv_float[value];

    return _min + (_max - _min) * valuef;
}
//////////////////////////////////////////////////////////////////////////
static dz_float_t __get_timeline_key_value( dz_float_t _t, const dz_timeline_key_t * _key )
{
    switch( _key->type )
    {
    case DZ_TIMELINE_KEY_CONST:
        {
            const dz_float_t value = _key->const_value;

            return value;
        }break;
    case DZ_TIMELINE_KEY_RANDOMIZE:
        {
            const dz_float_t value = _key->randomize_min_value + (_key->randomize_max_value - _key->randomize_min_value) * _t;

            return value;
        }break;
    case __DZ_TIMELINE_KEY_MAX__:
    default:
        break;
    };

    return 0.f;
}
//////////////////////////////////////////////////////////////////////////
static dz_float_t __get_timeline_value( dz_float_t _t, const dz_timeline_key_t * _key, dz_float_t _p )
{
    for( ; _key->interpolate != DZ_NULLPTR && _key->interpolate->key->p < _p; _key = _key->interpolate->key );

    const dz_float_t current_value = __get_timeline_key_value( _t, _key );

    if( _key->interpolate == DZ_NULLPTR ||
        _key->p > _p )
    {
        return current_value;
    }

    const dz_timeline_key_t * next = _key->interpolate->key;

    const dz_float_t next_value = __get_timeline_key_value( _t, next );

    dz_float_t t = ( _p - _key->p ) * _key->d_inv;
    t = DZ_MAX( 0.f, DZ_MIN( 1.f, t ) );

    switch( _key->interpolate->type )
    {
    case DZ_TIMELINE_INTERPOLATE_STEP:
        return current_value;
    case DZ_TIMELINE_INTERPOLATE_LINEAR:
        return current_value + ( next_value - current_value ) * t;
    case DZ_TIMELINE_INTERPOLATE_BEZIER2:
    {
        const dz_float_t one_minus_t = 1.f - t;
        const dz_float_t control = current_value + ( next_value - current_value ) * _key->interpolate->p0;
        return one_minus_t * one_minus_t * current_value + 2.f * one_minus_t * t * control + t * t * next_value;
    }
    case DZ_TIMELINE_INTERPOLATE_HERMITE:
    {
        const dz_float_t t2 = t * t;
        const dz_float_t t3 = t2 * t;
        const dz_float_t h00 = 2.f * t3 - 3.f * t2 + 1.f;
        const dz_float_t h10 = t3 - 2.f * t2 + t;
        const dz_float_t h01 = -2.f * t3 + 3.f * t2;
        const dz_float_t h11 = t3 - t2;
        return h00 * current_value + h10 * _key->interpolate->out_tangent + h01 * next_value + h11 * _key->interpolate->in_tangent;
    }
    case __DZ_TIMELINE_INTERPOLATE_MAX__:
    default:
        return current_value;
    }
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_instance_set_physics_transform( dz_instance_t * _instance, dz_uint32_t _id, const dz_transform_t * _transform )
{
    for( dz_uint32_t index = 0; index != _instance->effect->physics_object_count; ++index )
    {
        if( _instance->effect->physics_objects[index].id == _id )
        {
            _instance->physics_transforms[index] = *_transform;
            _instance->physics_transforms[index].rotation = dz_math_quat_normalize( _transform->rotation );
            return DZ_SUCCESSFUL;
        }
    }

    return DZ_FAILURE_INVALID_DATA;
}
//////////////////////////////////////////////////////////////////////////
void dz_instance_create( const dz_service_t * _service, dz_instance_t ** _instance, const dz_effect_t * _effect, dz_userdata_t _ud )
{
    dz_instance_t * instance = DZ_NEW( _service, dz_instance_t );

    instance->effect = _effect;

    instance->init_seed = _effect->seed;
    instance->seed = _effect->seed;

    instance->partices = DZ_NULLPTR;
    instance->partices_count = 0;
    instance->partices_capacity = 0;
    instance->particle_limit = 10922;

    instance->loop = DZ_FALSE;
    instance->emit_pause = DZ_FALSE;
    instance->started = DZ_FALSE;
    instance->stopped = DZ_FALSE;
    instance->paused = DZ_FALSE;

    instance->time = 0.f;
    instance->fixed_step = 1.f / 60.f;
    instance->fixed_step_accumulator = 0.f;
    instance->emitter_instance_count = 0;

    instance->x = 0.f;
    instance->y = 0.f;
    instance->z = 0.f;

    instance->angle = 0.f;
    instance->transform.position = dz_math_vec3( 0.f, 0.f, 0.f );
    instance->transform.rotation = dz_math_quat_identity();
    instance->transform.scale = dz_math_vec3( 1.f, 1.f, 1.f );
    instance->birth_order = 0;

    for( dz_uint32_t index = 0; index != _effect->physics_object_count; ++index )
    {
        instance->physics_transforms[index] = _effect->physics_objects[index].transform;
    }

    instance->r = 1.f;
    instance->g = 1.f;
    instance->b = 1.f;
    instance->a = 1.f;

    instance->ud = _ud;

    *_instance = instance;

}
//////////////////////////////////////////////////////////////////////////
void dz_instance_destroy( const dz_service_t * _service, const dz_instance_t * _instance )
{
    DZ_FREE( _service, _instance->partices );

    DZ_FREE( _service, _instance );
}
//////////////////////////////////////////////////////////////////////////
void dz_instance_set_seed( dz_instance_t * const _instance, dz_uint32_t _seed )
{
    _instance->init_seed = _seed;
}
//////////////////////////////////////////////////////////////////////////
dz_uint32_t dz_instance_get_seed( const dz_instance_t * _instance )
{
    return _instance->init_seed;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_instance_set_particle_limit( dz_instance_t * const _instance, dz_uint16_t _limit )
{
#ifdef DZ_DEBUG
    if( _limit > 10922 )
    {
        return DZ_FAILURE;
    }
#endif

    _instance->particle_limit = _limit;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_uint16_t dz_instance_get_particle_limit( const dz_instance_t * _instance )
{
    return _instance->particle_limit;
}
//////////////////////////////////////////////////////////////////////////
void dz_instance_set_ud( dz_instance_t * const _instance, dz_userdata_t _ud )
{
    _instance->ud = _ud;
}
//////////////////////////////////////////////////////////////////////////
dz_userdata_t dz_instance_get_ud( const dz_instance_t * _instance )
{
    return _instance->ud;
}
//////////////////////////////////////////////////////////////////////////
void dz_instance_set_effect( dz_instance_t * const _instance, const dz_effect_t * _effect )
{
    _instance->effect = _effect;
}
//////////////////////////////////////////////////////////////////////////
const dz_effect_t * dz_instance_get_effect( const dz_instance_t * _instance )
{
    return _instance->effect;
}
//////////////////////////////////////////////////////////////////////////
void dz_instance_set_loop( dz_instance_t * const _instance, dz_bool_t _loop )
{
    _instance->loop = _loop;
}
//////////////////////////////////////////////////////////////////////////
dz_bool_t dz_instance_get_loop( const dz_instance_t * _instance )
{
    return _instance->loop;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_instance_set_time( dz_instance_t * const _instance, dz_float_t _time )
{
#ifdef DZ_DEBUG
    if( _time < 0.f )
    {
        return DZ_FAILURE;
    }
#endif

    _instance->time = _time;
    _instance->emitter_instance_count = 0;
    _instance->started = _time > 0.f ? DZ_TRUE : DZ_FALSE;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_float_t dz_instance_get_time( const dz_instance_t * _instance )
{
    return _instance->time;
}
//////////////////////////////////////////////////////////////////////////
void dz_instance_set_fixed_step( dz_instance_t * _instance, dz_float_t _step )
{
    _instance->fixed_step_accumulator = 0.f;
    _instance->fixed_step = _step;
}
//////////////////////////////////////////////////////////////////////////
dz_float_t dz_instance_get_fixed_step( const dz_instance_t * _instance )
{
    return _instance->fixed_step;
}
//////////////////////////////////////////////////////////////////////////
static dz_float_t __get_affector_value_rands( dz_particle_t * _particle, const dz_effect_t * _effect, dz_affector_timeline_type_e _type, dz_float_t _p )
{
    const dz_effect_layer_desc_t * layer = _effect->layers + _particle->layer_index;

    const dz_timeline_key_t * timeline_key = layer->affector->timelines[_type];

    if( timeline_key == DZ_NULLPTR )
    {
        const dz_float_t default_value = __get_affector_timeline_default( _type );

        return default_value;
    }

    const dz_float_t value = __get_timeline_value( _particle->rands[_type], timeline_key, _p );

    return value;
}
//////////////////////////////////////////////////////////////////////////
static dz_bool_t __material_get_texture_range( const dz_material_t * _material, dz_uint32_t * const _begin, dz_uint32_t * const _count )
{
    if( _material->textures_count == 0 )
    {
        *_begin = 0;
        *_count = 0;

        return DZ_FALSE;
    }

    dz_uint32_t begin = _material->texture_index;
    if( begin >= _material->textures_count )
    {
        begin = _material->textures_count - 1;
    }

    dz_uint32_t count = _material->texture_count;
    if( count == 0 )
    {
        count = 1;
    }

    const dz_uint32_t available = _material->textures_count - begin;
    if( count > available )
    {
        count = available;
    }

    *_begin = begin;
    *_count = count;

    return count != 0 ? DZ_TRUE : DZ_FALSE;
}
//////////////////////////////////////////////////////////////////////////
static const dz_texture_t * __material_get_texture( const dz_material_t * _material )
{
    dz_uint32_t begin;
    dz_uint32_t count;
    if( __material_get_texture_range( _material, &begin, &count ) == DZ_FALSE )
    {
        return DZ_NULLPTR;
    }

    DZ_UNUSED( count );

    return _material->textures[begin].texture;
}
//////////////////////////////////////////////////////////////////////////
static const dz_texture_t * __material_get_texture_random_weight( const dz_material_t * _material, dz_uint32_t * const _seed )
{
    dz_uint32_t begin;
    dz_uint32_t count;
    if( __material_get_texture_range( _material, &begin, &count ) == DZ_FALSE )
    {
        return DZ_NULLPTR;
    }

    if( count == 1 )
    {
        return _material->textures[begin].texture;
    }

    dz_float_t total_weight = 0.f;

    for( dz_uint32_t index = 0; index != count; ++index )
    {
        total_weight += DZ_MAX( _material->textures[begin + index].random_weight, 0.f );
    }

    if( total_weight <= 0.f )
    {
        return _material->textures[begin].texture;
    }

    dz_float_t random_weight = __get_randf( _seed ) * total_weight;

    for( dz_uint32_t index = 0; index != count; ++index )
    {
        const dz_material_texture_t * material_texture = &_material->textures[begin + index];
        const dz_float_t weight = DZ_MAX( material_texture->random_weight, 0.f );

        if( weight <= 0.f )
        {
            continue;
        }

        if( random_weight <= weight )
        {
            return material_texture->texture;
        }

        random_weight -= weight;
    }

    return _material->textures[begin + count - 1].texture;
}
//////////////////////////////////////////////////////////////////////////
static const dz_texture_t * __material_get_sequence_texture( const dz_material_t * _material, dz_float_t _time )
{
    dz_uint32_t begin;
    dz_uint32_t count;
    if( __material_get_texture_range( _material, &begin, &count ) == DZ_FALSE )
    {
        return DZ_NULLPTR;
    }

    dz_float_t textures_time = 0.f;

    for( dz_uint32_t index = 0; index != count; ++index )
    {
        textures_time += _material->textures[begin + index].texture->sequence_delay;
    }

    if( textures_time <= 0.f )
    {
        return DZ_NULLPTR;
    }

    dz_float_t texture_time = _time;
    for( ; texture_time >= textures_time; texture_time -= textures_time )
        ;

    for( dz_uint32_t index = 0; index != count; ++index )
    {
        const dz_texture_t * texture = _material->textures[begin + index].texture;

        texture_time -= texture->sequence_delay;

        if( texture_time > 0.f )
        {
            continue;
        }

        return texture;
    }

    return _material->textures[begin + count - 1].texture;
}
//////////////////////////////////////////////////////////////////////////
static void __particle_collision_response( dz_particle_t * _particle, dz_vec3_t _normal, const dz_physics_object_desc_t * _object )
{
    if( _object->response == DZ_COLLISION_KILL )
    {
        _particle->time = -1.f;
        return;
    }

    const dz_float_t normal_velocity = _particle->vx * _normal.x + _particle->vy * _normal.y + _particle->vz * _normal.z;
    if( normal_velocity >= 0.f )
    {
        return;
    }

    const dz_float_t normal_factor = _object->response == DZ_COLLISION_BOUNCE ? ( 1.f + DZ_MAX( _object->restitution, 0.f ) ) : 1.f;
    _particle->vx -= normal_factor * normal_velocity * _normal.x;
    _particle->vy -= normal_factor * normal_velocity * _normal.y;
    _particle->vz -= normal_factor * normal_velocity * _normal.z;

    const dz_float_t friction = DZ_MAX( 0.f, DZ_MIN( _object->friction, 1.f ) );
    _particle->vx *= 1.f - friction;
    _particle->vy *= 1.f - friction;
    _particle->vz *= 1.f - friction;
}
//////////////////////////////////////////////////////////////////////////
static dz_vec3_t __inverse_transform_point( const dz_transform_t * _transform, dz_vec3_t _point )
{
    dz_quat_t inverse = dz_math_quat_normalize( _transform->rotation );
    inverse.x = -inverse.x;
    inverse.y = -inverse.y;
    inverse.z = -inverse.z;
    dz_vec3_t local = dz_math_quat_rotate3( inverse, dz_math_sub3( _point, _transform->position ) );
    if( _transform->scale.x != 0.f )
    {
        local.x /= _transform->scale.x;
    }
    if( _transform->scale.y != 0.f )
    {
        local.y /= _transform->scale.y;
    }
    if( _transform->scale.z != 0.f )
    {
        local.z /= _transform->scale.z;
    }
    return local;
}
//////////////////////////////////////////////////////////////////////////
static dz_bool_t __segment_triangle_intersection( dz_vec3_t _start, dz_vec3_t _end, dz_vec3_t _a, dz_vec3_t _b, dz_vec3_t _c, dz_float_t * _time, dz_vec3_t * _normal )
{
    const dz_vec3_t direction = dz_math_sub3( _end, _start );
    const dz_vec3_t edge1 = dz_math_sub3( _b, _a );
    const dz_vec3_t edge2 = dz_math_sub3( _c, _a );
    const dz_vec3_t p = dz_math_cross3( direction, edge2 );
    const dz_float_t determinant = dz_math_dot3( edge1, p );
    if( fabsf( determinant ) <= 0.000001f )
    {
        return DZ_FALSE;
    }

    const dz_float_t inverse = 1.f / determinant;
    const dz_vec3_t from_a = dz_math_sub3( _start, _a );
    const dz_float_t u = dz_math_dot3( from_a, p ) * inverse;
    if( u < 0.f || u > 1.f )
    {
        return DZ_FALSE;
    }

    const dz_vec3_t q = dz_math_cross3( from_a, edge1 );
    const dz_float_t v = dz_math_dot3( direction, q ) * inverse;
    if( v < 0.f || u + v > 1.f )
    {
        return DZ_FALSE;
    }

    const dz_float_t time = dz_math_dot3( edge2, q ) * inverse;
    if( time < 0.f || time > 1.f )
    {
        return DZ_FALSE;
    }

    dz_vec3_t normal = dz_math_normalize3( dz_math_cross3( edge1, edge2 ), dz_math_vec3( 0.f, 1.f, 0.f ) );
    if( dz_math_dot3( normal, direction ) > 0.f )
    {
        normal = dz_math_mul3( normal, -1.f );
    }
    *_time = time;
    *_normal = normal;
    return DZ_TRUE;
}
//////////////////////////////////////////////////////////////////////////
static dz_bool_t __segment_sphere_intersection( const dz_service_t * _service, dz_vec3_t _start, dz_vec3_t _end, dz_vec3_t _center, dz_float_t _radius, dz_float_t * _time,
                                                dz_vec3_t * _normal )
{
    const dz_vec3_t direction = dz_math_sub3( _end, _start );
    const dz_vec3_t offset = dz_math_sub3( _start, _center );
    const dz_float_t a = dz_math_dot3( direction, direction );
    const dz_float_t c = dz_math_dot3( offset, offset ) - _radius * _radius;
    if( a <= 0.0000001f || c <= 0.f )
    {
        return DZ_FALSE;
    }

    const dz_float_t b = dz_math_dot3( offset, direction );
    const dz_float_t discriminant = b * b - a * c;
    if( b >= 0.f || discriminant < 0.f )
    {
        return DZ_FALSE;
    }

    const dz_float_t time = ( -b - DZ_SQRTF( _service, discriminant ) ) / a;
    if( time < 0.f || time > 1.f )
    {
        return DZ_FALSE;
    }

    const dz_vec3_t point = dz_math_add3( _start, dz_math_mul3( direction, time ) );
    *_time = time;
    *_normal = dz_math_normalize3( dz_math_sub3( point, _center ), dz_math_vec3( 0.f, 1.f, 0.f ) );
    return DZ_TRUE;
}
//////////////////////////////////////////////////////////////////////////
static dz_bool_t __segment_box_intersection( dz_vec3_t _start, dz_vec3_t _end, dz_vec3_t _extents, dz_float_t * _time, dz_vec3_t * _normal )
{
    const dz_vec3_t direction = dz_math_sub3( _end, _start );
    const dz_float_t starts[3] = { _start.x, _start.y, _start.z };
    const dz_float_t directions[3] = { direction.x, direction.y, direction.z };
    const dz_float_t extents[3] = { _extents.x, _extents.y, _extents.z };
    dz_float_t enter = 0.f;
    dz_float_t leave = 1.f;
    dz_vec3_t enter_normal = dz_math_vec3( 0.f, 1.f, 0.f );

    for( dz_uint32_t axis = 0; axis != 3U; ++axis )
    {
        if( fabsf( directions[axis] ) <= 0.000001f )
        {
            if( starts[axis] < -extents[axis] || starts[axis] > extents[axis] )
            {
                return DZ_FALSE;
            }
            continue;
        }

        dz_float_t first = ( -extents[axis] - starts[axis] ) / directions[axis];
        dz_float_t second = ( extents[axis] - starts[axis] ) / directions[axis];
        dz_float_t sign = -1.f;
        if( first > second )
        {
            const dz_float_t swap = first;
            first = second;
            second = swap;
            sign = 1.f;
        }
        if( first > enter )
        {
            enter = first;
            enter_normal = dz_math_vec3( 0.f, 0.f, 0.f );
            if( axis == 0U )
            {
                enter_normal.x = sign;
            }
            else if( axis == 1U )
            {
                enter_normal.y = sign;
            }
            else
            {
                enter_normal.z = sign;
            }
        }
        leave = DZ_MIN( leave, second );
        if( enter > leave )
        {
            return DZ_FALSE;
        }
    }

    if( enter < 0.f || enter > 1.f )
    {
        return DZ_FALSE;
    }
    *_time = enter;
    *_normal = enter_normal;
    return DZ_TRUE;
}
//////////////////////////////////////////////////////////////////////////
static dz_bool_t __segment_aabb_intersection( dz_vec3_t _start, dz_vec3_t _end, const dz_aabb_t * _bounds, dz_float_t _maximum_time )
{
    const dz_vec3_t center = dz_math_mul3( dz_math_add3( _bounds->minimum, _bounds->maximum ), 0.5f );
    const dz_vec3_t extents = dz_math_mul3( dz_math_sub3( _bounds->maximum, _bounds->minimum ), 0.5f );
    dz_float_t time;
    dz_vec3_t normal;
    return __segment_box_intersection( dz_math_sub3( _start, center ), dz_math_sub3( _end, center ), extents, &time, &normal ) == DZ_TRUE && time <= _maximum_time;
}
//////////////////////////////////////////////////////////////////////////
static void __mesh_bvh_intersection( const dz_mesh_desc_t * _mesh, const dz_mesh_bvh_t * _bvh, dz_uint32_t _node_index, dz_vec3_t _start, dz_vec3_t _end, dz_float_t * _earliest,
                                     dz_vec3_t * _normal )
{
    const dz_mesh_bvh_node_t * node = _bvh->nodes + _node_index;
    if( __segment_aabb_intersection( _start, _end, &node->bounds, *_earliest ) == DZ_FALSE )
    {
        return;
    }

    if( node->count != 0U )
    {
        for( dz_uint32_t offset = 0; offset != node->count; ++offset )
        {
            const dz_uint32_t triangle = _bvh->triangles[node->first + offset] * 3U;
            const dz_vec3_t a = _mesh->vertices[_mesh->indices[triangle + 0U]].position;
            const dz_vec3_t b = _mesh->vertices[_mesh->indices[triangle + 1U]].position;
            const dz_vec3_t c = _mesh->vertices[_mesh->indices[triangle + 2U]].position;
            dz_float_t hit_time;
            dz_vec3_t hit_normal;
            if( __segment_triangle_intersection( _start, _end, a, b, c, &hit_time, &hit_normal ) == DZ_TRUE && hit_time < *_earliest )
            {
                *_earliest = hit_time;
                *_normal = hit_normal;
            }
        }
        return;
    }

    __mesh_bvh_intersection( _mesh, _bvh, node->left, _start, _end, _earliest, _normal );
    __mesh_bvh_intersection( _mesh, _bvh, node->right, _start, _end, _earliest, _normal );
}
//////////////////////////////////////////////////////////////////////////
static dz_vec3_t __transform_normal( const dz_transform_t * _transform, dz_vec3_t _normal )
{
    _normal.x /= _transform->scale.x;
    _normal.y /= _transform->scale.y;
    _normal.z /= _transform->scale.z;
    return dz_math_normalize3( dz_math_quat_rotate3( _transform->rotation, _normal ), dz_math_vec3( 0.f, 1.f, 0.f ) );
}
//////////////////////////////////////////////////////////////////////////
static void __particle_apply_physics( const dz_service_t * _service, const dz_instance_t * _instance, dz_particle_t * _particle, dz_float_t _time )
{
    const dz_effect_t * effect = _instance->effect;

    for( dz_uint32_t index = 0; index != effect->physics_object_count; ++index )
    {
        const dz_physics_object_desc_t * object = effect->physics_objects + index;
        const dz_transform_t * transform = _instance->physics_transforms + index;

        switch( object->type )
        {
        case DZ_PHYSICS_GRAVITY:
            _particle->vx += object->direction.x * object->strength * _time;
            _particle->vy += object->direction.y * object->strength * _time;
            _particle->vz += object->direction.z * object->strength * _time;
            break;
        case DZ_PHYSICS_WIND:
        {
            const dz_float_t phase = _particle->x * 0.173f + _particle->y * 0.367f + _particle->z * 0.619f + _particle->time * 2.137f;
            const dz_float_t turbulence = object->turbulence * DZ_SINF( _service, phase );
            const dz_float_t strength = object->strength + turbulence;
            _particle->vx += object->direction.x * strength * _time;
            _particle->vy += object->direction.y * strength * _time;
            _particle->vz += object->direction.z * strength * _time;
        }
        break;
        case DZ_PHYSICS_MAGNET:
        {
            dz_vec3_t delta = dz_math_sub3( transform->position, dz_math_vec3( _particle->x, _particle->y, _particle->z ) );
            const dz_float_t distance = dz_math_length3( delta );
            if( distance > 0.000001f )
            {
                delta = dz_math_mul3( delta, 1.f / distance );
                const dz_float_t attenuation = 1.f / ( 1.f + DZ_MAX( object->falloff, 0.f ) * distance );
                _particle->vx += delta.x * object->strength * attenuation * _time;
                _particle->vy += delta.y * object->strength * attenuation * _time;
                _particle->vz += delta.z * object->strength * attenuation * _time;
            }
        }
        break;
        case DZ_PHYSICS_PLANE:
        {
            dz_vec3_t normal = dz_math_normalize3( object->direction, dz_math_vec3( 0.f, 1.f, 0.f ) );
            normal = dz_math_quat_rotate3( transform->rotation, normal );
            const dz_vec3_t previous = dz_math_vec3( _particle->previous_x, _particle->previous_y, _particle->previous_z );
            const dz_vec3_t current = dz_math_vec3( _particle->x, _particle->y, _particle->z );
            const dz_float_t previous_distance = dz_math_dot3( dz_math_sub3( previous, transform->position ), normal );
            const dz_float_t current_distance = dz_math_dot3( dz_math_sub3( current, transform->position ), normal );
            if( previous_distance >= 0.f && current_distance < 0.f )
            {
                _particle->x -= current_distance * normal.x;
                _particle->y -= current_distance * normal.y;
                _particle->z -= current_distance * normal.z;
                __particle_collision_response( _particle, normal, object );
            }
        }
        break;
        case DZ_PHYSICS_SPHERE:
        {
            const dz_float_t radius_scale = DZ_MAX( fabsf( transform->scale.x ), DZ_MAX( fabsf( transform->scale.y ), fabsf( transform->scale.z ) ) );
            const dz_float_t radius = DZ_MAX( object->radius * radius_scale, 0.f );
            const dz_vec3_t previous = dz_math_vec3( _particle->previous_x, _particle->previous_y, _particle->previous_z );
            const dz_vec3_t current = dz_math_vec3( _particle->x, _particle->y, _particle->z );
            dz_float_t hit_time;
            dz_vec3_t hit_normal;
            if( radius > 0.f && __segment_sphere_intersection( _service, previous, current, transform->position, radius, &hit_time, &hit_normal ) == DZ_TRUE )
            {
                const dz_vec3_t hit = dz_math_add3( previous, dz_math_mul3( dz_math_sub3( current, previous ), hit_time ) );
                _particle->x = hit.x + hit_normal.x * 0.0001f;
                _particle->y = hit.y + hit_normal.y * 0.0001f;
                _particle->z = hit.z + hit_normal.z * 0.0001f;
                __particle_collision_response( _particle, hit_normal, object );
                break;
            }

            const dz_vec3_t delta = dz_math_sub3( current, transform->position );
            const dz_float_t distance = dz_math_length3( delta );
            if( distance < radius && radius > 0.f )
            {
                const dz_vec3_t normal = dz_math_normalize3( delta, dz_math_vec3( 0.f, 1.f, 0.f ) );
                _particle->x = transform->position.x + normal.x * radius;
                _particle->y = transform->position.y + normal.y * radius;
                _particle->z = transform->position.z + normal.z * radius;
                __particle_collision_response( _particle, normal, object );
            }
        }
        break;
        case DZ_PHYSICS_BOX:
        {
            const dz_vec3_t previous_world = dz_math_vec3( _particle->previous_x, _particle->previous_y, _particle->previous_z );
            const dz_vec3_t current_world = dz_math_vec3( _particle->x, _particle->y, _particle->z );
            const dz_vec3_t previous = __inverse_transform_point( transform, previous_world );
            const dz_vec3_t current = __inverse_transform_point( transform, current_world );
            dz_float_t hit_time;
            dz_vec3_t local_normal;
            if( __segment_box_intersection( previous, current, object->half_extents, &hit_time, &local_normal ) == DZ_TRUE )
            {
                const dz_vec3_t hit = dz_math_add3( previous_world, dz_math_mul3( dz_math_sub3( current_world, previous_world ), hit_time ) );
                const dz_vec3_t normal = dz_math_normalize3( dz_math_quat_rotate3( transform->rotation, local_normal ), dz_math_vec3( 0.f, 1.f, 0.f ) );
                _particle->x = hit.x + normal.x * 0.0001f;
                _particle->y = hit.y + normal.y * 0.0001f;
                _particle->z = hit.z + normal.z * 0.0001f;
                __particle_collision_response( _particle, normal, object );
            }
        }
        break;
        case DZ_PHYSICS_MESH:
        {
            const dz_uint32_t mesh_index = dz_effect_find_mesh_index( effect, object->mesh_id );
            if( mesh_index == DZ_RESOURCE_ID_NONE )
            {
                break;
            }

            const dz_mesh_desc_t * mesh = effect->meshes + mesh_index;
            const dz_mesh_bvh_t * bvh = effect->mesh_bvhs + mesh_index;
            const dz_vec3_t previous_world = dz_math_vec3( _particle->previous_x, _particle->previous_y, _particle->previous_z );
            const dz_vec3_t current_world = dz_math_vec3( _particle->x, _particle->y, _particle->z );
            const dz_vec3_t previous = __inverse_transform_point( transform, previous_world );
            const dz_vec3_t current = __inverse_transform_point( transform, current_world );
            dz_float_t earliest = 2.f;
            dz_vec3_t local_normal = dz_math_vec3( 0.f, 1.f, 0.f );
            __mesh_bvh_intersection( mesh, bvh, 0U, previous, current, &earliest, &local_normal );

            if( earliest <= 1.f )
            {
                dz_vec3_t normal = __transform_normal( transform, local_normal );
                const dz_vec3_t world_direction = dz_math_sub3( current_world, previous_world );
                if( dz_math_dot3( normal, world_direction ) > 0.f )
                {
                    normal = dz_math_mul3( normal, -1.f );
                }
                const dz_vec3_t hit = dz_math_add3( previous_world, dz_math_mul3( world_direction, earliest ) );
                _particle->x = hit.x + normal.x * 0.0001f;
                _particle->y = hit.y + normal.y * 0.0001f;
                _particle->z = hit.z + normal.z * 0.0001f;
                __particle_collision_response( _particle, normal, object );
            }
        }
        break;
        case __DZ_PHYSICS_OBJECT_MAX__:
        default:
            break;
        }
    }
}
//////////////////////////////////////////////////////////////////////////
static void __particle_update( const dz_service_t * _service, const dz_instance_t * _instance, dz_particle_t * _p, dz_float_t _time )
{
    const dz_effect_t * _effect = _instance->effect;
    _p->time += _time;

    const dz_float_t p = _p->time / _p->life;

    const dz_float_t move_speed = __get_affector_value_rands( _p, _effect, DZ_AFFECTOR_TIMELINE_MOVE_SPEED, p );
    const dz_float_t move_accelerate = __get_affector_value_rands( _p, _effect, DZ_AFFECTOR_TIMELINE_MOVE_ACCELERATE, p );

    const dz_float_t rotate_speed = __get_affector_value_rands( _p, _effect, DZ_AFFECTOR_TIMELINE_ROTATE_SPEED, p );
    const dz_float_t rotate_accelerate = __get_affector_value_rands( _p, _effect, DZ_AFFECTOR_TIMELINE_ROTATE_ACCELERATE, p );

    const dz_float_t spin_speed = __get_affector_value_rands( _p, _effect, DZ_AFFECTOR_TIMELINE_SPIN_SPEED, p );
    const dz_float_t spin_accelerate = __get_affector_value_rands( _p, _effect, DZ_AFFECTOR_TIMELINE_SPIN_ACCELERATE, p );

    const dz_float_t scale = __get_affector_value_rands( _p, _effect, DZ_AFFECTOR_TIMELINE_SCALE, p );
    const dz_float_t aspect = __get_affector_value_rands( _p, _effect, DZ_AFFECTOR_TIMELINE_ASPECT, p );

    _p->scale = scale;
    _p->aspect = aspect;

    const dz_float_t r = __get_affector_value_rands( _p, _effect, DZ_AFFECTOR_TIMELINE_COLOR_R, p );
    const dz_float_t g = __get_affector_value_rands( _p, _effect, DZ_AFFECTOR_TIMELINE_COLOR_G, p );
    const dz_float_t b = __get_affector_value_rands( _p, _effect, DZ_AFFECTOR_TIMELINE_COLOR_B, p );
    const dz_float_t a = __get_affector_value_rands( _p, _effect, DZ_AFFECTOR_TIMELINE_COLOR_A, p );

    _p->color_r = r * _p->born_color_r;
    _p->color_g = g * _p->born_color_g;
    _p->color_b = b * _p->born_color_b;
    _p->color_a = a * _p->born_color_a;

    _p->rotate_accelerate_aux += rotate_accelerate * _time;
    _p->angle += rotate_speed * _time + _p->rotate_accelerate_aux * _time;

    _p->spin_accelerate_aux += spin_accelerate * _time;
    _p->spin += spin_speed * _time + _p->spin_accelerate_aux * _time;

    const dz_float_t direction_z = DZ_MAX( -1.f, DZ_MIN( _p->initial_direction_z + __get_affector_value_rands( _p, _effect, DZ_AFFECTOR_TIMELINE_DIRECTION_Z, p ), 1.f ) );
    const dz_float_t direction_xy = DZ_SQRTF( _service, DZ_MAX( 0.f, 1.f - direction_z * direction_z ) );
    const dz_float_t dx = DZ_COSF( _service, _p->angle ) * direction_xy;
    const dz_float_t dy = DZ_SINF( _service, _p->angle ) * direction_xy;

    const dz_float_t strafe_size = __get_affector_value_rands( _p, _effect, DZ_AFFECTOR_TIMELINE_STRAFE_SIZE, p );

    if( strafe_size != 0.f )
    {
        const dz_float_t strafe_speed = __get_affector_value_rands( _p, _effect, DZ_AFFECTOR_TIMELINE_STRAFE_SPEED, p );
        const dz_float_t strafe_frenquence = __get_affector_value_rands( _p, _effect, DZ_AFFECTOR_TIMELINE_STRAFE_FRENQUENCE, p );

        const dz_float_t strafe_shift = _p->rands[DZ_AFFECTOR_TIMELINE_STRAFE_SHIFT];

        const dz_float_t strafex = -dy * DZ_COSF( _service, strafe_shift * DZ_PI + strafe_frenquence * _p->time ) * strafe_size * strafe_speed * _time;
        const dz_float_t strafey = dx * DZ_SINF( _service, strafe_shift * DZ_PI + strafe_frenquence * _p->time ) * strafe_size * strafe_speed * _time;

        _p->x += strafex;
        _p->y += strafey;
    }

    _p->previous_x = _p->x;
    _p->previous_y = _p->y;
    _p->previous_z = _p->z;
    _p->birth_x = _p->x;
    _p->birth_y = _p->y;
    _p->birth_z = _p->z;

    _p->move_accelerate_aux += move_accelerate * _time;
    const dz_float_t authored_speed = move_speed + _p->move_accelerate_aux;
    const dz_float_t authored_delta = authored_speed - _p->authored_speed;
    _p->vx += dx * authored_delta;
    _p->vy += dy * authored_delta;
    _p->vz += direction_z * authored_delta;
    _p->authored_speed = authored_speed;

    _p->vx += __get_affector_value_rands( _p, _effect, DZ_AFFECTOR_TIMELINE_GRAVITY_X, p ) * _time;
    _p->vy += __get_affector_value_rands( _p, _effect, DZ_AFFECTOR_TIMELINE_GRAVITY_Y, p ) * _time;
    _p->vz += __get_affector_value_rands( _p, _effect, DZ_AFFECTOR_TIMELINE_GRAVITY_Z, p ) * _time;

    const dz_float_t drag = DZ_MAX( 0.f, __get_affector_value_rands( _p, _effect, DZ_AFFECTOR_TIMELINE_DRAG, p ) );
    const dz_float_t drag_factor = 1.f / ( 1.f + drag * _time );
    _p->vx *= drag_factor;
    _p->vy *= drag_factor;
    _p->vz *= drag_factor;

    _p->x += _p->vx * _time;
    _p->y += _p->vy * _time;
    _p->z += _p->vz * _time;

    __particle_apply_physics( _service, _instance, _p, _time );

    const dz_float_t sx = DZ_COSF( _service, _p->angle + _p->spin );
    const dz_float_t sy = DZ_SINF( _service, _p->angle + _p->spin );

    _p->sx = sx;
    _p->sy = sy;
    _p->sz = direction_z;

    // update texture
    const dz_effect_layer_desc_t * layer = _effect->layers + _p->layer_index;

    const dz_material_t * material = layer->material;

    switch( material->mode )
    {
    case DZ_MATERIAL_MODE_SOLID:
        {
            _p->texture = DZ_NULLPTR;
        }break;
    case DZ_MATERIAL_MODE_TEXTURE:
        {
            if( _p->texture == DZ_NULLPTR )
            {
                _p->texture = __material_get_texture( material );
            }
        }break;
    case DZ_MATERIAL_MODE_SEQUENCE:
        {
            _p->texture = __material_get_sequence_texture( material, _p->time );
        }break;
    default:
        break;
    }

}
//////////////////////////////////////////////////////////////////////////
static dz_float_t __get_timeline_value_seed( dz_uint32_t * _seed, const dz_timeline_key_t * _timeline, dz_float_t _p )
{
    const dz_float_t t = __get_randf( _seed );

    const dz_float_t value = __get_timeline_value( t, _timeline, _p );

    return value;
}
//////////////////////////////////////////////////////////////////////////
static dz_float_t __get_shape_value_seed( dz_uint32_t * const _seed, const dz_effect_layer_desc_t * _layer, dz_shape_timeline_type_e _type, dz_float_t _p )
{
    const dz_timeline_key_t * timeline_key = _layer->shape->timelines[_type];

    if( timeline_key == DZ_NULLPTR )
    {
        const dz_float_t default_value = __get_shape_timeline_default( _type );

        return default_value;
    }

    const dz_float_t value = __get_timeline_value_seed( _seed, timeline_key, _p );

    return value;
}
//////////////////////////////////////////////////////////////////////////
static dz_float_t __get_emitter_value_seed( dz_uint32_t * const _seed, const dz_effect_layer_desc_t * _layer, dz_emitter_timeline_type_e _type, dz_float_t _p )
{
    const dz_timeline_key_t * timeline_key = _layer->emitter->timelines[_type];

    if( timeline_key == DZ_NULLPTR )
    {
        const dz_float_t default_value = __get_emitter_timeline_default( _type );

        return default_value;
    }

    const dz_float_t value = __get_timeline_value_seed( _seed, timeline_key, _p );

    return value;
}
//////////////////////////////////////////////////////////////////////////
static dz_float_t __get_affector_value_seed( dz_uint32_t * const _seed, const dz_effect_layer_desc_t * _layer, dz_affector_timeline_type_e _type, dz_float_t _p )
{
    const dz_timeline_key_t * timeline_key = _layer->affector->timelines[_type];

    if( timeline_key == DZ_NULLPTR )
    {
        const dz_float_t default_value = __get_affector_timeline_default( _type );

        return default_value;
    }

    const dz_float_t value = __get_timeline_value_seed( _seed, timeline_key, _p );

    return value;
}
//////////////////////////////////////////////////////////////////////////
static dz_float_t __calc_triangle_area( dz_float_t ax, dz_float_t ay, dz_float_t bx, dz_float_t by, dz_float_t cx, dz_float_t cy )
{
    const dz_float_t area = (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by)) * 0.5f;

    if( area < 0.f )
    {
        return -area;
    }

    return area;
}
//////////////////////////////////////////////////////////////////////////
static dz_float_t __calc_triangle_area3( dz_vec3_t _a, dz_vec3_t _b, dz_vec3_t _c )
{
    return 0.5f * dz_math_length3( dz_math_cross3( dz_math_sub3( _b, _a ), dz_math_sub3( _c, _a ) ) );
}
//////////////////////////////////////////////////////////////////////////
static dz_float_t __calc_tetrahedron_volume( dz_vec3_t _a, dz_vec3_t _b, dz_vec3_t _c )
{
    return fabsf( dz_math_dot3( _a, dz_math_cross3( _b, _c ) ) ) / 6.f;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __sample_mesh_shape( const dz_effect_t * _effect, const dz_shape_t * _shape, dz_bool_t _volume, dz_uint32_t * _seed, dz_vec3_t * _position )
{
    const dz_mesh_desc_t * mesh = dz_effect_find_mesh( _effect, _shape->mesh_id );
    if( mesh == DZ_NULLPTR || mesh->index_count < 3U )
    {
        return DZ_FAILURE_INVALID_DATA;
    }

    dz_float_t total_weight = 0.f;
    for( dz_uint32_t index = 0; index != mesh->index_count; index += 3U )
    {
        const dz_vec3_t a = mesh->vertices[mesh->indices[index + 0U]].position;
        const dz_vec3_t b = mesh->vertices[mesh->indices[index + 1U]].position;
        const dz_vec3_t c = mesh->vertices[mesh->indices[index + 2U]].position;
        total_weight += _volume == DZ_TRUE ? __calc_tetrahedron_volume( a, b, c ) : __calc_triangle_area3( a, b, c );
    }

    if( total_weight <= 0.f || isfinite( total_weight ) == 0 )
    {
        return DZ_FAILURE_INVALID_DATA;
    }

    dz_float_t selected_weight = __get_randf( _seed ) * total_weight;
    dz_uint32_t selected_index = 0U;
    for( dz_uint32_t index = 0; index != mesh->index_count; index += 3U )
    {
        const dz_vec3_t a = mesh->vertices[mesh->indices[index + 0U]].position;
        const dz_vec3_t b = mesh->vertices[mesh->indices[index + 1U]].position;
        const dz_vec3_t c = mesh->vertices[mesh->indices[index + 2U]].position;
        const dz_float_t weight = _volume == DZ_TRUE ? __calc_tetrahedron_volume( a, b, c ) : __calc_triangle_area3( a, b, c );
        selected_index = index;
        if( selected_weight <= weight )
        {
            break;
        }
        selected_weight -= weight;
    }

    const dz_vec3_t a = mesh->vertices[mesh->indices[selected_index + 0U]].position;
    const dz_vec3_t b = mesh->vertices[mesh->indices[selected_index + 1U]].position;
    const dz_vec3_t c = mesh->vertices[mesh->indices[selected_index + 2U]].position;

    if( _volume == DZ_FALSE )
    {
        const dz_float_t root = sqrtf( __get_randf( _seed ) );
        const dz_float_t r = __get_randf( _seed );
        *_position = dz_math_add3( dz_math_mul3( a, 1.f - root ), dz_math_add3( dz_math_mul3( b, root * ( 1.f - r ) ), dz_math_mul3( c, root * r ) ) );
        return DZ_SUCCESSFUL;
    }

    dz_float_t u = __get_randf( _seed );
    dz_float_t v = __get_randf( _seed );
    dz_float_t w = __get_randf( _seed );
    if( u + v > 1.f )
    {
        u = 1.f - u;
        v = 1.f - v;
    }
    if( v + w > 1.f )
    {
        const dz_float_t old_w = w;
        w = 1.f - u - v;
        v = 1.f - old_w;
    }
    else if( u + v + w > 1.f )
    {
        const dz_float_t old_w = w;
        w = u + v + w - 1.f;
        u = 1.f - v - old_w;
    }

    *_position = dz_math_add3( dz_math_mul3( a, u ), dz_math_add3( dz_math_mul3( b, v ), dz_math_mul3( c, w ) ) );
    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_uint32_t __calc_mask_threshold_value_count( const void * _buffer, dz_uint32_t _pitch, dz_uint32_t _bites, dz_uint32_t _width, dz_uint32_t _height, dz_uint32_t _threshold )
{
    dz_uint32_t threshold_value_count = 0;

    const void * buffer_iterator = _buffer;

    for( dz_uint32_t h = 0; h != _height; ++h )
    {
        for( dz_uint32_t w = 0; w != _width; ++w )
        {
            dz_uint32_t value = 0;

            switch( _bites )
            {
            case 1:
                {
                    const dz_uint8_t * point = (const dz_uint8_t *)buffer_iterator + w;

                    value = (dz_uint32_t)*point;
                }break;
            case 2:
                {
                    const dz_uint16_t * point = (const dz_uint16_t *)buffer_iterator + w;

                    value = (dz_uint32_t)*point;
                }break;
            case 4:
                {
                    const dz_uint32_t * point = (const dz_uint32_t *)buffer_iterator + w;

                    value = (dz_uint32_t)*point;
                }break;
            default:
                break;
            }

            if( value <= _threshold )
            {
                continue;
            }

            ++threshold_value_count;
        }

        buffer_iterator = (const dz_uint8_t *)buffer_iterator + _pitch;
    }

    return threshold_value_count;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __get_mask_threshold_value( const void * _buffer, dz_uint32_t _pitch, dz_uint32_t _bites, dz_uint32_t _width, dz_uint32_t _height, dz_uint32_t _threshold, dz_uint32_t _index, dz_uint32_t * _x, dz_uint32_t * _y )
{
    const void * buffer_iterator = _buffer;

    for( dz_uint32_t h = 0; h != _height; ++h )
    {
        for( dz_uint32_t w = 0; w != _width; ++w )
        {
            dz_uint32_t mask_value = 0;

            switch( _bites )
            {
            case 1:
                {
                    const dz_uint8_t * mask_point = (const dz_uint8_t *)buffer_iterator + w;

                    mask_value = (dz_uint32_t)*mask_point;
                }break;
            case 2:
                {
                    const dz_uint16_t * mask_point = (const dz_uint16_t *)buffer_iterator + w;

                    mask_value = (dz_uint32_t)*mask_point;
                }break;
            case 4:
                {
                    const dz_uint32_t * mask_point = (const dz_uint32_t *)buffer_iterator + w;

                    mask_value = (dz_uint32_t)*mask_point;
                }break;
            default:
                break;
            }

            if( mask_value <= _threshold )
            {
                continue;
            }

            if( _index-- == 0 )
            {
                *_x = w;
                *_y = h;

                return DZ_SUCCESSFUL;
            }
        }

        buffer_iterator = (const dz_uint8_t *)buffer_iterator + _pitch;
    }

    return DZ_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
static dz_float_t __effect_layer_get_life( const dz_effect_t * _effect, const dz_effect_layer_desc_t * _layer );
//////////////////////////////////////////////////////////////////////////
static dz_result_t __emitter_setup_particle( const dz_service_t * _service, dz_instance_t * _instance, dz_effect_emitter_instance_t * const _emitter_instance, dz_particle_t * _p, dz_float_t _life, dz_float_t _spawn_time )
{
    dz_uint32_t * seed = &_emitter_instance->seed;

    for( dz_uint32_t index = 0; index != __DZ_AFFECTOR_TIMELINE_MAX__; ++index )
    {
        _p->rands[index] = __get_randf( seed );
    }

    _p->life = _life;
    _p->time = 0.f;
    _p->layer_index = _emitter_instance->layer_index;

    _p->move_accelerate_aux = 0.f;
    _p->rotate_accelerate_aux = 0.f;
    _p->spin_accelerate_aux = 0.f;

    _p->x = _emitter_instance->x;
    _p->y = _emitter_instance->y;
    _p->z = _emitter_instance->z;

    _p->angle = _emitter_instance->angle;

    const dz_effect_t * effect = _instance->effect;
    const dz_effect_layer_desc_t * layer = effect->layers + _emitter_instance->layer_index;

    const dz_float_t layer_life = __effect_layer_get_life( effect, layer );
    const dz_float_t t = layer_life > 0.f ? _spawn_time / layer_life : 0.f;

    dz_shape_type_e shape_type = layer->shape->type;

    switch( shape_type )
    {
    case DZ_SHAPE_POINT:
        {
            _p->x += 0.f;
            _p->y += 0.f;

            const dz_float_t angle = __get_randf( seed ) * DZ_PI2;

            _p->angle += angle;
        }break;
    case DZ_SHAPE_SEGMENT:
        {
            _p->x += 0.f;
            _p->y += 0.f;

            const dz_float_t angle_min = __get_shape_value_seed( seed, layer, DZ_SHAPE_SEGMENT_ANGLE_MIN, t );
            const dz_float_t angle_max = __get_shape_value_seed( seed, layer, DZ_SHAPE_SEGMENT_ANGLE_MAX, t );

            const dz_float_t angle = __get_randf2( seed, angle_min, angle_max );

            _p->angle += angle;
        }break;
    case DZ_SHAPE_CIRCLE:
        {
            const dz_float_t radius_min = __get_shape_value_seed( seed, layer, DZ_SHAPE_CIRCLE_RADIUS_MIN, t );
            const dz_float_t radius_max = __get_shape_value_seed( seed, layer, DZ_SHAPE_CIRCLE_RADIUS_MAX, t );

            const dz_float_t angle = __get_randf( seed ) * DZ_PI2;
            const dz_float_t radius = radius_min + DZ_SQRTF( _service, __get_randf( seed ) ) * (radius_max - radius_min);

            const dz_float_t rx = radius * DZ_COSF( _service, angle );
            const dz_float_t ry = radius * DZ_SINF( _service, angle );

            _p->x += rx;
            _p->y += ry;

            const dz_float_t angle_min = __get_shape_value_seed( seed, layer, DZ_SHAPE_CIRCLE_ANGLE_MIN, t );
            const dz_float_t angle_max = __get_shape_value_seed( seed, layer, DZ_SHAPE_CIRCLE_ANGLE_MAX, t );

            const dz_float_t angle_dispersion = __get_randf2( seed, angle_min, angle_max );

            _p->angle += angle + angle_dispersion;
        }break;
    case DZ_SHAPE_LINE:
        {
            const dz_float_t angle = __get_shape_value_seed( seed, layer, DZ_SHAPE_LINE_ANGLE, t );

            const dz_float_t dx = DZ_COSF( _service, angle );
            const dz_float_t dy = DZ_SINF( _service, angle );

            const dz_float_t size = __get_shape_value_seed( seed, layer, DZ_SHAPE_LINE_SIZE, t );
            const dz_float_t offset = __get_shape_value_seed( seed, layer, DZ_SHAPE_LINE_OFFSET, t );

            const dz_float_t l = (__get_randf( seed ) - 0.5f) * size;

            _p->x += dy * (offset - l);
            _p->y += dx * (offset + l);

            _p->angle += angle;
        }break;
    case DZ_SHAPE_RECT:
        {
            const dz_float_t width_min = __get_shape_value_seed( seed, layer, DZ_SHAPE_RECT_WIDTH_MIN, t );
            const dz_float_t width_max = __get_shape_value_seed( seed, layer, DZ_SHAPE_RECT_WIDTH_MAX, t );
            const dz_float_t height_min = __get_shape_value_seed( seed, layer, DZ_SHAPE_RECT_HEIGHT_MIN, t );
            const dz_float_t height_max = __get_shape_value_seed( seed, layer, DZ_SHAPE_RECT_HEIGHT_MAX, t );

            DZ_TODO DZ_UNUSED( width_min );
            DZ_TODO DZ_UNUSED( height_min );

            const dz_float_t x = (__get_randf( seed ) - 0.5f) * width_max;
            const dz_float_t y = (__get_randf( seed ) - 0.5f) * height_max;

            _p->x += x;
            _p->y += y;

            dz_float_t angle = __get_randf( seed ) * DZ_PI2;

            _p->angle += angle;
        }break;
    case DZ_SHAPE_POLYGON:
        {
            dz_float_t total_area = 0.f;
            dz_float_t areas[1024];

            const dz_float_t * triangles = layer->shape->triangles;
            const dz_uint32_t triangle_count = layer->shape->triangle_count;

            if( triangles == DZ_NULLPTR || triangle_count == 0U || triangle_count > 1024U )
            {
                return DZ_FAILURE_INVALID_DATA;
            }

            for( dz_uint32_t index = 0; index != triangle_count; ++index )
            {
                const dz_float_t ax = triangles[index * 6 + 0];
                const dz_float_t ay = triangles[index * 6 + 1];
                const dz_float_t bx = triangles[index * 6 + 2];
                const dz_float_t by = triangles[index * 6 + 3];
                const dz_float_t cx = triangles[index * 6 + 4];
                const dz_float_t cy = triangles[index * 6 + 5];

                const dz_float_t area = __calc_triangle_area( ax, ay, bx, by, cx, cy );

                total_area += area;

                areas[index] = total_area;
            }

            const dz_float_t triangle_rand = __get_randf( seed );

            const dz_float_t triangle_find_area = triangle_rand * total_area;

            dz_uint32_t triangle_found_index = 0;

            for( dz_uint32_t index = 0; index != triangle_count; ++index )
            {
                const dz_float_t area = areas[index];

                if( area < triangle_find_area )
                {
                    continue;
                }

                triangle_found_index = index;

                break;
            }

            const dz_float_t rax = triangles[triangle_found_index * 6 + 0];
            const dz_float_t ray = triangles[triangle_found_index * 6 + 1];
            const dz_float_t rbx = triangles[triangle_found_index * 6 + 2];
            const dz_float_t rby = triangles[triangle_found_index * 6 + 3];
            const dz_float_t rcx = triangles[triangle_found_index * 6 + 4];
            const dz_float_t rcy = triangles[triangle_found_index * 6 + 5];

            const dz_float_t r1 = __get_randf( seed );
            const dz_float_t r2 = __get_randf( seed );

            const dz_float_t qr1 = DZ_SQRTF( _service, r1 );

            const dz_float_t tx = (1.f - qr1) * rax + (qr1 * (1.f - r2)) * rbx + (qr1 * r2) * rcx;
            const dz_float_t ty = (1.f - qr1) * ray + (qr1 * (1.f - r2)) * rby + (qr1 * r2) * rcy;

            _p->x += tx;
            _p->y += ty;

            const dz_float_t angle = __get_randf( seed ) * DZ_PI2;

            _p->angle += angle;
        }break;
    case DZ_SHAPE_MASK:
        {
            const void * mask_buffer = layer->shape->mask_buffer;
            const dz_uint32_t mask_bites = layer->shape->mask_bites;
            const dz_uint32_t mask_pitch = layer->shape->mask_pitch;
            const dz_uint32_t mask_width = layer->shape->mask_width;
            const dz_uint32_t mask_height = layer->shape->mask_height;
            const dz_uint32_t mask_threshold = layer->shape->mask_threshold;
            const dz_float_t mask_scale = layer->shape->mask_scale;

            const dz_uint32_t threshold_value_count = __calc_mask_threshold_value_count( mask_buffer, mask_pitch, mask_bites, mask_width, mask_height, mask_threshold );

            if( threshold_value_count == 0U )
            {
                return DZ_FAILURE_INVALID_DATA;
            }

            const dz_float_t r = __get_randf( seed );

            const dz_uint32_t threshold_value_index = (dz_uint32_t)(r * (threshold_value_count - 1) + 0.5f);

            dz_uint32_t w_found;
            dz_uint32_t h_found;
            if( __get_mask_threshold_value( mask_buffer, mask_pitch, mask_bites, mask_width, mask_height, mask_threshold, threshold_value_index, &w_found, &h_found ) == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }

            _p->x += w_found * mask_scale;
            _p->y += h_found * mask_scale;

            const dz_float_t angle = __get_randf( seed ) * DZ_PI2;

            _p->angle += angle;
        }break;
        case DZ_SHAPE_SPHERE:
        {
            const dz_float_t radius_min = __get_shape_value_seed( seed, layer, DZ_SHAPE_SPHERE_RADIUS_MIN, t );
            const dz_float_t radius_max = __get_shape_value_seed( seed, layer, DZ_SHAPE_SPHERE_RADIUS_MAX, t );
            const dz_float_t azimuth = __get_randf( seed ) * DZ_PI2;
            const dz_float_t normal_z = __get_randf( seed ) * 2.f - 1.f;
            const dz_float_t radial_xy = DZ_SQRTF( _service, DZ_MAX( 0.f, 1.f - normal_z * normal_z ) );
            const dz_float_t radius = radius_min + ( radius_max - radius_min ) * __get_randf( seed );

            _p->x += radius * radial_xy * DZ_COSF( _service, azimuth );
            _p->y += radius * radial_xy * DZ_SINF( _service, azimuth );
            _p->z += radius * normal_z;
            _p->angle += azimuth;
        }
        break;
        case DZ_SHAPE_BOX:
        {
            const dz_float_t width = __get_shape_value_seed( seed, layer, DZ_SHAPE_BOX_WIDTH, t );
            const dz_float_t height = __get_shape_value_seed( seed, layer, DZ_SHAPE_BOX_HEIGHT, t );
            const dz_float_t depth = __get_shape_value_seed( seed, layer, DZ_SHAPE_BOX_DEPTH, t );

            _p->x += ( __get_randf( seed ) - 0.5f ) * width;
            _p->y += ( __get_randf( seed ) - 0.5f ) * height;
            _p->z += ( __get_randf( seed ) - 0.5f ) * depth;
            _p->angle += __get_randf( seed ) * DZ_PI2;
        }
        break;
        case DZ_SHAPE_CONE:
        {
            const dz_float_t radius = __get_shape_value_seed( seed, layer, DZ_SHAPE_CONE_RADIUS, t );
            const dz_float_t height = __get_shape_value_seed( seed, layer, DZ_SHAPE_CONE_HEIGHT, t );
            const dz_float_t h = __get_randf( seed ) * height;
            const dz_float_t local_radius = radius * ( 1.f - h / DZ_MAX( height, 0.000001f ) ) * DZ_SQRTF( _service, __get_randf( seed ) );
            const dz_float_t azimuth = __get_randf( seed ) * DZ_PI2;

            _p->x += local_radius * DZ_COSF( _service, azimuth );
            _p->y += h;
            _p->z += local_radius * DZ_SINF( _service, azimuth );
            _p->angle += azimuth;
        }
        break;
        case DZ_SHAPE_CYLINDER:
        {
            const dz_float_t radius = __get_shape_value_seed( seed, layer, DZ_SHAPE_CYLINDER_RADIUS, t );
            const dz_float_t height = __get_shape_value_seed( seed, layer, DZ_SHAPE_CYLINDER_HEIGHT, t );
            const dz_float_t local_radius = radius * DZ_SQRTF( _service, __get_randf( seed ) );
            const dz_float_t azimuth = __get_randf( seed ) * DZ_PI2;

            _p->x += local_radius * DZ_COSF( _service, azimuth );
            _p->y += ( __get_randf( seed ) - 0.5f ) * height;
            _p->z += local_radius * DZ_SINF( _service, azimuth );
            _p->angle += azimuth;
        }
        break;
        case DZ_SHAPE_MESH_SURFACE:
        case DZ_SHAPE_MESH_VOLUME:
        {
            dz_vec3_t sampled_position;
            const dz_result_t sample_result = __sample_mesh_shape( effect, layer->shape, shape_type == DZ_SHAPE_MESH_VOLUME ? DZ_TRUE : DZ_FALSE, seed, &sampled_position );
            if( sample_result != DZ_SUCCESSFUL )
            {
                return sample_result;
            }

            _p->x += sampled_position.x;
            _p->y += sampled_position.y;
            _p->z += sampled_position.z;
        }
        break;
        case __DZ_SHAPE_MAX__:
        default:
            return DZ_FAILURE;
            break;
        }

    const dz_float_t spawn_x = _p->x - _emitter_instance->x;
    const dz_float_t spawn_y = _p->y - _emitter_instance->y;
    const dz_float_t layer_cos = DZ_COSF( _service, _emitter_instance->angle );
    const dz_float_t layer_sin = DZ_SINF( _service, _emitter_instance->angle );

    _p->x = _emitter_instance->x + spawn_x * layer_cos - spawn_y * layer_sin;
    _p->y = _emitter_instance->y + spawn_x * layer_sin + spawn_y * layer_cos;

    dz_vec3_t local_position = dz_math_vec3( _p->x - _emitter_instance->x, _p->y - _emitter_instance->y, _p->z - _emitter_instance->z );
    local_position = dz_math_transform_point( &layer->shape->transform, local_position );
    _p->x = _emitter_instance->x + local_position.x;
    _p->y = _emitter_instance->y + local_position.y;
    _p->z = _emitter_instance->z + local_position.z;

    const dz_float_t spin_min = __get_emitter_value_seed( seed, layer, DZ_EMITTER_SPAWN_SPIN_MIN, t );
    const dz_float_t spin_max = __get_emitter_value_seed( seed, layer, DZ_EMITTER_SPAWN_SPIN_MAX, t );

    _p->spin = (__get_randf( seed ) * 2.f - 1.f) * __get_randf2( seed, spin_min, spin_max );

    const dz_float_t sx = DZ_COSF( _service, _p->spin );
    const dz_float_t sy = DZ_SINF( _service, _p->spin );

    _p->sx = sx;
    _p->sy = sy;
    _p->sz = 0.f;

    const dz_float_t elevation_min = __get_emitter_value_seed( seed, layer, DZ_EMITTER_SPAWN_ELEVATION_MIN, t );
    const dz_float_t elevation_max = __get_emitter_value_seed( seed, layer, DZ_EMITTER_SPAWN_ELEVATION_MAX, t );
    const dz_float_t elevation = __get_randf2( seed, elevation_min, elevation_max );
    _p->vx = 0.f;
    _p->vy = 0.f;
    _p->vz = 0.f;
    _p->sz = DZ_SINF( _service, elevation );
    _p->initial_direction_z = _p->sz;
    _p->authored_speed = 0.f;
    _p->previous_x = _p->x;
    _p->previous_y = _p->y;
    _p->previous_z = _p->z;
    _p->birth_order = _instance->birth_order++;

    _p->born_color_r = _instance->r;
    _p->born_color_g = _instance->g;
    _p->born_color_b = _instance->b;
    _p->born_color_a = _instance->a;

    const dz_material_t * material = layer->material;

    switch( material->mode )
    {
    case DZ_MATERIAL_MODE_SOLID:
        {
            _p->texture = DZ_NULLPTR;
        }break;
    case DZ_MATERIAL_MODE_TEXTURE:
        {
            _p->texture = __material_get_texture_random_weight( material, seed );
        }break;
    case DZ_MATERIAL_MODE_SEQUENCE:
        {
            _p->texture = DZ_NULLPTR;
        }break;
    default:
        {
            _p->texture = DZ_NULLPTR;
        }break;
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __emitter_spawn_particle( const dz_service_t * _service, dz_instance_t * _instance, dz_effect_emitter_instance_t * const _emitter_instance, dz_float_t _life, dz_float_t _spawn_time )
{
    if( _instance->partices_count >= _instance->partices_capacity )
    {
        dz_uint16_t new_capacity;
        if( _instance->partices_capacity == 0 )
        {
            new_capacity = DZ_MIN( _instance->particle_limit, 16 );
        }
        else
        {
            const dz_uint32_t grown_capacity = (dz_uint32_t)_instance->partices_capacity * 2U;
            new_capacity = (dz_uint16_t)DZ_MIN( grown_capacity, _instance->particle_limit );
        }

        dz_particle_t * new_particles = DZ_REALLOCN( _service, _instance->partices, dz_particle_t, new_capacity );
        _instance->partices = new_particles;
        _instance->partices_capacity = new_capacity;
    }

    dz_particle_t * p = _instance->partices + _instance->partices_count;

    const dz_result_t setup_result = __emitter_setup_particle( _service, _instance, _emitter_instance, p, _life, _spawn_time );
    if( setup_result != DZ_SUCCESSFUL )
    {
        return setup_result;
    }

    ++_instance->partices_count;

    const dz_float_t time = _emitter_instance->time - _spawn_time;

    __particle_update( _service, _instance, p, time );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_instance_set_position( dz_instance_t * const _instance, dz_float_t _x, dz_float_t _y )
{
    _instance->x = _x;
    _instance->y = _y;
    _instance->transform.position.x = _x;
    _instance->transform.position.y = _y;
}
//////////////////////////////////////////////////////////////////////////
void dz_instance_get_position( const dz_instance_t * _instance, dz_float_t * const _x, dz_float_t * const _y )
{
    *_x = _instance->x;
    *_y = _instance->y;
}
//////////////////////////////////////////////////////////////////////////
void dz_instance_set_position3( dz_instance_t * const _instance, const dz_vec3_t * _position )
{
    _instance->x = _position->x;
    _instance->y = _position->y;
    _instance->z = _position->z;
    _instance->transform.position = *_position;
}
//////////////////////////////////////////////////////////////////////////
void dz_instance_get_position3( const dz_instance_t * _instance, dz_vec3_t * _position )
{
    *_position = _instance->transform.position;
}
//////////////////////////////////////////////////////////////////////////
void dz_instance_set_transform( dz_instance_t * const _instance, const dz_transform_t * _transform )
{
    _instance->transform = *_transform;
    _instance->transform.rotation = dz_math_quat_normalize( _transform->rotation );
    _instance->x = _transform->position.x;
    _instance->y = _transform->position.y;
    _instance->z = _transform->position.z;
}
//////////////////////////////////////////////////////////////////////////
void dz_instance_get_transform( const dz_instance_t * _instance, dz_transform_t * _transform )
{
    *_transform = _instance->transform;
}
//////////////////////////////////////////////////////////////////////////
void dz_instance_set_color( dz_instance_t * const _instance, dz_float_t _r, dz_float_t _g, dz_float_t _b, dz_float_t _a )
{
    _instance->r = _r;
    _instance->g = _g;
    _instance->b = _b;
    _instance->a = _a;
}
//////////////////////////////////////////////////////////////////////////
void dz_instance_get_color( const dz_instance_t * _instance, dz_float_t * const _r, dz_float_t * const _g, dz_float_t * const _b, dz_float_t * const _a )
{
    *_r = _instance->r;
    *_g = _instance->g;
    *_b = _instance->b;
    *_a = _instance->a;
}
//////////////////////////////////////////////////////////////////////////
void dz_instance_set_rotate( dz_instance_t * const _instance, dz_float_t _angle )
{
    _instance->angle = _angle;
}
//////////////////////////////////////////////////////////////////////////
dz_float_t dz_instance_get_rotate( const dz_instance_t * _instance )
{
    return _instance->angle;
}
//////////////////////////////////////////////////////////////////////////
void dz_instance_reset( dz_instance_t * const _instance )
{
    _instance->seed = _instance->init_seed;

    _instance->time = 0.f;
    _instance->emitter_instance_count = 0;
    _instance->started = DZ_FALSE;
    _instance->stopped = DZ_FALSE;
    _instance->paused = DZ_FALSE;
    _instance->emit_pause = DZ_FALSE;
    _instance->fixed_step_accumulator = 0.f;
    _instance->birth_order = 0;

    _instance->partices_count = 0;
    for( dz_uint32_t index = 0; index != _instance->effect->physics_object_count; ++index )
    {
        _instance->physics_transforms[index] = _instance->effect->physics_objects[index].transform;
    }
}
//////////////////////////////////////////////////////////////////////////
void dz_instance_restart( dz_instance_t * const _instance )
{
    dz_instance_reset( _instance );
}
//////////////////////////////////////////////////////////////////////////
void dz_instance_pause( dz_instance_t * const _instance )
{
    _instance->paused = DZ_TRUE;
}
//////////////////////////////////////////////////////////////////////////
void dz_instance_resume( dz_instance_t * const _instance )
{
    _instance->paused = DZ_FALSE;
}
//////////////////////////////////////////////////////////////////////////
dz_bool_t dz_instance_is_paused( const dz_instance_t * _instance )
{
    return _instance->paused;
}
//////////////////////////////////////////////////////////////////////////
void dz_instance_stop( dz_instance_t * const _instance )
{
    _instance->stopped = DZ_TRUE;
    _instance->emit_pause = DZ_TRUE;
    _instance->emitter_instance_count = 0;
}
//////////////////////////////////////////////////////////////////////////
void dz_instance_emit_pause( dz_instance_t * const _instance )
{
    _instance->emit_pause = DZ_TRUE;
}
//////////////////////////////////////////////////////////////////////////
void dz_instance_emit_resume( dz_instance_t * const _instance )
{
    _instance->emit_pause = DZ_FALSE;
}
//////////////////////////////////////////////////////////////////////////
dz_bool_t dz_instance_is_emit_pause( const dz_instance_t * _instance )
{
    return _instance->emit_pause;
}
//////////////////////////////////////////////////////////////////////////
static dz_particle_t * __find_first_dead_particle( dz_particle_t * _p, const dz_particle_t * _end )
{
    for( ; _p != _end; ++_p )
    {
        if( _p->time < 0.f )
        {
            return _p;
        }
    }

    return DZ_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
typedef struct dz_effect_runtime_event_t
{
    dz_effect_event_type_e type;
    dz_uint32_t source_layer_index;

    dz_float_t x;
    dz_float_t y;
    dz_float_t angle;

    dz_float_t sx;
    dz_float_t sy;
} dz_effect_runtime_event_t;
//////////////////////////////////////////////////////////////////////////
#define DZ_EFFECT_RUNTIME_EVENT_MAX 256
//////////////////////////////////////////////////////////////////////////
static dz_float_t __effect_layer_get_life( const dz_effect_t * _effect, const dz_effect_layer_desc_t * _layer )
{
    if( _layer->life > 0.f )
    {
        return _layer->life;
    }

    return _effect->life;
}
//////////////////////////////////////////////////////////////////////////
static void __push_runtime_event( dz_effect_runtime_event_t * const _events, dz_uint32_t * const _event_count, const dz_effect_runtime_event_t * _event )
{
    _events[(*_event_count)++] = *_event;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __instance_start_layer( dz_instance_t * const _instance, dz_uint32_t _layer_index, dz_float_t _x, dz_float_t _y, dz_float_t _angle, dz_float_t _sx, dz_float_t _sy, dz_float_t _delay )
{
    if( _layer_index >= _instance->effect->layer_count )
    {
        return DZ_FAILURE_INVALID_DATA;
    }

    dz_effect_emitter_instance_t * emitter_instance = _instance->emitter_instances + _instance->emitter_instance_count++;
    const dz_effect_layer_desc_t * layer = _instance->effect->layers + _layer_index;
    const dz_uint32_t base_seed = layer->seed != 0 ? layer->seed : _instance->seed;

    emitter_instance->layer_index = _layer_index;
    emitter_instance->seed = base_seed ^ (dz_uint32_t)__get_rand( &_instance->seed );
    emitter_instance->time = -DZ_MAX( _delay, 0.f );
    emitter_instance->emitter_time = 0.f;
    emitter_instance->x = _x;
    emitter_instance->y = _y;
    emitter_instance->z = _instance->z + layer->z;
    emitter_instance->angle = _angle;
    emitter_instance->sx = _sx;
    emitter_instance->sy = _sy;
    emitter_instance->active = DZ_TRUE;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_uint32_t __get_randu32_range( dz_uint32_t * const _seed, dz_uint32_t _min, dz_uint32_t _max )
{
    if( _max <= _min )
    {
        return _min;
    }

    const dz_uint32_t range = _max - _min + 1;
    const dz_uint32_t value = (dz_uint32_t)__get_rand( _seed ) % range;

    return _min + value;
}
//////////////////////////////////////////////////////////////////////////
static dz_bool_t __trigger_match_source( const dz_effect_trigger_desc_t * _trigger, const dz_effect_runtime_event_t * _event )
{
    if( _trigger->event_type == DZ_EFFECT_EVENT_EFFECT_START || _trigger->event_type == DZ_EFFECT_EVENT_TIME || _trigger->event_type == DZ_EFFECT_EVENT_CUSTOM )
    {
        return DZ_TRUE;
    }

    return _trigger->source_layer_index == _event->source_layer_index ? DZ_TRUE : DZ_FALSE;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __instance_process_trigger( const dz_service_t * _service, dz_instance_t * const _instance, const dz_effect_trigger_desc_t * _trigger, const dz_effect_runtime_event_t * _event )
{
    if( _trigger->event_type != _event->type )
    {
        return DZ_SUCCESSFUL;
    }

    if( __trigger_match_source( _trigger, _event ) == DZ_FALSE )
    {
        return DZ_SUCCESSFUL;
    }

    const dz_float_t probability = DZ_MAX( 0.f, DZ_MIN( _trigger->probability, 1.f ) );

    if( probability <= 0.f )
    {
        return DZ_SUCCESSFUL;
    }

    if( probability < 1.f && __get_randf( &_instance->seed ) > probability )
    {
        return DZ_SUCCESSFUL;
    }

    const dz_uint32_t spawn_count = __get_randu32_range( &_instance->seed, _trigger->spawn_count_min, _trigger->spawn_count_max );

    const dz_effect_t * effect = _instance->effect;
    const dz_effect_layer_desc_t * target_layer = effect->layers + _trigger->target_layer_index;

    for( dz_uint32_t index = 0; index != spawn_count; ++index )
    {
        dz_float_t x = _trigger->inherit_position == DZ_TRUE ? _event->x : _instance->x;
        dz_float_t y = _trigger->inherit_position == DZ_TRUE ? _event->y : _instance->y;
        dz_float_t angle = _trigger->inherit_angle == DZ_TRUE ? _event->angle : _instance->angle;

        x += target_layer->x + _trigger->offset_x;
        y += target_layer->y + _trigger->offset_y;
        angle += target_layer->angle + _trigger->angle_offset;

        const dz_float_t sx = _trigger->inherit_velocity == DZ_TRUE ? _event->sx : DZ_COSF( _service, angle );
        const dz_float_t sy = _trigger->inherit_velocity == DZ_TRUE ? _event->sy : DZ_SINF( _service, angle );
        const dz_float_t delay = __get_randf2( &_instance->seed, _trigger->delay_min, _trigger->delay_max );

        const dz_result_t start_result = __instance_start_layer( _instance, _trigger->target_layer_index, x, y, angle, sx, sy, delay );
        if( start_result != DZ_SUCCESSFUL )
        {
            return start_result;
        }
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __instance_process_event( const dz_service_t * _service, dz_instance_t * const _instance, const dz_effect_runtime_event_t * _event )
{
    const dz_effect_t * effect = _instance->effect;

    for( dz_uint32_t index = 0; index != effect->trigger_count; ++index )
    {
        const dz_effect_trigger_desc_t * trigger = effect->triggers + index;

        const dz_result_t trigger_result = __instance_process_trigger( _service, _instance, trigger, _event );
        if( trigger_result != DZ_SUCCESSFUL )
        {
            return trigger_result;
        }
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __instance_process_effect_start( const dz_service_t * _service, dz_instance_t * const _instance )
{
    dz_effect_runtime_event_t event;
    event.type = DZ_EFFECT_EVENT_EFFECT_START;
    event.source_layer_index = DZ_EFFECT_LAYER_NONE;
    event.x = _instance->x;
    event.y = _instance->y;
    event.angle = _instance->angle;
    event.sx = DZ_COSF( _service, _instance->angle );
    event.sy = DZ_SINF( _service, _instance->angle );

    return __instance_process_event( _service, _instance, &event );
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __instance_process_time_triggers( const dz_service_t * _service, dz_instance_t * const _instance, dz_float_t _old_time, dz_float_t _new_time, dz_bool_t _wrapped )
{
    const dz_effect_t * effect = _instance->effect;

    for( dz_uint32_t index = 0; index != effect->trigger_count; ++index )
    {
        const dz_effect_trigger_desc_t * trigger = effect->triggers + index;

        if( trigger->event_type != DZ_EFFECT_EVENT_TIME )
        {
            continue;
        }

        dz_bool_t crossed = DZ_FALSE;

        if( _wrapped == DZ_TRUE )
        {
            crossed = trigger->time > _old_time || trigger->time <= _new_time ? DZ_TRUE : DZ_FALSE;
        }
        else
        {
            crossed = trigger->time > _old_time && trigger->time <= _new_time ? DZ_TRUE : DZ_FALSE;
        }

        if( crossed == DZ_FALSE )
        {
            continue;
        }

        dz_effect_runtime_event_t event;
        event.type = DZ_EFFECT_EVENT_TIME;
        event.source_layer_index = DZ_EFFECT_LAYER_NONE;
        event.x = _instance->x;
        event.y = _instance->y;
        event.angle = _instance->angle;
        event.sx = DZ_COSF( _service, _instance->angle );
        event.sy = DZ_SINF( _service, _instance->angle );

        const dz_result_t trigger_result = __instance_process_trigger( _service, _instance, trigger, &event );
        if( trigger_result != DZ_SUCCESSFUL )
        {
            return trigger_result;
        }
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __emitter_update_instance( const dz_service_t * _service, dz_instance_t * const _instance, dz_effect_emitter_instance_t * const _emitter_instance, dz_float_t _time, dz_effect_runtime_event_t * const _events, dz_uint32_t * const _event_count )
{
    if( _emitter_instance->active == DZ_FALSE )
    {
        return DZ_SUCCESSFUL;
    }

    if( _emitter_instance->time < 0.f )
    {
        _emitter_instance->time += _time;

        if( _emitter_instance->time < 0.f )
        {
            return DZ_SUCCESSFUL;
        }

        _time = _emitter_instance->time;
        _emitter_instance->time = 0.f;
    }

    const dz_effect_t * effect = _instance->effect;
    const dz_effect_layer_desc_t * layer = effect->layers + _emitter_instance->layer_index;
    const dz_float_t layer_life = __effect_layer_get_life( effect, layer );

    if( layer_life <= 0.f )
    {
        _emitter_instance->active = DZ_FALSE;

        return DZ_SUCCESSFUL;
    }

    dz_float_t new_time = _emitter_instance->time + _time;

    if( new_time > layer_life )
    {
        new_time = layer_life;
    }

    _emitter_instance->time = new_time;

    for( ;; )
    {
        const dz_float_t instance_p = _emitter_instance->emitter_time / layer_life;

        dz_float_t delay = __get_emitter_value_seed( &_emitter_instance->seed, layer, DZ_EMITTER_SPAWN_DELAY, instance_p );

        if( delay <= 0.f )
        {
            delay = 0.0001f;
        }

        if( _emitter_instance->emitter_time + delay > layer_life )
        {
            break;
        }

        if( _emitter_instance->time - _emitter_instance->emitter_time < delay )
        {
            break;
        }

        const dz_float_t spawn_time = _emitter_instance->emitter_time + delay;
        const dz_float_t spawn_p = spawn_time / layer_life;

        dz_float_t count = __get_emitter_value_seed( &_emitter_instance->seed, layer, DZ_EMITTER_SPAWN_COUNT, spawn_p );

        while( count > 0.f )
        {
            const dz_float_t life = __get_affector_value_seed( &_emitter_instance->seed, layer, DZ_AFFECTOR_TIMELINE_LIFE, spawn_p );
            const dz_float_t ptime = _emitter_instance->time - spawn_time;

            if( life > ptime && _instance->partices_count < _instance->particle_limit )
            {
                const dz_result_t spawn_result = __emitter_spawn_particle( _service, _instance, _emitter_instance, life, spawn_time );
                if( spawn_result != DZ_SUCCESSFUL )
                {
                    return spawn_result;
                }
            }
            else
            {
                dz_particle_t p_fake;
                const dz_result_t setup_result = __emitter_setup_particle( _service, _instance, _emitter_instance, &p_fake, life, spawn_time );
                if( setup_result != DZ_SUCCESSFUL )
                {
                    return setup_result;
                }
            }

            count -= 1.f;
        }

        _emitter_instance->emitter_time += delay;
    }

    if( _emitter_instance->time >= layer_life )
    {
        dz_effect_runtime_event_t event;
        event.type = DZ_EFFECT_EVENT_LAYER_EMIT_COMPLETE;
        event.source_layer_index = _emitter_instance->layer_index;
        event.x = _emitter_instance->x;
        event.y = _emitter_instance->y;
        event.angle = _emitter_instance->angle;
        event.sx = _emitter_instance->sx;
        event.sy = _emitter_instance->sy;

        __push_runtime_event( _events, _event_count, &event );

        _emitter_instance->active = DZ_FALSE;
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static void __instance_compact_emitter_instances( dz_instance_t * const _instance )
{
    dz_uint32_t write = 0;

    for( dz_uint32_t index = 0; index != _instance->emitter_instance_count; ++index )
    {
        const dz_effect_emitter_instance_t * emitter_instance = _instance->emitter_instances + index;

        if( emitter_instance->active == DZ_FALSE )
        {
            continue;
        }

        _instance->emitter_instances[write++] = *emitter_instance;
    }

    _instance->emitter_instance_count = write;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __instance_update_step( const dz_service_t * _service, dz_instance_t * const _instance, dz_float_t _time )
{
    const dz_effect_t * effect = _instance->effect;

    dz_effect_runtime_event_t events[DZ_EFFECT_RUNTIME_EVENT_MAX];
    dz_uint32_t event_count = 0;

    dz_particle_t * p = _instance->partices;
    const dz_particle_t * p_end = _instance->partices + _instance->partices_count;

    while( p != p_end )
    {
        if( p->time < 0.f )
        {
            return DZ_FAILURE_INVALID_DATA;
        }

        if( p->time + _time < p->life )
        {
            __particle_update( _service, _instance, p, _time );
        }
        else
        {
            const dz_float_t death_time = p->life - p->time;

            if( death_time > 0.f )
            {
                __particle_update( _service, _instance, p, death_time );
            }

            dz_effect_runtime_event_t event;
            event.type = DZ_EFFECT_EVENT_PARTICLE_DEATH;
            event.source_layer_index = p->layer_index;
            event.x = p->x;
            event.y = p->y;
            event.angle = p->angle;
            event.sx = p->sx;
            event.sy = p->sy;

            __push_runtime_event( events, &event_count, &event );

            p->time = -1.f;
        }

        ++p;
    }

    dz_particle_t * p_dead = __find_first_dead_particle( _instance->partices, p_end );

    if( p_dead != DZ_NULLPTR )
    {
        if( _instance->partices_count == 0 )
        {
            return DZ_FAILURE_INVALID_DATA;
        }

        --_instance->partices_count;

        const dz_particle_t * p_sweep = p_dead;

        ++p_sweep;

        for( ; p_sweep != p_end; ++p_sweep )
        {
            if( p_sweep->time >= 0.f )
            {
                *p_dead++ = *p_sweep;
            }
            else
            {
                if( _instance->partices_count == 0 )
                {
                    return DZ_FAILURE_INVALID_DATA;
                }

                --_instance->partices_count;
            }
        }
    }

    if( _instance->emit_pause == DZ_TRUE )
    {
        return DZ_SUCCESSFUL;
    }

    const dz_float_t effect_life = effect->life;
    if( effect_life <= 0.f || isfinite( effect_life ) == 0 )
    {
        return DZ_FAILURE_INVALID_DATA;
    }

    const dz_float_t old_time = _instance->time;
    dz_bool_t wrapped = DZ_FALSE;

    if( _instance->time + _time > effect_life )
    {
        if( _instance->loop == DZ_FALSE )
        {
            _instance->time = effect_life;
        }
        else
        {
            _instance->time += _time - effect_life;
            wrapped = DZ_TRUE;
            _instance->started = DZ_FALSE;
        }
    }
    else
    {
        _instance->time += _time;
    }

    if( _instance->started == DZ_FALSE )
    {
        const dz_result_t start_result = __instance_process_effect_start( _service, _instance );
        if( start_result != DZ_SUCCESSFUL )
        {
            return start_result;
        }

        _instance->started = DZ_TRUE;
    }

    const dz_result_t time_trigger_result = __instance_process_time_triggers( _service, _instance, old_time, _instance->time, wrapped );
    if( time_trigger_result != DZ_SUCCESSFUL )
    {
        return time_trigger_result;
    }

    for( dz_uint32_t index = 0; index != event_count; ++index )
    {
        const dz_result_t event_result = __instance_process_event( _service, _instance, events + index );
        if( event_result != DZ_SUCCESSFUL )
        {
            return event_result;
        }
    }

    event_count = 0;

    for( dz_uint32_t index = 0; index != _instance->emitter_instance_count; ++index )
    {
        const dz_result_t emitter_result = __emitter_update_instance( _service, _instance, _instance->emitter_instances + index, _time, events, &event_count );
        if( emitter_result != DZ_SUCCESSFUL )
        {
            return emitter_result;
        }
    }

    for( dz_uint32_t index = 0; index != event_count; ++index )
    {
        const dz_result_t event_result = __instance_process_event( _service, _instance, events + index );
        if( event_result != DZ_SUCCESSFUL )
        {
            return event_result;
        }
    }

    __instance_compact_emitter_instances( _instance );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_instance_update( const dz_service_t * _service, dz_instance_t * const _instance, dz_float_t _time )
{
    if( _instance->stopped == DZ_TRUE || _instance->paused == DZ_TRUE || _time == 0.f )
    {
        return DZ_SUCCESSFUL;
    }

    _instance->fixed_step_accumulator += (double)_time;
    const double step = (double)_instance->fixed_step;
    const double step_epsilon = step * 0.0001;

    while( _instance->fixed_step_accumulator + step_epsilon >= step )
    {
        const dz_result_t result = __instance_update_step( _service, _instance, (dz_float_t)step );
        if( result != DZ_SUCCESSFUL )
        {
            return result;
        }

        _instance->fixed_step_accumulator -= step;
    }

    if( _instance->fixed_step_accumulator < 0.0 && _instance->fixed_step_accumulator > -step_epsilon )
    {
        _instance->fixed_step_accumulator = 0.0;
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_instance_seek( const dz_service_t * _service, dz_instance_t * const _instance, dz_float_t _time, dz_float_t _step )
{
    dz_instance_reset( _instance );
    _instance->fixed_step = _step;

    const double quotient = (double)_time / (double)_step;
    const dz_uint32_t full_steps = (dz_uint32_t)( quotient + 0.0001 );
    for( dz_uint32_t index = 0; index != full_steps; ++index )
    {
        const dz_result_t result = __instance_update_step( _service, _instance, _step );
        if( result != DZ_SUCCESSFUL )
        {
            dz_instance_reset( _instance );
            return result;
        }
    }

    const double remainder = (double)_time - (double)full_steps * (double)_step;
    if( remainder > (double)_step * 0.0001 )
    {
        const dz_result_t result = __instance_update_step( _service, _instance, (dz_float_t)remainder );
        if( result != DZ_SUCCESSFUL )
        {
            dz_instance_reset( _instance );
            return result;
        }
    }

    _instance->fixed_step_accumulator = 0.f;
    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_instance_state_e dz_instance_get_state( const dz_instance_t * _instance )
{
    dz_instance_state_e state = DZ_INSTANCE_PROCESS;

    if( _instance->loop == DZ_TRUE )
    {
        return state;
    }

    const dz_effect_t * effect = _instance->effect;

    if( _instance->time < effect->life || _instance->emitter_instance_count != 0 )
    {
        return state;
    }

    state |= DZ_INSTANCE_EMIT_COMPLETE;

    if( _instance->partices_count == 0U )
    {
        state |= DZ_INSTANCE_PARTICLE_COMPLETE;
    }

    return state;
}
//////////////////////////////////////////////////////////////////////////
dz_uint16_t dz_instance_get_particle_count( const dz_instance_t * _instance )
{
    return _instance->partices_count;
}
//////////////////////////////////////////////////////////////////////////
void dz_instance_get_particle_state( const dz_instance_t * _instance, dz_uint16_t _index, dz_particle_state_t * _state )
{
    const dz_particle_t * particle = _instance->partices + _index;
    _state->position = dz_math_vec3( particle->x, particle->y, particle->z );
    _state->previous_position = dz_math_vec3( particle->previous_x, particle->previous_y, particle->previous_z );
    _state->velocity = dz_math_vec3( particle->vx, particle->vy, particle->vz );
    _state->age = particle->time;
    _state->life = particle->life;
    _state->layer_index = particle->layer_index;
    _state->birth_order = particle->birth_order;
}
//////////////////////////////////////////////////////////////////////////
void dz_instance_get_aabb( const dz_instance_t * _instance, dz_aabb_t * _aabb )
{
    _aabb->valid = DZ_FALSE;
    _aabb->minimum = dz_math_vec3( 0.f, 0.f, 0.f );
    _aabb->maximum = dz_math_vec3( 0.f, 0.f, 0.f );

    for( dz_uint16_t index = 0; index != _instance->partices_count; ++index )
    {
        const dz_particle_t * particle = _instance->partices + index;
        const dz_effect_layer_desc_t * layer = _instance->effect->layers + particle->layer_index;
        dz_float_t extent = DZ_PARTICLE_SIZE * 0.5f * fabsf( particle->scale ) * DZ_MAX( fabsf( particle->aspect ), 1.f );
        if( particle->texture != DZ_NULLPTR )
        {
            extent = 0.5f * DZ_MAX( particle->texture->width, particle->texture->height ) * fabsf( particle->scale ) * DZ_MAX( fabsf( particle->aspect ), 1.f );
        }
        else if( layer->particle_mode == DZ_PARTICLE_MODE_MESH && layer->mesh_id != DZ_RESOURCE_ID_NONE )
        {
            const dz_mesh_desc_t * mesh = dz_effect_find_mesh( _instance->effect, layer->mesh_id );
            if( mesh != DZ_NULLPTR )
            {
                const dz_float_t mx = DZ_MAX( fabsf( mesh->bounds.minimum.x ), fabsf( mesh->bounds.maximum.x ) ) * fabsf( layer->scale.x * particle->scale * particle->aspect );
                const dz_float_t my = DZ_MAX( fabsf( mesh->bounds.minimum.y ), fabsf( mesh->bounds.maximum.y ) ) * fabsf( layer->scale.y * particle->scale );
                const dz_float_t mz = DZ_MAX( fabsf( mesh->bounds.minimum.z ), fabsf( mesh->bounds.maximum.z ) ) * fabsf( layer->scale.z * particle->scale );
                extent = sqrtf( mx * mx + my * my + mz * mz );
            }
        }

        dz_vec3_t segment_start = dz_math_vec3( particle->previous_x, particle->previous_y, particle->previous_z );
        if( layer->particle_mode == DZ_PARTICLE_MODE_TRAIL )
        {
            const dz_float_t trail_age = DZ_MIN( DZ_MAX( particle->time, 0.f ), layer->trail_lifetime );
            segment_start =
                dz_math_sub3( dz_math_vec3( particle->x, particle->y, particle->z ), dz_math_mul3( dz_math_vec3( particle->vx, particle->vy, particle->vz ), trail_age ) );
            extent = DZ_MAX( extent, layer->trail_width * fabsf( particle->scale ) * 0.5f );
        }
        else if( layer->particle_mode == DZ_PARTICLE_MODE_BEAM )
        {
            segment_start = dz_math_vec3( particle->birth_x, particle->birth_y, particle->birth_z );
            extent = DZ_MAX( extent, layer->trail_width * fabsf( particle->scale ) * 0.5f );
        }

        const dz_vec3_t minimum =
            dz_math_vec3( DZ_MIN( particle->x, segment_start.x ) - extent, DZ_MIN( particle->y, segment_start.y ) - extent, DZ_MIN( particle->z, segment_start.z ) - extent );
        const dz_vec3_t maximum =
            dz_math_vec3( DZ_MAX( particle->x, segment_start.x ) + extent, DZ_MAX( particle->y, segment_start.y ) + extent, DZ_MAX( particle->z, segment_start.z ) + extent );

        if( _aabb->valid == DZ_FALSE )
        {
            _aabb->minimum = minimum;
            _aabb->maximum = maximum;
            _aabb->valid = DZ_TRUE;
        }
        else
        {
            _aabb->minimum.x = DZ_MIN( _aabb->minimum.x, minimum.x );
            _aabb->minimum.y = DZ_MIN( _aabb->minimum.y, minimum.y );
            _aabb->minimum.z = DZ_MIN( _aabb->minimum.z, minimum.z );
            _aabb->maximum.x = DZ_MAX( _aabb->maximum.x, maximum.x );
            _aabb->maximum.y = DZ_MAX( _aabb->maximum.y, maximum.y );
            _aabb->maximum.z = DZ_MAX( _aabb->maximum.z, maximum.z );
        }
    }

}
