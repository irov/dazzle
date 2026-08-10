#ifndef DZ_DAZZLE_H_
#define DZ_DAZZLE_H_

#include "dazzle/config.h"

dz_uint32_t dz_get_magic( void );
dz_uint32_t dz_get_version( void );

typedef struct dz_vec2_t
{
    dz_float_t x;
    dz_float_t y;
} dz_vec2_t;

typedef struct dz_vec3_t
{
    dz_float_t x;
    dz_float_t y;
    dz_float_t z;
} dz_vec3_t;

typedef struct dz_vec4_t
{
    dz_float_t x;
    dz_float_t y;
    dz_float_t z;
    dz_float_t w;
} dz_vec4_t;

typedef struct dz_quat_t
{
    dz_float_t x;
    dz_float_t y;
    dz_float_t z;
    dz_float_t w;
} dz_quat_t;

typedef struct dz_mat4_t
{
    dz_float_t m[16];
} dz_mat4_t;

typedef struct dz_transform_t
{
    dz_vec3_t position;
    dz_quat_t rotation;
    dz_vec3_t scale;
} dz_transform_t;

typedef struct dz_aabb_t
{
    dz_vec3_t minimum;
    dz_vec3_t maximum;
    dz_bool_t valid;
} dz_aabb_t;

typedef struct dz_mesh_vertex_t
{
    dz_vec3_t position;
    dz_vec3_t normal;
    dz_vec4_t tangent;
    dz_vec2_t uv0;
    dz_vec2_t uv1;
} dz_mesh_vertex_t;

typedef struct dz_mesh_desc_t
{
    dz_uint32_t id;
    const dz_mesh_vertex_t * vertices;
    dz_uint32_t vertex_count;
    const dz_uint32_t * indices;
    dz_uint32_t index_count;
    dz_aabb_t bounds;
} dz_mesh_desc_t;

typedef enum dz_projection_type_e
{
    DZ_PROJECTION_ORTHOGRAPHIC = 0,
    DZ_PROJECTION_PERSPECTIVE = 1,

    __DZ_PROJECTION_MAX__
} dz_projection_type_e;

typedef struct dz_project_profile_t
{
    dz_projection_type_e projection;
    dz_vec3_t position;
    dz_vec3_t forward;
    dz_vec3_t up;
    dz_float_t field_of_view;
    dz_float_t orthographic_height;
    dz_float_t near_plane;
    dz_float_t far_plane;
} dz_project_profile_t;

typedef struct dz_camera_state_t
{
    dz_projection_type_e projection;
    dz_vec3_t position;
    dz_vec3_t forward;
    dz_vec3_t up;
    dz_float_t field_of_view;
    dz_float_t orthographic_height;
    dz_float_t near_plane;
    dz_float_t far_plane;
    dz_float_t viewport_width;
    dz_float_t viewport_height;
} dz_camera_state_t;

void dz_project_profile_default( dz_project_profile_t * _profile, dz_projection_type_e _projection );
void dz_quat_to_euler_xyz_degrees( const dz_quat_t * _quat, dz_vec3_t * _euler );
void dz_quat_from_euler_xyz_degrees( const dz_vec3_t * _euler, dz_quat_t * _quat );
void dz_camera_state_from_profile( const dz_project_profile_t * _profile, dz_float_t _viewport_width, dz_float_t _viewport_height, dz_camera_state_t * _camera );
void dz_mat4_identity( dz_mat4_t * _matrix );
void dz_camera_compute_view( const dz_camera_state_t * _camera, dz_mat4_t * _view );
void dz_camera_compute_projection( const dz_camera_state_t * _camera, dz_mat4_t * _projection );
void dz_camera_test_aabb( const dz_camera_state_t * _camera, const dz_aabb_t * _aabb, dz_bool_t * _visible );

typedef struct dz_service_t dz_service_t;

typedef void * (*dz_malloc_t)(dz_size_t _size, dz_userdata_t _ud);
typedef void * (*dz_realloc_t)(void * const _ptr, dz_size_t _size, dz_userdata_t _ud);
typedef void (*dz_free_t)(const void * _ptr, dz_userdata_t _ud);
typedef dz_float_t (*dz_sqrtf_t)(dz_float_t _a, dz_userdata_t _ud);
typedef dz_float_t(*dz_cosf_t)(dz_float_t _a, dz_userdata_t _ud);
typedef dz_float_t(*dz_sinf_t)(dz_float_t _a, dz_userdata_t _ud);

typedef struct dz_service_providers_t
{
    dz_malloc_t f_malloc;
    dz_realloc_t f_realloc;
    dz_free_t f_free;
    dz_sqrtf_t f_sqrtf;
    dz_cosf_t f_cosf;
    dz_sinf_t f_sinf;
} dz_service_providers_t;

void dz_service_create( dz_service_t ** _service, const dz_service_providers_t * _providers, dz_userdata_t _ud );
void dz_service_destroy( dz_service_t * const _service );

void dz_service_get_providers( const dz_service_t * _service, dz_service_providers_t * _providers );

typedef struct dz_texture_t dz_texture_t;

void dz_texture_create( const dz_service_t * _service, dz_texture_t ** _texture, dz_userdata_t _ud );
void dz_texture_destroy( const dz_service_t * _service, const dz_texture_t * _texture );

void dz_texture_set_ud( dz_texture_t * const _texture, dz_userdata_t _ud );
dz_userdata_t dz_texture_get_ud( const dz_texture_t * _texture );

void dz_texture_set_uv( dz_texture_t * const _texture, const dz_float_t * _u, const dz_float_t * _v );
void dz_texture_get_uv( const dz_texture_t * _texture, dz_float_t * const _u, dz_float_t * const _v );

void dz_texture_set_width( dz_texture_t * const _texture, dz_float_t _width );
dz_float_t dz_texture_get_width( const dz_texture_t * _texture );

void dz_texture_set_height( dz_texture_t * const _texture, dz_float_t _height );
dz_float_t dz_texture_get_height( const dz_texture_t * _texture );

void dz_texture_set_trim_offset( dz_texture_t * const _texture, dz_float_t _x, dz_float_t _y );
void dz_texture_get_trim_offset( const dz_texture_t * _texture, dz_float_t * const _x, dz_float_t * const _y );

void dz_texture_set_trim_size( dz_texture_t * _texture, dz_float_t _width, dz_float_t _height );
void dz_texture_get_trim_size( const dz_texture_t * _texture, dz_float_t * const _width, dz_float_t * const _height );

void dz_texture_set_sequence_delay( dz_texture_t * const _texture, dz_float_t _delay );
dz_float_t dz_texture_get_sequence_delay( const dz_texture_t * _texture );

typedef struct dz_atlas_t dz_atlas_t;

void dz_atlas_create( const dz_service_t * _service, dz_atlas_t ** _atlas, dz_userdata_t _surface, dz_userdata_t _ud );
void dz_atlas_destroy( const dz_service_t * _service, const dz_atlas_t * _atlas );

void dz_atlas_set_ud( dz_atlas_t * const _atlas, dz_userdata_t _ud );
dz_userdata_t dz_atlas_get_ud( const dz_atlas_t * _atlas );

void dz_atlas_set_surface( dz_atlas_t * const _atlas, dz_userdata_t _surface );
dz_userdata_t dz_atlas_get_surface( const dz_atlas_t * _atlas );

typedef enum dz_blend_type_e
{
    DZ_BLEND_NORMAL,
    DZ_BLEND_ADD,
    DZ_BLEND_MULTIPLY,
    DZ_BLEND_SCREEN,

    __DZ_BLEND_MAX__
} dz_blend_type_e;

dz_blend_type_e dz_material_get_default_blend( void );

typedef enum dz_material_mode_e
{
    DZ_MATERIAL_MODE_SOLID,
    DZ_MATERIAL_MODE_TEXTURE,
    DZ_MATERIAL_MODE_SEQUENCE,

    __DZ_MATERIAL_MODE_MAX__
} dz_material_mode_e;

dz_material_mode_e dz_material_get_default_mode( void );

typedef struct dz_material_t dz_material_t;

void dz_material_create( const dz_service_t * _service, dz_material_t ** _material, dz_userdata_t _ud );
void dz_material_destroy( const dz_service_t * _service, const dz_material_t * _material );

void dz_material_set_ud( dz_material_t * const _material, dz_userdata_t _ud );
dz_userdata_t dz_material_get_ud( const dz_material_t * _material );

void dz_material_set_blend( dz_material_t * const _material, dz_blend_type_e _blend );
dz_blend_type_e dz_material_get_blend( const dz_material_t * _material );

void dz_material_set_color( dz_material_t * const _material, dz_float_t _r, dz_float_t _g, dz_float_t _b, dz_float_t _a );
void dz_material_get_color( const dz_material_t * _material, dz_float_t * const _r, dz_float_t * const _g, dz_float_t * const _b, dz_float_t * const _a );

void dz_material_set_atlas( dz_material_t * const _material, const dz_atlas_t * _atlas );
const dz_atlas_t * dz_material_get_atlas( const dz_material_t * _material );

dz_uint32_t dz_material_get_texture_slot_count( const dz_material_t * _material );
dz_result_t dz_material_add_texture( dz_material_t * const _material, const dz_texture_t * _texture );
dz_result_t dz_material_get_texture( const dz_material_t * _material, dz_uint32_t _index, const dz_texture_t ** _texture );
dz_result_t dz_material_set_texture_random_weight( dz_material_t * const _material, dz_uint32_t _index, dz_float_t _weight );
dz_result_t dz_material_get_texture_random_weight( const dz_material_t * _material, dz_uint32_t _index, dz_float_t * const _weight );
dz_result_t dz_material_pop_texture( dz_material_t * const _material, const dz_texture_t ** _texture );

void dz_material_set_texture_index( dz_material_t * const _material, dz_uint32_t _index );
dz_uint32_t dz_material_get_texture_index( const dz_material_t * _material );

void dz_material_set_texture_count( dz_material_t * const _material, dz_uint32_t _count );
dz_uint32_t dz_material_get_texture_count( const dz_material_t * _material );



void dz_material_set_mode( dz_material_t * const _material, dz_material_mode_e _mode );
dz_material_mode_e dz_material_get_mode( const dz_material_t * _material );

typedef enum dz_timeline_key_type_e
{
    DZ_TIMELINE_KEY_CONST,
    DZ_TIMELINE_KEY_RANDOMIZE,

    __DZ_TIMELINE_KEY_MAX__
} dz_timeline_key_type_e;

typedef struct dz_timeline_key_t dz_timeline_key_t;

dz_result_t dz_timeline_key_create( const dz_service_t * _service, dz_timeline_key_t ** _key, dz_float_t _p, dz_timeline_key_type_e _type, dz_userdata_t _ud );
void dz_timeline_key_destroy( const dz_service_t * _service, const dz_timeline_key_t * _key );

void dz_timeline_key_set_ud( dz_timeline_key_t * const _key, dz_userdata_t _ud );
dz_userdata_t dz_timeline_key_get_ud( const dz_timeline_key_t * _key );

void dz_timeline_key_set_type( dz_timeline_key_t * const _key, dz_timeline_key_type_e _type );
dz_timeline_key_type_e dz_timeline_key_get_type( const dz_timeline_key_t * _key );

void dz_timeline_key_set_p( dz_timeline_key_t * const _key, dz_float_t _p );
dz_float_t dz_timeline_key_get_p( const dz_timeline_key_t * _key );

void dz_timeline_key_set_const_value( dz_timeline_key_t * const _key, dz_float_t _value );
void dz_timeline_key_get_const_value( const dz_timeline_key_t * _key, dz_float_t * const _value );

void dz_timeline_key_set_randomize_min_max( dz_timeline_key_t * const _key, dz_float_t _min, dz_float_t _max );
void dz_timeline_key_get_randomize_min_max( const dz_timeline_key_t * _key, dz_float_t * const _min, dz_float_t * const _max );

typedef enum dz_timeline_interpolate_type_e
{
    DZ_TIMELINE_INTERPOLATE_STEP,
    DZ_TIMELINE_INTERPOLATE_LINEAR,
    DZ_TIMELINE_INTERPOLATE_BEZIER2,
    DZ_TIMELINE_INTERPOLATE_HERMITE,

    __DZ_TIMELINE_INTERPOLATE_MAX__
} dz_timeline_interpolate_type_e;

typedef struct dz_timeline_interpolate_t dz_timeline_interpolate_t;

void dz_timeline_interpolate_create( const dz_service_t * _service, dz_timeline_interpolate_t ** _interpolate, dz_timeline_interpolate_type_e _type, dz_userdata_t _ud );
void dz_timeline_interpolate_destroy( const dz_service_t * _service, const dz_timeline_interpolate_t * _interpolate );

void dz_timeline_interpolate_set_ud( dz_timeline_interpolate_t * const _interpolate, dz_userdata_t _ud );
dz_userdata_t dz_timeline_interpolate_get_ud( const dz_timeline_interpolate_t * _interpolate );

void dz_timeline_interpolate_set_type( dz_timeline_interpolate_t * const _interpolate, dz_timeline_interpolate_type_e _type );
dz_timeline_interpolate_type_e dz_timeline_interpolate_get_type( const dz_timeline_interpolate_t * _interpolate );

void dz_timeline_interpolate_set_bezier2( dz_timeline_interpolate_t * const _interpolate, dz_float_t _p0, dz_float_t _p1 );
void dz_timeline_interpolate_get_bezier2( const dz_timeline_interpolate_t * _interpolate, dz_float_t * const _p0, dz_float_t * const _p1 );
void dz_timeline_interpolate_set_hermite( dz_timeline_interpolate_t * const _interpolate, dz_float_t _out_tangent, dz_float_t _in_tangent );
void dz_timeline_interpolate_get_hermite( const dz_timeline_interpolate_t * _interpolate, dz_float_t * const _out_tangent, dz_float_t * const _in_tangent );

const dz_timeline_key_t * dz_timeline_interpolate_get_key( const dz_timeline_interpolate_t * _interpolate );
const dz_timeline_interpolate_t * dz_timeline_key_get_interpolate( const dz_timeline_key_t * _key );
void dz_timeline_interpolate_set_key( dz_timeline_interpolate_t * const _interpolate, dz_timeline_key_t * const _key );
dz_result_t dz_timeline_key_set_interpolate( dz_timeline_key_t * const _key, dz_timeline_interpolate_t * const _interpolate );

typedef struct dz_affector_t dz_affector_t;

void dz_affector_create( const dz_service_t * _service, dz_affector_t ** _affector, dz_userdata_t _ud );
void dz_affector_destroy( const dz_service_t * _service, const dz_affector_t * _affector );

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
    DZ_AFFECTOR_TIMELINE_SCALE,
    DZ_AFFECTOR_TIMELINE_ASPECT,
    DZ_AFFECTOR_TIMELINE_COLOR_R,
    DZ_AFFECTOR_TIMELINE_COLOR_G,
    DZ_AFFECTOR_TIMELINE_COLOR_B,
    DZ_AFFECTOR_TIMELINE_COLOR_A,
    DZ_AFFECTOR_TIMELINE_DIRECTION_Z,
    DZ_AFFECTOR_TIMELINE_GRAVITY_X,
    DZ_AFFECTOR_TIMELINE_GRAVITY_Y,
    DZ_AFFECTOR_TIMELINE_GRAVITY_Z,
    DZ_AFFECTOR_TIMELINE_DRAG,

    __DZ_AFFECTOR_TIMELINE_MAX__
} dz_affector_timeline_type_e;

void dz_affector_set_timeline( dz_affector_t * const _affector, dz_affector_timeline_type_e _type, const dz_timeline_key_t * _timeline );
const dz_timeline_key_t * dz_affector_get_timeline( const dz_affector_t * _affector, dz_affector_timeline_type_e _type );

typedef enum dz_timeline_limit_status_e
{
    DZ_TIMELINE_LIMIT_NORMAL = 0x00000000,
    DZ_TIMELINE_LIMIT_MIN = 0x00000001,
    DZ_TIMELINE_LIMIT_MAX = 0x00000002,
    DZ_TIMELINE_LIMIT_MINMAX = 0x00000003
} dz_timeline_limit_status_e;

void dz_affector_timeline_get_limit( dz_affector_timeline_type_e _timeline, dz_timeline_limit_status_e * const _status, dz_float_t * const _min, dz_float_t * const _max, dz_float_t * const _default, dz_float_t * const _factor );

dz_float_t dz_affector_get_particle_size( void );

typedef enum dz_shape_type_e
{
    DZ_SHAPE_POINT,
    DZ_SHAPE_SEGMENT,
    DZ_SHAPE_CIRCLE,
    DZ_SHAPE_LINE,
    DZ_SHAPE_RECT,
    DZ_SHAPE_POLYGON,
    DZ_SHAPE_MASK,
    DZ_SHAPE_SPHERE,
    DZ_SHAPE_BOX,
    DZ_SHAPE_CONE,
    DZ_SHAPE_CYLINDER,
    DZ_SHAPE_MESH_SURFACE,
    DZ_SHAPE_MESH_VOLUME,

    __DZ_SHAPE_MAX__
} dz_shape_type_e;

typedef struct dz_shape_t dz_shape_t;

void dz_shape_create( const dz_service_t * _service, dz_shape_t ** _shape, dz_shape_type_e _type, dz_userdata_t _ud );
void dz_shape_destroy( const dz_service_t * _service, const dz_shape_t * _shape );

void dz_shape_set_ud( dz_shape_t * const _shape, dz_userdata_t _ud );
dz_userdata_t dz_shape_get_ud( const dz_shape_t * _shape );

void dz_shape_set_type( dz_shape_t * const _shape, dz_shape_type_e _type );
dz_shape_type_e dz_shape_get_type( const dz_shape_t * _shape );
void dz_shape_set_transform( dz_shape_t * _shape, const dz_transform_t * _transform );
void dz_shape_get_transform( const dz_shape_t * _shape, dz_transform_t * _transform );
void dz_shape_set_dimensions( dz_shape_t * _shape, const dz_vec3_t * _dimensions );
void dz_shape_get_dimensions( const dz_shape_t * _shape, dz_vec3_t * _dimensions );
void dz_shape_set_mesh_id( dz_shape_t * _shape, dz_uint32_t _mesh_id );
dz_uint32_t dz_shape_get_mesh_id( const dz_shape_t * _shape );

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
    DZ_SHAPE_LINE_OFFSET,
    DZ_SHAPE_RECT_WIDTH_MIN,
    DZ_SHAPE_RECT_WIDTH_MAX,
    DZ_SHAPE_RECT_HEIGHT_MIN,
    DZ_SHAPE_RECT_HEIGHT_MAX,
    DZ_SHAPE_SPHERE_RADIUS_MIN,
    DZ_SHAPE_SPHERE_RADIUS_MAX,
    DZ_SHAPE_BOX_WIDTH,
    DZ_SHAPE_BOX_HEIGHT,
    DZ_SHAPE_BOX_DEPTH,
    DZ_SHAPE_CONE_RADIUS,
    DZ_SHAPE_CONE_HEIGHT,
    DZ_SHAPE_CYLINDER_RADIUS,
    DZ_SHAPE_CYLINDER_HEIGHT,

    __DZ_SHAPE_TIMELINE_MAX__
} dz_shape_timeline_type_e;

void dz_shape_set_timeline( dz_shape_t * const _shape, dz_shape_timeline_type_e _type, const dz_timeline_key_t * _timeline );
const dz_timeline_key_t * dz_shape_get_timeline( const dz_shape_t * _shape, dz_shape_timeline_type_e _type );

void dz_shape_timeline_get_limit( dz_shape_timeline_type_e _timeline, dz_timeline_limit_status_e * const _status, dz_float_t * const _min, dz_float_t * const _max, dz_float_t * const _default, dz_float_t * const _factor );

void dz_shape_set_polygon( dz_shape_t * const _shape, const dz_float_t * _triangles, dz_uint32_t _count );
void dz_shape_get_polygon( const dz_shape_t * _shape, const dz_float_t ** _triangles, dz_uint32_t * _count );

void dz_shape_set_mask( dz_shape_t * const _shape, const void * _buffer, dz_uint32_t _bites, dz_uint32_t _pitch, dz_uint32_t _width, dz_uint32_t _height );
void dz_shape_get_mask( const dz_shape_t * _shape, const void ** _buffer, dz_uint32_t * const _bites, dz_uint32_t * const _pitch, dz_uint32_t * const _width, dz_uint32_t * const _height );

void dz_shape_set_mask_scale( dz_shape_t * const _shape, dz_float_t _scale );
dz_float_t dz_shape_get_mask_scale( const dz_shape_t * _shape );

void dz_shape_set_mask_threshold( dz_shape_t * const _shape, dz_uint32_t _threshold );
dz_uint32_t dz_shape_get_mask_threshold( const dz_shape_t * _shape );

typedef struct dz_emitter_t dz_emitter_t;

void dz_emitter_create( const dz_service_t * _service, dz_emitter_t ** _emitter, dz_userdata_t _ud );
void dz_emitter_destroy( const dz_service_t * _service, const dz_emitter_t * _emitter );

void dz_emitter_set_ud( dz_emitter_t * const _emitter, dz_userdata_t _ud );
dz_userdata_t dz_emitter_get_ud( const dz_emitter_t * _emitter );

void dz_emitter_set_life( dz_emitter_t * const _emitter, dz_float_t _life );
dz_float_t dz_emitter_get_life( const dz_emitter_t * _emitter );

typedef enum dz_emitter_timeline_type_e
{
    DZ_EMITTER_SPAWN_DELAY,
    DZ_EMITTER_SPAWN_COUNT,
    DZ_EMITTER_SPAWN_SPIN_MIN,
    DZ_EMITTER_SPAWN_SPIN_MAX,
    DZ_EMITTER_SPAWN_ELEVATION_MIN,
    DZ_EMITTER_SPAWN_ELEVATION_MAX,

    __DZ_EMITTER_TIMELINE_MAX__
} dz_emitter_timeline_type_e;

void dz_emitter_set_timeline( dz_emitter_t * _emitter, dz_emitter_timeline_type_e _type, const dz_timeline_key_t * _timeline );
const dz_timeline_key_t * dz_emitter_get_timeline( const dz_emitter_t * _emitter, dz_emitter_timeline_type_e _type );

void dz_emitter_timeline_get_limit( dz_emitter_timeline_type_e _timeline, dz_timeline_limit_status_e * const _status, dz_float_t * const _min, dz_float_t * const _max, dz_float_t * const _default, dz_float_t * const _factor );

typedef struct dz_effect_t dz_effect_t;

typedef enum dz_particle_mode_e
{
    DZ_PARTICLE_MODE_SPRITE = 0,
    DZ_PARTICLE_MODE_TRAIL = 1,
    DZ_PARTICLE_MODE_BEAM = 2,
    DZ_PARTICLE_MODE_PATH = 3,
    DZ_PARTICLE_MODE_MESH = 4,

    __DZ_PARTICLE_MODE_MAX__
} dz_particle_mode_e;

typedef enum dz_particle_orientation_e
{
    DZ_PARTICLE_ORIENTATION_CAMERA = 0,
    DZ_PARTICLE_ORIENTATION_CAMERA_AXIS = 1,
    DZ_PARTICLE_ORIENTATION_VELOCITY = 2,
    DZ_PARTICLE_ORIENTATION_WORLD = 3,

    __DZ_PARTICLE_ORIENTATION_MAX__
} dz_particle_orientation_e;

typedef enum dz_particle_sort_e
{
    DZ_PARTICLE_SORT_NONE = 0,
    DZ_PARTICLE_SORT_BIRTH_ASCENDING = 1,
    DZ_PARTICLE_SORT_BIRTH_DESCENDING = 2,
    DZ_PARTICLE_SORT_CAMERA_NEAR = 3,
    DZ_PARTICLE_SORT_CAMERA_FAR = 4,

    __DZ_PARTICLE_SORT_MAX__
} dz_particle_sort_e;

#define DZ_EFFECT_LAYER_MAX 64
#define DZ_EFFECT_TRIGGER_MAX 128
#define DZ_EFFECT_EMITTER_INSTANCE_MAX 128
#define DZ_EFFECT_LAYER_NONE (~0U)
#define DZ_RESOURCE_ID_NONE ( ~0U )
#define DZ_EFFECT_MESH_MAX 64

typedef enum dz_effect_event_type_e
{
    DZ_EFFECT_EVENT_EFFECT_START,
    DZ_EFFECT_EVENT_TIME,
    DZ_EFFECT_EVENT_LAYER_EMIT_COMPLETE,
    DZ_EFFECT_EVENT_LAYER_PARTICLE_COMPLETE,
    DZ_EFFECT_EVENT_PARTICLE_DEATH,
    DZ_EFFECT_EVENT_CUSTOM,

    __DZ_EFFECT_EVENT_MAX__
} dz_effect_event_type_e;

typedef struct dz_effect_layer_desc_t
{
    const dz_material_t * material;
    const dz_shape_t * shape;
    const dz_emitter_t * emitter;
    const dz_affector_t * affector;

    dz_float_t x;
    dz_float_t y;
    dz_float_t angle;

    dz_float_t z;
    dz_quat_t rotation;
    dz_vec3_t scale;
    dz_particle_mode_e particle_mode;
    dz_particle_orientation_e orientation;
    dz_particle_sort_e sorting;
    dz_vec3_t orientation_axis;
    dz_uint32_t mesh_id;
    dz_float_t trail_width;
    dz_float_t trail_lifetime;

    dz_float_t life;
    dz_uint32_t seed;
} dz_effect_layer_desc_t;

void dz_effect_layer_desc_default( dz_effect_layer_desc_t * _layer );

typedef struct dz_effect_trigger_desc_t
{
    dz_effect_event_type_e event_type;

    dz_uint32_t source_layer_index;
    dz_uint32_t target_layer_index;

    dz_float_t time;
    dz_float_t probability;

    dz_uint32_t spawn_count_min;
    dz_uint32_t spawn_count_max;

    dz_float_t delay_min;
    dz_float_t delay_max;

    dz_bool_t inherit_position;
    dz_bool_t inherit_angle;
    dz_bool_t inherit_velocity;

    dz_float_t offset_x;
    dz_float_t offset_y;
    dz_float_t angle_offset;
} dz_effect_trigger_desc_t;

void dz_effect_create( const dz_service_t * _service, dz_effect_t ** _effect, dz_float_t _life, dz_uint32_t _seed, dz_userdata_t _ud );
void dz_effect_create_with_profile( const dz_service_t * _service, dz_effect_t ** _effect, const dz_project_profile_t * _profile, dz_float_t _life, dz_uint32_t _seed,
                                    dz_userdata_t _ud );
void dz_effect_destroy( const dz_service_t * _service, const dz_effect_t * _effect );
void dz_effect_get_project_profile( const dz_effect_t * _effect, dz_project_profile_t * _profile );
void dz_effect_set_camera_defaults( dz_effect_t * _effect, const dz_project_profile_t * _profile );

dz_uint32_t dz_effect_get_mesh_count( const dz_effect_t * _effect );
void dz_effect_add_mesh( const dz_service_t * _service, dz_effect_t * _effect, const dz_mesh_desc_t * _mesh );
dz_result_t dz_effect_get_mesh( const dz_effect_t * _effect, dz_uint32_t _id, dz_mesh_desc_t * _mesh );
void dz_effect_get_mesh_at( const dz_effect_t * _effect, dz_uint32_t _index, dz_mesh_desc_t * _mesh );
dz_result_t dz_effect_remove_mesh( const dz_service_t * _service, dz_effect_t * _effect, dz_uint32_t _id );

void dz_effect_set_ud( dz_effect_t * const _effect, dz_userdata_t _ud );
dz_userdata_t dz_effect_get_ud( const dz_effect_t * _effect );

void dz_effect_set_atlas( dz_effect_t * const _effect, const dz_atlas_t * _atlas );
const dz_atlas_t * dz_effect_get_atlas( const dz_effect_t * _effect );

dz_uint32_t dz_effect_get_layer_count( const dz_effect_t * _effect );
dz_result_t dz_effect_add_layer( dz_effect_t * const _effect, const dz_effect_layer_desc_t * _layer, dz_uint32_t * const _index );
void dz_effect_remove_layer( dz_effect_t * const _effect, dz_uint32_t _index, dz_effect_layer_desc_t * const _layer );
dz_result_t dz_effect_set_layer( dz_effect_t * const _effect, dz_uint32_t _index, const dz_effect_layer_desc_t * _layer );
void dz_effect_get_layer( const dz_effect_t * _effect, dz_uint32_t _index, dz_effect_layer_desc_t * const _layer );

dz_uint32_t dz_effect_get_trigger_count( const dz_effect_t * _effect );
dz_result_t dz_effect_add_trigger( dz_effect_t * const _effect, const dz_effect_trigger_desc_t * _trigger, dz_uint32_t * const _index );
void dz_effect_remove_trigger( dz_effect_t * const _effect, dz_uint32_t _index, dz_effect_trigger_desc_t * const _trigger );
dz_result_t dz_effect_set_trigger( dz_effect_t * const _effect, dz_uint32_t _index, const dz_effect_trigger_desc_t * _trigger );
void dz_effect_get_trigger( const dz_effect_t * _effect, dz_uint32_t _index, dz_effect_trigger_desc_t * const _trigger );

void dz_effect_set_life( dz_effect_t * const _effect, dz_float_t _life );
dz_float_t dz_effect_get_life( const dz_effect_t * _effect );

void dz_effect_set_seed( dz_effect_t * const _effect, dz_uint32_t _seed );
dz_uint32_t dz_effect_get_seed( const dz_effect_t * _effect );

typedef struct dz_instance_t dz_instance_t;

void dz_instance_create( const dz_service_t * _service, dz_instance_t ** _instance, const dz_effect_t * _effect, dz_userdata_t _ud );
void dz_instance_destroy( const dz_service_t * _service, const dz_instance_t * _instance );

void dz_instance_set_ud( dz_instance_t * const _instance, dz_userdata_t _ud );
dz_userdata_t dz_instance_get_ud( const dz_instance_t * _instance );

void dz_instance_set_effect( dz_instance_t * const _instance, const dz_effect_t * _effect );
const dz_effect_t * dz_instance_get_effect( const dz_instance_t * _instance );

void dz_instance_set_loop( dz_instance_t * const _instance, dz_bool_t _loop );
dz_bool_t dz_instance_get_loop( const dz_instance_t * _instance );

dz_result_t dz_instance_set_time( dz_instance_t * const _instance, dz_float_t _time );
dz_float_t dz_instance_get_time( const dz_instance_t * _instance );
void dz_instance_set_fixed_step( dz_instance_t * _instance, dz_float_t _step );
dz_float_t dz_instance_get_fixed_step( const dz_instance_t * _instance );

void dz_instance_set_seed( dz_instance_t * const _instance, dz_uint32_t _seed );
dz_uint32_t dz_instance_get_seed( const dz_instance_t * _instance );

dz_result_t dz_instance_set_particle_limit( dz_instance_t * const _instance, dz_uint16_t _limit );
dz_uint16_t dz_instance_get_particle_limit( const dz_instance_t * _instance );

void dz_instance_set_position( dz_instance_t * const _instance, dz_float_t _x, dz_float_t _y );
void dz_instance_get_position( const dz_instance_t * _instance, dz_float_t * const _x, dz_float_t * const _y );
void dz_instance_set_position3( dz_instance_t * const _instance, const dz_vec3_t * _position );
void dz_instance_get_position3( const dz_instance_t * _instance, dz_vec3_t * _position );
void dz_instance_set_transform( dz_instance_t * const _instance, const dz_transform_t * _transform );
void dz_instance_get_transform( const dz_instance_t * _instance, dz_transform_t * _transform );

void dz_instance_set_color( dz_instance_t * const _instance, dz_float_t _r, dz_float_t _g, dz_float_t _b, dz_float_t _a );
void dz_instance_get_color( const dz_instance_t * _instance, dz_float_t * const _r, dz_float_t * const _g, dz_float_t * const _b, dz_float_t * const _a );

void dz_instance_set_rotate( dz_instance_t * const _instance, dz_float_t _angle );
dz_float_t dz_instance_get_rotate( const dz_instance_t * _instance );

void dz_instance_reset( dz_instance_t * const _instance );
void dz_instance_restart( dz_instance_t * const _instance );
void dz_instance_pause( dz_instance_t * const _instance );
void dz_instance_resume( dz_instance_t * const _instance );
dz_bool_t dz_instance_is_paused( const dz_instance_t * _instance );
void dz_instance_stop( dz_instance_t * const _instance );
dz_result_t dz_instance_seek( const dz_service_t * _service, dz_instance_t * const _instance, dz_float_t _time, dz_float_t _step );

void dz_instance_emit_pause( dz_instance_t * const _instance );
void dz_instance_emit_resume( dz_instance_t * const _instance );
dz_bool_t dz_instance_is_emit_pause( const dz_instance_t * _instance );

dz_result_t dz_instance_update( const dz_service_t * _service, dz_instance_t * const _instance, dz_float_t _time );

typedef enum dz_instance_state_e
{
    DZ_INSTANCE_PROCESS = 0x00000001,
    DZ_INSTANCE_EMIT_COMPLETE = 0x00000002,
    DZ_INSTANCE_PARTICLE_COMPLETE = 0x00000004,
} dz_instance_state_e;

dz_instance_state_e dz_instance_get_state( const dz_instance_t * _instance );

dz_uint16_t dz_instance_get_particle_count( const dz_instance_t * _instance );
typedef struct dz_particle_state_t
{
    dz_vec3_t position;
    dz_vec3_t previous_position;
    dz_vec3_t velocity;
    dz_float_t age;
    dz_float_t life;
    dz_uint32_t layer_index;
    dz_uint32_t birth_order;
} dz_particle_state_t;
void dz_instance_get_particle_state( const dz_instance_t * _instance, dz_uint16_t _index, dz_particle_state_t * _state );
void dz_instance_get_aabb( const dz_instance_t * _instance, dz_aabb_t * _aabb );

typedef enum dz_index_type_e
{
    DZ_INDEX_UINT16 = 0,
    DZ_INDEX_UINT32 = 1
} dz_index_type_e;

typedef enum dz_primitive_type_e
{
    DZ_PRIMITIVE_TRIANGLES = 0,
    DZ_PRIMITIVE_LINES = 1
} dz_primitive_type_e;

typedef enum dz_vertex_semantic_flags_e
{
    DZ_VERTEX_SEMANTIC_POSITION = 0x00000001,
    DZ_VERTEX_SEMANTIC_NORMAL = 0x00000002,
    DZ_VERTEX_SEMANTIC_TANGENT = 0x00000004,
    DZ_VERTEX_SEMANTIC_COLOR = 0x00000008,
    DZ_VERTEX_SEMANTIC_UV0 = 0x00000010,
    DZ_VERTEX_SEMANTIC_UV1 = 0x00000020
} dz_vertex_semantic_flags_e;

typedef struct dz_render_requirements_t
{
    dz_uint32_t vertex_count;
    dz_uint32_t index_count;
    dz_uint32_t chunk_count;
    dz_uint32_t vertex_semantics;
    dz_index_type_e index_type;
} dz_render_requirements_t;

typedef struct dz_render_stream_t
{
    void * buffer;
    dz_size_t size;
    dz_size_t offset;
    dz_size_t stride;
} dz_render_stream_t;

typedef struct dz_render_buffers_t
{
    dz_render_stream_t positions;
    dz_render_stream_t normals;
    dz_render_stream_t tangents;
    dz_render_stream_t colors;
    dz_render_stream_t uv0;
    dz_render_stream_t uv1;
    void * indices;
    dz_size_t indices_size;
    dz_index_type_e index_type;
} dz_render_buffers_t;

typedef enum dz_depth_compare_e
{
    DZ_DEPTH_ALWAYS = 0,
    DZ_DEPTH_LESS = 1,
    DZ_DEPTH_LESS_EQUAL = 2
} dz_depth_compare_e;

typedef enum dz_cull_mode_e
{
    DZ_CULL_NONE = 0,
    DZ_CULL_BACK = 1,
    DZ_CULL_FRONT = 2
} dz_cull_mode_e;

#define DZ_TECHNIQUE_ID_MAX 64
#define DZ_MATERIAL_PASS_MAX 8
#define DZ_UNIFORM_NAME_MAX 48
#define DZ_MATERIAL_UNIFORM_MAX 16
#define DZ_MATERIAL_TEXTURE_BINDING_MAX 8

typedef enum dz_uniform_semantic_e
{
    DZ_UNIFORM_CUSTOM = 0,
    DZ_UNIFORM_VIEW = 1,
    DZ_UNIFORM_PROJECTION = 2,
    DZ_UNIFORM_VIEW_PROJECTION = 3,
    DZ_UNIFORM_CAMERA_POSITION = 4,
    DZ_UNIFORM_INSTANCE_TRANSFORM = 5,
    DZ_UNIFORM_TIME = 6,

    __DZ_UNIFORM_SEMANTIC_MAX__
} dz_uniform_semantic_e;

typedef struct dz_uniform_desc_t
{
    char name[DZ_UNIFORM_NAME_MAX];
    dz_uniform_semantic_e semantic;
    dz_uint32_t value_count;
    dz_float_t values[16];
} dz_uniform_desc_t;

typedef enum dz_sampler_filter_e
{
    DZ_SAMPLER_NEAREST = 0,
    DZ_SAMPLER_LINEAR = 1
} dz_sampler_filter_e;

typedef enum dz_sampler_wrap_e
{
    DZ_SAMPLER_CLAMP = 0,
    DZ_SAMPLER_REPEAT = 1,
    DZ_SAMPLER_MIRRORED_REPEAT = 2
} dz_sampler_wrap_e;

typedef struct dz_texture_binding_desc_t
{
    char uniform_name[DZ_UNIFORM_NAME_MAX];
    dz_uint32_t texture_slot;
    dz_sampler_filter_e min_filter;
    dz_sampler_filter_e mag_filter;
    dz_sampler_wrap_e wrap_u;
    dz_sampler_wrap_e wrap_v;
} dz_texture_binding_desc_t;

typedef struct dz_material_pass_desc_t
{
    char technique_id[DZ_TECHNIQUE_ID_MAX];
    dz_blend_type_e blend;
    dz_bool_t depth_test;
    dz_bool_t depth_write;
    dz_depth_compare_e depth_compare;
    dz_cull_mode_e cull;
    dz_uint8_t color_mask;
    dz_uint32_t uniform_count;
    dz_uniform_desc_t uniforms[DZ_MATERIAL_UNIFORM_MAX];
    dz_uint32_t texture_binding_count;
    dz_texture_binding_desc_t texture_bindings[DZ_MATERIAL_TEXTURE_BINDING_MAX];
} dz_material_pass_desc_t;

dz_uint32_t dz_material_get_pass_count( const dz_material_t * _material );
void dz_material_add_pass( dz_material_t * _material, const dz_material_pass_desc_t * _pass, dz_uint32_t * _index );
void dz_material_remove_pass( dz_material_t * _material, dz_uint32_t _index );
void dz_material_set_pass( dz_material_t * _material, dz_uint32_t _index, const dz_material_pass_desc_t * _pass );
void dz_material_get_pass( const dz_material_t * _material, dz_uint32_t _index, dz_material_pass_desc_t * _pass );

typedef struct dz_render_chunk_t
{
    dz_uint32_t vertex_offset;
    dz_uint32_t vertex_count;
    dz_uint32_t index_offset;
    dz_uint32_t index_count;
    dz_uint32_t material_pass;
    dz_float_t sort_key;
    dz_primitive_type_e primitive;
    dz_material_pass_desc_t pass;
    dz_userdata_t surface;
} dz_render_chunk_t;

void dz_instance_prepare_render( const dz_instance_t * _instance, const dz_camera_state_t * _camera, dz_render_requirements_t * _requirements );
dz_result_t dz_instance_fill_render( const dz_instance_t * _instance, const dz_camera_state_t * _camera, const dz_render_buffers_t * _buffers, dz_render_chunk_t * _chunks,
                                     dz_uint32_t _chunk_capacity, dz_uint32_t * _chunk_count );

typedef enum dz_physics_object_type_e
{
    DZ_PHYSICS_GRAVITY = 0,
    DZ_PHYSICS_WIND = 1,
    DZ_PHYSICS_MAGNET = 2,
    DZ_PHYSICS_PLANE = 3,
    DZ_PHYSICS_SPHERE = 4,
    DZ_PHYSICS_BOX = 5,
    DZ_PHYSICS_MESH = 6,

    __DZ_PHYSICS_OBJECT_MAX__
} dz_physics_object_type_e;

typedef enum dz_collision_response_e
{
    DZ_COLLISION_BOUNCE = 0,
    DZ_COLLISION_SLIDE = 1,
    DZ_COLLISION_KILL = 2
} dz_collision_response_e;

typedef struct dz_physics_object_desc_t
{
    dz_uint32_t id;
    dz_uint32_t mesh_id;
    dz_physics_object_type_e type;
    dz_transform_t transform;
    dz_vec3_t direction;
    dz_vec3_t half_extents;
    dz_float_t radius;
    dz_float_t strength;
    dz_float_t falloff;
    dz_float_t turbulence;
    dz_float_t restitution;
    dz_float_t friction;
    dz_collision_response_e response;
} dz_physics_object_desc_t;

#define DZ_EFFECT_PHYSICS_OBJECT_MAX 64

dz_uint32_t dz_effect_get_physics_object_count( const dz_effect_t * _effect );
void dz_effect_add_physics_object( dz_effect_t * _effect, const dz_physics_object_desc_t * _object, dz_uint32_t * _index );
void dz_effect_set_physics_object( dz_effect_t * _effect, dz_uint32_t _index, const dz_physics_object_desc_t * _object );
void dz_effect_remove_physics_object( dz_effect_t * _effect, dz_uint32_t _index );
void dz_effect_get_physics_object( const dz_effect_t * _effect, dz_uint32_t _index, dz_physics_object_desc_t * _object );
dz_result_t dz_instance_set_physics_transform( dz_instance_t * _instance, dz_uint32_t _id, const dz_transform_t * _transform );

#endif
