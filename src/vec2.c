#include <math.h>
#include "vec2.h"

void vec2_add_inplace(Vec2* target, Vec2 vec_b) {
    target->x = target->x + vec_b.x;
    target->y = target->y + vec_b.y;
}

Vec2 vec2_add(Vec2 a, Vec2 b) {
    Vec2 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;

    return result;
}

void vec2_subtract_inplace(Vec2* target, Vec2 vec_b) {
    target->x = target->x - vec_b.x;
    target->y = target->y - vec_b.y;
}

Vec2 vec2_subtract(Vec2 a, Vec2 b) {
    Vec2 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;

    return result;
}

void vec2_scale_inplace(Vec2* target, double scalar) {
    target->x = target->x * scalar;
    target->y = target->y * scalar;
}

Vec2 vec2_scale(Vec2 vec, double scalar) {
    Vec2 result;
    result.x = vec.x * scalar;
    result.y = vec.y * scalar;

    return result;
}

double vec2_length(Vec2 vec) {
    return sqrt((vec.x * vec.x) + (vec.y * vec.y));
}

double vec2_dot(Vec2 a, Vec2 b) {
    return (a.x * b.x) + (a.y * b.y);
}

Vec2 vec2_normalize(Vec2 vec) {
    return vec2_scale(vec, 1.0 / vec2_length(vec));
}

void vec2_normalize_inplace(Vec2* target) {
    *target = vec2_scale(*target, 1.0 / vec2_length(*target));
}

double vec2_distance(Vec2 a, Vec2 b) {
    return vec2_length(vec2_subtract(a, b));
}

Vec2 vec2_negate(Vec2 vec) {
    Vec2 result;
    result.x = -vec.x;
    result.y = -vec.y;
    return result;
}

void vec2_negate_inplace(Vec2* target) {
    target->x = -target->x;
    target->y = -target->y;
}

void vec2_zero(Vec2* target) {
    target->x = 0.0;
    target->y = 0.0;
}
