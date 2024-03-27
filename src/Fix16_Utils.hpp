#pragma once

#include "libfixmath/fix16.hpp"

struct int16_t_vec2
{
    int16_t x;
    int16_t y;
};

struct int16_t_Point2d
{
    int16_t x;
    int16_t y;
    int16_t u;
    int16_t v;
};

struct color8_vec
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct fix16_vec2
{
    Fix16 x;
    Fix16 y;
};

struct fix16_vec3
{
    Fix16 x;
    Fix16 y;
    Fix16 z;
};

struct uint_fix16_t
{
    unsigned int uint;
    Fix16        fix16;
};

Fix16 easeInLinear(Fix16 currentValue, Fix16 targetValue, Fix16 deltaTime, Fix16 maxChangeOverTime);
Fix16 easeInLinearWithSlack(Fix16 currentValue, Fix16 targetValue, Fix16 slack, Fix16 deltaTime, Fix16 maxChangeOverTime);
Fix16 calculateDistance(const fix16_vec3& v1, const fix16_vec3& v2);
Fix16 calculateLength(const fix16_vec3& v);
fix16_vec3 crossProduct(const fix16_vec3& a, const fix16_vec3& b);
fix16_vec3 sub_vec3(const fix16_vec3& a, const fix16_vec3& b);
fix16_vec3 calculateNormal(const fix16_vec3& v0, const fix16_vec3& v1, const fix16_vec3& v2);
fix16_vec2 calculate2DForward(const fix16_vec2& rotation2D);
void normalize_fix16_vec3(fix16_vec3& vec);
fix16_vec2 fix16_vec2_normalized(fix16_vec2& vec);
Fix16 fix16_vec2_dot(fix16_vec2 a, fix16_vec2 b);