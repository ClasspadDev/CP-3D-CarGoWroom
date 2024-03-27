#pragma once

// TODO: Make separate file for fix16 vectors instead. . .
#include "RenderFP3D.hpp"

#include "RenderUtils.hpp"

struct u_pair {
    unsigned First;
    unsigned Second;
};

struct u_triple {
    unsigned First;
    unsigned Second;
    unsigned Third;
};

class Model
{
private:
    bool loaded_from_file;

    // Transform raw model vertices to the geometric center
    void _centerModel();

#ifdef PER_MODEL_CLEAR
    int16_t_vec2 bbox_max;
    int16_t_vec2 bbox_min;
#endif

public:

    Model(char* fname, char* ftexture, bool centerVertices);
    ~Model();

    fix16_vec3 position;
    fix16_vec2 rotation;
    fix16_vec3 scale;

    fix16_vec3* vertices;
    unsigned    vertex_count;
    u_triple*   faces;
    unsigned    faces_count;

    fix16_vec2* uv_coords;
    unsigned    uv_coord_count;
    u_triple*   uv_faces;
    unsigned    uv_face_count;

    bool has_texture;
    int gen_textureWidth;
    int gen_textureHeight;
    uint32_t * gen_uv_tex; // Malloced array of size: gen_textureWidth * gen_textureHeight

    fix16_vec3& getPosition_ref();
    fix16_vec2& getRotation_ref();
    fix16_vec3& getScale_ref();
#ifdef PER_MODEL_CLEAR
    int16_t_vec2& getBoundBox_max();
    int16_t_vec2& getBoundBox_min();
#endif

    uint16_t render_mode;

    color_t color;

    // size of a sphere that encapsulates the model (when model is centered using _centerModel)
    Fix16 encapsulating_radius;
    // Some extra info about collision (TODO: Perhaps there is better way. . .)
    unsigned char collision_extra;

    // Run obj through python script to generate binary format
    bool load_from_binary_obj_file(char* fname, char* ftexture, bool center=true);

    // Scale raw model vertices
    void _scaleModel(Fix16 factor);
    void _scaleModel_Z(Fix16 factor);
    // Scale raw model vertices such that distance between its 2 furthest
    // apart vertices is given maxWidth
    void _scaleModelTo(Fix16 maxWidth);
    // Changes the transfrom point of model by shifting all vertices
    void _shiftTransform(fix16_vec3 transform);
    // Sets encapsulating_radius correctly. Must call _centerModel before!
    void _calculateEncapsulatingSphere();

    void _update_vertex_center();

};
