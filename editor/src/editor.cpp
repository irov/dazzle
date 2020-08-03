#include "editor.h"

#include "dazzle/dazzle.hpp"

#include "opengl.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <stdlib.h>
#include <stdio.h>

#include "curve.hpp"

#include <sstream>

//////////////////////////////////////////////////////////////////////////
typedef std::stringstream Stringstream;
//////////////////////////////////////////////////////////////////////////
static constexpr uint32_t MAX_POINTS = 100;
static constexpr float HEIGHT_TO_WIDTH_RATIO = 0.4f;
//////////////////////////////////////////////////////////////////////////
namespace ImGui
{
    int Curve( const char * label, const ImVec2 & size, int maxpoints, ImVec2 * points );
    float CurveValue( float p, int maxpoints, const ImVec2 * points );
};
//////////////////////////////////////////////////////////////////////////
ImVec4 clear_color = ImVec4( 0.45f, 0.55f, 0.60f, 1.00f );
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
static void * dz_malloc( dz_size_t _size, dz_userdata_t _ud )
{
    DZ_UNUSED( _ud );

    void * p = malloc( _size );

    return p;
}
//////////////////////////////////////////////////////////////////////////
static void * dz_realloc( void * _ptr, dz_size_t _size, dz_userdata_t _ud )
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
static float dz_cosf( float _a, dz_userdata_t _ud )
{
    DZ_UNUSED( _ud );

    float value = cosf( _a );

    return value;
}
//////////////////////////////////////////////////////////////////////////
static float dz_sinf( float _a, dz_userdata_t _ud )
{
    DZ_UNUSED( _ud );

    float value = sinf( _a );

    return value;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __set_shape_timeline_const( dz_service_t * _service, dz_shape_data_t * _shape_data, dz_shape_data_timeline_type_e _type, float _value )
{
    dz_timeline_key_t * timeline;
    if( dz_timeline_key_create( _service, &timeline, 0.f, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    if( dz_timeline_key_const_set_value( timeline, _value ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    dz_shape_data_set_timeline( _shape_data, _type, timeline );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __set_emitter_timeline_const( dz_service_t * _service, dz_emitter_data_t * _emitter_data, dz_emitter_data_timeline_type_e _type, float _value )
{
    dz_timeline_key_t * timeline;
    if( dz_timeline_key_create( _service, &timeline, 0.f, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    if( dz_timeline_key_const_set_value( timeline, _value ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    dz_emitter_data_set_timeline( _emitter_data, _type, timeline );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __set_affector_timeline_const( dz_service_t * _service, dz_affector_data_t * _affector_data, dz_affector_data_timeline_type_e _type, float _value )
{
    dz_timeline_key_t * timeline;
    if( dz_timeline_key_create( _service, &timeline, 0.f, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    if( dz_timeline_key_const_set_value( timeline, _value ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    dz_affector_data_set_timeline( _affector_data, _type, timeline );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __set_affector_timeline_linear( dz_service_t * _service, dz_affector_data_t * _affector_data, dz_affector_data_timeline_type_e _type, float _time, float _value0, float _value1 )
{
    dz_timeline_key_t * key0;
    if( dz_timeline_key_create( _service, &key0, 0.f, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    if( dz_timeline_key_const_set_value( key0, _value0 ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    dz_timeline_interpolate_t * interpolate;
    if( dz_timeline_interpolate_create( _service, &interpolate, DZ_TIMELINE_INTERPOLATE_LINEAR, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    dz_timeline_key_t * key1;
    if( dz_timeline_key_create( _service, &key1, _time, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    if( dz_timeline_key_const_set_value( key1, _value1 ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    dz_timeline_key_set_interpolate( key0, interpolate, key1 );

    dz_affector_data_set_timeline( _affector_data, _type, key0 );

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
static void glfw_framebufferSizeCallback( GLFWwindow * _window, int _width, int _height )
{
    DZ_UNUSED( _window );

    glViewport( 0, 0, _width, _height );
}
//////////////////////////////////////////////////////////////////////////
static void glfw_scrollCallback( GLFWwindow * _window, double _x, double _y )
{
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
static void glfw_cursorPosCallback( GLFWwindow * _window, double _x, double _y )
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
editor::editor()
{
}
//////////////////////////////////////////////////////////////////////////
editor::~editor()
{
}
//////////////////////////////////////////////////////////////////////////
int editor::run()
{
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

    camera_offset_x = window_width * 0.5f;
    camera_offset_y = window_height * 0.5f;

    GLFWwindow * fwWindow = glfwCreateWindow( window_width, window_height, "graphics", 0, 0 );

    if( fwWindow == 0 )
    {
        glfwTerminate();

        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent( fwWindow );
    glfwSetFramebufferSizeCallback( fwWindow, &glfw_framebufferSizeCallback );
    glfwSetScrollCallback( fwWindow, &glfw_scrollCallback );
    glfwSetCursorPosCallback( fwWindow, &glfw_cursorPosCallback );

    double cursorPosX;
    double cursorPosY;
    glfwGetCursorPos( fwWindow, &cursorPosX, &cursorPosY );

    mouse_pos_x = (float)cursorPosX;
    mouse_pos_y = (float)cursorPosY;

    if( gladLoadGLLoader( (GLADloadproc)glfwGetProcAddress ) == 0 )
    {
        return EXIT_FAILURE;
    }

    glfwSwapInterval( 1 );

    ImGuiContext * context = ImGui::CreateContext();
    (void)context;

    if( context == nullptr )
    {
        return EXIT_FAILURE;
    }

    if( ImGui_ImplGlfw_InitForOpenGL( fwWindow, true ) == false )
    {
        return EXIT_FAILURE;
    }

    const char * glsl_version = "#version 330";

    if( ImGui_ImplOpenGL3_Init( glsl_version ) == false )
    {
        return EXIT_FAILURE;
    }

    if( ImGui_ImplOpenGL3_CreateFontsTexture() == false )
    {
        return EXIT_FAILURE;
    }

    if( ImGui_ImplOpenGL3_CreateDeviceObjects() == false )
    {
        return EXIT_FAILURE;
    }

    dz_service_providers_t providers;
    providers.f_malloc = &dz_malloc;
    providers.f_realloc = &dz_realloc;
    providers.f_free = &dz_free;
    providers.f_cosf = &dz_cosf;
    providers.f_sinf = &dz_sinf;

    dz_service_t * service;
    if( dz_service_create( &service, &providers, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return EXIT_FAILURE;
    }

    dz_shape_data_t * shape_data;
    if( dz_shape_data_create( service, &shape_data, DZ_SHAPE_DATA_SEGMENT, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return EXIT_FAILURE;
    }

    __set_shape_timeline_const( service, shape_data, DZ_SHAPE_DATA_SEGMENT_ANGLE_MIN, -3.14159f * 0.25f );
    __set_shape_timeline_const( service, shape_data, DZ_SHAPE_DATA_SEGMENT_ANGLE_MAX, 3.14159f * 0.25f );

    dz_emitter_data_t * emitter_data;
    if( dz_emitter_data_create( service, &emitter_data, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return EXIT_FAILURE;
    }

    dz_emitter_data_set_life( emitter_data, 1000.f );

    __set_emitter_timeline_const( service, emitter_data, DZ_EMITTER_DATA_SPAWN_DELAY, 0.1f );
    __set_emitter_timeline_const( service, emitter_data, DZ_EMITTER_DATA_SPAWN_COUNT, 3.f );

    dz_affector_data_t * affector_data;
    if( dz_affector_data_create( service, &affector_data ) == DZ_FAILURE )
    {
        return EXIT_FAILURE;
    }

    typedef struct
    {
        dz_affector_data_timeline_type_e type;
        float time;
        float value0;
        float value1;
        ImVec2 param[MAX_POINTS];
    } timeline_data_t;

    timeline_data_t timeline_datas[] = {
        { DZ_AFFECTOR_DATA_TIMELINE_LIFE, 1.f, 3.f, 5.f },
        { DZ_AFFECTOR_DATA_TIMELINE_CHANCE_EXTRA_LIFE, 1.f, 0.05f, 0.2f },
        { DZ_AFFECTOR_DATA_TIMELINE_EXTRA_LIFE, 1.f, 2.f, 3.f },

        { DZ_AFFECTOR_DATA_TIMELINE_MOVE_SPEED, 1.f, 100.f, 300.f },
        { DZ_AFFECTOR_DATA_TIMELINE_MOVE_ACCELERATE, 1.f, 0.1f, 0.5f },
        { DZ_AFFECTOR_DATA_TIMELINE_ROTATE_SPEED, 1.f, 0.0f, 0.1f },
        { DZ_AFFECTOR_DATA_TIMELINE_ROTATE_ACCELERATE, 1.f, 0.0f, 0.f },
        { DZ_AFFECTOR_DATA_TIMELINE_SPIN_SPEED, 1.f, 0.01f, 0.1f },
        { DZ_AFFECTOR_DATA_TIMELINE_SPIN_ACCELERATE, 1.f, 0.001f, 0.f },
        { DZ_AFFECTOR_DATA_TIMELINE_STRAFE_SPEED, 1.f, 5.f, 20.f },
        { DZ_AFFECTOR_DATA_TIMELINE_STRAFE_SIZE, 1.f, 50.f, 100.f },
        { DZ_AFFECTOR_DATA_TIMELINE_STRAFE_SHIFT, 1.f, 0.f, 0.f },
        { DZ_AFFECTOR_DATA_TIMELINE_SIZE, 1.f, 25.f, 75.f },
        { DZ_AFFECTOR_DATA_TIMELINE_COLOR_R, 1.f, 0.75f, 0.25f },
        { DZ_AFFECTOR_DATA_TIMELINE_COLOR_G, 1.f, 0.5f, 0.1f },
        { DZ_AFFECTOR_DATA_TIMELINE_COLOR_B, 1.f, 0.25f, 0.9f },
        { DZ_AFFECTOR_DATA_TIMELINE_COLOR_A, 1.f, 1.f, 0.f },
    };

    for( uint32_t index = 0; index != __DZ_AFFECTOR_DATA_TIMELINE_MAX__; ++index )
    {
        timeline_data_t & data = timeline_datas[index];

        if( __set_affector_timeline_linear( service, affector_data, data.type, data.time, data.value0, data.value1 ) == DZ_FAILURE )
        {
            return EXIT_FAILURE;
        }

        // setup start points for param
        data.param[0].x = 0.f;
        data.param[0].y = 0.25f;
        data.param[2].x = 1.f;
        data.param[2].y = 0.25f;

        data.param[3].x = -1.f; // init data so editor knows to take it from here
    }

    dz_emitter_t * emitter;
    if( dz_emitter_create( service, shape_data, emitter_data, affector_data, 0, &emitter ) == DZ_FAILURE )
    {
        return EXIT_FAILURE;
    }

    uint32_t max_vertex_count = 8196 * 2;
    uint32_t max_index_count = 32768;

    example_opengl_handle_t * opengl_handle;
    if( initialize_opengl( &opengl_handle, (float)window_width, (float)window_height, max_vertex_count, max_index_count ) == false )
    {
        return EXIT_FAILURE;
    }

    while( glfwWindowShouldClose( fwWindow ) == 0 )
    {
        glfwPollEvents();

        // setup imgui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        ImGui::NewFrame();

        ImGui::Begin( "Properties" );

        ImGui::Text( "Timeline:" );

        float width = ImGui::GetWindowContentRegionWidth();
        ImVec2 size( width, width * HEIGHT_TO_WIDTH_RATIO );

        for( uint32_t index = 0; index != __DZ_AFFECTOR_DATA_TIMELINE_MAX__; ++index )
        {
            timeline_data_t & data = timeline_datas[index];

            Stringstream ss;

            ss << "Param " << index;

            if( ImGui::Curve( ss.str().c_str(), size, MAX_POINTS, data.param ) )
            {
                // curve changed

                dz_emitter_destroy( service, emitter );

                emitter = DZ_NULLPTR;

                if( dz_emitter_create( service, shape_data, emitter_data, affector_data, 0, &emitter ) == DZ_FAILURE )
                {
                    return EXIT_FAILURE;
                }
            }
        }

        ImGui::Text( "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate );

        ImGui::End();

        ImGui::EndFrame();

        ImGui::Render();

        // render editor stuff (dazzle example)
        dz_emitter_update( service, emitter, 0.01f );

        opengl_set_camera( opengl_handle, camera_offset_x, camera_offset_y, camera_scale );

        glClearColor( 0.45f, 0.55f, 0.60f, 1.00f );
        glClear( GL_COLOR_BUFFER_BIT );

        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, opengl_handle->textureId );

        opengl_use_texture_program( opengl_handle );

        glBindBuffer( GL_ARRAY_BUFFER, opengl_handle->VBO );
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, opengl_handle->IBO );

        void * vertices = glMapBuffer( GL_ARRAY_BUFFER, GL_WRITE_ONLY );
        void * indices = glMapBuffer( GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY );

        dz_emitter_mesh_t mesh;
        mesh.position_buffer = vertices;
        mesh.position_offset = offsetof( gl_vertex_t, x );
        mesh.position_stride = sizeof( gl_vertex_t );

        mesh.color_buffer = vertices;
        mesh.color_offset = offsetof( gl_vertex_t, c );
        mesh.color_stride = sizeof( gl_vertex_t );

        mesh.uv_buffer = vertices;
        mesh.uv_offset = offsetof( gl_vertex_t, u );
        mesh.uv_stride = sizeof( gl_vertex_t );

        mesh.index_buffer = indices;

        mesh.flags = DZ_EMITTER_MESH_FLAG_NONE;
        mesh.r = 1.f;
        mesh.g = 1.f;
        mesh.b = 1.f;
        mesh.a = 1.f;

        dz_emitter_mesh_chunk_t chunks[16];
        uint32_t chunk_count;

        dz_emitter_compute_mesh( emitter, &mesh, chunks, 16, &chunk_count );

        glUnmapBuffer( GL_ARRAY_BUFFER );
        glUnmapBuffer( GL_ELEMENT_ARRAY_BUFFER );

        for( uint32_t index = 0; index != chunk_count; ++index )
        {
            dz_emitter_mesh_chunk_t * chunk = chunks + index;

            glDrawElements( GL_TRIANGLES, chunk->index_size, GL_UNSIGNED_SHORT, DZ_NULLPTR );
        }

        // render imgui
        ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

        glfwSwapBuffers( fwWindow );
    }

    dz_emitter_destroy( service, emitter );
    dz_emitter_data_destroy( service, emitter_data );
    dz_affector_data_destroy( service, affector_data );
    dz_shape_data_destroy( service, shape_data );
    dz_service_destroy( service );

    return EXIT_SUCCESS;
}