#include "editor.h"

#include "curve.hpp"

#include <stdlib.h>
#include <stdio.h>

//////////////////////////////////////////////////////////////////////////
namespace ImGui
{
    int Curve( const char * _label, const ImVec2 & _size, const int _maxpoints, ImVec2 * _points, float _x_min, float _x_max, float _y_min, float _y_max );
    float CurveValue( float _p, int _maxpoints, const ImVec2 * _points );
};
//////////////////////////////////////////////////////////////////////////
// aspect ratio 3:4
//static constexpr float WINDOW_WIDTH = 1024.f;
//static constexpr float WINDOW_HEIGHT = 768.f;

// aspect ratio HD 720p
static constexpr float WINDOW_WIDTH = 1280.f;
static constexpr float WINDOW_HEIGHT = 720.f; 
static constexpr float TIMELINE_PANEL_WIDTH = 330.f;
static constexpr int32_t CONTENT_CONTROLS_PANE_LINES_COUNT = 5;
//////////////////////////////////////////////////////////////////////////
static const char * CURVE_LABEL = "Add with <Ctrl> | Delete with <Alt>";
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
static dz_result_t __set_shape_timeline_const( dz_service_t * _service, dz_shape_t * _shape, dz_shape_timeline_type_e _type, float _value )
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

    dz_shape_set_timeline( _shape, _type, timeline );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __reset_shape_timeline_linear_from_points( dz_service_t * _service, dz_shape_t * _shape, dz_shape_timeline_type_e _type, PointsArray _points )
{
    // first create new timeline
    dz_timeline_key_t * key0;
    if( dz_timeline_key_create( _service, &key0, _points[0].x, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    if( dz_timeline_key_const_set_value( key0, _points[0].y ) == DZ_FAILURE )
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

        if( dz_timeline_key_const_set_value( nextKey, _points[i].y ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        dz_timeline_key_set_interpolate( prevKey, interpolate, nextKey );

        prevKey = nextKey;
    }

    // destroy old timeline
    const dz_timeline_key_t * oldKey0 = dz_shape_get_timeline( _shape, _type );

    dz_timeline_key_destroy( _service, oldKey0 );

    // set new timeline to affector
    dz_shape_set_timeline( _shape, _type, key0 );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __set_emitter_timeline_const( dz_service_t * _service, dz_emitter_t * _emitter, dz_emitter_timeline_type_e _type, float _value )
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

    dz_emitter_set_timeline( _emitter, _type, timeline );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __reset_emitter_timeline_linear_from_points( dz_service_t * _service, dz_emitter_t * _emitter, dz_emitter_timeline_type_e _type, PointsArray _points )
{
    // first create new timeline
    dz_timeline_key_t * key0;
    if( dz_timeline_key_create( _service, &key0, _points[0].x, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    if( dz_timeline_key_const_set_value( key0, _points[0].y ) == DZ_FAILURE )
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

        if( dz_timeline_key_const_set_value( nextKey, _points[i].y ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        dz_timeline_key_set_interpolate( prevKey, interpolate, nextKey );

        prevKey = nextKey;
    }

    // destroy old timeline
    const dz_timeline_key_t * oldKey0 = dz_emitter_get_timeline( _emitter, _type );

    dz_timeline_key_destroy( _service, oldKey0 );

    // set new timeline to affector
    dz_emitter_set_timeline( _emitter, _type, key0 );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __set_affector_timeline_const( dz_service_t * _service, dz_affector_t * _affector, dz_affector_timeline_type_e _type, float _value )
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

    dz_affector_set_timeline( _affector, _type, timeline );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __reset_affector_timeline_linear_from_points( dz_service_t * _service, dz_affector_t * _affector, dz_affector_timeline_type_e _type, PointsArray _points )
{
    // first create new timeline
    dz_timeline_key_t * key0;
    if( dz_timeline_key_create( _service, &key0, _points[0].x, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    if( dz_timeline_key_const_set_value( key0, _points[0].y ) == DZ_FAILURE )
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

        if( dz_timeline_key_const_set_value( nextKey, _points[i].y ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        dz_timeline_key_set_interpolate( prevKey, interpolate, nextKey );

        prevKey = nextKey;
    }

    // destroy old timeline
    const dz_timeline_key_t * oldKey0 = dz_affector_get_timeline( _affector, _type );

    dz_timeline_key_destroy( _service, oldKey0 );

    // set new timeline to affector
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
static void glfw_framebufferSizeCallback( GLFWwindow * _window, int _width, int _height )
{
    DZ_UNUSED( _window );

    glViewport( 0, 0, _width, _height );
}
//////////////////////////////////////////////////////////////////////////
static void glfw_scrollCallback( GLFWwindow * _window, double _x, double _y )
{
    DZ_UNUSED( _x );

    editor * p_editor = reinterpret_cast<editor *>(glfwGetWindowUserPointer( _window ));

    if( glfwGetKey( _window, GLFW_KEY_SPACE ) != GLFW_PRESS )
    {
        return;
    }

    const ImVec2 & dzWindowPos = p_editor->getDzWindowPos();

    float mouse_pos_x_norm = mouse_pos_x - dzWindowPos.x;
    float mouse_pos_y_norm = mouse_pos_y - dzWindowPos.y;

    camera_offset_x -= mouse_pos_x_norm / camera_scale;
    camera_offset_y -= mouse_pos_y_norm / camera_scale;

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

    camera_offset_x += mouse_pos_x_norm / camera_scale;
    camera_offset_y += mouse_pos_y_norm / camera_scale;
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
static void glfw_keyCallback( GLFWwindow * _window, int _key, int _scancode, int _action, int _mods )
{
    DZ_UNUSED( _key );
    DZ_UNUSED( _scancode );
    DZ_UNUSED( _action );
    DZ_UNUSED( _mods );
    
    if( glfwGetKey( _window, GLFW_KEY_ESCAPE ) == GLFW_PRESS )
    {
        camera_scale = 1.f;
        camera_offset_x = 0.f;
        camera_offset_y = 0.f;
    }
    else if( glfwGetKey( _window, GLFW_KEY_GRAVE_ACCENT ) == GLFW_PRESS )
    {
        editor * p_editor = reinterpret_cast<editor *>(glfwGetWindowUserPointer( _window ));

        p_editor->m_showDebugInfo = !p_editor->m_showDebugInfo;
    }
}
//////////////////////////////////////////////////////////////////////////
editor::editor()
    : m_windowWidth( WINDOW_WIDTH )
    , m_windowHeight( WINDOW_HEIGHT )

    , m_dzWindowPos( 0.f, 0.f )
    , m_dzWindowSize( 500.f, 500.f )

    , m_backgroundColor( 0.f, 0.f, 0.f, 1.f )

    , m_showDebugInfo( false )
    , m_pause( false )

    , m_service( nullptr )
    , m_atlas( nullptr )
    , m_texture( nullptr )
    , m_material( nullptr )
    
    , m_shape( nullptr )
    , m_emitter( nullptr )
    , m_affector( nullptr )

    , m_loop( DZ_FALSE )

    , m_effect( nullptr )
    , m_fwWindow( nullptr )

    , m_shapeType( DZ_SHAPE_RECT )
{
}
//////////////////////////////////////////////////////////////////////////
editor::~editor()
{
}
//////////////////////////////////////////////////////////////////////////
int editor::init()
{
    // init window
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

        m_fwWindow = glfwCreateWindow( (uint32_t)m_windowWidth, (uint32_t)m_windowHeight, "dazzle particle editor", 0, 0 );

        glfwSetWindowUserPointer( m_fwWindow, this );

        if( m_fwWindow == 0 )
        {
            glfwTerminate();

            return EXIT_FAILURE;
        }

        glfwMakeContextCurrent( m_fwWindow );
        glfwSetFramebufferSizeCallback( m_fwWindow, &glfw_framebufferSizeCallback );
        glfwSetScrollCallback( m_fwWindow, &glfw_scrollCallback );
        glfwSetCursorPosCallback( m_fwWindow, &glfw_cursorPosCallback );
        glfwSetKeyCallback( m_fwWindow, &glfw_keyCallback );

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

    // init opengl
    {
        uint32_t max_vertex_count = 8196 * 2;
        uint32_t max_index_count = 32768;

        if( dz_render_initialize( &m_openglDesc, max_vertex_count, max_index_count ) == DZ_FAILURE )
        {
            return EXIT_FAILURE;
        }

        dz_render_set_proj( &m_openglDesc, -(float)m_dzWindowSize.x * 0.5f, (float)m_dzWindowSize.x * 0.5f, -(float)m_dzWindowSize.y * 0.5f, (float)m_dzWindowSize.y * 0.5f );

        char texture_path[250];
        sprintf( texture_path, "%s/%s"
            , DAZZLE_EDITOR_CONTENT_DIR
            , "particle.png"
        );

        m_textureId = dz_render_make_texture( texture_path );
    }

    // init emitter
    {
        // service
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

        if( dz_atlas_create( m_service, &m_atlas, &m_textureId, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return EXIT_FAILURE;
        }

        if( dz_texture_create( m_service, &m_texture, &m_textureId ) == DZ_FAILURE )
        {
            return EXIT_FAILURE;
        }

        dz_atlas_add_texture( m_atlas, m_texture );

        if( dz_material_create( m_service, &m_material, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return EXIT_FAILURE;
        }

        dz_material_set_blend( m_material, DZ_BLEND_ADD );
        dz_material_set_atlas( m_material, m_atlas );

        // shape data
        if( dz_shape_create( m_service, &m_shape, m_shapeType, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return EXIT_FAILURE;
        }

        const char * shapeTypeNames[] = {
            "Segment angle min",
            "Segment angle max",
            "Circle radius min",
            "Circle radius max",
            "Circle angle min",
            "Circle angle max",
            "Line angle",
            "Line size",
            "Rect width min",
            "Rect width max",
            "Rect height min",
            "Rect height max",
        };

        for( uint32_t index = 0; index != __DZ_SHAPE_TIMELINE_MAX__; ++index )
        {
            timeline_shape_t & data = m_timelineShapeData[index];

            data.type = static_cast<dz_shape_timeline_type_e>(index);
            data.name = shapeTypeNames[index];

            dz_timeline_limit_status_e status; float min = 0.f, max = 0.f, default = 0.f, factor = 0.f;
            dz_shape_timeline_get_limit( data.type, &status, &min, &max, &default, &factor );
            
            data.zoom = 1;

            if( __set_shape_timeline_const( m_service, m_shape, data.type, default ) == DZ_FAILURE )
            {
                return EXIT_FAILURE;
            }

            data.pointsData[0].x = 0.f;
            data.pointsData[0].y = default;

            data.pointsData[1].x = -1.f; // init data so editor knows to take it from here
        }

        // emitter data
        if( dz_emitter_create( m_service, &m_emitter, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return EXIT_FAILURE;
        }

        dz_emitter_set_life( m_emitter, 1000.f );

        const char * emitterTypeNames[] = {
            "Spawn delay",
            "Spawn count",
            "Spawn spin min",
            "Spawn spin max",
        };

        for( uint32_t index = 0; index != __DZ_EMITTER_TIMELINE_MAX__; ++index )
        {
            timeline_emitter_t & data = m_timelineEmitterData[index];

            data.type = static_cast<dz_emitter_timeline_type_e>(index);
            data.name = emitterTypeNames[index];

            dz_timeline_limit_status_e status; float min = 0.f, max = 0.f, default = 0.f, factor = 0.f;
            dz_emitter_timeline_get_limit( data.type, &status, &min, &max, &default, &factor );

            data.zoom = 1;

            if( __set_emitter_timeline_const( m_service, m_emitter, data.type, default ) == DZ_FAILURE )
            {
                return EXIT_FAILURE;
            }

            data.pointsData[0].x = 0.f;
            data.pointsData[0].y = default;

            data.pointsData[1].x = -1.f; // init data so editor knows to take it from here
        }

        // affector data
        if( dz_affector_create( m_service, &m_affector, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return EXIT_FAILURE;
        }

        const char * affectorTypeNames[] = {
            "Life",
            "Move speed",
            "Move accelerate",
            "Rotate speed",
            "Rotate accelerate",
            "Spin speed",
            "Spin accelerate",
            "Strafe speed",
            "Strafe frequence",
            "Strafe size",
            "Strafe shift",
            "Size",
            "Color Red",
            "Color Green",
            "Color Blue",
            "Color Alpha",
        };

        for( uint32_t index = 0; index != __DZ_AFFECTOR_TIMELINE_MAX__; ++index )
        {
            timeline_affector_t & data = m_timelineAffectorData[index];

            data.type = static_cast<dz_affector_timeline_type_e>(index);
            data.name = affectorTypeNames[index];

            dz_timeline_limit_status_e status; float min = 0.f, max = 0.f, default = 0.f, factor = 0.f;
            dz_affector_timeline_get_limit( data.type, &status, &min, &max, &default, &factor );

            data.zoom = 1;

            if( __set_affector_timeline_const( m_service, m_affector, data.type, default ) == DZ_FAILURE )
            {
                return EXIT_FAILURE;
            }

            data.pointsData[0].x = 0.f;
            data.pointsData[0].y = default;

            data.pointsData[1].x = -1.f; // init data so editor knows to take it from here
        }

        // emitter
        if( dz_effect_create( m_service, &m_effect, m_material, m_shape, m_emitter, m_affector, 0, 5.f, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return EXIT_FAILURE;
        }

        dz_effect_set_loop( m_effect, m_loop );
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

        // layout
        ImGuiWindowFlags window_flags = 0;
        window_flags |= ImGuiWindowFlags_NoTitleBar;
        window_flags |= ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoResize;
        window_flags |= ImGuiWindowFlags_MenuBar;

        ImGui::SetNextWindowPos( ImVec2( 0.f, 0.f), ImGuiCond_Always );
        ImGui::SetNextWindowSize( ImVec2( m_windowWidth, m_windowHeight ), ImGuiCond_Always );
        if( ImGui::Begin( "Example: Simple layout", NULL, window_flags ) )
        {
            // menu bar
            if( this->showMenuBar() == EXIT_FAILURE )
            {
                return EXIT_FAILURE;
            }

            // Left
            enum SelectedType
            {
                SELECTED_SHAPE_DATA,
                SELECTED_AFFECTOR_DATA,
                SELECTED_EMITTER_DATA,

                __SELECTED_DATA_MAX__
            };

            static int selected = SELECTED_SHAPE_DATA;
            {
                ImGui::BeginChild( "left pane", ImVec2( 150, 0 ), true );

                if( ImGui::Selectable( "Shape", selected == SelectedType::SELECTED_SHAPE_DATA ) )
                    selected = SelectedType::SELECTED_SHAPE_DATA;

                if( ImGui::Selectable( "Affector", selected == SelectedType::SELECTED_AFFECTOR_DATA ) )
                    selected = SelectedType::SELECTED_AFFECTOR_DATA;

                if( ImGui::Selectable( "Emitter", selected == SelectedType::SELECTED_EMITTER_DATA ) )
                    selected = SelectedType::SELECTED_EMITTER_DATA;

                ImGui::EndChild();
            }
            ImGui::SameLine();

            // Right
            {
                ImGui::BeginGroup();

                // elements
                ImGui::BeginChild( "Selected data", ImVec2( TIMELINE_PANEL_WIDTH, 0.f ), true );

                switch( selected )
                {
                case SelectedType::SELECTED_SHAPE_DATA:
                    {
                        if( this->showShapeData() )
                        {
                            return EXIT_FAILURE;
                        }
                    } 
                    break;
                case SelectedType::SELECTED_AFFECTOR_DATA:
                    {
                        if( this->showAffectorData() )
                        {
                            return EXIT_FAILURE;
                        }
                    }
                    break;
                case SelectedType::SELECTED_EMITTER_DATA:
                    {
                        if( this->showEmitterData() )
                        {
                            return EXIT_FAILURE;
                        }
                    }
                    break;
                }

                ImGui::EndChild();

                ImGui::EndGroup();
            }
            ImGui::SameLine();

            // Right
            {
                ImGui::BeginGroup();

                ImGui::BeginChild( "item view", ImVec2( 0, 0 ), true ); // Leave room for 1 line below us

                if ( this->showContentPane() == EXIT_FAILURE )
                {
                    return EXIT_FAILURE;
                }

                ImGui::EndChild();

                ImGui::EndGroup();
            }
        }

        ImGui::End();

        ImGui::EndFrame();

        ImGui::Render();
    }

    if( m_pause == false )
    {
        // update and render dazzle
        dz_effect_update( m_service, m_effect, 0.005f );
    }

    // update camera
    dz_render_set_camera( &m_openglDesc, camera_offset_x, camera_offset_y, camera_scale );

    return EXIT_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
int editor::render()
{
    // render background
    glViewport( 0, 0, (GLsizei)m_windowWidth, (GLsizei)m_windowHeight );
    glClearColor( m_backgroundColor.x, m_backgroundColor.y, m_backgroundColor.z, m_backgroundColor.w );
    glClear( GL_COLOR_BUFFER_BIT );

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
const ImVec2 & editor::getDzWindowPos() const
{
    return m_dzWindowPos;
}
//////////////////////////////////////////////////////////////////////////
const ImVec2 & editor::getDzWindowSize() const
{
    return m_dzWindowSize;
}
//////////////////////////////////////////////////////////////////////////
int editor::resetEffect()
{
    dz_shape_set_type( m_shape, m_shapeType );

    dz_effect_reset( m_effect );

    dz_effect_emit_resume( m_effect );

    return EXIT_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
int editor::showMenuBar()
{
    if( ImGui::BeginMenuBar() )
    {
        if( ImGui::BeginMenu( "File" ) )
        {
            if( ImGui::MenuItem( "Open", "CTRL+O" ) )
            {
            }
            ImGui::EndMenu();
        }
        if( ImGui::BeginMenu( "Edit" ) )
        {
            if( ImGui::MenuItem( "Undo", "CTRL+Z" ) )
            {
            }
            if( ImGui::MenuItem( "Redo", "CTRL+Y", false, false ) )
            {
            }  // Disabled item
            ImGui::Separator();
            if( ImGui::MenuItem( "Cut", "CTRL+X" ) )
            {
            }
            if( ImGui::MenuItem( "Copy", "CTRL+C" ) )
            {
            }
            if( ImGui::MenuItem( "Paste", "CTRL+V" ) )
            {
            }

            ImGui::MenuItem( "Debug info", "CTRL+I", &m_showDebugInfo );

            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    return EXIT_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
static void __pointsDataToCurve( ImVec2 * _pointsData, ImVec2 * _pointsCurve, float _min, float _range )
{
    int end = 0;
    for( ; end < MAX_POINTS && _pointsData[end].x >= 0; end++ )
    {
        _pointsCurve[end].x = _pointsData[end].x;
        _pointsCurve[end].y = (_pointsData[end].y - _min) / _range;
    }
    _pointsCurve[end].x = -1;
};
//////////////////////////////////////////////////////////////////////////
static void __pointsCurveToData( ImVec2 * _pointsCurve, ImVec2 * _pointsData, float _min, float _range )
{
    int end = 0;
    for( ; end < MAX_POINTS && _pointsCurve[end].x >= 0; end++ )
    {
        _pointsData[end].x = _pointsCurve[end].x;
        _pointsData[end].y = _min + (_pointsCurve[end].y * _range);
    }
    _pointsData[end].x = -1;
};
//////////////////////////////////////////////////////////////////////////
static void __setupLimits( ImVec2 * _pointsData, dz_timeline_limit_status_e _status, float _min, float _max, float * _factor, int32_t * _zoom, float * _y_min, float * _y_max )
{
    if( _status != DZ_TIMELINE_LIMIT_NORMAL )
    {
        // zoom up
        int32_t nextZoomUp = *_zoom + 1;

        float nextFactorUp = nextZoomUp * (*_factor);

        float y_min_up = _min, y_max_up = _max;

        if( _status == DZ_TIMELINE_LIMIT_MAX )
        {
            y_max_up = nextFactorUp;
        }
        else if( _status == DZ_TIMELINE_LIMIT_MIN )
        {
            y_min_up = -nextFactorUp;
        }
        else if( _status == DZ_TIMELINE_LIMIT_MINMAX )
        {
            y_min_up = -nextFactorUp;
            y_max_up = nextFactorUp;
        }

        bool availableZoomUp = y_min_up >= _min && y_max_up <= _max;

        if( availableZoomUp == true )
        {
            ImGui::SameLine();
            if( ImGui::Button( "+" ) == true )
            {
                *_zoom = nextZoomUp;
            }
        }

        // zoom down
        int32_t nextZoomDown = *_zoom - 1;

        if( nextZoomDown < 1 )
        {
            nextZoomDown = 1;
        }

        float nextFactorDown = nextZoomDown * (*_factor);

        float y_min_down = _min, y_max_down = _max;

        if( _status == DZ_TIMELINE_LIMIT_MAX )
        {
            y_max_down = nextFactorDown;
        }
        else if( _status == DZ_TIMELINE_LIMIT_MIN )
        {
            y_min_down = -nextFactorDown;
        }
        else if( _status == DZ_TIMELINE_LIMIT_MINMAX )
        {
            y_min_down = -nextFactorDown;
            y_max_down = nextFactorDown;
        }

        bool availableZoomDown = true;

        int end = 0;
        for( ; end < MAX_POINTS && _pointsData[end].x >= 0; end++ )
        {
            if( _pointsData[end].y < y_min_down || _pointsData[end].y > y_max_down )
            {
                availableZoomDown = false;
                break;
            }
        }

        if( availableZoomDown == true )
        {
            ImGui::SameLine();
            if( ImGui::Button( "-" ) == true )
            {
                *_zoom = nextZoomDown;
            }
        }
    }

    *_factor *= *_zoom;

    if( _status == DZ_TIMELINE_LIMIT_MAX )
    {
        *_y_min = _min;
        *_y_max = *_factor;
    }
    else if( _status == DZ_TIMELINE_LIMIT_MIN )
    {
        *_y_min = -*_factor;
        *_y_max = _max;
    }
    else if( _status == DZ_TIMELINE_LIMIT_MINMAX )
    {
        *_y_min = -*_factor;
        *_y_max = *_factor;
    }
};
//////////////////////////////////////////////////////////////////////////
int editor::showShapeData()
{
    const char * shape_type_names[] = {
        "Point",
        "Segment",
        "Circle",
        "Line",
        "Rect",
        //"Polygon",
        //"Mask",
    };

    static int selected_type = m_shapeType;

    ImGui::Combo( "Shape type", &selected_type, shape_type_names, IM_ARRAYSIZE( shape_type_names ), IM_ARRAYSIZE( shape_type_names ) );

    if( selected_type != m_shapeType )
    {
        m_shapeType = static_cast<dz_shape_type_e>(selected_type);

        if( this->resetEffect() == EXIT_FAILURE )
        {
            return EXIT_FAILURE;
        }
    }

    // timeline
    ImGui::Spacing();
    ImGui::Text( "Shape timelines:" );

    float width = ImGui::GetWindowContentRegionWidth();
    ImVec2 size( width, width * HEIGHT_TO_WIDTH_RATIO );

    static bool headerFlags[__DZ_SHAPE_TIMELINE_MAX__] = { false };

    ImGui::Separator();

    for( uint32_t index = 0; index != __DZ_SHAPE_TIMELINE_MAX__; ++index )
    {
        timeline_shape_t & data = m_timelineShapeData[index];

        bool show = false;

        switch( m_shapeType )
        {
        case DZ_SHAPE_SEGMENT:
            if( data.type >= DZ_SHAPE_SEGMENT_ANGLE_MIN && data.type <= DZ_SHAPE_SEGMENT_ANGLE_MAX )
            {
                show = true;
            }
            break;
        case DZ_SHAPE_CIRCLE:
            if( data.type >= DZ_SHAPE_CIRCLE_RADIUS_MIN && data.type <= DZ_SHAPE_CIRCLE_ANGLE_MAX )
            {
                show = true;
            }
            break;
        case DZ_SHAPE_LINE:
            if( data.type >= DZ_SHAPE_LINE_ANGLE && data.type <= DZ_SHAPE_LINE_SIZE )
            {
                show = true;
            }
            break;
        case DZ_SHAPE_RECT:
            if( data.type >= DZ_SHAPE_RECT_WIDTH_MIN && data.type <= DZ_SHAPE_RECT_HEIGHT_MAX )
            {
                show = true;
            }
            break;
        }

        if( show == true )
        {
            ImGui::PushID( index );

            ImGui::Checkbox( data.name, &headerFlags[index] );

            if( headerFlags[index] == true )
            {
                dz_timeline_limit_status_e status;
                float min = 0.f, max = 0.f, default = 0.f, factor = 0.f;
                dz_shape_timeline_get_limit( data.type, &status, &min, &max, &default, &factor );
                float life = dz_effect_get_life( m_effect );

                // curve
                float x_min = 0.f;
                float x_max = life;

                float y_min = min;
                float y_max = max;

                __setupLimits( data.pointsData, status, min, max, &factor, &(data.zoom), &y_min, &y_max );

                float y_range = y_max - y_min;

                __pointsDataToCurve( data.pointsData, data.pointsCurve, y_min, y_range );

                if( ImGui::Curve( CURVE_LABEL, size, MAX_POINTS, data.pointsCurve, x_min, x_max, y_min, y_max ) != 0 )
                {
                    __pointsCurveToData( data.pointsCurve, data.pointsData, y_min, y_range );

                    if( __reset_shape_timeline_linear_from_points( m_service, m_shape, data.type, data.pointsData ) == DZ_FAILURE )
                    {
                        return EXIT_FAILURE;
                    }
                }
            }

            ImGui::Separator();

            ImGui::PopID();
        }
    }

    return EXIT_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
int editor::showAffectorData()
{
    // timeline
    ImGui::Spacing();
    ImGui::Text( "Affector timelines:" );

    float width = ImGui::GetWindowContentRegionWidth();
    ImVec2 size( width, width * HEIGHT_TO_WIDTH_RATIO );

    static bool headerFlags[__DZ_AFFECTOR_TIMELINE_MAX__] = { false };

    ImGui::Separator();

    for( uint32_t index = 0; index != __DZ_AFFECTOR_TIMELINE_MAX__; ++index )
    {
        ImGui::PushID( index );

        timeline_affector_t & data = m_timelineAffectorData[index];

        ImGui::Checkbox( data.name, &headerFlags[index] );

        if( headerFlags[index] == true )
        {
            dz_timeline_limit_status_e status;
            float min = 0.f, max = 0.f, default = 0.f, factor = 0.f;
            dz_affector_timeline_get_limit( data.type, &status, &min, &max, &default, &factor );
            float life = dz_effect_get_life( m_effect );

            // curve
            float x_min = 0.f;
            float x_max = life;

            float y_min = min;
            float y_max = max;

            __setupLimits( data.pointsData, status, min, max, &factor, &(data.zoom), &y_min, &y_max );

            float y_range = y_max - y_min;

            __pointsDataToCurve( data.pointsData, data.pointsCurve, y_min, y_range );

            if( ImGui::Curve( CURVE_LABEL, size, MAX_POINTS, data.pointsCurve, x_min, x_max, y_min, y_max ) != 0 )
            {
                __pointsCurveToData( data.pointsCurve, data.pointsData, y_min, y_range );

                if( __reset_affector_timeline_linear_from_points( m_service, m_affector, data.type, data.pointsData ) == DZ_FAILURE )
                {
                    return EXIT_FAILURE;
                }
            }
        }

        ImGui::Separator();

        ImGui::PopID();
    }

    return EXIT_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
int editor::showEmitterData()
{
    // timeline
    ImGui::Spacing();
    ImGui::Text( "Emitter timelines:" );

    float width = ImGui::GetWindowContentRegionWidth();
    ImVec2 size( width, width * HEIGHT_TO_WIDTH_RATIO );

    static bool headerFlags[__DZ_EMITTER_TIMELINE_MAX__] = { false };

    ImGui::Separator();

    for( uint32_t index = 0; index != __DZ_EMITTER_TIMELINE_MAX__; ++index )
    {
        timeline_emitter_t & data = m_timelineEmitterData[index];

        ImGui::PushID( index );

        ImGui::Checkbox( data.name, &headerFlags[index] );

        if( headerFlags[index] == true )
        {
            dz_timeline_limit_status_e status;
            float min = 0.f, max = 0.f, default = 0.f, factor = 0.f;
            dz_emitter_timeline_get_limit( data.type, &status, &min, &max, &default, &factor );
            float life = dz_effect_get_life( m_effect );

            // curve
            float x_min = 0.f;
            float x_max = life;

            float y_min = min;
            float y_max = max;

            __setupLimits( data.pointsData, status, min, max, &factor, &(data.zoom), &y_min, &y_max );

            float y_range = y_max - y_min;

            __pointsDataToCurve( data.pointsData, data.pointsCurve, y_min, y_range );

            if( ImGui::Curve( CURVE_LABEL, size, MAX_POINTS, data.pointsCurve, x_min, x_max, y_min, y_max ) != 0 )
            {
                __pointsCurveToData( data.pointsCurve, data.pointsData, y_min, y_range );

                if( __reset_emitter_timeline_linear_from_points( m_service, m_emitter, data.type, data.pointsData ) == DZ_FAILURE )
                {
                    return EXIT_FAILURE;
                }
            }
        }

        ImGui::Separator();

        ImGui::PopID();
    }

    return EXIT_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
static void __draw_callback( const ImDrawList * parent_list, const ImDrawCmd * cmd )
{
    DZ_UNUSED( parent_list );
 
    editor * e = reinterpret_cast<editor *>(cmd->UserCallbackData);

    e->showDazzleCanvas();
}
//////////////////////////////////////////////////////////////////////////
void editor::showDazzleCanvas()
{
    // render dazzle
    dz_render_set_proj( &m_openglDesc, -(float)m_dzWindowSize.x * 0.5f, (float)m_dzWindowSize.x * 0.5f, -(float)m_dzWindowSize.y * 0.5f, (float)m_dzWindowSize.y * 0.5f );

    dz_render_use_texture_program( &m_openglDesc );

    GLint oldViewport[4];
    glGetIntegerv( GL_VIEWPORT, oldViewport );

    glViewport( (GLint)m_dzWindowPos.x, (GLint)m_dzWindowPos.y, (GLsizei)m_dzWindowSize.x, (GLsizei)m_dzWindowSize.y );

    dz_render_effect( &m_openglDesc, m_effect );

    glViewport( oldViewport[0], oldViewport[1], oldViewport[2], oldViewport[3] );
}
//////////////////////////////////////////////////////////////////////////
int editor::showContentPane()
{
    // content
    float columnWidth = ImGui::GetColumnWidth();
    float columnHeight = ImGui::GetWindowHeight() - ImGui::GetFrameHeightWithSpacing() * (CONTENT_CONTROLS_PANE_LINES_COUNT + 1);

    m_dzWindowSize.x = columnWidth;
    m_dzWindowSize.y = columnHeight;

    ImGuiWindow * window = ImGui::GetCurrentWindow();
    const ImGuiID id = window->GetID( "testrender" );
    if( window->SkipItems )
        return EXIT_FAILURE;

    ImVec2 cursorPos = window->DC.CursorPos;

    m_dzWindowPos = ImVec2( cursorPos.x, m_windowHeight - cursorPos.y - m_dzWindowSize.y );

    ImGui::BeginChild( "Another Window", m_dzWindowSize );

    window->DrawList->AddRectFilled( cursorPos, cursorPos + m_dzWindowSize, ImGui::GetColorU32( ImGuiCol_FrameBg, 1 ) );

    ImGui::GetWindowDrawList()->AddCallback( &__draw_callback, this );
    ImGui::GetWindowDrawList()->AddCallback( ImDrawCallback_ResetRenderState, nullptr );

    if( m_showDebugInfo == true )
    {
        char buf[500];

        sprintf( buf,
            "Application average %.3f ms/frame (%.1f FPS)\n\n"
            "      window size: (%.2f, %.2f)\n"
            "     camera_scale: %.2f\n"
            " camera_scale_min: %.2f\n"
            " camera_scale_max: %.2f\n"
            "camera_scale_step: %.2f\n"
            "  camera_offset_x: %.2f\n"
            "  camera_offset_y: %.2f\n"
            "      mouse_pos_x: %.2f\n"
            "      mouse_pos_y: %.2f\n"
            , 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate
            , m_dzWindowSize.x, m_dzWindowSize.y
            , camera_scale
            , camera_scale_min
            , camera_scale_max
            , camera_scale_step
            , camera_offset_x
            , camera_offset_y
            , mouse_pos_x
            , mouse_pos_y
        );

        window->DrawList->AddText( cursorPos, ImGui::GetColorU32( ImGuiCol_Text, 0.7f ), buf );
    }

    ImGui::EndChild();

    ImGui::Separator();

    // controls
    ImGui::BeginGroup();
    ImGui::BeginChild( "ContentPaneControlls" );

    ImGui::Spacing();

    if (this->showContentPaneControls() == EXIT_FAILURE)
    {
        return EXIT_FAILURE;
    }

    ImGui::EndChild();
    ImGui::EndGroup();
    
    return EXIT_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
int editor::showContentPaneControls()
{
    float life = dz_effect_get_life( m_effect );
    float time = dz_effect_get_time( m_effect );

    // buttons
    if( ImGui::Button( "Reset" ) )
    {
        if( this->resetEffect() == EXIT_FAILURE )
        {
            return EXIT_FAILURE;
        }
    }
    ImGui::SameLine();

    if( ImGui::Button( "Pause" ) )
    {
        m_pause = true;
    }
    ImGui::SameLine();

    if( ImGui::Button( "Resume" ) )
    {
        m_pause = false;
    }
    ImGui::SameLine();

    // loop
    static bool isLoop = m_loop == DZ_TRUE ? true : false;
    if( ImGui::Checkbox( "Loop", &isLoop ) == true )
    {
        m_loop = isLoop == true ? DZ_TRUE : DZ_FALSE;

        dz_effect_set_loop( m_effect, m_loop );
    }
    ImGui::SameLine();

    // time
    char label[100];
    sprintf( label, "Time: %%.3f s / %.3f s"
        , life
    );

    if( ImGui::SliderFloat( "", &time, 0.0f, life, label ) == true )
    {
        this->resetEffect();

        dz_effect_update( m_service, m_effect, time );
    }
    
    // life
    if( ImGui::InputFloat( "Life", &life, 0.f, 0.f, NULL, ImGuiInputTextFlags_None ) == true )
    {
        dz_effect_set_life( m_effect, life );

        this->resetEffect();
    }
    //ImGui::SameLine();

    // camera
    if( ImGui::Button( "Reset camera" ) )
    {
        camera_scale = 1.f;
        camera_offset_x = 0.f;
        camera_offset_y = 0.f;
    }
    ImGui::SameLine();

    ImGui::Text( "Camera move/scroll: <Space> + Mouse" );

    // emitter states
    {
        ImGui::Text( "Emitter states:" );
        ImGui::SameLine();

        dz_effect_state_e emitter_state = dz_emitter_get_state( m_effect );

        auto lamdba_addBoolIndicator = []( bool _value, const char * _msg )
        {
            ImVec4 colorGreen( ImColor( 0, 255, 0 ) );
            ImVec4 colorRed( ImColor( 255, 0, 0 ) );

            ImGui::PushStyleColor( ImGuiCol_Text, _value ? colorGreen : colorRed );
            ImGui::Text( _msg );
            ImGui::PopStyleColor( 1 );
            ImGui::SameLine();
        };

        lamdba_addBoolIndicator( emitter_state & DZ_EFFECT_EMIT_COMPLETE, "[Emit complete]" );
        lamdba_addBoolIndicator( emitter_state & DZ_EFFECT_PARTICLE_COMPLETE, "[Particle complete]" );
    }

    return EXIT_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
void editor::finalize()
{
    // finalize emitter
    {
        dz_effect_destroy( m_service, m_effect );
        dz_emitter_destroy( m_service, m_emitter );
        dz_affector_destroy( m_service, m_affector );
        dz_shape_destroy( m_service, m_shape );
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
