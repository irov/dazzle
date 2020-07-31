#include "editor.h"

#include "dazzle/dazzle.hpp"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <stdlib.h>
#include <stdio.h>

#include "curve.hpp"

//////////////////////////////////////////////////////////////////////////
static constexpr uint32_t MAX_POINTS = 100;
static constexpr float HEIGHT_TO_WIDTH_RATIO = 0.4;;
//////////////////////////////////////////////////////////////////////////
namespace ImGui
{
    int Curve( const char * label, const ImVec2 & size, int maxpoints, ImVec2 * points );
    float CurveValue( float p, int maxpoints, const ImVec2 * points );
};
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
ImVec4 clear_color = ImVec4( 0.45f, 0.55f, 0.60f, 1.00f );
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

    GLFWwindow * fwWindow = glfwCreateWindow( window_width, window_height, "dazzle particles editor", 0, 0 );

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

    ImVec2 param_1[MAX_POINTS];

    param_1[0].x = 0.f;
    param_1[0].y = 0.25f;
    param_1[2].x = 1.f;
    param_1[2].y = 0.25f;

    param_1[3].x = -1.f; // init data so editor knows to take it from here

    ImVec2 param_2[MAX_POINTS];

    param_2[0].x = 0;
    param_2[0].y = 0;
    param_2[1].x = 0.5f;
    param_2[1].y = 0.8f;
    param_2[2].x = 1;
    param_2[2].y = 1;

    param_2[3].x = -1; // init data so editor knows to take it from here

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
        
        if( ImGui::Curve( "Param 1", size, MAX_POINTS, param_1 ) )
        {
            // curve changed
        }

        if( ImGui::Curve( "Param 2", size, MAX_POINTS, param_2 ) )
        {
            // curve changed
        }

        ImGui::Text( "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate );

        ImGui::End();

        ImGui::EndFrame();

        ImGui::Render();

        // render editor stuff
        int display_w, display_h;
        glfwGetFramebufferSize( fwWindow, &display_w, &display_h );
        glViewport( 0, 0, display_w, display_h );
        glClearColor( clear_color.x, clear_color.y, clear_color.z, clear_color.w );

        // render imgui
        glClear( GL_COLOR_BUFFER_BIT );
        ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

        glfwSwapBuffers( fwWindow );
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow( fwWindow );
    glfwTerminate();

    return EXIT_SUCCESS;
}
