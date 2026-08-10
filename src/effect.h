#ifndef DZ_EFFECT_H_
#define DZ_EFFECT_H_

#include "dazzle/dazzle.h"

#include "shape.h"
#include "emitter.h"
#include "affector.h"
#include "particle.h"

typedef struct dz_mesh_bvh_node_t
{
    dz_aabb_t bounds;
    dz_uint32_t first;
    dz_uint32_t count;
    dz_uint32_t left;
    dz_uint32_t right;
} dz_mesh_bvh_node_t;

typedef struct dz_mesh_bvh_t
{
    dz_mesh_bvh_node_t * nodes;
    dz_uint32_t node_count;
    dz_uint32_t * triangles;
} dz_mesh_bvh_t;

typedef struct dz_effect_t
{
    dz_project_profile_t profile;

    const dz_atlas_t * atlas;

    dz_effect_layer_desc_t layers[DZ_EFFECT_LAYER_MAX];
    dz_uint32_t layer_count;

    dz_effect_trigger_desc_t triggers[DZ_EFFECT_TRIGGER_MAX];
    dz_uint32_t trigger_count;

    dz_mesh_desc_t meshes[DZ_EFFECT_MESH_MAX];
    dz_mesh_bvh_t mesh_bvhs[DZ_EFFECT_MESH_MAX];
    dz_uint32_t mesh_count;

    dz_physics_object_desc_t physics_objects[DZ_EFFECT_PHYSICS_OBJECT_MAX];
    dz_uint32_t physics_object_count;

    dz_uint32_t seed;

    dz_float_t life;

    dz_userdata_t ud;
} dz_effect_t;

dz_uint32_t dz_effect_find_mesh_index( const dz_effect_t * _effect, dz_uint32_t _id );
const dz_mesh_desc_t * dz_effect_find_mesh( const dz_effect_t * _effect, dz_uint32_t _id );

#endif
