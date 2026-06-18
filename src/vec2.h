#ifndef VEC2_H
#define VEC2_H

typedef struct {
    float x;
    float y;
} Vec2;

void vec2_add_inplace(Vec2* target, const Vec2* vec_b);
Vec2 vec2_add(const Vec2* vec_a, const Vec2* vec_b);

void vec2_subtract_inplace(Vec2* target, const Vec2* vec_b);
Vec2 vec2_subtract(const Vec2* vec_a, const Vec2* vec_b);

void vec2_scale_inplace(Vec2* target, float scalar);
Vec2 vec2_scale(const Vec2* vec, float scalar);

float vec2_length(const Vec2* vec);
float vec2_distance(const Vec2* vec_a, const Vec2* vec_b);

float vec2_dot(const Vec2* vec_a, const Vec2* vec_b);

void vec2_normalize_inplace(Vec2* target);
Vec2 vec2_normalize(const Vec2* vec);

void vec2_negate_inplace(Vec2* target);
Vec2 vec2_negate(const Vec2* vec);

void vec2_zero(Vec2* target);

#endif // VEC2_H
