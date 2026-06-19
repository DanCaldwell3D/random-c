#ifndef VEC2_H
#define VEC2_H

typedef struct {
    double x;
    double y;
} Vec2;

/** Add vec_b to target in-place: target += vec_b */
void vec2_add_inplace(Vec2* target, Vec2 vec_b);

/** Return a + b as a new Vec2 */
Vec2 vec2_add(Vec2 a, Vec2 b);

/** Subtract vec_b from target in-place: target -= vec_b */
void vec2_subtract_inplace(Vec2* target, Vec2 vec_b);

/** Return a - b as a new Vec2 */
Vec2 vec2_subtract(Vec2 a, Vec2 b);

/** Scale target by scalar in-place: target *= scalar */
void vec2_scale_inplace(Vec2* target, double scalar);

/** Return vec * scalar as a new Vec2 */
Vec2 vec2_scale(Vec2 vec, double scalar);

/** Return the Euclidean length (magnitude) of vec */
double vec2_length(Vec2 vec);

/** Return the Euclidean distance between a and b */
double vec2_distance(Vec2 a, Vec2 b);

/** Return the dot product of a and b */
double vec2_dot(Vec2 a, Vec2 b);

/** Normalize target in-place to unit length */
void vec2_normalize_inplace(Vec2* target);

/** Return vec normalized to unit length as a new Vec2 */
Vec2 vec2_normalize(Vec2 vec);

/** Negate target in-place: target = -target */
void vec2_negate_inplace(Vec2* target);

/** Return -vec as a new Vec2 */
Vec2 vec2_negate(Vec2 vec);

/** Zero out target: target = (0, 0) */
void vec2_zero(Vec2* target);

#endif // VEC2_H
