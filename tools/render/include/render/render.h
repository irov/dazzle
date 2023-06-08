#ifndef DZ_RENDER_H_
#define DZ_RENDER_H_

#include "dazzle/dazzle.h"

#include "glad/gl.h"

//////////////////////////////////////////////////////////////////////////
dz_result_t dz_render_error_check( const char * _file, dz_uint32_t _line );
//////////////////////////////////////////////////////////////////////////
#define GLCALL( Method, Args )\
    do{\
        Method Args;\
        dz_render_error_check(__FILE__, __LINE__);\
    }while(0)
//////////////////////////////////////////////////////////////////////////
#define GLCALLR( R, Method, Args )\
    do{\
        R = Method Args;\
        dz_render_error_check(__FILE__, __LINE__);\
    }while(0)
//////////////////////////////////////////////////////////////////////////
#define IF_GLCALL( Method, Args )\
    Method Args;\
    if( dz_render_error_check(__FILE__, __LINE__) == true )
//////////////////////////////////////////////////////////////////////////
typedef struct gl_vertex_t
{
    dz_float_t x;
    dz_float_t y;
    dz_uint32_t c;
    dz_float_t u;
    dz_float_t v;
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
dz_result_t dz_render_initialize( dz_render_desc_t * _desc, dz_uint16_t _max_vertex_count, dz_uint16_t _max_index_count );
void dz_render_finalize( dz_render_desc_t * _desc );
//////////////////////////////////////////////////////////////////////////
void dz_render_set_proj( const dz_render_desc_t * _desc, dz_float_t _left, dz_float_t _right, dz_float_t _top, dz_float_t _bottom );
//////////////////////////////////////////////////////////////////////////
GLuint dz_render_make_texture( const char * _path, dz_int32_t * const _out_width, dz_int32_t * const _out_height );
GLuint dz_render_make_texture_from_memory( const void * _buffer, dz_size_t _size, dz_int32_t * const _out_width, dz_int32_t * const _out_height );
void dz_render_delete_texture( GLuint _id );
//////////////////////////////////////////////////////////////////////////
void dz_render_use_color_program( dz_render_desc_t * _desc );
void dz_render_use_texture_program( dz_render_desc_t * _desc );
void dz_render_set_camera( const dz_render_desc_t * _desc, dz_float_t _offsetX, dz_float_t _offsetY, dz_float_t _scale );
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_render_instance( const dz_render_desc_t * _desc, const dz_instance_t * _instnace );
//////////////////////////////////////////////////////////////////////////

#endif