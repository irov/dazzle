#pragma once

#include "dazzle/dazzle.hpp"

#include "glad/glad.h"

//////////////////////////////////////////////////////////////////////////
typedef struct gl_vertex_t
{
    float x;
    float y;
    dz_uint32_t c;
    float u;
    float v;
} gl_vertex_t;
//////////////////////////////////////////////////////////////////////////
typedef dz_uint16_t gl_index_t;
//////////////////////////////////////////////////////////////////////////
typedef struct dz_render_handle_t
{
    GLuint VAO;
    GLuint VBO;
    GLuint IBO;

    GLuint shaderCurrentProgram;
    GLuint shaderColorProgram;
    GLuint shaderTextureProgram;
}dz_render_handle_t;
//////////////////////////////////////////////////////////////////////////
bool dz_render_initialize( dz_render_handle_t ** _handle, float _width, float _height, int _max_vertex_count, int _max_index_count );
void dz_render_finalize( dz_render_handle_t * _handle );
//////////////////////////////////////////////////////////////////////////
GLuint dz_render_make_texture( const char * _path );
void dz_render_delete_texture( GLuint _id );
//////////////////////////////////////////////////////////////////////////
void dz_render_use_color_program( dz_render_handle_t * _handle );
void dz_render_use_texture_program( dz_render_handle_t * _handle );
void dz_render_set_camera( dz_render_handle_t * _handle, float _offsetX, float _offsetY, float _scale );
//////////////////////////////////////////////////////////////////////////