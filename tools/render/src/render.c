#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_NO_SIMD
#include "stb/stb_image.h"

#include "render/render.h"

#include "glad/glad.h"

//////////////////////////////////////////////////////////////////////////
static void __make_ortho( float _l, float _r, float _t, float _b, float _n, float _f, float _m[16] )
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
    glShaderSource( vertexShaderColor, 1, &_vertexShaderSource, NULL );
    glCompileShader( vertexShaderColor );

    GLint vertexShader_success;
    glGetShaderiv( vertexShaderColor, GL_COMPILE_STATUS, &vertexShader_success );

    if( vertexShader_success == 0 )
    {
        char infoLog[512];
        glGetShaderInfoLog( vertexShaderColor, 512, NULL, infoLog );

        printf( "error: %s\n"
            , infoLog
        );

        return EXIT_FAILURE;
    }

    GLuint fragmentShaderColor = glCreateShader( GL_FRAGMENT_SHADER );
    glShaderSource( fragmentShaderColor, 1, &_fragmentShaderSource, NULL );
    glCompileShader( fragmentShaderColor );

    GLint fragmentShader_success;
    glGetShaderiv( fragmentShaderColor, GL_COMPILE_STATUS, &fragmentShader_success );
    if( fragmentShader_success == 0 )
    {
        char infoLog[512];
        glGetShaderInfoLog( fragmentShaderColor, 512, NULL, infoLog );

        printf( "error: %s\n"
            , infoLog
        );

        return EXIT_FAILURE;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader( shaderProgram, vertexShaderColor );
    glAttachShader( shaderProgram, fragmentShaderColor );
    glLinkProgram( shaderProgram );

    GLint shaderProgram_success;
    glGetProgramiv( shaderProgram, GL_LINK_STATUS, &shaderProgram_success );
    if( shaderProgram_success == 0 )
    {
        char infoLog[512];
        glGetProgramInfoLog( shaderProgram, 512, NULL, infoLog );

        printf( "error: %s\n"
            , infoLog
        );

        return EXIT_FAILURE;
    }

    glDeleteShader( vertexShaderColor );
    glDeleteShader( fragmentShaderColor );

    return shaderProgram;
}
//////////////////////////////////////////////////////////////////////////
GLuint dz_render_make_texture( const char * _path )
{
    int width;
    int height;
    int comp;

    uint8_t * data = stbi_load( _path, &width, &height, &comp, STBI_default );

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
    glGenTextures( 1, &id );
    glBindTexture( GL_TEXTURE_2D, id );
    glTexImage2D( GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, data );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

    glBindTexture( GL_TEXTURE_2D, 0 );

    stbi_image_free( data );

    return id;
}
//////////////////////////////////////////////////////////////////////////
void dz_render_delete_texture( GLuint _id )
{
    glDeleteTextures( 1, &_id );
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
dz_result_t dz_render_initialize( dz_render_desc_t * _desc, int32_t _max_vertex_count, int32_t _max_index_count )
{
    GLuint shaderColorProgram = __make_program( vertexShaderColorSource, fragmentShaderColorSource );
    GLuint shaderTextureProgram = __make_program( vertexShaderTextureSource, fragmentShaderTextureSource );

    glUseProgram( shaderColorProgram );

    GLint uOffsetColorLocation = glGetUniformLocation( shaderColorProgram, "uOffset" );

    if( uOffsetColorLocation >= 0 )
    {
        glUniform2f( uOffsetColorLocation, 0.f, 0.f );
    }

    GLint uScaleColorLocation = glGetUniformLocation( shaderColorProgram, "uScale" );

    if( uScaleColorLocation >= 0 )
    {
        glUniform1f( uScaleColorLocation, 1.f );
    }

    glUseProgram( shaderTextureProgram );

    GLint texLocRGB = glGetUniformLocation( shaderTextureProgram, "uTextureRGB" );

    if( texLocRGB >= 0 )
    {
        glUniform1i( texLocRGB, 0 );
    }

    GLint uOffsetTextureLocation = glGetUniformLocation( shaderTextureProgram, "uOffset" );

    if( uOffsetTextureLocation >= 0 )
    {
        glUniform2f( uOffsetTextureLocation, 0.f, 0.f );
    }

    GLint uScaleTextureLocation = glGetUniformLocation( shaderTextureProgram, "uScale" );

    if( uScaleTextureLocation >= 0 )
    {
        glUniform1f( uScaleTextureLocation, 1.f );
    }

    GLuint VAO;
    glGenVertexArrays( 1, &VAO );
    glBindVertexArray( VAO );

    GLuint VBO;
    glGenBuffers( 1, &VBO );

    glBindBuffer( GL_ARRAY_BUFFER, VBO );

    glEnableVertexAttribArray( 0 );
    glEnableVertexAttribArray( 1 );
    glEnableVertexAttribArray( 2 );

    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, sizeof( gl_vertex_t ), (dz_uint8_t *)0 + offsetof( gl_vertex_t, x ) );
    glVertexAttribPointer( 1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof( gl_vertex_t ), (dz_uint8_t *)0 + offsetof( gl_vertex_t, c ) );
    glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, sizeof( gl_vertex_t ), (dz_uint8_t *)0 + offsetof( gl_vertex_t, u ) );

    GLuint IBO;
    glGenBuffers( 1, &IBO );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, IBO );

    glBindBuffer( GL_ARRAY_BUFFER, VBO );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glBufferData( GL_ARRAY_BUFFER, _max_vertex_count * sizeof( gl_vertex_t ), DZ_NULLPTR, GL_DYNAMIC_DRAW );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, _max_index_count * sizeof( gl_index_t ), DZ_NULLPTR, GL_DYNAMIC_DRAW );

    _desc->VAO = VAO;
    _desc->VBO = VBO;
    _desc->IBO = IBO;
    _desc->shaderCurrentProgram = shaderColorProgram;
    _desc->shaderColorProgram = shaderColorProgram;
    _desc->shaderTextureProgram = shaderTextureProgram;

    return DZ_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void dz_render_finalize( dz_render_desc_t * _desc )
{
    glDeleteVertexArrays( 1, &_desc->VAO );
    glDeleteBuffers( 1, &_desc->VBO );
    glDeleteBuffers( 1, &_desc->IBO );

    glDeleteProgram( _desc->shaderColorProgram );
    glDeleteProgram( _desc->shaderTextureProgram );
}
//////////////////////////////////////////////////////////////////////////
void dz_render_set_proj( const dz_render_desc_t * _desc, float _left, float _right, float _top, float _bottom )
{
    float zNear = -1.f;
    float zFar = 1.f;

    float projOrtho[16];
    __make_ortho( _left, _right, _top, _bottom, zNear, zFar, projOrtho );

    GLuint shaderColorProgram = _desc->shaderColorProgram;
    GLuint shaderTextureProgram = _desc->shaderTextureProgram;

    glUseProgram( shaderColorProgram );

    GLint wvpColorLocation = glGetUniformLocation( shaderColorProgram, "uWVP" );

    if( wvpColorLocation >= 0 )
    {
        glUniformMatrix4fv( wvpColorLocation, 1, GL_FALSE, projOrtho );
    }

    glUseProgram( shaderTextureProgram );

    GLint wvpTextureLocation = glGetUniformLocation( shaderTextureProgram, "uWVP" );

    if( wvpTextureLocation >= 0 )
    {
        glUniformMatrix4fv( wvpTextureLocation, 1, GL_FALSE, projOrtho );
    }
}
//////////////////////////////////////////////////////////////////////////
void dz_render_use_color_program( dz_render_desc_t * _handle )
{
    _handle->shaderCurrentProgram = _handle->shaderColorProgram;

    glUseProgram( _handle->shaderCurrentProgram );
}
//////////////////////////////////////////////////////////////////////////
void dz_render_use_texture_program( dz_render_desc_t * _handle )
{
    _handle->shaderCurrentProgram = _handle->shaderTextureProgram;

    glUseProgram( _handle->shaderCurrentProgram );
}
//////////////////////////////////////////////////////////////////////////
void dz_render_set_camera( const dz_render_desc_t * _handle, float _offsetX, float _offsetY, float _scale )
{
    glUseProgram( _handle->shaderCurrentProgram );

    GLint uOffsetColorLocation = glGetUniformLocation( _handle->shaderCurrentProgram, "uOffset" );

    if( uOffsetColorLocation >= 0 )
    {
        glUniform2f( uOffsetColorLocation, _offsetX, _offsetY );
    }

    GLint uScaleColorLocation = glGetUniformLocation( _handle->shaderCurrentProgram, "uScale" );

    if( uScaleColorLocation >= 0 )
    {
        glUniform1f( uScaleColorLocation, _scale );
    }
}