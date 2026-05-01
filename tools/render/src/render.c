#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_NO_SIMD
#include "stb_image/stb_image.h"

#include "render/render.h"

#include <stdlib.h>

//////////////////////////////////////////////////////////////////////////
static const char * __gl_get_error_string( GLenum _err )
{
    switch( _err )
    {
    case GL_INVALID_ENUM:
        return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE:
        return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION:
        return "GL_INVALID_OPERATION";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        return "GL_INVALID_FRAMEBUFFER_OPERATION";
    case GL_OUT_OF_MEMORY:
        return "GL_OUT_OF_MEMORY";
#if defined(MENGINE_PLATFORM_WINDOWS)
    case GL_STACK_UNDERFLOW:
        return "GL_STACK_UNDERFLOW";
    case GL_STACK_OVERFLOW:
        return "GL_STACK_OVERFLOW";
#endif
    default:
        {
        }
    }

    return "GL_UNKNOWN";
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_render_error_check( const char * _file, dz_uint32_t _line )
{
    GLenum err = glGetError();

    if( err == GL_NO_ERROR )
    {
        return DZ_FAILURE;
    }

    const char * err_str = __gl_get_error_string( err );

    printf( "%s:[%d] error %s:%d\n"
        , _file
        , _line
        , err_str
        , err
    );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static void __make_ortho( dz_float_t _l, dz_float_t _r, dz_float_t _t, dz_float_t _b, dz_float_t _n, dz_float_t _f, dz_float_t _m[16] )
{
    _m[0] = 2.f / (_r - _l);
    _m[5] = 2.f / (_t - _b);
    _m[10] = -2.f / (_f - _n);
    _m[12] = -(_r + _l) / (_r - _l);
    _m[13] = -(_t + _b) / (_t - _b);
    _m[14] = -(_f + _n) / (_f - _n);
    _m[15] = 1.f;

    _m[1] = _m[2] = _m[3] = _m[4] = _m[6] = _m[7] = _m[8] = _m[9] = _m[11] = 0.f;
}
//////////////////////////////////////////////////////////////////////////
static GLuint __make_program( const char * _vertexShaderSource, const char * _fragmentShaderSource )
{
    GLint vertexShaderColor = glCreateShader( GL_VERTEX_SHADER );
    GLCALL( glShaderSource, (vertexShaderColor, 1, &_vertexShaderSource, NULL) );
    GLCALL( glCompileShader, (vertexShaderColor) );

    GLint vertexShader_success;
    GLCALL( glGetShaderiv, (vertexShaderColor, GL_COMPILE_STATUS, &vertexShader_success) );

    if( vertexShader_success == 0 )
    {
        char infoLog[512];
        GLCALL( glGetShaderInfoLog, (vertexShaderColor, 512, NULL, infoLog) );

        printf( "error: %s\n"
            , infoLog
        );

        return EXIT_FAILURE;
    }

    GLuint fragmentShaderColor = glCreateShader( GL_FRAGMENT_SHADER );
    GLCALL( glShaderSource, (fragmentShaderColor, 1, &_fragmentShaderSource, NULL) );
    GLCALL( glCompileShader, (fragmentShaderColor) );

    GLint fragmentShader_success;
    GLCALL( glGetShaderiv, (fragmentShaderColor, GL_COMPILE_STATUS, &fragmentShader_success) );
    if( fragmentShader_success == 0 )
    {
        char infoLog[512];
        GLCALL( glGetShaderInfoLog, (fragmentShaderColor, 512, NULL, infoLog) );

        printf( "error: %s\n"
            , infoLog
        );

        return EXIT_FAILURE;
    }

    GLuint shaderProgram;
    GLCALLR( shaderProgram, glCreateProgram, () );
    GLCALL( glAttachShader, (shaderProgram, vertexShaderColor) );
    GLCALL( glAttachShader, (shaderProgram, fragmentShaderColor) );
    GLCALL( glLinkProgram, (shaderProgram) );

    GLint shaderProgram_success;
    GLCALL( glGetProgramiv, (shaderProgram, GL_LINK_STATUS, &shaderProgram_success) );

    if( shaderProgram_success == 0 )
    {
        char infoLog[512];
        GLCALL( glGetProgramInfoLog, (shaderProgram, 512, NULL, infoLog) );

        printf( "error: %s\n"
            , infoLog
        );

        return EXIT_FAILURE;
    }

    GLCALL( glDeleteShader, (vertexShaderColor) );
    GLCALL( glDeleteShader, (fragmentShaderColor) );

    return shaderProgram;
}
//////////////////////////////////////////////////////////////////////////
GLuint dz_render_make_texture( const char * _path, dz_int32_t * const _out_width, dz_int32_t * const _out_height )
{
    dz_int32_t width;
    dz_int32_t height;
    dz_int32_t comp;

    dz_uint8_t * data = stbi_load( _path, &width, &height, &comp, STBI_default );

    if( data == DZ_NULLPTR )
    {
        return 0;
    }

    GLint internal_format;
    GLenum format;
    switch( comp )
    {
    case 1:
        {
            internal_format = GL_R8;
            format = GL_RED;
        }break;
    case 2:
        {
            internal_format = GL_RG8;
            format = GL_RG;
        }break;
    case 3:
        {
            internal_format = GL_RGB8;
            format = GL_RGB;
        }break;
    case 4:
        {
            internal_format = GL_RGBA8;
            format = GL_RGBA;
        }break;
    default:
        return 0;
    }

    GLuint id;
    GLCALL( glGenTextures, (1, &id) );
    GLCALL( glBindTexture, (GL_TEXTURE_2D, id) );
    GLCALL( glTexImage2D, (GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, data) );

    GLCALL( glTexParameteri, (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR) );
    GLCALL( glTexParameteri, (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR) );
    GLCALL( glTexParameteri, (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT) );
    GLCALL( glTexParameteri, (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT) );

    GLCALL( glBindTexture, (GL_TEXTURE_2D, 0) );

    stbi_image_free( data );

    *_out_width = width;
    *_out_height = height;

    return id;
}
//////////////////////////////////////////////////////////////////////////
GLuint dz_render_make_texture_from_memory( const void * _buffer, dz_size_t _size, dz_int32_t * const _out_width, dz_int32_t * const _out_height )
{
    dz_int32_t width;
    dz_int32_t height;
    dz_int32_t comp;

    dz_uint8_t * data = stbi_load_from_memory( _buffer, _size, &width, &height, &comp, STBI_default );

    if( data == DZ_NULLPTR )
    {
        return 0;
    }

    GLint internal_format;
    GLenum format;
    switch( comp )
    {
    case 1:
        {
            internal_format = GL_R8;
            format = GL_RED;
        }break;
    case 2:
        {
            internal_format = GL_RG8;
            format = GL_RG;
        }break;
    case 3:
        {
            internal_format = GL_RGB8;
            format = GL_RGB;
        }break;
    case 4:
        {
            internal_format = GL_RGBA8;
            format = GL_RGBA;
        }break;
    default:
        return 0;
    }

    GLuint id;
    GLCALL( glGenTextures, (1, &id) );
    GLCALL( glBindTexture, (GL_TEXTURE_2D, id) );
    GLCALL( glTexImage2D, (GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, data) );

    GLCALL( glTexParameteri, (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR) );
    GLCALL( glTexParameteri, (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR) );
    GLCALL( glTexParameteri, (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT) );
    GLCALL( glTexParameteri, (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT) );

    GLCALL( glBindTexture, (GL_TEXTURE_2D, 0) );

    stbi_image_free( data );

    *_out_width = width;
    *_out_height = height;

    return id;
}
//////////////////////////////////////////////////////////////////////////
static dz_bool_t __is_alpha_pixel( const dz_uint8_t * _data, dz_int32_t _width, dz_int32_t _comp, dz_int32_t _x, dz_int32_t _y )
{
    dz_int32_t alpha_offset;

    switch( _comp )
    {
    case 2:
        alpha_offset = 1;
        break;
    case 4:
        alpha_offset = 3;
        break;
    default:
        return DZ_FALSE;
    }

    const dz_size_t index = (((dz_size_t)_y * (dz_size_t)_width + (dz_size_t)_x) * (dz_size_t)_comp) + (dz_size_t)alpha_offset;

    return _data[index] != 0 ? DZ_TRUE : DZ_FALSE;
}
//////////////////////////////////////////////////////////////////////////
static dz_result_t __find_nearest_alpha_pixel( const dz_uint8_t * _data, dz_int32_t _width, dz_int32_t _height, dz_int32_t _comp, dz_int32_t _x, dz_int32_t _y, dz_int32_t * const _out_x, dz_int32_t * const _out_y )
{
    if( __is_alpha_pixel( _data, _width, _comp, _x, _y ) == DZ_TRUE )
    {
        *_out_x = _x;
        *_out_y = _y;

        return DZ_SUCCESSFUL;
    }

    const dz_int32_t max_radius = DZ_MAX( _width, _height );

    for( dz_int32_t radius = 1; radius != max_radius; ++radius )
    {
        const dz_int32_t x0 = DZ_MAX( _x - radius, 0 );
        const dz_int32_t y0 = DZ_MAX( _y - radius, 0 );
        const dz_int32_t x1 = DZ_MIN( _x + radius, _width - 1 );
        const dz_int32_t y1 = DZ_MIN( _y + radius, _height - 1 );

        for( dz_int32_t x = x0; x <= x1; ++x )
        {
            if( __is_alpha_pixel( _data, _width, _comp, x, y0 ) == DZ_TRUE )
            {
                *_out_x = x;
                *_out_y = y0;

                return DZ_SUCCESSFUL;
            }

            if( y1 != y0 && __is_alpha_pixel( _data, _width, _comp, x, y1 ) == DZ_TRUE )
            {
                *_out_x = x;
                *_out_y = y1;

                return DZ_SUCCESSFUL;
            }
        }

        for( dz_int32_t y = y0 + 1; y < y1; ++y )
        {
            if( __is_alpha_pixel( _data, _width, _comp, x0, y ) == DZ_TRUE )
            {
                *_out_x = x0;
                *_out_y = y;

                return DZ_SUCCESSFUL;
            }

            if( x1 != x0 && __is_alpha_pixel( _data, _width, _comp, x1, y ) == DZ_TRUE )
            {
                *_out_x = x1;
                *_out_y = y;

                return DZ_SUCCESSFUL;
            }
        }
    }

    return DZ_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_render_find_alpha_bounds_near_from_memory( const void * _buffer, dz_size_t _size, dz_int32_t _x, dz_int32_t _y, dz_int32_t _border, dz_int32_t * const _out_x, dz_int32_t * const _out_y, dz_int32_t * const _out_width, dz_int32_t * const _out_height )
{
    if( _buffer == DZ_NULLPTR || _size == 0 )
    {
        return DZ_FAILURE;
    }

    dz_int32_t width;
    dz_int32_t height;
    dz_int32_t comp;

    dz_uint8_t * data = stbi_load_from_memory( _buffer, (int)_size, &width, &height, &comp, STBI_default );

    if( data == DZ_NULLPTR )
    {
        return DZ_FAILURE;
    }

    if( comp != 2 && comp != 4 )
    {
        stbi_image_free( data );

        return DZ_FAILURE;
    }

    _x = DZ_MAX( 0, DZ_MIN( _x, width - 1 ) );
    _y = DZ_MAX( 0, DZ_MIN( _y, height - 1 ) );

    dz_int32_t seed_x;
    dz_int32_t seed_y;

    if( __find_nearest_alpha_pixel( data, width, height, comp, _x, _y, &seed_x, &seed_y ) == DZ_FAILURE )
    {
        stbi_image_free( data );

        return DZ_FAILURE;
    }

    const dz_size_t pixel_count = (dz_size_t)width * (dz_size_t)height;

    dz_uint8_t * visited = (dz_uint8_t *)calloc( pixel_count, sizeof( dz_uint8_t ) );

    if( visited == DZ_NULLPTR )
    {
        stbi_image_free( data );

        return DZ_FAILURE;
    }

    dz_size_t * stack = (dz_size_t *)malloc( pixel_count * sizeof( dz_size_t ) );

    if( stack == DZ_NULLPTR )
    {
        free( visited );
        stbi_image_free( data );

        return DZ_FAILURE;
    }

    dz_int32_t min_x = width;
    dz_int32_t min_y = height;
    dz_int32_t max_x = -1;
    dz_int32_t max_y = -1;

    dz_size_t stack_count = 0;

    const dz_size_t seed_index = (dz_size_t)seed_y * (dz_size_t)width + (dz_size_t)seed_x;
    stack[stack_count++] = seed_index;
    visited[seed_index] = 1U;

    while( stack_count != 0 )
    {
        const dz_size_t index = stack[--stack_count];
        const dz_int32_t x = (dz_int32_t)(index % (dz_size_t)width);
        const dz_int32_t y = (dz_int32_t)(index / (dz_size_t)width);

        min_x = DZ_MIN( min_x, x );
        min_y = DZ_MIN( min_y, y );
        max_x = DZ_MAX( max_x, x );
        max_y = DZ_MAX( max_y, y );

        for( dz_int32_t dy = -1; dy <= 1; ++dy )
        {
            for( dz_int32_t dx = -1; dx <= 1; ++dx )
            {
                if( dx == 0 && dy == 0 )
                {
                    continue;
                }

                const dz_int32_t nx = x + dx;
                const dz_int32_t ny = y + dy;

                if( nx < 0 || ny < 0 || nx >= width || ny >= height )
                {
                    continue;
                }

                const dz_size_t neighbor_index = (dz_size_t)ny * (dz_size_t)width + (dz_size_t)nx;

                if( visited[neighbor_index] != 0U )
                {
                    continue;
                }

                visited[neighbor_index] = 1U;

                if( __is_alpha_pixel( data, width, comp, nx, ny ) == DZ_FALSE )
                {
                    continue;
                }

                stack[stack_count++] = neighbor_index;
            }
        }
    }

    free( stack );
    free( visited );
    stbi_image_free( data );

    if( max_x < min_x || max_y < min_y )
    {
        return DZ_FAILURE;
    }

    _border = DZ_MAX( _border, 0 );

    min_x = DZ_MAX( min_x - _border, 0 );
    min_y = DZ_MAX( min_y - _border, 0 );
    max_x = DZ_MIN( max_x + _border, width - 1 );
    max_y = DZ_MIN( max_y + _border, height - 1 );

    *_out_x = min_x;
    *_out_y = min_y;
    *_out_width = max_x - min_x + 1;
    *_out_height = max_y - min_y + 1;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_render_delete_texture( GLuint _id )
{
    GLCALL( glDeleteTextures, (1, &_id) );
}
//////////////////////////////////////////////////////////////////////////
static const char * vertexShaderColorSource = "#version 330 core\n"
"layout (location = 0) in vec2 inPos;\n"
"layout (location = 1) in vec4 inColor;\n"
"uniform mat4 uWVP;\n"
"uniform float uScale;\n"
"uniform vec2 uOffset;\n"
"out vec4 v2fColor;\n"
"void main()\n"
"{\n"
"   vec3 p = vec3(inPos.xy + uOffset, 0.0) * uScale;\n"
"   gl_Position = uWVP * vec4(p, 1.0);\n"
"   v2fColor = inColor;\n"
"}\0";
//////////////////////////////////////////////////////////////////////////
static const char * fragmentShaderColorSource = "#version 330 core\n"
"in vec4 v2fColor;\n"
"out vec4 oColor;\n"
"void main()\n"
"{\n"
"   oColor = v2fColor;\n"
"}\n\0";
//////////////////////////////////////////////////////////////////////////
static const char * vertexShaderTextureSource = "#version 330 core\n"
"layout (location = 0) in vec2 inPos;\n"
"layout (location = 1) in vec4 inColor;\n"
"layout (location = 2) in vec2 inUV;\n"
"uniform mat4 uWVP;\n"
"uniform float uScale;\n"
"uniform vec2 uOffset;\n"
"out vec4 v2fColor;\n"
"out vec2 v2fUV;\n"
"void main()\n"
"{\n"
"   vec3 p = vec3(inPos.xy + uOffset, 0.0) * uScale;\n"
"   gl_Position = uWVP * vec4(p, 1.0);\n"
"   v2fColor = inColor;\n"
"   v2fUV = inUV;\n"
"}\0";
//////////////////////////////////////////////////////////////////////////
static const char * fragmentShaderTextureSource = "#version 330 core\n"
"uniform sampler2D uTextureRGB;\n"
"in vec4 v2fColor;\n"
"in vec2 v2fUV;\n"
"out vec4 oColor;\n"
"void main()\n"
"{\n"
"   vec4 texColor = texture( uTextureRGB, v2fUV );\n"
"   oColor = texColor * v2fColor;\n"
"}\n\0";
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_render_initialize( dz_render_desc_t * _desc, dz_uint16_t _max_vertex_count, dz_uint16_t _max_index_count )
{
    GLuint shaderColorProgram = __make_program( vertexShaderColorSource, fragmentShaderColorSource );
    GLuint shaderTextureProgram = __make_program( vertexShaderTextureSource, fragmentShaderTextureSource );

    GLCALL( glUseProgram, (shaderColorProgram) );

    GLint uOffsetColorLocation;
    GLCALLR( uOffsetColorLocation, glGetUniformLocation, (shaderColorProgram, "uOffset") );

    if( uOffsetColorLocation >= 0 )
    {
        GLCALL( glUniform2f, (uOffsetColorLocation, 0.f, 0.f) );
    }

    GLint uScaleColorLocation;
    GLCALLR( uScaleColorLocation, glGetUniformLocation, (shaderColorProgram, "uScale") );

    if( uScaleColorLocation >= 0 )
    {
        GLCALL( glUniform1f, (uScaleColorLocation, 1.f) );
    }

    GLCALL( glUseProgram, (shaderTextureProgram) );

    GLint texLocRGB;
    GLCALLR( texLocRGB, glGetUniformLocation, (shaderTextureProgram, "uTextureRGB") );

    if( texLocRGB >= 0 )
    {
        GLCALL( glUniform1i, (texLocRGB, 0) );
    }

    GLint uOffsetTextureLocation;
    GLCALLR( uOffsetTextureLocation, glGetUniformLocation, (shaderTextureProgram, "uOffset") );

    if( uOffsetTextureLocation >= 0 )
    {
        GLCALL( glUniform2f, (uOffsetTextureLocation, 0.f, 0.f) );
    }

    GLint uScaleTextureLocation;
    GLCALLR( uScaleTextureLocation, glGetUniformLocation, (shaderTextureProgram, "uScale") );

    if( uScaleTextureLocation >= 0 )
    {
        GLCALL( glUniform1f, (uScaleTextureLocation, 1.f) );
    }

    GLuint VAO;
    GLCALL( glGenVertexArrays, (1, &VAO) );
    GLCALL( glBindVertexArray, (VAO) );

    GLuint VBO;
    GLCALL( glGenBuffers, (1, &VBO) );

    GLCALL( glBindBuffer, (GL_ARRAY_BUFFER, VBO) );

    GLCALL( glEnableVertexAttribArray, (0) );
    GLCALL( glEnableVertexAttribArray, (1) );
    GLCALL( glEnableVertexAttribArray, (2) );

    GLCALL( glVertexAttribPointer, (0, 2, GL_FLOAT, GL_FALSE, sizeof( gl_vertex_t ), (dz_uint8_t *)0 + offsetof( gl_vertex_t, x )) );
    GLCALL( glVertexAttribPointer, (1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof( gl_vertex_t ), (dz_uint8_t *)0 + offsetof( gl_vertex_t, c )) );
    GLCALL( glVertexAttribPointer, (2, 2, GL_FLOAT, GL_FALSE, sizeof( gl_vertex_t ), (dz_uint8_t *)0 + offsetof( gl_vertex_t, u )) );

    GLuint IBO;
    GLCALL( glGenBuffers, (1, &IBO) );
    GLCALL( glBindBuffer, (GL_ELEMENT_ARRAY_BUFFER, IBO) );

    GLCALL( glBindBuffer, (GL_ARRAY_BUFFER, VBO) );

    GLCALL( glBufferData, (GL_ARRAY_BUFFER, _max_vertex_count * sizeof( gl_vertex_t ), DZ_NULLPTR, GL_DYNAMIC_DRAW) );
    GLCALL( glBufferData, (GL_ELEMENT_ARRAY_BUFFER, _max_index_count * sizeof( gl_index_t ), DZ_NULLPTR, GL_DYNAMIC_DRAW) );

    // 1x1 white texture used as a fallback when a chunk has no surface bound (e.g. SOLID materials).
    GLuint whiteTextureId;
    GLCALL( glGenTextures, (1, &whiteTextureId) );
    GLCALL( glBindTexture, (GL_TEXTURE_2D, whiteTextureId) );
    static const dz_uint8_t whitePixel[4] = {0xFF, 0xFF, 0xFF, 0xFF};
    GLCALL( glTexImage2D, (GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel) );
    GLCALL( glTexParameteri, (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST) );
    GLCALL( glTexParameteri, (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST) );
    GLCALL( glTexParameteri, (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE) );
    GLCALL( glTexParameteri, (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE) );
    GLCALL( glBindTexture, (GL_TEXTURE_2D, 0) );

    _desc->VAO = VAO;
    _desc->VBO = VBO;
    _desc->IBO = IBO;
    _desc->shaderCurrentProgram = shaderTextureProgram;
    _desc->shaderColorProgram = shaderColorProgram;
    _desc->shaderTextureProgram = shaderTextureProgram;
    _desc->whiteTextureId = whiteTextureId;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_render_finalize( dz_render_desc_t * _desc )
{
    GLCALL( glDeleteVertexArrays, (1, &_desc->VAO) );
    GLCALL( glDeleteBuffers, (1, &_desc->VBO) );
    GLCALL( glDeleteBuffers, (1, &_desc->IBO) );

    GLCALL( glDeleteProgram, (_desc->shaderColorProgram) );
    GLCALL( glDeleteProgram, (_desc->shaderTextureProgram) );

    if( _desc->whiteTextureId != 0 )
    {
        GLCALL( glDeleteTextures, (1, &_desc->whiteTextureId) );
        _desc->whiteTextureId = 0;
    }
}
//////////////////////////////////////////////////////////////////////////
void dz_render_set_proj( const dz_render_desc_t * _desc, dz_float_t _left, dz_float_t _right, dz_float_t _top, dz_float_t _bottom )
{
    dz_float_t zNear = -1.f;
    dz_float_t zFar = 1.f;

    dz_float_t projOrtho[16];
    __make_ortho( _left, _right, _top, _bottom, zNear, zFar, projOrtho );

    GLuint shaderProgram = _desc->shaderCurrentProgram;

    GLCALL( glUseProgram, (shaderProgram) );

    GLint wvpLocation;
    GLCALLR( wvpLocation, glGetUniformLocation, (shaderProgram, "uWVP") );

    if( wvpLocation >= 0 )
    {
        GLCALL( glUniformMatrix4fv, (wvpLocation, 1, GL_FALSE, projOrtho) );
    }
}
//////////////////////////////////////////////////////////////////////////
void dz_render_use_color_program( dz_render_desc_t * _desc )
{
    _desc->shaderCurrentProgram = _desc->shaderColorProgram;
}
//////////////////////////////////////////////////////////////////////////
void dz_render_use_texture_program( dz_render_desc_t * _desc )
{
    _desc->shaderCurrentProgram = _desc->shaderTextureProgram;
}
//////////////////////////////////////////////////////////////////////////
void dz_render_set_camera( const dz_render_desc_t * _desc, dz_float_t _offsetX, dz_float_t _offsetY, dz_float_t _scale )
{
    GLCALL( glUseProgram, (_desc->shaderCurrentProgram) );

    GLint uOffsetColorLocation;
    GLCALLR( uOffsetColorLocation, glGetUniformLocation, (_desc->shaderCurrentProgram, "uOffset") );

    if( uOffsetColorLocation >= 0 )
    {
        GLCALL( glUniform2f, (uOffsetColorLocation, _offsetX, _offsetY) );
    }

    GLint uScaleColorLocation;
    GLCALLR( uScaleColorLocation, glGetUniformLocation, (_desc->shaderCurrentProgram, "uScale") );

    if( uScaleColorLocation >= 0 )
    {
        GLCALL( glUniform1f, (uScaleColorLocation, _scale) );
    }
}
//////////////////////////////////////////////////////////////////////////
dz_result_t dz_render_instance( const dz_render_desc_t * _desc, const dz_instance_t * _instance )
{
    GLCALL( glUseProgram, (_desc->shaderCurrentProgram) );

    GLCALL( glBindBuffer, (GL_ARRAY_BUFFER, _desc->VBO) );
    GLCALL( glBindBuffer, (GL_ELEMENT_ARRAY_BUFFER, _desc->IBO) );

    GLCALL( glEnableVertexAttribArray, (0) );
    GLCALL( glEnableVertexAttribArray, (1) );
    GLCALL( glEnableVertexAttribArray, (2) );

    GLCALL( glVertexAttribPointer, (0, 2, GL_FLOAT, GL_FALSE, sizeof( gl_vertex_t ), (dz_uint8_t *)0 + offsetof( gl_vertex_t, x )) );
    GLCALL( glVertexAttribPointer, (1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof( gl_vertex_t ), (dz_uint8_t *)0 + offsetof( gl_vertex_t, c )) );
    GLCALL( glVertexAttribPointer, (2, 2, GL_FLOAT, GL_FALSE, sizeof( gl_vertex_t ), (dz_uint8_t *)0 + offsetof( gl_vertex_t, u )) );

    void * vertices;
    GLCALLR( vertices, glMapBuffer, (GL_ARRAY_BUFFER, GL_WRITE_ONLY) );

    void * indices;
    GLCALLR( indices, glMapBuffer, (GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY) );

    dz_instance_mesh_t mesh;
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

    dz_instance_mesh_chunk_t chunks[16];
    dz_uint32_t chunk_count;

    dz_instance_compute_mesh( _instance, &mesh, chunks, 16, &chunk_count );

    GLCALL( glUnmapBuffer, (GL_ARRAY_BUFFER) );
    GLCALL( glUnmapBuffer, (GL_ELEMENT_ARRAY_BUFFER) );

    for( dz_uint32_t index = 0; index != chunk_count; ++index )
    {
        dz_instance_mesh_chunk_t * chunk = chunks + index;

        GLuint textureId = chunk->surface != DZ_NULLPTR ? *(GLuint *)chunk->surface : 0;

        if( textureId == 0 )
        {
            // SOLID material (or atlas without an image) — bind the 1x1 white texture so the fragment shader
            // multiplies by white and produces the vertex color unchanged.
            textureId = _desc->whiteTextureId;
        }

        GLCALL( glActiveTexture, (GL_TEXTURE0) );
        GLCALL( glBindTexture, (GL_TEXTURE_2D, textureId) );

        GLCALL( glEnable, (GL_BLEND) );

        switch( chunk->blend_type )
        {
        case DZ_BLEND_NORMAL:
            {
                GLCALL( glBlendFunc, (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );
            }break;
        case DZ_BLEND_ADD:
            {
                GLCALL( glBlendFunc, (GL_SRC_ALPHA, GL_ONE) );
            }break;
        case DZ_BLEND_MULTIPLY:
            {
                GLCALL( glBlendFunc, (GL_SRC_COLOR, GL_ONE_MINUS_SRC_ALPHA) );
            }break;
        case DZ_BLEND_SCREEN:
            {
                GLCALL( glBlendFunc, (GL_ONE, GL_ONE_MINUS_SRC_ALPHA) );
            }break;
        case __DZ_BLEND_MAX__:
        default:
            return DZ_FAILURE;
        }

        GLCALL( glDrawElements, (GL_TRIANGLES, chunk->index_count, GL_UNSIGNED_SHORT, (const void *)((dz_size_t)chunk->index_offset * sizeof( dz_uint16_t ))) );
    }

    GLCALL( glBindBuffer, (GL_ARRAY_BUFFER, 0) );
    GLCALL( glBindBuffer, (GL_ELEMENT_ARRAY_BUFFER, 0) );

    GLCALL( glDisableVertexAttribArray, (0) );
    GLCALL( glDisableVertexAttribArray, (1) );
    GLCALL( glDisableVertexAttribArray, (2) );

    GLCALL( glUseProgram, (0) );

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////