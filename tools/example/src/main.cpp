#include "dazzle/dazzle.hpp"
#include "render/render.hpp"

#include "glad/gl.h"
#include "GLFW/glfw3.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

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
    if( dz_timeline_interpolate_create( _service, &interpolate0, DZ_TIMELINE_INTERPOLATE_LINEAR, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

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
    if( dz_timeline_interpolate_create( _service, &interpolate1, DZ_TIMELINE_INTERPOLATE_LINEAR, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

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
    DZ_UNUSED( argc );
    DZ_UNUSED( argv );

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

    GLFWwindow * fwWindow = glfwCreateWindow( window_width, window_height, "graphics", 0, 0 );

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

    dz_render_use_texture_program( &opengl_desc );

    dz_render_set_proj( &opengl_desc, -(dz_float_t)window_width * 0.5f, (dz_float_t)window_width * 0.5f, -(dz_float_t)window_height * 0.5f, (dz_float_t)window_height * 0.5f );

    char texture_path[250];
    sprintf( texture_path, "%s/%s"
        , DAZZLE_EXAMPLE_CONTENT_DIR
        , "particle.png"
    );

    int width = 0;
    int height = 0;

    GLuint textureId = dz_render_make_texture( texture_path, &width, &height );

    dz_service_providers_t providers;
    providers.f_malloc = &dz_malloc;
    providers.f_realloc = &dz_realloc;
    providers.f_free = &dz_free;
    providers.f_sqrtf = &dz_sqrtf;
    providers.f_cosf = &dz_cosf;
    providers.f_sinf = &dz_sinf;

    dz_service_t * service;
    if( dz_service_create( &service, &providers, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return EXIT_FAILURE;
    }

    dz_atlas_t * atlas;
    if( dz_atlas_create( service, &atlas, &textureId, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return EXIT_FAILURE;
    }

    dz_texture_t * texture;
    if( dz_texture_create( service, &texture, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return EXIT_FAILURE;
    }

    dz_atlas_add_texture( atlas, texture );

    dz_material_t * material;
    if( dz_material_create( service, &material, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return EXIT_FAILURE;
    }

    dz_material_set_blend( material, DZ_BLEND_ADD );
    dz_material_set_atlas( material, atlas );

    dz_shape_t * shape;
    if( dz_shape_create( service, &shape, DZ_SHAPE_POINT, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return EXIT_FAILURE;
    }

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
    if( dz_emitter_create( service, &emitter, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return EXIT_FAILURE;
    }

    dz_emitter_set_life( emitter, 1000.f );

    __set_emitter_timeline_const( service, emitter, DZ_EMITTER_SPAWN_DELAY, 0.1f );
    __set_emitter_timeline_const( service, emitter, DZ_EMITTER_SPAWN_COUNT, 3.f );

    dz_affector_t * affector;
    if( dz_affector_create( service, &affector, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return EXIT_FAILURE;
    }

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
        {DZ_AFFECTOR_TIMELINE_LIFE, 0.5f, 1.f, 3.f, 5.f, 2.f},

        {DZ_AFFECTOR_TIMELINE_MOVE_SPEED, 0.5f, 1.f, 10.f, 50.f, 100.f},
        {DZ_AFFECTOR_TIMELINE_MOVE_ACCELERATE, 0.5f, 1.f, 0.f, 0.f, 0.f},
        {DZ_AFFECTOR_TIMELINE_ROTATE_SPEED, 0.5f, 1.f, 0.f, 0.1f, 0.f},
        {DZ_AFFECTOR_TIMELINE_ROTATE_ACCELERATE, 0.5f, 1.f, 0.f, 0.f, 0.f},
        {DZ_AFFECTOR_TIMELINE_SPIN_SPEED, 0.5f, 1.f, 0.01f, 0.1f, 0.f},
        {DZ_AFFECTOR_TIMELINE_SPIN_ACCELERATE, 0.5f, 1.f, 0.001f, 0.f, 0.f},
        {DZ_AFFECTOR_TIMELINE_STRAFE_SPEED, 0.5f, 1.f, 0.f, 0.f, 0.f},
        {DZ_AFFECTOR_TIMELINE_STRAFE_FRENQUENCE, 0.5f, 1.f, 0.f, 0.f, 0.f},
        {DZ_AFFECTOR_TIMELINE_STRAFE_SIZE, 0.5f, 1.f, 50.f, 100.f, 0.f},
        {DZ_AFFECTOR_TIMELINE_STRAFE_SHIFT, 0.5f, 1.f, 0.f, 0.f, 0.f},
        {DZ_AFFECTOR_TIMELINE_SCALE, 0.5f, 1.f, 1.f, 2.5f, 10.f},
        {DZ_AFFECTOR_TIMELINE_ASPECT, 0.f, 1.f, 1.f, 2.f, 10.f},
        {DZ_AFFECTOR_TIMELINE_COLOR_R, 0.5f, 1.f, 0.75f, 0.25f, 0.4f},
        {DZ_AFFECTOR_TIMELINE_COLOR_G, 0.5f, 1.f, 0.5f, 0.1f, 0.4f},
        {DZ_AFFECTOR_TIMELINE_COLOR_B, 0.5f, 1.f, 0.25f, 0.9f, 0.4f},
        {DZ_AFFECTOR_TIMELINE_COLOR_A, 0.05f, 1.f, 0.f, 1.f, 0.f},
    };

    for( dz_uint32_t index = 0; index != __DZ_AFFECTOR_TIMELINE_MAX__; ++index )
    {
        timeline_t data = timeline_datas[index];

        if( __set_affector_timeline_linear2( service, affector, data.type, data.time0, data.time1, data.value0, data.value1, data.value2 ) == DZ_FAILURE )
        {
            return EXIT_FAILURE;
        }
    }

    dz_effect_t * effect;
    if( dz_effect_create( service, &effect, material, shape, emitter, affector, 5.f, 0, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return EXIT_FAILURE;
    }

    dz_instance_t * instnace;
    if( dz_instance_create( service, &instnace, effect, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return EXIT_FAILURE;
    }

    while( glfwWindowShouldClose( fwWindow ) == 0 )
    {
        glfwPollEvents();

        dz_instance_update( service, instnace, 0.005f );

        dz_render_set_camera( &opengl_desc, camera_offset_x, camera_offset_y, camera_scale );

        glClearColor( 0, 0, 0, 255 );
        glClear( GL_COLOR_BUFFER_BIT );

        dz_render_instance( &opengl_desc, instnace );

        glfwSwapBuffers( fwWindow );
    }

    dz_effect_destroy( service, effect );
    dz_emitter_destroy( service, emitter );
    dz_affector_destroy( service, affector );
    dz_shape_destroy( service, shape );
    dz_service_destroy( service );

    return EXIT_SUCCESS;
}