#include "Fix16_Utils.hpp"

Fix16 easeInLinear(Fix16 currentValue, Fix16 targetValue, Fix16 deltaTime, Fix16 maxChangeOverTime) {
    // Calculate the change needed
    Fix16 change = targetValue - currentValue;
    // Calculate the change percentage based on the elapsed time
    Fix16 progress = deltaTime * maxChangeOverTime;
    // Apply linear interpolation
    Fix16 newValue = currentValue + change * progress;
    // Ensure newValue does not overshoot the targetValue
    if ((change > 0.0f && newValue > targetValue) || (change < 0.0f && newValue < targetValue))
        newValue = targetValue;
    return newValue;
}

Fix16 easeInLinearWithSlack(Fix16 currentValue, Fix16 targetValue, Fix16 slack, Fix16 deltaTime, Fix16 maxChangeOverTime) {
    // Calculate the change needed
    Fix16 change = targetValue - currentValue;
    if (Fix16(fix16_abs(change)) < slack){
        return currentValue;
    }
    // Calculate the change percentage based on the elapsed time
    Fix16 progress = deltaTime * maxChangeOverTime;
    // Apply linear interpolation
    Fix16 newValue = currentValue + change * progress;
    // Ensure newValue does not overshoot the targetValue
    if ((change > 0.0f && newValue > targetValue) || (change < 0.0f && newValue < targetValue))
        newValue = targetValue;
    return newValue;
}

Fix16 calculateDistance(const fix16_vec3& v1, const fix16_vec3& v2) {
    Fix16 dx = v2.x - v1.x;
    Fix16 dy = v2.y - v1.y;
    Fix16 dz = v2.z - v1.z;
    return fix16_sqrt(dx * dx + dy * dy + dz * dz);
}

Fix16 calculateLength(const fix16_vec3& v) {
    const auto x = v.x * v.x;
    const auto y = v.y * v.y;
    const auto z = v.z * v.z;
    return fix16_sqrt(x + y + z);
}

fix16_vec3 crossProduct(const fix16_vec3& a, const fix16_vec3& b)
{
    fix16_vec3 result;
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;
    return result;
}

fix16_vec3 sub_vec3(const fix16_vec3& a, const fix16_vec3& b)
{
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

Fix16 fix16_vec2_dot(fix16_vec2 a, fix16_vec2 b)
{
    return a.x * b.x + a.y * b.y;
}

fix16_vec3 calculateNormal(const fix16_vec3& v0, const fix16_vec3& v1, const fix16_vec3& v2)
{
    fix16_vec3 edge1 = {v1.x - v0.x, v1.y - v0.y, v1.z - v0.z};
    fix16_vec3 edge2 = {v2.x - v0.x, v2.y - v0.y, v2.z - v0.z};
    return crossProduct(edge1, edge2);
}

fix16_vec2 calculate2DForward(const fix16_vec2& rotation2D) {
    const Fix16 pitch = rotation2D.x;

    fix16_vec2 forward;
    forward.x = fix16_sin(pitch) ;
    forward.y = fix16_cos(pitch) ;

    return forward;
}

void normalize_fix16_vec3(fix16_vec3& vec)
{
    Fix16 length = fix16_sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
    vec.x /= length;
    vec.y /= length;
    vec.z /= length;
}

fix16_vec2 fix16_vec2_normalized(fix16_vec2& vec)
{
    Fix16 length = fix16_sqrt(vec.x * vec.x + vec.y * vec.y);
    return {vec.x / length, vec.y / length};
}
