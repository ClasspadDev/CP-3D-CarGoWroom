
#include "libfixmath/fix16.hpp"

#include "RenderFP3D.hpp"

#include "Model.hpp"

#include "Renderer.hpp"

#include "DynamicArray.hpp"

#include "Car.hpp"

#include "DynamicLinkedList.hpp"

#ifndef PC
#   include "app_description.hpp"
#   include <sdk/calc/calc.hpp>
#   include <sdk/os/input.hpp>
#   include <sdk/os/lcd.hpp>
#   include <sdk/os/debug.hpp>
#   include <sdk/os/mem.hpp>
#   include <sdk/os/file.hpp>
#   include "fps_functions.hpp"
#else
    // SDL2 as our graphics library
#   include <SDL2/SDL.h>
    // This is not a standard "header"!
    // These functions are pretty much 1-to-1 copied from hollyhock2
    // sdk but instead of drawing to calculator screen (vram)
    // it draws to SDL2 screen (texture).
#   include "PC_SDL_screen.hpp" // replaces "sdk/os/lcd.hpp"
#   include <iostream>  // std::string
#   include <unistd.h>  // File open & close
#   include <fcntl.h>   // File open & close
#endif

// Keymappings, both ClassPad and SDL2
#ifndef PC
#   define KEY_MOVE_LEFT       testKey(k1,k2,KEY_4)
#   define KEY_MOVE_RIGHT      testKey(k1,k2,KEY_6)
#   define KEY_MOVE_FORWARD    testKey(k1,k2,KEY_8)
#   define KEY_MOVE_BACKWARD   testKey(k1,k2,KEY_2)
#   define KEY_MOVE_UP         testKey(k1,k2,KEY_9)
#   define KEY_MOVE_DOWN       testKey(k1,k2,KEY_3)
#   define KEY_MOVE_FOV_ADD    testKey(k1,k2,KEY_ADD)
#   define KEY_MOVE_FOV_SUB    testKey(k1,k2,KEY_SUBTRACT)
#   define KEY_MOVE_REND_MODE  testKey(k1,k2,KEY_0)
#   define KEY_BOOST           testKey(k1,k2,KEY_Z)
#   ifdef LANDSCAPE_MODE
#       define KEY_ROTATE_LEFT     testKey(k1,k2,KEY_UP)
#       define KEY_ROTATE_RIGHT    testKey(k1,k2,KEY_DOWN)
#       define KEY_ROTATE_UP       testKey(k1,k2,KEY_RIGHT)
#       define KEY_ROTATE_DOWN     testKey(k1,k2,KEY_LEFT)
#   else
#       define KEY_ROTATE_LEFT     testKey(k1,k2,KEY_LEFT)
#       define KEY_ROTATE_RIGHT    testKey(k1,k2,KEY_RIGHT)
#       define KEY_ROTATE_UP       testKey(k1,k2,KEY_UP)
#       define KEY_ROTATE_DOWN     testKey(k1,k2,KEY_DOWN)
#   endif
#   define KEY_QUIT            testKey(k1,k2,KEY_CLEAR)
#else
#   define KEY_BOOST           key_space
#   define KEY_MOVE_LEFT       key_left
#   define KEY_MOVE_RIGHT      key_right
#   define KEY_MOVE_FORWARD    key_up
#   define KEY_MOVE_BACKWARD   key_down
#   define KEY_MOVE_UP         key_r
#   define KEY_MOVE_DOWN       key_f
#   define KEY_MOVE_FOV_ADD    key_1
#   define KEY_MOVE_FOV_SUB    key_2
#   define KEY_MOVE_REND_MODE  key_e
#   define KEY_ROTATE_LEFT     key_a
#   define KEY_ROTATE_RIGHT    key_d
#   define KEY_ROTATE_UP       key_w
#   define KEY_ROTATE_DOWN     key_s
#   define KEY_QUIT            key_ESCAPE
#endif

#include "DynamicArray.hpp"  // Include the source file

#ifndef PC
// fps10 from "fps_functions.hpp" in calculator case
extern int fps10;
#endif

#define MOVEMENT_SPEED   15.0f
#define CAMERA_SPEED      1.15f
#define FOV_UPDATE_SPEED 20.0f

inline fix16_vec2 calculate2DForward(const fix16_vec2& rotation2D) {
    const Fix16 pitch = rotation2D.x;

    fix16_vec2 forward;
    forward.x = fix16_sin(pitch) ;
    forward.y = fix16_cos(pitch) ;

    return forward;
}



void init_map(Renderer* renderer)
{
#ifdef PC
    std::cout << "map reading" << std::endl;
#endif

    char model2_path[] =
#ifdef PC
        "./3D_Converted_Models/little_endian_test2.pkObj";
#else
        "\\fls0\\big_endian_test2.pkObj";
#endif

    char map_path[] =
#ifdef PC
        "./python/little_map.map";
#else
        "\\fls0\\big_map.map";
#endif

    int fd = open(map_path, UNIVERSIAL_FILE_READ );
    char buff[32] = {0};
    memset(buff, 0, 32);

    // Read 4 bytes (map_elem_count)
    read(fd, buff, 4);
    uint32_t map_elem_count = *((uint32_t*)(buff+0));
    // Read elements
    for (uint32_t i=0; i<map_elem_count; i++){
        // Read 3 * 4 bytes (map_elem_count)
        read(fd, buff, 3*4);
        Fix16 x = *((Fix16*)(buff+0));
        Fix16 y = *((Fix16*)(buff+4));
        uint32_t elem_type = *((uint32_t*)(buff+8));

        // Add to model array
        // class MapElementTypes(Enum):
        //     Player = 0
        //     Wall   = 1
        //     Boost  = 2
        uint8_t r, g, b;
        unsigned char collision_info = 0;
        Fix16 modelScale = 4.0f;
        Fix16 scaleModel_Z = 1.0f;
        if      (elem_type == 0){ continue;} // Player
        else if (elem_type == 1) {
            collision_info = 1;
            modelScale = 8.0f;
            scaleModel_Z = 0.4f;
            r = 60; g = 10; b = 30;
        }
        else if (elem_type == 2) {
            collision_info = 2;
            modelScale = 4.0f;
            r = 10; g = 180; b = 30;
        }
        else {
            #ifdef PC
            std::cout
                    << "Node elem_Type unknown"
                    << std::endl;
            #endif
            continue;
        }
        auto m = renderer->addModel(model2_path, NO_TEXTURE);
        m->getPosition_ref().x = x;
        m->getPosition_ref().z = y;
        m->getPosition_ref().y = 0.0f;
        m->color = color(r, g, b);
        m->collision_extra = collision_info;

        // Rotation
        m->getRotation_ref().y = Fix16(3.145f/2.0f);
        m->render_mode = RENDER_MODES::LINES;

        // Scale model such that the max width
        m->_scaleModelTo(modelScale);
        if(scaleModel_Z != 1.0f){
            m->_scaleModel_Z(scaleModel_Z);
        }
        m->_calculateEncapsulatingSphere();

    }
    close(fd);
}



bool modelArrayEqual(const Pair<Model*, Fix16>& a, const Pair<Model*, Fix16>& b) {
    return a.first == b.first;
}

#ifndef PC
extern "C" void main()
{
    calcInit(); //backup screen and init some variables
#else // ifdef PC
int main(int argc, const char * argv[])
{
    SDL_Window *window;
    SDL_Renderer *sdl_renderer;
    SDL_Texture * texture;

    #define SDL2_MAX_FRAMERATE_LIST_COUNT 6
    uint32_t sdl_frame_rate_i = 0;
    uint32_t SDL2_MAX_FRAMERATE_LIST[SDL2_MAX_FRAMERATE_LIST_COUNT] = {
        1000, 35, 24, 16, 12, 9
    };
    uint32_t SDL2_MAX_FRAMERATE = SDL2_MAX_FRAMERATE_LIST[sdl_frame_rate_i];

    bool key_left = false;
    bool key_right = false;
    bool key_up = false;
    bool key_down = false;
    bool key_r = false;
    bool key_f = false;
    bool key_1 = false;
    bool key_2 = false;
    bool key_w = false;
    bool key_s = false;
    bool key_a = false;
    bool key_d = false;
    bool key_e = false;
    bool key_ESCAPE = false;
    bool key_space = false;
    bool KEY_MOVE_LEFT_prev = false;
#endif // PC
    bool KEY_RENDER_MODE_prev = false; // De-bouncing the button
    bool camera_position_prev = false; // De-bouncing the button
    char model1_path[] =
#ifdef PC
        "./3D_Converted_Models/little_endian_my_car.pkObj";
#else
        "\\fls0\\big_endian_my_car.pkObj";
#endif

    char model1_texture_path[] =
#ifdef PC
        "./3D_Converted_Models/little_endian_my_car.texture";
#else
        "\\fls0\\big_endian_my_car.texture";
#endif

    char model2_path[] =
#ifdef PC
        "./3D_Converted_Models/little_endian_test2.pkObj";
#else
        "\\fls0\\big_endian_test2.pkObj";
#endif


    fillScreen(FILL_SCREEN_COLOR);
#ifndef PC
    // Let user know that program has not crashed and we are loading model
    Debug_SetCursorPosition(1,1);
    Debug_PrintString("Load obj", false);
    LCD_Refresh();
#endif

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~  Creating Renderer and Models ~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    #define CAMERA_HEIGHT 10.0f

    // Create renderer
    Renderer renderer;
    renderer.get_lightPos().y = -10.0f;
    renderer.get_camera_pos().y = -CAMERA_HEIGHT;

#ifdef PC
    int init_status = renderer.custom_sdl2_init(&window, &sdl_renderer, &texture);
    if (init_status != 0) return init_status;
#endif

    // Create Car Model
    auto car_Model = renderer.addModel(model1_path, model1_texture_path);
    car_Model->_scaleModelTo(8.0f);
    car_Model->getRotation_ref().y = Fix16(fix16_pi);
    car_Model->_shiftTransform({0.0f,0.0f,1.0f});
    car_Model->color = color(255,0,0);
    car_Model->_calculateEncapsulatingSphere();

    // Create map out of file
    init_map(&renderer);

    // Car logic update
    Car car = Car();
    car.get_rot() = fix16_pi;

#ifdef PC
    uint32_t time_t0 = SDL_GetTicks();
    int accumulative_frames  = 0;
    uint32_t last_fps = 0;
    bool PC_ALLOW_RENDER = true;
#endif

    // Delta-time
    Fix16 last_dt = Fix16((int16_t) 0.0016f);

    bool accelerate = false;
    bool turn_left  = false;
    bool turn_right = false;
    bool car_break = false;
    bool boost = false;
    bool done = false;

    Fix16 camera_car_distance = 12.0f;

    uint16_t camera_position_preset = 0;

    while(!done)
    {

#ifndef PC
        // FPS & delta-time count timer
        fps_update();
#endif

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~ Key Presses ~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef PC
        // ClassPad keypresses are stored in k1 and k2 in bits
        uint32_t k1,k2; getKey(&k1,&k2);
#else // !PC -> PC
        // Get the next event
        SDL_Event event;
        if (SDL_PollEvent(&event))
        {
            switch( event.type ){
                /* Keyboard event */
                /* Pass the event data onto PrintKeyInfo() */
                case SDL_KEYDOWN:
                    switch( event.key.keysym.sym ){
                        case SDLK_LEFT:   key_left  = true; break;
                        case SDLK_RIGHT:  key_right = true; break;
                        case SDLK_UP:     key_up    = true; break;
                        case SDLK_DOWN:   key_down  = true; break;
                        case SDLK_r:      key_r     = true; break;
                        case SDLK_f:      key_f     = true; break;
                        case SDLK_a:      key_a     = true; break;
                        case SDLK_d:      key_d     = true; break;
                        case SDLK_w:      key_w     = true; break;
                        case SDLK_s:      key_s     = true; break;
                        case SDLK_1:      key_1     = true; break;
                        case SDLK_2:      key_2     = true; break;
                        case SDLK_e:      key_e     = true; break;
                        case SDLK_ESCAPE: key_ESCAPE = true; break;
                        case SDLK_SPACE:  key_space  = true; break;
                        default:                            break;
                    }
                    break;
                case SDL_KEYUP:
                    switch( event.key.keysym.sym ){
                        case SDLK_LEFT:  key_left  = false; break;
                        case SDLK_RIGHT: key_right = false; break;
                        case SDLK_UP:    key_up    = false; break;
                        case SDLK_DOWN:  key_down  = false; break;
                        case SDLK_r:     key_r     = false; break;
                        case SDLK_f:     key_f     = false; break;
                        case SDLK_a:     key_a     = false; break;
                        case SDLK_d:     key_d     = false; break;
                        case SDLK_w:     key_w     = false; break;
                        case SDLK_s:     key_s     = false; break;
                        case SDLK_1:     key_1     = false; break;
                        case SDLK_2:     key_2     = false; break;
                        case SDLK_e:     key_e     = false; break;
                        case SDLK_ESCAPE: key_ESCAPE = false; break;
                        case SDLK_SPACE:  key_space  = false; break;
                        default:                            break;
                    }
                    break;
                case SDL_QUIT: // Closing window i.e. pressing window "X" button
                    done = true;
                    break;
                default:
                    break;
            }
        }
#endif // PC

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~ Key Presses ~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef PC
    // Only poll Calculator when any key was pressed
    if (Input_IsAnyKeyDown())
    {
#else
    if (PC_ALLOW_RENDER)
    {
#endif
        if (KEY_QUIT)
            done = true;

        if (KEY_MOVE_LEFT) {
            //renderer.get_camera_rot().y += last_dt * 1.0f;

            #ifdef PC
            if(KEY_MOVE_LEFT_prev == false){
                sdl_frame_rate_i = (sdl_frame_rate_i+1) % SDL2_MAX_FRAMERATE_LIST_COUNT;
                SDL2_MAX_FRAMERATE = SDL2_MAX_FRAMERATE_LIST[sdl_frame_rate_i];
            }
            KEY_MOVE_LEFT_prev = true;
            #endif
        }
        #ifdef PC
        else {
            KEY_MOVE_LEFT_prev = false;
        }
        #endif

        if (KEY_MOVE_RIGHT) {
            if (camera_position_prev == false){

                const uint16_t camera_position_preset_COUNT = 4;
                camera_position_preset = (camera_position_preset+1) % camera_position_preset_COUNT;
                if (camera_position_preset == 0){
                    renderer.get_FOV() = 150.0f;
                    camera_car_distance         = 9.0f;
                    renderer.get_camera_pos().y = -CAMERA_HEIGHT;
                    renderer.get_camera_rot().y = 0.1f;
                }
                else if (camera_position_preset == 1){
                    renderer.get_FOV() = 150.0f;
                    camera_car_distance         = 6.0f;
                    renderer.get_camera_pos().y = -CAMERA_HEIGHT-6.0f;
                    renderer.get_camera_rot().y = 0.1f;
                }
                else if (camera_position_preset == 2){
                    renderer.get_FOV() = 130.0f;
                    camera_car_distance         = 3.0f;
                    renderer.get_camera_pos().y = -CAMERA_HEIGHT-15.0f;
                    renderer.get_camera_rot().y = 0.5f;
                }
                else if (camera_position_preset == 3){
                    renderer.get_FOV() = 120.0f;
                    camera_car_distance         = 1.0f;
                    renderer.get_camera_pos().y = -CAMERA_HEIGHT-21.0f;
                    renderer.get_camera_rot().y = 0.5f;
                }
            }
            camera_position_prev = true;
        } else {
            camera_position_prev = false;
        }

        if (KEY_MOVE_FORWARD)
            camera_car_distance -= last_dt * MOVEMENT_SPEED;
        if (KEY_MOVE_BACKWARD)
            camera_car_distance += last_dt * MOVEMENT_SPEED;

        if (KEY_MOVE_UP) {
            renderer.get_camera_pos().y -= last_dt * MOVEMENT_SPEED;
        }
        if (KEY_MOVE_DOWN){
            renderer.get_camera_pos().y += last_dt * MOVEMENT_SPEED;
        }

        if (KEY_MOVE_FOV_ADD) {
            //renderer.get_FOV() += last_dt * FOV_UPDATE_SPEED;
        }
        if (KEY_MOVE_FOV_SUB){
            //renderer.get_FOV() -= last_dt * FOV_UPDATE_SPEED;
        }

        if (KEY_MOVE_REND_MODE){
            if(KEY_RENDER_MODE_prev == false){
                if(car_Model->render_mode < RENDER_MODE_COUNT-1)
                    car_Model->render_mode = car_Model->render_mode + 1;
                else
                    car_Model->render_mode = 0;
            }
            KEY_RENDER_MODE_prev = true;
        } else {
            KEY_RENDER_MODE_prev = false;
        }

        if(KEY_ROTATE_UP)
            accelerate = true;
        if(KEY_ROTATE_DOWN)
            car_break = true;
        if(KEY_BOOST)
            boost = true;
        if(KEY_ROTATE_LEFT)
            turn_left = true;
        if(KEY_ROTATE_RIGHT)
            turn_right = true;

#ifndef PC
    } // Input_IsAnyKeyDown()
    else {
        KEY_RENDER_MODE_prev = false;
        camera_position_prev = false;
    }
#else
    } // (PC_ALLOW_RENDER)
#endif

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~ Main Loop ~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifdef PC
    if (PC_ALLOW_RENDER)
    {
#endif
        // ~~~~~~~~~~~~~~~~~~~~~  Collisions ~~~~~~~~~~~~~~~~~~~~~
        for (auto iter = renderer.getModelArray().node_begin(); iter != renderer.getModelArray().node_end(); ++iter) {
            auto node = (*iter);
            if (node->data.first == car_Model)
                continue;
            const auto modelPos = node->data.first->getPosition_ref();
            const auto carPos   = car_Model->getPosition_ref();
            const auto combinedPos = sub_vec3(carPos, modelPos);
            Fix16 dist = calculateLength(combinedPos);
            if(dist <= (node->data.first->encapsulating_radius + car_Model->encapsulating_radius)/2.0f)
            {
                if(node->data.first->collision_extra == 2){

                    car.add_boost(MAX_BOOST_TIME/4.0f);
                    // --- Remove boost ----
                    // Free memory of the created model
                    delete node->data.first;
                    // Remove node (internally frees its data)
                    renderer.getModelArray().remove(*node); // Removing by node is very fast
                }
                else if (node->data.first->collision_extra == 1){
                    // Wall
                    car.get_speed() = -15.0f;
                }

                // Only one collision per loop
                break;
            }
        }

        // Assuming camera has always moved due to car rolling always
        renderer.camera_move_dirty = true;

        // Due to numerical inaccuracies in fixed point math,
        // when delta time gets too large lets update twice.
        uint16_t update_count = 4;
        last_dt = last_dt/4.0f;
        // Update in loop logic thats dependent on delta time
        for (uint16_t i=0; i<update_count; i++)
        {
            car.update(last_dt, accelerate, car_break, turn_left, turn_right, boost);

            // Effect: Camera position lagging behind to give sense of speed
            auto cam_forward = calculate2DForward(renderer.get_camera_rot());
            const Fix16 cam_targ_x = car.get_pos().x - (cam_forward.x * camera_car_distance);
            const Fix16 cam_targ_y = car.get_pos().y - (cam_forward.y * camera_car_distance);
            renderer.get_camera_pos().x = easeInLinear(renderer.get_camera_pos().x, cam_targ_x, last_dt, 5.0f);
            renderer.get_camera_pos().z = easeInLinear(renderer.get_camera_pos().z, cam_targ_y, last_dt, 5.0f);

            // Effect: Camera rotation slightly lagging behind
            renderer.get_camera_rot().x = easeInLinear(renderer.get_camera_rot().x, car.get_rot(),
                last_dt,
                Fix16(3.5f)
            );

            // Update car model rotation
            // Effect: Slight delay in actual rotation makes it look both smoother and more "real"
            car_Model->getRotation_ref().x = easeInLinear(car_Model->getRotation_ref().x, car.get_rot() + Fix16(fix16_pi), last_dt, 5.5f);
        }
        // Reset the key states
        accelerate = false;
        car_break  = false;
        boost = false;
        turn_left  = false;
        turn_right = false;

        // Update car model position
        car_Model->position.x = car.get_pos().x;
        car_Model->position.z = car.get_pos().y;

        // Effect: FOV change based on speed
        renderer.get_FOV() = Fix16(170.0f) - car.get_speed_perc()*30.0f;

        renderer.get_minimapPos().x = -car.get_pos().x;
        renderer.get_minimapPos().y = -car.get_pos().y;

        // Draw car UI
        car.draw_UI();

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~ Rendering ~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        renderer.update();

#ifdef PC
    } // (PC_ALLOW_RENDER)
#endif
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~ Display FPS ~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef PC
        // Note that this is directly yanked from <insert_someones_git_and_name>.
        // The whole "fps_functions.hpp" is taken.
        // I have not written FPS calculation functionality.
        fps_formatted_update();
        fps_display();
        last_dt = Fix16(1.0f) / (Fix16(((int16_t) fps10)) / 10.0f);
#else
        // SDL_GetTicks() seems not to be super accurate, so adding frames to
        // accumulative_frames and updating the frame counter with some period
        const Uint32 FPS_UPDATE_FREQ_MS = 1000/SDL2_MAX_FRAMERATE;
        Uint32 time_t1 = SDL_GetTicks();
        //accumulative_frames++;
        if (time_t1 - time_t0 >= FPS_UPDATE_FREQ_MS) {
            accumulative_frames = 1;
            last_fps = (uint32_t) (accumulative_frames / ((time_t1 - time_t0) / 1000.0f));
            time_t0 = time_t1;
            accumulative_frames = 0;
            // Also update dt
            last_dt = Fix16(1.0f) / (Fix16(((int16_t) last_fps)));
            PC_ALLOW_RENDER = true;
        }   else {
            PC_ALLOW_RENDER = false;
            }
        sdl_debug_uint32_t(last_fps, 10, 10); // Support ONLY numbers 0-9!
#endif
        // // Limit Delta-time -> when it starts to be too high then car update loop
        // if(last_dt > 0.09f) last_dt = 0.09f;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~ Screen Flush ~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifdef PC
    if (PC_ALLOW_RENDER)
    {
#endif
        // Refershes screen and clears vram for new frame
        // 1. Refersh screen
        // 2. Clear VRAM for new frame
        renderer.screen_flush();
        car.clear_UI();

#ifdef PC
    } // (PC_ALLOW_RENDER)
#endif

    } // while(!done)

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~ End Program ~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef PC
    calcEnd(); //restore screen and do stuff
#else
    // End program without leaking memory
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
#endif
}
