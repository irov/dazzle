#include "dazzle/dazzle_aux.h"

//////////////////////////////////////////////////////////////////////////
const char * dz_blend_type_stringize( dz_blend_type_e _type )
{
    switch( _type )
    {
    case DZ_BLEND_NORMAL:
        {
            return "normal";
        }break;
    case DZ_BLEND_ADD:
        {
            return "add";
        }break;
    case DZ_BLEND_MULTIPLY:
        {
            return "multiply";
        }break;
    case DZ_BLEND_SCREEN:
        {
            return "screen";
        }break;
    case __DZ_BLEND_MAX__:
    default:
        break;
    }

    return DZ_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
const char * dz_material_mode_stringize( dz_material_mode_e _mode )
{
    switch( _mode )
    {
    case DZ_MATERIAL_MODE_SOLID:
        {
            return "solid";
        }break;
    case DZ_MATERIAL_MODE_TEXTURE:
        {
            return "texture";
        }break;
    case DZ_MATERIAL_MODE_SEQUENCE:
        {
            return "sequence";
        }break;
    case __DZ_MATERIAL_MODE_MAX__:
    default:
        break;
    }

    return DZ_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
const char * dz_timeline_interpolate_type_stringize( dz_timeline_interpolate_type_e _type )
{
    switch( _type )
    {
    case DZ_TIMELINE_INTERPOLATE_LINEAR:
        {
            return "linear";
        }break;
    case DZ_TIMELINE_INTERPOLATE_BEZIER2:
        {
            return "bezier2";
        }break;
    case __DZ_TIMELINE_INTERPOLATE_MAX__:
    default:
        break;
    }

    return DZ_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
const char * dz_timeline_key_type_stringize( dz_timeline_key_type_e _type )
{
    switch( _type )
    {
    case DZ_TIMELINE_KEY_CONST:
        {
            return "const";
        }break;
    case DZ_TIMELINE_KEY_RANDOMIZE:
        {
            return "randomize";
        }break;
    case __DZ_TIMELINE_KEY_MAX__:
    default:
        break;
    }

    return DZ_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
const char * dz_affector_timeline_type_stringize( dz_affector_timeline_type_e _type )
{
    switch( _type )
    {
    case DZ_AFFECTOR_TIMELINE_LIFE:
        {
            return "life";
        }break;
    case DZ_AFFECTOR_TIMELINE_MOVE_SPEED:
        {
            return "move_speed";
        }break;
    case DZ_AFFECTOR_TIMELINE_MOVE_ACCELERATE:
        {
            return "move_accelerate";
        }break;
    case DZ_AFFECTOR_TIMELINE_ROTATE_SPEED:
        {
            return "rotate_speed";
        }break;
    case DZ_AFFECTOR_TIMELINE_ROTATE_ACCELERATE:
        {
            return "rotate_accelerate";
        }break;
    case DZ_AFFECTOR_TIMELINE_SPIN_SPEED:
        {
            return "spin_speed";
        }break;
    case DZ_AFFECTOR_TIMELINE_SPIN_ACCELERATE:
        {
            return "spin_accelerate";
        }break;
    case DZ_AFFECTOR_TIMELINE_STRAFE_SPEED:
        {
            return "strafe_speed";
        }break;
    case DZ_AFFECTOR_TIMELINE_STRAFE_FRENQUENCE:
        {
            return "strafe_frenquence";
        }break;
    case DZ_AFFECTOR_TIMELINE_STRAFE_SIZE:
        {
            return "strafe_size";
        }break;
    case DZ_AFFECTOR_TIMELINE_STRAFE_SHIFT:
        {
            return "strafe_shift";
        }break;
    case DZ_AFFECTOR_TIMELINE_SCALE:
        {
            return "scale";
        }break;
    case DZ_AFFECTOR_TIMELINE_ASPECT:
        {
            return "aspect";
        }break;
    case DZ_AFFECTOR_TIMELINE_COLOR_R:
        {
            return "color_r";
        }break;
    case DZ_AFFECTOR_TIMELINE_COLOR_G:
        {
            return "color_g";
        }break;
    case DZ_AFFECTOR_TIMELINE_COLOR_B:
        {
            return "color_b";
        }break;
    case DZ_AFFECTOR_TIMELINE_COLOR_A:
        {
            return "color_a";
        }break;
    case __DZ_AFFECTOR_TIMELINE_MAX__:
    default:
        break;
    }

    return DZ_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
const char * dz_shape_type_stringize( dz_shape_type_e _type )
{
    switch( _type )
    {
    case DZ_SHAPE_POINT:
        {
            return "point";
        }break;
    case DZ_SHAPE_SEGMENT:
        {
            return "segment";
        }break;
    case DZ_SHAPE_CIRCLE:
        {
            return "circle";
        }break;
    case DZ_SHAPE_LINE:
        {
            return "line";
        }break;
    case DZ_SHAPE_RECT:
        {
            return "rect";
        }break;
    case DZ_SHAPE_POLYGON:
        {
            return "polygon";
        }break;
    case DZ_SHAPE_MASK:
        {
            return "mask";
        }break;
    case __DZ_SHAPE_MAX__:
    default:
        break;
    }

    return DZ_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
const char * dz_shape_timeline_type_stringize( dz_shape_timeline_type_e _type )
{
    switch( _type )
    {
    case DZ_SHAPE_SEGMENT_ANGLE_MIN:
        {
            return "segment_angle_min";
        }break;
    case DZ_SHAPE_SEGMENT_ANGLE_MAX:
        {
            return "segment_angle_max";
        }break;
    case DZ_SHAPE_CIRCLE_RADIUS_MIN:
        {
            return "circle_radius_min";
        }break;
    case DZ_SHAPE_CIRCLE_RADIUS_MAX:
        {
            return "circle_radius_max";
        }break;
    case DZ_SHAPE_CIRCLE_ANGLE_MIN:
        {
            return "circle_angle_min";
        }break;
    case DZ_SHAPE_CIRCLE_ANGLE_MAX:
        {
            return "circle_angle_max";
        }break;
    case DZ_SHAPE_LINE_ANGLE:
        {
            return "line_angle";
        }break;
    case DZ_SHAPE_LINE_SIZE:
        {
            return "line_size";
        }break;
    case DZ_SHAPE_LINE_OFFSET:
        {
            return "line_offset";
        }break;
    case DZ_SHAPE_RECT_WIDTH_MIN:
        {
            return "rect_width_min";
        }break;
    case DZ_SHAPE_RECT_WIDTH_MAX:
        {
            return "rect_width_max";
        }break;
    case DZ_SHAPE_RECT_HEIGHT_MIN:
        {
            return "rect_height_min";
        }break;
    case DZ_SHAPE_RECT_HEIGHT_MAX:
        {
            return "rect_height_max";
        }break;
    case __DZ_SHAPE_TIMELINE_MAX__:
    default:
        break;
    }

    return DZ_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
const char * dz_emitter_timeline_type_stringize( dz_emitter_timeline_type_e _type )
{
    switch( _type )
    {
    case DZ_EMITTER_SPAWN_DELAY:
        {
            return "delay";
        }break;
    case DZ_EMITTER_SPAWN_COUNT:
        {
            return "count";
        }break;
    case DZ_EMITTER_SPAWN_SPIN_MIN:
        {
            return "spin_min";
        }break;
    case DZ_EMITTER_SPAWN_SPIN_MAX:
        {
            return "spin_max";
        }break;
    case __DZ_EMITTER_TIMELINE_MAX__:
    default:
        break;
    }

    return DZ_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////