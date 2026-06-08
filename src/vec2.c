#include <math.h>

typedef struct {
    float x;
    float y;
} Vec2;

void vec2_add_inplace(Vec2* vec_a, const Vec2* vec_b) {
    vec_a->x = vec_a->x + vec_b->x;
    vec_a->y = vec_a->y + vec_b->y;
}

Vec2 vec2_add(const Vec2* vec_a, const Vec2* vec_b) {
    Vec2 result;
    result.x = vec_a->x + vec_b->x;
    result.y = vec_a->y + vec_b->y;

    return result;
}

void vec2_subtract_inplace(Vec2* vec_a, const Vec2* vec_b) {
    vec_a->x = vec_a->x - vec_b->x;
    vec_a->y = vec_a->y - vec_b->y;
}

Vec2 vec2_subtract(const Vec2* vec_a, const Vec2* vec_b) {
    Vec2 result;
    result.x = vec_a->x - vec_b->x;
    result.y = vec_a->y - vec_b->y;

    return result;
}

void vec2_scale_inplace(Vec2* vec, const float scalar) {
    vec->x = vec->x * scalar;
    vec->y = vec->y * scalar;
}

Vec2 vec2_scale(const Vec2* vec, const float scalar) {
    Vec2 result;
    result.x = vec->x * scalar;
    result.y = vec->y * scalar;

    return result;
}

float vec2_length(const Vec2* vec) {
    const float vec_length = sqrtf((vec->x * vec->x) + (vec->y * vec->y));

    return vec_length;
}

float vec2_dotproduct(const Vec2* vec_a, const Vec2* vec_b) {
    const float dot_product = (vec_a->x * vec_b->x) + (vec_a->y * vec_b->y);

    return dot_product;
}