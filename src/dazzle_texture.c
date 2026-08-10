#include "dazzle/dazzle.h"

#include "alloc.h"
#include "texture.h"

//////////////////////////////////////////////////////////////////////////
void dz_texture_create( const dz_service_t * _service, dz_texture_t ** _texture, dz_userdata_t _ud )
{
    dz_texture_t * texture = DZ_NEW( _service, dz_texture_t );

    texture->u[0] = 0.f;
    texture->v[0] = 0.f;
    texture->u[1] = 1.f;
    texture->v[1] = 0.f;
    texture->u[2] = 1.f;
    texture->v[2] = 1.f;
    texture->u[3] = 0.f;
    texture->v[3] = 1.f;

    texture->width = 0.f;
    texture->height = 0.f;

    texture->trim_offset_x = 0.f;
    texture->trim_offset_y = 0.f;

    texture->trim_width = 0.f;
    texture->trim_height = 0.f;

    texture->sequence_delay = 1.f;

    texture->ud = _ud;

    *_texture = texture;

}
//////////////////////////////////////////////////////////////////////////
void dz_texture_destroy( const dz_service_t * _service, const dz_texture_t * _texture )
{
    DZ_FREE( _service, _texture );
}
//////////////////////////////////////////////////////////////////////////
void dz_texture_set_ud( dz_texture_t * const _texture, dz_userdata_t _ud )
{
    _texture->ud = _ud;
}
//////////////////////////////////////////////////////////////////////////
dz_userdata_t dz_texture_get_ud( const dz_texture_t * _texture )
{
    return _texture->ud;
}
//////////////////////////////////////////////////////////////////////////
void dz_texture_set_uv( dz_texture_t * const _texture, const dz_float_t * _u, const dz_float_t * _v )
{
    _texture->u[0] = _u[0];
    _texture->v[0] = _v[0];
    _texture->u[1] = _u[1];
    _texture->v[1] = _v[1];
    _texture->u[2] = _u[2];
    _texture->v[2] = _v[2];
    _texture->u[3] = _u[3];
    _texture->v[3] = _v[3];
}
//////////////////////////////////////////////////////////////////////////
void dz_texture_get_uv( const dz_texture_t * _texture, dz_float_t * const _u, dz_float_t * const _v )
{
    _u[0] = _texture->u[0];
    _v[0] = _texture->v[0];
    _u[1] = _texture->u[1];
    _v[1] = _texture->v[1];
    _u[2] = _texture->u[2];
    _v[2] = _texture->v[2];
    _u[3] = _texture->u[3];
    _v[3] = _texture->v[3];
}
//////////////////////////////////////////////////////////////////////////
void dz_texture_set_width( dz_texture_t * const _texture, dz_float_t _width )
{
    _texture->width = _width;
}
//////////////////////////////////////////////////////////////////////////
dz_float_t dz_texture_get_width( const dz_texture_t * _texture )
{
    return _texture->width;
}
//////////////////////////////////////////////////////////////////////////
void dz_texture_set_height( dz_texture_t * const _texture, dz_float_t _height )
{
    _texture->height = _height;
}
//////////////////////////////////////////////////////////////////////////
dz_float_t dz_texture_get_height( const dz_texture_t * _texture )
{
    return _texture->height;
}
//////////////////////////////////////////////////////////////////////////
void dz_texture_set_trim_offset( dz_texture_t * const _texture, dz_float_t _x, dz_float_t _y )
{
    _texture->trim_offset_x = _x;
    _texture->trim_offset_y = _y;
}
//////////////////////////////////////////////////////////////////////////
void dz_texture_get_trim_offset( const dz_texture_t * _texture, dz_float_t * const _x, dz_float_t * const _y )
{
    *_x = _texture->trim_offset_x;
    *_y = _texture->trim_offset_y;
}
//////////////////////////////////////////////////////////////////////////
void dz_texture_set_trim_size( dz_texture_t * _texture, dz_float_t _width, dz_float_t _height )
{
    _texture->trim_width = _width;
    _texture->trim_height = _height;
}
//////////////////////////////////////////////////////////////////////////
void dz_texture_get_trim_size( const dz_texture_t * _texture, dz_float_t * const _width, dz_float_t * const _height )
{
    *_width = _texture->trim_width;
    *_height = _texture->trim_height;
}
//////////////////////////////////////////////////////////////////////////
void dz_texture_set_sequence_delay( dz_texture_t * const _texture, dz_float_t _delay )
{
    _texture->sequence_delay = _delay;
}
//////////////////////////////////////////////////////////////////////////
dz_float_t dz_texture_get_sequence_delay( const dz_texture_t * _texture )
{
    return _texture->sequence_delay;
}
