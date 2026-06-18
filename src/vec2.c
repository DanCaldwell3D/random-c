#include <math.h>
#include "vec2.h"

/** Add vec_b to target in-place: target += vec_b */
void vec2_add_inplace(Vec2* target, const Vec2* vec_b) {
    target->x = target->x + vec_b->x;
    target->y = target->y + vec_b->y;
}

/** Return vec_a + vec_b as a new Vec2 */
Vec2 vec2_add(const Vec2* vec_a, const Vec2* vec_b) {
    Vec2 result;
    result.x = vec_a->x + vec_b->x;
    result.y = vec_a->y + vec_b->y;

    return result;
}

/** Subtract vec_b from target in-place: target -= vec_b */
void vec2_subtract_inplace(Vec2* target, const Vec2* vec_b) {
    target->x = target->x - vec_b->x;
    target->y = target->y - vec_b->y;
}

/** Return vec_a - vec_b as a new Vec2 */
Vec2 vec2_subtract(const Vec2* vec_a, const Vec2* vec_b) {
    Vec2 result;
    result.x = vec_a->x - vec_b->x;
    result.y = vec_a->y - vec_b->y;

    return result;
}

/** Scale target by scalar in-place: target *= scalar */
void vec2_scale_inplace(Vec2* target, float scalar) {
    target->x = target->x * scalar;
    target->y = target->y * scalar;
}

/** Return vec * scalar as a new Vec2 */
Vec2 vec2_scale(const Vec2* vec, float scalar) {
    Vec2 result;
    result.x = vec->x * scalar;
    result.y = vec->y * scalar;

    return result;
}

/** Return the Euclidean length (magnitude) of vec */
float vec2_length(const Vec2* vec) {
    const float vec_length = sqrtf((vec->x * vec->x) + (vec->y * vec->y));

    return vec_length;
}

/** Return the dot product of vec_a and vec_b: a.x*b.x + a.y*b.y */
float vec2_dot(const Vec2* vec_a, const Vec2* vec_b) {
    const float dot_product = (vec_a->x * vec_b->x) + (vec_a->y * vec_b->y);

    return dot_product;
}

/** Return vec normalized to unit length as a new Vec2 */
Vec2 vec2_normalize(const Vec2* vec) {
    Vec2 result;

    const float scale_factor = 1.0 / vec2_length(vec);
    result = vec2_scale(vec, scale_factor);

    return result;
}

/** Normalize target in-place to unit length */
void vec2_normalize_inplace(Vec2* target) {
    const float scale_factor = 1.0 / vec2_length(target);
    vec2_scale_inplace(target, scale_factor);
}

/** Return the Euclidean distance between vec_a and vec_b */
float vec2_distance(const Vec2* vec_a, const Vec2* vec_b) {
    Vec2 temp = vec2_subtract(vec_a, vec_b);
    float distance = vec2_length(&temp);

    return distance;
}

/** Return -vec as a new Vec2 */
Vec2 vec2_negate(const Vec2* vec) {
    Vec2 result;

    result.x = -vec->x;
    result.y = -vec->y;

    return result;
}

/** Negate target in-place: target = -target */
void vec2_negate_inplace(Vec2* target) {
    target->x = -target->x;
    target->y = -target->y;
}

/** Zero out target: target = (0, 0) */
void vec2_zero(Vec2* target) {
    target->x = 0.0;
    target->y = 0.0;
}
