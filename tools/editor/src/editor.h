#pragma once

#include "dazzle/dazzle.hpp"
#include "render/render.hpp"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

//////////////////////////////////////////////////////////////////////////
// prefix ER_ means editor
//////////////////////////////////////////////////////////////////////////
static constexpr uint32_t ER_CURVE_MAX_POINTS = 100;
static constexpr float ER_CURVE_BOX_HEIGHT_TO_WIDTH_RATIO = 0.4f;
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
    float x = 0.f;
    float y = 0.f;

    float y2 = 0.f;

    er_curve_point_mode_e mode = ER_CURVE_POINT_MODE_NORMAL;

} er_curve_point_t;
//////////////////////////////////////////////////////////////////////////
typedef er_curve_point_t PointsArray[ER_CURVE_MAX_POINTS];
//////////////////////////////////////////////////////////////////////////
class editor
{
public:
    editor();
    ~editor();

protected:
    int init();
    void finalize();

    int update();
    int render();

public:
    int run();

public:
    const ImVec2 & getDzWindowPos() const;
    const ImVec2 & getDzWindowSize() const;

protected:
    int resetEffect();

    int showMenuBar();

    int showEffectData();
    int showShapeData();
    int showAffectorData();
    int showEmitterData();
    int showMaterialData();

    int showContentPane();
    int showContentPaneControls();

public:
    void showDazzleCanvas();

public:
    float m_windowWidth;
    float m_windowHeight;

    ImVec2 m_dzWindowPos;
    ImVec2 m_dzWindowSize;

    ImVec4 m_backgroundColor;

    bool m_showDebugInfo;
    bool m_showCanvasLines;

    bool m_pause;

    dz_service_t * m_service;

    dz_atlas_t * m_atlas;
    dz_texture_t * m_texture;
    dz_material_t * m_material;
    dz_shape_t * m_shape;
    dz_emitter_t * m_emitter;
    dz_affector_t * m_affector;

    dz_effect_t * m_effect;

    dz_bool_t m_loop;

    int m_textureWidth;
    int m_textureHeight;

    GLuint m_textureId;

    dz_render_desc_t m_openglDesc;
    GLFWwindow * m_fwWindow;

    typedef struct
    {
        dz_shape_timeline_type_e type;
        const char * name;

        int32_t zoom;

        int selectedPoint;

        PointsArray pointsData;
        PointsArray pointsCurve;
    } timeline_shape_t;

    timeline_shape_t m_timelineShapeData[__DZ_SHAPE_TIMELINE_MAX__];

    typedef struct
    {
        dz_affector_timeline_type_e type;
        const char * name;

        int32_t zoom;

        int selectedPoint;

        PointsArray pointsData;
        PointsArray pointsCurve;
    } timeline_affector_t;

    timeline_affector_t m_timelineAffectorData[__DZ_AFFECTOR_TIMELINE_MAX__];

    typedef struct
    {
        dz_emitter_timeline_type_e type;
        const char * name;

        int32_t zoom;

        int selectedPoint;

        PointsArray pointsData;
        PointsArray pointsCurve;
    } timeline_emitter_t;

    timeline_emitter_t m_timelineEmitterData[__DZ_EMITTER_TIMELINE_MAX__];

    dz_shape_type_e m_shapeType;
};