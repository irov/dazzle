#include "dazzle/dazzle.hpp"
#include "render/render.hpp"

#include "glad/gl.h"
#include "GLFW/glfw3.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////
static void * dz_malloc( dz_size_t _size, dz_userdata_t _ud )
{
    DZ_UNUSED( _ud );

    void * p = malloc( _size );

    return p;
}
//////////////////////////////////////////////////////////////////////////
static void * dz_realloc( void * const _ptr, dz_size_t _size, dz_userdata_t _ud )
{
    DZ_UNUSED( _ud );

    void * p = realloc( _ptr, _size );

    return p;
}
//////////////////////////////////////////////////////////////////////////
static void dz_free( const void * _ptr, dz_userdata_t _ud )
{
    DZ_UNUSED( _ud );

    free( (void *)_ptr );
}
//////////////////////////////////////////////////////////////////////////
static dz_float_t dz_sqrtf( dz_float_t _a, dz_userdata_t _ud )
{
    DZ_UNUSED( _ud );

    dz_float_t value = sqrtf( _a );

    return value;
}
//////////////////////////////////////////////////////////////////////////
static dz_float_t dz_cosf( dz_float_t _a, dz_userdata_t _ud )
{
    DZ_UNUSED( _ud );

    dz_float_t value = cosf( _a );

    return value;
}
//////////////////////////////////////////////////////////////////////////
static dz_float_t dz_sinf( dz_float_t _a, dz_userdata_t _ud )
{
    DZ_UNUSED( _ud );

    dz_float_t value = sinf( _a );

    return value;
}
////////////////////////////////////////////////////////////////////////////
//static dz_result_t __set_shape_timeline_const( dz_service_t * _service, dz_shape_t * _shape, dz_shape_timeline_type_e _type, float _value )
//{
//    dz_timeline_key_t * timeline;
//    if( dz_timeline_key_create( _service, &timeline, 0.f, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR ) == DZ_FAILURE )
//    {
//        return DZ_FAILURE;
//    }
//
//    if( dz_timeline_key_const_set_value( timeline, _value ) == DZ_FAILURE )
//    {
//        return DZ_FAILURE;
//    }
//
//    dz_shape_set_timeline( _shape, _type, timeline );
//
//    return DZ_SUCCESSFUL;
//}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __set_emitter_timeline_const( dz_service_t * _service, dz_emitter_t * _emitter, dz_emitter_timeline_type_e _type, dz_float_t _value )
{
    dz_timeline_key_t * timeline;
    if( dz_timeline_key_create( _service, &timeline, 0.f, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    dz_timeline_key_set_const_value( timeline, _value );

    dz_emitter_set_timeline( _emitter, _type, timeline );

    return DZ_SUCCESSFUL;
}
////////////////////////////////////////////////////////////////////////////
//static dz_result_t __set_affector_timeline_const( dz_service_t * _service, dz_affector_t * _affector, dz_affector_timeline_type_e _type, float _value )
//{
//    dz_timeline_key_t * timeline;
//    if( dz_timeline_key_create( _service, &timeline, 0.f, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR ) == DZ_FAILURE )
//    {
//        return DZ_FAILURE;
//    }
//
//    if( dz_timeline_key_const_set_value( timeline, _value ) == DZ_FAILURE )
//    {
//        return DZ_FAILURE;
//    }
//
//    dz_affector_set_timeline( _affector, _type, timeline );
//
//    return DZ_SUCCESSFUL;
//}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __set_affector_timeline_linear2( dz_service_t * _service, dz_affector_t * _affector, dz_affector_timeline_type_e _type, dz_float_t _time0, dz_float_t _time1, dz_float_t _value0, dz_float_t _value1, dz_float_t _value2 )
{
    dz_timeline_key_t * key0;
    if( dz_timeline_key_create( _service, &key0, 0.f, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    dz_timeline_key_set_const_value( key0, _value0 );

    dz_timeline_interpolate_t * interpolate0;
    dz_timeline_interpolate_create( _service, &interpolate0, DZ_TIMELINE_INTERPOLATE_LINEAR, DZ_NULLPTR );

    dz_timeline_key_t * key1;
    if( dz_timeline_key_create( _service, &key1, _time0, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    dz_timeline_key_set_const_value( key1, _value1 );

    dz_timeline_interpolate_set_key( interpolate0, key1 );

    if( dz_timeline_key_set_interpolate( key0, interpolate0 ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    dz_timeline_interpolate_t * interpolate1;
    dz_timeline_interpolate_create( _service, &interpolate1, DZ_TIMELINE_INTERPOLATE_LINEAR, DZ_NULLPTR );

    dz_timeline_key_t * key2;
    if( dz_timeline_key_create( _service, &key2, _time1, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    dz_timeline_key_set_const_value( key2, _value2 );

    dz_timeline_interpolate_set_key( interpolate1, key2 );

    if( dz_timeline_key_set_interpolate( key1, interpolate1 ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    dz_affector_set_timeline( _affector, _type, key0 );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
float camera_scale = 1.f;
float camera_scale_min = 0.125f;
float camera_scale_max = 16.f;
float camera_scale_step = 0.125f;
float camera_offset_x = 0.f;
float camera_offset_y = 0.f;
//////////////////////////////////////////////////////////////////////////
float mouse_pos_x = 0.f;
float mouse_pos_y = 0.f;
//////////////////////////////////////////////////////////////////////////
static void __glfw_framebufferSizeCallback( GLFWwindow * _window, int _width, int _height )
{
    DZ_UNUSED( _window );

    glViewport( 0, 0, _width, _height );
}
//////////////////////////////////////////////////////////////////////////
static void __glfw_scrollCallback( GLFWwindow * _window, double _x, double _y )
{
    DZ_UNUSED( _window );
    DZ_UNUSED( _x );

    camera_offset_x -= mouse_pos_x / camera_scale;
    camera_offset_y -= mouse_pos_y / camera_scale;

    float scroll = (float)_y * camera_scale_step;

    if( camera_scale + scroll > camera_scale_max )
    {
        camera_scale = camera_scale_max;
    }
    else if( camera_scale + scroll < camera_scale_min )
    {
        camera_scale = camera_scale_min;
    }
    else
    {
        camera_scale += scroll;
    }

    camera_offset_x += mouse_pos_x / camera_scale;
    camera_offset_y += mouse_pos_y / camera_scale;
}
//////////////////////////////////////////////////////////////////////////
static void __glfw_cursorPosCallback( GLFWwindow * _window, double _x, double _y )
{
    if( glfwGetKey( _window, GLFW_KEY_SPACE ) == GLFW_PRESS &&
        glfwGetMouseButton( _window, GLFW_MOUSE_BUTTON_LEFT ) == GLFW_PRESS )
    {
        const float dx = (float)_x - mouse_pos_x;
        const float dy = (float)_y - mouse_pos_y;

        camera_offset_x += dx / camera_scale;
        camera_offset_y += dy / camera_scale;
    }

    mouse_pos_x = (float)_x;
    mouse_pos_y = (float)_y;
}
//////////////////////////////////////////////////////////////////////////
int main( int argc, char ** argv )
{
    const dz_projection_type_e projection = argc > 1 && strcmp( argv[1], "perspective" ) == 0 ? DZ_PROJECTION_PERSPECTIVE : DZ_PROJECTION_ORTHOGRAPHIC;

    if( glfwInit() == 0 )
    {
        return EXIT_FAILURE;
    }

    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );

    int window_width = 1024;
    int window_height = 768;

    //camera_offset_x = window_width * 0.5f;
    //camera_offset_y = window_height * 0.5f;

    GLFWwindow * fwWindow = glfwCreateWindow( window_width, window_height, projection == DZ_PROJECTION_PERSPECTIVE ? "Dazzle - Perspective" : "Dazzle - Orthographic", 0, 0 );

    if( fwWindow == 0 )
    {
        glfwTerminate();

        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent( fwWindow );
    glfwSetFramebufferSizeCallback( fwWindow, &__glfw_framebufferSizeCallback );
    glfwSetScrollCallback( fwWindow, &__glfw_scrollCallback );
    glfwSetCursorPosCallback( fwWindow, &__glfw_cursorPosCallback );

    double cursorPosX;
    double cursorPosY;
    glfwGetCursorPos( fwWindow, &cursorPosX, &cursorPosY );

    mouse_pos_x = (float)cursorPosX;
    mouse_pos_y = (float)cursorPosY;

    if( gladLoadGL( (GLADloadfunc)&glfwGetProcAddress ) == 0 )
    {
        return EXIT_FAILURE;
    }

    glfwSwapInterval( 1 );

    dz_uint16_t max_vertex_count = 65535;
    dz_uint16_t max_index_count = 65535;

    dz_render_desc_t opengl_desc;
    if( dz_render_initialize( &opengl_desc, max_vertex_count, max_index_count ) == DZ_FAILURE )
    {
        return EXIT_FAILURE;
    }

    static const char * demoVertexShader =
        "#version 330 core\n"
        "layout(location=0) in vec3 inPos;\n"
        "layout(location=3) in vec4 inColor;\n"
        "uniform mat4 uViewProjection;\n"
        "out vec4 vColor;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = uViewProjection * vec4(inPos, 1.0);\n"
        "    vColor = inColor;\n"
        "}\n";
    static const char * demoFragmentShader =
        "#version 330 core\n"
        "in vec4 vColor;\n"
        "uniform float uTime;\n"
        "out vec4 oColor;\n"
        "void main()\n"
        "{\n"
        "    float pulse = 0.35 + 0.25 * sin(uTime * 3.0);\n"
        "    oColor = vec4(vColor.rgb * pulse, vColor.a * pulse);\n"
        "}\n";
    if( dz_render_register_technique( &opengl_desc, "demo.glow", demoVertexShader, demoFragmentShader ) != DZ_SUCCESSFUL )
    {
        return EXIT_FAILURE;
    }

    dz_render_use_texture_program( &opengl_desc );

    dz_render_set_proj( &opengl_desc, -(dz_float_t)window_width * 0.5f, (dz_float_t)window_width * 0.5f, -(dz_float_t)window_height * 0.5f, (dz_float_t)window_height * 0.5f );

    const dz_uint8_t whitePixel[4] = { 255U, 255U, 255U, 255U };
    GLuint textureId = 0;
    GLCALL( glGenTextures, ( 1, &textureId ) );
    GLCALL( glBindTexture, ( GL_TEXTURE_2D, textureId ) );
    GLCALL( glTexImage2D, ( GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel ) );
    GLCALL( glTexParameteri, ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ) );
    GLCALL( glTexParameteri, ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ) );
    GLCALL( glBindTexture, ( GL_TEXTURE_2D, 0 ) );
    const int width = 32;
    const int height = 32;

    dz_service_providers_t providers;
    providers.f_malloc = &dz_malloc;
    providers.f_realloc = &dz_realloc;
    providers.f_free = &dz_free;
    providers.f_sqrtf = &dz_sqrtf;
    providers.f_cosf = &dz_cosf;
    providers.f_sinf = &dz_sinf;

    dz_service_t * service;
    dz_service_create( &service, &providers, DZ_NULLPTR );

    dz_atlas_t * atlas;
    dz_atlas_create( service, &atlas, &textureId, DZ_NULLPTR );

    dz_texture_t * texture;
    dz_texture_create( service, &texture, DZ_NULLPTR );

    dz_float_t u[4] = {0.f, 1.f, 1.f, 0.f};
    dz_float_t v[4] = {0.f, 0.f, 1.f, 1.f};

    dz_texture_set_uv( texture, u, v );
    dz_texture_set_width( texture, (dz_float_t)width );
    dz_texture_set_height( texture, (dz_float_t)height );
    dz_texture_set_trim_offset( texture, 0.f, 0.f );
    dz_texture_set_trim_size( texture, (dz_float_t)width, (dz_float_t)height );

    dz_material_t * material;
    dz_material_create( service, &material, DZ_NULLPTR );

    dz_material_set_blend( material, DZ_BLEND_ADD );
    dz_material_set_mode( material, DZ_MATERIAL_MODE_TEXTURE );
    dz_material_set_atlas( material, atlas );
    dz_material_set_texture_index( material, 0 );
    dz_material_set_texture_count( material, 1 );

    if( dz_material_add_texture( material, texture ) == DZ_FAILURE )
    {
        return EXIT_FAILURE;
    }

    dz_material_pass_desc_t glowPass = {};
    snprintf( glowPass.technique_id, sizeof( glowPass.technique_id ), "%s", "demo.glow" );
    glowPass.blend = DZ_BLEND_ADD;
    glowPass.depth_compare = DZ_DEPTH_LESS_EQUAL;
    glowPass.color_mask = 0x0fU;
    glowPass.uniform_count = 1U;
    snprintf( glowPass.uniforms[0].name, sizeof( glowPass.uniforms[0].name ), "%s", "uTime" );
    glowPass.uniforms[0].semantic = DZ_UNIFORM_TIME;
    dz_material_add_pass( material, &glowPass, DZ_NULLPTR );

    dz_shape_t * shape;
    dz_shape_create( service, &shape, DZ_SHAPE_POINT, DZ_NULLPTR );

    //__set_shape_timeline_const( service, shape, DZ_SHAPE_RECT_WIDTH_MAX, 300.f );
    //__set_shape_timeline_const( service, shape, DZ_SHAPE_RECT_HEIGHT_MAX, 200.f );

    //float triangles[] = {-100.f, -100.f, 100.f, 0.f, 100.f, 100.f, 300.f, 200.f, 400.f, 200.f, 400.f, 500.f};
    //dz_shape_set_polygon( shape, triangles, 2 );

    dz_uint8_t mask[] = {
        0, 0, 0, 0, 0,
        0, 1, 1, 1, 0,
        1, 0, 0, 0, 1,
        0, 1, 1, 1, 0,
        0, 0, 0, 0, 0
    };

    dz_shape_set_mask( shape, mask, 1, 5, 5, 5 );

    dz_shape_set_mask_scale( shape, 50.f );

    dz_emitter_t * emitter;
    dz_emitter_create( service, &emitter, DZ_NULLPTR );

    dz_emitter_set_life( emitter, 1000.f );

    __set_emitter_timeline_const( service, emitter, DZ_EMITTER_SPAWN_DELAY, 0.1f );
    __set_emitter_timeline_const( service, emitter, DZ_EMITTER_SPAWN_COUNT, 3.f );

    dz_affector_t * affector;
    dz_affector_create( service, &affector, DZ_NULLPTR );

    typedef struct timeline_t
    {
        dz_affector_timeline_type_e type;
        dz_float_t time0;
        dz_float_t time1;
        dz_float_t value0;
        dz_float_t value1;
        dz_float_t value2;
    } timeline_t;

    timeline_t timeline_datas[] = {
        { DZ_AFFECTOR_TIMELINE_LIFE, 0.5f, 1.f, 3.f, 5.f, 2.f },

        { DZ_AFFECTOR_TIMELINE_MOVE_SPEED, 0.5f, 1.f, 10.f, 50.f, 100.f },
        { DZ_AFFECTOR_TIMELINE_MOVE_ACCELERATE, 0.5f, 1.f, 0.f, 0.f, 0.f },
        { DZ_AFFECTOR_TIMELINE_ROTATE_SPEED, 0.5f, 1.f, 0.f, 0.1f, 0.f },
        { DZ_AFFECTOR_TIMELINE_ROTATE_ACCELERATE, 0.5f, 1.f, 0.f, 0.f, 0.f },
        { DZ_AFFECTOR_TIMELINE_SPIN_SPEED, 0.5f, 1.f, 0.01f, 0.1f, 0.f },
        { DZ_AFFECTOR_TIMELINE_SPIN_ACCELERATE, 0.5f, 1.f, 0.001f, 0.f, 0.f },
        { DZ_AFFECTOR_TIMELINE_STRAFE_SPEED, 0.5f, 1.f, 0.f, 0.f, 0.f },
        { DZ_AFFECTOR_TIMELINE_STRAFE_FRENQUENCE, 0.5f, 1.f, 0.f, 0.f, 0.f },
        { DZ_AFFECTOR_TIMELINE_STRAFE_SIZE, 0.5f, 1.f, 50.f, 100.f, 0.f },
        { DZ_AFFECTOR_TIMELINE_STRAFE_SHIFT, 0.5f, 1.f, 0.f, 0.f, 0.f },
        { DZ_AFFECTOR_TIMELINE_SCALE, 0.5f, 1.f, 1.f, 2.5f, 10.f },
        { DZ_AFFECTOR_TIMELINE_ASPECT, 0.f, 1.f, 1.f, 2.f, 10.f },
        { DZ_AFFECTOR_TIMELINE_COLOR_R, 0.5f, 1.f, 0.75f, 0.25f, 0.4f },
        { DZ_AFFECTOR_TIMELINE_COLOR_G, 0.5f, 1.f, 0.5f, 0.1f, 0.4f },
        { DZ_AFFECTOR_TIMELINE_COLOR_B, 0.5f, 1.f, 0.25f, 0.9f, 0.4f },
        { DZ_AFFECTOR_TIMELINE_COLOR_A, 0.05f, 1.f, 0.f, 1.f, 0.f },
        { DZ_AFFECTOR_TIMELINE_DIRECTION_Z, 0.f, 1.f, 0.15f, 0.35f, -0.1f },
        { DZ_AFFECTOR_TIMELINE_GRAVITY_X, 0.f, 1.f, 0.f, 0.f, 0.f },
        { DZ_AFFECTOR_TIMELINE_GRAVITY_Y, 0.f, 1.f, -4.f, -4.f, -4.f },
        { DZ_AFFECTOR_TIMELINE_GRAVITY_Z, 0.f, 1.f, 0.f, 0.f, 0.f },
        { DZ_AFFECTOR_TIMELINE_DRAG, 0.f, 1.f, 0.05f, 0.1f, 0.2f },
    };
    static_assert( sizeof( timeline_datas ) / sizeof( timeline_datas[0] ) == __DZ_AFFECTOR_TIMELINE_MAX__, "affector demo timelines must cover every channel" );

    for( dz_uint32_t index = 0; index != __DZ_AFFECTOR_TIMELINE_MAX__; ++index )
    {
        timeline_t data = timeline_datas[index];

        if( __set_affector_timeline_linear2( service, affector, data.type, data.time0, data.time1, data.value0, data.value1, data.value2 ) == DZ_FAILURE )
        {
            return EXIT_FAILURE;
        }
    }

    dz_project_profile_t projectProfile;
    dz_project_profile_default( &projectProfile, projection );
    projectProfile.far_plane = 5000.f;
    if( projection == DZ_PROJECTION_ORTHOGRAPHIC )
    {
        projectProfile.orthographic_height = (dz_float_t)window_height;
    }
    else
    {
        projectProfile.position.z = 700.f;
    }

    dz_effect_t * effect;
    dz_effect_create_with_profile( service, &effect, &projectProfile, 5.f, 0, DZ_NULLPTR );

    dz_effect_set_atlas( effect, atlas );

    const dz_mesh_vertex_t demoMeshVertices[4] = { { { -12.f, -12.f, 0.f }, { 0.f, 0.f, -1.f }, { 1.f, 0.f, 0.f, 1.f }, { 0.f, 0.f }, { 0.f, 0.f } },
                                                   { { 12.f, -12.f, 0.f }, { 0.f, 0.f, -1.f }, { 1.f, 0.f, 0.f, 1.f }, { 1.f, 0.f }, { 1.f, 0.f } },
                                                   { { 0.f, 16.f, 0.f }, { 0.f, 0.f, -1.f }, { 1.f, 0.f, 0.f, 1.f }, { 0.5f, 1.f }, { 0.5f, 1.f } },
                                                   { { 0.f, 0.f, 24.f }, { 0.f, 1.f, 0.f }, { 1.f, 0.f, 0.f, 1.f }, { 0.5f, 0.5f }, { 0.5f, 0.5f } } };
    const dz_uint32_t demoMeshIndices[12] = { 0U, 2U, 1U, 0U, 1U, 3U, 1U, 2U, 3U, 2U, 0U, 3U };
    dz_mesh_desc_t demoMesh = {};
    demoMesh.id = 100U;
    demoMesh.vertices = demoMeshVertices;
    demoMesh.vertex_count = 4U;
    demoMesh.indices = demoMeshIndices;
    demoMesh.index_count = 12U;
    dz_effect_add_mesh( service, effect, &demoMesh );

    for( dz_uint32_t mode = 0; mode != __DZ_PARTICLE_MODE_MAX__; ++mode )
    {
        dz_effect_layer_desc_t layer;
        dz_effect_layer_desc_default( &layer );
        layer.material = material;
        layer.shape = shape;
        layer.emitter = emitter;
        layer.affector = affector;
        layer.x = ( (dz_float_t)mode - 2.f ) * 110.f;
        layer.z = projection == DZ_PROJECTION_PERSPECTIVE ? ( (dz_float_t)( mode % 2U ) - 0.5f ) * 80.f : 0.f;
        layer.life = 5.f;
        layer.seed = mode * 17U;
        layer.particle_mode = (dz_particle_mode_e)mode;
        layer.orientation = mode == DZ_PARTICLE_MODE_MESH ? DZ_PARTICLE_ORIENTATION_WORLD : DZ_PARTICLE_ORIENTATION_CAMERA;
        layer.sorting = DZ_PARTICLE_SORT_CAMERA_FAR;
        layer.mesh_id = mode == DZ_PARTICLE_MODE_MESH ? demoMesh.id : DZ_RESOURCE_ID_NONE;
        layer.trail_width = 12.f;
        layer.trail_lifetime = 0.4f;

        dz_uint32_t layer_index;
        if( dz_effect_add_layer( effect, &layer, &layer_index ) != DZ_SUCCESSFUL )
        {
            return EXIT_FAILURE;
        }

        dz_effect_trigger_desc_t trigger = {};
        trigger.event_type = DZ_EFFECT_EVENT_EFFECT_START;
        trigger.source_layer_index = DZ_EFFECT_LAYER_NONE;
        trigger.target_layer_index = layer_index;
        trigger.probability = 1.f;
        trigger.spawn_count_min = 1U;
        trigger.spawn_count_max = 1U;
        if( dz_effect_add_trigger( effect, &trigger, DZ_NULLPTR ) != DZ_SUCCESSFUL )
        {
            return EXIT_FAILURE;
        }
    }

    dz_physics_object_desc_t wind = {};
    wind.id = 1U;
    wind.type = DZ_PHYSICS_WIND;
    wind.transform.rotation.w = 1.f;
    wind.transform.scale = { 1.f, 1.f, 1.f };
    wind.direction = { 1.f, 0.f, 0.2f };
    wind.strength = 8.f;
    wind.turbulence = 4.f;
    wind.response = DZ_COLLISION_BOUNCE;
    dz_effect_add_physics_object( effect, &wind, DZ_NULLPTR );

    dz_physics_object_desc_t magnet = {};
    magnet.id = 2U;
    magnet.type = DZ_PHYSICS_MAGNET;
    magnet.transform.rotation.w = 1.f;
    magnet.transform.scale = { 1.f, 1.f, 1.f };
    magnet.transform.position = { 0.f, 80.f, 40.f };
    magnet.strength = 35.f;
    magnet.falloff = 0.02f;
    magnet.response = DZ_COLLISION_BOUNCE;
    dz_effect_add_physics_object( effect, &magnet, DZ_NULLPTR );

    dz_physics_object_desc_t plane = {};
    plane.id = 3U;
    plane.type = DZ_PHYSICS_PLANE;
    plane.transform.rotation.w = 1.f;
    plane.transform.scale = { 1.f, 1.f, 1.f };
    plane.transform.position.y = -260.f;
    plane.direction.y = 1.f;
    plane.restitution = 0.65f;
    plane.friction = 0.1f;
    plane.response = DZ_COLLISION_BOUNCE;
    dz_effect_add_physics_object( effect, &plane, DZ_NULLPTR );

    dz_instance_t * instnace;
    dz_instance_create( service, &instnace, effect, DZ_NULLPTR );

    while( glfwWindowShouldClose( fwWindow ) == 0 )
    {
        glfwPollEvents();

        dz_instance_update( service, instnace, 0.005f );

        glClearColor( 0, 0, 0, 255 );
        glClear( GL_COLOR_BUFFER_BIT );

        int framebufferWidth;
        int framebufferHeight;
        glfwGetFramebufferSize( fwWindow, &framebufferWidth, &framebufferHeight );
        dz_camera_state_t camera;
        dz_camera_state_from_profile( &projectProfile, (dz_float_t)framebufferWidth, (dz_float_t)framebufferHeight, &camera );
        camera.position.x -= camera_offset_x;
        camera.position.y += camera_offset_y;
        if( camera.projection == DZ_PROJECTION_ORTHOGRAPHIC )
        {
            camera.orthographic_height /= camera_scale;
        }
        dz_render_instance_camera( &opengl_desc, instnace, &camera );

        glfwSwapBuffers( fwWindow );
    }

    dz_instance_destroy( service, instnace );
    dz_effect_destroy( service, effect );
    dz_service_destroy( service );
    dz_render_delete_texture( textureId );
    dz_render_finalize( &opengl_desc );
    glfwDestroyWindow( fwWindow );
    glfwTerminate();

    return EXIT_SUCCESS;
}
