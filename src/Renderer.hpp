#pragma once

#include "Model.hpp"

#include "DynamicArray.hpp"
#include "DynamicLinkedList.hpp"

#include "Pair.hpp"

#ifdef PC
#   include <SDL2/SDL.h>
#endif

#define FILL_SCREEN_COLOR color(190,190,190)

#define _NO_TEXTURE_IMPL    (char*)NO_TEXTURE_PATH
#define NO_TEXTURE          _NO_TEXTURE_IMPL

const char NO_TEXTURE_PATH[] = "\0";

const uint16_t RENDER_MODE_COUNT = 4;

enum RENDER_MODES {
    POINT_CLOUD     = 0,
    LINES           = 1,
    TEXTURED        = 2,
    TEXTURED_LIGHT  = 3
};

class Renderer
{
private:
    //DynamicArray<Pair<Model*, Fix16>> modelArray;
    DynamicLinkedList<Pair<Model*, Fix16>> modelArray;

    fix16_vec3 camera_pos;
    fix16_vec2 camera_rot;
    Fix16 FOV;

    fix16_vec3 lightPos;
    fix16_vec3 directionalLightDir;

    int16_t_vec2 minimapPos;

#ifndef PER_MODEL_CLEAR
    int16_t_vec2 bbox_max;
    int16_t_vec2 bbox_min;
#endif

public:

    bool camera_move_dirty;

    DynamicLinkedList<Pair<Model*, Fix16>>& getModelArray();
    // If model has no texture, set as NO_TEXTURE
    Model* addModel(char* model_path, char* texture_path, bool centerVertices=true);
    unsigned int getModelCount();

    void update();

    fix16_vec3& get_camera_pos();
    fix16_vec2& get_camera_rot();
    Fix16     & get_FOV();
    fix16_vec3& get_lightPos();
    int16_t_vec2& get_minimapPos();

    void screen_flush();

    void draw_Minimap(bool clear);

#ifdef PC
    int custom_sdl2_init(SDL_Window **window, SDL_Renderer **sdl_renderer, SDL_Texture ** texture);
    SDL_Window * _window;
    SDL_Renderer * _sdl_renderer;
    SDL_Texture * _texture;
#endif

    Renderer();
    ~Renderer();
};
