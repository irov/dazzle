#include "editor.h"

#include "dazzle/dazzle.hpp"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <stdlib.h>
#include <stdio.h>
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

    while( glfwWindowShouldClose( fwWindow ) == 0 )
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        ImGui::NewFrame();

        ImGui::Begin( "Properties" );

        static bool wireframe = false;
        static bool texture = false;

        ImGui::Text( "Content scale:" );
        ImGui::SliderFloat( "##ContentScale", &camera_scale, camera_scale_min, camera_scale_max );

        if( ImGui::Button( "Reset scale" ) )
        {
            camera_scale = 1.f;
            camera_offset_x = 0.f;
            camera_offset_y = 0.f;
        }

        ImGui::Checkbox( "wireframe", &wireframe );
        ImGui::Checkbox( "texture", &texture );

        static bool fill = false;

        ImGui::Checkbox( "fill", &fill );

        static bool Custom = false;
        ImGui::Checkbox( "Custom", &Custom );

        ImGui::Text( "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate );

        ImGui::End();

        ImGui::EndFrame();

        // 1. render imgui
        ImGui::Render();

        // 2. render editor
        int display_w, display_h;
        glfwGetFramebufferSize( fwWindow, &display_w, &display_h );
        glViewport( 0, 0, display_w, display_h );
        glClearColor( clear_color.x, clear_color.y, clear_color.z, clear_color.w );
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
