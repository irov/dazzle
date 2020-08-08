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
static constexpr float TIMELINE_PANEL_WIDTH = 350.f;
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
////////////////////////////////////////////////////////////////////////////
//static dz_result_t __set_shape_timeline_linear( dz_service_t * _service, dz_shape_t * _shape, dz_shape_timeline_type_e _type, float _time0, float _value0, float _value1 )
//{
//    dz_timeline_key_t * key0;
//    if( dz_timeline_key_create( _service, &key0, 0.f, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR ) == DZ_FAILURE )
//    {
//        return DZ_FAILURE;
//    }
//
//    if( dz_timeline_key_const_set_value( key0, _value0 ) == DZ_FAILURE )
//    {
//        return DZ_FAILURE;
//    }
//
//    dz_timeline_interpolate_t * interpolate0;
//    if( dz_timeline_interpolate_create( _service, &interpolate0, DZ_TIMELINE_INTERPOLATE_LINEAR, DZ_NULLPTR ) == DZ_FAILURE )
//    {
//        return DZ_FAILURE;
//    }
//
//    dz_timeline_key_t * key1;
//    if( dz_timeline_key_create( _service, &key1, _time0, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR ) == DZ_FAILURE )
//    {
//        return DZ_FAILURE;
//    }
//
//    if( dz_timeline_key_const_set_value( key1, _value1 ) == DZ_FAILURE )
//    {
//        return DZ_FAILURE;
//    }
//
//    dz_timeline_key_set_interpolate( key0, interpolate0, key1 );
//
//    dz_shape_set_timeline( _shape, _type, key0 );
//
//    return DZ_SUCCESSFUL;
//}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __set_shape_timeline_linear_from_points( dz_service_t * _service, dz_shape_t * _shape, dz_shape_timeline_type_e _type, PointsArray _points, float _y_multiplier )
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

    // set new timeline to affector
    dz_shape_set_timeline( _shape, _type, key0 );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __reset_shape_timeline_linear_from_points( dz_service_t * _service, dz_shape_t * _shape, dz_shape_timeline_type_e _type, PointsArray _points, float _y_multiplier )
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
////////////////////////////////////////////////////////////////////////////
//static dz_result_t __set_emitter_timeline_linear( dz_service_t * _service, dz_emitter_t * _emitter, dz_emitter_timeline_type_e _type, float _time0, float _value0, float _value1 )
//{
//    dz_timeline_key_t * key0;
//    if( dz_timeline_key_create( _service, &key0, 0.f, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR ) == DZ_FAILURE )
//    {
//        return DZ_FAILURE;
//    }
//
//    if( dz_timeline_key_const_set_value( key0, _value0 ) == DZ_FAILURE )
//    {
//        return DZ_FAILURE;
//    }
//
//    dz_timeline_interpolate_t * interpolate0;
//    if( dz_timeline_interpolate_create( _service, &interpolate0, DZ_TIMELINE_INTERPOLATE_LINEAR, DZ_NULLPTR ) == DZ_FAILURE )
//    {
//        return DZ_FAILURE;
//    }
//
//    dz_timeline_key_t * key1;
//    if( dz_timeline_key_create( _service, &key1, _time0, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR ) == DZ_FAILURE )
//    {
//        return DZ_FAILURE;
//    }
//
//    if( dz_timeline_key_const_set_value( key1, _value1 ) == DZ_FAILURE )
//    {
//        return DZ_FAILURE;
//    }
//
//    dz_timeline_key_set_interpolate( key0, interpolate0, key1 );
//
//    dz_emitter_set_timeline( _emitter, _type, key0 );
//
//    return DZ_SUCCESSFUL;
//}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __reset_emitter_timeline_linear_from_points( dz_service_t * _service, dz_emitter_t * _emitter, dz_emitter_timeline_type_e _type, PointsArray _points, float _y_multiplier )
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
    const dz_timeline_key_t * oldKey0 = dz_emitter_get_timeline( _emitter, _type );

    dz_timeline_key_destroy( _service, oldKey0 );

    // set new timeline to affector
    dz_emitter_set_timeline( _emitter, _type, key0 );

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
////////////////////////////////////////////////////////////////////////////
//static dz_result_t __set_affector_timeline_linear( dz_service_t * _service, dz_affector_t * _affector, dz_affector_timeline_type_e _type, float _time0, float _value0, float _value1 )
//{
//    dz_timeline_key_t * key0;
//    if( dz_timeline_key_create( _service, &key0, 0.f, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR ) == DZ_FAILURE )
//    {
//        return DZ_FAILURE;
//    }
//
//    if( dz_timeline_key_const_set_value( key0, _value0 ) == DZ_FAILURE )
//    {
//        return DZ_FAILURE;
//    }
//
//    dz_timeline_interpolate_t * interpolate0;
//    if( dz_timeline_interpolate_create( _service, &interpolate0, DZ_TIMELINE_INTERPOLATE_LINEAR, DZ_NULLPTR ) == DZ_FAILURE )
//    {
//        return DZ_FAILURE;
//    }
//
//    dz_timeline_key_t * key1;
//    if( dz_timeline_key_create( _service, &key1, _time0, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR ) == DZ_FAILURE )
//    {
//        return DZ_FAILURE;
//    }
//
//    if( dz_timeline_key_const_set_value( key1, _value1 ) == DZ_FAILURE )
//    {
//        return DZ_FAILURE;
//    }
//
//    dz_timeline_key_set_interpolate( key0, interpolate0, key1 );
//
//    dz_affector_set_timeline( _affector, _type, key0 );
//
//    return DZ_SUCCESSFUL;
//}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __set_affector_timeline_linear2( dz_service_t * _service, dz_affector_t * _affector, dz_affector_timeline_type_e _type, float _time0, float _time1, float _value0, float _value1, float _value2 )
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

    dz_affector_set_timeline( _affector, _type, key0 );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __reset_affector_timeline_linear_from_points( dz_service_t * _service, dz_affector_t * _affector, dz_affector_timeline_type_e _type, PointsArray _points, float _y_multiplier )
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
editor::editor()
    : m_windowWidth( WINDOW_WIDTH )
    , m_windowHeight( WINDOW_HEIGHT )

    , m_dzWindowPos( 0.f, 0.f )
    //, m_dzWindowSize( 500.f, 500.f )
    , m_dzWindowSize( 732.f, 616.f )

    , m_backgroundColor( 0.f, 0.f, 0.f, 1.f )

    , m_showDebugInfo( false )

    , m_service( nullptr )

    , m_shapeData( nullptr )
    , m_emitterData( nullptr )
    , m_affectorData( nullptr )

    , m_emitter( nullptr )
    , m_fwWindow( nullptr )

    , m_timelineShapeData{
        {DZ_SHAPE_SEGMENT_ANGLE_MIN, "Segment angle min", -DZ_PI * 0.25f, 100.f},
        {DZ_SHAPE_SEGMENT_ANGLE_MAX, "Segment angle max", DZ_PI * 0.25f, 100.f},
        {DZ_SHAPE_CIRCLE_RADIUS_MIN, "Circle radius min", 50.f, 200.f},
        {DZ_SHAPE_CIRCLE_RADIUS_MAX, "Circle radius max", 100.f, 200.f},
        {DZ_SHAPE_CIRCLE_ANGLE_MIN, "Circle angle min", -DZ_PI * 0.25f, 100.f},
        {DZ_SHAPE_CIRCLE_ANGLE_MAX, "Circle angle max", DZ_PI * 0.25f, 100.f},
        {DZ_SHAPE_LINE_ANGLE, "Line angle", 0.f, 100.f},
        {DZ_SHAPE_LINE_SIZE, "Line size", 200.f, 500.f},
        {DZ_SHAPE_RECT_WIDTH_MIN, "Rect width min", 0.f, 500.f},
        {DZ_SHAPE_RECT_WIDTH_MAX, "Rect width max", 300.f, 500.f},
        {DZ_SHAPE_RECT_HEIGHT_MIN, "Rect height min", 0.f, 500.f},
        {DZ_SHAPE_RECT_HEIGHT_MAX, "Rect height max", 200.f, 500.f},
    }

    , m_timelineAffectorData{
        {DZ_AFFECTOR_TIMELINE_LIFE, "Life", 0.5f, 1.f, 3.f, 5.f, 2.f, 10.f},

        {DZ_AFFECTOR_TIMELINE_MOVE_SPEED, "Speed", 0.5f, 1.f, 100.f, 50.f, 0.f, 200.f},
        {DZ_AFFECTOR_TIMELINE_MOVE_ACCELERATE, "Accelerate", 0.5f, 1.f, 0.1f, 0.5f, 0.f, 1.f},
        {DZ_AFFECTOR_TIMELINE_ROTATE_SPEED, "Rotate speed", 0.5f, 1.f, 0.0f, 0.1f, 0.f, 0.5f},
        {DZ_AFFECTOR_TIMELINE_ROTATE_ACCELERATE, "Rotate accelerate", 0.5f, 1.f, 0.0f, 0.f, 0.f, 1.f},
        {DZ_AFFECTOR_TIMELINE_SPIN_SPEED, "Spin speed", 0.5f, 1.f, 0.01f, 0.1f, 0.f, 0.5f},
        {DZ_AFFECTOR_TIMELINE_SPIN_ACCELERATE, "Spin accelerate", 0.5f, 1.f, 0.001f, 0.f, 0.f, 1.f},
        {DZ_AFFECTOR_TIMELINE_STRAFE_SPEED, "Strafe speed", 0.5f, 1.f, 0.f, 0.f, 0.f, 1.f},
        {DZ_AFFECTOR_TIMELINE_STRAFE_FRENQUENCE, "Strafe frequence", 0.5f, 1.f, 0.f, 0.f, 0.f, 1.f},
        {DZ_AFFECTOR_TIMELINE_STRAFE_SIZE, "Strafe size", 0.5f, 1.f, 50.f, 100.f, 0.f, 200.f},
        {DZ_AFFECTOR_TIMELINE_STRAFE_SHIFT, "Strafe shift", 0.5f, 1.f, 0.f, 0.f, 0.f, 1.f},
        {DZ_AFFECTOR_TIMELINE_SIZE, "Size", 0.5f, 1.f, 25.f, 75.f, 0.f, 200.f},
        {DZ_AFFECTOR_TIMELINE_COLOR_R, "Color Red", 0.5f, 1.f, 0.75f, 0.25f, 0.4f, 1.f},
        {DZ_AFFECTOR_TIMELINE_COLOR_G, "Color Green", 0.5f, 1.f, 0.5f, 0.1f, 0.4f, 1.f},
        {DZ_AFFECTOR_TIMELINE_COLOR_B, "Color Blue", 0.5f, 1.f, 0.25f, 0.9f, 0.4f, 1.f},
        {DZ_AFFECTOR_TIMELINE_COLOR_A, "Color Alpha", 0.05f, 1.f, 0.f, 1.f, 0.f, 1.f},
    }

    //, m_timelineAffectorData{
    //    { DZ_AFFECTOR_TIMELINE_LIFE, "Life", 3.f, 2.f, 10.f },

    //    { DZ_AFFECTOR_TIMELINE_MOVE_SPEED, "Speed", 100.f, 0.f, 200.f },
    //    { DZ_AFFECTOR_TIMELINE_MOVE_ACCELERATE, "Accelerate", 0.1f, 0.f,  1.f },
    //    { DZ_AFFECTOR_TIMELINE_ROTATE_SPEED, "Rotate speed", 0.0f, 0.f, 0.5f },
    //    { DZ_AFFECTOR_TIMELINE_ROTATE_ACCELERATE, "Rotate accelerate", 0.0f, 0.f, 1.f },
    //    { DZ_AFFECTOR_TIMELINE_SPIN_SPEED, "Spin speed", 0.01f, 0.f, 0.5f },
    //    { DZ_AFFECTOR_TIMELINE_SPIN_ACCELERATE, "Spin accelerate", 0.001f, 0.f, 1.f },
    //    { DZ_AFFECTOR_TIMELINE_STRAFE_SPEED, "Strafe speed", 0.f, 0.f, 1.f },
    //    { DZ_AFFECTOR_TIMELINE_STRAFE_FRENQUENCE, "Strafe frequence", 0.f, 0.f, 1.f },
    //    { DZ_AFFECTOR_TIMELINE_STRAFE_SIZE, "Strafe size", 50.f, 0.f, 200.f },
    //    { DZ_AFFECTOR_TIMELINE_STRAFE_SHIFT, "Strafe shift", 0.f, 0.f, 1.f },
    //    { DZ_AFFECTOR_TIMELINE_SIZE, "Size", 25.f, 0.f, 200.f },
    //    { DZ_AFFECTOR_TIMELINE_COLOR_R, "Color Red", 0.75f, 0.4f, 1.f },
    //    { DZ_AFFECTOR_TIMELINE_COLOR_G, "Color Green", 0.5f, 0.4f, 1.f },
    //    { DZ_AFFECTOR_TIMELINE_COLOR_B, "Color Blue", 0.25f, 0.4f, 1.f },
    //    { DZ_AFFECTOR_TIMELINE_COLOR_A, "Color Alpha", 1.f, 0.f, 1.f },
    //}

    , m_timelineEmitterData{
        {DZ_EMITTER_SPAWN_DELAY, "Spawn delay", 0.1f, 1.f},
        {DZ_EMITTER_SPAWN_COUNT, "Spawn count", 1.f, 10.f},
        {DZ_EMITTER_SPAWN_SPIN_MIN, "Spawn spin min", 0.f, 10.f},
        {DZ_EMITTER_SPAWN_SPIN_MAX, "Spawn spin max", 0.f, 10.f},
    }

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

        // shape data
        if( dz_shape_create( m_service, &m_shapeData, m_shapeType, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return EXIT_FAILURE;
        }

        for( uint32_t index = 0; index != __DZ_SHAPE_TIMELINE_MAX__; ++index )
        {
            timeline_shape_t & data = m_timelineShapeData[index];

            if( __set_shape_timeline_const( m_service, m_shapeData, data.type, data.startValue ) == DZ_FAILURE )
            {
                return EXIT_FAILURE;
            }

            data.param[0].x = 0.f;
            data.param[0].y = data.startValue / data.maxValue;

            data.param[1].x = -1.f; // init data so editor knows to take it from here
        }

        // emitter data
        if( dz_emitter_create( m_service, &m_emitterData, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return EXIT_FAILURE;
        }

        dz_emitter_set_life( m_emitterData, 1000.f );

        for( uint32_t index = 0; index != __DZ_EMITTER_TIMELINE_MAX__; ++index )
        {
            timeline_emitter_t & data = m_timelineEmitterData[index];

            if( __set_emitter_timeline_const( m_service, m_emitterData, data.type, data.startValue ) == DZ_FAILURE )
            {
                return EXIT_FAILURE;
            }

            data.param[0].x = 0.f;
            data.param[0].y = data.startValue / data.maxValue;

            data.param[1].x = -1.f; // init data so editor knows to take it from here
        }

        // affector data
        if( dz_affector_create( m_service, &m_affectorData, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return EXIT_FAILURE;
        }

        for( uint32_t index = 0; index != __DZ_AFFECTOR_TIMELINE_MAX__; ++index )
        {
            timeline_affector_t & data = m_timelineAffectorData[index];

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

        // emitter
        if( dz_effect_create( m_service, &m_emitter, DZ_NULLPTR, m_shapeData, m_emitterData, m_affectorData, 0, 5.f, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return EXIT_FAILURE;
        }
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
            //ImGui::BeginGroup();

            //ImGui::EndGroup();

            // data type
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

                if( ImGui::Selectable( "Shape data", selected == SelectedType::SELECTED_SHAPE_DATA ) )
                    selected = SelectedType::SELECTED_SHAPE_DATA;

                if( ImGui::Selectable( "Affector data", selected == SelectedType::SELECTED_AFFECTOR_DATA ) )
                    selected = SelectedType::SELECTED_AFFECTOR_DATA;

                if( ImGui::Selectable( "Emitter data", selected == SelectedType::SELECTED_EMITTER_DATA ) )
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

    // update and render dazzle
    dz_effect_update( m_service, m_emitter, 0.005f );

    // update camera
    dz_render_set_camera( &m_openglDesc, camera_offset_x, camera_offset_y, camera_scale );

    return EXIT_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
int editor::render()
{
    // render background
    {
        glClearColor( m_backgroundColor.x, m_backgroundColor.y, m_backgroundColor.z, m_backgroundColor.w );
        glClear( GL_COLOR_BUFFER_BIT );
    }

    // render imgui
    ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

    // render dazzle
    {
        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, m_textureId );

        dz_render_use_texture_program( &m_openglDesc );

        glBindBuffer( GL_ARRAY_BUFFER, m_openglDesc.VBO );
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_openglDesc.IBO );

        void * vertices = glMapBuffer( GL_ARRAY_BUFFER, GL_WRITE_ONLY );
        void * indices = glMapBuffer( GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY );

        dz_effect_mesh_t mesh;
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

        mesh.flags = DZ_EFFECT_MESH_FLAG_NONE;
        mesh.r = 1.f;
        mesh.g = 1.f;
        mesh.b = 1.f;
        mesh.a = 1.f;

        dz_effect_mesh_chunk_t chunks[16];
        uint32_t chunk_count;

        dz_effect_compute_mesh( m_emitter, &mesh, chunks, 16, &chunk_count );

        glUnmapBuffer( GL_ARRAY_BUFFER );
        glUnmapBuffer( GL_ELEMENT_ARRAY_BUFFER );

        glViewport( (GLint)m_dzWindowPos.x, (GLint)m_dzWindowPos.y, (GLsizei)m_dzWindowSize.x, (GLsizei)m_dzWindowSize.y );

        for( uint32_t index = 0; index != chunk_count; ++index )
        {
            dz_effect_mesh_chunk_t * chunk = chunks + index;

            glDrawElements( GL_TRIANGLES, chunk->index_size, GL_UNSIGNED_SHORT, DZ_NULLPTR );
        }
    }

    glViewport( 0, 0, (GLsizei)m_windowWidth, (GLsizei)m_windowHeight );

    //// render imgui
    //ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

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
int editor::resetEmitter()
{
    dz_effect_destroy( m_service, m_emitter );

    dz_shape_destroy( m_service, m_shapeData );

    if( dz_shape_create( m_service, &m_shapeData, m_shapeType, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return EXIT_FAILURE;
    }

    for( uint32_t index = 0; index != __DZ_SHAPE_TIMELINE_MAX__; ++index )
    {
        timeline_shape_t & data = m_timelineShapeData[index];

        if( __set_shape_timeline_linear_from_points( m_service, m_shapeData, data.type, data.param, data.maxValue ) == DZ_FAILURE )
        {
            return EXIT_FAILURE;
        }
    }

    if( dz_effect_create( m_service, &m_emitter, DZ_NULLPTR, m_shapeData, m_emitterData, m_affectorData, 0, 5.f, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return EXIT_FAILURE;
    }

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
int editor::showShapeData()
{
    dz_shape_type_e current_shape_type = dz_shape_get_type( m_shapeData );

    const char * shape_type_names[] = {
        "Point",
        "Segment",
        "Circle",
        "Line",
        "Rect",
        //"Polygon",
        //"Mask",
    };
    static int selected_type = current_shape_type;

    ImGui::Combo( "Shape type", &selected_type, shape_type_names, IM_ARRAYSIZE( shape_type_names ), IM_ARRAYSIZE( shape_type_names ) );

    if( selected_type != current_shape_type )
    {
        m_shapeType = static_cast<dz_shape_type_e>(selected_type);

        this->resetEmitter();
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
                float min = 0.f, max = 0.f, default = 0.f;
                dz_shape_timeline_get_limit( data.type, &status, &min, &max, &default );

                float life = dz_effect_get_life( m_emitter );
                if( ImGui::Curve( "Edit with <Ctrl>", size, MAX_POINTS, data.param, 0.f, life, 0.f, data.maxValue ) != 0 )
                {
                    if( __reset_shape_timeline_linear_from_points( m_service, m_shapeData, data.type, data.param, data.maxValue ) == DZ_FAILURE )
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
            float life = dz_effect_get_life( m_emitter );
            if( ImGui::Curve( "Edit with <Ctrl>", size, MAX_POINTS, data.param, 0.f, life, 0.f, data.maxValue ) != 0 )
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
            float life = dz_effect_get_life( m_emitter );
            if( ImGui::Curve( "Edit with <Ctrl>", size, MAX_POINTS, data.param, 0.f, life, 0.f, data.maxValue ) != 0 )
            {
                if( __reset_emitter_timeline_linear_from_points( m_service, m_emitterData, data.type, data.param, data.maxValue ) == DZ_FAILURE )
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
int editor::showContentPane()
{
    // content
    float columnWidth = ImGui::GetColumnWidth();
    float columnHeight = ImGui::GetWindowHeight() - ImGui::GetFrameHeightWithSpacing() * 3;

    if( m_dzWindowSize.x != columnWidth || m_dzWindowSize.y != columnHeight )
    {
        m_dzWindowSize.x = columnWidth;
        m_dzWindowSize.y = columnHeight;

        // init opengl

        dz_render_set_proj( &m_openglDesc, -(float)m_dzWindowSize.x * 0.5f, (float)m_dzWindowSize.x * 0.5f, -(float)m_dzWindowSize.y * 0.5f, (float)m_dzWindowSize.y * 0.5f );
    }

    ImGuiWindow * window = ImGui::GetCurrentWindow();
    ImGuiContext & g = *GImGui;
    const ImGuiStyle & style = g.Style;
    const ImGuiID id = window->GetID( "testrender" );
    if( window->SkipItems )
        return EXIT_FAILURE;

    ImVec2 cursorPos = window->DC.CursorPos;

    m_dzWindowPos = ImVec2( cursorPos.x, m_windowHeight - cursorPos.y - m_dzWindowSize.y );

    ImRect bb( cursorPos, cursorPos + m_dzWindowSize );
    ImGui::ItemSize( bb );
    if( !ImGui::ItemAdd( bb, NULL ) )
        return 0;

    ImGui::RenderFrame( bb.Min, bb.Max, ImGui::GetColorU32( ImGuiCol_FrameBg, 1 ), true, style.FrameRounding );

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

        window->DrawList->AddText( ImVec2( bb.Min.x + style.FramePadding.x, bb.Min.y + style.FramePadding.y ), ImGui::GetColorU32( ImGuiCol_Text, 0.7f ), buf );
    }

    // controls
    ImGui::Separator();
    ImGui::Spacing();

    float life = dz_effect_get_life( m_emitter );
    float time = dz_effect_get_time( m_emitter );

    if( ImGui::Button( "Reset" ) )
    {
        this->resetEmitter();
    }
    ImGui::SameLine();

    ImGui::Text( "Life: %.3f s", life );
    ImGui::SameLine();

    ImGui::SliderFloat( "", &time, 0.0f, life, "Time: %.3f s" );
    ImGui::SameLine();

    ImGui::Spacing();

    // emitter states
    {
        ImGui::Text( "Emitter states:" );
        ImGui::SameLine();

        dz_effect_state_e emitter_state = dz_emitter_get_state( m_emitter );

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
        dz_effect_destroy( m_service, m_emitter );
        dz_emitter_destroy( m_service, m_emitterData );
        dz_affector_destroy( m_service, m_affectorData );
        dz_shape_destroy( m_service, m_shapeData );
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