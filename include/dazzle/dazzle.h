#ifndef DZ_DAZZLE_H_
#define DZ_DAZZLE_H_

#include "dazzle/config.h"

typedef struct dz_service_t dz_service_t;

typedef void * (*dz_malloc_t)(dz_size_t _size, dz_userdata_t _ud);
typedef void * (*dz_realloc_t)(void * _ptr, dz_size_t _size, dz_userdata_t _ud);
typedef void (*dz_free_t)(const void * _ptr, dz_userdata_t _ud);
typedef float (*dz_sqrtf_t)(float _a, dz_userdata_t _ud);
typedef float (*dz_cosf_t)(float _a, dz_userdata_t _ud);
typedef float (*dz_sinf_t)(float _a, dz_userdata_t _ud);

typedef struct dz_service_providers_t
{
    dz_malloc_t f_malloc;
    dz_realloc_t f_realloc;
    dz_free_t f_free;
    dz_sqrtf_t f_sqrtf;
    dz_cosf_t f_cosf;
    dz_sinf_t f_sinf;
} dz_service_providers_t;

dz_result_t dz_service_create( dz_service_t ** _service, const dz_service_providers_t * _providers, dz_userdata_t _ud );
void dz_service_destroy( dz_service_t * _service );

void dz_service_get_providers( dz_service_t * _service, dz_service_providers_t * _providers );

typedef enum dz_blend_type_e
{
    DZ_BLEND_NORNAL,
    DZ_BLEND_ADD,
    DZ_BLEND_MULTIPLY,
    DZ_BLEND_SCREEN,

    __DZ_BLEND_MAX__
} dz_blend_type_e;

typedef struct dz_texture_t dz_texture_t;

dz_result_t dz_texture_create( dz_service_t * _service, dz_texture_t ** _texture, dz_userdata_t _ud );
void dz_texture_destroy( dz_service_t * _service, const dz_texture_t * _texture );

dz_userdata_t dz_texture_get_ud( const dz_texture_t * _texture );

void dz_texture_set_uv( dz_texture_t * _texture, const float * _u, const float * _v );
void dz_texture_get_uv( const dz_texture_t * _texture, float * _u, float * _v );

typedef struct dz_material_t dz_material_t;

dz_result_t dz_material_create( dz_service_t * _service, dz_material_t ** _material, dz_userdata_t _ud );
void dz_material_destroy( dz_service_t * _service, const dz_material_t * _material );

dz_userdata_t dz_material_get_ud( const dz_material_t * _material );

void dz_material_set_blend( dz_material_t * _material, dz_blend_type_e _blend );
dz_blend_type_e dz_material_get_blend( const dz_material_t * _material );

void dz_material_set_color( dz_material_t * _material, float _r, float _g, float _b, float _a );
void dz_material_get_color( const dz_material_t * _material, float * _r, float * _g, float * _b, float * _a );

void dz_material_set_texture( dz_material_t * _material, const dz_texture_t * _texture );
const dz_texture_t * dz_material_get_texture( dz_material_t * _material );

typedef enum dz_timeline_key_type_e
{
    DZ_TIMELINE_KEY_CONST,
    DZ_TIMELINE_KEY_RANDOMIZE,

    __DZ_TIMELINE_KEY_MAX__
} dz_timeline_key_type_e;

typedef struct dz_timeline_key_t dz_timeline_key_t;

dz_result_t dz_timeline_key_create( dz_service_t * _service, dz_timeline_key_t ** _key, float _p, dz_timeline_key_type_e _type, dz_userdata_t _ud );
void dz_timeline_key_destroy( dz_service_t * _service, const dz_timeline_key_t * _key );

dz_userdata_t dz_timeline_key_get_ud( const dz_timeline_key_t * _key );

float dz_timeline_key_get_p( const dz_timeline_key_t * _key );

dz_result_t dz_timeline_key_const_set_value( dz_timeline_key_t * _key, float _value );
dz_result_t dz_timeline_key_const_get_value( const dz_timeline_key_t * _key, float * _value );

dz_result_t dz_timeline_key_randomize_set_min_max( dz_timeline_key_t * _key, float _min, float _max );
dz_result_t dz_timeline_key_randomize_get_min_max( const dz_timeline_key_t * _key, float * _min, float * _max );

typedef enum dz_timeline_interpolate_type_e
{
    DZ_TIMELINE_INTERPOLATE_LINEAR,
    DZ_TIMELINE_INTERPOLATE_BEZIER2,

    __DZ_TIMELINE_INTERPOLATE_MAX__
} dz_timeline_interpolate_type_e;

typedef struct dz_timeline_interpolate_t dz_timeline_interpolate_t;

dz_result_t dz_timeline_interpolate_create( dz_service_t * _service, dz_timeline_interpolate_t ** _interpolate, dz_timeline_interpolate_type_e _type, dz_userdata_t _ud );
void dz_timeline_interpolate_destroy( dz_service_t * _service, const dz_timeline_interpolate_t * _interpolate );

dz_userdata_t dz_timeline_interpolate_get_ud( const dz_timeline_interpolate_t * _key );

const dz_timeline_key_t * dz_timeline_interpolate_get_key( const dz_timeline_interpolate_t * _interpolate );
const dz_timeline_interpolate_t * dz_timeline_key_get_interpolate( const dz_timeline_key_t * _key );
void dz_timeline_key_set_interpolate( dz_timeline_key_t * _key0, dz_timeline_interpolate_t * _interpolate, dz_timeline_key_t * _key1 );

typedef struct dz_affector_t dz_affector_t;

dz_result_t dz_affector_create( dz_service_t * _service, dz_affector_t ** _affector, dz_userdata_t _ud );
void dz_affector_destroy( dz_service_t * _service, const dz_affector_t * _affector );

dz_userdata_t dz_affector_get_ud( const dz_affector_t * _affector );

typedef enum dz_affector_timeline_type_e
{
    DZ_AFFECTOR_TIMELINE_LIFE,
    DZ_AFFECTOR_TIMELINE_MOVE_SPEED,
    DZ_AFFECTOR_TIMELINE_MOVE_ACCELERATE,
    DZ_AFFECTOR_TIMELINE_ROTATE_SPEED,
    DZ_AFFECTOR_TIMELINE_ROTATE_ACCELERATE,
    DZ_AFFECTOR_TIMELINE_SPIN_SPEED,
    DZ_AFFECTOR_TIMELINE_SPIN_ACCELERATE,
    DZ_AFFECTOR_TIMELINE_STRAFE_SPEED,
    DZ_AFFECTOR_TIMELINE_STRAFE_FRENQUENCE,
    DZ_AFFECTOR_TIMELINE_STRAFE_SIZE,
    DZ_AFFECTOR_TIMELINE_STRAFE_SHIFT,
    DZ_AFFECTOR_TIMELINE_SIZE,
    DZ_AFFECTOR_TIMELINE_COLOR_R,
    DZ_AFFECTOR_TIMELINE_COLOR_G,
    DZ_AFFECTOR_TIMELINE_COLOR_B,
    DZ_AFFECTOR_TIMELINE_COLOR_A,

    __DZ_AFFECTOR_TIMELINE_MAX__
} dz_affector_timeline_type_e;

void dz_affector_set_timeline( dz_affector_t * _affector, dz_affector_timeline_type_e _type, const dz_timeline_key_t * _timeline );
const dz_timeline_key_t * dz_affector_get_timeline( const dz_affector_t * _affector, dz_affector_timeline_type_e _type );

typedef enum dz_timeline_limit_status_e
{
    DZ_TIMELINE_LIMIT_NORMAL = 0x00000000,
    DZ_TIMELINE_LIMIT_MIN = 0x00000001,
    DZ_TIMELINE_LIMIT_MAX = 0x00000002,
    DZ_TIMELINE_LIMIT_MINMAX = 0x00000003
} dz_timeline_limit_status_e;

void dz_affector_timeline_get_limit( dz_affector_timeline_type_e _timeline, dz_timeline_limit_status_e * _status, float * _min, float * _max, float * _default );

typedef enum dz_shape_type_e
{
    DZ_SHAPE_POINT,
    DZ_SHAPE_SEGMENT,
    DZ_SHAPE_CIRCLE,
    DZ_SHAPE_LINE,
    DZ_SHAPE_RECT,
    DZ_SHAPE_POLYGON,
    DZ_SHAPE_MASK,

    __DZ_SHAPE_MAX__
} dz_shape_type_e;

typedef struct dz_shape_t dz_shape_t;

dz_result_t dz_shape_create( dz_service_t * _service, dz_shape_t ** _shape, dz_shape_type_e _type, dz_userdata_t _ud );
void dz_shape_destroy( dz_service_t * _service, const dz_shape_t * _shape );

dz_userdata_t dz_shape_get_ud( const dz_shape_t * _shape );
dz_shape_type_e dz_shape_get_type( const dz_shape_t * _shape );

typedef enum dz_shape_timeline_type_e
{
    DZ_SHAPE_SEGMENT_ANGLE_MIN,
    DZ_SHAPE_SEGMENT_ANGLE_MAX,
    DZ_SHAPE_CIRCLE_RADIUS_MIN,
    DZ_SHAPE_CIRCLE_RADIUS_MAX,
    DZ_SHAPE_CIRCLE_ANGLE_MIN,
    DZ_SHAPE_CIRCLE_ANGLE_MAX,
    DZ_SHAPE_LINE_ANGLE,
    DZ_SHAPE_LINE_SIZE,
    DZ_SHAPE_RECT_WIDTH_MIN,
    DZ_SHAPE_RECT_WIDTH_MAX,
    DZ_SHAPE_RECT_HEIGHT_MIN,
    DZ_SHAPE_RECT_HEIGHT_MAX,

    __DZ_SHAPE_TIMELINE_MAX__
} dz_shape_timeline_type_e;

void dz_shape_set_timeline( dz_shape_t * _shape, dz_shape_timeline_type_e _type, const dz_timeline_key_t * _timeline );
const dz_timeline_key_t * dz_shape_get_timeline( const dz_shape_t * _shape, dz_shape_timeline_type_e _type );

void dz_shape_timeline_get_limit( dz_shape_timeline_type_e _timeline, dz_timeline_limit_status_e * _status, float * _min, float * _max, float * _default );

dz_result_t dz_shape_set_polygon( dz_shape_t * _shape, const float * _triangles, uint32_t _count );
void dz_shape_get_polygon( const dz_shape_t * _shape, const float ** _triangles, uint32_t * _count );

dz_result_t dz_shape_set_mask( dz_shape_t * _shape, const void * _buffer, uint32_t _bites, uint32_t _pitch, uint32_t _width, uint32_t _height );
void dz_shape_get_mask( const dz_shape_t * _shape, const void ** _buffer, uint32_t * _bites, uint32_t * _pitch, uint32_t * _width, uint32_t * _height );

void dz_shape_set_mask_scale( dz_shape_t * _shape, float _scale );
float dz_shape_get_mask_scale( const dz_shape_t * _shape );

void dz_shape_set_mask_threshold( dz_shape_t * _shape, uint32_t _threshold );
uint32_t dz_shape_get_mask_threshold( const dz_shape_t * _shape );

typedef struct dz_emitter_t dz_emitter_t;

dz_result_t dz_emitter_create( dz_service_t * _service, dz_emitter_t ** _emitter, dz_userdata_t _ud );
void dz_emitter_destroy( dz_service_t * _service, const dz_emitter_t * _emitter );

dz_userdata_t dz_emitter_get_ud( const dz_emitter_t * _emitter );

void dz_emitter_set_life( dz_emitter_t * _emitter, float _life );
float dz_emitter_get_life( const dz_emitter_t * _emitter );

typedef enum dz_emitter_timeline_type_e
{
    DZ_EMITTER_SPAWN_DELAY,
    DZ_EMITTER_SPAWN_COUNT,
    DZ_EMITTER_SPAWN_SPIN_MIN,
    DZ_EMITTER_SPAWN_SPIN_MAX,

    __DZ_EMITTER_TIMELINE_MAX__
} dz_emitter_timeline_type_e;

void dz_emitter_set_timeline( dz_emitter_t * _emitter, dz_emitter_timeline_type_e _type, const dz_timeline_key_t * _timeline );
const dz_timeline_key_t * dz_emitter_get_timeline( const dz_emitter_t * _emitter, dz_emitter_timeline_type_e _type );

void dz_emitter_timeline_get_limit( dz_emitter_timeline_type_e _timeline, dz_timeline_limit_status_e * _status, float * _min, float * _max, float * _default );

typedef struct dz_effect_t dz_effect_t;

dz_result_t dz_effect_create( dz_service_t * _service, dz_effect_t ** _effect, const dz_material_t * _material, const dz_shape_t * _shape, const dz_emitter_t * _emitter, const dz_affector_t * _affector, uint32_t _seed, float _life, dz_userdata_t _ud );
void dz_effect_destroy( dz_service_t * _service, const dz_effect_t * _effect );

dz_userdata_t dz_effect_get_ud( const dz_effect_t * _effect );

const dz_material_t * dz_effect_set_material( dz_effect_t * _effect, const dz_material_t * _material );
const dz_shape_t * dz_effect_set_shape( dz_effect_t * _effect, const dz_shape_t * _shape );
const dz_emitter_t * dz_effect_set_emitter( dz_effect_t * _effect, const dz_emitter_t * _emitter );
const dz_affector_t * dz_effect_set_affector( dz_effect_t * _effect, const dz_affector_t * _affector );

void dz_effect_set_life( dz_effect_t * _effect, float _life );
float dz_effect_get_life( const dz_effect_t * _effect );

void dz_effect_set_loop( dz_effect_t * _effect, dz_bool_t _loop );
dz_bool_t dz_effect_get_loop( const dz_effect_t * _effect );

void dz_effect_set_time( dz_effect_t * _effect, float _time );
float dz_effect_get_time( const dz_effect_t * _effect );

uint32_t dz_effect_get_seed( const dz_effect_t * _effect );

void dz_effect_set_particle_limit( dz_effect_t * _effect, uint32_t _limit );
uint32_t dz_effect_get_particle_limit( const dz_effect_t * _effect );

void dz_effect_set_position( dz_effect_t * _effect, float _x, float _y );
void dz_effect_get_position( const dz_effect_t * _effect, float * _x, float * _y );

void dz_effect_set_rotate( dz_effect_t * _effect, float _angle );
float dz_effect_get_rotate( const dz_effect_t * _effect );

void dz_effect_reset( dz_effect_t * _effect );

void dz_effect_spawn_pause( dz_effect_t * _effect );
void dz_effect_spawn_resume( dz_effect_t * _effect );
dz_bool_t dz_effect_is_spawn_pause( const dz_effect_t * _effect );

dz_result_t dz_effect_update( dz_service_t * _service, dz_effect_t * _effect, float _time );

typedef enum dz_effect_state_e
{
    DZ_EFFECT_PROCESS = 0x00000001,
    DZ_EFFECT_EMIT_COMPLETE = 0x00000002,
    DZ_EFFECT_PARTICLE_COMPLETE = 0x00000004,
} dz_effect_state_e;

dz_effect_state_e dz_emitter_get_state( const dz_effect_t * _effect );

typedef enum dz_effect_mesh_flags_e
{
    DZ_EFFECT_MESH_FLAG_NONE = 0x00000000,
    DZ_EFFECT_MESH_FLAG_INDEX32 = 0x00000001,
} dz_effect_mesh_flags_e;

typedef struct dz_effect_mesh_chunk_t
{
    uint16_t offset;
    uint16_t vertex_size;
    uint16_t index_size;

    dz_blend_type_e blend_type;

    const dz_texture_t * texture;
} dz_effect_mesh_chunk_t;

typedef struct dz_effect_mesh_t
{
    void * position_buffer;
    dz_size_t position_offset;
    dz_size_t position_stride;

    void * color_buffer;
    dz_size_t color_offset;
    dz_size_t color_stride;

    void * uv_buffer;
    dz_size_t uv_offset;
    dz_size_t uv_stride;

    void * index_buffer;

    dz_effect_mesh_flags_e flags;
    float r;
    float g;
    float b;
    float a;

    float m[16];
} dz_effect_mesh_t;

void dz_effect_compute_mesh( const dz_effect_t * _effect, dz_effect_mesh_t * _mesh, dz_effect_mesh_chunk_t * _chunks, uint32_t _capacity, uint32_t * _count );

#endif
