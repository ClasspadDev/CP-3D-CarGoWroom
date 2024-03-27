#include "Car.hpp"

#include "RenderUtils.hpp"

#ifndef PC
#   include <sdk/calc/calc.hpp>
#else
#   include <iostream>
#   include "PC_SDL_screen.hpp" // replaces "sdk/os/lcd.hpp"
#endif

Car::Car(/* args */)
:
    pos({0.0f, 0.0f}),
    vel({0.0f, 0.0f}),
    acc({0.0f, 0.0f}),
    rot({0.0f}),
    wheelNorm({0.0f}),
    speed({0.0f}),
    speed_perc({0.0f}),
    boostLeft({0.0f}),
    boostLeft_UI({0.0f}),
    using_boost(false)
{
}

Car::~Car()
{
}

fix16_vec2& Car::get_pos() {
    return pos;
}

Fix16& Car::get_rot()  {
    return rot;
}

Fix16& Car::get_speed() {
    return speed;
}
Fix16& Car::get_speed_perc() {
    return speed_perc;
}
Fix16& Car::get_wheelNorm() {
    return wheelNorm;
}

inline void drawSpeedIndicator(const Fix16 speed, const color_t colorr)
{
#ifdef LANDSCAPE_MODE
    const auto needle_width = 30.0f;
    const auto edge_offset = 20;
    const auto offset_x = (SCREEN_X/6);
    const auto offset_y = (SCREEN_Y) - (int) needle_width - edge_offset;
    const auto sinSpeed = ((speed/MAX_VELOCITY)*fix16_pi).sin();
    const auto cosSpeed = ((speed/MAX_VELOCITY)*fix16_pi).cos();
    const auto x1 = offset_x + (int16_t) (sinSpeed * needle_width);
    const auto y1 = offset_y - (int16_t) (cosSpeed * needle_width);
    const auto x2 = offset_x;
    const auto y2 = offset_y;
    line(x1, y1,   x2, y2,   colorr );
    line(x1, y1+1, x2, y2+1, colorr );
#else
    const auto needle_width = 30.0f;
    const auto edge_offset = 20;
    const auto offset_x = (SCREEN_X) - (int) needle_width - edge_offset;
    const auto offset_y = (SCREEN_Y*3/4);
    const auto sinSpeed = ((speed/MAX_VELOCITY)*fix16_pi).sin();
    const auto cosSpeed = ((speed/MAX_VELOCITY)*fix16_pi).cos();
    const auto x1 = offset_x - (int16_t) (cosSpeed * needle_width);
    const auto y1 = offset_y - (int16_t) (sinSpeed * needle_width);
    const auto x2 = offset_x;
    const auto y2 = offset_y;
    line(x1, y1,   x2, y2,   colorr );
    line(x1, y1+1, x2, y2+1, colorr );
#endif
}

inline void drawBoostIndicator(const Fix16 boostLeft, const color_t colorr)
{
#ifdef LANDSCAPE_MODE
    const auto needle_width = 80.0f;
    const auto edge_offset = 10;
    const auto offset_x = (SCREEN_X/6);
    const auto offset_y = (SCREEN_Y) - edge_offset;
    const auto x1 = offset_x + (int16_t) ((boostLeft/MAX_BOOST_TIME) * needle_width);
    const auto y1 = offset_y;
    const auto x2 = offset_x;
    const auto y2 = offset_y;
    const auto x1_full = offset_x + (int16_t) needle_width;
    const auto x2_full = offset_x;
    // Actual amount
    line( x1,  y1,       x2,   y2,   colorr );
    line( x1,  y1+1,     x2,   y2+1, colorr );
    // Border
    line( x1_full,  y1-2,     x2_full,   y2-2, color(0,0,125));
    line( x1_full,  y1+3,     x2_full,   y2+3, color(0,0,125));
#else
#   error "TODO BOOST INDICATOR PORTRAIT MODE"
#endif
}

void Car::clear_UI()
{
    drawSpeedIndicator(speed,        FILL_SCREEN_COLOR);
    drawBoostIndicator(boostLeft_UI, FILL_SCREEN_COLOR);
}

void Car::draw_UI()
{
    // Draw speed indicatior
    drawSpeedIndicator(speed,        color(178,0,0));
    drawBoostIndicator(boostLeft_UI, color(12,40,210));
}

void Car::add_boost(Fix16 boostTime)
{
    boostLeft += boostTime;
    if (boostLeft > MAX_BOOST_TIME)
        boostLeft = MAX_BOOST_TIME;
}

void Car::update(
    const Fix16 dt,
    bool accelerate,
    bool car_break,
    bool turn_left,
    bool turn_right,
    bool boost
) {

    speed_perc = speed / MAX_VELOCITY;
    const Fix16 speed_pn  = Fix16(1.0f) - speed_perc;

    const Fix16 speed_p2   = speed / (MAX_VELOCITY*6);
    const Fix16 speed_pn2  = Fix16(1.0f) - speed_p2;
    const Fix16 speed_pn2_pow2  = speed_pn2*speed_pn2;


    Fix16 very_low_speed = speed_perc / 0.18f;
    if (very_low_speed > 1.0f) very_low_speed = 1.0f;

    // Turning based on speed
    const float turn_alpha = 0.2f;
    const Fix16 turn_perc = speed_pn * Fix16(1.0f-turn_alpha) + Fix16(turn_alpha);

    if      (turn_left && speed > 0.0f)  {
        if (wheelNorm > 0.0f) wheelNorm -= (dt * MAX_TURN_SPEED) * turn_perc * very_low_speed * 4.0f;
        else                  wheelNorm -= (dt * MAX_TURN_SPEED) * turn_perc * very_low_speed;
        auto const wheelSpeedLimiting = very_low_speed*speed_pn2_pow2;
        if (wheelNorm < -wheelSpeedLimiting) wheelNorm = -wheelSpeedLimiting;
    }
    else if (turn_right && speed > 0.0f) {
        if (wheelNorm < 0.0f) wheelNorm += (dt * MAX_TURN_SPEED) * turn_perc * very_low_speed * 4.0f;
        else                  wheelNorm += (dt * MAX_TURN_SPEED) * turn_perc * very_low_speed;
        auto const wheelSpeedLimiting = very_low_speed*speed_pn2_pow2;
        if (wheelNorm > wheelSpeedLimiting) wheelNorm = wheelSpeedLimiting;
    }
    else {
        //wheelNorm += (-wheelNorm)*10.0f*dt;
        const Fix16 max_change_over_time = MAX_TURN_SPEED/5.0f;
        wheelNorm = easeInLinear(wheelNorm, Fix16(0.0f), dt, max_change_over_time*6.0f);
    }

    rot += (wheelNorm * MAX_TURN_ANGLE) * dt;

    // Turning affects also speed in real-life
    speed -= Fix16(fix16_abs(wheelNorm)) * dt * 2.0f;

    const Fix16 sin_rot = (rot).sin();
    const Fix16 cos_rot = (rot).cos();

    // Acceleration
    if(accelerate)
    {
        // speed
        Fix16 multiplier;
        if      (speed < 0.05f) multiplier = 0.30f;
        else if (speed < 0.15f) multiplier = 0.75f;
        else if (speed < 0.20f) multiplier = 1.00f;
        else if (speed < 0.30f) multiplier = 0.90f;
        else if (speed < 0.50f) multiplier = 0.80f;
        else if (speed < 0.65f) multiplier = 0.70f;
        else if (speed < 0.80f) multiplier = 0.60f;
        else                    multiplier = 0.20f;
        speed += dt * speed_pn * HORSEPOWER * multiplier;

    }

    // Boost
    if(boost || using_boost)
    {
        using_boost = true;
        if(boostLeft > 0.0f){
            boostLeft -= dt;
            if (boostLeft < 0.0f) {
                boostLeft = 0.0f;
            }
            speed += dt * HORSEPOWER/3.0f;
        } else {
            using_boost = false;
        }
    }
    // Break / Reversing
    if (car_break)
    {
        speed -= dt * BREAK_FRICTION;
        // Speed limiting
        if (speed < -MAX_REVERSE_VELOCITY) {
            speed = -MAX_REVERSE_VELOCITY;
        }
        // When breaking stop boost
        using_boost = false;
    }

    // Friction
    if (speed > 0.0f)
    {
        speed -= dt * VEL_FRICTION;

        if (speed < 0.0f){
            speed = 0.0f;
        }
        // Speed limiting
        if (speed > MAX_VELOCITY) {
            speed = MAX_VELOCITY;
        }
    }
    else {
        // Friction
        speed += dt * VEL_FRICTION;
    }

    // Position
    pos.x += sin_rot * dt * speed;
    pos.y += cos_rot * dt * speed;


    // Only smooth UI boost indicatior when we got more boost
    if(boostLeft > boostLeft_UI){
        boostLeft_UI = easeInLinear(boostLeft_UI, boostLeft, dt, 5.0f);
    } else {
        boostLeft_UI = boostLeft;
    }

}