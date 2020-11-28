#ifndef DZ_RENDER_H_
#define DZ_RENDER_H_

#include "dazzle/dazzle.h"

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
typedef struct dz_render_desc_t
{
    GLuint VAO;
    GLuint VBO;
    GLuint IBO;

    GLuint shaderCurrentProgram;
    GLuint shaderColorProgram;
    GLuint shaderTextureProgram;
}dz_render_desc_t;
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_render_initialize( dz_render_desc_t * _desc, int32_t _max_vertex_count, int32_t _max_index_count );
void dz_render_finalize( dz_render_desc_t * _desc );
//////////////////////////////////////////////////////////////////////////
void dz_render_set_proj( const dz_render_desc_t * _desc, float _left, float _right, float _top, float _bottom );
//////////////////////////////////////////////////////////////////////////
GLuint dz_render_make_texture( const char * _path, int * _out_width, int * _out_height );
GLuint dz_render_make_texture_from_memory( const void * _buffer, size_t _size, int * _out_width, int * _out_height );
void dz_render_delete_texture( GLuint _id );
//////////////////////////////////////////////////////////////////////////
void dz_render_use_color_program( dz_render_desc_t * _desc );
void dz_render_use_texture_program( dz_render_desc_t * _desc );
void dz_render_set_camera( const dz_render_desc_t * _desc, float _offsetX, float _offsetY, float _scale );
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_render_instance( const dz_render_desc_t * _desc, const dz_instance_t * _instnace );
//////////////////////////////////////////////////////////////////////////

#endif