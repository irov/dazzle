#include "dazzle/dazzle.h"

#include "service.h"
#include "timeline_key.h"
#include "timeline_interpolate.h"
#include "shape.h"
#include "affector.h"
#include "emitter.h"
#include "particle.h"
#include "texture.h"
#include "atlas.h"
#include "material.h"
#include "emitter.h"
#include "effect.h"

#include "alloc.h"
#include "math.h"

//////////////////////////////////////////////////////////////////////////
uint32_t dz_get_magic( void )
{
    return 'D' + ('A' << 8) + ('Z' << 16) + ('Z' << 24);
}
//////////////////////////////////////////////////////////////////////////
uint32_t dz_get_version( void )
{
    return 1;
}
//////////////////////////////////////////////////////////////////////////
typedef struct dz_timeline_limits_t
{
    dz_timeline_limit_status_e status;

    float min_value;
    float max_value;
    float default_value;
    float factor_value;
} dz_timeline_limits_t;
//////////////////////////////////////////////////////////////////////////
static const dz_timeline_limits_t affector_timeline_limits[__DZ_AFFECTOR_TIMELINE_MAX__] = {
    {DZ_TIMELINE_LIMIT_MAX, 0.f, DZ_FLT_MAX, 5.f, 10.f}, //DZ_AFFECTOR_TIMELINE_LIFE
    {DZ_TIMELINE_LIMIT_MAX, 0.f, DZ_FLT_MAX, 50.f, 100.f}, //DZ_AFFECTOR_TIMELINE_MOVE_SPEED
    {DZ_TIMELINE_LIMIT_MINMAX, DZ_FLT_MIN, DZ_FLT_MAX, 0.f, 100.f}, //DZ_AFFECTOR_TIMELINE_MOVE_ACCELERATE
    {DZ_TIMELINE_LIMIT_MINMAX, DZ_FLT_MIN, DZ_FLT_MAX, 0.f, DZ_PI2}, //DZ_AFFECTOR_TIMELINE_ROTATE_SPEED
    {DZ_TIMELINE_LIMIT_MINMAX, DZ_FLT_MIN, DZ_FLT_MAX, 0.f, DZ_PI2}, //DZ_AFFECTOR_TIMELINE_ROTATE_ACCELERATE
    {DZ_TIMELINE_LIMIT_MINMAX, DZ_FLT_MIN, DZ_FLT_MAX, 0.f, DZ_PI2}, //DZ_AFFECTOR_TIMELINE_SPIN_SPEED
    {DZ_TIMELINE_LIMIT_MINMAX, DZ_FLT_MIN, DZ_FLT_MAX, 0.f, DZ_PI2}, //DZ_AFFECTOR_TIMELINE_SPIN_ACCELERATE
    {DZ_TIMELINE_LIMIT_MINMAX, DZ_FLT_MIN, DZ_FLT_MAX, 0.f, 1.f}, //DZ_AFFECTOR_TIMELINE_STRAFE_SPEED
    {DZ_TIMELINE_LIMIT_MINMAX, DZ_FLT_MIN, DZ_FLT_MAX, 0.f, DZ_PI2}, //DZ_AFFECTOR_TIMELINE_STRAFE_FRENQUENCE
    {DZ_TIMELINE_LIMIT_MINMAX, DZ_FLT_MIN, DZ_FLT_MAX, 0.f, 10.f}, //DZ_AFFECTOR_TIMELINE_STRAFE_SIZE
    {DZ_TIMELINE_LIMIT_MINMAX, DZ_PI2N, DZ_PI2, 0.f, 0.f}, //DZ_AFFECTOR_TIMELINE_STRAFE_SHIFT
    {DZ_TIMELINE_LIMIT_MAX, 0.f, DZ_FLT_MAX, 50.f, 100.f}, //DZ_AFFECTOR_TIMELINE_SIZE
    {DZ_TIMELINE_LIMIT_NORMAL, 0.f, 1.f, 1.f, 1.f}, //DZ_AFFECTOR_TIMELINE_COLOR_R
    {DZ_TIMELINE_LIMIT_NORMAL, 0.f, 1.f, 1.f, 1.f}, //DZ_AFFECTOR_TIMELINE_COLOR_G
    {DZ_TIMELINE_LIMIT_NORMAL, 0.f, 1.f, 1.f, 1.f}, //DZ_AFFECTOR_TIMELINE_COLOR_B
    {DZ_TIMELINE_LIMIT_NORMAL, 0.f, 1.f, 1.f, 1.f} //DZ_AFFECTOR_TIMELINE_COLOR_A
};
//////////////////////////////////////////////////////////////////////////
void dz_affector_timeline_get_limit( dz_affector_timeline_type_e _timeline, dz_timeline_limit_status_e * _status, float * _min, float * _max, float * _default, float * _factor )
{
    const dz_timeline_limits_t * limit = affector_timeline_limits + _timeline;

    *_status = limit->status;
    *_min = limit->min_value;
    *_max = limit->max_value;
    *_default = limit->default_value;
    *_factor = limit->factor_value;
}
//////////////////////////////////////////////////////////////////////////
static float __get_affector_timeline_default( dz_affector_timeline_type_e _timeline )
{
    const dz_timeline_limits_t * limit = affector_timeline_limits + _timeline;

    float default_value = limit->default_value;

    return default_value;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_service_create( dz_service_t ** _service, const dz_service_providers_t * _providers, dz_userdata_t _ud )
{
    dz_service_t * service = (dz_service_t *)(*_providers->f_malloc)(sizeof( dz_service_t ), _ud);

    service->providers = *_providers;
    service->ud = _ud;

    *_service = service;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_service_destroy( dz_service_t * _service )
{
    DZ_FREE( _service, _service );
}
//////////////////////////////////////////////////////////////////////////
void dz_service_get_providers( dz_service_t * _service, dz_service_providers_t * _providers )
{
    *_providers = _service->providers;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_texture_create( dz_service_t * _service, dz_texture_t ** _texture, dz_userdata_t _ud )
{
    dz_texture_t * texture = DZ_NEW( _service, dz_texture_t );

    texture->u[0] = 0.f;
    texture->v[0] = 0.f;
    texture->u[1] = 1.f;
    texture->v[1] = 0.f;
    texture->u[2] = 1.f;
    texture->v[2] = 1.f;
    texture->u[3] = 1.f;
    texture->v[3] = 0.f;

    texture->trim_offset_x = 0.f;
    texture->trim_offset_y = 0.f;

    texture->trim_width = 1.f;
    texture->trim_height = 1.f;

    texture->random_weight = 1.f;
    texture->sequence_delay = 1.f;

    texture->ud = _ud;

    *_texture = texture;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_texture_destroy( dz_service_t * _service, const dz_texture_t * _texture )
{
    DZ_FREE( _service, _texture );
}
//////////////////////////////////////////////////////////////////////////
void dz_texture_set_ud( dz_texture_t * _texture, dz_userdata_t _ud )
{
    _texture->ud = _ud;
}
//////////////////////////////////////////////////////////////////////////
dz_userdata_t dz_texture_get_ud( const dz_texture_t * _texture )
{
    return _texture->ud;
}
//////////////////////////////////////////////////////////////////////////
void dz_texture_set_uv( dz_texture_t * _texture, const float * _u, const float * _v )
{
    _texture->u[0] = _u[0];
    _texture->v[0] = _v[0];
    _texture->u[1] = _u[1];
    _texture->v[1] = _v[1];
    _texture->u[2] = _u[2];
    _texture->v[2] = _v[2];
    _texture->u[3] = _u[3];
    _texture->v[3] = _v[3];
}
//////////////////////////////////////////////////////////////////////////
void dz_texture_get_uv( const dz_texture_t * _texture, float * _u, float * _v )
{
    _u[0] = _texture->u[0];
    _v[0] = _texture->v[0];
    _u[1] = _texture->u[1];
    _v[1] = _texture->v[1];
    _u[2] = _texture->u[2];
    _v[2] = _texture->v[2];
    _u[3] = _texture->u[3];
    _v[3] = _texture->v[3];
}
//////////////////////////////////////////////////////////////////////////
void dz_texture_set_trim_offset( dz_texture_t * _texture, float _x, float _y )
{
    _texture->trim_offset_x = _x;
    _texture->trim_offset_y = _y;
}
//////////////////////////////////////////////////////////////////////////
void dz_texture_get_trim_offset( const dz_texture_t * _texture, float * _x, float * _y )
{
    *_x = _texture->trim_offset_x;
    *_y = _texture->trim_offset_y;
}
//////////////////////////////////////////////////////////////////////////
void dz_texture_set_trim_width( dz_texture_t * _texture, float _width, float _height )
{
    _texture->trim_width = _width;
    _texture->trim_height = _height;
}
//////////////////////////////////////////////////////////////////////////
void dz_texture_get_trim_width( const dz_texture_t * _texture, float * _width, float * _height )
{
    *_width = _texture->trim_width;
    *_height = _texture->trim_height;
}
//////////////////////////////////////////////////////////////////////////
void dz_texture_set_random_weight( dz_texture_t * _texture, float _weight )
{
    _texture->random_weight = _weight;
}
//////////////////////////////////////////////////////////////////////////
float dz_texture_get_random_weight( const dz_texture_t * _texture )
{
    return _texture->random_weight;
}
//////////////////////////////////////////////////////////////////////////
void dz_texture_set_sequence_delay( dz_texture_t * _texture, float _delay )
{
    _texture->sequence_delay = _delay;
}
//////////////////////////////////////////////////////////////////////////
float dz_texture_get_sequence_delay( const dz_texture_t * _texture )
{
    return _texture->sequence_delay;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_atlas_create( dz_service_t * _service, dz_atlas_t ** _atlas, dz_userdata_t _surface, dz_userdata_t _ud )
{
    dz_atlas_t * atlas = DZ_NEW( _service, dz_atlas_t );

    atlas->texture_count = 0;

    atlas->surface = _surface;
    atlas->ud = _ud;

    *_atlas = atlas;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_atlas_destroy( dz_service_t * _service, const dz_atlas_t * _atlas )
{
    DZ_FREE( _service, _atlas );
}
//////////////////////////////////////////////////////////////////////////
void dz_atlas_set_surface( dz_atlas_t * _atlas, dz_userdata_t _surface )
{
    _atlas->surface = _surface;
}
//////////////////////////////////////////////////////////////////////////
dz_userdata_t dz_atlas_get_surface( const dz_atlas_t * _atlas )
{
    return _atlas->surface;
}
//////////////////////////////////////////////////////////////////////////
void dz_atlas_set_ud( dz_atlas_t * _atlas, dz_userdata_t _ud )
{
    _atlas->ud = _ud;
}
//////////////////////////////////////////////////////////////////////////
dz_userdata_t dz_atlas_get_ud( const dz_atlas_t * _atlas )
{
    return _atlas->ud;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_atlas_add_texture( dz_atlas_t * _atlas, const dz_texture_t * _texture )
{
    _atlas->textures[_atlas->texture_count++] = _texture;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_atlas_get_texture( const dz_atlas_t * _atlas, uint32_t _index, const dz_texture_t ** _texture )
{
    const dz_texture_t * texture = _atlas->textures[_index];

    *_texture = texture;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
uint32_t dz_atlas_get_texture_count( const dz_atlas_t * _atlas )
{
    return _atlas->texture_count;
}
//////////////////////////////////////////////////////////////////////////
dz_blend_type_e dz_material_get_default_blend( void )
{
    return DZ_BLEND_NORNAL;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_material_create( dz_service_t * _service, dz_material_t ** _material, dz_userdata_t _ud )
{
    dz_material_t * material = DZ_NEW( _service, dz_material_t );

    material->blend_type = dz_material_get_default_blend();

    material->r = 1.f;
    material->g = 1.f;
    material->b = 1.f;
    material->a = 1.f;

    material->ud = _ud;

    *_material = material;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_material_destroy( dz_service_t * _service, const dz_material_t * _material )
{
    DZ_FREE( _service, _material );
}
//////////////////////////////////////////////////////////////////////////
void dz_material_set_ud( dz_material_t * _material, dz_userdata_t _ud )
{
    _material->ud = _ud;
}
//////////////////////////////////////////////////////////////////////////
dz_userdata_t dz_material_get_ud( const dz_material_t * _material )
{
    return _material->ud;
}
//////////////////////////////////////////////////////////////////////////
void dz_material_set_blend( dz_material_t * _material, dz_blend_type_e _blend )
{
    _material->blend_type = _blend;
}
//////////////////////////////////////////////////////////////////////////
dz_blend_type_e dz_material_get_blend( const dz_material_t * _material )
{
    return _material->blend_type;
}
//////////////////////////////////////////////////////////////////////////
void dz_material_set_color( dz_material_t * _material, float _r, float _g, float _b, float _a )
{
    _material->r = _r;
    _material->g = _g;
    _material->b = _b;
    _material->a = _a;
}
//////////////////////////////////////////////////////////////////////////
void dz_material_get_color( const dz_material_t * _material, float * _r, float * _g, float * _b, float * _a )
{
    *_r = _material->r;
    *_g = _material->g;
    *_b = _material->b;
    *_a = _material->a;
}
//////////////////////////////////////////////////////////////////////////
void dz_material_set_atlas( dz_material_t * _material, const dz_atlas_t * _atlas )
{
    _material->atlas = _atlas;
}
//////////////////////////////////////////////////////////////////////////
const dz_atlas_t * dz_material_get_atlas( const dz_material_t * _material )
{
    return _material->atlas;
}
//////////////////////////////////////////////////////////////////////////
void dz_material_set_mode( dz_material_t * _material, dz_material_mode_e _mode )
{
    _material->mode = _mode;
}
//////////////////////////////////////////////////////////////////////////
dz_material_mode_e dz_material_get_mode( const dz_material_t * _material )
{
    return _material->mode;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_timeline_interpolate_create( dz_service_t * _service, dz_timeline_interpolate_t ** _interpolate, dz_timeline_interpolate_type_e _type, dz_userdata_t _ud )
{
    dz_timeline_interpolate_t * interpolate = DZ_NEW( _service, dz_timeline_interpolate_t );

    interpolate->type = _type;
    interpolate->key = DZ_NULLPTR;
    interpolate->ud = _ud;

    interpolate->p0 = 0.f;
    interpolate->p1 = 0.f;

    *_interpolate = interpolate;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_timeline_interpolate_destroy( dz_service_t * _service, const dz_timeline_interpolate_t * _interpolate )
{
    if( _interpolate->key != DZ_NULLPTR )
    {
        dz_timeline_key_destroy( _service, _interpolate->key );
    }

    DZ_FREE( _service, _interpolate );
}
//////////////////////////////////////////////////////////////////////////
void dz_timeline_interpolate_set_ud( dz_timeline_interpolate_t * _interpolate, dz_userdata_t _ud )
{
    _interpolate->ud = _ud;
}
//////////////////////////////////////////////////////////////////////////
dz_userdata_t dz_timeline_interpolate_get_ud( const dz_timeline_interpolate_t * _interpolate )
{
    return _interpolate->ud;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_timeline_key_create( dz_service_t * _service, dz_timeline_key_t ** _key, float _p, dz_timeline_key_type_e _type, dz_userdata_t _ud )
{
#ifdef DZ_DEBUG
    if( _p < 0.f || _p > 1.f )
    {
        return DZ_FAILURE;
    }
#endif

    dz_timeline_key_t * key = DZ_NEW( _service, dz_timeline_key_t );

    key->p = _p;
    key->type = _type;
    key->interpolate = DZ_NULLPTR;
    key->ud = _ud;
    key->const_value = 0.f;
    key->randomize_min_value = 0.f;
    key->randomize_max_value = 0.f;

    *_key = key;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_timeline_key_destroy( dz_service_t * _service, const dz_timeline_key_t * _key )
{
    if( _key->interpolate != DZ_NULLPTR )
    {
        dz_timeline_interpolate_destroy( _service, _key->interpolate );
    }

    DZ_FREE( _service, _key );
}
//////////////////////////////////////////////////////////////////////////
void dz_timeline_key_set_ud( dz_timeline_key_t * _key, dz_userdata_t _ud )
{
    _key->ud = _ud;
}
//////////////////////////////////////////////////////////////////////////
dz_userdata_t dz_timeline_key_get_ud( const dz_timeline_key_t * _key )
{
    return _key->ud;
}
//////////////////////////////////////////////////////////////////////////
void dz_timeline_key_set_type( dz_timeline_key_t * _key, dz_timeline_key_type_e _type )
{
    _key->type = _type;
}
//////////////////////////////////////////////////////////////////////////
dz_timeline_key_type_e dz_timeline_key_get_type( const dz_timeline_key_t * _key )
{
    return _key->type;
}
//////////////////////////////////////////////////////////////////////////
const dz_timeline_key_t * dz_timeline_interpolate_get_key( const dz_timeline_interpolate_t * _interpolate )
{
    const dz_timeline_key_t * key = _interpolate->key;

    return key;
}
//////////////////////////////////////////////////////////////////////////
const dz_timeline_interpolate_t * dz_timeline_key_get_interpolate( const dz_timeline_key_t * _key )
{
    const dz_timeline_interpolate_t * interpolate = _key->interpolate;

    return interpolate;
}
//////////////////////////////////////////////////////////////////////////
void dz_timeline_key_set_interpolate( dz_timeline_key_t * _key0, dz_timeline_interpolate_t * _interpolate, dz_timeline_key_t * _key1 )
{
    _key0->interpolate = _interpolate;
    _interpolate->key = _key1;
}
//////////////////////////////////////////////////////////////////////////
void dz_timeline_key_set_p( dz_timeline_key_t * _key, float _p )
{
    _key->p = _p;
}
//////////////////////////////////////////////////////////////////////////
float dz_timeline_key_get_p( const dz_timeline_key_t * _key )
{
    return _key->p;
}
//////////////////////////////////////////////////////////////////////////
void dz_timeline_key_set_const_value( dz_timeline_key_t * _key, float _value )
{
    _key->const_value = _value;
}
//////////////////////////////////////////////////////////////////////////
void dz_timeline_key_get_const_value( const dz_timeline_key_t * _key, float * _value )
{
    *_value = _key->const_value;
}
//////////////////////////////////////////////////////////////////////////
void dz_timeline_key_set_randomize_min_max( dz_timeline_key_t * _key, float _min, float _max )
{
    _key->randomize_min_value = _min;
    _key->randomize_max_value = _max;
}
//////////////////////////////////////////////////////////////////////////
void dz_timeline_key_get_randomize_min_max( const dz_timeline_key_t * _key, float * _min, float * _max )
{
    *_min = _key->randomize_min_value;
    *_max = _key->randomize_max_value;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_affector_create( dz_service_t * _service, dz_affector_t ** _affector, dz_userdata_t _ud )
{
    dz_affector_t * affector = DZ_NEW( _service, dz_affector_t );

    for( uint32_t index = 0; index != __DZ_AFFECTOR_TIMELINE_MAX__; ++index )
    {
        affector->timelines[index] = DZ_NULLPTR;
    }

    affector->ud = _ud;

    *_affector = affector;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_affector_destroy( dz_service_t * _service, const dz_affector_t * _affector )
{
    for( uint32_t index = 0; index != __DZ_AFFECTOR_TIMELINE_MAX__; ++index )
    {
        const dz_timeline_key_t * timeline = _affector->timelines[index];
        dz_timeline_key_destroy( _service, timeline );
    }

    DZ_FREE( _service, _affector );
}
//////////////////////////////////////////////////////////////////////////
dz_userdata_t dz_affector_get_ud( const dz_affector_t * _affector )
{
    return _affector->ud;
}
//////////////////////////////////////////////////////////////////////////
void dz_affector_set_timeline( dz_affector_t * _affector, dz_affector_timeline_type_e _type, const dz_timeline_key_t * _timeline )
{
    _affector->timelines[_type] = _timeline;
}
//////////////////////////////////////////////////////////////////////////
const dz_timeline_key_t * dz_affector_get_timeline( const dz_affector_t * _affector, dz_affector_timeline_type_e _type )
{
    const dz_timeline_key_t * timeline = _affector->timelines[_type];

    return timeline;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_shape_create( dz_service_t * _service, dz_shape_t ** _shape, dz_shape_type_e _type, dz_userdata_t _ud )
{
    dz_shape_t * shape = DZ_NEW( _service, dz_shape_t );

    shape->type = _type;

    for( uint32_t index = 0; index != __DZ_SHAPE_TIMELINE_MAX__; ++index )
    {
        shape->timelines[index] = DZ_NULLPTR;
    }

    shape->triangles = DZ_NULLPTR;
    shape->triangle_count = 0;

    shape->mask_buffer = DZ_NULLPTR;
    shape->mask_bites = 0;
    shape->mask_pitch = 0;
    shape->mask_width = 0;
    shape->mask_height = 0;
    shape->mask_threshold = 0;
    shape->mask_scale = 1.f;


    shape->ud = _ud;

    *_shape = shape;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_shape_destroy( dz_service_t * _service, const dz_shape_t * _shape )
{
    for( uint32_t index = 0; index != __DZ_SHAPE_TIMELINE_MAX__; ++index )
    {
        const dz_timeline_key_t * timeline = _shape->timelines[index];

        if( timeline == DZ_NULLPTR )
        {
            continue;
        }

        dz_timeline_key_destroy( _service, timeline );
    }

    DZ_FREE( _service, _shape );
}
//////////////////////////////////////////////////////////////////////////
void dz_shape_set_ud( dz_shape_t * _shape, dz_userdata_t _ud )
{
    _shape->ud = _ud;
}
//////////////////////////////////////////////////////////////////////////
dz_userdata_t dz_shape_get_ud( const dz_shape_t * _shape )
{
    return _shape->ud;
}
//////////////////////////////////////////////////////////////////////////
void dz_shape_set_type( dz_shape_t * _shape, dz_shape_type_e _type )
{
    _shape->type = _type;
}
//////////////////////////////////////////////////////////////////////////
dz_shape_type_e dz_shape_get_type( const dz_shape_t * _shape )
{
    return _shape->type;
}
//////////////////////////////////////////////////////////////////////////
void dz_shape_set_timeline( dz_shape_t * _shape, dz_shape_timeline_type_e _type, const dz_timeline_key_t * _timeline )
{
    _shape->timelines[_type] = _timeline;
}
//////////////////////////////////////////////////////////////////////////
const dz_timeline_key_t * dz_shape_get_timeline( const dz_shape_t * _shape, dz_shape_timeline_type_e _type )
{
    const dz_timeline_key_t * timeline = _shape->timelines[_type];

    return timeline;
}
//////////////////////////////////////////////////////////////////////////
static const dz_timeline_limits_t shape_timeline_limits[__DZ_SHAPE_TIMELINE_MAX__] = {
    {DZ_TIMELINE_LIMIT_NORMAL, -DZ_PI2, DZ_PI2, -DZ_PI * 0.25f, DZ_PI2}, //DZ_SHAPE_SEGMENT_ANGLE_MIN
    {DZ_TIMELINE_LIMIT_NORMAL, -DZ_PI2, DZ_PI2, DZ_PI * 0.25f, DZ_PI2}, //DZ_SHAPE_SEGMENT_ANGLE_MAX
    {DZ_TIMELINE_LIMIT_MAX, 0.f, DZ_FLT_MAX, 0.f, 100.f}, //DZ_SHAPE_CIRCLE_RADIUS_MIN
    {DZ_TIMELINE_LIMIT_MAX, 0.f, DZ_FLT_MAX, 1.f, 100.f}, //DZ_SHAPE_CIRCLE_RADIUS_MAX
    {DZ_TIMELINE_LIMIT_NORMAL, -DZ_PI2, DZ_PI2, -DZ_PI * 0.05f, DZ_PI2}, //DZ_SHAPE_CIRCLE_ANGLE_MIN
    {DZ_TIMELINE_LIMIT_NORMAL, -DZ_PI2, DZ_PI2, DZ_PI * 0.05f, DZ_PI2}, //DZ_SHAPE_CIRCLE_ANGLE_MAX
    {DZ_TIMELINE_LIMIT_NORMAL, -DZ_PI2, DZ_PI2, 0.f, DZ_PI2}, //DZ_SHAPE_LINE_ANGLE
    {DZ_TIMELINE_LIMIT_MAX, 0.f, DZ_FLT_MAX, 1.f, 100.f}, //DZ_SHAPE_LINE_SIZE
    {DZ_TIMELINE_LIMIT_MAX, 0.f, DZ_FLT_MAX, 0.f, 100.f}, //DZ_SHAPE_RECT_WIDTH_MIN
    {DZ_TIMELINE_LIMIT_MAX, 0.f, DZ_FLT_MAX, 1.f, 100.f}, //DZ_SHAPE_RECT_WIDTH_MAX
    {DZ_TIMELINE_LIMIT_MAX, 0.f, DZ_FLT_MAX, 0.f, 100.f}, //DZ_SHAPE_RECT_HEIGHT_MIN
    {DZ_TIMELINE_LIMIT_MAX, 0.f, DZ_FLT_MAX, 1.f, 100.f}, //DZ_SHAPE_RECT_HEIGHT_MAX
};
//////////////////////////////////////////////////////////////////////////
void dz_shape_timeline_get_limit( dz_shape_timeline_type_e _timeline, dz_timeline_limit_status_e * _status, float * _min, float * _max, float * _default, float * _factor )
{
    const dz_timeline_limits_t * limit = shape_timeline_limits + _timeline;

    *_status = limit->status;
    *_min = limit->min_value;
    *_max = limit->max_value;
    *_default = limit->default_value;
    *_factor = limit->factor_value;
}
//////////////////////////////////////////////////////////////////////////
static float __get_shape_timeline_default( dz_shape_timeline_type_e _timeline )
{
    const dz_timeline_limits_t * limit = shape_timeline_limits + _timeline;

    float default_value = limit->default_value;

    return default_value;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_shape_set_polygon( dz_shape_t * _shape, const float * _triangles, uint32_t _count )
{
    _shape->triangles = _triangles;
    _shape->triangle_count = _count;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_shape_get_polygon( const dz_shape_t * _shape, const float ** _triangles, uint32_t * _count )
{
    *_triangles = _shape->triangles;
    *_count = _shape->triangle_count;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_shape_set_mask( dz_shape_t * _shape, const void * _buffer, uint32_t _bites, uint32_t _pitch, uint32_t _width, uint32_t _height )
{
#ifdef DZ_DEBUG
    if( _bites != 1 && _bites != 2 && _bites != 4 )
    {
        return DZ_FAILURE;
    }
#endif

    _shape->mask_buffer = _buffer;
    _shape->mask_bites = _bites;
    _shape->mask_pitch = _pitch;
    _shape->mask_width = _width;
    _shape->mask_height = _height;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_shape_get_mask( const dz_shape_t * _shape, const void ** _buffer, uint32_t * _bites, uint32_t * _pitch, uint32_t * _width, uint32_t * _height )
{
    *_buffer = _shape->mask_buffer;
    *_bites = _shape->mask_bites;
    *_pitch = _shape->mask_pitch;
    *_width = _shape->mask_width;
    *_height = _shape->mask_height;
}
//////////////////////////////////////////////////////////////////////////
void dz_shape_set_mask_scale( dz_shape_t * _shape, float _scale )
{
    _shape->mask_scale = _scale;
}
//////////////////////////////////////////////////////////////////////////
float dz_shape_get_mask_scale( const dz_shape_t * _shape )
{
    return _shape->mask_scale;
}
//////////////////////////////////////////////////////////////////////////
void dz_shape_set_mask_threshold( dz_shape_t * _shape, uint32_t _threshold )
{
    _shape->mask_threshold = _threshold;
}
//////////////////////////////////////////////////////////////////////////
uint32_t dz_shape_get_mask_threshold( const dz_shape_t * _shape )
{
    return _shape->mask_threshold;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_emitter_create( dz_service_t * _service, dz_emitter_t ** _emitter, dz_userdata_t _ud )
{
    dz_emitter_t * emitter = DZ_NEW( _service, dz_emitter_t );

    emitter->life = 0.f;

    for( uint32_t index = 0; index != __DZ_EMITTER_TIMELINE_MAX__; ++index )
    {
        emitter->timelines[index] = DZ_NULLPTR;
    }

    emitter->ud = _ud;

    *_emitter = emitter;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_emitter_destroy( dz_service_t * _service, const dz_emitter_t * _emitter )
{
    for( uint32_t index = 0; index != __DZ_EMITTER_TIMELINE_MAX__; ++index )
    {
        const dz_timeline_key_t * timeline = _emitter->timelines[index];

        if( timeline == DZ_NULLPTR )
        {
            continue;
        }

        dz_timeline_key_destroy( _service, timeline );
    }

    DZ_FREE( _service, _emitter );
}
//////////////////////////////////////////////////////////////////////////
void dz_emitter_set_ud( dz_emitter_t * _emitter, dz_userdata_t _ud )
{
    _emitter->ud = _ud;
}
//////////////////////////////////////////////////////////////////////////
dz_userdata_t dz_emitter_get_ud( const dz_emitter_t * _emitter )
{
    return _emitter->ud;
}
//////////////////////////////////////////////////////////////////////////
void dz_emitter_set_life( dz_emitter_t * _emitter, float _life )
{
    _emitter->life = _life;
}
//////////////////////////////////////////////////////////////////////////
float dz_emitter_get_life( const dz_emitter_t * _emitter )
{
    return _emitter->life;
}
//////////////////////////////////////////////////////////////////////////
void dz_emitter_set_timeline( dz_emitter_t * _emitter, dz_emitter_timeline_type_e _type, const dz_timeline_key_t * _timeline )
{
    _emitter->timelines[_type] = _timeline;
}
//////////////////////////////////////////////////////////////////////////
const dz_timeline_key_t * dz_emitter_get_timeline( const dz_emitter_t * _emitter, dz_emitter_timeline_type_e _type )
{
    const dz_timeline_key_t * timeline = _emitter->timelines[_type];

    return timeline;
}
//////////////////////////////////////////////////////////////////////////
static const dz_timeline_limits_t emitter_timeline_limits[__DZ_EMITTER_TIMELINE_MAX__] = {
    {DZ_TIMELINE_LIMIT_MAX, 0.0009765625f, DZ_FLT_MAX, 0.1f, 1.f}, //DZ_EMITTER_SPAWN_DELAY
    {DZ_TIMELINE_LIMIT_MAX, 0.f, DZ_FLT_MAX, 5.f, 10.f}, //DZ_EMITTER_SPAWN_COUNT
    {DZ_TIMELINE_LIMIT_MAX, 0.f, DZ_FLT_MAX, 0.f, 10.f}, //DZ_EMITTER_SPAWN_SPIN_MIN
    {DZ_TIMELINE_LIMIT_MAX, 0.f, DZ_FLT_MAX, 1.f, 10.f}, //DZ_EMITTER_SPAWN_SPIN_MAX
};
//////////////////////////////////////////////////////////////////////////
void dz_emitter_timeline_get_limit( dz_emitter_timeline_type_e _timeline, dz_timeline_limit_status_e * _status, float * _min, float * _max, float * _default, float * _factor )
{
    const dz_timeline_limits_t * limit = emitter_timeline_limits + _timeline;

    *_status = limit->status;
    *_min = limit->min_value;
    *_max = limit->max_value;
    *_default = limit->default_value;
    *_factor = limit->factor_value;
}
//////////////////////////////////////////////////////////////////////////
static float __get_emitter_timeline_default( dz_emitter_timeline_type_e _timeline )
{
    const dz_timeline_limits_t * limit = emitter_timeline_limits + _timeline;

    float default_value = limit->default_value;

    return default_value;
}
//////////////////////////////////////////////////////////////////////////
static uint16_t __get_rand( uint32_t * _seed )
{
    uint32_t value = (*_seed * 1103515245U) + 12345U;

    *_seed = value;

    return value & 0xffff;
}
//////////////////////////////////////////////////////////////////////////
static float __get_randf( uint32_t * _seed )
{
    uint16_t value = __get_rand( _seed );

    const float inv_65535 = 1.f / 65535.f;

    float valuef = (float)value * inv_65535;

    return valuef;
}
//////////////////////////////////////////////////////////////////////////
static float __get_randf2( uint32_t * _seed, float _min, float _max )
{
    uint16_t value = __get_rand( _seed );

    const float inv_65535 = 1.f / 65535.f;

    float valuef = (float)value * inv_65535;

    return _min + (_max - _min) * valuef;
}
//////////////////////////////////////////////////////////////////////////
static float __get_timeline_key_value( float _t, const dz_timeline_key_t * _key )
{
    switch( _key->type )
    {
    case DZ_TIMELINE_KEY_CONST:
        {
            float value = _key->const_value;

            return value;
        }break;
    case DZ_TIMELINE_KEY_RANDOMIZE:
        {
            float value = _key->randomize_min_value + (_key->randomize_max_value - _key->randomize_min_value) * _t;

            return value;
        }break;
    case __DZ_TIMELINE_KEY_MAX__:
    default:
        break;
    };

    return 0.f;
}
//////////////////////////////////////////////////////////////////////////
static float __get_timeline_value( float _t, const dz_timeline_key_t * _key, float _life, float _time )
{
    for( ; _key->interpolate != DZ_NULLPTR && _key->interpolate->key->p * _life < _time; _key = _key->interpolate->key );

    float current_value = __get_timeline_key_value( _t, _key );

    if( _key->interpolate == DZ_NULLPTR ||
        _key->p * _life > _time )
    {
        return current_value;
    }

    const dz_timeline_key_t * next = _key->interpolate->key;

    float next_value = __get_timeline_key_value( _t, next );

    float t = (_time - _key->p * _life) / (next->p * _life - _key->p * _life);

    float value = current_value + (next_value - current_value) * t;

    return value;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_effect_create( dz_service_t * _service, dz_effect_t ** _effect, const dz_material_t * _material, const dz_shape_t * _shape, const dz_emitter_t * _emitter, const dz_affector_t * _affector, uint32_t _seed, float _life, dz_userdata_t _ud )
{
#ifdef DZ_DEBUG
    if( _service == DZ_NULLPTR )
    {
        return DZ_FAILURE;
    }

    if( _material == DZ_NULLPTR )
    {
        return DZ_FAILURE;
    }

    if( _shape == DZ_NULLPTR )
    {
        return DZ_FAILURE;
    }

    if( _emitter == DZ_NULLPTR )
    {
        return DZ_FAILURE;
    }

    if( _affector == DZ_NULLPTR )
    {
        return DZ_FAILURE;
    }

    if( _life < 0.f )
    {
        return DZ_FAILURE;
    }
#endif

    dz_effect_t * effect = DZ_NEW( _service, dz_effect_t );

    effect->material = _material;
    effect->shape = _shape;
    effect->emitter = _emitter;
    effect->affector = _affector;

    effect->init_seed = _seed;
    effect->seed = _seed;

    effect->partices = DZ_NULLPTR;
    effect->partices_count = 0;
    effect->partices_capacity = 0;
    effect->particle_limit = ~0U;

    effect->loop = DZ_FALSE;
    effect->emit_pause = DZ_FALSE;

    effect->life = _life;

    effect->time = 0.f;
    effect->emitter_time = 0.f;

    effect->x = 0.f;
    effect->y = 0.f;

    effect->angle = 0.f;

    effect->ud = _ud;

    *_effect = effect;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_effect_destroy( dz_service_t * _service, const dz_effect_t * _effect )
{
    DZ_FREE( _service, _effect );
}
//////////////////////////////////////////////////////////////////////////
void dz_effect_set_ud( dz_effect_t * _effect, dz_userdata_t _ud )
{
    _effect->ud = _ud;
}
//////////////////////////////////////////////////////////////////////////
dz_userdata_t dz_effect_get_ud( const dz_effect_t * _effect )
{
    return _effect->ud;
}
//////////////////////////////////////////////////////////////////////////
void dz_effect_set_material( dz_effect_t * _effect, const dz_material_t * _material )
{
    _effect->material = _material;
}
//////////////////////////////////////////////////////////////////////////
const dz_material_t * dz_effect_get_material( const dz_effect_t * _effect )
{
    return _effect->material;
}
//////////////////////////////////////////////////////////////////////////
void dz_effect_set_shape( dz_effect_t * _effect, const dz_shape_t * _shape )
{
    _effect->shape = _shape;
}
//////////////////////////////////////////////////////////////////////////
const dz_shape_t * dz_effect_get_shape( const dz_effect_t * _effect )
{
    return _effect->shape;
}
//////////////////////////////////////////////////////////////////////////
void dz_effect_set_emitter( dz_effect_t * _effect, const dz_emitter_t * _emitter )
{
    _effect->emitter = _emitter;
}
//////////////////////////////////////////////////////////////////////////
const dz_emitter_t * dz_effect_get_emitter( const dz_effect_t * _effect )
{
    return _effect->emitter;
}
//////////////////////////////////////////////////////////////////////////
void dz_effect_set_affector( dz_effect_t * _effect, const dz_affector_t * _affector )
{
    _effect->affector = _affector;
}
//////////////////////////////////////////////////////////////////////////
const dz_affector_t * dz_effect_get_affector( const dz_effect_t * _effect )
{
    return _effect->affector;
}
//////////////////////////////////////////////////////////////////////////
void dz_effect_set_seed( dz_effect_t * _effect, uint32_t _seed )
{
    _effect->init_seed = _seed;
}
//////////////////////////////////////////////////////////////////////////
uint32_t dz_effect_get_seed( const dz_effect_t * _effect )
{
    return _effect->init_seed;
}
//////////////////////////////////////////////////////////////////////////
void dz_effect_set_particle_limit( dz_effect_t * _effect, uint32_t _limit )
{
    _effect->particle_limit = _limit;
}
//////////////////////////////////////////////////////////////////////////
uint32_t dz_effect_get_particle_limit( const dz_effect_t * _effect )
{
    return _effect->particle_limit;
}
//////////////////////////////////////////////////////////////////////////
void dz_effect_set_life( dz_effect_t * _effect, float _life )
{
    _effect->life = _life;
}
//////////////////////////////////////////////////////////////////////////
float dz_effect_get_life( const dz_effect_t * _effect )
{
    return _effect->life;
}
//////////////////////////////////////////////////////////////////////////
void dz_effect_set_loop( dz_effect_t * _effect, dz_bool_t _loop )
{
    _effect->loop = _loop;
}
//////////////////////////////////////////////////////////////////////////
dz_bool_t dz_effect_get_loop( const dz_effect_t * _effect )
{
    return _effect->loop;
}
//////////////////////////////////////////////////////////////////////////
void dz_effect_set_time( dz_effect_t * _effect, float _time )
{
    _effect->time = _time;
    _effect->emitter_time = _time;
}
//////////////////////////////////////////////////////////////////////////
float dz_effect_get_time( const dz_effect_t * _effect )
{
    return _effect->time;
}
//////////////////////////////////////////////////////////////////////////
static float __get_affector_value_rands( dz_particle_t * _p, const dz_effect_t * _effect, dz_affector_timeline_type_e _type )
{
    const dz_timeline_key_t * timeline_key = _effect->affector->timelines[_type];

    if( timeline_key == DZ_NULLPTR )
    {
        float default_value = __get_affector_timeline_default( _type );

        return default_value;
    }

    float time = _p->time;

    float value = __get_timeline_value( _p->rands[_type], timeline_key, _p->life, time );

    return value;
}
//////////////////////////////////////////////////////////////////////////
static void __particle_update( dz_service_t * _service, const dz_effect_t * _emitter, dz_particle_t * _p, float _time )
{
    _p->time += _time;

    const float move_speed = __get_affector_value_rands( _p, _emitter, DZ_AFFECTOR_TIMELINE_MOVE_SPEED );
    const float move_accelerate = __get_affector_value_rands( _p, _emitter, DZ_AFFECTOR_TIMELINE_MOVE_ACCELERATE );

    const float rotate_speed = __get_affector_value_rands( _p, _emitter, DZ_AFFECTOR_TIMELINE_ROTATE_SPEED );
    const float rotate_accelerate = __get_affector_value_rands( _p, _emitter, DZ_AFFECTOR_TIMELINE_ROTATE_ACCELERATE );

    const float spin_speed = __get_affector_value_rands( _p, _emitter, DZ_AFFECTOR_TIMELINE_SPIN_SPEED );
    const float spin_accelerate = __get_affector_value_rands( _p, _emitter, DZ_AFFECTOR_TIMELINE_SPIN_ACCELERATE );

    _p->size = __get_affector_value_rands( _p, _emitter, DZ_AFFECTOR_TIMELINE_SIZE );

    _p->color_r = __get_affector_value_rands( _p, _emitter, DZ_AFFECTOR_TIMELINE_COLOR_R );
    _p->color_g = __get_affector_value_rands( _p, _emitter, DZ_AFFECTOR_TIMELINE_COLOR_G );
    _p->color_b = __get_affector_value_rands( _p, _emitter, DZ_AFFECTOR_TIMELINE_COLOR_B );
    _p->color_a = __get_affector_value_rands( _p, _emitter, DZ_AFFECTOR_TIMELINE_COLOR_A );

    _p->rotate_accelerate_aux += rotate_accelerate * _time * _time;
    _p->angle += rotate_speed * _time + _p->rotate_accelerate_aux;

    _p->spin_accelerate_aux += spin_accelerate * _time * _time;
    _p->spin += spin_speed * _time + _p->spin_accelerate_aux;

    const float dx = DZ_COSF( _service, _p->angle );
    const float dy = DZ_SINF( _service, _p->angle );

    const float strafe_size = __get_affector_value_rands( _p, _emitter, DZ_AFFECTOR_TIMELINE_STRAFE_SIZE );

    if( strafe_size != 0.f )
    {
        const float strafe_speed = __get_affector_value_rands( _p, _emitter, DZ_AFFECTOR_TIMELINE_STRAFE_SPEED );
        const float strafe_frenquence = __get_affector_value_rands( _p, _emitter, DZ_AFFECTOR_TIMELINE_STRAFE_FRENQUENCE );

        const float strafe_shift = _p->rands[DZ_AFFECTOR_TIMELINE_STRAFE_SHIFT];

        const float strafex = -dy * DZ_COSF( _service, strafe_shift * DZ_PI + strafe_frenquence * _p->time ) * strafe_size * strafe_speed * _time;
        const float strafey = dx * DZ_SINF( _service, strafe_shift * DZ_PI + strafe_frenquence * _p->time ) * strafe_size * strafe_speed * _time;

        _p->x += strafex;
        _p->y += strafey;
    }

    _p->move_accelerate_aux += move_accelerate * _time * _time;

    const float movex = dx * (move_speed * _time + _p->move_accelerate_aux);
    const float movey = dy * (move_speed * _time + _p->move_accelerate_aux);

    _p->x += movex;
    _p->y += movey;

    const float sx = DZ_COSF( _service, _p->angle + _p->spin );
    const float sy = DZ_SINF( _service, _p->angle + _p->spin );

    _p->sx = sx;
    _p->sy = sy;
}
//////////////////////////////////////////////////////////////////////////
static float __get_timeline_value_seed( uint32_t * _seed, const dz_timeline_key_t * _timeline, float _life, float _time )
{
    const float t = __get_randf( _seed );

    const float value = __get_timeline_value( t, _timeline, _life, _time );

    return value;
}
//////////////////////////////////////////////////////////////////////////
static float __get_shape_value_seed( dz_effect_t * _effect, dz_shape_timeline_type_e _type, float _life, float _time )
{
    const dz_timeline_key_t * timeline_key = _effect->shape->timelines[_type];

    if( timeline_key == DZ_NULLPTR )
    {
        const float default_value = __get_shape_timeline_default( _type );

        return default_value;
    }

    const float value = __get_timeline_value_seed( &_effect->seed, timeline_key, _life, _time );

    return value;
}
//////////////////////////////////////////////////////////////////////////
static float __get_emitter_value_seed( dz_effect_t * _effect, dz_emitter_timeline_type_e _type, float _life, float _time )
{
    const dz_timeline_key_t * timeline_key = _effect->emitter->timelines[_type];

    if( timeline_key == DZ_NULLPTR )
    {
        const float default_value = __get_emitter_timeline_default( _type );

        return default_value;
    }

    const float value = __get_timeline_value_seed( &_effect->seed, timeline_key, _life, _time );

    return value;
}
//////////////////////////////////////////////////////////////////////////
static float __get_affector_value_seed( dz_effect_t * _effect, dz_affector_timeline_type_e _type, float _life, float _time )
{
    const dz_timeline_key_t * timeline_key = _effect->affector->timelines[_type];

    if( timeline_key == DZ_NULLPTR )
    {
        const float default_value = __get_affector_timeline_default( _type );

        return default_value;
    }

    const float value = __get_timeline_value_seed( &_effect->seed, timeline_key, _life, _time );

    return value;
}
//////////////////////////////////////////////////////////////////////////
static float __calc_triangle_area( float ax, float ay, float bx, float by, float cx, float cy )
{
    const float area = (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by)) * 0.5f;

    if( area < 0.f )
    {
        return -area;
    }

    return area;
}
//////////////////////////////////////////////////////////////////////////
static uint32_t __calc_mask_threshold_value_count( const void * _buffer, uint32_t _pitch, uint32_t _bites, uint32_t _width, uint32_t _height, uint32_t _threshold )
{
    uint32_t threshold_value_count = 0;

    const void * buffer_iterator = _buffer;

    for( uint32_t h = 0; h != _height; ++h )
    {
        for( uint32_t w = 0; w != _width; ++w )
        {
            uint32_t value = 0;

            switch( _bites )
            {
            case 1:
                {
                    const uint8_t * point = (const uint8_t *)buffer_iterator + w;

                    value = (uint32_t)*point;
                }break;
            case 2:
                {
                    const uint16_t * point = (const uint16_t *)buffer_iterator + w;

                    value = (uint32_t)*point;
                }break;
            case 4:
                {
                    const uint32_t * point = (const uint32_t *)buffer_iterator + w;

                    value = (uint32_t)*point;
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

        buffer_iterator = (const uint8_t *)buffer_iterator + _pitch;
    }

    return threshold_value_count;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __get_mask_threshold_value( const void * _buffer, uint32_t _pitch, uint32_t _bites, uint32_t _width, uint32_t _height, uint32_t _threshold, uint32_t _index, uint32_t * _x, uint32_t * _y )
{
    const void * buffer_iterator = _buffer;

    for( uint32_t h = 0; h != _height; ++h )
    {
        for( uint32_t w = 0; w != _width; ++w )
        {
            uint32_t mask_value = 0;

            switch( _bites )
            {
            case 1:
                {
                    const uint8_t * mask_point = (const uint8_t *)buffer_iterator + w;

                    mask_value = (uint32_t)*mask_point;
                }break;
            case 2:
                {
                    const uint16_t * mask_point = (const uint16_t *)buffer_iterator + w;

                    mask_value = (uint32_t)*mask_point;
                }break;
            case 4:
                {
                    const uint32_t * mask_point = (const uint32_t *)buffer_iterator + w;

                    mask_value = (uint32_t)*mask_point;
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

        buffer_iterator = (const uint8_t *)buffer_iterator + _pitch;
    }

    return DZ_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __emitter_spawn_particle( dz_service_t * _service, dz_effect_t * _effect, float _life, float _spawn_time )
{
    const float time = _effect->time - _spawn_time;

    if( _effect->partices_count >= _effect->partices_capacity )
    {
        if( _effect->partices_capacity == 0 )
        {
            _effect->partices_capacity = 16;
        }
        else
        {
            _effect->partices_capacity *= 2;
        }

        _effect->partices = DZ_REALLOCN( _service, _effect->partices, dz_particle_t, _effect->partices_capacity );

        if( _effect->partices == DZ_NULLPTR )
        {
            return DZ_FAILURE;
        }
    }

    dz_particle_t * p = _effect->partices + _effect->partices_count++;

    for( uint32_t index = 0; index != __DZ_AFFECTOR_TIMELINE_MAX__; ++index )
    {
        p->rands[index] = __get_randf( &_effect->seed );
    }

    p->life = _life;
    p->time = 0.f;

    p->move_accelerate_aux = 0.f;
    p->rotate_accelerate_aux = 0.f;
    p->spin_accelerate_aux = 0.f;

    p->x = _effect->x;
    p->y = _effect->y;

    p->angle = _effect->angle;

    dz_shape_type_e shape_type = _effect->shape->type;

    switch( shape_type )
    {
    case DZ_SHAPE_POINT:
        {
            p->x += 0.f;
            p->y += 0.f;

            const float angle = __get_randf( &_effect->seed ) * DZ_PI2;

            p->angle += angle;
        }break;
    case DZ_SHAPE_SEGMENT:
        {
            p->x += 0.f;
            p->y += 0.f;

            const float angle_min = __get_shape_value_seed( _effect, DZ_SHAPE_SEGMENT_ANGLE_MIN, _life, _spawn_time );
            const float angle_max = __get_shape_value_seed( _effect, DZ_SHAPE_SEGMENT_ANGLE_MAX, _life, _spawn_time );

            const float angle = __get_randf2( &_effect->seed, angle_min, angle_max );

            p->angle += angle;
        }break;
    case DZ_SHAPE_CIRCLE:
        {
            const float radius_min = __get_shape_value_seed( _effect, DZ_SHAPE_CIRCLE_RADIUS_MIN, _life, _spawn_time );
            const float radius_max = __get_shape_value_seed( _effect, DZ_SHAPE_CIRCLE_RADIUS_MAX, _life, _spawn_time );

            const float angle = __get_randf( &_effect->seed ) * DZ_PI2;
            const float radius = radius_min + DZ_SQRTF( _service, __get_randf( &_effect->seed ) ) * (radius_max - radius_min);

            const float rx = radius * DZ_COSF( _service, angle );
            const float ry = radius * DZ_SINF( _service, angle );

            p->x += rx;
            p->y += ry;

            const float angle_min = __get_shape_value_seed( _effect, DZ_SHAPE_CIRCLE_ANGLE_MIN, _life, _spawn_time );
            const float angle_max = __get_shape_value_seed( _effect, DZ_SHAPE_CIRCLE_ANGLE_MAX, _life, _spawn_time );

            const float angle_dispersion = __get_randf2( &_effect->seed, angle_min, angle_max );

            p->angle += angle + angle_dispersion;
        }break;
    case DZ_SHAPE_LINE:
        {
            const float angle = __get_shape_value_seed( _effect, DZ_SHAPE_LINE_ANGLE, _life, _spawn_time );

            const float dx = DZ_COSF( _service, angle );
            const float dy = DZ_SINF( _service, angle );

            const float size = __get_shape_value_seed( _effect, DZ_SHAPE_LINE_SIZE, _life, _spawn_time );

            const float l = (__get_randf( &_effect->seed ) - 0.5f) * size;

            p->x += -dy * l;
            p->y += dx * l;

            p->angle += angle;
        }break;
    case DZ_SHAPE_RECT:
        {
            const float width_min = __get_shape_value_seed( _effect, DZ_SHAPE_RECT_WIDTH_MIN, _life, _spawn_time );
            const float width_max = __get_shape_value_seed( _effect, DZ_SHAPE_RECT_WIDTH_MAX, _life, _spawn_time );
            const float height_min = __get_shape_value_seed( _effect, DZ_SHAPE_RECT_HEIGHT_MIN, _life, _spawn_time );
            const float height_max = __get_shape_value_seed( _effect, DZ_SHAPE_RECT_HEIGHT_MAX, _life, _spawn_time );

            DZ_TODO DZ_UNUSED( width_min );
            DZ_TODO DZ_UNUSED( height_min );

            const float x = (__get_randf( &_effect->seed ) - 0.5f) * width_max;
            const float y = (__get_randf( &_effect->seed ) - 0.5f) * height_max;

            p->x += x;
            p->y += y;

            float angle = __get_randf( &_effect->seed ) * DZ_PI2;

            p->angle += angle;
        }break;
    case DZ_SHAPE_POLYGON:
        {
            float total_area = 0.f;
            float areas[1024];

            const float * triangles = _effect->shape->triangles;
            const uint32_t triangle_count = _effect->shape->triangle_count;

            for( uint32_t index = 0; index != triangle_count; ++index )
            {
                const float ax = triangles[index * 6 + 0];
                const float ay = triangles[index * 6 + 1];
                const float bx = triangles[index * 6 + 2];
                const float by = triangles[index * 6 + 3];
                const float cx = triangles[index * 6 + 4];
                const float cy = triangles[index * 6 + 5];

                const float area = __calc_triangle_area( ax, ay, bx, by, cx, cy );

                total_area += area;

                areas[index] = total_area;
            }

            const float triangle_rand = __get_randf( &_effect->seed );

            const float triangle_find_area = triangle_rand * total_area;

            uint32_t triangle_found_index = 0;

            for( uint32_t index = 0; index != triangle_count; ++index )
            {
                const float area = areas[index];

                if( area < triangle_find_area )
                {
                    continue;
                }

                triangle_found_index = index;

                break;
            }

            const float rax = triangles[triangle_found_index * 6 + 0];
            const float ray = triangles[triangle_found_index * 6 + 1];
            const float rbx = triangles[triangle_found_index * 6 + 2];
            const float rby = triangles[triangle_found_index * 6 + 3];
            const float rcx = triangles[triangle_found_index * 6 + 4];
            const float rcy = triangles[triangle_found_index * 6 + 5];

            const float r1 = __get_randf( &_effect->seed );
            const float r2 = __get_randf( &_effect->seed );

            const float qr1 = DZ_SQRTF( _service, r1 );

            const float tx = (1.f - qr1) * rax + (qr1 * (1.f - r2)) * rbx + (qr1 * r2) * rcx;
            const float ty = (1.f - qr1) * ray + (qr1 * (1.f - r2)) * rby + (qr1 * r2) * rcy;

            p->x += tx;
            p->y += ty;

            const float angle = __get_randf( &_effect->seed ) * DZ_PI2;

            p->angle += angle;
        }break;
    case DZ_SHAPE_MASK:
        {
            const void * mask_buffer = _effect->shape->mask_buffer;
            const uint32_t mask_bites = _effect->shape->mask_bites;
            const uint32_t mask_pitch = _effect->shape->mask_pitch;
            const uint32_t mask_width = _effect->shape->mask_width;
            const uint32_t mask_height = _effect->shape->mask_height;
            const uint32_t mask_threshold = _effect->shape->mask_threshold;
            const float mask_scale = _effect->shape->mask_scale;

            uint32_t threshold_value_count = __calc_mask_threshold_value_count( mask_buffer, mask_pitch, mask_bites, mask_width, mask_height, mask_threshold );

            const float r = __get_randf( &_effect->seed );

            uint32_t threshold_value_index = (uint32_t)(r * (threshold_value_count - 1) + 0.5f);

            uint32_t w_found;
            uint32_t h_found;
            if( __get_mask_threshold_value( mask_buffer, mask_pitch, mask_bites, mask_width, mask_height, mask_threshold, threshold_value_index, &w_found, &h_found ) == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }

            p->x += w_found * mask_scale;
            p->y += h_found * mask_scale;

            const float angle = __get_randf( &_effect->seed ) * DZ_PI2;

            p->angle += angle;
        }break;
    case __DZ_SHAPE_MAX__:
    default:
        break;
    }

    const float spin_min = __get_emitter_value_seed( _effect, DZ_EMITTER_SPAWN_SPIN_MIN, _life, _spawn_time );
    const float spin_max = __get_emitter_value_seed( _effect, DZ_EMITTER_SPAWN_SPIN_MAX, _life, _spawn_time );

    p->spin = (__get_randf( &_effect->seed ) * 2.f - 1.f) * __get_randf2( &_effect->seed, spin_min, spin_max );

    const float sx = DZ_COSF( _service, p->spin );
    const float sy = DZ_SINF( _service, p->spin );

    p->sx = sx;
    p->sy = sy;

    __particle_update( _service, _effect, p, time );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_effect_set_position( dz_effect_t * _effect, float _x, float _y )
{
    _effect->x = _x;
    _effect->y = _y;
}
//////////////////////////////////////////////////////////////////////////
void dz_effect_get_position( const dz_effect_t * _effect, float * _x, float * _y )
{
    *_x = _effect->x;
    *_y = _effect->y;
}
//////////////////////////////////////////////////////////////////////////
void dz_effect_set_rotate( dz_effect_t * _effect, float _angle )
{
    _effect->angle = _angle;
}
//////////////////////////////////////////////////////////////////////////
float dz_effect_get_rotate( const dz_effect_t * _effect )
{
    return _effect->angle;
}
//////////////////////////////////////////////////////////////////////////
void dz_effect_reset( dz_effect_t * _effect )
{
    _effect->seed = _effect->init_seed;

    _effect->time = 0.f;
    _effect->emitter_time = 0.f;

    _effect->partices_count = 0;
}
//////////////////////////////////////////////////////////////////////////
void dz_effect_emit_pause( dz_effect_t * _effect )
{
    _effect->emit_pause = DZ_TRUE;
}
//////////////////////////////////////////////////////////////////////////
void dz_effect_emit_resume( dz_effect_t * _effect )
{
    _effect->emit_pause = DZ_FALSE;
}
//////////////////////////////////////////////////////////////////////////
dz_bool_t dz_effect_is_emit_pause( const dz_effect_t * _effect )
{
    return _effect->emit_pause;
}
//////////////////////////////////////////////////////////////////////////
static dz_particle_t * __find_dead_particle( dz_particle_t * _p, dz_particle_t * _end )
{
    for( ; _p != _end; ++_p )
    {
        if( _p->time < 0.f )
        {
            return _p;
        }
    }

    return _end;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_effect_update( dz_service_t * _service, dz_effect_t * _effect, float _time )
{
    dz_particle_t * p = _effect->partices;
    dz_particle_t * p_end = _effect->partices + _effect->partices_count;
    while( p != p_end )
    {
        if( p->time + _time < p->life )
        {
            __particle_update( _service, _effect, p, _time );
        }
        else
        {
            p->time = -1.f;
        }

        ++p;
    }

    dz_particle_t * p_dead = __find_dead_particle( _effect->partices, p_end );

    if( p_dead != p_end )
    {
        dz_particle_t * p_sweep = p_dead;
        ++p_sweep;
        for( ; p_sweep != p_end; ++p_sweep )
        {
            if( p_sweep->time >= 0.f )
            {
                *p_dead++ = *p_sweep;
            }
            else
            {
                --_effect->partices_count;
            }
        }
    }

    if( _effect->emit_pause == DZ_TRUE )
    {
        return DZ_SUCCESSFUL;
    }

    if( _effect->time + _time > _effect->life )
    {
        if( _effect->loop == DZ_FALSE )
        {
            _effect->time = _effect->life;
        }
        else
        {
            _effect->time += _time - _effect->life;
            _effect->emitter_time -= _effect->life;
        }
    }
    else
    {
        _effect->time += _time;
    }

    for( ;;)
    {
        float delay = __get_emitter_value_seed( _effect, DZ_EMITTER_SPAWN_DELAY, _effect->life, _effect->emitter_time );

        if( _effect->loop == DZ_FALSE && _effect->emitter_time + delay > _effect->life )
        {
            break;
        }

        if( _effect->time - _effect->emitter_time < delay )
        {
            break;
        }

        float spawn_time = _effect->emitter_time + delay;

        float count = __get_emitter_value_seed( _effect, DZ_EMITTER_SPAWN_COUNT, _effect->life, spawn_time );

        while( count > 0.f )
        {
            float life = __get_affector_value_seed( _effect, DZ_AFFECTOR_TIMELINE_LIFE, _effect->life, spawn_time );

            float ptime = _effect->time - spawn_time;

            if( life > ptime && _effect->partices_count < _effect->particle_limit )
            {
                if( __emitter_spawn_particle( _service, _effect, life, spawn_time ) == DZ_FAILURE )
                {
                    return DZ_FAILURE;
                }
            }

            count -= 1.f;
        }

        _effect->emitter_time += delay;
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_effect_state_e dz_effect_get_state( const dz_effect_t * _effect )
{
    dz_effect_state_e state = DZ_EFFECT_PROCESS;

    if( _effect->loop == DZ_TRUE )
    {
        return state;
    }

    if( _effect->time < _effect->life )
    {
        return state;
    }

    state |= DZ_EFFECT_EMIT_COMPLETE;

    if( _effect->partices_count == 0U )
    {
        state |= DZ_EFFECT_PARTICLE_COMPLETE;
    }

    return state;
}
//////////////////////////////////////////////////////////////////////////
uint32_t dz_effect_get_particle_count( const dz_effect_t * _effect )
{
    return _effect->partices_count;
}
//////////////////////////////////////////////////////////////////////////
static void __particle_compute_positions( const dz_particle_t * _p, uint16_t _iterator, dz_effect_mesh_t * _mesh )
{
    const float hs = _p->size * 0.5f;

    const float hsx = _p->sx * hs;
    const float hsy = _p->sy * hs;

    const float ux = hsx;
    const float uy = hsy;

    const float vx = -hsy;
    const float vy = hsx;

    float * p0 = (float *)((uint8_t *)(_mesh->position_buffer) + _mesh->position_offset + _mesh->position_stride * (_iterator * 4 + 0));

    p0[0] = _p->x - ux + vx;
    p0[1] = _p->y - uy + vy;

    float * p1 = (float *)((uint8_t *)(_mesh->position_buffer) + _mesh->position_offset + _mesh->position_stride * (_iterator * 4 + 1));

    p1[0] = _p->x + ux + vx;
    p1[1] = _p->y + uy + vy;

    float * p2 = (float *)((uint8_t *)(_mesh->position_buffer) + _mesh->position_offset + _mesh->position_stride * (_iterator * 4 + 2));

    p2[0] = _p->x + ux - vx;
    p2[1] = _p->y + uy - vy;

    float * p3 = (float *)((uint8_t *)(_mesh->position_buffer) + _mesh->position_offset + _mesh->position_stride * (_iterator * 4 + 3));

    p3[0] = _p->x - ux - vx;
    p3[1] = _p->y - uy - vy;
}
//////////////////////////////////////////////////////////////////////////
static void __particle_compute_colors( const dz_particle_t * _p, uint16_t _iterator, dz_effect_mesh_t * _mesh )
{
    const uint8_t r8 = (uint8_t)(_mesh->r * _p->color_r * 255.5f);
    const uint8_t g8 = (uint8_t)(_mesh->g * _p->color_g * 255.5f);
    const uint8_t b8 = (uint8_t)(_mesh->b * _p->color_b * 255.5f);
    const uint8_t a8 = (uint8_t)(_mesh->a * _p->color_a * 255.5f);

    const uint32_t color = (a8 << 24) | (r8 << 16) | (g8 << 8) | (b8 << 0);

    uint32_t * c0 = (uint32_t *)((uint8_t *)(_mesh->color_buffer) + _mesh->color_offset + _mesh->color_stride * (_iterator * 4 + 0));

    c0[0] = color;

    uint32_t * c1 = (uint32_t *)((uint8_t *)(_mesh->color_buffer) + _mesh->color_offset + _mesh->color_stride * (_iterator * 4 + 1));

    c1[0] = color;

    uint32_t * c2 = (uint32_t *)((uint8_t *)(_mesh->color_buffer) + _mesh->color_offset + _mesh->color_stride * (_iterator * 4 + 2));

    c2[0] = color;

    uint32_t * c3 = (uint32_t *)((uint8_t *)(_mesh->color_buffer) + _mesh->color_offset + _mesh->color_stride * (_iterator * 4 + 3));

    c3[0] = color;
}
//////////////////////////////////////////////////////////////////////////
static void __particle_compute_uvs( const dz_particle_t * _p, uint16_t _iterator, dz_effect_mesh_t * _mesh )
{
    DZ_UNUSED( _p );

    float * uv0 = (float *)((uint8_t *)(_mesh->uv_buffer) + _mesh->uv_offset + _mesh->uv_stride * (_iterator * 4 + 0));

    uv0[0] = 0.f;
    uv0[1] = 0.f;

    float * uv1 = (float *)((uint8_t *)(_mesh->uv_buffer) + _mesh->uv_offset + _mesh->uv_stride * (_iterator * 4 + 1));

    uv1[0] = 1.f;
    uv1[1] = 0.f;

    float * uv2 = (float *)((uint8_t *)(_mesh->uv_buffer) + _mesh->uv_offset + _mesh->uv_stride * (_iterator * 4 + 2));

    uv2[0] = 1.f;
    uv2[1] = 1.f;

    float * uv3 = (float *)((uint8_t *)(_mesh->uv_buffer) + _mesh->uv_offset + _mesh->uv_stride * (_iterator * 4 + 3));

    uv3[0] = 0.f;
    uv3[1] = 1.f;
}
//////////////////////////////////////////////////////////////////////////
static void __particle_compute_index( uint16_t _iterator, dz_effect_mesh_t * _mesh )
{
    uint16_t * index_buffer = (uint16_t *)(_mesh->index_buffer) + _iterator * 6;
    index_buffer[0] = _iterator * 4 + 0;
    index_buffer[1] = _iterator * 4 + 1;
    index_buffer[2] = _iterator * 4 + 3;
    index_buffer[3] = _iterator * 4 + 3;
    index_buffer[4] = _iterator * 4 + 1;
    index_buffer[5] = _iterator * 4 + 2;
}
//////////////////////////////////////////////////////////////////////////
void dz_effect_compute_mesh( const dz_effect_t * _effect, dz_effect_mesh_t * _mesh, dz_effect_mesh_chunk_t * _chunks, uint32_t _capacity, uint32_t * _count )
{
    DZ_UNUSED( _capacity );

    uint16_t particle_iterator = 0;

    const dz_particle_t * p_begin = _effect->partices + _effect->partices_count;
    const dz_particle_t * p_end = _effect->partices;
    for( ; p_begin != p_end; --p_begin )
    {
        const dz_particle_t * p = p_begin - 1;

        __particle_compute_positions( p, particle_iterator, _mesh );
        __particle_compute_colors( p, particle_iterator, _mesh );
        __particle_compute_uvs( p, particle_iterator, _mesh );
        __particle_compute_index( particle_iterator, _mesh );

        ++particle_iterator;
    }

    if( particle_iterator == 0 )
    {
        *_count = 0;

        return;
    }

    dz_effect_mesh_chunk_t * chunk = _chunks + 0;
    chunk->offset = 0;
    chunk->vertex_size = particle_iterator * 4;
    chunk->index_size = particle_iterator * 6;

    const dz_material_t * material = _effect->material;

    chunk->blend_type = material->blend_type;
    chunk->surface = material->atlas->surface;

    *_count = 1;
}
//////////////////////////////////////////////////////////////////////////
#define DZ_WRITE(W, U, V) if( (*W)(&V, sizeof(V), U) == DZ_FAILURE ) return DZ_FAILURE
#define DZ_WRITEN(W, U, V, N) if( (*W)(V, sizeof(*V) * N, U) == DZ_FAILURE ) return DZ_FAILURE
//////////////////////////////////////////////////////////////////////////
static dz_result_t __write_enum( uint32_t _enum, dz_write_t _write, dz_userdata_t _ud )
{
    DZ_WRITE( _write, _ud, _enum );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
#define DZ_WRITE_ENUM(W, U, V) __write_enum(V, W, U)
//////////////////////////////////////////////////////////////////////////
static dz_result_t __write_bool( dz_bool_t _b, dz_write_t _write, dz_userdata_t _ud )
{
    uint8_t v = (uint8_t)_b;

    DZ_WRITE( _write, _ud, v );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
#define DZ_WRITE_BOOL(W, U, V) __write_bool(V, W, U)
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_header_write( dz_write_t _write, dz_userdata_t _ud )
{
    uint32_t magic = dz_get_magic();

    DZ_WRITE( _write, _ud, magic );

    uint32_t version = dz_get_version();

    DZ_WRITE( _write, _ud, version );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __write_texture( const dz_texture_t * _texture, dz_write_t _write, dz_userdata_t _ud )
{
    DZ_WRITEN( _write, _ud, _texture->u, 4 );
    DZ_WRITEN( _write, _ud, _texture->v, 4 );

    DZ_WRITE( _write, _ud, _texture->trim_offset_x );
    DZ_WRITE( _write, _ud, _texture->trim_offset_y );

    DZ_WRITE( _write, _ud, _texture->trim_width );
    DZ_WRITE( _write, _ud, _texture->trim_height );

    DZ_WRITE( _write, _ud, _texture->random_weight );
    DZ_WRITE( _write, _ud, _texture->sequence_delay );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __write_atlas( const dz_atlas_t * _atlas, dz_write_t _write, dz_userdata_t _ud )
{
    DZ_WRITE( _write, _ud, _atlas->texture_count );

    for( uint32_t index = 0; index != _atlas->texture_count; ++index )
    {
        const dz_texture_t * texture = _atlas->textures[index];

        if( __write_texture( texture, _write, _ud ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __write_material( const dz_material_t * _material, dz_write_t _write, dz_userdata_t _ud )
{
    DZ_WRITE_ENUM( _write, _ud, _material->blend_type );

    DZ_WRITE( _write, _ud, _material->r );
    DZ_WRITE( _write, _ud, _material->g );
    DZ_WRITE( _write, _ud, _material->b );
    DZ_WRITE( _write, _ud, _material->a );

    DZ_WRITE_ENUM( _write, _ud, _material->mode );

    if( _material->atlas == DZ_NULLPTR )
    {
        DZ_WRITE_BOOL( _write, _ud, DZ_FALSE );
    }
    else
    {
        DZ_WRITE_BOOL( _write, _ud, DZ_TRUE );

        if( __write_atlas( _material->atlas, _write, _ud ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __write_timeline_key( const dz_timeline_key_t * _key, dz_write_t _write, dz_userdata_t _ud );
//////////////////////////////////////////////////////////////////////////
static dz_result_t __write_timeline_interpolate( const dz_timeline_interpolate_t * _interpolate, dz_write_t _write, dz_userdata_t _ud )
{
    DZ_WRITE_ENUM( _write, _ud, _interpolate->type );

    DZ_WRITE( _write, _ud, _interpolate->p0 );
    DZ_WRITE( _write, _ud, _interpolate->p1 );

    if( _interpolate->key == DZ_NULLPTR )
    {
        DZ_WRITE_BOOL( _write, _ud, DZ_FALSE );
    }
    else
    {
        DZ_WRITE_BOOL( _write, _ud, DZ_TRUE );

        if( __write_timeline_key( _interpolate->key, _write, _ud ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __write_timeline_key( const dz_timeline_key_t * _key, dz_write_t _write, dz_userdata_t _ud )
{
    DZ_WRITE( _write, _ud, _key->p );

    DZ_WRITE_ENUM( _write, _ud, _key->type );

    DZ_WRITE( _write, _ud, _key->const_value );

    DZ_WRITE( _write, _ud, _key->randomize_min_value );
    DZ_WRITE( _write, _ud, _key->randomize_max_value );

    if( _key->interpolate == DZ_NULLPTR )
    {
        DZ_WRITE_BOOL( _write, _ud, DZ_FALSE );
    }
    else
    {
        DZ_WRITE_BOOL( _write, _ud, DZ_TRUE );

        if( __write_timeline_interpolate( _key->interpolate, _write, _ud ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __write_shape( const dz_shape_t * _shape, dz_write_t _write, dz_userdata_t _ud )
{
    DZ_WRITE_ENUM( _write, _ud, _shape->type );

    for( uint32_t index = 0; index != __DZ_SHAPE_TIMELINE_MAX__; ++index )
    {
        const dz_timeline_key_t * timeline = _shape->timelines[index];

        if( __write_timeline_key( timeline, _write, _ud ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __write_emitter( const dz_emitter_t * _emitter, dz_write_t _write, dz_userdata_t _ud )
{
    DZ_WRITE( _write, _ud, _emitter->life );

    for( uint32_t index = 0; index != __DZ_EMITTER_TIMELINE_MAX__; ++index )
    {
        const dz_timeline_key_t * timeline = _emitter->timelines[index];

        if( __write_timeline_key( timeline, _write, _ud ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __write_affector( const dz_affector_t * _affector, dz_write_t _write, dz_userdata_t _ud )
{
    for( uint32_t index = 0; index != __DZ_AFFECTOR_TIMELINE_MAX__; ++index )
    {
        const dz_timeline_key_t * timeline = _affector->timelines[index];

        if( __write_timeline_key( timeline, _write, _ud ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_effect_write( const dz_effect_t * _effect, dz_write_t _write, dz_userdata_t _ud )
{
    DZ_WRITE( _write, _ud, _effect->init_seed );
    DZ_WRITE( _write, _ud, _effect->particle_limit );

    DZ_WRITE( _write, _ud, _effect->life );

    if( __write_material( _effect->material, _write, _ud ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    if( __write_shape( _effect->shape, _write, _ud ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    if( __write_emitter( _effect->emitter, _write, _ud ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    if( __write_affector( _effect->affector, _write, _ud ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    return DZ_SUCCESSFUL;
}