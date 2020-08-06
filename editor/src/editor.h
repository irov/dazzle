#pragma once

#include "dazzle/dazzle.hpp"

#include "opengl.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

//////////////////////////////////////////////////////////////////////////
static constexpr uint32_t MAX_POINTS = 100;
static constexpr float HEIGHT_TO_WIDTH_RATIO = 0.4f;
//////////////////////////////////////////////////////////////////////////
typedef ImVec2 PointsArray[MAX_POINTS];
//////////////////////////////////////////////////////////////////////////
class editor
{
public:
    editor();
    virtual ~editor();

protected:
    int init();
    void finalize();

    int update();
    int render();

public:
    int run();

protected:
    int resetEmitter();

protected:
    float m_windowWidth;
    float m_windowHeight;

    dz_service_t * m_service;

    dz_shape_data_t * m_shapeData;
    dz_emitter_data_t * m_emitterData;
    dz_affector_data_t * m_affectorData;

    dz_emitter_t * m_emitter;

    example_opengl_handle_t * m_openglHandle;
    GLFWwindow * m_fwWindow;

    typedef struct
    {
        dz_shape_data_timeline_type_e type;
        const char * name;

        float value0;

        float maxValue;

        PointsArray param;
    } timeline_shape_data_t;

    timeline_shape_data_t m_timelineShapeData[__DZ_SHAPE_DATA_TIMELINE_MAX__];

    typedef struct
    {
        dz_affector_data_timeline_type_e type;
        const char * name;

        float time0;
        float time1;
        float value0;
        float value1;
        float value2;

        float maxValue;

        PointsArray param;
    } timeline_affector_data_t;

    timeline_affector_data_t m_timelineAffectorData[__DZ_AFFECTOR_DATA_TIMELINE_MAX__];

    typedef struct
    {
        dz_emitter_data_timeline_type_e type;
        const char * name;

        float value0;

        float maxValue;

        PointsArray param;
    } timeline_emitter_data_t;

    timeline_emitter_data_t m_timelineEmitterData[__DZ_EMITTER_DATA_TIMELINE_MAX__];

    dz_shape_data_type_e m_shapeType;
};