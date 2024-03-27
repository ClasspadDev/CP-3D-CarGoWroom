#include "Renderer.hpp"

#include "Pair.hpp"

#include "Fix16_Utils.hpp"

#include "RenderUtils.hpp"

#ifndef PC
#   include <sdk/os/lcd.hpp>
#   include <sdk/calc/calc.hpp>
#   include <sdk/os/input.hpp>
#else
#   include "PC_SDL_screen.hpp" // replaces "sdk/os/lcd.hpp"
#endif

Renderer::Renderer()
:   camera_pos({-15.0f, -1.6f, -15.0f}),
    camera_rot({0.6f, 0.4f}),
    FOV(150.0f),
    lightPos({0.0f, 0.0f, 0.0f}),
    directionalLightDir({0.0f, -0.7071f, 0.7071f}),
    minimapPos({0, 0}),
    camera_move_dirty(true)
{
    directionalLightDir = {-1.0f, -0.5f, 0.0f};
    normalize_fix16_vec3(directionalLightDir);
}

#ifdef PC
    extern uint32_t screenPixels[SCREEN_X * SCREEN_Y];
#endif

Renderer::~Renderer()
{
    // Free memory of the created models (modelArray itself has destructor which frees its own memory)
    for (auto& it : modelArray) {
        delete it.first;
    }
}

//DynamicArray<Pair<Model*, Fix16>>& Renderer::getModelArray()
DynamicLinkedList<Pair<Model*, Fix16>>& Renderer::getModelArray()
{
    return modelArray;
}

// If model has no texture, set it as NO_TEXTURE
Model* Renderer::addModel(char* model_path, char* texture_path, bool centerVertices)
{
    // Create new object
    auto m = new Model(model_path, texture_path, centerVertices);
    modelArray.push_back({m, 0.0f});
    // Return pointer back for reference
    return m;
}

unsigned int Renderer::getModelCount()
{
    return modelArray.getSize();
}

fix16_vec3& Renderer::get_camera_pos(){
    return camera_pos;
}
fix16_vec2& Renderer::get_camera_rot(){
    return camera_rot;
}
Fix16& Renderer::get_FOV(){
    return FOV;
}
fix16_vec3& Renderer::get_lightPos(){
    return lightPos;
}
int16_t_vec2& Renderer::get_minimapPos(){
    return minimapPos;
}

#ifdef PC
int Renderer::custom_sdl2_init(SDL_Window **window, SDL_Renderer **sdl_renderer, SDL_Texture ** texture)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        return -1;

    _window = SDL_CreateWindow("Classpad II PC demo",
                                SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED,
    #ifdef LANDSCAPE_MODE
                                (int) ((float) SCREEN_Y * (float) WINDOW_SIZE_MULTIPLIER),
                                (int) ((float) SCREEN_X * (float) WINDOW_SIZE_MULTIPLIER),
    #else
                                (int) ((float) SCREEN_X * (float) WINDOW_SIZE_MULTIPLIER),
                                (int) ((float) SCREEN_Y * (float) WINDOW_SIZE_MULTIPLIER),
    #endif
                                SDL_WINDOW_OPENGL);
    *window = _window;

    if (*window == nullptr) {
        SDL_Log("Could not create a window: %s", SDL_GetError());
        return -1;
    }

    _sdl_renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    *sdl_renderer = _sdl_renderer;
    if (sdl_renderer == nullptr) {
        SDL_Log("Could not create a sdl_renderer: %s", SDL_GetError());
        return -1;
    }

    _texture = SDL_CreateTexture(*sdl_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, SCREEN_X, SCREEN_Y);

    *texture = _texture;

    return 0;
}
#endif

void Renderer::screen_flush()
{
#ifndef PC
    LCD_Refresh();
#else
    SDL_UpdateTexture(_texture, NULL, screenPixels, SCREEN_X * sizeof(Uint32));
    SDL_RenderClear(_sdl_renderer);

    #ifdef LANDSCAPE_MODE
    // I have no idea how this rotation works but it does..
    SDL_Rect srcrect;
    SDL_Rect dstrect;
    auto sx = (int) ((float) SCREEN_X * (float) WINDOW_SIZE_MULTIPLIER);
    auto sy = (int) ((float) SCREEN_Y * (float) WINDOW_SIZE_MULTIPLIER);
    srcrect.x = 0;
    srcrect.y = 0;
    srcrect.w = sy;
    srcrect.h = sx;
    dstrect.x = 0;
    dstrect.y = sx;
    dstrect.w = sx;
    dstrect.h = sy;
    SDL_Point rotationCenter = {0, 0};
    SDL_RenderCopyEx(_sdl_renderer, _texture, &srcrect, &dstrect, -90.0, &rotationCenter, SDL_FLIP_NONE);
    #else
    SDL_RenderCopyEx(_sdl_renderer, _texture, NULL, NULL,   0.0, NULL, SDL_FLIP_NONE);
    #endif

    SDL_RenderPresent(_sdl_renderer);
#endif

#ifdef CLEAR_FULL_SCREEN
    fillScreen(FILL_SCREEN_COLOR);
#else

#ifdef PER_MODEL_CLEAR
    // FOR EACH MODEL
    // unsigned models_to_draw =  modelArray.getSize()/2;
    // int skip_first_models = modelArray.getSize() - models_to_draw;
    for (auto& it : modelArray) {

        // if(skip_first_models >= 0) {
        //     skip_first_models -= 1;
        //     continue;
        // }

        // Bounding box per model
        auto& bbox_max = it.first->getBoundBox_max();
        auto& bbox_min = it.first->getBoundBox_min();
#endif
        // Clear models
        for(int x=bbox_min.x; x<bbox_max.x; x++){
            for(int y=bbox_min.y; y<bbox_max.y; y++){
                // UNSAFE Pixel setting (not checking if the pixel is inside)
                #ifdef PC
                    setPixel_Unsafe(x,y, FILL_SCREEN_COLOR);
                #else
                    vram[width*y + x] = FILL_SCREEN_COLOR;
                #endif
            }
        }
#ifdef PER_MODEL_CLEAR
        // Reset the bound box
        bbox_max = {0, 0};
        bbox_min = {SCREEN_X, SCREEN_Y};
    }
#endif // PER_MODEL_CLEAR

        // Clear FPS Text (only for pc as pc draws on screen differently)
    #ifdef PC
            for(int x=10; x < 50; x++){
                for(int y=10; y < 6*4; y++){
                    setPixel_Unsafe(x,y, FILL_SCREEN_COLOR);
                }
            }
    #endif

    #ifdef LANDSCAPE_MODE
        // Clear rotation visualizer
        for(int x=SCREEN_X-ROTATION_VISUALIZER_LINE_WIDTH*2-ROTATION_VISALIZER_EDGE_OFFSET; x<SCREEN_X; x++){
            for(int y=SCREEN_Y-ROTATION_VISUALIZER_LINE_WIDTH*2-ROTATION_VISALIZER_EDGE_OFFSET; y<=SCREEN_Y; y++){
    #else
        // Clear rotation visualizer
        for(int x=SCREEN_X-ROTATION_VISUALIZER_LINE_WIDTH*2-ROTATION_VISALIZER_EDGE_OFFSET; x<SCREEN_X; x++){
            for(int y=0; y<=ROTATION_VISUALIZER_LINE_WIDTH*2+ROTATION_VISALIZER_EDGE_OFFSET; y++){
    #endif
    #ifdef PC
                setPixel_Unsafe(x,y, FILL_SCREEN_COLOR);
    #else
                vram[width*y + x] = FILL_SCREEN_COLOR;
    #endif
            }
        }

#endif // !CLEAR_FULL_SCREEN

    // Clear minimap
    draw_Minimap(true);
}

inline void draw_box(int size, int x, int y, color_t colorr)
{
    for(int16_t i=-size/2; i<size/2; i++) {
        for(int16_t j=-size/2; j<size/2; j++) {
            setPixel(x+i, y+j, colorr);
        }
    }
}

void Renderer::draw_Minimap(bool clear)
{
#ifdef LANDSCAPE_MODE
    const auto edge_offset = 5;
    const auto half_size   = 28;

    const auto offset_x = (SCREEN_X)   - edge_offset;
    const auto offset_y = (SCREEN_Y/2) - edge_offset;

    const auto x1 = offset_x;
    const auto y1 = offset_y - half_size;

    const auto x2 = x1;
    const auto y2 = offset_y + half_size;

    const auto x3 = offset_x - half_size*2;
    const auto y3 = y2;

    const auto x4 = x3;
    const auto y4 = y1;

    color_t colorr;
    if(clear) colorr = FILL_SCREEN_COLOR;
    else      colorr = color(12,12,32);

    // Draw edges
    line(x1, y1, x2, y2, colorr);
    line(x2, y2, x3, y3, colorr);
    line(x3, y3, x4, y4, colorr);
    line(x4, y4, x1, y1, colorr);

    // Center point of map
    const auto x_map_middle = offset_x - half_size;
    const auto y_map_middle = offset_y;

    // Map position offset
    fix16_vec3 minimap_pos_vec_tmp ({
        (int16_t)minimapPos.x, 0.0f, (int16_t)minimapPos.y
    });
    rotateOnPlane(minimap_pos_vec_tmp.x, minimap_pos_vec_tmp.z, camera_rot.x+PI_DIV_2);

    // Minimap scaling factor
    const auto scale_div = 5;

    // Draw each model
    const int dot_size = 2;
    for (auto& it : modelArray)
    {
        auto model_pos = it.first->getPosition_ref();
        //
        fix16_vec3 temp({ model_pos.x, 0.0f, model_pos.z });
        rotateOnPlane(temp.x, temp.z, camera_rot.x+PI_DIV_2);
        //
        const auto x = x_map_middle - (int16_t) temp.x / scale_div - (int16_t) minimap_pos_vec_tmp.x / scale_div;
        const auto y = y_map_middle + (int16_t) temp.z / scale_div + (int16_t) minimap_pos_vec_tmp.z / scale_div;

        // Skip if model was out of range
        if(x3+dot_size/2 > x || x >=x1-dot_size/2 || y1+dot_size/2 > y || y >=y2-dot_size/2)
            continue;
        // Draw square of size sx,sy at loaction x,y
        color_t model_color;
        if(clear) model_color = FILL_SCREEN_COLOR;
        else      model_color = it.first->color;
        draw_box(dot_size, x, y, model_color);
    }
#else

#endif
}

bool modelArrayCompare(const Pair<Model*, Fix16>& a, const Pair<Model*, Fix16>& b) {
    return a.second < b.second;
}

void Renderer::update()
{

#ifndef PER_MODEL_CLEAR
    bbox_max = {0, 0};
    bbox_min = {SCREEN_X, SCREEN_Y};
#endif

    // TODO: Different RENDER_MODEs have a lot in common and could therefore be
    //       combined for cleaner code. BUT cleaner code does not mean faster code here
    //       as we would want to avoid doing bunch of if checks if possible.
    //       -> Too lazy right now to figure this out..

    if (camera_move_dirty)
    {
        camera_move_dirty = false;
        // Sort all models in order from camera. A cheap way to have alteast
        // some kind of order between models. Correct way would be to do this
        // per triangle but calculating square root per triangle takes long time
        // -> Accepting tradeoff of accuracy to gain speed.

        // TODO: MAKE THIS DISTANCE ALSO TAKE INTO ACCOUNT IS IT VISIBLE OR NOT!!
        //
        unsigned i = 0;
        for (auto& it : modelArray) {
            Model* m = it.first;
            Fix16 dist = calculateDistance(m->getPosition_ref(), camera_pos);
            it.second = dist;
            i++;
        }

        // Sort by camera distance (in reverse order).
        //sort_modelRenderOrder(modelArray.getRawArray(), modelArray.getSize());
        modelArray.sort(modelArrayCompare);
    }

    bool is_valid;
    // unsigned models_to_draw =  modelArray.getSize()/2;
    // int skip_first_models = modelArray.getSize() - models_to_draw;
    for (auto& it : modelArray) {

        // if(skip_first_models >= 0) {
        //     skip_first_models -= 1;
        //     continue;
        // }

        auto RENDER_MODE = it.first->render_mode;
        #ifdef PER_MODEL_CLEAR
        auto& bbox_max = it.first->getBoundBox_max();
        auto& bbox_min = it.first->getBoundBox_min();
        #endif

        // Point-cloud
        if (RENDER_MODE == RENDER_MODES::POINT_CLOUD){
            Fix16 fix16_sink;

            // Get screen coordinates
            for (unsigned v_id=0; v_id<it.first->vertex_count; v_id++){
                fix16_vec2 screen_vec2;
                screen_vec2 = getScreenCoordinate(
                    FOV, it.first->vertices[v_id],
                    it.first->getPosition_ref(), it.first->getRotation_ref(),
                    it.first->getScale_ref(),
                    camera_pos, camera_rot,
                    &fix16_sink, &is_valid
                );
                if(is_valid == false)
                    continue;
                int16_t x = (int16_t)screen_vec2.x;
                int16_t y = (int16_t)screen_vec2.y;
                draw_center_square(x,y,5,5, color(0,0,0));
                // Check bbox
                if (bbox_max.x < x+2) bbox_max.x = x+2;
                if (bbox_max.y < y+2) bbox_max.y = y+2;
                if (bbox_min.x > x-2) bbox_min.x = x-2;
                if (bbox_min.y > y-2) bbox_min.y = y-2;
            }
        }

        // Line-render
        else if (RENDER_MODE == RENDER_MODES::LINES)
        {
            // Allocate memory
            int16_t_vec2* screen_coords = (int16_t_vec2*) malloc(sizeof(int16_t_vec2) * it.first->vertex_count);

            Fix16 fix16_sink;
            // Get screen coordinates
            for (unsigned v_id=0; v_id<it.first->vertex_count; v_id++){
                fix16_vec2 screen_vec2;
                screen_vec2 = getScreenCoordinate(
                    FOV, it.first->vertices[v_id],
                    it.first->getPosition_ref(), it.first->getRotation_ref(),
                    it.first->getScale_ref(),
                    camera_pos, camera_rot,
                    &fix16_sink, &is_valid
                );
                int16_t x = (int16_t)screen_vec2.x;
                int16_t y = (int16_t)screen_vec2.y;
                screen_coords[v_id] = {x, y};
                // Check bbox
                if(is_valid == false)
                    continue;
                if (bbox_max.x < x) bbox_max.x = x;
                if (bbox_max.y < y) bbox_max.y = y;
                if (bbox_min.x > x) bbox_min.x = x;
                if (bbox_min.y > y) bbox_min.y = y;
            }

            for (unsigned int f_id=0; f_id<it.first->faces_count; f_id++)
            {
                const auto v0 = screen_coords[it.first->faces[f_id].First];
                const auto v1 = screen_coords[it.first->faces[f_id].Second];
                const auto v2 = screen_coords[it.first->faces[f_id].Third];
                if( v0.x == (int16_t) -999 ||
                    v1.x == (int16_t) -999 ||
                    v2.x == (int16_t) -999
                ){
                    continue;
                }
                line(v0.x,v0.y, v1.x, v1.y, it.first->color);
                line(v1.x,v1.y, v2.x, v2.y, it.first->color);
                line(v2.x,v2.y, v0.x, v0.y, it.first->color);
            }
            free(screen_coords);
        }

        // Textured faces without light
        else if (RENDER_MODE == RENDER_MODES::TEXTURED)
        {
            // Check first if model has texture
            if (!it.first->has_texture){
                it.first->render_mode++;
                continue;
            }

            // Allocate memory
            int16_t_vec2* screen_coords = (int16_t_vec2*) malloc(sizeof(int16_t_vec2) * it.first->vertex_count);
            Fix16 * vert_z_depths = (Fix16*) malloc(sizeof(Fix16) * it.first->vertex_count);
            uint_fix16_t * face_draw_order = (uint_fix16_t*) malloc(sizeof(uint_fix16_t) * it.first->faces_count);

            // Get screen coordinates
            for (unsigned v_id=0; v_id<it.first->vertex_count; v_id++){
                fix16_vec2 screen_vec2;
                screen_vec2 = getScreenCoordinate(
                    FOV, it.first->vertices[v_id],
                    it.first->getPosition_ref(), it.first->getRotation_ref(),
                    it.first->getScale_ref(),
                    camera_pos, camera_rot,
                    &vert_z_depths[v_id], &is_valid
                );
                int16_t x = (int16_t)screen_vec2.x;
                int16_t y = (int16_t)screen_vec2.y;
                screen_coords[v_id] = {x, y};
                // Check bbox
                if(is_valid == false)
                    continue;
                if (bbox_max.x < x) bbox_max.x = x;
                if (bbox_max.y < y) bbox_max.y = y;
                if (bbox_min.x > x) bbox_min.x = x;
                if (bbox_min.y > y) bbox_min.y = y;
            }

            // Init the face_draw_order
            for (unsigned f_id=0; f_id<it.first->faces_count; f_id++)
            {
                unsigned int f_v0_id = it.first->faces[f_id].First;
                unsigned int f_v1_id = it.first->faces[f_id].Second;
                unsigned int f_v2_id = it.first->faces[f_id].Third;
                // Get face z-depth
                Fix16 f_z_depth  = vert_z_depths[f_v0_id]/3.0f;
                f_z_depth       += vert_z_depths[f_v1_id]/3.0f;
                f_z_depth       += vert_z_depths[f_v2_id]/3.0f;

                // Init index = f_id
                face_draw_order[f_id].uint = f_id;
                face_draw_order[f_id].fix16 = f_z_depth;
            }
            // Sorting
            bubble_sort(face_draw_order, it.first->faces_count);

            // Draw face edges
            for (unsigned int ordered_id=0; ordered_id<it.first->faces_count; ordered_id++)
            {
                auto f_id = face_draw_order[ordered_id].uint;
                const auto v0 = screen_coords[it.first->faces[f_id].First];
                const auto v1 = screen_coords[it.first->faces[f_id].Second];
                const auto v2 = screen_coords[it.first->faces[f_id].Third];
                if( v0.x == (int16_t) -999 ||
                    v1.x == (int16_t) -999 ||
                    v2.x == (int16_t) -999
                ){
                    continue;
                }
                auto uv0_fix16_norm = it.first->uv_coords[it.first->uv_faces[f_id].First];
                auto uv1_fix16_norm = it.first->uv_coords[it.first->uv_faces[f_id].Second];
                auto uv2_fix16_norm = it.first->uv_coords[it.first->uv_faces[f_id].Third];

                auto v0_u = (int16_t) (uv0_fix16_norm.x * (Fix16((int16_t)it.first->gen_textureWidth)));
                auto v0_v = (int16_t) (uv0_fix16_norm.y * (Fix16((int16_t)it.first->gen_textureHeight)));

                auto v1_u = (int16_t) (uv1_fix16_norm.x * (Fix16((int16_t)it.first->gen_textureWidth)));
                auto v1_v = (int16_t) (uv1_fix16_norm.y * (Fix16((int16_t)it.first->gen_textureHeight)));

                auto v2_u = (int16_t) (uv2_fix16_norm.x * (Fix16((int16_t)it.first->gen_textureWidth)));
                auto v2_v = (int16_t) (uv2_fix16_norm.y * (Fix16((int16_t)it.first->gen_textureHeight)));

                int16_t_Point2d v0_screen = {v0.x,v0.y, v0_u, v0_v};
                int16_t_Point2d v1_screen = {v1.x,v1.y, v1_u, v1_v};
                int16_t_Point2d v2_screen = {v2.x,v2.y, v2_u, v2_v};

                drawTriangle(
                    v0_screen, v1_screen, v2_screen,
                    //gen_uv_tex, gen_textureWidth, gen_textureHeight
                    it.first->gen_uv_tex,
                    it.first->gen_textureWidth,
                    it.first->gen_textureHeight
                );
            }
            free(face_draw_order);
            free(vert_z_depths);
            free(screen_coords);
        }

        // Texture + light
        if (RENDER_MODE == RENDER_MODES::TEXTURED_LIGHT)
        {
            // Check first if model has texture
            if (!it.first->has_texture){
                it.first->render_mode = 0;
                continue;
            }

            // Allocate memory
            int16_t_vec2* screen_coords = (int16_t_vec2*) malloc(sizeof(int16_t_vec2) * it.first->vertex_count);
            Fix16 * vert_z_depths = (Fix16*) malloc(sizeof(Fix16) * it.first->vertex_count);
            uint_fix16_t * face_draw_order = (uint_fix16_t*) malloc(sizeof(uint_fix16_t) * it.first->faces_count);
            fix16_vec3* face_normals = (fix16_vec3*) malloc(sizeof(fix16_vec3) * it.first->faces_count);

            // Get screen coordinates
            for (unsigned v_id=0; v_id<it.first->vertex_count; v_id++){
                fix16_vec2 screen_vec2;
                screen_vec2 = getScreenCoordinate(
                    FOV, it.first->vertices[v_id],
                    it.first->getPosition_ref(), it.first->getRotation_ref(),
                    it.first->getScale_ref(),
                    camera_pos, camera_rot,
                    &vert_z_depths[v_id], &is_valid
                );
                int16_t x = (int16_t)screen_vec2.x;
                int16_t y = (int16_t)screen_vec2.y;
                screen_coords[v_id] = {x, y};
                // Check bbox
                if(is_valid == false)
                    continue;
                if (bbox_max.x < x) bbox_max.x = x;
                if (bbox_max.y < y) bbox_max.y = y;
                if (bbox_min.x > x) bbox_min.x = x;
                if (bbox_min.y > y) bbox_min.y = y;
            }

            // Init the face_draw_order
            for (unsigned f_id=0; f_id<it.first->faces_count; f_id++)
            {
                unsigned int f_v0_id = it.first->faces[f_id].First;
                unsigned int f_v1_id = it.first->faces[f_id].Second;
                unsigned int f_v2_id = it.first->faces[f_id].Third;

                // Get face z-depth
                Fix16 f_z_depth  = vert_z_depths[f_v0_id]/3.0f;
                f_z_depth       += vert_z_depths[f_v1_id]/3.0f;
                f_z_depth       += vert_z_depths[f_v2_id]/3.0f;

                // Init index = f_id
                face_draw_order[f_id].uint = f_id;
                face_draw_order[f_id].fix16 = f_z_depth;
                // ----- Calculate also face normals here

                // Face vertices
                fix16_vec3 v0 = it.first->vertices[f_v0_id];
                fix16_vec3 v1 = it.first->vertices[f_v1_id];
                fix16_vec3 v2 = it.first->vertices[f_v2_id];
                // Model rotation
                rotateOnPlane(v0.x, v0.z, it.first->rotation.x);
                rotateOnPlane(v0.y, v0.z, it.first->rotation.y);
                rotateOnPlane(v1.x, v1.z, it.first->rotation.x);
                rotateOnPlane(v1.y, v1.z, it.first->rotation.y);
                rotateOnPlane(v2.x, v2.z, it.first->rotation.x);
                rotateOnPlane(v2.y, v2.z, it.first->rotation.y);
                // Model translation
                v0.x += it.first->position.x;
                v0.y += it.first->position.y;
                v0.z += it.first->position.z;
                v1.x += it.first->position.x;
                v1.y += it.first->position.y;
                v1.z += it.first->position.z;
                v2.x += it.first->position.x;
                v2.y += it.first->position.y;
                v2.z += it.first->position.z;
                // Calculate face normal
                auto face_norm = calculateNormal(v0, v1, v2);
                normalize_fix16_vec3(face_norm);
                //
                face_normals[f_id] = face_norm;
            }

            // Sorting
            bubble_sort(face_draw_order, it.first->faces_count);

            // Draw face edges
            for (unsigned int ordered_id=0; ordered_id<it.first->faces_count; ordered_id++)
            {
                auto f_id = face_draw_order[ordered_id].uint;
                const auto v0 = screen_coords[it.first->faces[f_id].First];
                const auto v1 = screen_coords[it.first->faces[f_id].Second];
                const auto v2 = screen_coords[it.first->faces[f_id].Third];
                if( v0.x == (int16_t) -999 ||
                    v1.x == (int16_t) -999 ||
                    v2.x == (int16_t) -999
                ){
                    continue;
                }
                auto uv0_fix16_norm = it.first->uv_coords[it.first->uv_faces[f_id].First];
                auto uv1_fix16_norm = it.first->uv_coords[it.first->uv_faces[f_id].Second];
                auto uv2_fix16_norm = it.first->uv_coords[it.first->uv_faces[f_id].Third];

                auto v0_u = (int16_t) (uv0_fix16_norm.x * (Fix16((int16_t)it.first->gen_textureWidth)));
                auto v0_v = (int16_t) (uv0_fix16_norm.y * (Fix16((int16_t)it.first->gen_textureHeight)));

                auto v1_u = (int16_t) (uv1_fix16_norm.x * (Fix16((int16_t)it.first->gen_textureWidth)));
                auto v1_v = (int16_t) (uv1_fix16_norm.y * (Fix16((int16_t)it.first->gen_textureHeight)));

                auto v2_u = (int16_t) (uv2_fix16_norm.x * (Fix16((int16_t)it.first->gen_textureWidth)));
                auto v2_v = (int16_t) (uv2_fix16_norm.y * (Fix16((int16_t)it.first->gen_textureHeight)));

                int16_t_Point2d v0_screen = {v0.x,v0.y, v0_u, v0_v};
                int16_t_Point2d v1_screen = {v1.x,v1.y, v1_u, v1_v};
                int16_t_Point2d v2_screen = {v2.x,v2.y, v2_u, v2_v};

                Fix16 lightIntensity = calculateLightIntensityDirLight(
                        directionalLightDir, face_normals[f_id], Fix16(1.0f)
                );
                drawTriangle(
                    v0_screen, v1_screen, v2_screen,
                    it.first->gen_uv_tex,
                    it.first->gen_textureWidth,
                    it.first->gen_textureHeight,
                    lightIntensity
                );
            }
            free(face_draw_order);
            free(vert_z_depths);
            free(screen_coords);
            free(face_normals);
        }

        #ifdef PER_MODEL_CLEAR
        //Some buffer around bbox (as draw lines may draw over the bbox)
        bbox_min.x -= 2;
        bbox_min.y -= 2;
        bbox_max.x += 2;
        bbox_max.y += 2;
        // Check bbox - clamp
        if(bbox_min.x < 0)        bbox_min.x = 0;
        if(bbox_min.y < 0)        bbox_min.y = 0;
        if(bbox_max.x > SCREEN_X) bbox_max.x = SCREEN_X;
        if(bbox_max.y > SCREEN_Y) bbox_max.y = SCREEN_Y;
        #endif
    }

#ifndef PER_MODEL_CLEAR
    // Some buffer around bbox (as draw lines may draw over the bbox)
    bbox_min.x -= 2;
    bbox_min.y -= 2;
    bbox_max.x += 2;
    bbox_max.y += 2;

    // Check bbox - clamp
    if(bbox_min.x < 0)        bbox_min.x = 0;
    if(bbox_min.y < 0)        bbox_min.y = 0;
    if(bbox_max.x > SCREEN_X) bbox_max.x = SCREEN_X;
    if(bbox_max.y > SCREEN_Y) bbox_max.y = SCREEN_Y;
#endif
    // Draw rotation visualizer in corner
    draw_RotationVisualizer(camera_rot);

    // Draw minimap
    draw_Minimap(false);
}

