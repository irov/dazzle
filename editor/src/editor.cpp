#include "editor.h"

#include "curve.hpp"

#include <stdlib.h>
#include <stdio.h>

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
static float dz_sqrtf( float _a, dz_userdata_t _ud )
{
    DZ_UNUSED( _ud );

    float value = sqrtf( _a );

    return value;
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
static dz_result_t __set_affector_timeline_linear2( dz_service_t * _service, dz_affector_data_t * _affector_data, dz_affector_data_timeline_type_e _type, float _time0, float _time1, float _value0, float _value1, float _value2 )
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

    if( dz_timeline_key_const_set_value( key1, _value1 ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    dz_timeline_key_set_interpolate( key0, interpolate0, key1 );

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

    if( dz_timeline_key_const_set_value( key2, _value2 ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    dz_timeline_key_set_interpolate( key1, interpolate1, key2 );

    dz_affector_data_set_timeline( _affector_data, _type, key0 );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __reset_affector_timeline_linear_from_points( dz_service_t * _service, dz_affector_data_t * _affector_data, dz_affector_data_timeline_type_e _type, PointsArray _points, float _y_multiplier )
{
    // first create new timeline
    dz_timeline_key_t * key0;
    if( dz_timeline_key_create( _service, &key0, _points[0].x, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    if( dz_timeline_key_const_set_value( key0, _points[0].y * _y_multiplier ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    int32_t max = 0;
    while( max < MAX_POINTS && _points[max].x >= 0 ) max++;

    dz_timeline_key_t * prevKey = key0;
    for( int32_t i = 1; i < max; i++ )
    {
        dz_timeline_interpolate_t * interpolate;
        if( dz_timeline_interpolate_create( _service, &interpolate, DZ_TIMELINE_INTERPOLATE_LINEAR, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        dz_timeline_key_t * nextKey;
        if( dz_timeline_key_create( _service, &nextKey, _points[i].x, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        if( dz_timeline_key_const_set_value( nextKey, _points[i].y * _y_multiplier ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        dz_timeline_key_set_interpolate( prevKey, interpolate, nextKey );

        prevKey = nextKey;
    }

    // destroy old timeline
    const dz_timeline_key_t * oldKey0 = dz_affector_data_get_timeline( _affector_data, _type );

    dz_timeline_key_destroy( _service, oldKey0 );

    // set new timeline to affector
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
    if( glfwGetKey( _window, GLFW_KEY_SPACE ) != GLFW_PRESS )
    {
        return;
    }

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
    : m_windowWidth( 0.f )
    , m_windowHeight( 0.f )
    
    , m_service( nullptr )

    , m_shapeData( nullptr )
    , m_emitterData( nullptr )
    , m_affectorData( nullptr )

    , m_emitter( nullptr )

    , m_openglHandle( nullptr )
    , m_fwWindow( nullptr )

    , m_timelineData{
        { DZ_AFFECTOR_DATA_TIMELINE_LIFE, "Life", 0.5f, 1.f, 3.f, 5.f, 2.f, 10.f },

        { DZ_AFFECTOR_DATA_TIMELINE_MOVE_SPEED, "Speed", 0.5f, 1.f, 100.f, 50.f, 0.f, 200.f },
        { DZ_AFFECTOR_DATA_TIMELINE_MOVE_ACCELERATE, "Accelerate", 0.5f, 1.f, 0.1f, 0.5f, 0.f, 1.f },
        { DZ_AFFECTOR_DATA_TIMELINE_ROTATE_SPEED, "Rotate speed", 0.5f, 1.f, 0.0f, 0.1f, 0.f, 0.5f },
        { DZ_AFFECTOR_DATA_TIMELINE_ROTATE_ACCELERATE, "Rotate accelerate", 0.5f, 1.f, 0.0f, 0.f, 0.f, 1.f },
        { DZ_AFFECTOR_DATA_TIMELINE_SPIN_SPEED, "Spin speed", 0.5f, 1.f, 0.01f, 0.1f, 0.f, 0.5f },
        { DZ_AFFECTOR_DATA_TIMELINE_SPIN_ACCELERATE, "Spin accelerate", 0.5f, 1.f, 0.001f, 0.f, 0.f, 1.f },
        { DZ_AFFECTOR_DATA_TIMELINE_STRAFE_SPEED, "Strafe speed", 0.5f, 1.f, 0.f, 0.f, 0.f, 1.f },
        { DZ_AFFECTOR_DATA_TIMELINE_STRAFE_FRENQUENCE, "Strafe frequence", 0.5f, 1.f, 0.f, 0.f, 0.f, 1.f },
        { DZ_AFFECTOR_DATA_TIMELINE_STRAFE_SIZE, "Strafe size", 0.5f, 1.f, 50.f, 100.f, 0.f, 200.f },
        { DZ_AFFECTOR_DATA_TIMELINE_STRAFE_SHIFT, "Strafe shift", 0.5f, 1.f, 0.f, 0.f, 0.f, 1.f },
        { DZ_AFFECTOR_DATA_TIMELINE_SIZE, "Size", 0.5f, 1.f, 25.f, 75.f, 0.f, 200.f },
        { DZ_AFFECTOR_DATA_TIMELINE_COLOR_R, "Color Red", 0.5f, 1.f, 0.75f, 0.25f, 0.4f, 1.f },
        { DZ_AFFECTOR_DATA_TIMELINE_COLOR_G, "Color Green", 0.5f, 1.f, 0.5f, 0.1f, 0.4f, 1.f },
        { DZ_AFFECTOR_DATA_TIMELINE_COLOR_B, "Color Blue", 0.5f, 1.f, 0.25f, 0.9f, 0.4f, 1.f },
        { DZ_AFFECTOR_DATA_TIMELINE_COLOR_A, "Color Alpha", 0.05f, 1.f, 0.f, 1.f, 0.f, 2.f }
    }
{
}
//////////////////////////////////////////////////////////////////////////
editor::~editor()
{
}
//////////////////////////////////////////////////////////////////////////
int editor::init()
{
    // init opengl
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

        m_windowWidth = 1024;
        m_windowHeight = 768;

        camera_offset_x = m_windowWidth * 0.5f;
        camera_offset_y = m_windowHeight * 0.5f;

        m_fwWindow = glfwCreateWindow( m_windowWidth, m_windowHeight, "dazzle particle editor", 0, 0 );

        if( m_fwWindow == 0 )
        {
            glfwTerminate();

            return EXIT_FAILURE;
        }

        glfwMakeContextCurrent( m_fwWindow );
        glfwSetFramebufferSizeCallback( m_fwWindow, &glfw_framebufferSizeCallback );
        glfwSetScrollCallback( m_fwWindow, &glfw_scrollCallback );
        glfwSetCursorPosCallback( m_fwWindow, &glfw_cursorPosCallback );

        double cursorPosX;
        double cursorPosY;
        glfwGetCursorPos( m_fwWindow, &cursorPosX, &cursorPosY );

        mouse_pos_x = (float)cursorPosX;
        mouse_pos_y = (float)cursorPosY;

        if( gladLoadGLLoader( (GLADloadproc)glfwGetProcAddress ) == 0 )
        {
            return EXIT_FAILURE;
        }

        glfwSwapInterval( 1 );
    }

    // init emitter
    {
        dz_service_providers_t providers;
        providers.f_malloc = &dz_malloc;
        providers.f_realloc = &dz_realloc;
        providers.f_free = &dz_free;
        providers.f_sqrtf = &dz_sqrtf;
        providers.f_cosf = &dz_cosf;
        providers.f_sinf = &dz_sinf;

        if( dz_service_create( &m_service, &providers, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return EXIT_FAILURE;
        }

        if( dz_shape_data_create( m_service, &m_shapeData, DZ_SHAPE_DATA_RECT, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return EXIT_FAILURE;
        }

        __set_shape_timeline_const( m_service, m_shapeData, DZ_SHAPE_DATA_RECT_WIDTH_MAX, 300.f );
        __set_shape_timeline_const( m_service, m_shapeData, DZ_SHAPE_DATA_RECT_HEIGHT_MAX, 200.f );

        if( dz_emitter_data_create( m_service, &m_emitterData, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return EXIT_FAILURE;
        }

        dz_emitter_data_set_life( m_emitterData, 1000.f );

        __set_emitter_timeline_const( m_service, m_emitterData, DZ_EMITTER_DATA_SPAWN_DELAY, 0.1f );
        __set_emitter_timeline_const( m_service, m_emitterData, DZ_EMITTER_DATA_SPAWN_COUNT, 3.f );

        if( dz_affector_data_create( m_service, &m_affectorData ) == DZ_FAILURE )
        {
            return EXIT_FAILURE;
        }

        for( uint32_t index = 0; index != __DZ_AFFECTOR_DATA_TIMELINE_MAX__; ++index )
        {
            timeline_data_t & data = m_timelineData[index];

            if( __set_affector_timeline_linear2( m_service, m_affectorData, data.type, data.time0, data.time1, data.value0, data.value1, data.value2 ) == DZ_FAILURE )
            {
                return EXIT_FAILURE;
            }

            data.param[0].x = 0.f;
            data.param[0].y = data.value0 / data.maxValue;
            data.param[1].x = data.time0;
            data.param[1].y = data.value1 / data.maxValue;
            data.param[2].x = data.time1;
            data.param[2].y = data.value2 / data.maxValue;

            data.param[3].x = -1.f; // init data so editor knows to take it from here
        }

        if( dz_emitter_create( m_service, m_shapeData, m_emitterData, m_affectorData, 0, 0.f, &m_emitter ) == DZ_FAILURE )
        {
            return EXIT_FAILURE;
        }
    }

    // init opengl
    {
        uint32_t max_vertex_count = 8196 * 2;
        uint32_t max_index_count = 32768;


        if( initialize_opengl( &m_openglHandle, (float)m_windowWidth, (float)m_windowHeight, max_vertex_count, max_index_count ) == false )
        {
            return EXIT_FAILURE;
        }
    }

    // init imgui
    {

        ImGuiContext * context = ImGui::CreateContext();
        (void)context;

        if( context == nullptr )
        {
            return EXIT_FAILURE;
        }

        if( ImGui_ImplGlfw_InitForOpenGL( m_fwWindow, true ) == false )
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
    }

    return EXIT_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
int editor::update()
{
    glfwPollEvents();

    // update imgui
    {
        // setup imgui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        ImGui::NewFrame();

        // properties
        ImGui::Begin( "Properties" );
        ImGui::Spacing();
        ImGui::Text( "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate );
        ImGui::End();

        // affector data
        ImGui::Begin( "Affector data" );

        float width = ImGui::GetWindowContentRegionWidth();
        ImVec2 size( width, width * HEIGHT_TO_WIDTH_RATIO );

        static bool headerFlags[__DZ_AFFECTOR_DATA_TIMELINE_MAX__] = { false };

        for( uint32_t index = 0; index != __DZ_AFFECTOR_DATA_TIMELINE_MAX__; ++index )
        {
            ImGui::PushID( index );

            timeline_data_t & data = m_timelineData[index];

            ImGui::Spacing();

            ImGui::Text( data.name );
            ImGui::SameLine( 200.f );
            ImGui::Checkbox( "Edit", &headerFlags[index] );

            if( headerFlags[index] == true )
            {
                if( ImGui::Curve( "Timeline", size, MAX_POINTS, data.param ) != 0 )
                {
                    if( __reset_affector_timeline_linear_from_points( m_service, m_affectorData, data.type, data.param, data.maxValue ) == DZ_FAILURE )
                    {
                        return EXIT_FAILURE;
                    }
                }
            }

            ImGui::Separator();

            ImGui::PopID();
        }

        //for( uint32_t index = 0; index != __DZ_AFFECTOR_DATA_TIMELINE_MAX__; ++index )
        //{
        //    ImGui::Spacing();
        //    timeline_data_t & data = m_timelineData[index];
        //    if( ImGui::CollapsingHeader( data.name ) )
        //    {
        //        if( ImGui::Curve( data.name, size, MAX_POINTS, data.param ) != 0 )
        //        {
        //            if( __reset_affector_timeline_linear_from_points( m_service, m_affectorData, data.type, data.param, data.maxValue ) == DZ_FAILURE )
        //            {
        //                return EXIT_FAILURE;
        //            }
        //        }
        //    }
        //}

        ImGui::End();

        ImGui::EndFrame();

        ImGui::Render();
    }

    // update and render dazzle
    dz_emitter_update( m_service, m_emitter, 0.005f );

    // update camera
    opengl_set_camera( m_openglHandle, camera_offset_x, camera_offset_y, camera_scale );

    return EXIT_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
int editor::render()
{
    // render background
    {
        glClearColor( 0.45f, 0.55f, 0.60f, 1.00f );
        glClear( GL_COLOR_BUFFER_BIT );
    }

    // render dazzle
    {
        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, m_openglHandle->textureId );

        opengl_use_texture_program( m_openglHandle );

        glBindBuffer( GL_ARRAY_BUFFER, m_openglHandle->VBO );
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_openglHandle->IBO );

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

        dz_emitter_compute_mesh( m_emitter, &mesh, chunks, 16, &chunk_count );

        glUnmapBuffer( GL_ARRAY_BUFFER );
        glUnmapBuffer( GL_ELEMENT_ARRAY_BUFFER );

        for( uint32_t index = 0; index != chunk_count; ++index )
        {
            dz_emitter_mesh_chunk_t * chunk = chunks + index;

            glDrawElements( GL_TRIANGLES, chunk->index_size, GL_UNSIGNED_SHORT, DZ_NULLPTR );
        }
    }

    // render imgui
    ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

    glfwSwapBuffers( m_fwWindow );

    return EXIT_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
int editor::run()
{
    if( this->init() == EXIT_FAILURE )
    {
        return EXIT_FAILURE;
    }

    // update loop
    while( glfwWindowShouldClose( m_fwWindow ) == 0 )
    {
        if( this->update() == EXIT_FAILURE )
        {
            return EXIT_FAILURE; // maybe break loop?
        }

        if( this->render() == EXIT_FAILURE )
        {
            return EXIT_FAILURE; // maybe break loop?
        }
    }

    this->finalize();

    return EXIT_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
void editor::finalize()
{
    // finalize emitter
    {
        dz_emitter_destroy( m_service, m_emitter );
        dz_emitter_data_destroy( m_service, m_emitterData );
        dz_affector_data_destroy( m_service, m_affectorData );
        dz_shape_data_destroy( m_service, m_shapeData );
        dz_service_destroy( m_service );
    }

    // finalize imgui
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    // finalize opengl
    {
        glfwDestroyWindow( m_fwWindow );
        glfwTerminate();
    }
}
