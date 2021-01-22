#include "editor.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include "nfd.h"
#include "zip.h"
#include "unzip.h"

#ifdef _WIN32
#   define USEWIN32IOAPI
#   include "iowin32.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <sstream>

//////////////////////////////////////////////////////////////////////////
typedef enum er_window_type_e
{
    ER_WINDOW_TYPE_EFFECT_DATA,
    ER_WINDOW_TYPE_SHAPE_DATA,
    ER_WINDOW_TYPE_AFFECTOR_DATA,
    ER_WINDOW_TYPE_EMITTER_DATA,
    ER_WINDOW_TYPE_MATERIAL_DATA,

    __ER_WINDOW_TYPE_MAX__
} er_window_type_e;
//////////////////////////////////////////////////////////////////////////
//static constexpr float ER_WINDOW_WIDTH = 1024.f;  // aspect ratio 3:4
//static constexpr float ER_WINDOW_HEIGHT = 768.f;
static constexpr float ER_WINDOW_WIDTH = 1280.f;    // aspect ratio HD 720p
static constexpr float ER_WINDOW_HEIGHT = 720.f; 
static constexpr float ER_TIMELINE_PANEL_WIDTH = 330.f;
static constexpr int32_t ER_CONTENT_CONTROLS_PANE_LINES_COUNT = 5;
static constexpr ImGuiID ER_CURVE_ID_NONE = 0;
static constexpr int ER_CURVE_POINT_NONE = -1;
//////////////////////////////////////////////////////////////////////////
static const char * ER_DEFAULT_PARTICLE_TEXTURE_FILE_NAME = "particle.png";
//////////////////////////////////////////////////////////////////////////
// All texts
//////////////////////////////////////////////////////////////////////////
static const char * ER_TITLE = "Dazzle particle editor";
static const char * ER_MENU_FILE = "File";
static const char * ER_MENU_FILE_ITEM_OPEN = "Open";
static const char * ER_MENU_FILE_ITEM_SAVE = "Save";
static const char * ER_MENU_FILE_ITEM_EXPORT = "Export";
static const char * ER_MENU_EDIT = "Edit";
static const char * ER_MENU_EDIT_ITEM_UNDO = "Undo";
static const char * ER_MENU_EDIT_ITEM_REDO = "Redo";
static const char * ER_MENU_EDIT_ITEM_SHOW_DEBUG_INFO = "Show debug info";
static const char * ER_MENU_EDIT_ITEM_SHOW_CANVAS_LINES = "Show canvase lines";
//////////////////////////////////////////////////////////////////////////
static const char * ER_CURVE_BTN_ZOOM_UP_TEXT = "+";
static const char * ER_CURVE_BTN_ZOOM_DOWN_TEXT = "-";
static const char * ER_CURVE_COMBO_MODE_LABEL_TEXT = "Mode";
static const char * ER_WINDOW_EFFECT_TITLE = "Effect data:";
static const char * ER_WINDOW_EFFECT_SEED_TEXT = "Seed";
static const char * ER_WINDOW_SHAPE_TITLE = "Shape timelines:";
static const char * ER_WINDOW_COMBO_SHAPE_TYPE_TEXT = "Shape type";
static const char * ER_WINDOW_AFFECTOR_TITLE = "Affector timelines:";
static const char * ER_WINDOW_EMITTER_TITLE = "Emitter timelines:";
static const char * ER_WINDOW_MATERIAL_TITLE = "Material data";
static const char * ER_WINDOW_MATERIAL_COMBO_BLEND_MODE_TEXT = "Blend mode";
static const char * ER_WINDOW_MATERIAL_TEXTURE_TITLE = "Texture";
static const char * ER_WINDOW_MATERIAL_TEXTURE_SIZE_LABEL = "Size:";
static const char * ER_WINDOW_MATERIAL_TEXTURE_BTN_BROWSE = "Browse";
static const char * ER_WINDOW_CONTROLS_BTN_RESET_TEXT = "Reset";
static const char * ER_WINDOW_CONTROLS_BTN_PAUSE_TEXT = "Pause";
static const char * ER_WINDOW_CONTROLS_BTN_RESUME_TEXT = "Resume";
static const char * ER_WINDOW_CONTROLS_BTN_LOOP_TEXT = "Loop";
static const char * ER_WINDOW_CONTROLS_TIMELINE_PREFIX_TEXT = "Time:";
static const char * ER_WINDOW_CONTROLS_INPUT_LIFE_TEXT = "Life";
static const char * ER_WINDOW_CONTROLS_BTN_RESET_CAMERA_TEXT = "Reset camera";
static const char * ER_WINDOW_CONTROLS_CAMERA_MOVE_HELP_TEXT = "Camera move/scroll: <Space> + Mouse";
static const char * ER_WINDOW_CONTROLS_EMIT_STATES_LABEL_TEXT = "Emitter states:";
static const char * ER_WINDOW_CONTROLS_EMIT_COMPLETE_STATE_TEXT = "[Emit complete]";
static const char * ER_WINDOW_CONTROLS_PARTICLE_COMPLETE_STATE_TEXT = "[Particle complete]";
//////////////////////////////////////////////////////////////////////////
static const char * ER_WINDOW_TYPE_NAMES[] = {
    "Effect",            //ER_WINDOW_TYPE_EFFECT_DATA
    "Shape",             //ER_WINDOW_TYPE_SHAPE_DATA
    "Affector",          //ER_WINDOW_TYPE_AFFECTOR_DATA
    "Emitter",           //ER_WINDOW_TYPE_EMITTER_DATA
    "Material",          //ER_WINDOW_TYPE_MATERIAL_DATA
};
//////////////////////////////////////////////////////////////////////////
static const char * ER_TIMELINE_KEY_MODE_NAMES[] = {
    "Normal",            // ER_CURVE_POINT_MODE_NORMAL
    "Random",            // ER_CURVE_POINT_MODE_RANDOM
};
//////////////////////////////////////////////////////////////////////////
static const char * ER_BLEND_MODE_NAMES[] = {
    "Normal",            //DZ_BLEND_NORNAL
    "Add",               //DZ_BLEND_ADD
    "Multiply",          //DZ_BLEND_MULTIPLY
    "Screen",            //DZ_BLEND_SCREEN
};
//////////////////////////////////////////////////////////////////////////
static const char * ER_SHAPE_DATA_NAMES[] = {
    "Segment angle min", //DZ_SHAPE_SEGMENT_ANGLE_MIN
    "Segment angle max", //DZ_SHAPE_SEGMENT_ANGLE_MAX
    "Circle radius min", //DZ_SHAPE_CIRCLE_RADIUS_MIN
    "Circle radius max", //DZ_SHAPE_CIRCLE_RADIUS_MAX
    "Circle angle min",  //DZ_SHAPE_CIRCLE_ANGLE_MIN
    "Circle angle max",  //DZ_SHAPE_CIRCLE_ANGLE_MAX
    "Line angle",        //DZ_SHAPE_LINE_ANGLE
    "Line size",         //DZ_SHAPE_LINE_SIZE
    "Rect width min",    //DZ_SHAPE_RECT_WIDTH_MIN
    "Rect width max",    //DZ_SHAPE_RECT_WIDTH_MAX
    "Rect height min",   //DZ_SHAPE_RECT_HEIGHT_MIN
    "Rect height max",   //DZ_SHAPE_RECT_HEIGHT_MAX
};
//////////////////////////////////////////////////////////////////////////
const char * ER_SHAPE_TYPE_NAMES[] = {
    "Point",             //DZ_SHAPE_POINT
    "Segment",           //DZ_SHAPE_SEGMENT
    "Circle",            //DZ_SHAPE_CIRCLE
    "Line",              //DZ_SHAPE_LINE
    "Rect",              //DZ_SHAPE_RECT
    "Polygon",           //DZ_SHAPE_POLYGON
    "Mask",              //DZ_SHAPE_MASK
};
//////////////////////////////////////////////////////////////////////////
const char * ER_EMITTER_DATA_NAMES[] = {
    "Spawn delay (inv)", //DZ_EMITTER_SPAWN_DELAY inverted, means value = 1 / DZ_EMITTER_SPAWN_DELAY
    "Spawn count",       //DZ_EMITTER_SPAWN_COUNT
    "Spawn spin min",    //DZ_EMITTER_SPAWN_SPIN_MIN
    "Spawn spin max",    //DZ_EMITTER_SPAWN_SPIN_MAX
};
//////////////////////////////////////////////////////////////////////////
const char * ER_AFFECTOR_DATA_NAMES[] = {
    "Life",              //DZ_AFFECTOR_TIMELINE_LIFE
    "Move speed",        //DZ_AFFECTOR_TIMELINE_MOVE_SPEED
    "Move accelerate",   //DZ_AFFECTOR_TIMELINE_MOVE_ACCELERATE
    "Rotate speed",      //DZ_AFFECTOR_TIMELINE_ROTATE_SPEED
    "Rotate accelerate", //DZ_AFFECTOR_TIMELINE_ROTATE_ACCELERATE
    "Spin speed",        //DZ_AFFECTOR_TIMELINE_SPIN_SPEED
    "Spin accelerate",   //DZ_AFFECTOR_TIMELINE_SPIN_ACCELERATE
    "Strafe speed",      //DZ_AFFECTOR_TIMELINE_STRAFE_SPEED
    "Strafe frequence",  //DZ_AFFECTOR_TIMELINE_STRAFE_FRENQUENCE
    "Strafe size",       //DZ_AFFECTOR_TIMELINE_STRAFE_SIZE
    "Strafe shift",      //DZ_AFFECTOR_TIMELINE_STRAFE_SHIFT
    "Size",              //DZ_AFFECTOR_TIMELINE_SIZE
    "Color Red",         //DZ_AFFECTOR_TIMELINE_COLOR_R
    "Color Green",       //DZ_AFFECTOR_TIMELINE_COLOR_G
    "Color Blue",        //DZ_AFFECTOR_TIMELINE_COLOR_B
    "Color Alpha",       //DZ_AFFECTOR_TIMELINE_COLOR_A
};
//////////////////////////////////////////////////////////////////////////
struct my_json_load_data_t
{
    const uint8_t * buffer;
    size_t carriage;
    size_t capacity;
};
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
static dz_result_t addZipFile( zipFile _zf, const char * _file, const void * _buffer, size_t _size )
{
    zip_fileinfo zi;
    zi.tmz_date.tm_sec = 0;
    zi.tmz_date.tm_min = 0;
    zi.tmz_date.tm_hour = 0;
    zi.tmz_date.tm_mday = 0;
    zi.tmz_date.tm_mon = 0;
    zi.tmz_date.tm_year = 0;
    zi.dosDate = 0;
    zi.internal_fa = 0;
    zi.external_fa = 0;

    if( zipOpenNewFileInZip3_64( _zf, _file, &zi,
        NULL, 0, NULL, 0, NULL /* comment*/,
        Z_DEFLATED, 6, 0,
        -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
        NULL, 0, 0 ) != ZIP_OK )
    {
        return DZ_FAILURE;
    }

    if( zipWriteInFileInZip( _zf, _buffer, _size ) != ZIP_OK )
    {
        return DZ_FAILURE;
    }

    if( zipCloseFileInZip( _zf ) != ZIP_OK )
    {
        return DZ_FAILURE;
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t openZipFile( unzFile _uf, const char * _file, std::vector<uint8_t> * _buffer )
{
    unz_global_info64 gi;
    if( unzGetGlobalInfo64( _uf, &gi ) != UNZ_OK )
    {
        return DZ_FAILURE;
    }

    if( unzLocateFile( _uf, _file, 0 ) != UNZ_OK )
    {
        return DZ_FAILURE;
    }

    char filename_inzip[256];
    unz_file_info64 file_info;
    if( unzGetCurrentFileInfo64( _uf, &file_info, filename_inzip, sizeof( filename_inzip ), NULL, 0, NULL, 0 ) != UNZ_OK )
    {
        return DZ_FAILURE;
    }

    if( unzOpenCurrentFile( _uf ) != UNZ_OK )
    {
        return DZ_FAILURE;
    }

    void * content_buffer = malloc( file_info.uncompressed_size );
    size_t content_size = file_info.uncompressed_size;

    unzReadCurrentFile( _uf, content_buffer, content_size );

    _buffer->assign( reinterpret_cast<const uint8_t *>(content_buffer), reinterpret_cast<const uint8_t *>(content_buffer) + content_size );

    free( content_buffer );

    unzCloseCurrentFile( _uf );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __set_shape_timeline_const( dz_service_t * _service, dz_shape_t * _shape, dz_shape_timeline_type_e _type, float _value )
{
    dz_timeline_key_t * timeline;
    if( dz_timeline_key_create( _service, &timeline, 0.f, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    dz_timeline_key_set_const_value( timeline, _value );

    dz_shape_set_timeline( _shape, _type, timeline );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __reset_shape_timeline_linear_from_points( dz_service_t * _service, dz_shape_t * _shape, dz_shape_timeline_type_e _type, PointsArray _points )
{
    // first create new timeline
    dz_timeline_key_t * key0 = DZ_NULLPTR;

    if( _points[0].mode == ER_CURVE_POINT_MODE_NORMAL )
    {
        if( dz_timeline_key_create( _service, &key0, _points[0].x, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        dz_timeline_key_set_const_value( key0, _points[0].y );
    }
    else if( _points[0].mode == ER_CURVE_POINT_MODE_RANDOM )
    {
        if( dz_timeline_key_create( _service, &key0, _points[0].x, DZ_TIMELINE_KEY_RANDOMIZE, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        dz_timeline_key_set_randomize_min_max( key0, _points[0].y, _points[0].y2 );
    }
    else
    {
        return DZ_FAILURE;
    }

    int32_t max = 0;
    while( max < ER_CURVE_MAX_POINTS && _points[max].x >= 0 ) max++;

    dz_timeline_key_t * prevKey = key0;
    for( int32_t i = 1; i < max; i++ )
    {
        dz_timeline_interpolate_t * interpolate;
        if( dz_timeline_interpolate_create( _service, &interpolate, DZ_TIMELINE_INTERPOLATE_LINEAR, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        dz_timeline_key_t * nextKey;

        if( _points[i].mode == ER_CURVE_POINT_MODE_NORMAL )
        {
            if( dz_timeline_key_create( _service, &nextKey, _points[i].x, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR ) == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }

            dz_timeline_key_set_const_value( nextKey, _points[i].y );
        }
        else if( _points[i].mode == ER_CURVE_POINT_MODE_RANDOM )
        {
            if( dz_timeline_key_create( _service, &nextKey, _points[i].x, DZ_TIMELINE_KEY_RANDOMIZE, DZ_NULLPTR ) == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }

            dz_timeline_key_set_randomize_min_max( nextKey, _points[i].y, _points[i].y2 );
        }
        else
        {
            return DZ_FAILURE;
        }

        dz_timeline_key_set_interpolate( prevKey, interpolate );
        dz_timeline_interpolate_set_key( interpolate, nextKey );

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

    dz_timeline_key_set_const_value( timeline, _value );

    dz_emitter_set_timeline( _emitter, _type, timeline );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __reset_emitter_timeline_linear_from_points( dz_service_t * _service, dz_emitter_t * _emitter, dz_emitter_timeline_type_e _type, PointsArray _points )
{
    // first create new timeline
    dz_timeline_key_t * key0;

    if( _points[0].mode == ER_CURVE_POINT_MODE_NORMAL )
    {
        if( dz_timeline_key_create( _service, &key0, _points[0].x, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        dz_timeline_key_set_const_value( key0, _points[0].y );
    }
    else if( _points[0].mode == ER_CURVE_POINT_MODE_RANDOM )
    {
        if( dz_timeline_key_create( _service, &key0, _points[0].x, DZ_TIMELINE_KEY_RANDOMIZE, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        dz_timeline_key_set_randomize_min_max( key0, _points[0].y, _points[0].y2 );
    }
    else
    {
        return DZ_FAILURE;
    }

    int32_t max = 0;
    while( max < ER_CURVE_MAX_POINTS && _points[max].x >= 0 ) max++;

    dz_timeline_key_t * prevKey = key0;
    for( int32_t i = 1; i < max; i++ )
    {
        dz_timeline_interpolate_t * interpolate;
        if( dz_timeline_interpolate_create( _service, &interpolate, DZ_TIMELINE_INTERPOLATE_LINEAR, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        dz_timeline_key_t * nextKey;

        if( _points[i].mode == ER_CURVE_POINT_MODE_NORMAL )
        {
            if( dz_timeline_key_create( _service, &nextKey, _points[i].x, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR ) == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }

            dz_timeline_key_set_const_value( nextKey, _points[i].y );
        }
        else if( _points[i].mode == ER_CURVE_POINT_MODE_RANDOM )
        {
            if( dz_timeline_key_create( _service, &nextKey, _points[i].x, DZ_TIMELINE_KEY_RANDOMIZE, DZ_NULLPTR ) == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }

            dz_timeline_key_set_randomize_min_max( nextKey, _points[i].y, _points[i].y2 );
        }
        else
        {
            return DZ_FAILURE;
        }

        dz_timeline_key_set_interpolate( prevKey, interpolate );
        dz_timeline_interpolate_set_key( interpolate, nextKey );

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

    dz_timeline_key_set_const_value( timeline, _value );

    dz_affector_set_timeline( _affector, _type, timeline );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __reset_affector_timeline_linear_from_points( dz_service_t * _service, dz_affector_t * _affector, dz_affector_timeline_type_e _type, PointsArray _points )
{
    // first create new timeline
    dz_timeline_key_t * key0;
    
    if( _points[0].mode == ER_CURVE_POINT_MODE_NORMAL )
    {
        if( dz_timeline_key_create( _service, &key0, _points[0].x, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        dz_timeline_key_set_const_value( key0, _points[0].y );
    }
    else if( _points[0].mode == ER_CURVE_POINT_MODE_RANDOM )
    {
        if( dz_timeline_key_create( _service, &key0, _points[0].x, DZ_TIMELINE_KEY_RANDOMIZE, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }
        
        dz_timeline_key_set_randomize_min_max( key0, _points[0].y, _points[0].y2 );
    }
    else
    {
        return DZ_FAILURE;
    }

    int32_t max = 0;
    while( max < ER_CURVE_MAX_POINTS && _points[max].x >= 0 ) max++;

    dz_timeline_key_t * prevKey = key0;
    for( int32_t i = 1; i < max; i++ )
    {
        dz_timeline_interpolate_t * interpolate;
        if( dz_timeline_interpolate_create( _service, &interpolate, DZ_TIMELINE_INTERPOLATE_LINEAR, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        dz_timeline_key_t * nextKey;

        if( _points[i].mode == ER_CURVE_POINT_MODE_NORMAL )
        {
            if( dz_timeline_key_create( _service, &nextKey, _points[i].x, DZ_TIMELINE_KEY_CONST, DZ_NULLPTR ) == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }

            dz_timeline_key_set_const_value( nextKey, _points[i].y );
        }
        else if( _points[i].mode == ER_CURVE_POINT_MODE_RANDOM )
        {
            if( dz_timeline_key_create( _service, &nextKey, _points[i].x, DZ_TIMELINE_KEY_RANDOMIZE, DZ_NULLPTR ) == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }

            dz_timeline_key_set_randomize_min_max( nextKey, _points[i].y, _points[i].y2 );
        }
        else
        {
            return DZ_FAILURE;
        }

        dz_timeline_key_set_interpolate( prevKey, interpolate );
        dz_timeline_interpolate_set_key( interpolate, nextKey );

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
    : m_windowWidth( ER_WINDOW_WIDTH )
    , m_windowHeight( ER_WINDOW_HEIGHT )

    , m_dzWindowPos( 0.f, 0.f )
    , m_dzWindowSize( 500.f, 500.f )

    , m_backgroundColor( 0.f, 0.f, 0.f, 1.f )

    , m_showDebugInfo( false )
    , m_showCanvasLines( false )
    , m_pause( false )

    , m_textureWidth( 0 )
    , m_textureHeight( 0 )

    , m_service( nullptr )
    , m_atlas( nullptr )
    , m_texture( nullptr )
    , m_material( nullptr )
    
    , m_shape( nullptr )
    , m_emitter( nullptr )
    , m_affector( nullptr )

    , m_loop( DZ_FALSE )

    , m_effect( nullptr )
    , m_instance( nullptr )
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
            return DZ_FAILURE;
        }

        glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
        glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE );
        glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
        glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );

        m_fwWindow = glfwCreateWindow( (uint32_t)m_windowWidth, (uint32_t)m_windowHeight, ER_TITLE, 0, 0 );

        glfwSetWindowUserPointer( m_fwWindow, this );

        if( m_fwWindow == 0 )
        {
            glfwTerminate();

            return DZ_FAILURE;
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
            return DZ_FAILURE;
        }

        glfwSwapInterval( 1 );
    }

    // init opengl
    {
        uint32_t max_vertex_count = 8196 * 2;
        uint32_t max_index_count = 32768;

        if( dz_render_initialize( &m_openglDesc, max_vertex_count, max_index_count ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        dz_render_set_proj( &m_openglDesc, -(float)m_dzWindowSize.x * 0.5f, (float)m_dzWindowSize.x * 0.5f, -(float)m_dzWindowSize.y * 0.5f, (float)m_dzWindowSize.y * 0.5f );

        m_textureId = 0;
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
            return DZ_FAILURE;
        }

        if( dz_atlas_create( m_service, &m_atlas, &m_textureId, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        if( dz_texture_create( m_service, &m_texture, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        dz_atlas_add_texture( m_atlas, m_texture );

        if( dz_material_create( m_service, &m_material, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        dz_blend_type_e blend_type = dz_material_get_default_blend();

        dz_material_set_blend( m_material, blend_type );
        dz_material_set_atlas( m_material, m_atlas );

        // shape data
        if( dz_shape_create( m_service, &m_shape, m_shapeType, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        for( uint32_t index = 0; index != __DZ_SHAPE_TIMELINE_MAX__; ++index )
        {
            timeline_shape_t & data = m_timelineShapeData[index];

            data.type = static_cast<dz_shape_timeline_type_e>(index);
            data.name = ER_SHAPE_DATA_NAMES[index];

            dz_timeline_limit_status_e status; float min = 0.f, max = 0.f, default = 0.f, factor = 0.f;
            dz_shape_timeline_get_limit( data.type, &status, &min, &max, &default, &factor );
            
            data.zoom = 1;

            data.selectedPoint = ER_CURVE_POINT_NONE;

            if( __set_shape_timeline_const( m_service, m_shape, data.type, default ) == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }

            data.pointsData[0].x = 0.f;
            data.pointsData[0].y = default;

            data.pointsData[1].x = -1.f; // init data so editor knows to take it from here
        }

        // emitter data
        if( dz_emitter_create( m_service, &m_emitter, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        dz_emitter_set_life( m_emitter, 1000.f );

        for( uint32_t index = 0; index != __DZ_EMITTER_TIMELINE_MAX__; ++index )
        {
            timeline_emitter_t & data = m_timelineEmitterData[index];

            data.type = static_cast<dz_emitter_timeline_type_e>(index);
            data.name = ER_EMITTER_DATA_NAMES[index];

            dz_timeline_limit_status_e status; float min = 0.f, max = 0.f, default = 0.f, factor = 0.f;
            dz_emitter_timeline_get_limit( data.type, &status, &min, &max, &default, &factor );

            data.zoom = 1;

            data.selectedPoint = ER_CURVE_POINT_NONE;

            if( __set_emitter_timeline_const( m_service, m_emitter, data.type, default ) == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }

            data.pointsData[0].x = 0.f;
            data.pointsData[0].y = default;

            data.pointsData[1].x = -1.f; // init data so editor knows to take it from here
        }

        // affector data
        if( dz_affector_create( m_service, &m_affector, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        for( uint32_t index = 0; index != __DZ_AFFECTOR_TIMELINE_MAX__; ++index )
        {
            timeline_affector_t & data = m_timelineAffectorData[index];

            data.type = static_cast<dz_affector_timeline_type_e>(index);
            data.name = ER_AFFECTOR_DATA_NAMES[index];

            dz_timeline_limit_status_e status; float min = 0.f, max = 0.f, default = 0.f, factor = 0.f;
            dz_affector_timeline_get_limit( data.type, &status, &min, &max, &default, &factor );

            data.zoom = 1;

            data.selectedPoint = ER_CURVE_POINT_NONE;

            if( __set_affector_timeline_const( m_service, m_affector, data.type, default ) == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }

            data.pointsData[0].x = 0.f;
            data.pointsData[0].y = default;

            data.pointsData[1].x = -1.f; // init data so editor knows to take it from here
        }

        // emitter
        if( dz_effect_create( m_service, &m_effect, m_material, m_shape, m_emitter, m_affector, 5.f, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        if( dz_instance_create( m_service, &m_instance, m_effect, 0, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        dz_instance_set_loop( m_instance, m_loop );
    }

    // init imgui
    {
        ImGuiContext * context = ImGui::CreateContext();
        (void)context;

        if( context == nullptr )
        {
            return DZ_FAILURE;
        }

        if( ImGui_ImplGlfw_InitForOpenGL( m_fwWindow, true ) == false )
        {
            return DZ_FAILURE;
        }

        const char * glsl_version = "#version 330";

        if( ImGui_ImplOpenGL3_Init( glsl_version ) == false )
        {
            return DZ_FAILURE;
        }

        if( ImGui_ImplOpenGL3_CreateFontsTexture() == false )
        {
            return DZ_FAILURE;
        }

        if( ImGui_ImplOpenGL3_CreateDeviceObjects() == false )
        {
            return DZ_FAILURE;
        }
    }

    return DZ_SUCCESSFUL;
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
        if( ImGui::Begin( "LAYOUT", NULL, window_flags ) )
        {
            // menu bar
            if( this->showMenuBar() == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }

            // left panel
            static int selected = ER_WINDOW_TYPE_EFFECT_DATA;
            {
                ImGui::BeginChild( "LEFT_PANEL", ImVec2( 150, 0 ), true );

                for( int32_t selectableIdx = 0; selectableIdx < IM_ARRAYSIZE( ER_WINDOW_TYPE_NAMES ); selectableIdx++ )
                {
                    const bool isSelected = selected == selectableIdx;
                    if( ImGui::Selectable( ER_WINDOW_TYPE_NAMES[selectableIdx], isSelected ) )
                    {
                        selected = selectableIdx;
                    }
                }

                ImGui::EndChild();
            }
            ImGui::SameLine();

            // middle panel
            {
                ImGui::BeginGroup();

                // elements
                ImGui::BeginChild( "WINDOW_SELECT", ImVec2( ER_TIMELINE_PANEL_WIDTH, 0.f ), true );

                switch( selected )
                {
                case ER_WINDOW_TYPE_EFFECT_DATA:
                    {
                        if( this->showEffectData() == DZ_FAILURE )
                        {
                            return DZ_FAILURE;
                        }
                    } 
                    break;
                case ER_WINDOW_TYPE_SHAPE_DATA:
                    {
                        if( this->showShapeData() == DZ_FAILURE )
                        {
                            return DZ_FAILURE;
                        }
                    } 
                    break;
                case ER_WINDOW_TYPE_AFFECTOR_DATA:
                    {
                        if( this->showAffectorData() == DZ_FAILURE )
                        {
                            return DZ_FAILURE;
                        }
                    }
                    break;
                case ER_WINDOW_TYPE_EMITTER_DATA:
                    {
                        if( this->showEmitterData() == DZ_FAILURE )
                        {
                            return DZ_FAILURE;
                        }
                    }
                    break;
                case ER_WINDOW_TYPE_MATERIAL_DATA:
                    {
                        if( this->showMaterialData() == DZ_FAILURE )
                        {
                            return DZ_FAILURE;
                        }
                    }
                    break;
                }

                ImGui::EndChild();

                ImGui::EndGroup();
            }
            ImGui::SameLine();

            // right panel
            {
                ImGui::BeginGroup();

                ImGui::BeginChild( "ITEM_VIEW", ImVec2( 0, 0 ), true );

                if ( this->showContentPane() == DZ_FAILURE )
                {
                    return DZ_FAILURE;
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
        dz_instance_update( m_service, m_instance, 0.005f );
    }

    // update camera
    dz_render_set_camera( &m_openglDesc, camera_offset_x, camera_offset_y, camera_scale );

    return DZ_SUCCESSFUL;
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

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
int editor::run( int argc, char ** argv )
{
    DZ_UNUSED( argc );
    DZ_UNUSED( argv );

    if( this->init() == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    // update loop
    while( glfwWindowShouldClose( m_fwWindow ) == 0 )
    {
        if( this->update() == DZ_FAILURE )
        {
            return DZ_FAILURE; // maybe break loop?
        }

        if( this->render() == DZ_FAILURE )
        {
            return DZ_FAILURE; // maybe break loop?
        }
    }

    this->finalize();

    return DZ_SUCCESSFUL;
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

    dz_instance_reset( m_instance );

    dz_instance_emit_resume( m_instance );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
int editor::saveEffect()
{
    nfdchar_t * outPath = NULL;
    nfdresult_t result = NFD_SaveDialog( "dz", nullptr, &outPath );

    if( result == NFD_OKAY )
    {
        puts( "Success!" );
        puts( outPath );

        jpp::object json = dz_evict_write( m_effect );

        std::string dumpJson;

        this->dumpJSON_( json, &dumpJson, /*_needCompactDump*/ false );

        zipFile zf = zipOpen64( outPath, 0 );

        if( addZipFile( zf, "data.json", dumpJson.data(), dumpJson.size() ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        if( addZipFile( zf, "atlas.png", m_atlasBuffer.data(), m_atlasBuffer.size() ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        if( zipClose( zf, NULL ) != ZIP_OK )
        {
            return DZ_FAILURE;
        }

        free( outPath );
    }
    else if( result == NFD_CANCEL )
    {
        puts( "User pressed cancel." );
    }
    else
    {
        printf( "Error: %s\n", NFD_GetError() );
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
int editor::loadEffect()
{
    nfdchar_t * outPath = NULL;
    nfdresult_t result = NFD_OpenDialog( NULL, NULL, &outPath );

    if( result == NFD_OKAY )
    {
        puts( "Success!" );
        puts( outPath );

        unzFile uf = unzOpen64( outPath );

        unz_global_info64 gi;
        if( unzGetGlobalInfo64( uf, &gi ) != UNZ_OK )
        {
            return DZ_FAILURE;
        }

        std::vector<uint8_t> data_buffer;
        openZipFile( uf, "data.json", &data_buffer );
                
        jpp::object data;
        this->loadJSON_( data_buffer.data(), data_buffer.size(), &data );

        dz_effect_destroy( m_service, m_effect );
        dz_emitter_destroy( m_service, m_emitter );
        dz_affector_destroy( m_service, m_affector );
        dz_shape_destroy( m_service, m_shape );
        dz_instance_destroy( m_service, m_instance );

        m_effect = nullptr;
        m_emitter = nullptr;
        m_affector = nullptr;
        m_shape = nullptr;
        m_instance = nullptr;

        if( dz_evict_load( m_service, &m_effect, data ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        if( dz_instance_create( m_service, &m_instance, m_effect, 0, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        m_material = const_cast<dz_material_t *>(dz_effect_get_material( m_effect ));

        m_atlas = const_cast<dz_atlas_t *>(dz_material_get_atlas( m_material ));

        if( dz_atlas_get_texture( m_atlas, 0, const_cast<const dz_texture_t * *>(&m_texture) ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        m_shape = const_cast<dz_shape_t *>(dz_effect_get_shape( m_effect ));
        m_emitter = const_cast<dz_emitter_t *>(dz_effect_get_emitter( m_effect ));
        m_affector = const_cast<dz_affector_t *>(dz_effect_get_affector( m_effect ));

        m_shapeType = dz_shape_get_type( m_shape );

        m_atlasBuffer.clear();

        openZipFile( uf, "atlas.png", &m_atlasBuffer );

        dz_render_delete_texture( m_textureId );

        m_textureId = dz_render_make_texture_from_memory( m_atlasBuffer.data(), m_atlasBuffer.size(), &m_textureWidth, &m_textureHeight );

        dz_atlas_set_surface( m_atlas, &m_textureId );

        unzCloseCurrentFile( uf );

        free( outPath );

        if( this->resetEffectData() == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }
    }
    else if( result == NFD_CANCEL )
    {
        puts( "User pressed cancel." );
    }
    else
    {
        printf( "Error: %s\n", NFD_GetError() );
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
int editor::exportEffect()
{
    nfdchar_t * outPath = NULL;
    nfdresult_t result = NFD_SaveDialog( NULL, NULL, &outPath );

    if( result == NFD_OKAY )
    {
        puts( "Success!" );
        puts( outPath );

        FILE * f = fopen( outPath, "wb" );

        auto lambda_write = []( const void * _data, dz_size_t _size, dz_userdata_t _ud )
        {
            FILE * f = (FILE *)(_ud);

            fwrite( _data, _size, 1, f );

            return DZ_SUCCESSFUL;
        };

        dz_header_write( lambda_write, (dz_userdata_t)f );

        dz_effect_write( m_effect, lambda_write, (dz_userdata_t)f );

        fclose( f );

        free( outPath );
    }
    else if( result == NFD_CANCEL )
    {
        puts( "User pressed cancel." );
    }
    else
    {
        printf( "Error: %s\n", NFD_GetError() );
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
int editor::showMenuBar()
{
    if( ImGui::BeginMenuBar() )
    {
        if( ImGui::BeginMenu( ER_MENU_FILE ) )
        {
            if( ImGui::MenuItem( ER_MENU_FILE_ITEM_OPEN ) ) // todo
            {
                if( this->loadEffect() == DZ_FAILURE )
                {
                    return DZ_FAILURE;
                }
            }
            if( ImGui::MenuItem( ER_MENU_FILE_ITEM_SAVE ) ) // todo
            {
                if( this->saveEffect() == DZ_FAILURE )
                {
                    return DZ_FAILURE;
                }
            }
            if( ImGui::MenuItem( ER_MENU_FILE_ITEM_EXPORT ) ) // todo
            {
                if( this->exportEffect() == DZ_FAILURE )
                {
                    return DZ_FAILURE;
                }
            }
            ImGui::EndMenu();
        }
        if( ImGui::BeginMenu( ER_MENU_EDIT ) )
        {
            if( ImGui::MenuItem( ER_MENU_EDIT_ITEM_UNDO, NULL, false, false ) ) // todo
            {
            }
            if( ImGui::MenuItem( ER_MENU_EDIT_ITEM_REDO, NULL, false, false ) ) // todo
            {
            }
            ImGui::Separator();

            ImGui::MenuItem( ER_MENU_EDIT_ITEM_SHOW_DEBUG_INFO, "~", &m_showDebugInfo );

            ImGui::MenuItem( ER_MENU_EDIT_ITEM_SHOW_CANVAS_LINES, NULL, &m_showCanvasLines );

            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static void __pointsDataToCurve( er_curve_point_t * _pointsData, er_curve_point_t * _pointsCurve, float _min, float _range )
{
    int end = 0;
    for( ; end < ER_CURVE_MAX_POINTS && _pointsData[end].x >= 0; end++ )
    {
        _pointsCurve[end].x = _pointsData[end].x;
        _pointsCurve[end].y = (_pointsData[end].y - _min) / _range;
        _pointsCurve[end].y2 = (_pointsData[end].y2 - _min) / _range;
        _pointsCurve[end].mode = _pointsData[end].mode;
    }
    _pointsCurve[end].x = -1;
};
//////////////////////////////////////////////////////////////////////////
static void __pointsDataToCurveInv( er_curve_point_t * _pointsData, er_curve_point_t * _pointsCurve, float _min, float _range )
{
    int end = 0;
    for( ; end < ER_CURVE_MAX_POINTS && _pointsData[end].x >= 0; end++ )
    {
        _pointsCurve[end].x = _pointsData[end].x;
        _pointsCurve[end].y = 1.f / ((_pointsData[end].y - _min) * _range);
        _pointsCurve[end].y2 = 1.f / ((_pointsData[end].y2 - _min) * _range);
        _pointsCurve[end].mode = _pointsData[end].mode;
    }
    _pointsCurve[end].x = -1;
};
//////////////////////////////////////////////////////////////////////////
static void __pointsCurveToData( er_curve_point_t * _pointsCurve, er_curve_point_t * _pointsData, float _min, float _range )
{
    int end = 0;
    for( ; end < ER_CURVE_MAX_POINTS && _pointsCurve[end].x >= 0; end++ )
    {
        _pointsData[end].x = _pointsCurve[end].x;
        _pointsData[end].y = _min + (_pointsCurve[end].y * _range);
        _pointsData[end].y2 = _min + (_pointsCurve[end].y2 * _range);
        _pointsData[end].mode = _pointsCurve[end].mode;
    }
    _pointsData[end].x = -1;
};
//////////////////////////////////////////////////////////////////////////
static void __pointsCurveToDataInv( er_curve_point_t * _pointsCurve, er_curve_point_t * _pointsData, float _min, float _range )
{
    int end = 0;
    for( ; end < ER_CURVE_MAX_POINTS && _pointsCurve[end].x >= 0; end++ )
    {
        _pointsData[end].x = _pointsCurve[end].x;
        _pointsData[end].y = _min + 1.f / (_pointsCurve[end].y * _range);
        _pointsData[end].y2 = _min + 1.f / (_pointsCurve[end].y2 * _range);
        _pointsData[end].mode = _pointsCurve[end].mode;
    }
    _pointsData[end].x = -1;
};
//////////////////////////////////////////////////////////////////////////
static void __setupLimits( er_curve_point_t * _pointsData, dz_timeline_limit_status_e _status, float _min, float _max, float * _factor, int32_t * _zoom, float * _y_min, float * _y_max )
{
    if( _status != DZ_TIMELINE_LIMIT_NORMAL )
    {
        // zoom up
        int32_t nextZoomUp = *_zoom * 2;

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
            if( ImGui::Button( ER_CURVE_BTN_ZOOM_UP_TEXT ) == true )
            {
                *_zoom = nextZoomUp;
            }
        }

        // zoom down
        int32_t nextZoomDown = *_zoom / 2;

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
        for( ; end < ER_CURVE_MAX_POINTS && _pointsData[end].x >= 0; end++ )
        {
            if( _pointsData[end].mode == ER_CURVE_POINT_MODE_NORMAL )
            {
                if( _pointsData[end].y < y_min_down || _pointsData[end].y > y_max_down )
                {
                    availableZoomDown = false;
                    break;
                }
            }
            else if( _pointsData[end].mode == ER_CURVE_POINT_MODE_RANDOM )
            {
                if( _pointsData[end].y < y_min_down || _pointsData[end].y2 < y_min_down || _pointsData[end].y > y_max_down || _pointsData[end].y2 > y_max_down )
                {
                    availableZoomDown = false;
                    break;
                }
            }
        }

        if( availableZoomDown == true )
        {
            ImGui::SameLine();
            if( ImGui::Button( ER_CURVE_BTN_ZOOM_DOWN_TEXT ) == true )
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
static void __setupLimitsInv( er_curve_point_t * _pointsData, dz_timeline_limit_status_e _status, float _min, float _max, float * _factor, int32_t * _zoom, float * _y_min, float * _y_max )
{
    if( _status != DZ_TIMELINE_LIMIT_NORMAL )
    {
        float max_value = 0.f;
        {
            int end = 0;
            for( ; end < ER_CURVE_MAX_POINTS && _pointsData[end].x >= 0; end++ )
            {
                float value = 1.f / _pointsData[end].y;
                if( value > max_value )
                {
                    max_value = value;
                }
            }
        }

        float up_limit = *_zoom * (*_factor);
        while( max_value > up_limit)
        {
            (*_zoom)++;
            up_limit = *_zoom * (*_factor);
        }

        // zoom up
        {
            int32_t nextZoomUp = *_zoom * 2;

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
                if( ImGui::Button( ER_CURVE_BTN_ZOOM_UP_TEXT ) == true )
                {
                    *_zoom = nextZoomUp;
                }
            }
        }

        // zoom down
        {
            int32_t nextZoomDown = *_zoom / 2;

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
            for( ; end < ER_CURVE_MAX_POINTS && _pointsData[end].x >= 0; end++ )
            {
                float value = 1.f / _pointsData[end].y;
                if( value < y_min_down || value > y_max_down )
                {
                    availableZoomDown = false;
                    break;
                }
            }

            if( availableZoomDown == true )
            {
                ImGui::SameLine();
                if( ImGui::Button( ER_CURVE_BTN_ZOOM_DOWN_TEXT ) == true )
                {
                    *_zoom = nextZoomDown;
                }
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
static int __setupCurve( const char * _label, const ImVec2 & _size, const int _maxpoints, er_curve_point_t * _points, int * _selected, float _x_min = 0.f, float _x_max = 1.f, float _y_min = 0.f, float _y_max = 1.f )
{
    int modified = 0;
    int i;
    if( _maxpoints < 2 || _points == 0 )
        return 0;

    if( _points[0].x < 0 )
    {
        _points[0].x = 0;
        _points[0].y = 0;
        _points[1].x = 1;
        _points[1].y = 1;
        _points[2].x = -1;
    }

    ImGuiWindow * window = ImGui::GetCurrentWindow();
    ImGuiContext & g = *GImGui;
    const ImGuiStyle & style = g.Style;
    const ImGuiID id = window->GetID( _label );
    if( window->SkipItems )
        return 0;

    ImRect bb( window->DC.CursorPos, window->DC.CursorPos + _size );
    ImGui::ItemSize( bb );
    if( !ImGui::ItemAdd( bb, NULL ) )
        return 0;

    const bool hovered = ImGui::IsItemHovered();

    int max = 0;
    while( max < _maxpoints && _points[max].x >= 0 ) max++;

    ImGui::RenderFrame( bb.Min, bb.Max, ImGui::GetColorU32( ImGuiCol_FrameBg, 1 ), true, style.FrameRounding );

    float ht = bb.Max.y - bb.Min.y;
    float wd = bb.Max.x - bb.Min.x;

    static ImGuiID active_id = ER_CURVE_ID_NONE;
    static int active_point = ER_CURVE_POINT_NONE;
    static bool is_active_y2 = false;
    static bool is_point_added = false;

    if( g.IO.MouseReleased[0] == true )
    {
        active_id = ER_CURVE_ID_NONE;
        active_point = ER_CURVE_POINT_NONE;
        is_active_y2 = false;
        is_point_added = false;
    }

    bool isMoved = false;

    if( active_id == id && active_point != ER_CURVE_POINT_NONE )
    {
        modified = 1;
        ImVec2 pos = (g.IO.MousePos - bb.Min) / (bb.Max - bb.Min);
        pos.y = 1 - pos.y;

        if( pos.x < 0.f || active_point == 0 )
        {
            pos.x = 0.f;
        }

        if( pos.x > 1.f || (max > 1 && active_point == max - 1) )
        {
            pos.x = 1.f;
        }

        if( pos.y < 0.f )
        {
            pos.y = 0.f;
        }

        if( pos.y > 1.f )
        {
            pos.y = 1.f;
        }

        _points[active_point].x = pos.x;

        if( is_active_y2 == true )
        {
            if( pos.y < _points[active_point].y )
            {
                _points[active_point].y2 = _points[active_point].y;
            }
            else
            {
                _points[active_point].y2 = pos.y;
            }
        }
        else
        {
            if( _points[active_point].mode == ER_CURVE_POINT_MODE_NORMAL )
            {
                _points[active_point].y = pos.y;
            }
            else if( _points[active_point].mode == ER_CURVE_POINT_MODE_RANDOM )
            {
                if( pos.y > _points[active_point].y2 )
                {
                    _points[active_point].y = _points[active_point].y2;
                }
                else
                {
                    _points[active_point].y = pos.y;
                }
            }
        }

        if( max > 2 )
        {
            if( active_point - 1 > 0 && _points[active_point].x < _points[active_point - 1].x )
            {
                _points[active_point].x = _points[active_point - 1].x;
            }
            else if( active_point + 1 < max - 1 && _points[active_point].x > _points[active_point + 1].x )
            {
                _points[active_point].x = _points[active_point + 1].x;
            }
        }

        isMoved = true;
    }
    else if( hovered && active_id == ER_CURVE_ID_NONE )
    {
        ImGui::SetHoveredID( id );
        if( g.IO.MouseDown[0] == true )
        {
            modified = 1;
            ImVec2 pos = (g.IO.MousePos - bb.Min) / (bb.Max - bb.Min);
            pos.y = 1 - pos.y;

            int left = 0;
            while( left < max && _points[left].x < pos.x ) left++;
            if( left ) left--;

            if( g.IO.KeyCtrl == true && is_point_added == false )
            {
                // add new point
                if( max < _maxpoints && max == 1 && _maxpoints > 2 )
                {
                    _points[1].x = pos.x;
                    _points[1].y = pos.y;
                    _points[1].mode = ER_CURVE_POINT_MODE_NORMAL;
                    _points[2].x = 1.f;
                    _points[2].y = _points[0].y;
                    _points[2].y2 = _points[0].y2;
                    _points[2].mode = ER_CURVE_POINT_MODE_NORMAL;

                    *_selected = 1;

                    max = 3;
                }
                else if( max < _maxpoints )
                {
                    max++;
                    for( i = max; i > left; i-- )
                    {
                        _points[i] = _points[i - 1];
                    }
                    _points[left + 1].x = pos.x;
                    _points[left + 1].y = pos.y;
                    _points[left + 1].mode = ER_CURVE_POINT_MODE_NORMAL;

                    *_selected = left + 1;
                }
                if( max < _maxpoints )
                    _points[max].x = -1;

                active_id = ER_CURVE_ID_NONE;
                active_point = ER_CURVE_POINT_NONE;
                is_active_y2 = false;

                is_point_added = true;
            }
            else
            {
                int sel = -1;

                if( active_point == ER_CURVE_POINT_NONE )
                {
                    ImVec2 p( _points[left].x, _points[left].y );
                    p = p - pos;

                    ImVec2 p_y2( _points[left].x, _points[left].y2 );
                    p_y2 = p_y2 - pos;

                    if( max == 1 )
                    {
                        if( _points[left].mode == ER_CURVE_POINT_MODE_NORMAL )
                        {
                            float p1d = abs( p.y );
                            if( p1d < (1.f / 16.f) ) sel = left;
                        }
                        else if( _points[left].mode == ER_CURVE_POINT_MODE_RANDOM )
                        {
                            float p1d = abs( p.y );
                            if( p1d < (1.f / 16.f) )
                            {
                                sel = left;
                                is_active_y2 = false;
                            }

                            float p1d_y2 = abs( p_y2.y );
                            if( p1d_y2 < (1.f / 16.f) )
                            {
                                sel = left;
                                is_active_y2 = true;
                            }
                        }
                    }
                    else
                    {
                        float p1d = sqrt( p.x * p.x + p.y * p.y );
                        float p1d_y2 = sqrt( p_y2.x * p_y2.x + p_y2.y * p_y2.y );
                        p.x = _points[left + 1].x;
                        p.y = _points[left + 1].y;
                        p = p - pos;
                        p_y2.x = _points[left + 1].x;
                        p_y2.y = _points[left + 1].y2;
                        p_y2 = p_y2 - pos;
                        float p2d = sqrt( p.x * p.x + p.y * p.y );
                        float p2d_y2 = sqrt( p_y2.x * p_y2.x + p_y2.y * p_y2.y );

                        if( _points[left].mode == ER_CURVE_POINT_MODE_NORMAL )
                        {
                            if( p1d < (1.f / 16.f) ) sel = left;
                        }
                        else if( _points[left].mode == ER_CURVE_POINT_MODE_RANDOM )
                        {
                            if( p1d < (1.f / 16.f) )
                            {
                                sel = left;
                                is_active_y2 = false;
                            }

                            if( p1d_y2 < (1.f / 16.f) )
                            {
                                sel = left;
                                is_active_y2 = true;
                            }
                        }

                        if( _points[left + 1].mode == ER_CURVE_POINT_MODE_NORMAL )
                        {
                            if( p2d < (1.f / 16.f) ) sel = left + 1;
                        }
                        else if( _points[left + 1].mode == ER_CURVE_POINT_MODE_RANDOM )
                        {
                            if( p2d < (1.f / 16.f) )
                            {
                                sel = left + 1;
                                is_active_y2 = false;
                            }

                            if( p2d_y2 < (1.f / 16.f) )
                            {
                                sel = left + 1;
                                is_active_y2 = true;
                            }
                        }
                    }

                    active_id = id;
                    active_point = sel;
                    *_selected = sel;
                }
                else
                {
                    sel = active_point;
                }

                if( sel != -1 )
                {
                    if( g.IO.KeyAlt && max > 1 )
                    {
                        // kill selected
                        modified = 1;
                        for( i = sel + 1; i < max; i++ )
                        {
                            _points[i - 1] = _points[i];
                        }
                        max--;
                        _points[max].x = -1;

                        active_id = ER_CURVE_ID_NONE;
                        active_point = ER_CURVE_POINT_NONE;
                        *_selected = ER_CURVE_POINT_NONE;
                    }
                    else
                    {
                        _points[sel].x = pos.x;

                        if( is_active_y2 == true )
                        {
                            _points[sel].y2 = pos.y;
                        }
                        else
                        {
                            _points[sel].y = pos.y;
                        }

                        isMoved = true;
                    }
                }
            }

            // snap first/last to min/max
            if( _points[0].x < _points[max - 1].x )
            {
                _points[0].x = 0.f;
                _points[max - 1].x = 1.f;
            }
            else
            {
                _points[0].x = 1.f;
                _points[max - 1].x = 0.f;
            }
        }
    }

    // horizontal grid lines
    window->DrawList->AddLine(
        ImVec2( bb.Min.x, bb.Min.y + ht / 2.f ),
        ImVec2( bb.Max.x, bb.Min.y + ht / 2.f ),
        ImGui::GetColorU32( ImGuiCol_TextDisabled ) );

    window->DrawList->AddLine(
        ImVec2( bb.Min.x, bb.Min.y + ht / 4.f ),
        ImVec2( bb.Max.x, bb.Min.y + ht / 4.f ),
        ImGui::GetColorU32( ImGuiCol_TextDisabled ) );

    window->DrawList->AddLine(
        ImVec2( bb.Min.x, bb.Min.y + ht / 4.f * 3.f ),
        ImVec2( bb.Max.x, bb.Min.y + ht / 4.f * 3.f ),
        ImGui::GetColorU32( ImGuiCol_TextDisabled ) );

    // vertical grid lines
    for( i = 0; i < 9; i++ )
    {
        window->DrawList->AddLine(
            ImVec2( bb.Min.x + (wd / 10.f) * (i + 1), bb.Min.y ),
            ImVec2( bb.Min.x + (wd / 10.f) * (i + 1), bb.Max.y ),
            ImGui::GetColorU32( ImGuiCol_TextDisabled ) );
    }

    ImU32 lineColorIdle = ImGui::GetColorU32( ImGuiCol_PlotLinesHovered );
    ImU32 lineColorActive = ImGui::GetColorU32( ImGuiCol_PlotHistogram );
    ImU32 lineColorSelected = ImGui::GetColorU32( ImVec4( 0.f, 1.f, 0.f, 1.f ) );

    // lines and points
    if( max == 1 )  // draw line when 1 point
    {
        if( _points[i - 1].mode == ER_CURVE_POINT_MODE_NORMAL )
        {
            ImVec2 a( 0.f, _points[0].y );
            ImVec2 b( 1.f, _points[0].y );
            a.y = 1 - a.y;
            b.y = 1 - b.y;
            a = a * (bb.Max - bb.Min) + bb.Min;
            b = b * (bb.Max - bb.Min) + bb.Min;

            if( active_id == id && active_point == 0 )
            {
                window->DrawList->AddLine( a, b, lineColorActive );
            }
            else if( *_selected == 0 )
            {
                window->DrawList->AddLine( a, b, lineColorSelected );
            }
            else
            {
                window->DrawList->AddLine( a, b, lineColorIdle );
            }
        }

        if( _points[0].mode == ER_CURVE_POINT_MODE_RANDOM )
        {
            ImVec2 a( 0.f, _points[0].y2 );
            ImVec2 b( 1.f, _points[0].y2 );

            a.y = 1 - a.y;
            b.y = 1 - b.y;
            a = a * (bb.Max - bb.Min) + bb.Min;
            b = b * (bb.Max - bb.Min) + bb.Min;

            if( active_id == id && active_point == 0 )
            {
                window->DrawList->AddLine( a, b, lineColorActive );
            }
            else if( *_selected == 0 )
            {
                window->DrawList->AddLine( a, b, lineColorSelected );
            }
            else
            {
                window->DrawList->AddLine( a, b, lineColorIdle );
            }
        }
    }
    else
    {
        for( i = 1; i < max; i++ )
        {
            if( _points[i - 1].mode == ER_CURVE_POINT_MODE_NORMAL )
            {
                if( _points[i].mode == ER_CURVE_POINT_MODE_NORMAL )
                {
                    ImVec2 a( _points[i - 1].x, _points[i - 1].y );
                    ImVec2 b( _points[i].x, _points[i].y );
                    a.y = 1 - a.y;
                    b.y = 1 - b.y;
                    a = a * (bb.Max - bb.Min) + bb.Min;
                    b = b * (bb.Max - bb.Min) + bb.Min;
                    window->DrawList->AddLine( a, b, lineColorIdle );
                }
                else if( _points[i].mode == ER_CURVE_POINT_MODE_RANDOM )
                {
                    ImVec2 a( _points[i - 1].x, _points[i - 1].y );
                    ImVec2 b( _points[i].x, _points[i].y );
                    ImVec2 b2( _points[i].x, _points[i].y2 );
                    a.y = 1 - a.y;
                    b.y = 1 - b.y;
                    b2.y = 1 - b2.y;
                    a = a * (bb.Max - bb.Min) + bb.Min;
                    b = b * (bb.Max - bb.Min) + bb.Min;
                    b2 = b2 * (bb.Max - bb.Min) + bb.Min;
                    window->DrawList->AddLine( a, b, lineColorIdle );
                    window->DrawList->AddLine( a, b2, lineColorIdle );
                }
            }
            else if (_points[i - 1].mode == ER_CURVE_POINT_MODE_RANDOM )
            {
                if( _points[i].mode == ER_CURVE_POINT_MODE_NORMAL )
                {
                    ImVec2 a( _points[i - 1].x, _points[i - 1].y );
                    ImVec2 a2( _points[i - 1].x, _points[i - 1].y2 );
                    ImVec2 b( _points[i].x, _points[i].y );
                    a.y = 1 - a.y;
                    a2.y = 1 - a2.y;
                    b.y = 1 - b.y;
                    a = a * (bb.Max - bb.Min) + bb.Min;
                    a2 = a2 * (bb.Max - bb.Min) + bb.Min;
                    b = b * (bb.Max - bb.Min) + bb.Min;
                    window->DrawList->AddLine( a, b, lineColorIdle );
                    window->DrawList->AddLine( a2, b, lineColorIdle );
                }
                else if( _points[i].mode == ER_CURVE_POINT_MODE_RANDOM )
                {
                    ImVec2 a( _points[i - 1].x, _points[i - 1].y );
                    ImVec2 a2( _points[i - 1].x, _points[i - 1].y2 );
                    ImVec2 b( _points[i].x, _points[i].y );
                    ImVec2 b2( _points[i].x, _points[i].y2 );
                    a.y = 1 - a.y;
                    a2.y = 1 - a2.y;
                    b.y = 1 - b.y;
                    b2.y = 1 - b2.y;
                    a = a * (bb.Max - bb.Min) + bb.Min;
                    a2 = a2 * (bb.Max - bb.Min) + bb.Min;
                    b = b * (bb.Max - bb.Min) + bb.Min;
                    b2 = b2 * (bb.Max - bb.Min) + bb.Min;
                    window->DrawList->AddLine( a, b, lineColorIdle );
                    window->DrawList->AddLine( a2, b2, lineColorIdle );
                }
            }
        }

        //if( hovered )
        if( (active_id == ER_CURVE_ID_NONE && hovered == true) || (active_id == id) )
        {
            // control points
            for( i = 0; i < max; i++ )
            {
                ImVec2 p( _points[i].x, _points[i].y );
                p.y = 1.f - p.y;
                p = p * (bb.Max - bb.Min) + bb.Min;
                ImVec2 a = p - ImVec2( 2.f, 2.f );
                ImVec2 b = p + ImVec2( 2.f, 2.f );

                if( active_point == i )
                {
                    window->DrawList->AddRect( a, b, lineColorActive );

                    if( _points[i].mode == ER_CURVE_POINT_MODE_RANDOM )
                    {
                        ImVec2 p2( _points[i].x, _points[i].y2 );
                        p2.y = 1.f - p2.y;
                        p2 = p2 * (bb.Max - bb.Min) + bb.Min;
                        ImVec2 a2 = p2 - ImVec2( 2.f, 2.f );
                        ImVec2 b2 = p2 + ImVec2( 2.f, 2.f );
                        window->DrawList->AddRect( a2, b2, lineColorActive );
                    }
                }
                else if( *_selected == i )
                {
                    window->DrawList->AddRect( a, b, lineColorSelected );

                    if( _points[i].mode == ER_CURVE_POINT_MODE_RANDOM )
                    {
                        ImVec2 p2( _points[i].x, _points[i].y2 );
                        p2.y = 1.f - p2.y;
                        p2 = p2 * (bb.Max - bb.Min) + bb.Min;
                        ImVec2 a2 = p2 - ImVec2( 2.f, 2.f );
                        ImVec2 b2 = p2 + ImVec2( 2.f, 2.f );
                        window->DrawList->AddRect( a2, b2, lineColorSelected );
                    }
                }
                else
                {
                    window->DrawList->AddRect( a, b, lineColorIdle );

                    if( _points[i].mode == ER_CURVE_POINT_MODE_RANDOM )
                    {
                        ImVec2 p2( _points[i].x, _points[i].y2 );
                        p2.y = 1.f - p2.y;
                        p2 = p2 * (bb.Max - bb.Min) + bb.Min;
                        ImVec2 a2 = p2 - ImVec2( 2.f, 2.f );
                        ImVec2 b2 = p2 + ImVec2( 2.f, 2.f );
                        window->DrawList->AddRect( a2, b2, lineColorIdle );
                    }
                }
            }
        }
        else if( *_selected != ER_CURVE_POINT_NONE )
        {
            ImVec2 p( _points[*_selected].x, _points[*_selected].y );
            p.y = 1.f - p.y;
            p = p * (bb.Max - bb.Min) + bb.Min;
            ImVec2 a = p - ImVec2( 2.f, 2.f );
            ImVec2 b = p + ImVec2( 2.f, 2.f );
            window->DrawList->AddRect( a, b, lineColorSelected );

            if( _points[*_selected].mode == ER_CURVE_POINT_MODE_RANDOM )
            {
                ImVec2 p2( _points[*_selected].x, _points[*_selected].y2 );
                p2.y = 1.f - p2.y;
                p2 = p2 * (bb.Max - bb.Min) + bb.Min;
                ImVec2 a2 = p2 - ImVec2( 2.f, 2.f );
                ImVec2 b2 = p2 + ImVec2( 2.f, 2.f );
                window->DrawList->AddRect( a2, b2, lineColorSelected );
            }
        }
    }

    // texts
    ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetColorU32( ImGuiCol_Text, 0.7f ) );

    // - labels on curve
    {
        char buf[256];

        sprintf( buf, "%.2f", _y_min );
        ImGui::RenderTextClipped( ImVec2( bb.Min.x, bb.Min.y + style.FramePadding.y ), bb.Max, buf, NULL, NULL, ImVec2( 0.f, 1.f ) );

        sprintf( buf, "%.2f", _y_max );
        ImGui::RenderTextClipped( ImVec2( bb.Min.x, bb.Min.y + style.FramePadding.y ), bb.Max, buf, NULL, NULL, ImVec2( 0.f, 0.f ) );

        sprintf( buf, "%.2f", _x_max );
        ImGui::RenderTextClipped( ImVec2( bb.Min.x, bb.Min.y + style.FramePadding.y ), bb.Max, buf, NULL, NULL, ImVec2( 1.f, 1.f ) );
    
        // debug text
        //sprintf( buf, "my_id=%d\nactive_id=%d\nactive=%d\nselected=%d\nis_moved=%s\nis_ctrl=%s\nmax=%d"
        //    , id
        //    , active_id
        //    , active_point
        //    , *_selected
        //    , isMoved == true ? "true" : "false"
        //    , g.IO.KeyCtrl == true ? "true" : "false"
        //    , max
        //);
        //ImGui::RenderTextClipped( ImVec2( bb.Min.x, bb.Min.y + style.FramePadding.y ), bb.Max, buf, NULL, NULL, ImVec2( 0.5f, 0.5f ) );
    }

    // - position down curve
    if( hovered )
    {
        ImVec2 pos = (g.IO.MousePos - bb.Min) / (bb.Max - bb.Min);
        pos.y = 1.f - pos.y;

        float x = 0.f;

        if( max > 1 )
        {
            x = _x_min + pos.x * (_x_max - _x_min);
        }

        float y = _y_min + pos.y * (_y_max - _y_min);

        ImGui::Text( "(%.3f,%.3f)", x, y );
    }
    else
    {
        ImGui::Text( "(0.000, 0.000)" );
    }

    ImGui::PopStyleColor( 1 );

    return modified;
}
//////////////////////////////////////////////////////////////////////////
static int __setupSelectCurvePointMode( int _selectedPoint, float _factor, float _min, float _max, er_curve_point_t * _pointsData, er_curve_point_t * _pointsCurve )
{
    int modified = 0;
    if( _selectedPoint != ER_CURVE_POINT_NONE )
    {
        int mode = _pointsCurve[_selectedPoint].mode;
        if( ImGui::Combo( ER_CURVE_COMBO_MODE_LABEL_TEXT, &mode, ER_TIMELINE_KEY_MODE_NAMES, IM_ARRAYSIZE( ER_TIMELINE_KEY_MODE_NAMES ) ) == true )
        {
            er_curve_point_mode_e selected_mode = static_cast<er_curve_point_mode_e>(mode);

            if( selected_mode != _pointsCurve[_selectedPoint].mode )
            {
                if( selected_mode == ER_CURVE_POINT_MODE_NORMAL )
                {
                    float distance = _pointsData[_selectedPoint].y2 - _pointsData[_selectedPoint].y;
                    _pointsData[_selectedPoint].y = _pointsData[_selectedPoint].y + distance / 2.f;
                }
                else if( selected_mode == ER_CURVE_POINT_MODE_RANDOM )
                {
                    float normalValue = _pointsData[_selectedPoint].y;
                    float randMinValue = normalValue - 0.25f * _factor;
                    
                    if( randMinValue < _min )
                    {
                        randMinValue = _min;
                    }

                    float randMaxValue = normalValue + 0.25f * _factor;

                    if( randMaxValue > _max )
                    {
                        randMaxValue = _max;
                    }

                    _pointsData[_selectedPoint].y = randMinValue;
                    _pointsData[_selectedPoint].y2 = randMaxValue;
                }

                _pointsCurve[_selectedPoint].mode = selected_mode;
                _pointsData[_selectedPoint].mode = selected_mode;

                modified = 1;
            }
        }
    }

    return modified;
}
//////////////////////////////////////////////////////////////////////////
int editor::readTimelineKey( const dz_timeline_key_t * _key, er_curve_point_t * _pointsData, size_t _index )
{
    if( _index + 1 >= ER_CURVE_MAX_POINTS )
    {
        return DZ_FAILURE;
    }

    dz_timeline_key_type_e key_type = dz_timeline_key_get_type( _key );

    if( key_type == DZ_TIMELINE_KEY_CONST )
    {
        float p = dz_timeline_key_get_p( _key );

        float const_value;
        dz_timeline_key_get_const_value( _key, &const_value );

        _pointsData[_index].x = p;
        _pointsData[_index].y = const_value;
        _pointsData[_index].y2 = 0.f;
        _pointsData[_index].mode = ER_CURVE_POINT_MODE_NORMAL;


    }
    else if( key_type == DZ_TIMELINE_KEY_RANDOMIZE )
    {
        float p = dz_timeline_key_get_p( _key );

        float randomize_min;
        float randomize_max;
        dz_timeline_key_get_randomize_min_max( _key, &randomize_min, &randomize_max );

        _pointsData[_index].x = p;
        _pointsData[_index].y = randomize_min;
        _pointsData[_index].y2 = randomize_max;
        _pointsData[_index].mode = ER_CURVE_POINT_MODE_RANDOM;
    }
    else
    {
        return DZ_FAILURE;
    }

    _pointsData[_index + 1].x = -1.f; // init data so editor knows to take it from here

    const dz_timeline_interpolate_t * interpolate = dz_timeline_key_get_interpolate( _key );

    if( interpolate != DZ_NULLPTR )
    {
        const dz_timeline_key_t * key = dz_timeline_interpolate_get_key( interpolate );

        if( key != DZ_NULLPTR )
        {
            if( this->readTimelineKey( key, _pointsData, _index + 1 ) == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }
        }
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
int editor::resetEffectData()
{
    //float life = dz_effect_get_life( m_effect );

    // shape data
    for( uint32_t index = 0; index != __DZ_SHAPE_TIMELINE_MAX__; ++index )
    {
        timeline_shape_t & data = m_timelineShapeData[index];

        data.type = static_cast<dz_shape_timeline_type_e>(index);
        data.selectedPoint = ER_CURVE_POINT_NONE;

        const dz_timeline_key_t * key = dz_shape_get_timeline( m_shape, data.type );

        data.pointsData[0].x = -1.f; // init data so editor knows to take it from here

        if( key != DZ_NULLPTR )
        {
            if( this->readTimelineKey( key, data.pointsData, 0 ) == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }
        }

        dz_timeline_limit_status_e status;
        float min = 0.f, max = 0.f, default = 0.f, factor = 0.f;
        dz_shape_timeline_get_limit( data.type, &status, &min, &max, &default, &factor );

        // curve
        float y_min = min;
        float y_max = max;

        __setupLimits( data.pointsData, status, min, max, &factor, &(data.zoom), &y_min, &y_max );

        float y_range = y_max - y_min;

        __pointsDataToCurve( data.pointsData, data.pointsCurve, y_min, y_range );
    }

    for( uint32_t index = 0; index != __DZ_EMITTER_TIMELINE_MAX__; ++index )
    {
        timeline_emitter_t & data = m_timelineEmitterData[index];

        data.type = static_cast<dz_emitter_timeline_type_e>(index);
        data.selectedPoint = ER_CURVE_POINT_NONE;

        const dz_timeline_key_t * key = dz_emitter_get_timeline( m_emitter, data.type );

        data.pointsData[0].x = -1.f; // init data so editor knows to take it from here

        if( key != DZ_NULLPTR )
        {
            if( this->readTimelineKey( key, data.pointsData, 0 ) == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }
        }

        dz_timeline_limit_status_e status;
        float min = 0.f, max = 0.f, default = 0.f, factor = 0.f;
        dz_emitter_timeline_get_limit( data.type, &status, &min, &max, &default, &factor );

        // curve
        float y_min = min;
        float y_max = max;

        __setupLimits( data.pointsData, status, min, max, &factor, &(data.zoom), &y_min, &y_max );

        float y_range = y_max - y_min;

        __pointsDataToCurve( data.pointsData, data.pointsCurve, y_min, y_range );
    }

    // affector 
    for( uint32_t index = 0; index != __DZ_AFFECTOR_TIMELINE_MAX__; ++index )
    {
        timeline_affector_t & data = m_timelineAffectorData[index];

        data.type = static_cast<dz_affector_timeline_type_e>(index);
        data.selectedPoint = ER_CURVE_POINT_NONE;

        const dz_timeline_key_t * key = dz_affector_get_timeline( m_affector, data.type );

        data.pointsData[0].x = -1.f; // init data so editor knows to take it from here

        if( key != DZ_NULLPTR )
        {
            if( this->readTimelineKey( key, data.pointsData, 0 ) == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }
        }

        if( index == DZ_AFFECTOR_TIMELINE_STRAFE_SHIFT )
        {
            continue;
        }

        dz_timeline_limit_status_e status;
        float min = 0.f, max = 0.f, default = 0.f, factor = 0.f;
        dz_affector_timeline_get_limit( data.type, &status, &min, &max, &default, &factor );

        // curve
        float y_min = min;
        float y_max = max;

        __setupLimits( data.pointsData, status, min, max, &factor, &(data.zoom), &y_min, &y_max );

        float y_range = y_max - y_min;

        __pointsDataToCurve( data.pointsData, data.pointsCurve, y_min, y_range );
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
int editor::showEffectData()
{
    ImGui::Spacing();

    ImGui::Text( ER_WINDOW_EFFECT_TITLE );
    ImGui::Separator();

    ImGui::Spacing();

    int seed = dz_instance_get_seed( m_instance );

    if( ImGui::InputInt( ER_WINDOW_EFFECT_SEED_TEXT, &seed, 0, 0, ImGuiInputTextFlags_None ) == true )
    {
        dz_instance_set_seed( m_instance, seed );

        this->resetEffect();
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
int editor::showShapeData()
{
    static int selected_type = m_shapeType;

    // ignore all shape types starts with DZ_SHAPE_POLYGON
    ImGui::Combo( ER_WINDOW_COMBO_SHAPE_TYPE_TEXT, &selected_type, ER_SHAPE_TYPE_NAMES, DZ_SHAPE_POLYGON, DZ_SHAPE_POLYGON );

    if( selected_type != m_shapeType )
    {
        m_shapeType = static_cast<dz_shape_type_e>(selected_type);

        if( this->resetEffect() == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }
    }

    // timeline
    ImGui::Spacing();
    ImGui::Text( ER_WINDOW_SHAPE_TITLE );

    float width = ImGui::GetWindowContentRegionWidth();
    ImVec2 size( width, width * ER_CURVE_BOX_HEIGHT_TO_WIDTH_RATIO );

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

                if( __setupCurve( data.name, size, ER_CURVE_MAX_POINTS, data.pointsCurve, &(data.selectedPoint), x_min, x_max, y_min, y_max ) != 0 )
                {
                    __pointsCurveToData( data.pointsCurve, data.pointsData, y_min, y_range );

                    if( __reset_shape_timeline_linear_from_points( m_service, m_shape, data.type, data.pointsData ) == DZ_FAILURE )
                    {
                        return DZ_FAILURE;
                    }
                }

                if( __setupSelectCurvePointMode( data.selectedPoint, factor, y_min, y_max, data.pointsData, data.pointsCurve ) != 0 )
                {
                    if( __reset_shape_timeline_linear_from_points( m_service, m_shape, data.type, data.pointsData ) == DZ_FAILURE )
                    {
                        return DZ_FAILURE;
                    }
                }
            }

            ImGui::Separator();

            ImGui::PopID();
        }
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
int editor::showAffectorData()
{
    // timeline
    ImGui::Spacing();
    ImGui::Text( ER_WINDOW_AFFECTOR_TITLE );

    float width = ImGui::GetWindowContentRegionWidth();
    ImVec2 size( width, width * ER_CURVE_BOX_HEIGHT_TO_WIDTH_RATIO );

    static bool headerFlags[__DZ_AFFECTOR_TIMELINE_MAX__] = { false };

    ImGui::Separator();

    for( uint32_t index = 0; index != __DZ_AFFECTOR_TIMELINE_MAX__; ++index )
    {
        // ignore
        if( index == DZ_AFFECTOR_TIMELINE_STRAFE_SHIFT )
        {
            continue;
        }

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

            if( __setupCurve( data.name, size, ER_CURVE_MAX_POINTS, data.pointsCurve, &(data.selectedPoint), x_min, x_max, y_min, y_max ) != 0 )
            {
                __pointsCurveToData( data.pointsCurve, data.pointsData, y_min, y_range );

                if( __reset_affector_timeline_linear_from_points( m_service, m_affector, data.type, data.pointsData ) == DZ_FAILURE )
                {
                    return DZ_FAILURE;
                }
            }

            if( __setupSelectCurvePointMode( data.selectedPoint, factor, y_min, y_max, data.pointsData, data.pointsCurve ) != 0 )
            {
                if( __reset_affector_timeline_linear_from_points( m_service, m_affector, data.type, data.pointsData ) == DZ_FAILURE )
                {
                    return DZ_FAILURE;
                }
            }
        }

        ImGui::Separator();

        ImGui::PopID();
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
int editor::showEmitterData()
{   
    // timeline
    ImGui::Spacing();
    ImGui::Text( ER_WINDOW_EMITTER_TITLE );

    float width = ImGui::GetWindowContentRegionWidth();
    ImVec2 size( width, width * ER_CURVE_BOX_HEIGHT_TO_WIDTH_RATIO );

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

            // inv
            if( index == DZ_EMITTER_SPAWN_DELAY )
            {
                __setupLimitsInv( data.pointsData, status, min, max, &factor, &(data.zoom), &y_min, &y_max );

                float y_range = y_max - y_min;

                __pointsDataToCurveInv( data.pointsData, data.pointsCurve, y_min, y_range );

                if( __setupCurve( data.name, size, ER_CURVE_MAX_POINTS, data.pointsCurve, &(data.selectedPoint), x_min, x_max, y_min, y_max ) != 0 )
                {
                    __pointsCurveToDataInv( data.pointsCurve, data.pointsData, y_min, y_range );

                    if( __reset_emitter_timeline_linear_from_points( m_service, m_emitter, data.type, data.pointsData ) == DZ_FAILURE )
                    {
                        return DZ_FAILURE;
                    }
                }

                if( __setupSelectCurvePointMode( data.selectedPoint, factor, y_min, y_max, data.pointsData, data.pointsCurve ) != 0 )
                {
                    if( __reset_emitter_timeline_linear_from_points( m_service, m_emitter, data.type, data.pointsData ) == DZ_FAILURE )
                    {
                        return DZ_FAILURE;
                    }
                }
            }
            else // other
            {
                __setupLimits( data.pointsData, status, min, max, &factor, &(data.zoom), &y_min, &y_max );

                float y_range = y_max - y_min;

                __pointsDataToCurve( data.pointsData, data.pointsCurve, y_min, y_range );

                if( __setupCurve( data.name, size, ER_CURVE_MAX_POINTS, data.pointsCurve, &(data.selectedPoint), x_min, x_max, y_min, y_max ) != 0 )
                {
                    __pointsCurveToData( data.pointsCurve, data.pointsData, y_min, y_range );

                    if( __reset_emitter_timeline_linear_from_points( m_service, m_emitter, data.type, data.pointsData ) == DZ_FAILURE )
                    {
                        return DZ_FAILURE;
                    }
                }

                if( __setupSelectCurvePointMode( data.selectedPoint, factor, y_min, y_max, data.pointsData, data.pointsCurve ) != 0 )
                {
                    if( __reset_emitter_timeline_linear_from_points( m_service, m_emitter, data.type, data.pointsData ) == DZ_FAILURE )
                    {
                        return DZ_FAILURE;
                    }
                }
            }
        }

        ImGui::Separator();

        ImGui::PopID();
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
int editor::showMaterialData()
{
    ImGui::Spacing();
    ImGui::Text( ER_WINDOW_MATERIAL_TITLE );
    ImGui::Separator();

    static int blend_current = dz_material_get_blend( m_material );
    if( ImGui::Combo( ER_WINDOW_MATERIAL_COMBO_BLEND_MODE_TEXT, &blend_current, ER_BLEND_MODE_NAMES, IM_ARRAYSIZE( ER_BLEND_MODE_NAMES ) ) == true )
    {
        dz_material_set_blend( m_material, (dz_blend_type_e)blend_current );
    }

    ImGui::Spacing();
    ImGui::Text( ER_WINDOW_MATERIAL_TEXTURE_TITLE );
    ImGui::Separator();

    ImGui::Text( ER_WINDOW_MATERIAL_TEXTURE_SIZE_LABEL );
    ImGui::SameLine();
    ImGui::Text( "%d x %d", m_textureWidth, m_textureHeight );
    
    ImGui::SameLine();

    if( ImGui::Button( ER_WINDOW_MATERIAL_TEXTURE_BTN_BROWSE ) == true )
    {
        nfdchar_t * texturePath = NULL;
        nfdresult_t result = NFD_OpenDialog( NULL, NULL, &texturePath );

        if( result == NFD_OKAY )
        {
            puts( "Success!" );
            puts( texturePath );

            dz_render_delete_texture( m_textureId );

            m_textureId = dz_render_make_texture( texturePath, &m_textureWidth, &m_textureHeight );

            dz_atlas_set_surface( m_atlas, &m_textureId );

            FILE * f = fopen( texturePath, "rb" );
            fseek( f, 0L, SEEK_END );
            size_t sz = ftell( f );
            rewind( f );

            m_atlasBuffer.resize( sz );
            fread( m_atlasBuffer.data(), sz, 1, f );
            fclose( f );

            free( texturePath );
        }
        else if( result == NFD_CANCEL )
        {
            puts( "User pressed cancel." );
        }
        else
        {
            printf( "Error: %s\n", NFD_GetError() );
        }
    }

    ImGui::Image( (void *)(intptr_t)m_textureId, ImVec2( (float)m_textureWidth, (float)m_textureHeight ) );

    return DZ_SUCCESSFUL;
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

    dz_render_instance( &m_openglDesc, m_instance );

    glViewport( oldViewport[0], oldViewport[1], oldViewport[2], oldViewport[3] );
}
//////////////////////////////////////////////////////////////////////////
bool editor::dumpJSON_( const jpp::object & _json, std::string * _out, bool _needCompactDump )
{
    auto my_jpp_dump_callback = []( const char * _buffer, size_t _size, void * _ud )
    {
        std::string * p_str = static_cast<std::string *>(_ud);

        p_str->append( _buffer, _size );

        return 0;
    };

    bool result = false;

    if( _needCompactDump == true )
    {
        result = jpp::dump_compact( _json, my_jpp_dump_callback, _out );
    }
    else
    {
        result = jpp::dump( _json, my_jpp_dump_callback, _out );
    }

    return result;
}
//////////////////////////////////////////////////////////////////////////
void editor::loadJSON_( const void * _buffer, size_t _size, jpp::object * _out ) const
{
    my_json_load_data_t jd;
    jd.buffer = static_cast<const uint8_t *>(_buffer);
    jd.carriage = 0;
    jd.capacity = _size;

    auto  my_jpp_error = []( int32_t _line, int32_t _column, int32_t _position, const char * _source, const char * _text, void * _ud )
    {
        DZ_UNUSED( _ud );

        printf( "jpp error: %s\nline: %d\n column: %d\nposition: %d\nsource: %s\n"
            , _text
            , _line
            , _column
            , _position
            , _source
        );
    };

    auto my_jpp_load_callback = []( void * _buffer, size_t _buflen, void * _data )
    {
        my_json_load_data_t * jd = static_cast<my_json_load_data_t *>(_data);

        if( _buflen > jd->capacity - jd->carriage )
        {
            _buflen = jd->capacity - jd->carriage;
        }

        if( _buflen <= 0 )
        {
            return (size_t)0;
        }

        const uint8_t * jd_buffer = jd->buffer + jd->carriage;
        std::memcpy( _buffer, jd_buffer, _buflen );
        jd->carriage += _buflen;

        return _buflen;
    };

    jpp::object json = jpp::load( my_jpp_load_callback, my_jpp_error, &jd );

    if( json == jpp::detail::invalid )
    {
        return;
    }

    *_out = json;
}
//////////////////////////////////////////////////////////////////////////
int editor::showContentPane()
{
    // content
    float columnWidth = ImGui::GetColumnWidth();
    float columnHeight = ImGui::GetWindowHeight() - ImGui::GetFrameHeightWithSpacing() * (ER_CONTENT_CONTROLS_PANE_LINES_COUNT + 1);

    m_dzWindowSize.x = columnWidth;
    m_dzWindowSize.y = columnHeight;

    ImGuiWindow * window = ImGui::GetCurrentWindow();
    const ImGuiID id = window->GetID( "DAZZLE_RENDER_CANVAS" );
    if( window->SkipItems )
    {
        return DZ_FAILURE;
    }

    ImVec2 cursorPos = window->DC.CursorPos;

    m_dzWindowPos = ImVec2( cursorPos.x, m_windowHeight - cursorPos.y - m_dzWindowSize.y );

    ImGui::BeginChild( "DAZZLE_CANVAS_WINDOW", m_dzWindowSize );

    window->DrawList->AddRectFilled( cursorPos, cursorPos + m_dzWindowSize, ImGui::GetColorU32( ImGuiCol_FrameBg, 1 ) );

    if( m_showCanvasLines == true )
    {
        const float leftBound = cursorPos.x;
        const float rightBound = cursorPos.x + columnWidth;
        const float upperBound = cursorPos.y;
        const float downBound = cursorPos.y + columnHeight;

        const float halfWidth = columnWidth / 2.f;
        const float halfHeight = columnHeight / 2.f;

        ImVec2 verticalLineStartPos( cursorPos.x + halfWidth + camera_offset_x, cursorPos.y );
        ImVec2 verticalLineEndPos( cursorPos.x + halfWidth + camera_offset_x, cursorPos.y + columnHeight );

        if( verticalLineStartPos.x > leftBound
            && verticalLineStartPos.x < rightBound
            && verticalLineEndPos.x > leftBound
            && verticalLineEndPos.x < rightBound )
        {
            window->DrawList->AddLine( verticalLineStartPos, verticalLineEndPos, ImGui::GetColorU32( ImGuiCol_TextDisabled ) );
        }

        ImVec2 horizontalLineStartPos( cursorPos.x, cursorPos.y + halfHeight + camera_offset_y );
        ImVec2 horizontalLineEndPos( cursorPos.x + columnWidth, cursorPos.y + halfHeight + camera_offset_y );

        if( horizontalLineStartPos.y > upperBound
            && horizontalLineStartPos.y < downBound
            && horizontalLineEndPos.y > upperBound
            && horizontalLineEndPos.y < downBound )
        {
            window->DrawList->AddLine( horizontalLineStartPos, horizontalLineEndPos, ImGui::GetColorU32( ImGuiCol_TextDisabled ) );
        }

        // wip
        //const int linesCount = 20;

        //// vertical grid lines
        //for( int i = 0; i < linesCount - 1; i++ )
        //{
        //    window->DrawList->AddLine(
        //        ImVec2( cursorPos.x + (columnWidth / linesCount) * (i + 1), cursorPos.y ),
        //        ImVec2( cursorPos.x + (columnWidth / linesCount) * (i + 1), cursorPos.y + columnHeight ),
        //        ImGui::GetColorU32( ImGuiCol_TextDisabled ) );
        //}

        //// vertical grid lines
        //for( int i = 0; i < linesCount - 1; i++ )
        //{
        //    window->DrawList->AddLine(
        //        ImVec2( cursorPos.x, cursorPos.y + (columnHeight / linesCount) * (i + 1) ),
        //        ImVec2( cursorPos.x + columnWidth, cursorPos.y + (columnHeight / linesCount) * (i + 1) ),
        //        ImGui::GetColorU32( ImGuiCol_TextDisabled ) );
        //}
    }

    ImGui::GetWindowDrawList()->AddCallback( &__draw_callback, this );
    ImGui::GetWindowDrawList()->AddCallback( ImDrawCallback_ResetRenderState, nullptr );

    if( m_showDebugInfo == true )
    {
        uint32_t particle_count = dz_instance_get_particle_count( m_instance );
        uint32_t particle_limit = dz_instance_get_particle_limit( m_instance );

        char buf[1000];

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
            "      particle_count: %u\n"
            "      particle_limit: %u\n"
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
            , particle_count
            , particle_limit != ~0U ? particle_limit : 0
        );

        window->DrawList->AddText( cursorPos, ImGui::GetColorU32( ImGuiCol_Text, 0.7f ), buf );
    }

    ImGui::EndChild();

    ImGui::Separator();

    // controls
    ImGui::BeginGroup();
    ImGui::BeginChild( "CONTROLS_PANEL" );

    ImGui::Spacing();

    if( this->showContentPaneControls() == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    ImGui::EndChild();
    ImGui::EndGroup();
    
    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
int editor::showContentPaneControls()
{
    float life = dz_effect_get_life( m_effect );
    float time = dz_instance_get_time( m_instance );

    // buttons
    if( ImGui::Button( ER_WINDOW_CONTROLS_BTN_RESET_TEXT ) )
    {
        if( this->resetEffect() == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }
    }
    ImGui::SameLine();

    if( ImGui::Button( ER_WINDOW_CONTROLS_BTN_PAUSE_TEXT ) )
    {
        m_pause = true;
    }
    ImGui::SameLine();

    if( ImGui::Button( ER_WINDOW_CONTROLS_BTN_RESUME_TEXT ) )
    {
        m_pause = false;
    }
    ImGui::SameLine();

    // loop
    bool isLoop = m_loop == DZ_TRUE ? true : false;
    if( ImGui::Checkbox( ER_WINDOW_CONTROLS_BTN_LOOP_TEXT, &isLoop ) == true )
    {
        m_loop = isLoop == true ? DZ_TRUE : DZ_FALSE;

        dz_instance_set_loop( m_instance, m_loop );
    }
    ImGui::SameLine();

    // time
    char label[100];
    sprintf( label, "%s %%.3f s / %.3f s"
        , ER_WINDOW_CONTROLS_TIMELINE_PREFIX_TEXT
        , life
    );

    if( ImGui::SliderFloat( "", &time, 0.0f, life, label ) == true )
    {
        this->resetEffect();

        dz_instance_update( m_service, m_instance, time );
    }
    
    // life
    if( ImGui::InputFloat( ER_WINDOW_CONTROLS_INPUT_LIFE_TEXT, &life, 0.f, 0.f, NULL, ImGuiInputTextFlags_None ) == true )
    {
        dz_effect_set_life( m_effect, life );

        this->resetEffect();
    }
    //ImGui::SameLine();

    // camera
    if( ImGui::Button( ER_WINDOW_CONTROLS_BTN_RESET_CAMERA_TEXT ) )
    {
        camera_scale = 1.f;
        camera_offset_x = 0.f;
        camera_offset_y = 0.f;
    }
    ImGui::SameLine();

    ImGui::Text( ER_WINDOW_CONTROLS_CAMERA_MOVE_HELP_TEXT );

    // emitter states
    {
        ImGui::Text( ER_WINDOW_CONTROLS_EMIT_STATES_LABEL_TEXT );
        ImGui::SameLine();

        dz_instance_state_e emitter_state = dz_instance_get_state( m_instance );

        auto lamdba_addBoolIndicator = []( bool _value, const char * _msg )
        {
            ImVec4 colorGreen( ImColor( 0, 255, 0 ) );
            ImVec4 colorRed( ImColor( 255, 0, 0 ) );

            ImGui::PushStyleColor( ImGuiCol_Text, _value ? colorGreen : colorRed );
            ImGui::Text( _msg );
            ImGui::PopStyleColor( 1 );
            ImGui::SameLine();
        };

        lamdba_addBoolIndicator( emitter_state & DZ_INSTANCE_EMIT_COMPLETE, ER_WINDOW_CONTROLS_EMIT_COMPLETE_STATE_TEXT );
        lamdba_addBoolIndicator( emitter_state & DZ_INSTANCE_PARTICLE_COMPLETE, ER_WINDOW_CONTROLS_PARTICLE_COMPLETE_STATE_TEXT );
    }

    return DZ_SUCCESSFUL;
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
