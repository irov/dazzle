#include "editor.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include "nfd.h"
#include "zip.h"
#include "unzip.h"

#ifdef _WIN32
#   ifdef APIENTRY
#       undef APIENTRY
#   endif
#   define USEWIN32IOAPI
#   include "iowin32.h"
#endif

#include "dazzle/dazzle_read.hpp"
#include "dazzle/dazzle_write.hpp"

#include "stb_image/stb_image.h"
#include "zlib.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <algorithm>
#include <cstring>

//////////////////////////////////////////////////////////////////////////
typedef enum er_window_type_e
{
    ER_WINDOW_TYPE_EFFECT_DATA,
    ER_WINDOW_TYPE_COMPOSER_DATA,
    ER_WINDOW_TYPE_SHAPE_DATA,
    ER_WINDOW_TYPE_AFFECTOR_DATA,
    ER_WINDOW_TYPE_EMITTER_DATA,
    ER_WINDOW_TYPE_MATERIAL_DATA,
    ER_WINDOW_TYPE_ATLAS_DATA,

    __ER_WINDOW_TYPE_MAX__
} er_window_type_e;
//////////////////////////////////////////////////////////////////////////
//static constexpr dz_uint32_t ER_WINDOW_WIDTH = 1024;  // aspect ratio 3:4
//static constexpr dz_uint32_t ER_WINDOW_HEIGHT = 768;
static constexpr dz_uint32_t ER_WINDOW_WIDTH = 1280;    // aspect ratio HD 720p
static constexpr dz_uint32_t ER_WINDOW_HEIGHT = 720;
static constexpr dz_float_t ER_TIMELINE_PANEL_WIDTH = 430.f;
static constexpr int32_t ER_CONTENT_CONTROLS_PANE_LINES_COUNT = 5;
static constexpr ImGuiID ER_CURVE_ID_NONE = 0;
static constexpr int ER_CURVE_POINT_NONE = -1;
static constexpr dz_float_t ER_CURVE_PLOT_BORDER_SIZE = 8.f;
static constexpr dz_float_t ER_CURVE_PLOT_HOVER_RADIUS = 4.f;
static constexpr dz_float_t ER_CURVE_PLOT_HOVER_RADIUS_POW_2 = ER_CURVE_PLOT_HOVER_RADIUS * ER_CURVE_PLOT_HOVER_RADIUS;
static constexpr dz_uint32_t ER_ATLAS_TEXTURE_MAX = 64;
static constexpr dz_float_t ER_LAYER_GIZMO_HIT_RADIUS = 14.f;
static constexpr dz_float_t ER_LAYER_GIZMO_HIT_RADIUS_POW_2 = ER_LAYER_GIZMO_HIT_RADIUS * ER_LAYER_GIZMO_HIT_RADIUS;
static constexpr dz_float_t ER_LAYER_GIZMO_SELECTED_RADIUS = 6.f;
static constexpr dz_float_t ER_LAYER_GIZMO_RADIUS = 4.f;
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
static const char * ER_WINDOW_EFFECT_LIFE_TEXT = "Life";
static const char * ER_WINDOW_COMPOSER_TITLE = "Composer";
static const char * ER_WINDOW_COMPOSER_LAYER_MATERIAL_LABEL = "Material";
static const char * ER_WINDOW_COMPOSER_LAYER_SHAPE_LABEL = "Shape";
static const char * ER_WINDOW_COMPOSER_LAYER_EMITTER_LABEL = "Emitter";
static const char * ER_WINDOW_COMPOSER_LAYER_AFFECTOR_LABEL = "Affector";
static const char * ER_WINDOW_COMPOSER_LAYER_X_LABEL = "Layer X";
static const char * ER_WINDOW_COMPOSER_LAYER_Y_LABEL = "Layer Y";
static const char * ER_WINDOW_COMPOSER_LAYER_ANGLE_LABEL = "Layer Rotation";
static const char * ER_WINDOW_COMPOSER_LAYER_LIFE_LABEL = "Layer Life";
static const char * ER_WINDOW_COMPOSER_LAYER_SEED_LABEL = "Layer Seed";
static const char * ER_WINDOW_COMPOSER_LAYER_NAME_LABEL = "Layer Name";
static const char * ER_WINDOW_COMPOSER_LAYER_TRIGGERS_TITLE = "Layer Trigger";
static const char * ER_WINDOW_RESOURCE_NAME_LABEL = "Name";
static const char * ER_WINDOW_COMPOSER_TRIGGER_EVENT_LABEL = "Type";
static const char * ER_WINDOW_COMPOSER_TRIGGER_SOURCE_LABEL = "Source Layer";
static const char * ER_WINDOW_COMPOSER_TRIGGER_TIME_LABEL = "Trigger Time";
static const char * ER_WINDOW_COMPOSER_TRIGGER_PROBABILITY_LABEL = "Probability";
static const char * ER_WINDOW_COMPOSER_TRIGGER_COUNT_MIN_LABEL = "Spawn Count Min";
static const char * ER_WINDOW_COMPOSER_TRIGGER_COUNT_MAX_LABEL = "Spawn Count Max";
static const char * ER_WINDOW_COMPOSER_TRIGGER_DELAY_MIN_LABEL = "Delay Min";
static const char * ER_WINDOW_COMPOSER_TRIGGER_DELAY_MAX_LABEL = "Delay Max";
static const char * ER_WINDOW_COMPOSER_TRIGGER_INHERIT_POSITION_LABEL = "Inherit Position";
static const char * ER_WINDOW_COMPOSER_TRIGGER_INHERIT_ANGLE_LABEL = "Inherit Angle";
static const char * ER_WINDOW_COMPOSER_TRIGGER_INHERIT_VELOCITY_LABEL = "Inherit Velocity";
static const char * ER_WINDOW_COMPOSER_TRIGGER_OFFSET_X_LABEL = "Offset X";
static const char * ER_WINDOW_COMPOSER_TRIGGER_OFFSET_Y_LABEL = "Offset Y";
static const char * ER_WINDOW_COMPOSER_TRIGGER_ANGLE_OFFSET_LABEL = "Angle Offset";
static const char * ER_WINDOW_SHAPE_TITLE = "Shape timelines:";
static const char * ER_WINDOW_COMBO_SHAPE_TYPE_TEXT = "Shape type";
static const char * ER_WINDOW_AFFECTOR_TITLE = "Affector timelines:";
static const char * ER_WINDOW_EMITTER_TITLE = "Emitter timelines:";
static const char * ER_WINDOW_MATERIAL_TITLE = "Material data";
static const char * ER_WINDOW_MATERIAL_COMBO_BLEND_MODE_TEXT = "Blend mode";
static const char * ER_WINDOW_MATERIAL_COMBO_MODE_TEXT = "Mode";
static const char * ER_WINDOW_MATERIAL_UV_INDEX_LABEL = "UV";
static const char * ER_WINDOW_MATERIAL_UV_COUNT_LABEL = "UV Count";
static const char * ER_WINDOW_ATLAS_TITLE = "Atlas data";
static const char * ER_WINDOW_MATERIAL_TEXTURE_TITLE = "Texture";
static const char * ER_WINDOW_MATERIAL_TEXTURE_SIZE_LABEL = "Size:";
static const char * ER_WINDOW_MATERIAL_TEXTURE_BTN_APPEND = "Append Texture";
static const char * ER_WINDOW_MATERIAL_TEXTURE_BTN_ADD_REGION = "Add Region";
static const char * ER_WINDOW_MATERIAL_TEXTURE_BTN_REMOVE_REGION = "Remove Region";
static const char * ER_WINDOW_MATERIAL_TEXTURE_BTN_OPTIMIZE_ATLAS = "Optimize Atlas";
static const char * ER_WINDOW_MATERIAL_TEXTURE_BTN_CLEAR_ATLAS = "Clear Atlas";
static const char * ER_WINDOW_MATERIAL_TEXTURE_REGIONS_LABEL = "Regions:";
static const char * ER_WINDOW_MATERIAL_TEXTURE_BTN_RESET_UV = "Reset UV";
static const char * ER_WINDOW_MATERIAL_TEXTURE_REGION_X_LABEL = "Region X";
static const char * ER_WINDOW_MATERIAL_TEXTURE_REGION_Y_LABEL = "Region Y";
static const char * ER_WINDOW_MATERIAL_TEXTURE_REGION_WIDTH_LABEL = "Region Width";
static const char * ER_WINDOW_MATERIAL_TEXTURE_REGION_HEIGHT_LABEL = "Region Height";
static const char * ER_WINDOW_CONTROLS_BTN_RESET_TEXT = "Reset";
static const char * ER_WINDOW_CONTROLS_BTN_PAUSE_TEXT = "Pause";
static const char * ER_WINDOW_CONTROLS_BTN_RESUME_TEXT = "Resume";
static const char * ER_WINDOW_CONTROLS_BTN_LOOP_TEXT = "Loop";
static const char * ER_WINDOW_CONTROLS_FACTOR_PREFIX_TEXT = "Factor:";
static const char * ER_WINDOW_CONTROLS_TIME_PREFIX_TEXT = "Time:";
static const char * ER_WINDOW_CONTROLS_INPUT_LIFE_TEXT = "Life";
static const char * ER_WINDOW_CONTROLS_BTN_RESET_CAMERA_TEXT = "Reset camera";
static const char * ER_WINDOW_CONTROLS_CAMERA_MOVE_HELP_TEXT = "Camera move/scroll: <Space> + Mouse";
static const char * ER_WINDOW_CONTROLS_SHOW_LAYER_GIZMOS_TEXT = "Layer Gizmos";
static const char * ER_WINDOW_CONTROLS_SHOW_EFFECT_CENTER_TEXT = "Effect Center";
static const char * ER_WINDOW_CONTROLS_EMIT_STATES_LABEL_TEXT = "Emitter states:";
static const char * ER_WINDOW_CONTROLS_EMIT_COMPLETE_STATE_TEXT = "[Emit complete]";
static const char * ER_WINDOW_CONTROLS_PARTICLE_COMPLETE_STATE_TEXT = "[Particle complete]";
//////////////////////////////////////////////////////////////////////////
static const char * ER_WINDOW_TYPE_NAMES[] = {
    "Effect",            //ER_WINDOW_TYPE_EFFECT_DATA
    "Composer",          //ER_WINDOW_TYPE_COMPOSER_DATA
    "Shape",             //ER_WINDOW_TYPE_SHAPE_DATA
    "Affector",          //ER_WINDOW_TYPE_AFFECTOR_DATA
    "Emitter",           //ER_WINDOW_TYPE_EMITTER_DATA
    "Material",          //ER_WINDOW_TYPE_MATERIAL_DATA
    "Atlas",             //ER_WINDOW_TYPE_ATLAS_DATA
};
//////////////////////////////////////////////////////////////////////////
static const char * ER_MATERIAL_MODE_NAMES[] = {
    "Solid",             //DZ_MATERIAL_MODE_SOLID
    "Texture",           //DZ_MATERIAL_MODE_TEXTURE
    "Sequence",          //DZ_MATERIAL_MODE_SEQUENCE
};
//////////////////////////////////////////////////////////////////////////
static const char * ER_EFFECT_EVENT_NAMES[] = {
    "Effect Start",      //DZ_EFFECT_EVENT_EFFECT_START
    "Time",              //DZ_EFFECT_EVENT_TIME
    "Layer Emit Complete", //DZ_EFFECT_EVENT_LAYER_EMIT_COMPLETE
    "Layer Particle Complete", //DZ_EFFECT_EVENT_LAYER_PARTICLE_COMPLETE
    "Particle Death",    //DZ_EFFECT_EVENT_PARTICLE_DEATH
    "Custom",            //DZ_EFFECT_EVENT_CUSTOM
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
    "Line offset",       //DZ_SHAPE_LINE_OFFSET
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
    "Scale",             //DZ_AFFECTOR_TIMELINE_SCALE
    "Aspect",            //DZ_AFFECTOR_TIMELINE_ASPECT
    "Color Red",         //DZ_AFFECTOR_TIMELINE_COLOR_R
    "Color Green",       //DZ_AFFECTOR_TIMELINE_COLOR_G
    "Color Blue",        //DZ_AFFECTOR_TIMELINE_COLOR_B
    "Color Alpha",       //DZ_AFFECTOR_TIMELINE_COLOR_A
};
//////////////////////////////////////////////////////////////////////////
struct my_json_load_data_t
{
    const dz_uint8_t * buffer;
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
static dz_result_t openZipFile( unzFile _uf, const char * _file, std::vector<dz_uint8_t> * _buffer )
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

    if( file_info.uncompressed_size > (ZPOS64_T)((uInt)-1) )
    {
        return DZ_FAILURE;
    }

    const uInt content_size = (uInt)file_info.uncompressed_size;
    void * content_buffer = malloc( (size_t)content_size );

    unzReadCurrentFile( _uf, content_buffer, content_size );

    _buffer->assign( reinterpret_cast<const dz_uint8_t *>(content_buffer), reinterpret_cast<const dz_uint8_t *>(content_buffer) + content_size );

    free( content_buffer );

    unzCloseCurrentFile( _uf );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __set_shape_timeline_const( dz_service_t * _service, dz_shape_t * _shape, dz_shape_timeline_type_e _type, dz_float_t _value )
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

        dz_timeline_interpolate_set_key( interpolate, nextKey );

        if( dz_timeline_key_set_interpolate( prevKey, interpolate ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        prevKey = nextKey;
    }

    // destroy old timeline
    const dz_timeline_key_t * oldKey0 = dz_shape_get_timeline( _shape, _type );

    if( oldKey0 != DZ_NULLPTR )
    {
        dz_timeline_key_destroy( _service, oldKey0 );
    }

    // set new timeline to affector
    dz_shape_set_timeline( _shape, _type, key0 );

    return DZ_SUCCESSFUL;
}
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

        dz_timeline_interpolate_set_key( interpolate, nextKey );

        if( dz_timeline_key_set_interpolate( prevKey, interpolate ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

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
static dz_result_t __set_affector_timeline_const( dz_service_t * _service, dz_affector_t * _affector, dz_affector_timeline_type_e _type, dz_float_t _value )
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

        dz_timeline_interpolate_set_key( interpolate, nextKey );

        if( dz_timeline_key_set_interpolate( prevKey, interpolate ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

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
dz_float_t camera_scale = 1.f;
dz_float_t camera_scale_min = 0.125f;
dz_float_t camera_scale_max = 16.f;
dz_float_t camera_scale_step = 0.125f;
dz_float_t camera_offset_x = 0.f;
dz_float_t camera_offset_y = 0.f;
//////////////////////////////////////////////////////////////////////////
dz_float_t mouse_pos_x = 0.f;
dz_float_t mouse_pos_y = 0.f;
//////////////////////////////////////////////////////////////////////////
static void __glfw_framebufferSizeCallback( GLFWwindow * _window, int _width, int _height )
{
    DZ_UNUSED( _window );

    GLCALL( glViewport, (0, 0, _width, _height) );

    void * p = glfwGetWindowUserPointer( _window );

    editor * e = reinterpret_cast<editor *>(p);

    e->m_windowWidth = _width;
    e->m_windowHeight = _height;
}
//////////////////////////////////////////////////////////////////////////
static void __glfw_scrollCallback( GLFWwindow * _window, double _x, double _y )
{
    DZ_UNUSED( _x );

    editor * p_editor = reinterpret_cast<editor *>(glfwGetWindowUserPointer( _window ));

    if( glfwGetKey( _window, GLFW_KEY_SPACE ) != GLFW_PRESS )
    {
        return;
    }

    const ImVec2 & dzWindowPos = p_editor->getDzWindowPos();

    dz_float_t mouse_pos_x_norm = mouse_pos_x - dzWindowPos.x;
    dz_float_t mouse_pos_y_norm = mouse_pos_y - dzWindowPos.y;

    camera_offset_x -= mouse_pos_x_norm / camera_scale;
    camera_offset_y -= mouse_pos_y_norm / camera_scale;

    dz_float_t scroll = (dz_float_t)_y * camera_scale_step;

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
static void __glfw_cursorPosCallback( GLFWwindow * _window, double _x, double _y )
{
    if( glfwGetKey( _window, GLFW_KEY_SPACE ) == GLFW_PRESS &&
        glfwGetMouseButton( _window, GLFW_MOUSE_BUTTON_LEFT ) == GLFW_PRESS )
    {
        const dz_float_t dx = (dz_float_t)_x - mouse_pos_x;
        const dz_float_t dy = (dz_float_t)_y - mouse_pos_y;

        camera_offset_x += dx / camera_scale;
        camera_offset_y += dy / camera_scale;
    }

    mouse_pos_x = (dz_float_t)_x;
    mouse_pos_y = (dz_float_t)_y;
}
//////////////////////////////////////////////////////////////////////////
static void __glfw_keyCallback( GLFWwindow * _window, int _key, int _scancode, int _action, int _mods )
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
static void __editor_set_default_name( er_editor_instance_info_t * const _info, dz_uint32_t * const _nextId, const char * _prefix, dz_uint32_t _index );
//////////////////////////////////////////////////////////////////////////
editor::editor()
    : m_windowWidth( ER_WINDOW_WIDTH )
    , m_windowHeight( ER_WINDOW_HEIGHT )

    , m_dzWindowPos( 0.f, 0.f )
    , m_dzWindowSize( 500.f, 500.f )

    , m_backgroundColor( 0.f, 0.f, 0.f, 1.f )

    , m_showDebugInfo( false )
    , m_showCanvasLines( false )
    , m_showLayerGizmos( true )
    , m_showEffectCenter( true )
    , m_pause( false )
    , m_windowType( ER_WINDOW_TYPE_COMPOSER_DATA )

    , m_textureWidth( 0 )
    , m_textureHeight( 0 )

    , m_textureRegionSelecting( false )
    , m_textureRegionSelectStart( 0.f, 0.f )

    , m_service( nullptr )
    , m_atlas( nullptr )
    , m_texture( nullptr )
    , m_textureIndex( 0 )
    , m_material( nullptr )

    , m_shape( nullptr )
    , m_emitter( nullptr )
    , m_affector( nullptr )
    , m_materialCount( 0 )
    , m_shapeCount( 0 )
    , m_emitterCount( 0 )
    , m_affectorCount( 0 )
    , m_materialIndex( 0 )
    , m_shapeIndex( 0 )
    , m_emitterIndex( 0 )
    , m_affectorIndex( 0 )

    , m_loop( DZ_TRUE )
    , m_time_scale( 1.f )

    , m_effect( nullptr )
    , m_layerIndex( 0 )
    , m_triggerIndex( 0 )
    , m_nextEditorInstanceId( 1 )
    , m_layerGizmoDragging( false )
    , m_layerGizmoDragIndex( 0 )
    , m_layerGizmoDragOffset( 0.f, 0.f )
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
dz_result_t editor::init()
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
        glfwWindowHint( GLFW_RESIZABLE, GLFW_TRUE );

        m_fwWindow = glfwCreateWindow( m_windowWidth, m_windowHeight, ER_TITLE, 0, 0 );

        glfwSetWindowUserPointer( m_fwWindow, this );

        if( m_fwWindow == 0 )
        {
            glfwTerminate();

            return DZ_FAILURE;
        }

        glfwMakeContextCurrent( m_fwWindow );
        glfwSetFramebufferSizeCallback( m_fwWindow, &__glfw_framebufferSizeCallback );
        glfwSetScrollCallback( m_fwWindow, &__glfw_scrollCallback );
        glfwSetCursorPosCallback( m_fwWindow, &__glfw_cursorPosCallback );
        glfwSetKeyCallback( m_fwWindow, &__glfw_keyCallback );

        double cursorPosX;
        double cursorPosY;
        glfwGetCursorPos( m_fwWindow, &cursorPosX, &cursorPosY );

        mouse_pos_x = (dz_float_t)cursorPosX;
        mouse_pos_y = (dz_float_t)cursorPosY;

        if( gladLoadGL( (GLADloadfunc)&glfwGetProcAddress ) == 0 )
        {
            return DZ_FAILURE;
        }

        glfwSwapInterval( 1 );
    }

    // init opengl
    {
        dz_uint16_t max_vertex_count = 65535;
        dz_uint16_t max_index_count = 65535;

        if( dz_render_initialize( &m_openglDesc, max_vertex_count, max_index_count ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        dz_render_set_proj( &m_openglDesc, -(dz_float_t)m_dzWindowSize.x * 0.5f, (dz_float_t)m_dzWindowSize.x * 0.5f, -(dz_float_t)m_dzWindowSize.y * 0.5f, (dz_float_t)m_dzWindowSize.y * 0.5f );

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

        if( dz_material_create( m_service, &m_material, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        dz_blend_type_e blend_type = dz_material_get_default_blend();

        dz_material_set_blend( m_material, blend_type );
        dz_material_set_atlas( m_material, m_atlas );

        if( dz_material_add_texture( m_material, m_texture ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        // shape data
        if( dz_shape_create( m_service, &m_shape, m_shapeType, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        for( dz_uint32_t index = 0; index != __DZ_SHAPE_TIMELINE_MAX__; ++index )
        {
            timeline_shape_t & data = m_timelineShapeData[index];

            data.type = static_cast<dz_shape_timeline_type_e>(index);
            data.name = ER_SHAPE_DATA_NAMES[index];

            dz_timeline_limit_status_e status;
            dz_float_t min = 0.f, max = 0.f, default_value = 0.f, factor = 0.f;
            dz_shape_timeline_get_limit( data.type, &status, &min, &max, &default_value, &factor );

            data.zoom = 1;

            data.selectedPoint = ER_CURVE_POINT_NONE;

            if( __set_shape_timeline_const( m_service, m_shape, data.type, default_value ) == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }

            data.pointsData[0].x = 0.f;
            data.pointsData[0].y = default_value;

            data.pointsData[1].x = -1.f; // init data so editor knows to take it from here
        }

        // emitter data
        if( dz_emitter_create( m_service, &m_emitter, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        dz_emitter_set_life( m_emitter, 1000.f );

        for( dz_uint32_t index = 0; index != __DZ_EMITTER_TIMELINE_MAX__; ++index )
        {
            timeline_emitter_t & data = m_timelineEmitterData[index];

            data.type = static_cast<dz_emitter_timeline_type_e>(index);
            data.name = ER_EMITTER_DATA_NAMES[index];

            dz_timeline_limit_status_e status;
            dz_float_t min = 0.f, max = 0.f, default_value = 0.f, factor = 0.f;
            dz_emitter_timeline_get_limit( data.type, &status, &min, &max, &default_value, &factor );

            data.zoom = 1;

            data.selectedPoint = ER_CURVE_POINT_NONE;

            if( __set_emitter_timeline_const( m_service, m_emitter, data.type, default_value ) == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }

            data.pointsData[0].x = 0.f;
            data.pointsData[0].y = default_value;

            data.pointsData[1].x = -1.f; // init data so editor knows to take it from here
        }

        // affector data
        if( dz_affector_create( m_service, &m_affector, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        for( dz_uint32_t index = 0; index != __DZ_AFFECTOR_TIMELINE_MAX__; ++index )
        {
            timeline_affector_t & data = m_timelineAffectorData[index];

            data.type = static_cast<dz_affector_timeline_type_e>(index);
            data.name = ER_AFFECTOR_DATA_NAMES[index];

            dz_timeline_limit_status_e status;
            dz_float_t min = 0.f, max = 0.f, default_value = 0.f, factor = 0.f;
            dz_affector_timeline_get_limit( data.type, &status, &min, &max, &default_value, &factor );

            data.zoom = 1;

            data.selectedPoint = ER_CURVE_POINT_NONE;

            if( __set_affector_timeline_const( m_service, m_affector, data.type, default_value ) == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }

            data.pointsData[0].x = 0.f;
            data.pointsData[0].y = default_value;

            data.pointsData[1].x = -1.f; // init data so editor knows to take it from here
        }

        if( dz_effect_create( m_service, &m_effect, 5.f, 0, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        dz_effect_set_atlas( m_effect, m_atlas );

        dz_effect_layer_desc_t layer;
        layer.material = m_material;
        layer.shape = m_shape;
        layer.emitter = m_emitter;
        layer.affector = m_affector;
        layer.x = 0.f;
        layer.y = 0.f;
        layer.angle = 0.f;
        layer.life = 5.f;
        layer.seed = 0;

        if( dz_effect_add_layer( m_effect, &layer, &m_layerIndex ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        __editor_set_default_name( m_layerInfos + m_layerIndex, &m_nextEditorInstanceId, "Layer", m_layerIndex );

        this->rebuildResourceLists();

        dz_effect_trigger_desc_t trigger;
        trigger.event_type = DZ_EFFECT_EVENT_EFFECT_START;
        trigger.source_layer_index = DZ_EFFECT_LAYER_NONE;
        trigger.target_layer_index = m_layerIndex;
        trigger.time = 0.f;
        trigger.probability = 1.f;
        trigger.spawn_count_min = 1;
        trigger.spawn_count_max = 1;
        trigger.delay_min = 0.f;
        trigger.delay_max = 0.f;
        trigger.inherit_position = DZ_FALSE;
        trigger.inherit_angle = DZ_FALSE;
        trigger.inherit_velocity = DZ_FALSE;
        trigger.offset_x = 0.f;
        trigger.offset_y = 0.f;
        trigger.angle_offset = 0.f;

        if( dz_effect_add_trigger( m_effect, &trigger, &m_triggerIndex ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        __editor_set_default_name( m_triggerInfos + m_triggerIndex, &m_nextEditorInstanceId, "Trigger", m_triggerIndex );

        if( dz_instance_create( m_service, &m_instance, m_effect, DZ_NULLPTR ) == DZ_FAILURE )
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

        if( ImGui_ImplOpenGL3_CreateDeviceObjects() == false )
        {
            return DZ_FAILURE;
        }
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t editor::update( double _time )
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

        ImGui::SetNextWindowPos( ImVec2( 0.f, 0.f ), ImGuiCond_Always );
        ImGui::SetNextWindowSize( ImVec2( (dz_float_t)m_windowWidth, (dz_float_t)m_windowHeight ), ImGuiCond_Always );

        if( ImGui::Begin( "LAYOUT", NULL, window_flags ) )
        {
            // menu bar
            if( this->showMenuBar() == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }

            {
                ImGui::BeginChild( "CATEGORY_BAR", ImVec2( 0.f, ImGui::GetFrameHeightWithSpacing() + 8.f ), false, ImGuiWindowFlags_NoScrollbar );

                for( int32_t selectableIdx = 0; selectableIdx < IM_ARRAYSIZE( ER_WINDOW_TYPE_NAMES ); selectableIdx++ )
                {
                    const bool isSelected = m_windowType == selectableIdx;
                    if( ImGui::Selectable( ER_WINDOW_TYPE_NAMES[selectableIdx], isSelected, 0, ImVec2( 96.f, 0.f ) ) )
                    {
                        m_windowType = selectableIdx;
                    }

                    if( selectableIdx + 1 < IM_ARRAYSIZE( ER_WINDOW_TYPE_NAMES ) )
                    {
                        ImGui::SameLine();
                    }
                }

                ImGui::EndChild();
            }

            // left panel
            {
                ImGui::BeginChild( "INSTANCE_SELECT", ImVec2( 190.f, 0.f ), true );

                if( this->showResourceList( m_windowType ) == DZ_FAILURE )
                {
                    return DZ_FAILURE;
                }

                ImGui::EndChild();
            }
            ImGui::SameLine();

            // middle panel
            {
                ImGui::BeginGroup();

                // elements
                ImGui::BeginChild( "WINDOW_SELECT", ImVec2( ER_TIMELINE_PANEL_WIDTH, 0.f ), true );

                switch( m_windowType )
                {
                case ER_WINDOW_TYPE_EFFECT_DATA:
                    {
                        if( this->showEffectData() == DZ_FAILURE )
                        {
                            return DZ_FAILURE;
                        }
                    }
                    break;
                case ER_WINDOW_TYPE_COMPOSER_DATA:
                    {
                        if( this->showComposerData() == DZ_FAILURE )
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
                case ER_WINDOW_TYPE_ATLAS_DATA:
                    {
                        if( this->showAtlasData() == DZ_FAILURE )
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

                if( this->showContentPane() == DZ_FAILURE )
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
        dz_float_t time = (dz_float_t)_time * m_time_scale;

        dz_instance_update( m_service, m_instance, time );
    }
    else
    {
        dz_instance_update( m_service, m_instance, 0.f );
    }

    // update camera
    dz_render_set_camera( &m_openglDesc, camera_offset_x, camera_offset_y, camera_scale );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t editor::render()
{
    // render background
    GLCALL( glViewport, (0, 0, (GLsizei)m_windowWidth, (GLsizei)m_windowHeight) );
    GLCALL( glClearColor, (m_backgroundColor.x, m_backgroundColor.y, m_backgroundColor.z, m_backgroundColor.w) );
    GLCALL( glClear, (GL_COLOR_BUFFER_BIT) );

    // render imgui
    ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

    glfwSwapBuffers( m_fwWindow );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t editor::run( int argc, char ** argv )
{
    DZ_UNUSED( argc );
    DZ_UNUSED( argv );

    if( this->init() == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    double lastTime = glfwGetTime();

    // update loop
    while( glfwWindowShouldClose( m_fwWindow ) == 0 )
    {
        double time = glfwGetTime();
        double deltha = time - lastTime;
        lastTime = time;

        if( this->update( deltha ) == DZ_FAILURE )
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
static void __normalize_texture_region_pixels( dz_float_t * const _region, int _atlasWidth, int _atlasHeight )
{
    if( _atlasWidth <= 0 || _atlasHeight <= 0 )
    {
        _region[0] = 0.f;
        _region[1] = 0.f;
        _region[2] = 0.f;
        _region[3] = 0.f;

        return;
    }

    dz_float_t x0 = _region[0];
    dz_float_t y0 = _region[1];
    dz_float_t x1 = _region[0] + _region[2];
    dz_float_t y1 = _region[1] + _region[3];

    if( x1 < x0 )
    {
        dz_float_t temp = x0;
        x0 = x1;
        x1 = temp;
    }

    if( y1 < y0 )
    {
        dz_float_t temp = y0;
        y0 = y1;
        y1 = temp;
    }

    x0 = DZ_MAX( 0.f, DZ_MIN( x0, (dz_float_t)_atlasWidth ) );
    y0 = DZ_MAX( 0.f, DZ_MIN( y0, (dz_float_t)_atlasHeight ) );
    x1 = DZ_MAX( 0.f, DZ_MIN( x1, (dz_float_t)_atlasWidth ) );
    y1 = DZ_MAX( 0.f, DZ_MIN( y1, (dz_float_t)_atlasHeight ) );

    if( x1 - x0 < 1.f )
    {
        if( x0 + 1.f <= (dz_float_t)_atlasWidth )
        {
            x1 = x0 + 1.f;
        }
        else
        {
            x0 = (dz_float_t)_atlasWidth - 1.f;
            x1 = (dz_float_t)_atlasWidth;
        }
    }

    if( y1 - y0 < 1.f )
    {
        if( y0 + 1.f <= (dz_float_t)_atlasHeight )
        {
            y1 = y0 + 1.f;
        }
        else
        {
            y0 = (dz_float_t)_atlasHeight - 1.f;
            y1 = (dz_float_t)_atlasHeight;
        }
    }

    _region[0] = x0;
    _region[1] = y0;
    _region[2] = x1 - x0;
    _region[3] = y1 - y0;
}
//////////////////////////////////////////////////////////////////////////
static void __get_texture_region_pixels( const dz_texture_t * _texture, int _atlasWidth, int _atlasHeight, dz_float_t * const _region )
{
    dz_float_t u[4];
    dz_float_t v[4];
    dz_texture_get_uv( _texture, u, v );

    _region[0] = u[0] * (dz_float_t)_atlasWidth;
    _region[1] = v[0] * (dz_float_t)_atlasHeight;
    _region[2] = (u[2] - u[0]) * (dz_float_t)_atlasWidth;
    _region[3] = (v[2] - v[0]) * (dz_float_t)_atlasHeight;

    __normalize_texture_region_pixels( _region, _atlasWidth, _atlasHeight );
}
//////////////////////////////////////////////////////////////////////////
static void __set_texture_region_pixels( dz_texture_t * const _texture, int _atlasWidth, int _atlasHeight, dz_float_t * const _region )
{
    __normalize_texture_region_pixels( _region, _atlasWidth, _atlasHeight );

    if( _atlasWidth <= 0 || _atlasHeight <= 0 )
    {
        dz_texture_set_width( _texture, 0.f );
        dz_texture_set_height( _texture, 0.f );
        dz_texture_set_trim_offset( _texture, 0.f, 0.f );
        dz_texture_set_trim_size( _texture, 0.f, 0.f );

        return;
    }

    const dz_float_t x0 = _region[0];
    const dz_float_t y0 = _region[1];
    const dz_float_t x1 = _region[0] + _region[2];
    const dz_float_t y1 = _region[1] + _region[3];

    const dz_float_t invWidth = 1.f / (dz_float_t)_atlasWidth;
    const dz_float_t invHeight = 1.f / (dz_float_t)_atlasHeight;

    const dz_float_t u[4] = {x0 * invWidth, x1 * invWidth, x1 * invWidth, x0 * invWidth};
    const dz_float_t v[4] = {y0 * invHeight, y0 * invHeight, y1 * invHeight, y1 * invHeight};

    dz_texture_set_uv( _texture, u, v );
    dz_texture_set_width( _texture, _region[2] );
    dz_texture_set_height( _texture, _region[3] );
    dz_texture_set_trim_offset( _texture, 0.f, 0.f );
    dz_texture_set_trim_size( _texture, _region[2], _region[3] );
}
//////////////////////////////////////////////////////////////////////////
static void __copy_texture_data( dz_texture_t * const _target, const dz_texture_t * _source )
{
    dz_float_t u[4];
    dz_float_t v[4];
    dz_texture_get_uv( _source, u, v );
    dz_texture_set_uv( _target, u, v );

    dz_texture_set_width( _target, dz_texture_get_width( _source ) );
    dz_texture_set_height( _target, dz_texture_get_height( _source ) );

    dz_float_t trim_offset_x;
    dz_float_t trim_offset_y;
    dz_texture_get_trim_offset( _source, &trim_offset_x, &trim_offset_y );
    dz_texture_set_trim_offset( _target, trim_offset_x, trim_offset_y );

    dz_float_t trim_width;
    dz_float_t trim_height;
    dz_texture_get_trim_size( _source, &trim_width, &trim_height );
    dz_texture_set_trim_size( _target, trim_width, trim_height );

    dz_texture_set_sequence_delay( _target, dz_texture_get_sequence_delay( _source ) );
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __select_material_texture( dz_material_t * const _material, int * const _index, dz_texture_t ** const _texture )
{
    const dz_uint32_t textureCount = dz_material_get_texture_slot_count( _material );

    if( textureCount == 0 )
    {
        *_index = 0;
        *_texture = DZ_NULLPTR;

        return DZ_FAILURE;
    }

    if( *_index < 0 )
    {
        *_index = 0;
    }
    else if( (dz_uint32_t)*_index >= textureCount )
    {
        *_index = (int)textureCount - 1;
    }

    const dz_texture_t * texture = DZ_NULLPTR;
    if( dz_material_get_texture( _material, (dz_uint32_t)*_index, &texture ) == DZ_FAILURE )
    {
        *_texture = DZ_NULLPTR;

        return DZ_FAILURE;
    }

    *_texture = const_cast<dz_texture_t *>(texture);

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
template<class T>
static dz_uint32_t __resource_index_of( T * const * _resources, dz_uint32_t _count, const T * _resource )
{
    for( dz_uint32_t index = 0; index != _count; ++index )
    {
        if( _resources[index] == _resource )
        {
            return index;
        }
    }

    return _count;
}
//////////////////////////////////////////////////////////////////////////
template<class T>
static dz_result_t __resource_push_unique( T ** const _resources, dz_uint32_t * const _count, T * _resource )
{
    if( _resource == DZ_NULLPTR )
    {
        return DZ_SUCCESSFUL;
    }

    for( dz_uint32_t index = 0; index != *_count; ++index )
    {
        if( _resources[index] == _resource )
        {
            return DZ_SUCCESSFUL;
        }
    }

    if( *_count >= ER_EDITOR_RESOURCE_MAX )
    {
        return DZ_FAILURE;
    }

    _resources[(*_count)++] = _resource;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
template<class T>
static void __resource_erase( T ** const _resources, er_editor_instance_info_t * const _infos, dz_uint32_t * const _count, dz_uint32_t _index )
{
    for( dz_uint32_t index = _index + 1; index != *_count; ++index )
    {
        _resources[index - 1] = _resources[index];
        _infos[index - 1] = _infos[index];
    }

    --(*_count);
}
//////////////////////////////////////////////////////////////////////////
static dz_bool_t __effect_uses_material( const dz_effect_t * _effect, const dz_material_t * _material )
{
    const dz_uint32_t layerCount = dz_effect_get_layer_count( _effect );

    for( dz_uint32_t index = 0; index != layerCount; ++index )
    {
        dz_effect_layer_desc_t layer;
        if( dz_effect_get_layer( _effect, index, &layer ) == DZ_SUCCESSFUL && layer.material == _material )
        {
            return DZ_TRUE;
        }
    }

    return DZ_FALSE;
}
//////////////////////////////////////////////////////////////////////////
static dz_bool_t __effect_uses_shape( const dz_effect_t * _effect, const dz_shape_t * _shape )
{
    const dz_uint32_t layerCount = dz_effect_get_layer_count( _effect );

    for( dz_uint32_t index = 0; index != layerCount; ++index )
    {
        dz_effect_layer_desc_t layer;
        if( dz_effect_get_layer( _effect, index, &layer ) == DZ_SUCCESSFUL && layer.shape == _shape )
        {
            return DZ_TRUE;
        }
    }

    return DZ_FALSE;
}
//////////////////////////////////////////////////////////////////////////
static dz_bool_t __effect_uses_emitter( const dz_effect_t * _effect, const dz_emitter_t * _emitter )
{
    const dz_uint32_t layerCount = dz_effect_get_layer_count( _effect );

    for( dz_uint32_t index = 0; index != layerCount; ++index )
    {
        dz_effect_layer_desc_t layer;
        if( dz_effect_get_layer( _effect, index, &layer ) == DZ_SUCCESSFUL && layer.emitter == _emitter )
        {
            return DZ_TRUE;
        }
    }

    return DZ_FALSE;
}
//////////////////////////////////////////////////////////////////////////
static dz_bool_t __effect_uses_affector( const dz_effect_t * _effect, const dz_affector_t * _affector )
{
    const dz_uint32_t layerCount = dz_effect_get_layer_count( _effect );

    for( dz_uint32_t index = 0; index != layerCount; ++index )
    {
        dz_effect_layer_desc_t layer;
        if( dz_effect_get_layer( _effect, index, &layer ) == DZ_SUCCESSFUL && layer.affector == _affector )
        {
            return DZ_TRUE;
        }
    }

    return DZ_FALSE;
}
//////////////////////////////////////////////////////////////////////////
static void __editor_set_default_name( er_editor_instance_info_t * const _info, dz_uint32_t * const _nextId, const char * _prefix, dz_uint32_t _index )
{
    _info->id = (*_nextId)++;
    snprintf( _info->name, ER_EDITOR_NAME_MAX, "%s %u", _prefix, _index + 1 );
}
//////////////////////////////////////////////////////////////////////////
static dz_bool_t __editor_name_exists( const er_editor_instance_info_t * _infos, dz_uint32_t _count, dz_uint32_t _skipIndex, const char * _name )
{
    for( dz_uint32_t index = 0; index != _count; ++index )
    {
        if( index == _skipIndex )
        {
            continue;
        }

        if( strcmp( _infos[index].name, _name ) == 0 )
        {
            return DZ_TRUE;
        }
    }

    return DZ_FALSE;
}
//////////////////////////////////////////////////////////////////////////
static void __editor_set_unique_name( er_editor_instance_info_t * const _infos, dz_uint32_t _count, dz_uint32_t _index, const char * _prefix, const char * _name )
{
    char baseName[ER_EDITOR_NAME_MAX];

    if( _name == DZ_NULLPTR || _name[0] == '\0' )
    {
        snprintf( baseName, sizeof( baseName ), "%s %u", _prefix, _index + 1 );
    }
    else
    {
        snprintf( baseName, sizeof( baseName ), "%s", _name );
    }

    snprintf( _infos[_index].name, ER_EDITOR_NAME_MAX, "%s", baseName );

    if( __editor_name_exists( _infos, _count, _index, _infos[_index].name ) == DZ_FALSE )
    {
        return;
    }

    for( dz_uint32_t suffix = 2; suffix != 10000; ++suffix )
    {
        snprintf( _infos[_index].name, ER_EDITOR_NAME_MAX, "%s %u", baseName, suffix );

        if( __editor_name_exists( _infos, _count, _index, _infos[_index].name ) == DZ_FALSE )
        {
            return;
        }
    }
}
//////////////////////////////////////////////////////////////////////////
static bool __editor_name_input( const char * _label, er_editor_instance_info_t * const _infos, dz_uint32_t _count, dz_uint32_t _index, const char * _prefix )
{
    char name[ER_EDITOR_NAME_MAX];
    snprintf( name, sizeof( name ), "%s", _infos[_index].name );

    if( ImGui::InputText( _label, name, sizeof( name ) ) == false )
    {
        return false;
    }

    __editor_set_unique_name( _infos, _count, _index, _prefix, name );

    return true;
}
//////////////////////////////////////////////////////////////////////////
static jpp::array __editor_instance_infos_write( const er_editor_instance_info_t * _infos, dz_uint32_t _count )
{
    jpp::array array = jpp::make_array();

    for( dz_uint32_t index = 0; index != _count; ++index )
    {
        jpp::object info = jpp::make_object();
        info.set( "id", _infos[index].id );
        info.set( "name", _infos[index].name );

        array.push_back( info );
    }

    return array;
}
//////////////////////////////////////////////////////////////////////////
static void __editor_instance_infos_load( const jpp::object & _metadata, const char * _key, er_editor_instance_info_t * const _infos, dz_uint32_t _count, const char * _prefix, dz_uint32_t * const _maxId )
{
    jpp::object valuesObject;
    if( _metadata.exist( _key, &valuesObject ) == false )
    {
        return;
    }

    jpp::array values = valuesObject;
    const dz_uint32_t count = (dz_uint32_t)DZ_MIN( (dz_uint32_t)values.size(), _count );

    for( dz_uint32_t index = 0; index != count; ++index )
    {
        jpp::object info = values[index];

        const dz_uint32_t id = info.get( "id", _infos[index].id );
        const char * name = info.get( "name", _infos[index].name );

        _infos[index].id = id != 0 ? id : _infos[index].id;
        __editor_set_unique_name( _infos, _count, index, _prefix, name );

        if( *_maxId < _infos[index].id )
        {
            *_maxId = _infos[index].id;
        }
    }
}
//////////////////////////////////////////////////////////////////////////
static bool __resource_index_combo( const char * _label, const er_editor_instance_info_t * _infos, dz_uint32_t _count, dz_uint32_t _current, dz_uint32_t * const _selected )
{
    char preview[64];

    if( _current < _count )
    {
        snprintf( preview, sizeof( preview ), "%s", _infos[_current].name );
    }
    else
    {
        snprintf( preview, sizeof( preview ), "None" );
    }

    bool changed = false;

    if( ImGui::BeginCombo( _label, preview ) == true )
    {
        for( dz_uint32_t index = 0; index != _count; ++index )
        {
            char label[64];
            snprintf( label, sizeof( label ), "%s", _infos[index].name );

            const bool selected = index == _current;
            if( ImGui::Selectable( label, selected ) == true )
            {
                *_selected = index;
                changed = true;
            }

            if( selected == true )
            {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }

    return changed;
}
//////////////////////////////////////////////////////////////////////////
static bool __layer_index_combo( const char * _label, const er_editor_instance_info_t * _infos, dz_uint32_t _count, dz_bool_t _allowNone, dz_uint32_t _current, dz_uint32_t * const _selected )
{
    char preview[64];

    if( _current == DZ_EFFECT_LAYER_NONE )
    {
        snprintf( preview, sizeof( preview ), "None" );
    }
    else if( _current < _count )
    {
        snprintf( preview, sizeof( preview ), "%s", _infos[_current].name );
    }
    else
    {
        snprintf( preview, sizeof( preview ), "Invalid" );
    }

    bool changed = false;

    if( ImGui::BeginCombo( _label, preview ) == true )
    {
        if( _allowNone == DZ_TRUE )
        {
            const bool selected = _current == DZ_EFFECT_LAYER_NONE;
            if( ImGui::Selectable( "None", selected ) == true )
            {
                *_selected = DZ_EFFECT_LAYER_NONE;
                changed = true;
            }

            if( selected == true )
            {
                ImGui::SetItemDefaultFocus();
            }
        }

        for( dz_uint32_t index = 0; index != _count; ++index )
        {
            char label[64];
            snprintf( label, sizeof( label ), "%s", _infos[index].name );

            const bool selected = index == _current;
            if( ImGui::Selectable( label, selected ) == true )
            {
                *_selected = index;
                changed = true;
            }

            if( selected == true )
            {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }

    return changed;
}
//////////////////////////////////////////////////////////////////////////
static dz_bool_t __find_first_layer_trigger( const dz_effect_t * _effect, dz_uint32_t _layerIndex, dz_uint32_t * const _triggerIndex )
{
    const dz_uint32_t triggerCount = dz_effect_get_trigger_count( _effect );

    for( dz_uint32_t index = 0; index != triggerCount; ++index )
    {
        dz_effect_trigger_desc_t trigger;
        if( dz_effect_get_trigger( _effect, index, &trigger ) == DZ_FAILURE )
        {
            continue;
        }

        if( trigger.target_layer_index == _layerIndex )
        {
            *_triggerIndex = index;

            return DZ_TRUE;
        }
    }

    return DZ_FALSE;
}
//////////////////////////////////////////////////////////////////////////
static dz_bool_t __trigger_event_uses_source_layer( dz_effect_event_type_e _eventType )
{
    switch( _eventType )
    {
    case DZ_EFFECT_EVENT_LAYER_EMIT_COMPLETE:
    case DZ_EFFECT_EVENT_LAYER_PARTICLE_COMPLETE:
    case DZ_EFFECT_EVENT_PARTICLE_DEATH:
        return DZ_TRUE;
    case DZ_EFFECT_EVENT_EFFECT_START:
    case DZ_EFFECT_EVENT_TIME:
    case DZ_EFFECT_EVENT_CUSTOM:
    case __DZ_EFFECT_EVENT_MAX__:
    default:
        break;
    }

    return DZ_FALSE;
}
//////////////////////////////////////////////////////////////////////////
static dz_bool_t __trigger_event_uses_time( dz_effect_event_type_e _eventType )
{
    return _eventType == DZ_EFFECT_EVENT_TIME ? DZ_TRUE : DZ_FALSE;
}
//////////////////////////////////////////////////////////////////////////
static dz_bool_t __trigger_event_uses_inherit( dz_effect_event_type_e _eventType )
{
    return __trigger_event_uses_source_layer( _eventType );
}
//////////////////////////////////////////////////////////////////////////
static void __make_default_layer_trigger( dz_effect_trigger_desc_t * const _trigger, dz_uint32_t _layerIndex )
{
    _trigger->event_type = DZ_EFFECT_EVENT_EFFECT_START;
    _trigger->source_layer_index = DZ_EFFECT_LAYER_NONE;
    _trigger->target_layer_index = _layerIndex;
    _trigger->time = 0.f;
    _trigger->probability = 1.f;
    _trigger->spawn_count_min = 1;
    _trigger->spawn_count_max = 1;
    _trigger->delay_min = 0.f;
    _trigger->delay_max = 0.f;
    _trigger->inherit_position = DZ_FALSE;
    _trigger->inherit_angle = DZ_FALSE;
    _trigger->inherit_velocity = DZ_FALSE;
    _trigger->offset_x = 0.f;
    _trigger->offset_y = 0.f;
    _trigger->angle_offset = 0.f;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t editor::resetEffect()
{
    dz_shape_set_type( m_shape, m_shapeType );

    dz_instance_reset( m_instance );

    dz_instance_emit_resume( m_instance );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t editor::createDefaultMaterial( dz_material_t ** const _material )
{
    if( m_atlas == DZ_NULLPTR )
    {
        return DZ_FAILURE;
    }

    dz_material_t * material;
    if( dz_material_create( m_service, &material, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    dz_material_set_blend( material, dz_material_get_default_blend() );
    dz_material_set_mode( material, m_textureId != 0 ? DZ_MATERIAL_MODE_TEXTURE : dz_material_get_default_mode() );
    dz_material_set_atlas( material, m_atlas );
    dz_material_set_texture_index( material, 0 );
    dz_material_set_texture_count( material, 1 );

    dz_texture_t * texture;
    if( dz_texture_create( m_service, &texture, DZ_NULLPTR ) == DZ_FAILURE )
    {
        dz_material_destroy( m_service, material );

        return DZ_FAILURE;
    }

    dz_float_t region[4] = {0.f, 0.f, (dz_float_t)m_textureWidth, (dz_float_t)m_textureHeight};
    __set_texture_region_pixels( texture, m_textureWidth, m_textureHeight, region );

    if( dz_material_add_texture( material, texture ) == DZ_FAILURE )
    {
        dz_texture_destroy( m_service, texture );
        dz_material_destroy( m_service, material );

        return DZ_FAILURE;
    }

    if( m_effect != DZ_NULLPTR )
    {
        dz_effect_set_atlas( m_effect, m_atlas );
    }

    *_material = material;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t editor::createDefaultShape( dz_shape_t ** const _shape )
{
    dz_shape_t * shape;
    if( dz_shape_create( m_service, &shape, DZ_SHAPE_RECT, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    for( dz_uint32_t index = 0; index != __DZ_SHAPE_TIMELINE_MAX__; ++index )
    {
        dz_timeline_limit_status_e status;
        dz_float_t min = 0.f, max = 0.f, default_value = 0.f, factor = 0.f;
        dz_shape_timeline_get_limit( (dz_shape_timeline_type_e)index, &status, &min, &max, &default_value, &factor );

        DZ_UNUSED( status );
        DZ_UNUSED( min );
        DZ_UNUSED( max );
        DZ_UNUSED( factor );

        if( __set_shape_timeline_const( m_service, shape, (dz_shape_timeline_type_e)index, default_value ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }
    }

    *_shape = shape;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t editor::createDefaultEmitter( dz_emitter_t ** const _emitter )
{
    dz_emitter_t * emitter;
    if( dz_emitter_create( m_service, &emitter, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    dz_emitter_set_life( emitter, 1000.f );

    for( dz_uint32_t index = 0; index != __DZ_EMITTER_TIMELINE_MAX__; ++index )
    {
        dz_timeline_limit_status_e status;
        dz_float_t min = 0.f, max = 0.f, default_value = 0.f, factor = 0.f;
        dz_emitter_timeline_get_limit( (dz_emitter_timeline_type_e)index, &status, &min, &max, &default_value, &factor );

        DZ_UNUSED( status );
        DZ_UNUSED( min );
        DZ_UNUSED( max );
        DZ_UNUSED( factor );

        if( __set_emitter_timeline_const( m_service, emitter, (dz_emitter_timeline_type_e)index, default_value ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }
    }

    *_emitter = emitter;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t editor::createDefaultAffector( dz_affector_t ** const _affector )
{
    dz_affector_t * affector;
    if( dz_affector_create( m_service, &affector, DZ_NULLPTR ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    for( dz_uint32_t index = 0; index != __DZ_AFFECTOR_TIMELINE_MAX__; ++index )
    {
        dz_timeline_limit_status_e status;
        dz_float_t min = 0.f, max = 0.f, default_value = 0.f, factor = 0.f;
        dz_affector_timeline_get_limit( (dz_affector_timeline_type_e)index, &status, &min, &max, &default_value, &factor );

        DZ_UNUSED( status );
        DZ_UNUSED( min );
        DZ_UNUSED( max );
        DZ_UNUSED( factor );

        if( __set_affector_timeline_const( m_service, affector, (dz_affector_timeline_type_e)index, default_value ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }
    }

    *_affector = affector;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t editor::selectMaterialResource( dz_uint32_t _index )
{
    if( _index >= m_materialCount )
    {
        return DZ_FAILURE;
    }

    m_materialIndex = _index;
    m_material = m_materials[_index];

    const dz_atlas_t * effectAtlas = m_effect != DZ_NULLPTR ? dz_effect_get_atlas( m_effect ) : DZ_NULLPTR;
    if( effectAtlas != DZ_NULLPTR )
    {
        m_atlas = const_cast<dz_atlas_t *>(effectAtlas);
        dz_material_set_atlas( m_material, m_atlas );
    }
    else if( dz_material_get_atlas( m_material ) == DZ_NULLPTR )
    {
        dz_material_set_atlas( m_material, m_atlas );
        if( m_effect != DZ_NULLPTR )
        {
            dz_effect_set_atlas( m_effect, m_atlas );
        }
    }
    else
    {
        m_atlas = const_cast<dz_atlas_t *>(dz_material_get_atlas( m_material ));
        if( m_effect != DZ_NULLPTR )
        {
            dz_effect_set_atlas( m_effect, m_atlas );
        }
    }

    m_textureIndex = (int)dz_material_get_texture_index( m_material );

    if( dz_material_get_texture_slot_count( m_material ) != 0 )
    {
        if( __select_material_texture( m_material, &m_textureIndex, &m_texture ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }
    }
    else
    {
        m_texture = DZ_NULLPTR;
    }

    m_textureRegionSelecting = false;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t editor::selectShapeResource( dz_uint32_t _index )
{
    if( _index >= m_shapeCount )
    {
        return DZ_FAILURE;
    }

    m_shapeIndex = _index;
    m_shape = m_shapes[_index];
    m_shapeType = dz_shape_get_type( m_shape );

    return this->resetEffectData();
}
//////////////////////////////////////////////////////////////////////////
dz_result_t editor::selectEmitterResource( dz_uint32_t _index )
{
    if( _index >= m_emitterCount )
    {
        return DZ_FAILURE;
    }

    m_emitterIndex = _index;
    m_emitter = m_emitters[_index];

    return this->resetEffectData();
}
//////////////////////////////////////////////////////////////////////////
dz_result_t editor::selectAffectorResource( dz_uint32_t _index )
{
    if( _index >= m_affectorCount )
    {
        return DZ_FAILURE;
    }

    m_affectorIndex = _index;
    m_affector = m_affectors[_index];

    return this->resetEffectData();
}
//////////////////////////////////////////////////////////////////////////
void editor::rebuildResourceLists()
{
    m_materialCount = 0;
    m_shapeCount = 0;
    m_emitterCount = 0;
    m_affectorCount = 0;

    const dz_uint32_t layerCount = m_effect != DZ_NULLPTR ? dz_effect_get_layer_count( m_effect ) : 0;

    for( dz_uint32_t index = 0; index != layerCount; ++index )
    {
        dz_effect_layer_desc_t layer;
        if( dz_effect_get_layer( m_effect, index, &layer ) == DZ_FAILURE )
        {
            continue;
        }

        if( __resource_push_unique( m_materials, &m_materialCount, const_cast<dz_material_t *>(layer.material) ) == DZ_FAILURE )
        {
            continue;
        }

        if( __resource_push_unique( m_shapes, &m_shapeCount, const_cast<dz_shape_t *>(layer.shape) ) == DZ_FAILURE )
        {
            continue;
        }

        if( __resource_push_unique( m_emitters, &m_emitterCount, const_cast<dz_emitter_t *>(layer.emitter) ) == DZ_FAILURE )
        {
            continue;
        }

        if( __resource_push_unique( m_affectors, &m_affectorCount, const_cast<dz_affector_t *>(layer.affector) ) == DZ_FAILURE )
        {
            continue;
        }
    }

    for( dz_uint32_t index = 0; index != m_materialCount; ++index )
    {
        __editor_set_default_name( m_materialInfos + index, &m_nextEditorInstanceId, "Material", index );
    }

    for( dz_uint32_t index = 0; index != m_shapeCount; ++index )
    {
        __editor_set_default_name( m_shapeInfos + index, &m_nextEditorInstanceId, "Shape", index );
    }

    for( dz_uint32_t index = 0; index != m_emitterCount; ++index )
    {
        __editor_set_default_name( m_emitterInfos + index, &m_nextEditorInstanceId, "Emitter", index );
    }

    for( dz_uint32_t index = 0; index != m_affectorCount; ++index )
    {
        __editor_set_default_name( m_affectorInfos + index, &m_nextEditorInstanceId, "Affector", index );
    }

    this->syncSelectedResourceIndices();
}
//////////////////////////////////////////////////////////////////////////
void editor::syncSelectedResourceIndices()
{
    m_materialIndex = __resource_index_of( m_materials, m_materialCount, m_material );
    m_shapeIndex = __resource_index_of( m_shapes, m_shapeCount, m_shape );
    m_emitterIndex = __resource_index_of( m_emitters, m_emitterCount, m_emitter );
    m_affectorIndex = __resource_index_of( m_affectors, m_affectorCount, m_affector );

    if( m_materialIndex >= m_materialCount && m_materialCount != 0 )
    {
        m_materialIndex = 0;
    }

    if( m_shapeIndex >= m_shapeCount && m_shapeCount != 0 )
    {
        m_shapeIndex = 0;
    }

    if( m_emitterIndex >= m_emitterCount && m_emitterCount != 0 )
    {
        m_emitterIndex = 0;
    }

    if( m_affectorIndex >= m_affectorCount && m_affectorCount != 0 )
    {
        m_affectorIndex = 0;
    }
}
//////////////////////////////////////////////////////////////////////////
dz_result_t editor::ensureLayerTrigger( dz_uint32_t _layerIndex, dz_uint32_t * const _triggerIndex )
{
    const dz_uint32_t layerCount = dz_effect_get_layer_count( m_effect );
    if( _layerIndex >= layerCount )
    {
        return DZ_FAILURE;
    }

    dz_uint32_t triggerCount = dz_effect_get_trigger_count( m_effect );
    dz_uint32_t keepTriggerIndex = triggerCount;
    dz_bool_t changed = DZ_FALSE;

    if( m_triggerIndex < triggerCount )
    {
        dz_effect_trigger_desc_t trigger;
        if( dz_effect_get_trigger( m_effect, m_triggerIndex, &trigger ) == DZ_SUCCESSFUL && trigger.target_layer_index == _layerIndex )
        {
            keepTriggerIndex = m_triggerIndex;
        }
    }

    if( keepTriggerIndex == triggerCount )
    {
        for( dz_uint32_t index = 0; index != triggerCount; ++index )
        {
            dz_effect_trigger_desc_t trigger;
            if( dz_effect_get_trigger( m_effect, index, &trigger ) == DZ_FAILURE )
            {
                continue;
            }

            if( trigger.target_layer_index == _layerIndex && trigger.event_type != DZ_EFFECT_EVENT_TIME )
            {
                keepTriggerIndex = index;

                break;
            }
        }
    }

    if( keepTriggerIndex == triggerCount )
    {
        if( __find_first_layer_trigger( m_effect, _layerIndex, &keepTriggerIndex ) == DZ_FALSE )
        {
            dz_effect_trigger_desc_t trigger;
            __make_default_layer_trigger( &trigger, _layerIndex );

            if( dz_effect_add_trigger( m_effect, &trigger, &keepTriggerIndex ) == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }

            __editor_set_default_name( m_triggerInfos + keepTriggerIndex, &m_nextEditorInstanceId, "Trigger", keepTriggerIndex );
            triggerCount = dz_effect_get_trigger_count( m_effect );
            changed = DZ_TRUE;
        }
    }

    for( dz_uint32_t index = 0; index != triggerCount; )
    {
        dz_effect_trigger_desc_t trigger;
        if( dz_effect_get_trigger( m_effect, index, &trigger ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        if( trigger.target_layer_index == _layerIndex && index != keepTriggerIndex )
        {
            if( dz_effect_remove_trigger( m_effect, index, DZ_NULLPTR ) == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }

            for( dz_uint32_t shift = index + 1; shift != triggerCount; ++shift )
            {
                m_triggerInfos[shift - 1] = m_triggerInfos[shift];
            }

            --triggerCount;
            changed = DZ_TRUE;

            if( keepTriggerIndex > index )
            {
                --keepTriggerIndex;
            }

            continue;
        }

        ++index;
    }

    m_triggerIndex = keepTriggerIndex;
    *_triggerIndex = keepTriggerIndex;

    if( changed == DZ_TRUE && m_instance != DZ_NULLPTR )
    {
        if( this->resetEffect() == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t editor::selectLayer( dz_uint32_t _index )
{
    dz_effect_layer_desc_t layer;
    if( dz_effect_get_layer( m_effect, _index, &layer ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    m_layerIndex = _index;
    m_material = const_cast<dz_material_t *>(layer.material);
    m_shape = const_cast<dz_shape_t *>(layer.shape);
    m_emitter = const_cast<dz_emitter_t *>(layer.emitter);
    m_affector = const_cast<dz_affector_t *>(layer.affector);
    m_shapeType = dz_shape_get_type( m_shape );

    const dz_atlas_t * effectAtlas = dz_effect_get_atlas( m_effect );
    if( effectAtlas != DZ_NULLPTR )
    {
        m_atlas = const_cast<dz_atlas_t *>(effectAtlas);
        dz_material_set_atlas( m_material, m_atlas );
    }
    else if( dz_material_get_atlas( m_material ) == DZ_NULLPTR )
    {
        dz_material_set_atlas( m_material, m_atlas );
        dz_effect_set_atlas( m_effect, m_atlas );
    }
    else
    {
        m_atlas = const_cast<dz_atlas_t *>(dz_material_get_atlas( m_material ));
        dz_effect_set_atlas( m_effect, m_atlas );
    }

    m_textureIndex = (int)dz_material_get_texture_index( m_material );

    if( dz_material_get_texture_slot_count( m_material ) != 0 )
    {
        if( __select_material_texture( m_material, &m_textureIndex, &m_texture ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }
    }
    else
    {
        m_texture = nullptr;
    }

    m_textureRegionSelecting = false;

    this->syncSelectedResourceIndices();

    dz_uint32_t triggerIndex;
    if( this->ensureLayerTrigger( m_layerIndex, &triggerIndex ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    return this->resetEffectData();
}
//////////////////////////////////////////////////////////////////////////
void editor::setEffectAtlasesSurface()
{
    dz_atlas_t * effectAtlas = const_cast<dz_atlas_t *>(dz_effect_get_atlas( m_effect ));
    if( effectAtlas != nullptr )
    {
        dz_atlas_set_surface( effectAtlas, &m_textureId );
    }

    const dz_uint32_t layerCount = dz_effect_get_layer_count( m_effect );

    for( dz_uint32_t index = 0; index != layerCount; ++index )
    {
        dz_effect_layer_desc_t layer;
        if( dz_effect_get_layer( m_effect, index, &layer ) == DZ_FAILURE )
        {
            continue;
        }

        dz_atlas_t * atlas = const_cast<dz_atlas_t *>(dz_material_get_atlas( layer.material ));

        if( atlas != nullptr )
        {
            dz_atlas_set_surface( atlas, &m_textureId );
        }
    }
}
//////////////////////////////////////////////////////////////////////////
void editor::destroyEffectResources()
{
    if( m_instance != nullptr )
    {
        dz_instance_destroy( m_service, m_instance );
        m_instance = nullptr;
    }

    dz_material_t * materials[ER_EDITOR_RESOURCE_MAX];
    dz_shape_t * shapes[ER_EDITOR_RESOURCE_MAX];
    dz_emitter_t * emitters[ER_EDITOR_RESOURCE_MAX];
    dz_affector_t * affectors[ER_EDITOR_RESOURCE_MAX];
    dz_atlas_t * atlases[ER_EDITOR_RESOURCE_MAX];
    dz_uint32_t materialCount = 0;
    dz_uint32_t shapeCount = 0;
    dz_uint32_t emitterCount = 0;
    dz_uint32_t affectorCount = 0;
    dz_uint32_t atlasCount = 0;

    for( dz_uint32_t index = 0; index != m_materialCount; ++index )
    {
        __resource_push_unique( materials, &materialCount, m_materials[index] );
    }

    for( dz_uint32_t index = 0; index != m_shapeCount; ++index )
    {
        __resource_push_unique( shapes, &shapeCount, m_shapes[index] );
    }

    for( dz_uint32_t index = 0; index != m_emitterCount; ++index )
    {
        __resource_push_unique( emitters, &emitterCount, m_emitters[index] );
    }

    for( dz_uint32_t index = 0; index != m_affectorCount; ++index )
    {
        __resource_push_unique( affectors, &affectorCount, m_affectors[index] );
    }

    for( dz_uint32_t index = 0; index != m_materialCount; ++index )
    {
        dz_material_t * material = m_materials[index];
        const dz_atlas_t * atlas = dz_material_get_atlas( material );
        if( atlas != DZ_NULLPTR )
        {
            __resource_push_unique( atlases, &atlasCount, const_cast<dz_atlas_t *>(atlas) );
        }
    }

    if( m_effect != nullptr )
    {
        const dz_atlas_t * effectAtlas = dz_effect_get_atlas( m_effect );
        if( effectAtlas != nullptr )
        {
            __resource_push_unique( atlases, &atlasCount, const_cast<dz_atlas_t *>(effectAtlas) );
        }

        const dz_uint32_t layerCount = dz_effect_get_layer_count( m_effect );

        for( dz_uint32_t index = 0; index != layerCount; ++index )
        {
            dz_effect_layer_desc_t layer;
            if( dz_effect_get_layer( m_effect, index, &layer ) == DZ_FAILURE )
            {
                continue;
            }

            __resource_push_unique( materials, &materialCount, const_cast<dz_material_t *>(layer.material) );
            __resource_push_unique( shapes, &shapeCount, const_cast<dz_shape_t *>(layer.shape) );
            __resource_push_unique( emitters, &emitterCount, const_cast<dz_emitter_t *>(layer.emitter) );
            __resource_push_unique( affectors, &affectorCount, const_cast<dz_affector_t *>(layer.affector) );

            const dz_atlas_t * atlas = dz_material_get_atlas( layer.material );
            if( atlas != nullptr )
            {
                __resource_push_unique( atlases, &atlasCount, const_cast<dz_atlas_t *>(atlas) );
            }
        }

        dz_effect_destroy( m_service, m_effect );
        m_effect = nullptr;
    }

    for( dz_uint32_t index = 0; index != affectorCount; ++index )
    {
        dz_affector_destroy( m_service, affectors[index] );
    }

    for( dz_uint32_t index = 0; index != emitterCount; ++index )
    {
        dz_emitter_destroy( m_service, emitters[index] );
    }

    for( dz_uint32_t index = 0; index != shapeCount; ++index )
    {
        dz_shape_destroy( m_service, shapes[index] );
    }

    std::vector<const dz_texture_t *> materialTextures;
    for( dz_uint32_t index = 0; index != materialCount; ++index )
    {
        const dz_texture_t * texture;
        while( dz_material_pop_texture( materials[index], &texture ) == DZ_SUCCESSFUL )
        {
            if( std::find( materialTextures.begin(), materialTextures.end(), texture ) == materialTextures.end() )
            {
                materialTextures.push_back( texture );
            }
        }
    }

    for( const dz_texture_t * texture : materialTextures )
    {
        dz_texture_destroy( m_service, texture );
    }

    for( dz_uint32_t index = 0; index != materialCount; ++index )
    {
        dz_material_destroy( m_service, materials[index] );
    }

    for( dz_uint32_t index = 0; index != atlasCount; ++index )
    {
        dz_atlas_t * mutableAtlas = atlases[index];
        dz_atlas_destroy( m_service, mutableAtlas );
    }

    m_atlas = nullptr;
    m_texture = nullptr;
    m_material = nullptr;
    m_shape = nullptr;
    m_emitter = nullptr;
    m_affector = nullptr;
    m_materialCount = 0;
    m_shapeCount = 0;
    m_emitterCount = 0;
    m_affectorCount = 0;
    m_materialIndex = 0;
    m_shapeIndex = 0;
    m_emitterIndex = 0;
    m_affectorIndex = 0;
    m_layerIndex = 0;
    m_triggerIndex = 0;
    m_nextEditorInstanceId = 1;
    m_layerGizmoDragging = false;
    m_layerGizmoDragIndex = 0;
    m_layerGizmoDragOffset = ImVec2( 0.f, 0.f );
}
//////////////////////////////////////////////////////////////////////////
dz_result_t editor::saveEffect()
{
    nfdchar_t * outPath = NULL;
    nfdresult_t result = NFD_SaveDialog( "dz", nullptr, &outPath );

    if( result == NFD_OKAY )
    {
        puts( "Success!" );
        puts( outPath );

        dz_effect_set_atlas( m_effect, m_atlas );

        jpp::object json = dz_evict_write( m_effect );

        jpp::object editorMetadata = jpp::make_object();
        editorMetadata.set( "version", 1 );
        editorMetadata.set( "next_id", m_nextEditorInstanceId );
        editorMetadata.set( "layers", __editor_instance_infos_write( m_layerInfos, dz_effect_get_layer_count( m_effect ) ) );
        editorMetadata.set( "materials", __editor_instance_infos_write( m_materialInfos, m_materialCount ) );
        editorMetadata.set( "shapes", __editor_instance_infos_write( m_shapeInfos, m_shapeCount ) );
        editorMetadata.set( "emitters", __editor_instance_infos_write( m_emitterInfos, m_emitterCount ) );
        editorMetadata.set( "affectors", __editor_instance_infos_write( m_affectorInfos, m_affectorCount ) );

        json.set( "editor", editorMetadata );

        er_memory_buffer_t dumpJson;
        dumpJson.data = DZ_NULLPTR;
        dumpJson.size = 0;
        dumpJson.capacity = 0;

        if( this->dumpJSON_( json, &dumpJson, /*_needCompactDump*/ false ) == false )
        {
            free( dumpJson.data );

            return DZ_FAILURE;
        }

        zipFile zf = zipOpen64( outPath, 0 );

        if( addZipFile( zf, "data.json", dumpJson.data, dumpJson.size ) == DZ_FAILURE )
        {
            free( dumpJson.data );

            return DZ_FAILURE;
        }

        free( dumpJson.data );

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
dz_result_t editor::loadEffect()
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

        std::vector<dz_uint8_t> data_buffer;
        openZipFile( uf, "data.json", &data_buffer );

        jpp::object data;
        this->loadJSON_( data_buffer.data(), data_buffer.size(), &data );

        this->destroyEffectResources();

        if( dz_evict_load( m_service, &m_effect, data ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        const dz_uint32_t layerCount = dz_effect_get_layer_count( m_effect );

        const dz_atlas_t * sharedAtlas = dz_effect_get_atlas( m_effect );
        if( sharedAtlas == DZ_NULLPTR && layerCount != 0 )
        {
            dz_effect_layer_desc_t baseLayer;
            if( dz_effect_get_layer( m_effect, 0, &baseLayer ) == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }

            sharedAtlas = dz_material_get_atlas( baseLayer.material );
            dz_effect_set_atlas( m_effect, sharedAtlas );
        }

        if( sharedAtlas != DZ_NULLPTR )
        {
            m_atlas = const_cast<dz_atlas_t *>(sharedAtlas);

            for( dz_uint32_t index = 0; index != layerCount; ++index )
            {
                dz_effect_layer_desc_t layer;
                if( dz_effect_get_layer( m_effect, index, &layer ) == DZ_FAILURE )
                {
                    return DZ_FAILURE;
                }

                dz_material_set_atlas( const_cast<dz_material_t *>(layer.material), sharedAtlas );
            }
        }

        if( dz_instance_create( m_service, &m_instance, m_effect, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        dz_uint32_t seed = dz_effect_get_seed( m_effect );

        dz_instance_set_seed( m_instance, seed );
        dz_instance_set_loop( m_instance, m_loop );

        m_atlasBuffer.clear();

        openZipFile( uf, "atlas.png", &m_atlasBuffer );

        dz_render_delete_texture( m_textureId );

        m_textureId = dz_render_make_texture_from_memory( m_atlasBuffer.data(), m_atlasBuffer.size(), &m_textureWidth, &m_textureHeight );

        this->setEffectAtlasesSurface();

        m_nextEditorInstanceId = 1;

        for( dz_uint32_t index = 0; index != layerCount; ++index )
        {
            __editor_set_default_name( m_layerInfos + index, &m_nextEditorInstanceId, "Layer", index );
        }

        const dz_uint32_t triggerCount = dz_effect_get_trigger_count( m_effect );
        for( dz_uint32_t index = 0; index != triggerCount; ++index )
        {
            __editor_set_default_name( m_triggerInfos + index, &m_nextEditorInstanceId, "Trigger", index );
        }

        this->rebuildResourceLists();

        jpp::object editorMetadata;
        if( data.exist( "editor", &editorMetadata ) == true )
        {
            m_nextEditorInstanceId = editorMetadata.get( "next_id", m_nextEditorInstanceId );

            dz_uint32_t maxEditorInstanceId = 0;
            __editor_instance_infos_load( editorMetadata, "layers", m_layerInfos, layerCount, "Layer", &maxEditorInstanceId );
            __editor_instance_infos_load( editorMetadata, "materials", m_materialInfos, m_materialCount, "Material", &maxEditorInstanceId );
            __editor_instance_infos_load( editorMetadata, "shapes", m_shapeInfos, m_shapeCount, "Shape", &maxEditorInstanceId );
            __editor_instance_infos_load( editorMetadata, "emitters", m_emitterInfos, m_emitterCount, "Emitter", &maxEditorInstanceId );
            __editor_instance_infos_load( editorMetadata, "affectors", m_affectorInfos, m_affectorCount, "Affector", &maxEditorInstanceId );

            if( m_nextEditorInstanceId <= maxEditorInstanceId )
            {
                m_nextEditorInstanceId = maxEditorInstanceId + 1;
            }
        }

        if( this->selectLayer( 0 ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        m_textureRegionSelecting = false;

        unzCloseCurrentFile( uf );

        free( outPath );

        this->resetEffect();
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
dz_result_t editor::exportEffect()
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

        dz_effect_set_atlas( m_effect, m_atlas );

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
dz_result_t editor::showMenuBar()
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
static void __pointsDataToCurve( er_curve_point_t * _pointsData, er_curve_point_t * _pointsCurve, dz_float_t _min, dz_float_t _range )
{
    int32_t end = 0;
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
static void __pointsDataToCurveInv( er_curve_point_t * _pointsData, er_curve_point_t * _pointsCurve, dz_float_t _min, dz_float_t _range )
{
    int32_t end = 0;
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
static void __pointsCurveToData( er_curve_point_t * _pointsCurve, er_curve_point_t * _pointsData, dz_float_t _min, dz_float_t _range )
{
    int32_t end = 0;
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
static void __pointsCurveToDataInv( er_curve_point_t * _pointsCurve, er_curve_point_t * _pointsData, dz_float_t _min, dz_float_t _range )
{
    int32_t end = 0;
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
static void __setupLimits( er_curve_point_t * _pointsData, dz_timeline_limit_status_e _status, dz_float_t _min, dz_float_t _max, dz_float_t * _factor, int32_t * _zoom, dz_float_t * _y_min, dz_float_t * _y_max, bool _inv )
{
    if( _status != DZ_TIMELINE_LIMIT_NORMAL )
    {
        dz_float_t max_value = 0.f;
        {
            for( int index = 0; index < ER_CURVE_MAX_POINTS && _pointsData[index].x >= 0; index++ )
            {
                dz_float_t value = _inv == false ? _pointsData[index].y : 1.f / _pointsData[index].y;

                if( value > max_value )
                {
                    max_value = value;
                }
            }
        }

        dz_float_t up_limit = *_zoom * (*_factor);
        while( max_value > up_limit )
        {
            (*_zoom)++;
            up_limit = *_zoom * (*_factor);
        }

        // zoom up
        int32_t nextZoomUp = *_zoom * 2;

        dz_float_t nextFactorUp = nextZoomUp * (*_factor);

        dz_float_t y_min_up = _min;
        dz_float_t y_max_up = _max;

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

        dz_float_t nextFactorDown = nextZoomDown * (*_factor);

        dz_float_t y_min_down = _min;
        dz_float_t y_max_down = _max;

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

        for( int index = 0; index < ER_CURVE_MAX_POINTS && _pointsData[index].x >= 0.f; index++ )
        {
            dz_float_t value = _inv == false ? _pointsData[index].y : 1.f / _pointsData[index].y;

            er_curve_point_mode_e mode = _pointsData[index].mode;

            if( mode == ER_CURVE_POINT_MODE_NORMAL )
            {
                if( value < y_min_down || value > y_max_down )
                {
                    availableZoomDown = false;
                    break;
                }
            }
            else if( mode == ER_CURVE_POINT_MODE_RANDOM )
            {
                dz_float_t value2 = _inv == false ? _pointsData[index].y2 : 1.f / _pointsData[index].y2;

                if( value < y_min_down || value2 < y_min_down || value > y_max_down || value2 > y_max_down )
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
static int __setupCurve( const char * _label, const ImVec2 & _size, const int _maxpoints, er_curve_point_t * _points, int * _selected, dz_float_t _x_min = 0.f, dz_float_t _x_max = 1.f, dz_float_t _y_min = 0.f, dz_float_t _y_max = 1.f )
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

    ImGui::RenderFrame( bb.Min, bb.Max, ImGui::GetColorU32( ImGuiCol_Border, 1 ), true, style.FrameRounding );
    bb.Min.x += ER_CURVE_PLOT_BORDER_SIZE;
    bb.Min.y += ER_CURVE_PLOT_BORDER_SIZE;
    bb.Max.x -= ER_CURVE_PLOT_BORDER_SIZE;
    bb.Max.y -= ER_CURVE_PLOT_BORDER_SIZE;
    ImGui::RenderFrame( bb.Min, bb.Max, ImGui::GetColorU32( ImGuiCol_FrameBg, 1 ), true, style.FrameRounding );

    dz_float_t ht = bb.Max.y - bb.Min.y;
    dz_float_t wd = bb.Max.x - bb.Min.x;

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
                            dz_float_t p1d = abs( p.y );
                            if( p1d < (1.f / ER_CURVE_PLOT_HOVER_RADIUS_POW_2) ) sel = left;
                        }
                        else if( _points[left].mode == ER_CURVE_POINT_MODE_RANDOM )
                        {
                            dz_float_t p1d = abs( p.y );
                            if( p1d < (1.f / ER_CURVE_PLOT_HOVER_RADIUS_POW_2) )
                            {
                                sel = left;
                                is_active_y2 = false;
                            }

                            dz_float_t p1d_y2 = abs( p_y2.y );
                            if( p1d_y2 < (1.f / ER_CURVE_PLOT_HOVER_RADIUS_POW_2) )
                            {
                                sel = left;
                                is_active_y2 = true;
                            }
                        }
                    }
                    else
                    {
                        dz_float_t p1d = sqrt( p.x * p.x + p.y * p.y );
                        dz_float_t p1d_y2 = sqrt( p_y2.x * p_y2.x + p_y2.y * p_y2.y );
                        p.x = _points[left + 1].x;
                        p.y = _points[left + 1].y;
                        p = p - pos;
                        p_y2.x = _points[left + 1].x;
                        p_y2.y = _points[left + 1].y2;
                        p_y2 = p_y2 - pos;
                        dz_float_t p2d = sqrt( p.x * p.x + p.y * p.y );
                        dz_float_t p2d_y2 = sqrt( p_y2.x * p_y2.x + p_y2.y * p_y2.y );

                        if( _points[left].mode == ER_CURVE_POINT_MODE_NORMAL )
                        {
                            if( p1d < (1.f / ER_CURVE_PLOT_HOVER_RADIUS_POW_2) ) sel = left;
                        }
                        else if( _points[left].mode == ER_CURVE_POINT_MODE_RANDOM )
                        {
                            if( p1d < (1.f / ER_CURVE_PLOT_HOVER_RADIUS_POW_2) )
                            {
                                sel = left;
                                is_active_y2 = false;
                            }

                            if( p1d_y2 < (1.f / ER_CURVE_PLOT_HOVER_RADIUS_POW_2) )
                            {
                                sel = left;
                                is_active_y2 = true;
                            }
                        }

                        if( _points[left + 1].mode == ER_CURVE_POINT_MODE_NORMAL )
                        {
                            if( p2d < (1.f / ER_CURVE_PLOT_HOVER_RADIUS_POW_2) ) sel = left + 1;
                        }
                        else if( _points[left + 1].mode == ER_CURVE_POINT_MODE_RANDOM )
                        {
                            if( p2d < (1.f / ER_CURVE_PLOT_HOVER_RADIUS_POW_2) )
                            {
                                sel = left + 1;
                                is_active_y2 = false;
                            }

                            if( p2d_y2 < (1.f / ER_CURVE_PLOT_HOVER_RADIUS_POW_2) )
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
            else if( _points[i - 1].mode == ER_CURVE_POINT_MODE_RANDOM )
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

        dz_float_t x = 0.f;

        if( max > 1 )
        {
            x = _x_min + pos.x * (_x_max - _x_min);
        }

        dz_float_t y = _y_min + pos.y * (_y_max - _y_min);

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
static int __setupSelectCurvePointMode( int _selectedPoint, dz_float_t _factor, dz_float_t _min, dz_float_t _max, er_curve_point_t * _pointsData, er_curve_point_t * _pointsCurve )
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
                    dz_float_t distance = _pointsData[_selectedPoint].y2 - _pointsData[_selectedPoint].y;
                    _pointsData[_selectedPoint].y = _pointsData[_selectedPoint].y + distance / 2.f;
                }
                else if( selected_mode == ER_CURVE_POINT_MODE_RANDOM )
                {
                    dz_float_t normalValue = _pointsData[_selectedPoint].y;
                    dz_float_t randMinValue = normalValue - 0.25f * _factor;

                    if( randMinValue < _min )
                    {
                        randMinValue = _min;
                    }

                    dz_float_t randMaxValue = normalValue + 0.25f * _factor;

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
dz_result_t editor::readTimelineKey( const dz_timeline_key_t * _key, er_curve_point_t * _pointsData, size_t _index )
{
    if( _index + 1 >= ER_CURVE_MAX_POINTS )
    {
        return DZ_FAILURE;
    }

    dz_timeline_key_type_e key_type = dz_timeline_key_get_type( _key );

    if( key_type == DZ_TIMELINE_KEY_CONST )
    {
        dz_float_t p = dz_timeline_key_get_p( _key );

        dz_float_t const_value;
        dz_timeline_key_get_const_value( _key, &const_value );

        _pointsData[_index].x = p;
        _pointsData[_index].y = const_value;
        _pointsData[_index].y2 = 0.f;
        _pointsData[_index].mode = ER_CURVE_POINT_MODE_NORMAL;


    }
    else if( key_type == DZ_TIMELINE_KEY_RANDOMIZE )
    {
        dz_float_t p = dz_timeline_key_get_p( _key );

        dz_float_t randomize_min;
        dz_float_t randomize_max;
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
dz_result_t editor::resetEffectData()
{
    //dz_float_t life = dz_effect_get_life( m_effect );

    // shape data
    for( dz_uint32_t index = 0; index != __DZ_SHAPE_TIMELINE_MAX__; ++index )
    {
        timeline_shape_t & data = m_timelineShapeData[index];

        data.type = static_cast<dz_shape_timeline_type_e>(index);
        data.selectedPoint = ER_CURVE_POINT_NONE;

        const dz_timeline_key_t * key = dz_shape_get_timeline( m_shape, data.type );

        data.pointsData[0].x = -1.f; // init data so editor knows to take it from here

        dz_timeline_limit_status_e status;
        dz_float_t min = 0.f, max = 0.f, default_value = 0.f, factor = 0.f;
        dz_shape_timeline_get_limit( data.type, &status, &min, &max, &default_value, &factor );

        if( key != DZ_NULLPTR )
        {
            if( this->readTimelineKey( key, data.pointsData, 0 ) == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }
        }
        else
        {
            data.pointsData[0].x = 0.f;
            data.pointsData[0].y = default_value;

            data.pointsData[1].x = -1.f; // init data so editor knows to take it from here
        }

        // curve
        dz_float_t y_min = min;
        dz_float_t y_max = max;

        __setupLimits( data.pointsData, status, min, max, &factor, &(data.zoom), &y_min, &y_max, false );

        dz_float_t y_range = y_max - y_min;

        __pointsDataToCurve( data.pointsData, data.pointsCurve, y_min, y_range );
    }

    for( dz_uint32_t index = 0; index != __DZ_EMITTER_TIMELINE_MAX__; ++index )
    {
        timeline_emitter_t & data = m_timelineEmitterData[index];

        data.type = static_cast<dz_emitter_timeline_type_e>(index);
        data.selectedPoint = ER_CURVE_POINT_NONE;

        const dz_timeline_key_t * key = dz_emitter_get_timeline( m_emitter, data.type );

        data.pointsData[0].x = -1.f; // init data so editor knows to take it from here

        dz_timeline_limit_status_e status;
        dz_float_t min = 0.f, max = 0.f, default_value = 0.f, factor = 0.f;
        dz_emitter_timeline_get_limit( data.type, &status, &min, &max, &default_value, &factor );

        if( key != DZ_NULLPTR )
        {
            if( this->readTimelineKey( key, data.pointsData, 0 ) == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }
        }
        else
        {
            data.pointsData[0].x = 0.f;
            data.pointsData[0].y = default_value;

            data.pointsData[1].x = -1.f; // init data so editor knows to take it from here
        }

        // curve
        dz_float_t y_min = min;
        dz_float_t y_max = max;

        __setupLimits( data.pointsData, status, min, max, &factor, &(data.zoom), &y_min, &y_max, false );

        dz_float_t y_range = y_max - y_min;

        __pointsDataToCurve( data.pointsData, data.pointsCurve, y_min, y_range );
    }

    // affector
    for( dz_uint32_t index = 0; index != __DZ_AFFECTOR_TIMELINE_MAX__; ++index )
    {
        timeline_affector_t & data = m_timelineAffectorData[index];

        data.type = static_cast<dz_affector_timeline_type_e>(index);
        data.selectedPoint = ER_CURVE_POINT_NONE;

        const dz_timeline_key_t * key = dz_affector_get_timeline( m_affector, data.type );

        data.pointsData[0].x = -1.f; // init data so editor knows to take it from here

        dz_timeline_limit_status_e status;
        dz_float_t min = 0.f, max = 0.f, default_value = 0.f, factor = 0.f;
        dz_affector_timeline_get_limit( data.type, &status, &min, &max, &default_value, &factor );

        if( key != DZ_NULLPTR )
        {
            if( this->readTimelineKey( key, data.pointsData, 0 ) == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }
        }
        else
        {
            data.pointsData[0].x = 0.f;
            data.pointsData[0].y = default_value;

            data.pointsData[1].x = -1.f; // init data so editor knows to take it from here
        }

        if( index == DZ_AFFECTOR_TIMELINE_STRAFE_SHIFT )
        {
            continue;
        }

        // curve
        dz_float_t y_min = min;
        dz_float_t y_max = max;

        __setupLimits( data.pointsData, status, min, max, &factor, &(data.zoom), &y_min, &y_max, false );

        dz_float_t y_range = y_max - y_min;

        __pointsDataToCurve( data.pointsData, data.pointsCurve, y_min, y_range );
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t editor::showEffectData()
{
    ImGui::Spacing();

    ImGui::Text( ER_WINDOW_EFFECT_TITLE );
    ImGui::Separator();

    ImGui::Spacing();

    int seed = dz_instance_get_seed( m_instance );

    if( ImGui::InputInt( ER_WINDOW_EFFECT_SEED_TEXT, &seed, 0, 0, ImGuiInputTextFlags_None ) == true )
    {
        dz_effect_set_seed( m_effect, seed );

        dz_instance_set_seed( m_instance, seed );

        this->resetEffect();
    }

    dz_float_t life = dz_effect_get_life( m_effect );

    if( ImGui::InputFloat( ER_WINDOW_EFFECT_LIFE_TEXT, &life, 0.1f, 1.f, "%.3f", ImGuiInputTextFlags_None ) == true )
    {
        if( life < 0.f )
        {
            life = 0.f;
        }

        dz_effect_set_life( m_effect, life );

        this->resetEffect();
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t editor::showResourceList( int _selected )
{
    switch( _selected )
    {
    case ER_WINDOW_TYPE_EFFECT_DATA:
        {
            ImGui::Selectable( "Effect", true );
        }
        break;
    case ER_WINDOW_TYPE_COMPOSER_DATA:
        {
            dz_uint32_t layerCount = dz_effect_get_layer_count( m_effect );

            if( ImGui::Button( "+##Layer" ) == true )
            {
                dz_effect_layer_desc_t layer;
                if( dz_effect_get_layer( m_effect, m_layerIndex, &layer ) == DZ_FAILURE )
                {
                    return DZ_FAILURE;
                }

                dz_uint32_t layerIndex;
                if( dz_effect_add_layer( m_effect, &layer, &layerIndex ) == DZ_FAILURE )
                {
                    return DZ_FAILURE;
                }

                __editor_set_default_name( m_layerInfos + layerIndex, &m_nextEditorInstanceId, "Layer", layerIndex );

                const dz_uint32_t materialCount = m_materialCount;
                const dz_uint32_t shapeCount = m_shapeCount;
                const dz_uint32_t emitterCount = m_emitterCount;
                const dz_uint32_t affectorCount = m_affectorCount;

                if( __resource_push_unique( m_materials, &m_materialCount, const_cast<dz_material_t *>(layer.material) ) == DZ_FAILURE
                    || __resource_push_unique( m_shapes, &m_shapeCount, const_cast<dz_shape_t *>(layer.shape) ) == DZ_FAILURE
                    || __resource_push_unique( m_emitters, &m_emitterCount, const_cast<dz_emitter_t *>(layer.emitter) ) == DZ_FAILURE
                    || __resource_push_unique( m_affectors, &m_affectorCount, const_cast<dz_affector_t *>(layer.affector) ) == DZ_FAILURE )
                {
                    return DZ_FAILURE;
                }

                if( materialCount != m_materialCount )
                {
                    __editor_set_default_name( m_materialInfos + materialCount, &m_nextEditorInstanceId, "Material", materialCount );
                }

                if( shapeCount != m_shapeCount )
                {
                    __editor_set_default_name( m_shapeInfos + shapeCount, &m_nextEditorInstanceId, "Shape", shapeCount );
                }

                if( emitterCount != m_emitterCount )
                {
                    __editor_set_default_name( m_emitterInfos + emitterCount, &m_nextEditorInstanceId, "Emitter", emitterCount );
                }

                if( affectorCount != m_affectorCount )
                {
                    __editor_set_default_name( m_affectorInfos + affectorCount, &m_nextEditorInstanceId, "Affector", affectorCount );
                }

                dz_uint32_t sourceTriggerIndex;
                if( this->ensureLayerTrigger( m_layerIndex, &sourceTriggerIndex ) == DZ_FAILURE )
                {
                    return DZ_FAILURE;
                }

                dz_effect_trigger_desc_t trigger;
                if( dz_effect_get_trigger( m_effect, sourceTriggerIndex, &trigger ) == DZ_FAILURE )
                {
                    return DZ_FAILURE;
                }

                trigger.target_layer_index = layerIndex;

                if( trigger.source_layer_index == m_layerIndex )
                {
                    trigger.source_layer_index = layerIndex;
                }

                if( __trigger_event_uses_source_layer( trigger.event_type ) == DZ_FALSE )
                {
                    trigger.source_layer_index = DZ_EFFECT_LAYER_NONE;
                }

                if( dz_effect_add_trigger( m_effect, &trigger, &m_triggerIndex ) == DZ_FAILURE )
                {
                    return DZ_FAILURE;
                }

                __editor_set_default_name( m_triggerInfos + m_triggerIndex, &m_nextEditorInstanceId, "Trigger", m_triggerIndex );

                if( this->selectLayer( layerIndex ) == DZ_FAILURE )
                {
                    return DZ_FAILURE;
                }

                this->resetEffect();

                layerCount = dz_effect_get_layer_count( m_effect );
            }

            ImGui::SameLine();

            if( ImGui::Button( "-##Layer" ) == true && layerCount > 1 )
            {
                const dz_uint32_t removedLayerIndex = m_layerIndex;

                if( dz_effect_remove_layer( m_effect, removedLayerIndex, DZ_NULLPTR ) == DZ_FAILURE )
                {
                    return DZ_FAILURE;
                }

                for( dz_uint32_t index = removedLayerIndex + 1; index != layerCount; ++index )
                {
                    m_layerInfos[index - 1] = m_layerInfos[index];
                }

                layerCount = dz_effect_get_layer_count( m_effect );

                const dz_uint32_t triggerCountAfterRemove = dz_effect_get_trigger_count( m_effect );
                for( dz_uint32_t index = 0; index != triggerCountAfterRemove; ++index )
                {
                    __editor_set_default_name( m_triggerInfos + index, &m_nextEditorInstanceId, "Trigger", index );
                }

                if( m_layerIndex >= layerCount )
                {
                    m_layerIndex = layerCount - 1;
                }

                if( this->selectLayer( m_layerIndex ) == DZ_FAILURE )
                {
                    return DZ_FAILURE;
                }

                this->resetEffect();
            }

            ImGui::Separator();

            layerCount = dz_effect_get_layer_count( m_effect );
            for( dz_uint32_t index = 0; index != layerCount; ++index )
            {
                if( ImGui::Selectable( m_layerInfos[index].name, index == m_layerIndex ) == true )
                {
                    if( this->selectLayer( index ) == DZ_FAILURE )
                    {
                        return DZ_FAILURE;
                    }
                }
            }
        }
        break;
    case ER_WINDOW_TYPE_ATLAS_DATA:
        {
            ImGui::Selectable( "Atlas", true );
        }
        break;
    case ER_WINDOW_TYPE_MATERIAL_DATA:
        {
            if( ImGui::Button( "+##Material" ) == true )
            {
                if( m_materialCount >= ER_EDITOR_RESOURCE_MAX )
                {
                    return DZ_FAILURE;
                }

                dz_material_t * material;
                if( this->createDefaultMaterial( &material ) == DZ_FAILURE )
                {
                    return DZ_FAILURE;
                }

                const dz_uint32_t index = m_materialCount++;
                m_materials[index] = material;
                __editor_set_default_name( m_materialInfos + index, &m_nextEditorInstanceId, "Material", index );

                if( this->selectMaterialResource( index ) == DZ_FAILURE )
                {
                    return DZ_FAILURE;
                }
            }

            ImGui::SameLine();

            if( ImGui::Button( "-##Material" ) == true && m_materialCount > 1 && m_materialIndex < m_materialCount && __effect_uses_material( m_effect, m_materials[m_materialIndex] ) == DZ_FALSE )
            {
                dz_material_t * material = m_materials[m_materialIndex];

                dz_material_destroy( m_service, material );
                __resource_erase( m_materials, m_materialInfos, &m_materialCount, m_materialIndex );

                if( m_materialIndex >= m_materialCount )
                {
                    m_materialIndex = m_materialCount - 1;
                }

                if( this->selectMaterialResource( m_materialIndex ) == DZ_FAILURE )
                {
                    return DZ_FAILURE;
                }
            }

            ImGui::Separator();

            for( dz_uint32_t index = 0; index != m_materialCount; ++index )
            {
                if( ImGui::Selectable( m_materialInfos[index].name, index == m_materialIndex ) == true )
                {
                    if( this->selectMaterialResource( index ) == DZ_FAILURE )
                    {
                        return DZ_FAILURE;
                    }
                }
            }
        }
        break;
    case ER_WINDOW_TYPE_SHAPE_DATA:
        {
            if( ImGui::Button( "+##Shape" ) == true )
            {
                if( m_shapeCount >= ER_EDITOR_RESOURCE_MAX )
                {
                    return DZ_FAILURE;
                }

                dz_shape_t * shape;
                if( this->createDefaultShape( &shape ) == DZ_FAILURE )
                {
                    return DZ_FAILURE;
                }

                const dz_uint32_t index = m_shapeCount++;
                m_shapes[index] = shape;
                __editor_set_default_name( m_shapeInfos + index, &m_nextEditorInstanceId, "Shape", index );

                if( this->selectShapeResource( index ) == DZ_FAILURE )
                {
                    return DZ_FAILURE;
                }
            }

            ImGui::SameLine();

            if( ImGui::Button( "-##Shape" ) == true && m_shapeCount > 1 && m_shapeIndex < m_shapeCount && __effect_uses_shape( m_effect, m_shapes[m_shapeIndex] ) == DZ_FALSE )
            {
                dz_shape_t * shape = m_shapes[m_shapeIndex];

                dz_shape_destroy( m_service, shape );
                __resource_erase( m_shapes, m_shapeInfos, &m_shapeCount, m_shapeIndex );

                if( m_shapeIndex >= m_shapeCount )
                {
                    m_shapeIndex = m_shapeCount - 1;
                }

                if( this->selectShapeResource( m_shapeIndex ) == DZ_FAILURE )
                {
                    return DZ_FAILURE;
                }
            }

            ImGui::Separator();

            for( dz_uint32_t index = 0; index != m_shapeCount; ++index )
            {
                if( ImGui::Selectable( m_shapeInfos[index].name, index == m_shapeIndex ) == true )
                {
                    if( this->selectShapeResource( index ) == DZ_FAILURE )
                    {
                        return DZ_FAILURE;
                    }
                }
            }
        }
        break;
    case ER_WINDOW_TYPE_EMITTER_DATA:
        {
            if( ImGui::Button( "+##Emitter" ) == true )
            {
                if( m_emitterCount >= ER_EDITOR_RESOURCE_MAX )
                {
                    return DZ_FAILURE;
                }

                dz_emitter_t * emitter;
                if( this->createDefaultEmitter( &emitter ) == DZ_FAILURE )
                {
                    return DZ_FAILURE;
                }

                const dz_uint32_t index = m_emitterCount++;
                m_emitters[index] = emitter;
                __editor_set_default_name( m_emitterInfos + index, &m_nextEditorInstanceId, "Emitter", index );

                if( this->selectEmitterResource( index ) == DZ_FAILURE )
                {
                    return DZ_FAILURE;
                }
            }

            ImGui::SameLine();

            if( ImGui::Button( "-##Emitter" ) == true && m_emitterCount > 1 && m_emitterIndex < m_emitterCount && __effect_uses_emitter( m_effect, m_emitters[m_emitterIndex] ) == DZ_FALSE )
            {
                dz_emitter_t * emitter = m_emitters[m_emitterIndex];

                dz_emitter_destroy( m_service, emitter );
                __resource_erase( m_emitters, m_emitterInfos, &m_emitterCount, m_emitterIndex );

                if( m_emitterIndex >= m_emitterCount )
                {
                    m_emitterIndex = m_emitterCount - 1;
                }

                if( this->selectEmitterResource( m_emitterIndex ) == DZ_FAILURE )
                {
                    return DZ_FAILURE;
                }
            }

            ImGui::Separator();

            for( dz_uint32_t index = 0; index != m_emitterCount; ++index )
            {
                if( ImGui::Selectable( m_emitterInfos[index].name, index == m_emitterIndex ) == true )
                {
                    if( this->selectEmitterResource( index ) == DZ_FAILURE )
                    {
                        return DZ_FAILURE;
                    }
                }
            }
        }
        break;
    case ER_WINDOW_TYPE_AFFECTOR_DATA:
        {
            if( ImGui::Button( "+##Affector" ) == true )
            {
                if( m_affectorCount >= ER_EDITOR_RESOURCE_MAX )
                {
                    return DZ_FAILURE;
                }

                dz_affector_t * affector;
                if( this->createDefaultAffector( &affector ) == DZ_FAILURE )
                {
                    return DZ_FAILURE;
                }

                const dz_uint32_t index = m_affectorCount++;
                m_affectors[index] = affector;
                __editor_set_default_name( m_affectorInfos + index, &m_nextEditorInstanceId, "Affector", index );

                if( this->selectAffectorResource( index ) == DZ_FAILURE )
                {
                    return DZ_FAILURE;
                }
            }

            ImGui::SameLine();

            if( ImGui::Button( "-##Affector" ) == true && m_affectorCount > 1 && m_affectorIndex < m_affectorCount && __effect_uses_affector( m_effect, m_affectors[m_affectorIndex] ) == DZ_FALSE )
            {
                dz_affector_t * affector = m_affectors[m_affectorIndex];

                dz_affector_destroy( m_service, affector );
                __resource_erase( m_affectors, m_affectorInfos, &m_affectorCount, m_affectorIndex );

                if( m_affectorIndex >= m_affectorCount )
                {
                    m_affectorIndex = m_affectorCount - 1;
                }

                if( this->selectAffectorResource( m_affectorIndex ) == DZ_FAILURE )
                {
                    return DZ_FAILURE;
                }
            }

            ImGui::Separator();

            for( dz_uint32_t index = 0; index != m_affectorCount; ++index )
            {
                if( ImGui::Selectable( m_affectorInfos[index].name, index == m_affectorIndex ) == true )
                {
                    if( this->selectAffectorResource( index ) == DZ_FAILURE )
                    {
                        return DZ_FAILURE;
                    }
                }
            }
        }
        break;
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t editor::showComposerData()
{
    ImGui::Spacing();
    ImGui::Text( ER_WINDOW_COMPOSER_TITLE );
    ImGui::Separator();

    dz_uint32_t layerCount = dz_effect_get_layer_count( m_effect );

    if( layerCount != 0 )
    {
        dz_effect_layer_desc_t layer;
        if( dz_effect_get_layer( m_effect, m_layerIndex, &layer ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        __editor_name_input( ER_WINDOW_COMPOSER_LAYER_NAME_LABEL, m_layerInfos, layerCount, m_layerIndex, "Layer" );

        ImGui::Separator();

        bool layerChanged = false;
        bool layerResourcesChanged = false;

        dz_uint32_t materialIndex = __resource_index_of( m_materials, m_materialCount, layer.material );
        dz_uint32_t selectedMaterialIndex = materialIndex;
        if( __resource_index_combo( ER_WINDOW_COMPOSER_LAYER_MATERIAL_LABEL, m_materialInfos, m_materialCount, materialIndex, &selectedMaterialIndex ) == true )
        {
            layer.material = m_materials[selectedMaterialIndex];
            layerChanged = true;
            layerResourcesChanged = true;
        }

        ImGui::SameLine();
        if( ImGui::Button( ">##EditMaterial" ) == true && selectedMaterialIndex < m_materialCount )
        {
            if( this->selectMaterialResource( selectedMaterialIndex ) == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }

            m_windowType = ER_WINDOW_TYPE_MATERIAL_DATA;
        }

        dz_uint32_t shapeIndex = __resource_index_of( m_shapes, m_shapeCount, layer.shape );
        dz_uint32_t selectedShapeIndex = shapeIndex;
        if( __resource_index_combo( ER_WINDOW_COMPOSER_LAYER_SHAPE_LABEL, m_shapeInfos, m_shapeCount, shapeIndex, &selectedShapeIndex ) == true )
        {
            layer.shape = m_shapes[selectedShapeIndex];
            layerChanged = true;
            layerResourcesChanged = true;
        }

        ImGui::SameLine();
        if( ImGui::Button( ">##EditShape" ) == true && selectedShapeIndex < m_shapeCount )
        {
            if( this->selectShapeResource( selectedShapeIndex ) == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }

            m_windowType = ER_WINDOW_TYPE_SHAPE_DATA;
        }

        dz_uint32_t emitterIndex = __resource_index_of( m_emitters, m_emitterCount, layer.emitter );
        dz_uint32_t selectedEmitterIndex = emitterIndex;
        if( __resource_index_combo( ER_WINDOW_COMPOSER_LAYER_EMITTER_LABEL, m_emitterInfos, m_emitterCount, emitterIndex, &selectedEmitterIndex ) == true )
        {
            layer.emitter = m_emitters[selectedEmitterIndex];
            layerChanged = true;
            layerResourcesChanged = true;
        }

        ImGui::SameLine();
        if( ImGui::Button( ">##EditEmitter" ) == true && selectedEmitterIndex < m_emitterCount )
        {
            if( this->selectEmitterResource( selectedEmitterIndex ) == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }

            m_windowType = ER_WINDOW_TYPE_EMITTER_DATA;
        }

        dz_uint32_t affectorIndex = __resource_index_of( m_affectors, m_affectorCount, layer.affector );
        dz_uint32_t selectedAffectorIndex = affectorIndex;
        if( __resource_index_combo( ER_WINDOW_COMPOSER_LAYER_AFFECTOR_LABEL, m_affectorInfos, m_affectorCount, affectorIndex, &selectedAffectorIndex ) == true )
        {
            layer.affector = m_affectors[selectedAffectorIndex];
            layerChanged = true;
            layerResourcesChanged = true;
        }

        ImGui::SameLine();
        if( ImGui::Button( ">##EditAffector" ) == true && selectedAffectorIndex < m_affectorCount )
        {
            if( this->selectAffectorResource( selectedAffectorIndex ) == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }

            m_windowType = ER_WINDOW_TYPE_AFFECTOR_DATA;
        }

        ImGui::Separator();

        layerChanged |= ImGui::InputFloat( ER_WINDOW_COMPOSER_LAYER_X_LABEL, &layer.x, 1.f, 10.f, "%.2f", ImGuiInputTextFlags_None );
        layerChanged |= ImGui::InputFloat( ER_WINDOW_COMPOSER_LAYER_Y_LABEL, &layer.y, 1.f, 10.f, "%.2f", ImGuiInputTextFlags_None );
        layerChanged |= ImGui::InputFloat( ER_WINDOW_COMPOSER_LAYER_ANGLE_LABEL, &layer.angle, 0.1f, 1.f, "%.3f", ImGuiInputTextFlags_None );
        layerChanged |= ImGui::InputFloat( ER_WINDOW_COMPOSER_LAYER_LIFE_LABEL, &layer.life, 0.1f, 1.f, "%.3f", ImGuiInputTextFlags_None );

        int layerSeed = (int)layer.seed;
        if( ImGui::InputInt( ER_WINDOW_COMPOSER_LAYER_SEED_LABEL, &layerSeed, 1, 16, ImGuiInputTextFlags_None ) == true )
        {
            layer.seed = (dz_uint32_t)DZ_MAX( 0, layerSeed );
            layerChanged = true;
        }

        if( layerChanged == true )
        {
            if( layer.life < 0.f )
            {
                layer.life = 0.f;
            }

            if( dz_effect_set_layer( m_effect, m_layerIndex, &layer ) == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }

            if( layerResourcesChanged == true )
            {
                if( this->selectLayer( m_layerIndex ) == DZ_FAILURE )
                {
                    return DZ_FAILURE;
                }
            }

            this->resetEffect();
        }
    }

    if( layerCount == 0 )
    {
        return DZ_SUCCESSFUL;
    }

    ImGui::Separator();
    ImGui::Text( ER_WINDOW_COMPOSER_LAYER_TRIGGERS_TITLE );

    dz_uint32_t triggerIndex;
    if( this->ensureLayerTrigger( m_layerIndex, &triggerIndex ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    dz_effect_trigger_desc_t trigger;
    if( dz_effect_get_trigger( m_effect, triggerIndex, &trigger ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    bool triggerChanged = false;

    int eventType = (int)trigger.event_type;
    if( ImGui::Combo( ER_WINDOW_COMPOSER_TRIGGER_EVENT_LABEL, &eventType, ER_EFFECT_EVENT_NAMES, IM_ARRAYSIZE( ER_EFFECT_EVENT_NAMES ) ) == true )
    {
        trigger.event_type = (dz_effect_event_type_e)eventType;
        triggerChanged = true;
    }

    const dz_bool_t triggerUsesSourceLayer = __trigger_event_uses_source_layer( trigger.event_type );
    const dz_bool_t triggerUsesTime = __trigger_event_uses_time( trigger.event_type );
    const dz_bool_t triggerUsesInherit = __trigger_event_uses_inherit( trigger.event_type );

    if( triggerUsesSourceLayer == DZ_TRUE )
    {
        if( trigger.source_layer_index == DZ_EFFECT_LAYER_NONE || trigger.source_layer_index >= layerCount )
        {
            trigger.source_layer_index = m_layerIndex;
            triggerChanged = true;
        }

        dz_uint32_t sourceLayer = trigger.source_layer_index;
        if( __layer_index_combo( ER_WINDOW_COMPOSER_TRIGGER_SOURCE_LABEL, m_layerInfos, layerCount, DZ_FALSE, trigger.source_layer_index, &sourceLayer ) == true )
        {
            trigger.source_layer_index = sourceLayer;
            triggerChanged = true;
        }
    }

    if( triggerUsesTime == DZ_TRUE )
    {
        triggerChanged |= ImGui::InputFloat( ER_WINDOW_COMPOSER_TRIGGER_TIME_LABEL, &trigger.time, 0.1f, 1.f, "%.3f", ImGuiInputTextFlags_None );
    }

    triggerChanged |= ImGui::SliderFloat( ER_WINDOW_COMPOSER_TRIGGER_PROBABILITY_LABEL, &trigger.probability, 0.f, 1.f, "%.3f" );

    int spawnMin = (int)trigger.spawn_count_min;
    int spawnMax = (int)trigger.spawn_count_max;
    triggerChanged |= ImGui::InputInt( ER_WINDOW_COMPOSER_TRIGGER_COUNT_MIN_LABEL, &spawnMin, 1, 1, ImGuiInputTextFlags_None );
    triggerChanged |= ImGui::InputInt( ER_WINDOW_COMPOSER_TRIGGER_COUNT_MAX_LABEL, &spawnMax, 1, 1, ImGuiInputTextFlags_None );

    triggerChanged |= ImGui::InputFloat( ER_WINDOW_COMPOSER_TRIGGER_DELAY_MIN_LABEL, &trigger.delay_min, 0.1f, 1.f, "%.3f", ImGuiInputTextFlags_None );
    triggerChanged |= ImGui::InputFloat( ER_WINDOW_COMPOSER_TRIGGER_DELAY_MAX_LABEL, &trigger.delay_max, 0.1f, 1.f, "%.3f", ImGuiInputTextFlags_None );

    bool inheritPosition = trigger.inherit_position == DZ_TRUE;
    bool inheritAngle = trigger.inherit_angle == DZ_TRUE;
    bool inheritVelocity = trigger.inherit_velocity == DZ_TRUE;

    if( triggerUsesInherit == DZ_TRUE )
    {
        triggerChanged |= ImGui::Checkbox( ER_WINDOW_COMPOSER_TRIGGER_INHERIT_POSITION_LABEL, &inheritPosition );
        triggerChanged |= ImGui::Checkbox( ER_WINDOW_COMPOSER_TRIGGER_INHERIT_ANGLE_LABEL, &inheritAngle );
        triggerChanged |= ImGui::Checkbox( ER_WINDOW_COMPOSER_TRIGGER_INHERIT_VELOCITY_LABEL, &inheritVelocity );
    }

    triggerChanged |= ImGui::InputFloat( ER_WINDOW_COMPOSER_TRIGGER_OFFSET_X_LABEL, &trigger.offset_x, 1.f, 10.f, "%.2f", ImGuiInputTextFlags_None );
    triggerChanged |= ImGui::InputFloat( ER_WINDOW_COMPOSER_TRIGGER_OFFSET_Y_LABEL, &trigger.offset_y, 1.f, 10.f, "%.2f", ImGuiInputTextFlags_None );
    triggerChanged |= ImGui::InputFloat( ER_WINDOW_COMPOSER_TRIGGER_ANGLE_OFFSET_LABEL, &trigger.angle_offset, 0.1f, 1.f, "%.3f", ImGuiInputTextFlags_None );

    if( triggerChanged == true )
    {
        if( triggerUsesSourceLayer == DZ_FALSE )
        {
            trigger.source_layer_index = DZ_EFFECT_LAYER_NONE;
        }

        trigger.target_layer_index = m_layerIndex;

        spawnMin = DZ_MAX( 0, spawnMin );
        spawnMax = DZ_MAX( spawnMin, spawnMax );
        trigger.spawn_count_min = (dz_uint32_t)spawnMin;
        trigger.spawn_count_max = (dz_uint32_t)spawnMax;

        if( trigger.delay_max < trigger.delay_min )
        {
            trigger.delay_max = trigger.delay_min;
        }

        trigger.inherit_position = inheritPosition == true ? DZ_TRUE : DZ_FALSE;
        trigger.inherit_angle = inheritAngle == true ? DZ_TRUE : DZ_FALSE;
        trigger.inherit_velocity = inheritVelocity == true ? DZ_TRUE : DZ_FALSE;

        if( triggerUsesInherit == DZ_FALSE )
        {
            trigger.inherit_position = DZ_FALSE;
            trigger.inherit_angle = DZ_FALSE;
            trigger.inherit_velocity = DZ_FALSE;
        }

        if( dz_effect_set_trigger( m_effect, triggerIndex, &trigger ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        this->resetEffect();
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t editor::showShapeData()
{
    static int selected_type = m_shapeType;

    selected_type = m_shapeType;

    if( m_shapeIndex < m_shapeCount )
    {
        __editor_name_input( ER_WINDOW_RESOURCE_NAME_LABEL, m_shapeInfos, m_shapeCount, m_shapeIndex, "Shape" );
    }

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

    dz_float_t width = ImGui::GetContentRegionAvail().x;
    ImVec2 size( width, width * ER_CURVE_BOX_HEIGHT_TO_WIDTH_RATIO );

    static bool headerFlags[__DZ_SHAPE_TIMELINE_MAX__] = {false};

    ImGui::Separator();

    for( dz_uint32_t index = 0; index != __DZ_SHAPE_TIMELINE_MAX__; ++index )
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
            if( data.type >= DZ_SHAPE_LINE_ANGLE && data.type <= DZ_SHAPE_LINE_OFFSET )
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
                dz_float_t min = 0.f, max = 0.f, default_value = 0.f, factor = 0.f;
                dz_shape_timeline_get_limit( data.type, &status, &min, &max, &default_value, &factor );
                dz_float_t life = dz_effect_get_life( m_effect );

                // curve
                dz_float_t x_min = 0.f;
                dz_float_t x_max = life;

                dz_float_t y_min = min;
                dz_float_t y_max = max;

                __setupLimits( data.pointsData, status, min, max, &factor, &(data.zoom), &y_min, &y_max, false );

                dz_float_t y_range = y_max - y_min;

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
dz_result_t editor::showAffectorData()
{
    if( m_affectorIndex < m_affectorCount )
    {
        __editor_name_input( ER_WINDOW_RESOURCE_NAME_LABEL, m_affectorInfos, m_affectorCount, m_affectorIndex, "Affector" );
    }

    // timeline
    ImGui::Spacing();
    ImGui::Text( ER_WINDOW_AFFECTOR_TITLE );

    dz_float_t width = ImGui::GetContentRegionAvail().x;
    ImVec2 size( width, width * ER_CURVE_BOX_HEIGHT_TO_WIDTH_RATIO );

    static bool headerFlags[__DZ_AFFECTOR_TIMELINE_MAX__] = {false};

    ImGui::Separator();

    for( dz_uint32_t index = 0; index != __DZ_AFFECTOR_TIMELINE_MAX__; ++index )
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
            dz_float_t min = 0.f, max = 0.f, default_value = 0.f, factor = 0.f;
            dz_affector_timeline_get_limit( data.type, &status, &min, &max, &default_value, &factor );
            dz_float_t life = dz_effect_get_life( m_effect );

            // curve
            dz_float_t x_min = 0.f;
            dz_float_t x_max = life;

            dz_float_t y_min = min;
            dz_float_t y_max = max;

            __setupLimits( data.pointsData, status, min, max, &factor, &(data.zoom), &y_min, &y_max, false );

            dz_float_t y_range = y_max - y_min;

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
dz_result_t editor::showEmitterData()
{
    if( m_emitterIndex < m_emitterCount )
    {
        __editor_name_input( ER_WINDOW_RESOURCE_NAME_LABEL, m_emitterInfos, m_emitterCount, m_emitterIndex, "Emitter" );
    }

    // timeline
    ImGui::Spacing();
    ImGui::Text( ER_WINDOW_EMITTER_TITLE );

    dz_float_t width = ImGui::GetContentRegionAvail().x;
    ImVec2 size( width, width * ER_CURVE_BOX_HEIGHT_TO_WIDTH_RATIO );

    static bool headerFlags[__DZ_EMITTER_TIMELINE_MAX__] = {false};

    ImGui::Separator();

    for( dz_uint32_t index = 0; index != __DZ_EMITTER_TIMELINE_MAX__; ++index )
    {
        timeline_emitter_t & data = m_timelineEmitterData[index];

        ImGui::PushID( index );

        ImGui::Checkbox( data.name, &headerFlags[index] );

        if( headerFlags[index] == true )
        {
            dz_timeline_limit_status_e status;
            dz_float_t min = 0.f, max = 0.f, default_value = 0.f, factor = 0.f;
            dz_emitter_timeline_get_limit( data.type, &status, &min, &max, &default_value, &factor );
            dz_float_t life = dz_effect_get_life( m_effect );

            // curve
            dz_float_t x_min = 0.f;
            dz_float_t x_max = life;

            dz_float_t y_min = min;
            dz_float_t y_max = max;

            // inv
            if( index == DZ_EMITTER_SPAWN_DELAY )
            {
                __setupLimits( data.pointsData, status, min, max, &factor, &(data.zoom), &y_min, &y_max, true );

                dz_float_t y_range = y_max - y_min;

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
                __setupLimits( data.pointsData, status, min, max, &factor, &(data.zoom), &y_min, &y_max, false );

                dz_float_t y_range = y_max - y_min;

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
typedef struct er_atlas_pack_region_t
{
    dz_texture_t * texture;

    dz_int32_t sourceX;
    dz_int32_t sourceY;
    dz_int32_t width;
    dz_int32_t height;

    dz_int32_t packedX;
    dz_int32_t packedY;
} er_atlas_pack_region_t;
//////////////////////////////////////////////////////////////////////////
static void __append_png_u32( std::vector<dz_uint8_t> * const _output, dz_uint32_t _value )
{
    _output->push_back( (dz_uint8_t)((_value >> 24) & 0xff) );
    _output->push_back( (dz_uint8_t)((_value >> 16) & 0xff) );
    _output->push_back( (dz_uint8_t)((_value >> 8) & 0xff) );
    _output->push_back( (dz_uint8_t)(_value & 0xff) );
}
//////////////////////////////////////////////////////////////////////////
static void __append_png_chunk( std::vector<dz_uint8_t> * const _output, const char * _type, const dz_uint8_t * _data, dz_size_t _size )
{
    __append_png_u32( _output, (dz_uint32_t)_size );

    const dz_uint8_t typeBytes[4] = {(dz_uint8_t)_type[0], (dz_uint8_t)_type[1], (dz_uint8_t)_type[2], (dz_uint8_t)_type[3]};

    _output->insert( _output->end(), typeBytes, typeBytes + 4 );

    if( _size != 0 )
    {
        _output->insert( _output->end(), _data, _data + _size );
    }

    uLong chunkCrc = crc32( 0L, Z_NULL, 0 );
    chunkCrc = crc32( chunkCrc, typeBytes, 4 );

    if( _size != 0 )
    {
        chunkCrc = crc32( chunkCrc, _data, (uInt)_size );
    }

    __append_png_u32( _output, (dz_uint32_t)chunkCrc );
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __encode_png_rgba( const dz_uint8_t * _pixels, dz_int32_t _width, dz_int32_t _height, std::vector<dz_uint8_t> * const _output )
{
    if( _pixels == DZ_NULLPTR || _width <= 0 || _height <= 0 )
    {
        return DZ_FAILURE;
    }

    const dz_size_t pixelStride = 4;
    const dz_size_t sourceStride = (dz_size_t)_width * pixelStride;
    const dz_size_t filteredStride = sourceStride + 1;

    std::vector<dz_uint8_t> filtered;
    filtered.resize( filteredStride * (dz_size_t)_height );

    for( dz_int32_t row = 0; row != _height; ++row )
    {
        dz_uint8_t * filteredRow = filtered.data() + (dz_size_t)row * filteredStride;
        const dz_uint8_t * sourceRow = _pixels + (dz_size_t)row * sourceStride;

        filteredRow[0] = 0;
        std::memcpy( filteredRow + 1, sourceRow, sourceStride );
    }

    uLongf compressedSize = compressBound( (uLong)filtered.size() );

    std::vector<dz_uint8_t> compressed;
    compressed.resize( (dz_size_t)compressedSize );

    if( compress2( compressed.data(), &compressedSize, filtered.data(), (uLong)filtered.size(), Z_BEST_COMPRESSION ) != Z_OK )
    {
        return DZ_FAILURE;
    }

    compressed.resize( (dz_size_t)compressedSize );

    _output->clear();

    const dz_uint8_t pngSignature[8] = {137, 80, 78, 71, 13, 10, 26, 10};
    _output->insert( _output->end(), pngSignature, pngSignature + 8 );

    dz_uint8_t ihdr[13];
    ihdr[0] = (dz_uint8_t)((_width >> 24) & 0xff);
    ihdr[1] = (dz_uint8_t)((_width >> 16) & 0xff);
    ihdr[2] = (dz_uint8_t)((_width >> 8) & 0xff);
    ihdr[3] = (dz_uint8_t)(_width & 0xff);
    ihdr[4] = (dz_uint8_t)((_height >> 24) & 0xff);
    ihdr[5] = (dz_uint8_t)((_height >> 16) & 0xff);
    ihdr[6] = (dz_uint8_t)((_height >> 8) & 0xff);
    ihdr[7] = (dz_uint8_t)(_height & 0xff);
    ihdr[8] = 8;
    ihdr[9] = 6;
    ihdr[10] = 0;
    ihdr[11] = 0;
    ihdr[12] = 0;

    __append_png_chunk( _output, "IHDR", ihdr, sizeof( ihdr ) );
    __append_png_chunk( _output, "IDAT", compressed.data(), compressed.size() );
    __append_png_chunk( _output, "IEND", DZ_NULLPTR, 0 );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __get_texture_region_pixels_int( const dz_texture_t * _texture, dz_int32_t _atlasWidth, dz_int32_t _atlasHeight, er_atlas_pack_region_t * const _region )
{
    if( _atlasWidth <= 0 || _atlasHeight <= 0 )
    {
        return DZ_FAILURE;
    }

    dz_float_t region[4];
    __get_texture_region_pixels( _texture, _atlasWidth, _atlasHeight, region );

    dz_int32_t sourceX0 = (dz_int32_t)floorf( region[0] );
    dz_int32_t sourceY0 = (dz_int32_t)floorf( region[1] );
    dz_int32_t sourceX1 = (dz_int32_t)ceilf( region[0] + region[2] );
    dz_int32_t sourceY1 = (dz_int32_t)ceilf( region[1] + region[3] );

    sourceX0 = DZ_MAX( 0, DZ_MIN( sourceX0, _atlasWidth - 1 ) );
    sourceY0 = DZ_MAX( 0, DZ_MIN( sourceY0, _atlasHeight - 1 ) );
    sourceX1 = DZ_MAX( sourceX0 + 1, DZ_MIN( sourceX1, _atlasWidth ) );
    sourceY1 = DZ_MAX( sourceY0 + 1, DZ_MIN( sourceY1, _atlasHeight ) );

    _region->sourceX = sourceX0;
    _region->sourceY = sourceY0;
    _region->width = sourceX1 - sourceX0;
    _region->height = sourceY1 - sourceY0;
    _region->packedX = 0;
    _region->packedY = 0;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static void __pack_atlas_regions_shelf( std::vector<er_atlas_pack_region_t> * const _regions, const std::vector<dz_uint32_t> & _order, dz_int32_t _candidateWidth, dz_bool_t _apply, dz_int32_t * const _outWidth, dz_int32_t * const _outHeight )
{
    dz_int32_t cursorX = 0;
    dz_int32_t cursorY = 0;
    dz_int32_t rowHeight = 0;
    dz_int32_t usedWidth = 0;

    for( dz_uint32_t regionIndex : _order )
    {
        er_atlas_pack_region_t & region = (*_regions)[regionIndex];

        if( cursorX != 0 && cursorX + region.width > _candidateWidth )
        {
            cursorY += rowHeight;
            cursorX = 0;
            rowHeight = 0;
        }

        if( _apply == DZ_TRUE )
        {
            region.packedX = cursorX;
            region.packedY = cursorY;
        }

        cursorX += region.width;
        rowHeight = DZ_MAX( rowHeight, region.height );
        usedWidth = DZ_MAX( usedWidth, cursorX );
    }

    *_outWidth = usedWidth;
    *_outHeight = cursorY + rowHeight;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __pack_atlas_regions( std::vector<er_atlas_pack_region_t> * const _regions, dz_int32_t * const _outWidth, dz_int32_t * const _outHeight )
{
    if( _regions->empty() == true )
    {
        return DZ_FAILURE;
    }

    std::vector<dz_uint32_t> order;
    order.reserve( _regions->size() );

    dz_int32_t maxRegionWidth = 0;
    dz_int32_t totalRegionWidth = 0;

    for( dz_uint32_t index = 0; index != (dz_uint32_t)_regions->size(); ++index )
    {
        const er_atlas_pack_region_t & region = (*_regions)[index];

        order.push_back( index );

        maxRegionWidth = DZ_MAX( maxRegionWidth, region.width );
        totalRegionWidth += region.width;
    }

    std::sort( order.begin(), order.end(), [_regions]( dz_uint32_t _left, dz_uint32_t _right )
    {
        const er_atlas_pack_region_t & leftRegion = (*_regions)[_left];
        const er_atlas_pack_region_t & rightRegion = (*_regions)[_right];

        if( leftRegion.height != rightRegion.height )
        {
            return leftRegion.height > rightRegion.height;
        }

        return leftRegion.width > rightRegion.width;
    } );

    dz_int32_t bestCandidateWidth = maxRegionWidth;
    dz_int32_t bestPackedWidth = 0;
    dz_int32_t bestPackedHeight = 0;
    dz_size_t bestArea = (dz_size_t)-1;
    dz_size_t bestMaxSide = (dz_size_t)-1;
    dz_size_t bestSideDelta = (dz_size_t)-1;

    for( dz_int32_t candidateWidth = maxRegionWidth; candidateWidth <= totalRegionWidth; ++candidateWidth )
    {
        dz_int32_t packedWidth;
        dz_int32_t packedHeight;
        __pack_atlas_regions_shelf( _regions, order, candidateWidth, DZ_FALSE, &packedWidth, &packedHeight );

        const dz_size_t area = (dz_size_t)packedWidth * (dz_size_t)packedHeight;
        const dz_size_t maxSide = (dz_size_t)DZ_MAX( packedWidth, packedHeight );
        const dz_size_t minSide = (dz_size_t)DZ_MIN( packedWidth, packedHeight );
        const dz_size_t sideDelta = maxSide - minSide;

        if( maxSide < bestMaxSide || (maxSide == bestMaxSide && area < bestArea) || (maxSide == bestMaxSide && area == bestArea && sideDelta < bestSideDelta) )
        {
            bestCandidateWidth = candidateWidth;
            bestPackedWidth = packedWidth;
            bestPackedHeight = packedHeight;
            bestArea = area;
            bestMaxSide = maxSide;
            bestSideDelta = sideDelta;
        }
    }

    __pack_atlas_regions_shelf( _regions, order, bestCandidateWidth, DZ_TRUE, &bestPackedWidth, &bestPackedHeight );

    *_outWidth = bestPackedWidth;
    *_outHeight = bestPackedHeight;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t editor::optimizeAtlas()
{
    if( m_atlas == DZ_NULLPTR || m_atlasBuffer.empty() == true )
    {
        return DZ_FAILURE;
    }

    dz_int32_t sourceWidth;
    dz_int32_t sourceHeight;
    dz_int32_t sourceComp;

    dz_uint8_t * sourcePixels = stbi_load_from_memory( m_atlasBuffer.data(), (int)m_atlasBuffer.size(), &sourceWidth, &sourceHeight, &sourceComp, STBI_rgb_alpha );

    if( sourcePixels == DZ_NULLPTR )
    {
        return DZ_FAILURE;
    }

    DZ_UNUSED( sourceComp );

    std::vector<dz_texture_t *> materialTextures;
    for( dz_uint32_t materialIndex = 0; materialIndex != m_materialCount; ++materialIndex )
    {
        dz_material_t * material = m_materials[materialIndex];
        const dz_uint32_t textureCount = dz_material_get_texture_slot_count( material );

        for( dz_uint32_t textureIndex = 0; textureIndex != textureCount; ++textureIndex )
        {
            const dz_texture_t * texture = DZ_NULLPTR;
            if( dz_material_get_texture( material, textureIndex, &texture ) == DZ_FAILURE )
            {
                stbi_image_free( sourcePixels );

                return DZ_FAILURE;
            }

            dz_texture_t * mutableTexture = const_cast<dz_texture_t *>(texture);
            if( std::find( materialTextures.begin(), materialTextures.end(), mutableTexture ) == materialTextures.end() )
            {
                materialTextures.push_back( mutableTexture );
            }
        }
    }

    std::vector<er_atlas_pack_region_t> regions;
    regions.reserve( materialTextures.size() );

    for( dz_texture_t * texture : materialTextures )
    {
        er_atlas_pack_region_t region;
        region.texture = texture;

        if( __get_texture_region_pixels_int( texture, sourceWidth, sourceHeight, &region ) == DZ_FAILURE )
        {
            stbi_image_free( sourcePixels );

            return DZ_FAILURE;
        }

        regions.push_back( region );
    }

    if( regions.empty() == true )
    {
        stbi_image_free( sourcePixels );

        return DZ_FAILURE;
    }

    dz_int32_t packedWidth;
    dz_int32_t packedHeight;
    if( __pack_atlas_regions( &regions, &packedWidth, &packedHeight ) == DZ_FAILURE )
    {
        stbi_image_free( sourcePixels );

        return DZ_FAILURE;
    }

    std::vector<dz_uint8_t> packedPixels;
    packedPixels.resize( (dz_size_t)packedWidth * (dz_size_t)packedHeight * 4, 0 );

    for( const er_atlas_pack_region_t & region : regions )
    {
        for( dz_int32_t row = 0; row != region.height; ++row )
        {
            const dz_uint8_t * sourceRow = sourcePixels + (((dz_size_t)(region.sourceY + row) * (dz_size_t)sourceWidth + (dz_size_t)region.sourceX) * 4);
            dz_uint8_t * packedRow = packedPixels.data() + (((dz_size_t)(region.packedY + row) * (dz_size_t)packedWidth + (dz_size_t)region.packedX) * 4);

            std::memcpy( packedRow, sourceRow, (dz_size_t)region.width * 4 );
        }
    }

    stbi_image_free( sourcePixels );

    std::vector<dz_uint8_t> optimizedAtlasBuffer;
    if( __encode_png_rgba( packedPixels.data(), packedWidth, packedHeight, &optimizedAtlasBuffer ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    dz_int32_t optimizedWidth;
    dz_int32_t optimizedHeight;
    GLuint optimizedTextureId = dz_render_make_texture_from_memory( optimizedAtlasBuffer.data(), optimizedAtlasBuffer.size(), &optimizedWidth, &optimizedHeight );

    if( optimizedTextureId == 0 )
    {
        return DZ_FAILURE;
    }

    for( er_atlas_pack_region_t & region : regions )
    {
        dz_float_t textureRegion[4] = {(dz_float_t)region.packedX, (dz_float_t)region.packedY, (dz_float_t)region.width, (dz_float_t)region.height};

        __set_texture_region_pixels( region.texture, optimizedWidth, optimizedHeight, textureRegion );
    }

    dz_render_delete_texture( m_textureId );

    m_textureId = optimizedTextureId;
    m_textureWidth = optimizedWidth;
    m_textureHeight = optimizedHeight;
    m_atlasBuffer.swap( optimizedAtlasBuffer );

    dz_atlas_set_surface( m_atlas, &m_textureId );

    if( __select_material_texture( m_material, &m_textureIndex, &m_texture ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    m_textureRegionSelecting = false;

    return this->resetEffect();
}
//////////////////////////////////////////////////////////////////////////
dz_result_t editor::appendTextureToAtlas( const char * _path )
{
    if( _path == DZ_NULLPTR || m_atlas == DZ_NULLPTR || m_atlasBuffer.empty() == true || m_textureWidth <= 0 || m_textureHeight <= 0 )
    {
        return DZ_FAILURE;
    }

    dz_int32_t atlasWidth;
    dz_int32_t atlasHeight;
    dz_int32_t atlasComp;

    dz_uint8_t * atlasPixels = stbi_load_from_memory( m_atlasBuffer.data(), (int)m_atlasBuffer.size(), &atlasWidth, &atlasHeight, &atlasComp, STBI_rgb_alpha );

    if( atlasPixels == DZ_NULLPTR )
    {
        return DZ_FAILURE;
    }

    DZ_UNUSED( atlasComp );

    dz_int32_t appendWidth;
    dz_int32_t appendHeight;
    dz_int32_t appendComp;

    dz_uint8_t * appendPixels = stbi_load( _path, &appendWidth, &appendHeight, &appendComp, STBI_rgb_alpha );

    if( appendPixels == DZ_NULLPTR )
    {
        stbi_image_free( atlasPixels );

        return DZ_FAILURE;
    }

    DZ_UNUSED( appendComp );

    dz_int32_t appendXRight = atlasWidth;
    dz_int32_t appendYRight = 0;
    dz_int32_t newWidthRight = atlasWidth + appendWidth;
    dz_int32_t newHeightRight = DZ_MAX( atlasHeight, appendHeight );

    dz_int32_t appendXBottom = 0;
    dz_int32_t appendYBottom = atlasHeight;
    dz_int32_t newWidthBottom = DZ_MAX( atlasWidth, appendWidth );
    dz_int32_t newHeightBottom = atlasHeight + appendHeight;

    const dz_size_t rightMaxSide = (dz_size_t)DZ_MAX( newWidthRight, newHeightRight );
    const dz_size_t rightArea = (dz_size_t)newWidthRight * (dz_size_t)newHeightRight;
    const dz_size_t rightSideDelta = rightMaxSide - (dz_size_t)DZ_MIN( newWidthRight, newHeightRight );

    const dz_size_t bottomMaxSide = (dz_size_t)DZ_MAX( newWidthBottom, newHeightBottom );
    const dz_size_t bottomArea = (dz_size_t)newWidthBottom * (dz_size_t)newHeightBottom;
    const dz_size_t bottomSideDelta = bottomMaxSide - (dz_size_t)DZ_MIN( newWidthBottom, newHeightBottom );

    dz_int32_t appendX;
    dz_int32_t appendY;
    dz_int32_t newWidth;
    dz_int32_t newHeight;

    if( bottomMaxSide < rightMaxSide || (bottomMaxSide == rightMaxSide && bottomArea < rightArea) || (bottomMaxSide == rightMaxSide && bottomArea == rightArea && bottomSideDelta < rightSideDelta) )
    {
        appendX = appendXBottom;
        appendY = appendYBottom;
        newWidth = newWidthBottom;
        newHeight = newHeightBottom;
    }
    else
    {
        appendX = appendXRight;
        appendY = appendYRight;
        newWidth = newWidthRight;
        newHeight = newHeightRight;
    }

    typedef struct er_existing_texture_region_t
    {
        dz_texture_t * texture;
        dz_float_t region[4];
    } er_existing_texture_region_t;

    std::vector<dz_texture_t *> materialTextures;
    for( dz_uint32_t materialIndex = 0; materialIndex != m_materialCount; ++materialIndex )
    {
        dz_material_t * material = m_materials[materialIndex];
        const dz_uint32_t textureCount = dz_material_get_texture_slot_count( material );

        for( dz_uint32_t textureIndex = 0; textureIndex != textureCount; ++textureIndex )
        {
            const dz_texture_t * texture = DZ_NULLPTR;
            if( dz_material_get_texture( material, textureIndex, &texture ) == DZ_FAILURE )
            {
                stbi_image_free( appendPixels );
                stbi_image_free( atlasPixels );

                return DZ_FAILURE;
            }

            dz_texture_t * mutableTexture = const_cast<dz_texture_t *>(texture);
            if( std::find( materialTextures.begin(), materialTextures.end(), mutableTexture ) == materialTextures.end() )
            {
                materialTextures.push_back( mutableTexture );
            }
        }
    }

    std::vector<er_existing_texture_region_t> existingRegions;
    existingRegions.reserve( materialTextures.size() );

    for( dz_texture_t * texture : materialTextures )
    {
        er_existing_texture_region_t textureRegion;
        textureRegion.texture = texture;
        __get_texture_region_pixels( texture, atlasWidth, atlasHeight, textureRegion.region );

        existingRegions.push_back( textureRegion );
    }

    std::vector<dz_uint8_t> appendedPixels;
    appendedPixels.resize( (dz_size_t)newWidth * (dz_size_t)newHeight * 4, 0 );

    for( dz_int32_t row = 0; row != atlasHeight; ++row )
    {
        const dz_uint8_t * sourceRow = atlasPixels + ((dz_size_t)row * (dz_size_t)atlasWidth * 4);
        dz_uint8_t * targetRow = appendedPixels.data() + ((dz_size_t)row * (dz_size_t)newWidth * 4);

        std::memcpy( targetRow, sourceRow, (dz_size_t)atlasWidth * 4 );
    }

    for( dz_int32_t row = 0; row != appendHeight; ++row )
    {
        const dz_uint8_t * sourceRow = appendPixels + ((dz_size_t)row * (dz_size_t)appendWidth * 4);
        dz_uint8_t * targetRow = appendedPixels.data() + (((dz_size_t)(appendY + row) * (dz_size_t)newWidth + (dz_size_t)appendX) * 4);

        std::memcpy( targetRow, sourceRow, (dz_size_t)appendWidth * 4 );
    }

    stbi_image_free( appendPixels );
    stbi_image_free( atlasPixels );

    std::vector<dz_uint8_t> appendedAtlasBuffer;
    if( __encode_png_rgba( appendedPixels.data(), newWidth, newHeight, &appendedAtlasBuffer ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    dz_int32_t appendedTextureWidth;
    dz_int32_t appendedTextureHeight;
    GLuint appendedTextureId = dz_render_make_texture_from_memory( appendedAtlasBuffer.data(), appendedAtlasBuffer.size(), &appendedTextureWidth, &appendedTextureHeight );

    if( appendedTextureId == 0 )
    {
        return DZ_FAILURE;
    }

    for( er_existing_texture_region_t & existingRegion : existingRegions )
    {
        __set_texture_region_pixels( existingRegion.texture, appendedTextureWidth, appendedTextureHeight, existingRegion.region );
    }

    dz_render_delete_texture( m_textureId );

    m_textureId = appendedTextureId;
    m_textureWidth = appendedTextureWidth;
    m_textureHeight = appendedTextureHeight;
    m_atlasBuffer.swap( appendedAtlasBuffer );

    dz_atlas_set_surface( m_atlas, &m_textureId );

    m_textureRegionSelecting = false;

    if( dz_material_get_texture_slot_count( m_material ) != 0 )
    {
        if( __select_material_texture( m_material, &m_textureIndex, &m_texture ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }
    }

    return this->resetEffect();
}
//////////////////////////////////////////////////////////////////////////
static ImVec2 __screen_to_atlas_pixel( const ImVec2 & _screenPosition, const ImVec2 & _imageMin, dz_float_t _imageWidth, dz_float_t _imageHeight, int _atlasWidth, int _atlasHeight )
{
    ImVec2 point( 0.f, 0.f );

    if( _imageWidth <= 0.f || _imageHeight <= 0.f || _atlasWidth <= 0 || _atlasHeight <= 0 )
    {
        return point;
    }

    point.x = (_screenPosition.x - _imageMin.x) / _imageWidth * (dz_float_t)_atlasWidth;
    point.y = (_screenPosition.y - _imageMin.y) / _imageHeight * (dz_float_t)_atlasHeight;

    point.x = DZ_MAX( 0.f, DZ_MIN( point.x, (dz_float_t)_atlasWidth ) );
    point.y = DZ_MAX( 0.f, DZ_MIN( point.y, (dz_float_t)_atlasHeight ) );

    return point;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t editor::loadAtlasImage( const char * _path )
{
    if( _path == DZ_NULLPTR )
    {
        return DZ_FAILURE;
    }

    dz_int32_t textureWidth;
    dz_int32_t textureHeight;
    GLuint textureId = dz_render_make_texture( _path, &textureWidth, &textureHeight );

    if( textureId == 0 )
    {
        return DZ_FAILURE;
    }

    FILE * f = fopen( _path, "rb" );
    if( f == DZ_NULLPTR )
    {
        dz_render_delete_texture( textureId );

        return DZ_FAILURE;
    }

    fseek( f, 0L, SEEK_END );
    size_t sz = ftell( f );
    rewind( f );

    std::vector<dz_uint8_t> atlasBuffer;
    atlasBuffer.resize( sz );
    fread( atlasBuffer.data(), sz, 1, f );
    fclose( f );

    dz_render_delete_texture( m_textureId );

    m_textureId = textureId;
    m_textureWidth = textureWidth;
    m_textureHeight = textureHeight;
    m_atlasBuffer.swap( atlasBuffer );

    dz_atlas_set_surface( m_atlas, &m_textureId );

    for( dz_uint32_t materialIndex = 0; materialIndex != m_materialCount; ++materialIndex )
    {
        dz_material_t * material = m_materials[materialIndex];
        const dz_uint32_t textureCount = dz_material_get_texture_slot_count( material );

        for( dz_uint32_t textureIndex = 0; textureIndex != textureCount; ++textureIndex )
        {
            const dz_texture_t * texture = DZ_NULLPTR;
            if( dz_material_get_texture( material, textureIndex, &texture ) == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }

            dz_float_t region[4];
            __get_texture_region_pixels( texture, m_textureWidth, m_textureHeight, region );

            if( region[2] == 0.f || region[3] == 0.f )
            {
                region[0] = 0.f;
                region[1] = 0.f;
                region[2] = (dz_float_t)m_textureWidth;
                region[3] = (dz_float_t)m_textureHeight;

                __set_texture_region_pixels( const_cast<dz_texture_t *>(texture), m_textureWidth, m_textureHeight, region );
            }
        }
    }

    m_textureRegionSelecting = false;

    if( dz_material_get_texture_slot_count( m_material ) != 0 )
    {
        if( __select_material_texture( m_material, &m_textureIndex, &m_texture ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }
    }

    return this->resetEffect();
}
//////////////////////////////////////////////////////////////////////////
dz_result_t editor::clearAtlas()
{
    // Drop all texture slots from every material since their UVs reference the atlas image we are removing.
    for( dz_uint32_t materialIndex = 0; materialIndex != m_materialCount; ++materialIndex )
    {
        dz_material_t * material = m_materials[materialIndex];

        const dz_texture_t * popTexture = DZ_NULLPTR;
        while( dz_material_pop_texture( material, &popTexture ) == DZ_SUCCESSFUL )
        {
            dz_texture_destroy( m_service, popTexture );
        }

        dz_material_set_texture_index( material, 0 );
        dz_material_set_texture_count( material, 1 );
        dz_material_set_mode( material, dz_material_get_default_mode() );
    }

    dz_render_delete_texture( m_textureId );

    m_textureId = 0;
    m_textureWidth = 0;
    m_textureHeight = 0;
    m_atlasBuffer.clear();

    dz_atlas_set_surface( m_atlas, &m_textureId );

    m_textureIndex = 0;
    m_texture = DZ_NULLPTR;
    m_textureRegionSelecting = false;

    return this->resetEffect();
}
//////////////////////////////////////////////////////////////////////////
dz_result_t editor::showAtlasData()
{
    ImGui::Spacing();
    ImGui::Text( ER_WINDOW_ATLAS_TITLE );
    ImGui::Separator();

    ImGui::Text( ER_WINDOW_MATERIAL_TEXTURE_SIZE_LABEL );
    ImGui::SameLine();
    ImGui::Text( "%d x %d", m_textureWidth, m_textureHeight );

    if( ImGui::Button( ER_WINDOW_MATERIAL_TEXTURE_BTN_APPEND ) == true )
    {
        nfdchar_t * texturePath = NULL;
        nfdresult_t result = NFD_OpenDialog( NULL, NULL, &texturePath );

        if( result == NFD_OKAY )
        {
            if( m_textureId == 0 || m_atlasBuffer.empty() == true )
            {
                if( this->loadAtlasImage( texturePath ) == DZ_FAILURE )
                {
                    free( texturePath );

                    return DZ_FAILURE;
                }
            }
            else
            {
                if( this->appendTextureToAtlas( texturePath ) == DZ_FAILURE )
                {
                    free( texturePath );

                    return DZ_FAILURE;
                }
            }

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

    if( m_textureId != 0 && m_atlasBuffer.empty() == false )
    {
        ImGui::SameLine();

        if( ImGui::Button( ER_WINDOW_MATERIAL_TEXTURE_BTN_OPTIMIZE_ATLAS ) == true )
        {
            if( this->optimizeAtlas() == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }
        }

        ImGui::SameLine();

        if( ImGui::Button( ER_WINDOW_MATERIAL_TEXTURE_BTN_CLEAR_ATLAS ) == true )
        {
            if( this->clearAtlas() == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }
        }
    }

    if( m_textureId != 0 && m_textureWidth > 0 && m_textureHeight > 0 )
    {
        dz_float_t previewWidth = (dz_float_t)m_textureWidth;
        const dz_float_t availableWidth = ImGui::GetContentRegionAvail().x;

        if( availableWidth > 0.f && previewWidth > availableWidth )
        {
            previewWidth = availableWidth;
        }

        const dz_float_t previewHeight = previewWidth * (dz_float_t)m_textureHeight / (dz_float_t)m_textureWidth;

        ImGui::Image( (void *)(intptr_t)m_textureId, ImVec2( previewWidth, previewHeight ) );
    }

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t editor::showMaterialData()
{
    ImGui::Spacing();
    ImGui::Text( ER_WINDOW_MATERIAL_TITLE );
    ImGui::Separator();

    if( m_materialIndex < m_materialCount )
    {
        __editor_name_input( ER_WINDOW_RESOURCE_NAME_LABEL, m_materialInfos, m_materialCount, m_materialIndex, "Material" );
    }

    if( dz_material_get_atlas( m_material ) != m_atlas )
    {
        dz_material_set_atlas( m_material, m_atlas );
    }

    if( dz_effect_get_atlas( m_effect ) != m_atlas )
    {
        dz_effect_set_atlas( m_effect, m_atlas );
    }

    int blend_current = (int)dz_material_get_blend( m_material );
    if( ImGui::Combo( ER_WINDOW_MATERIAL_COMBO_BLEND_MODE_TEXT, &blend_current, ER_BLEND_MODE_NAMES, IM_ARRAYSIZE( ER_BLEND_MODE_NAMES ) ) == true )
    {
        dz_material_set_blend( m_material, (dz_blend_type_e)blend_current );
    }

    int mode_current = (int)dz_material_get_mode( m_material );
    if( ImGui::Combo( ER_WINDOW_MATERIAL_COMBO_MODE_TEXT, &mode_current, ER_MATERIAL_MODE_NAMES, IM_ARRAYSIZE( ER_MATERIAL_MODE_NAMES ) ) == true )
    {
        dz_material_set_mode( m_material, (dz_material_mode_e)mode_current );

        if( mode_current == DZ_MATERIAL_MODE_TEXTURE )
        {
            dz_material_set_texture_count( m_material, 1 );
        }

        this->resetEffect();
    }

    const dz_material_mode_e materialMode = dz_material_get_mode( m_material );

    bool materialTextureChanged = false;

    if( materialMode == DZ_MATERIAL_MODE_SOLID )
    {
        return DZ_SUCCESSFUL;
    }

    ImGui::Spacing();
    ImGui::Text( ER_WINDOW_MATERIAL_TEXTURE_TITLE );
    ImGui::Separator();

    if( m_textureId == 0 || m_textureWidth <= 0 || m_textureHeight <= 0 )
    {
        return DZ_SUCCESSFUL;
    }

    dz_uint32_t textureCount = dz_material_get_texture_slot_count( m_material );

    if( textureCount == 0 )
    {
        dz_texture_t * texture;
        if( dz_texture_create( m_service, &texture, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        dz_float_t region[4] = {0.f, 0.f, (dz_float_t)m_textureWidth, (dz_float_t)m_textureHeight};
        __set_texture_region_pixels( texture, m_textureWidth, m_textureHeight, region );

        if( dz_material_add_texture( m_material, texture ) == DZ_FAILURE )
        {
            dz_texture_destroy( m_service, texture );

            return DZ_FAILURE;
        }

        dz_material_set_texture_index( m_material, 0 );
        dz_material_set_texture_count( m_material, 1 );

        m_textureIndex = 0;
        m_texture = texture;
        textureCount = 1;
        materialTextureChanged = true;
    }

    dz_uint32_t textureIndex = dz_material_get_texture_index( m_material );
    if( textureIndex >= textureCount )
    {
        textureIndex = textureCount - 1;
        dz_material_set_texture_index( m_material, textureIndex );
        m_textureIndex = (int)textureIndex;
        __select_material_texture( m_material, &m_textureIndex, &m_texture );
    }

    m_textureIndex = (int)textureIndex;
    if( __select_material_texture( m_material, &m_textureIndex, &m_texture ) == DZ_FAILURE )
    {
        return DZ_FAILURE;
    }

    ImGui::Text( "%s %u", ER_WINDOW_MATERIAL_TEXTURE_REGIONS_LABEL, textureCount );

    if( textureCount < ER_ATLAS_TEXTURE_MAX && ImGui::Button( ER_WINDOW_MATERIAL_TEXTURE_BTN_ADD_REGION ) == true )
    {
        dz_float_t textureWeight = 1.f;
        dz_material_get_texture_random_weight( m_material, (dz_uint32_t)m_textureIndex, &textureWeight );

        dz_texture_t * texture;
        if( dz_texture_create( m_service, &texture, DZ_NULLPTR ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        __copy_texture_data( texture, m_texture );

        if( dz_material_add_texture( m_material, texture ) == DZ_FAILURE )
        {
            dz_texture_destroy( m_service, texture );

            return DZ_FAILURE;
        }

        if( dz_material_set_texture_random_weight( m_material, textureCount, textureWeight ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        m_textureIndex = (int)textureCount;
        m_texture = texture;
        dz_material_set_texture_index( m_material, textureCount );
        m_textureRegionSelecting = false;
        materialTextureChanged = true;
        textureCount = dz_material_get_texture_slot_count( m_material );
    }

    if( textureCount > 1 )
    {
        ImGui::SameLine();

        if( ImGui::Button( ER_WINDOW_MATERIAL_TEXTURE_BTN_REMOVE_REGION ) == true )
        {
            typedef struct er_material_texture_data_t
            {
                const dz_texture_t * texture;
                dz_float_t random_weight;
            } er_material_texture_data_t;

            std::vector<er_material_texture_data_t> textures;
            textures.reserve( textureCount - 1 );

            const dz_texture_t * removeTexture = DZ_NULLPTR;

            for( dz_uint32_t index = 0; index != textureCount; ++index )
            {
                const dz_texture_t * texture = DZ_NULLPTR;
                if( dz_material_get_texture( m_material, index, &texture ) == DZ_FAILURE )
                {
                    return DZ_FAILURE;
                }

                dz_float_t randomWeight;
                if( dz_material_get_texture_random_weight( m_material, index, &randomWeight ) == DZ_FAILURE )
                {
                    return DZ_FAILURE;
                }

                if( (int)index == m_textureIndex )
                {
                    removeTexture = texture;
                }
                else
                {
                    er_material_texture_data_t textureData = {texture, randomWeight};
                    textures.push_back( textureData );
                }
            }

            const dz_texture_t * popTexture = DZ_NULLPTR;
            while( dz_material_pop_texture( m_material, &popTexture ) == DZ_SUCCESSFUL )
            {
            }

            if( removeTexture != DZ_NULLPTR )
            {
                dz_texture_destroy( m_service, removeTexture );
            }

            for( const er_material_texture_data_t & textureData : textures )
            {
                if( dz_material_add_texture( m_material, textureData.texture ) == DZ_FAILURE )
                {
                    return DZ_FAILURE;
                }

                const dz_uint32_t index = dz_material_get_texture_slot_count( m_material ) - 1;
                if( dz_material_set_texture_random_weight( m_material, index, textureData.random_weight ) == DZ_FAILURE )
                {
                    return DZ_FAILURE;
                }
            }

            textureCount = dz_material_get_texture_slot_count( m_material );

            if( m_textureIndex >= (int)textureCount )
            {
                m_textureIndex = (int)textureCount - 1;
            }

            dz_material_set_texture_index( m_material, (dz_uint32_t)m_textureIndex );

            if( __select_material_texture( m_material, &m_textureIndex, &m_texture ) == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }

            m_textureRegionSelecting = false;
            materialTextureChanged = true;
        }
    }


    for( dz_uint32_t index = 0; index != textureCount; ++index )
    {
        const dz_texture_t * texture = DZ_NULLPTR;
        if( dz_material_get_texture( m_material, index, &texture ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        char label[32];
        snprintf( label, sizeof( label ), "Region %u", index + 1 );

        ImGui::PushID( (int)index );

        if( ImGui::Selectable( label, (int)index == m_textureIndex, 0, ImVec2( 100.f, 0.f ) ) == true )
        {
            m_textureIndex = (int)index;
            m_texture = const_cast<dz_texture_t *>(texture);
            dz_material_set_texture_index( m_material, index );
            m_textureRegionSelecting = false;
            materialTextureChanged = true;
        }

        ImGui::PopID();
    }

    textureIndex = (dz_uint32_t)m_textureIndex;

    char preview[32];
    snprintf( preview, sizeof( preview ), "Region %u", textureIndex + 1 );

    if( ImGui::BeginCombo( ER_WINDOW_MATERIAL_UV_INDEX_LABEL, preview ) == true )
    {
        for( dz_uint32_t index = 0; index != textureCount; ++index )
        {
            char label[32];
            snprintf( label, sizeof( label ), "Region %u", index + 1 );

            const bool selected = index == textureIndex;
            if( ImGui::Selectable( label, selected ) == true )
            {
                textureIndex = index;
                dz_material_set_texture_index( m_material, textureIndex );
                m_textureIndex = (int)textureIndex;
                __select_material_texture( m_material, &m_textureIndex, &m_texture );
                materialTextureChanged = true;
            }

            if( selected == true )
            {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }

    if( materialMode == DZ_MATERIAL_MODE_TEXTURE )
    {
        if( dz_material_get_texture_count( m_material ) != 1 )
        {
            dz_material_set_texture_count( m_material, 1 );
            materialTextureChanged = true;
        }
    }
    else if( materialMode == DZ_MATERIAL_MODE_SEQUENCE )
    {
        dz_uint32_t uvCount = dz_material_get_texture_count( m_material );
        if( uvCount == 0 )
        {
            uvCount = 1;
        }

        const dz_uint32_t maxCount = textureCount - textureIndex;
        if( uvCount > maxCount )
        {
            uvCount = maxCount;
        }

        int uvCountInput = (int)uvCount;
        if( ImGui::InputInt( ER_WINDOW_MATERIAL_UV_COUNT_LABEL, &uvCountInput, 1, 1, ImGuiInputTextFlags_None ) == true )
        {
            if( uvCountInput < 1 )
            {
                uvCountInput = 1;
            }
            else if( uvCountInput > (int)maxCount )
            {
                uvCountInput = (int)maxCount;
            }

            uvCount = (dz_uint32_t)uvCountInput;
            dz_material_set_texture_count( m_material, uvCount );
            materialTextureChanged = true;
        }

        if( dz_material_get_texture_count( m_material ) != uvCount )
        {
            dz_material_set_texture_count( m_material, uvCount );
            materialTextureChanged = true;
        }
    }

    dz_float_t randomWeight;
    if( dz_material_get_texture_random_weight( m_material, (dz_uint32_t)m_textureIndex, &randomWeight ) == DZ_SUCCESSFUL )
    {
        if( ImGui::InputFloat( "Random Weight", &randomWeight, 0.1f, 1.f, "%.3f", ImGuiInputTextFlags_None ) == true )
        {
            if( randomWeight < 0.f )
            {
                randomWeight = 0.f;
            }

            dz_material_set_texture_random_weight( m_material, (dz_uint32_t)m_textureIndex, randomWeight );
            materialTextureChanged = true;
        }
    }

    dz_float_t region[4];
    __get_texture_region_pixels( m_texture, m_textureWidth, m_textureHeight, region );

    if( region[2] == 0.f || region[3] == 0.f )
    {
        region[0] = 0.f;
        region[1] = 0.f;
        region[2] = (dz_float_t)m_textureWidth;
        region[3] = (dz_float_t)m_textureHeight;

        __set_texture_region_pixels( m_texture, m_textureWidth, m_textureHeight, region );
    }

    bool regionChanged = false;

    regionChanged |= ImGui::InputFloat( ER_WINDOW_MATERIAL_TEXTURE_REGION_X_LABEL, &region[0], 1.f, 10.f, "%.1f", ImGuiInputTextFlags_None );
    regionChanged |= ImGui::InputFloat( ER_WINDOW_MATERIAL_TEXTURE_REGION_Y_LABEL, &region[1], 1.f, 10.f, "%.1f", ImGuiInputTextFlags_None );
    regionChanged |= ImGui::InputFloat( ER_WINDOW_MATERIAL_TEXTURE_REGION_WIDTH_LABEL, &region[2], 1.f, 10.f, "%.1f", ImGuiInputTextFlags_None );
    regionChanged |= ImGui::InputFloat( ER_WINDOW_MATERIAL_TEXTURE_REGION_HEIGHT_LABEL, &region[3], 1.f, 10.f, "%.1f", ImGuiInputTextFlags_None );

    if( regionChanged == true )
    {
        __set_texture_region_pixels( m_texture, m_textureWidth, m_textureHeight, region );
    }

    if( ImGui::Button( ER_WINDOW_MATERIAL_TEXTURE_BTN_RESET_UV ) == true )
    {
        region[0] = 0.f;
        region[1] = 0.f;
        region[2] = (dz_float_t)m_textureWidth;
        region[3] = (dz_float_t)m_textureHeight;

        __set_texture_region_pixels( m_texture, m_textureWidth, m_textureHeight, region );

        m_textureRegionSelecting = false;
    }

    dz_float_t previewWidth = (dz_float_t)m_textureWidth;
    const dz_float_t availableWidth = ImGui::GetContentRegionAvail().x;

    if( availableWidth > 0.f && previewWidth > availableWidth )
    {
        previewWidth = availableWidth;
    }

    const dz_float_t previewHeight = previewWidth * (dz_float_t)m_textureHeight / (dz_float_t)m_textureWidth;

    ImGui::Image( (void *)(intptr_t)m_textureId, ImVec2( previewWidth, previewHeight ) );

    const ImVec2 imageMin = ImGui::GetItemRectMin();
    const ImVec2 imageMax = ImGui::GetItemRectMax();

    const dz_float_t imageWidth = imageMax.x - imageMin.x;
    const dz_float_t imageHeight = imageMax.y - imageMin.y;

    if( ImGui::IsItemHovered() == true && ImGui::IsMouseDoubleClicked( 0 ) == true )
    {
        const ImVec2 click = __screen_to_atlas_pixel( ImGui::GetIO().MousePos, imageMin, imageWidth, imageHeight, m_textureWidth, m_textureHeight );

        dz_int32_t alpha_x;
        dz_int32_t alpha_y;
        dz_int32_t alpha_width;
        dz_int32_t alpha_height;

        if( dz_render_find_alpha_bounds_near_from_memory( m_atlasBuffer.data(), m_atlasBuffer.size(), (dz_int32_t)click.x, (dz_int32_t)click.y, 1, &alpha_x, &alpha_y, &alpha_width, &alpha_height ) == DZ_SUCCESSFUL )
        {
            region[0] = (dz_float_t)alpha_x;
            region[1] = (dz_float_t)alpha_y;
            region[2] = (dz_float_t)alpha_width;
            region[3] = (dz_float_t)alpha_height;

            __set_texture_region_pixels( m_texture, m_textureWidth, m_textureHeight, region );
        }

        m_textureRegionSelecting = false;
    }
    else if( ImGui::IsItemHovered() == true && ImGui::IsMouseClicked( 0 ) == true )
    {
        m_textureRegionSelecting = true;
        m_textureRegionSelectStart = __screen_to_atlas_pixel( ImGui::GetIO().MousePos, imageMin, imageWidth, imageHeight, m_textureWidth, m_textureHeight );
    }

    if( m_textureRegionSelecting == true )
    {
        const ImVec2 current = __screen_to_atlas_pixel( ImGui::GetIO().MousePos, imageMin, imageWidth, imageHeight, m_textureWidth, m_textureHeight );

        region[0] = m_textureRegionSelectStart.x;
        region[1] = m_textureRegionSelectStart.y;
        region[2] = current.x - m_textureRegionSelectStart.x;
        region[3] = current.y - m_textureRegionSelectStart.y;

        m_textureRegionSelecting = ImGui::IsMouseDown( 0 );

        if( region[2] != 0.f || region[3] != 0.f )
        {
            __set_texture_region_pixels( m_texture, m_textureWidth, m_textureHeight, region );
        }
    }

    const dz_float_t scaleX = imageWidth / (dz_float_t)m_textureWidth;
    const dz_float_t scaleY = imageHeight / (dz_float_t)m_textureHeight;

    for( dz_uint32_t index = 0; index != textureCount; ++index )
    {
        const dz_texture_t * texture = DZ_NULLPTR;
        if( dz_material_get_texture( m_material, index, &texture ) == DZ_FAILURE )
        {
            return DZ_FAILURE;
        }

        dz_float_t textureRegion[4];
        __get_texture_region_pixels( texture, m_textureWidth, m_textureHeight, textureRegion );

        const ImVec2 selectionMin( imageMin.x + textureRegion[0] * scaleX, imageMin.y + textureRegion[1] * scaleY );
        const ImVec2 selectionMax( imageMin.x + (textureRegion[0] + textureRegion[2]) * scaleX, imageMin.y + (textureRegion[1] + textureRegion[3]) * scaleY );

        if( (int)index == m_textureIndex )
        {
            ImGui::GetWindowDrawList()->AddRectFilled( selectionMin, selectionMax, IM_COL32( 255, 216, 64, 48 ) );
            ImGui::GetWindowDrawList()->AddRect( selectionMin, selectionMax, IM_COL32( 255, 216, 64, 255 ) );
        }
        else
        {
            ImGui::GetWindowDrawList()->AddRect( selectionMin, selectionMax, IM_COL32( 64, 192, 255, 192 ) );
        }
    }

    dz_float_t u[4];
    dz_float_t v[4];
    dz_texture_get_uv( m_texture, u, v );

    ImGui::Text( "UV: %.4f %.4f - %.4f %.4f", u[0], v[0], u[2], v[2] );

    if( materialTextureChanged == true )
    {
        this->resetEffect();
    }

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
    dz_render_set_proj( &m_openglDesc, -(dz_float_t)m_dzWindowSize.x * 0.5f, (dz_float_t)m_dzWindowSize.x * 0.5f, -(dz_float_t)m_dzWindowSize.y * 0.5f, (dz_float_t)m_dzWindowSize.y * 0.5f );

    // Always use the texture program; chunks without a real surface fall back to a 1x1 white texture in the renderer,
    // which makes the fragment color equal to the vertex color (correct rendering for SOLID materials too).
    dz_render_use_texture_program( &m_openglDesc );

    GLint oldViewport[4];
    GLCALL( glGetIntegerv, (GL_VIEWPORT, oldViewport) );

    GLCALL( glViewport, ((GLint)m_dzWindowPos.x, (GLint)m_dzWindowPos.y, (GLsizei)m_dzWindowSize.x, (GLsizei)m_dzWindowSize.y) );

    dz_render_instance( &m_openglDesc, m_instance );

    GLCALL( glViewport, (oldViewport[0], oldViewport[1], oldViewport[2], oldViewport[3]) );
}
//////////////////////////////////////////////////////////////////////////
bool editor::dumpJSON_( const jpp::object & _json, er_memory_buffer_t * const _out, bool _needCompactDump )
{
    auto my_jpp_dump_callback = []( const char * _buffer, size_t _size, dz_userdata_t _ud )
    {
        er_memory_buffer_t * memoryBuffer = static_cast<er_memory_buffer_t *>(_ud);

        if( _size == 0 )
        {
            return 0;
        }

        const dz_size_t requiredSize = memoryBuffer->size + (dz_size_t)_size;

        if( requiredSize > memoryBuffer->capacity )
        {
            dz_size_t newCapacity = memoryBuffer->capacity != 0 ? memoryBuffer->capacity * 2 : 4096;

            while( newCapacity < requiredSize )
            {
                newCapacity *= 2;
            }

            void * newData = realloc( memoryBuffer->data, newCapacity );

            if( newData == DZ_NULLPTR )
            {
                return -1;
            }

            memoryBuffer->data = static_cast<dz_uint8_t *>(newData);
            memoryBuffer->capacity = newCapacity;
        }

        std::memcpy( memoryBuffer->data + memoryBuffer->size, _buffer, _size );
        memoryBuffer->size = requiredSize;

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
    jd.buffer = static_cast<const dz_uint8_t *>(_buffer);
    jd.carriage = 0;
    jd.capacity = _size;

    auto  my_jpp_error = []( int32_t _line, int32_t _column, int32_t _position, const char * _source, const char * _text, dz_userdata_t _ud )
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

    auto my_jpp_load_callback = []( void * _buffer, size_t _buflen, dz_userdata_t _data )
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

        const dz_uint8_t * jd_buffer = jd->buffer + jd->carriage;
        std::memcpy( _buffer, jd_buffer, _buflen );
        jd->carriage += _buflen;

        return _buflen;
    };

    jpp::object json = jpp::load( my_jpp_load_callback, jpp::JPP_LOAD_MODE_NONE, my_jpp_error, &jd );

    if( json == jpp::detail::invalid )
    {
        return;
    }
    *_out = json;
}
//////////////////////////////////////////////////////////////////////////
static ImVec2 __canvas_world_to_screen( const ImVec2 & _world, const ImVec2 & _canvasMin, const ImVec2 & _canvasSize )
{
    return ImVec2(
        _canvasMin.x + _canvasSize.x * 0.5f + (_world.x + camera_offset_x) * camera_scale,
        _canvasMin.y + _canvasSize.y * 0.5f - (_world.y + camera_offset_y) * camera_scale
    );
}
//////////////////////////////////////////////////////////////////////////
static ImVec2 __canvas_screen_to_world( const ImVec2 & _screen, const ImVec2 & _canvasMin, const ImVec2 & _canvasSize )
{
    return ImVec2(
        (_screen.x - _canvasMin.x - _canvasSize.x * 0.5f) / camera_scale - camera_offset_x,
        -(_screen.y - _canvasMin.y - _canvasSize.y * 0.5f) / camera_scale - camera_offset_y
    );
}
//////////////////////////////////////////////////////////////////////////
static dz_float_t __screen_distance_pow_2( const ImVec2 & _a, const ImVec2 & _b )
{
    const dz_float_t dx = _a.x - _b.x;
    const dz_float_t dy = _a.y - _b.y;

    return dx * dx + dy * dy;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t editor::showContentPane()
{
    // content
    dz_float_t columnWidth = ImGui::GetColumnWidth();
    dz_float_t columnHeight = ImGui::GetWindowHeight() - ImGui::GetFrameHeightWithSpacing() * (ER_CONTENT_CONTROLS_PANE_LINES_COUNT + 1);

    m_dzWindowSize.x = columnWidth;
    m_dzWindowSize.y = columnHeight;

    ImGuiWindow * window = ImGui::GetCurrentWindow();

    ImGuiID id = window->GetID( "DAZZLE_RENDER_CANVAS" );

    if( window->SkipItems == true )
    {
        return DZ_FAILURE;
    }

    ImVec2 cursorPos = window->DC.CursorPos;

    m_dzWindowPos = ImVec2( cursorPos.x, m_windowHeight - cursorPos.y - m_dzWindowSize.y );

    ImGui::BeginChild( "DAZZLE_CANVAS_WINDOW", m_dzWindowSize );

    window->DrawList->AddRectFilled( cursorPos, cursorPos + m_dzWindowSize, ImGui::GetColorU32( ImGuiCol_FrameBg, 1 ) );

    if( m_showCanvasLines == true )
    {
        const dz_float_t leftBound = cursorPos.x;
        const dz_float_t rightBound = cursorPos.x + columnWidth;
        const dz_float_t upperBound = cursorPos.y;
        const dz_float_t downBound = cursorPos.y + columnHeight;

        const ImVec2 originScreen = __canvas_world_to_screen( ImVec2( 0.f, 0.f ), cursorPos, m_dzWindowSize );

        ImVec2 verticalLineStartPos( originScreen.x, cursorPos.y );
        ImVec2 verticalLineEndPos( originScreen.x, cursorPos.y + columnHeight );

        if( verticalLineStartPos.x > leftBound
            && verticalLineStartPos.x < rightBound
            && verticalLineEndPos.x > leftBound
            && verticalLineEndPos.x < rightBound )
        {
            window->DrawList->AddLine( verticalLineStartPos, verticalLineEndPos, ImGui::GetColorU32( ImGuiCol_TextDisabled ) );
        }

        ImVec2 horizontalLineStartPos( cursorPos.x, originScreen.y );
        ImVec2 horizontalLineEndPos( cursorPos.x + columnWidth, originScreen.y );

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

    ImGui::SetCursorScreenPos( cursorPos );
    ImGui::InvisibleButton( "DAZZLE_CANVAS_HITBOX", m_dzWindowSize );

    const bool canvasHovered = ImGui::IsItemHovered();
    const bool spacePressed = glfwGetKey( m_fwWindow, GLFW_KEY_SPACE ) == GLFW_PRESS;
    const ImVec2 mouseWorld = __canvas_screen_to_world( ImGui::GetIO().MousePos, cursorPos, m_dzWindowSize );
    const dz_uint32_t layerCount = dz_effect_get_layer_count( m_effect );

    if( m_showLayerGizmos == false )
    {
        m_layerGizmoDragging = false;
    }

    if( ImGui::IsMouseReleased( 0 ) == true )
    {
        m_layerGizmoDragging = false;
    }

    if( m_showLayerGizmos == true && canvasHovered == true && spacePressed == false && ImGui::IsMouseClicked( 0 ) == true )
    {
        dz_uint32_t hitLayerIndex = DZ_EFFECT_LAYER_NONE;
        dz_float_t bestDistance = ER_LAYER_GIZMO_HIT_RADIUS_POW_2;

        for( dz_uint32_t index = 0; index != layerCount; ++index )
        {
            dz_effect_layer_desc_t layer;
            if( dz_effect_get_layer( m_effect, index, &layer ) == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }

            const ImVec2 layerScreen = __canvas_world_to_screen( ImVec2( layer.x, layer.y ), cursorPos, m_dzWindowSize );
            const dz_float_t distance = __screen_distance_pow_2( ImGui::GetIO().MousePos, layerScreen );

            if( distance <= bestDistance )
            {
                bestDistance = distance;
                hitLayerIndex = index;
            }
        }

        if( hitLayerIndex != DZ_EFFECT_LAYER_NONE )
        {
            dz_effect_layer_desc_t layer;
            if( dz_effect_get_layer( m_effect, hitLayerIndex, &layer ) == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }

            if( hitLayerIndex != m_layerIndex )
            {
                if( this->selectLayer( hitLayerIndex ) == DZ_FAILURE )
                {
                    return DZ_FAILURE;
                }
            }

            m_layerGizmoDragging = true;
            m_layerGizmoDragIndex = hitLayerIndex;
            m_layerGizmoDragOffset = ImVec2( layer.x - mouseWorld.x, layer.y - mouseWorld.y );
        }
    }

    if( m_layerGizmoDragging == true )
    {
        if( ImGui::IsMouseDown( 0 ) == false || spacePressed == true || m_layerGizmoDragIndex >= layerCount || m_showLayerGizmos == false )
        {
            m_layerGizmoDragging = false;
        }
        else
        {
            dz_effect_layer_desc_t layer;
            if( dz_effect_get_layer( m_effect, m_layerGizmoDragIndex, &layer ) == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }

            layer.x = mouseWorld.x + m_layerGizmoDragOffset.x;
            layer.y = mouseWorld.y + m_layerGizmoDragOffset.y;

            if( dz_effect_set_layer( m_effect, m_layerGizmoDragIndex, &layer ) == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }
        }
    }

    ImGui::GetWindowDrawList()->AddCallback( &__draw_callback, this );
    ImGui::GetWindowDrawList()->AddCallback( ImDrawCallback_ResetRenderState, nullptr );

    ImDrawList * drawList = ImGui::GetWindowDrawList();
    drawList->PushClipRect( cursorPos, cursorPos + m_dzWindowSize, true );

    if( m_showEffectCenter == true )
    {
        dz_float_t effectX;
        dz_float_t effectY;
        dz_instance_get_position( m_instance, &effectX, &effectY );

        const ImVec2 effectScreen = __canvas_world_to_screen( ImVec2( effectX, effectY ), cursorPos, m_dzWindowSize );
        const ImU32 effectColor = IM_COL32( 255, 255, 255, 230 );
        const ImU32 effectFillColor = IM_COL32( 255, 255, 255, 42 );

        drawList->AddCircleFilled( effectScreen, 12.f, effectFillColor, 24 );
        drawList->AddCircle( effectScreen, 8.f, effectColor, 24, 2.f );
        drawList->AddLine( ImVec2( effectScreen.x - 16.f, effectScreen.y ), ImVec2( effectScreen.x + 16.f, effectScreen.y ), effectColor, 2.f );
        drawList->AddLine( ImVec2( effectScreen.x, effectScreen.y - 16.f ), ImVec2( effectScreen.x, effectScreen.y + 16.f ), effectColor, 2.f );
    }

    if( m_showLayerGizmos == true )
    {
        for( dz_uint32_t index = 0; index != layerCount; ++index )
        {
            dz_effect_layer_desc_t layer;
            if( dz_effect_get_layer( m_effect, index, &layer ) == DZ_FAILURE )
            {
                return DZ_FAILURE;
            }

            const bool selected = index == m_layerIndex;
            const ImVec2 layerScreen = __canvas_world_to_screen( ImVec2( layer.x, layer.y ), cursorPos, m_dzWindowSize );
            const ImU32 color = selected == true ? IM_COL32( 255, 216, 64, 255 ) : IM_COL32( 64, 192, 255, 220 );
            const ImU32 fillColor = selected == true ? IM_COL32( 255, 216, 64, 72 ) : IM_COL32( 64, 192, 255, 48 );
            const dz_float_t radius = selected == true ? ER_LAYER_GIZMO_SELECTED_RADIUS : ER_LAYER_GIZMO_RADIUS;

            const ImVec2 rotationEnd(
                layerScreen.x + cosf( layer.angle ) * 26.f,
                layerScreen.y + sinf( layer.angle ) * 26.f
            );

            drawList->AddCircleFilled( layerScreen, ER_LAYER_GIZMO_HIT_RADIUS, fillColor, 24 );
            drawList->AddLine( layerScreen, rotationEnd, color, 2.f );
            drawList->AddCircleFilled( rotationEnd, 3.f, color, 12 );
            drawList->AddCircle( layerScreen, radius, color, 16, 2.f );
            drawList->AddLine( ImVec2( layerScreen.x - 10.f, layerScreen.y ), ImVec2( layerScreen.x + 10.f, layerScreen.y ), color, 2.f );
            drawList->AddLine( ImVec2( layerScreen.x, layerScreen.y - 10.f ), ImVec2( layerScreen.x, layerScreen.y + 10.f ), color, 2.f );

            char label[32];
            snprintf( label, sizeof( label ), "L%u", index + 1 );
            drawList->AddText( ImVec2( layerScreen.x + 12.f, layerScreen.y - 16.f ), color, label );
        }
    }

    drawList->PopClipRect();

    if( m_showDebugInfo == true )
    {
        dz_uint32_t particle_count = dz_instance_get_particle_count( m_instance );
        dz_uint32_t particle_limit = dz_instance_get_particle_limit( m_instance );

        char buf[1024];

        snprintf( buf, 1024,
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
dz_result_t editor::showContentPaneControls()
{
    dz_float_t life = dz_effect_get_life( m_effect );
    dz_float_t time = dz_instance_get_time( m_instance );

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

    // time_scale
    char factor_format[100];
    sprintf( factor_format, "%s %%.3f"
        , ER_WINDOW_CONTROLS_FACTOR_PREFIX_TEXT
    );

    ImGui::SliderFloat( "##TimeScale", &m_time_scale, 0.0f, 1.0f, factor_format );


    // time
    char time_format[100];
    sprintf( time_format, "%s %%.3f s / %.3f s"
        , ER_WINDOW_CONTROLS_TIME_PREFIX_TEXT
        , life
    );

    if( ImGui::SliderFloat( "##Time", &time, 0.0f, life, time_format ) == true )
    {
        this->resetEffect();

        dz_instance_update( m_service, m_instance, time );
    }

    // life
    if( ImGui::InputFloat( "##Life", &life, 0.f, 0.f, NULL, ImGuiInputTextFlags_None) == true )
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

    ImGui::Checkbox( ER_WINDOW_CONTROLS_SHOW_LAYER_GIZMOS_TEXT, &m_showLayerGizmos );
    ImGui::SameLine();

    ImGui::Checkbox( ER_WINDOW_CONTROLS_SHOW_EFFECT_CENTER_TEXT, &m_showEffectCenter );
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
        this->destroyEffectResources();

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
//////////////////////////////////////////////////////////////////////////