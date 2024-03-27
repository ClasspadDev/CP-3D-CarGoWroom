#pragma once

#include "Fix16_Utils.hpp"

#include "Renderer.hpp"

// Car turn Radians/sec
#define MAX_TURN_ANGLE     1.9f
// Wheen turn Radians/sec
#define MAX_TURN_SPEED    60.0f
// Faster acceleration
#define HORSEPOWER        80.0f
#define MAX_VELOCITY      90.0f
#define MAX_REVERSE_VELOCITY 6.0f

#define MAX_BOOST_TIME     3.0f

#define VEL_FRICTION       4.0f
#define BREAK_FRICTION    40.0f

// Actual car logic is in 2d, just the graphics is in 3D

class Car
{
private:

    fix16_vec2 pos;
    fix16_vec2 vel;
    fix16_vec2 acc;

    Fix16 rot;

    Fix16 wheelNorm;
    Fix16 speed;
    Fix16 speed_perc;

    Fix16 boostLeft;
    Fix16 boostLeft_UI; // This is smoothed UI version

    bool using_boost;

public:

    void update(
        const Fix16 dt,
        bool accelerate,
        bool car_break,
        bool turn_left,
        bool turn_right,
        bool boost
    );

    void clear_UI();
    void draw_UI();

    fix16_vec2& get_pos();
    Fix16& get_rot();
    Fix16& get_speed();
    Fix16& get_speed_perc();
    Fix16& get_wheelNorm();

    void add_boost(Fix16 boostTime);

    Car(/* args */);
    ~Car();
};
