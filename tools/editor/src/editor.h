#pragma once

#include "dazzle/dazzle.hpp"
#include "render/render.hpp"
#include "evict/evict.hpp"

#include "glad/gl.h"
#include "GLFW/glfw3.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <vector>

//////////////////////////////////////////////////////////////////////////
// prefix ER_ means editor
//////////////////////////////////////////////////////////////////////////
static constexpr dz_uint32_t ER_CURVE_MAX_POINTS = 100;
static constexpr dz_uint32_t ER_EDITOR_RESOURCE_MAX = 128;
static constexpr dz_uint32_t ER_EDITOR_NAME_MAX = 64;
static constexpr dz_float_t ER_CURVE_BOX_HEIGHT_TO_WIDTH_RATIO = 0.4f;
//////////////////////////////////////////////////////////////////////////
typedef enum er_curve_point_mode_e
{
    ER_CURVE_POINT_MODE_NORMAL,
    ER_CURVE_POINT_MODE_RANDOM,

    __ER_CURVE_POINT_MODE_MAX__
} er_curve_point_mode_e;
//////////////////////////////////////////////////////////////////////////
typedef struct er_curve_point_t
{
    dz_float_t x = 0.f;
    dz_float_t y = 0.f;

    dz_float_t y2 = 0.f;

    er_curve_point_mode_e mode = ER_CURVE_POINT_MODE_NORMAL;

} er_curve_point_t;
//////////////////////////////////////////////////////////////////////////
typedef er_curve_point_t PointsArray[ER_CURVE_MAX_POINTS];
//////////////////////////////////////////////////////////////////////////
typedef struct er_memory_buffer_t
{
    dz_uint8_t * data;
    dz_size_t size;
    dz_size_t capacity;
} er_memory_buffer_t;
//////////////////////////////////////////////////////////////////////////
typedef struct er_editor_instance_info_t
{
    dz_uint32_t id;
    char name[ER_EDITOR_NAME_MAX];
} er_editor_instance_info_t;
//////////////////////////////////////////////////////////////////////////
class editor
{
public:
    editor();
    ~editor();

protected:
    dz_result_t init();
    void finalize();

    dz_result_t update( double _time );
    dz_result_t render();

public:
    dz_result_t run( int argc, char ** argv );

public:
    const ImVec2 & getDzWindowPos() const;
    const ImVec2 & getDzWindowSize() const;

protected:
    dz_result_t resetEffect();
    dz_result_t createDefaultMaterial( dz_material_t ** const _material );
    dz_result_t createDefaultShape( dz_shape_t ** const _shape );
    dz_result_t createDefaultEmitter( dz_emitter_t ** const _emitter );
    dz_result_t createDefaultAffector( dz_affector_t ** const _affector );
    dz_result_t selectLayer( dz_uint32_t _index );
    dz_result_t ensureLayerTrigger( dz_uint32_t _layerIndex, dz_uint32_t * const _triggerIndex );
    dz_result_t selectMaterialResource( dz_uint32_t _index );
    dz_result_t selectShapeResource( dz_uint32_t _index );
    dz_result_t selectEmitterResource( dz_uint32_t _index );
    dz_result_t selectAffectorResource( dz_uint32_t _index );
    void rebuildResourceLists();
    void syncSelectedResourceIndices();
    void destroyEffectResources();
    void setEffectAtlasesSurface();

    dz_result_t saveEffect();
    dz_result_t loadEffect();

    dz_result_t exportEffect();

    dz_result_t readTimelineKey( const dz_timeline_key_t * _key, er_curve_point_t * _pointsData, size_t _index );

    dz_result_t resetEffectData();

    dz_result_t showMenuBar();

    dz_result_t showEffectData();
    dz_result_t showResourceList( int _selected );
    dz_result_t showComposerData();
    dz_result_t showShapeData();
    dz_result_t showAffectorData();
    dz_result_t showEmitterData();
    dz_result_t showAtlasData();
    dz_result_t showMaterialData();

    dz_result_t optimizeAtlas();
    dz_result_t appendTextureToAtlas( const char * _path );
    dz_result_t loadAtlasImage( const char * _path );
    dz_result_t clearAtlas();

    dz_result_t showContentPane();
    dz_result_t showContentPaneControls();

public:
    void showDazzleCanvas();

protected:
    bool dumpJSON_( const jpp::object & _json, er_memory_buffer_t * const _out, bool _needCompactDump );
    void loadJSON_( const void * _buffer, size_t _size, jpp::object * _out ) const;

public:
    dz_uint32_t m_windowWidth;
    dz_uint32_t m_windowHeight;

    ImVec2 m_dzWindowPos;
    ImVec2 m_dzWindowSize;

    ImVec4 m_backgroundColor;

    bool m_showDebugInfo;
    bool m_showCanvasLines;
    bool m_showLayerGizmos;
    bool m_showEffectCenter;

    bool m_pause;
    int m_windowType;

    dz_service_t * m_service;

    dz_atlas_t * m_atlas;
    dz_texture_t * m_texture;
    int m_textureIndex;
    dz_material_t * m_material;
    dz_shape_t * m_shape;
    dz_emitter_t * m_emitter;
    dz_affector_t * m_affector;

    dz_material_t * m_materials[ER_EDITOR_RESOURCE_MAX];
    dz_shape_t * m_shapes[ER_EDITOR_RESOURCE_MAX];
    dz_emitter_t * m_emitters[ER_EDITOR_RESOURCE_MAX];
    dz_affector_t * m_affectors[ER_EDITOR_RESOURCE_MAX];

    er_editor_instance_info_t m_materialInfos[ER_EDITOR_RESOURCE_MAX];
    er_editor_instance_info_t m_shapeInfos[ER_EDITOR_RESOURCE_MAX];
    er_editor_instance_info_t m_emitterInfos[ER_EDITOR_RESOURCE_MAX];
    er_editor_instance_info_t m_affectorInfos[ER_EDITOR_RESOURCE_MAX];
    er_editor_instance_info_t m_layerInfos[DZ_EFFECT_LAYER_MAX];
    er_editor_instance_info_t m_triggerInfos[DZ_EFFECT_TRIGGER_MAX];

    dz_uint32_t m_materialCount;
    dz_uint32_t m_shapeCount;
    dz_uint32_t m_emitterCount;
    dz_uint32_t m_affectorCount;

    dz_uint32_t m_materialIndex;
    dz_uint32_t m_shapeIndex;
    dz_uint32_t m_emitterIndex;
    dz_uint32_t m_affectorIndex;

    dz_effect_t * m_effect;
    dz_uint32_t m_layerIndex;
    dz_uint32_t m_triggerIndex;
    dz_uint32_t m_nextEditorInstanceId;
    bool m_layerGizmoDragging;
    dz_uint32_t m_layerGizmoDragIndex;
    ImVec2 m_layerGizmoDragOffset;

    dz_instance_t * m_instance;

    dz_bool_t m_loop;
    dz_float_t m_time_scale;

    int m_textureWidth;
    int m_textureHeight;

    bool m_textureRegionSelecting;
    ImVec2 m_textureRegionSelectStart;

    std::vector<dz_uint8_t> m_atlasBuffer;

    GLuint m_textureId;

    dz_render_desc_t m_openglDesc;
    GLFWwindow * m_fwWindow;

    typedef struct
    {
        dz_shape_timeline_type_e type;
        const char * name;

        dz_int32_t zoom;

        int selectedPoint;

        PointsArray pointsData;
        PointsArray pointsCurve;
    } timeline_shape_t;

    timeline_shape_t m_timelineShapeData[__DZ_SHAPE_TIMELINE_MAX__];

    typedef struct
    {
        dz_affector_timeline_type_e type;
        const char * name;

        dz_int32_t zoom;

        int selectedPoint;

        PointsArray pointsData;
        PointsArray pointsCurve;
    } timeline_affector_t;

    timeline_affector_t m_timelineAffectorData[__DZ_AFFECTOR_TIMELINE_MAX__];

    typedef struct
    {
        dz_emitter_timeline_type_e type;
        const char * name;

        dz_int32_t zoom;

        int selectedPoint;

        PointsArray pointsData;
        PointsArray pointsCurve;
    } timeline_emitter_t;

    timeline_emitter_t m_timelineEmitterData[__DZ_EMITTER_TIMELINE_MAX__];

    dz_shape_type_e m_shapeType;
};