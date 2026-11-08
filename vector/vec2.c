// vec2.c — vec2b, vec2f and vec2d.
//
// One shape, three widths, one translation unit. Accessors, constructors,
// arithmetic, the b16 bridge and the cold text projection all live here;
// vector.h declares them and owns the layout.

#include <math.h>

#include "vector.h"

;;OVERVIEW
;;DEFINITION

// ---------------------------------------------------------------- accessors

;;GETTER
b16 vec2b_x(const vec2b self) { return self.x; }

;;SETTER
void vec2b_setX(vec2b *self, const b16 value) { (*self).x = value; }

;;GETTER
b16 vec2b_y(vec2b self) { return self.y; }

;;SETTER
void vec2b_setY(vec2b *self, b16 value) { (*self).y = value; }

;;GETTER
f32 vec2f_x(vec2f self) { return self.x; }

;;SETTER
void vec2f_setX(vec2f *self, f32 value) { (*self).x = value; }

;;GETTER
f32 vec2f_y(vec2f self) { return self.y; }

;;SETTER
void vec2f_setY(vec2f *self, f32 value) { (*self).y = value; }

;;GETTER
d64 vec2d_x(vec2d self) { return self.x; }

;;SETTER
void vec2d_setX(vec2d *self, d64 value) { (*self).x = value; }

;;GETTER
d64 vec2d_y(vec2d self) { return self.y; }

;;SETTER
void vec2d_setY(vec2d *self, d64 value) { (*self).y = value; }

// ---------------------------------------------------------------- constructors

vec2b vec2b_0() { return (vec2b) { 0 }; }

vec2b vec2b_1(b16 scalar) {
    vec2b v = vec2b_0();
    vec2b_setX(&v, scalar);
    vec2b_setY(&v, scalar);
    return v;
}

vec2b vec2b_2(b16 x, b16 y) {
    vec2b v = vec2b_0();
    vec2b_setX(&v, x);
    vec2b_setY(&v, y);
    return v;
}

vec2f vec2f_0() { return (vec2f) { 0 }; }

vec2f vec2f_1(f32 scalar) {
    vec2f v = vec2f_0();
    vec2f_setX(&v, scalar);
    vec2f_setY(&v, scalar);
    return v;
}

vec2f vec2f_2(f32 x, f32 y) {
    vec2f v = vec2f_0();
    vec2f_setX(&v, x);
    vec2f_setY(&v, y);
    return v;
}

vec2d vec2d_0() { return (vec2d) { 0 }; }

vec2d vec2d_1(d64 scalar) {
    vec2d v = vec2d_0();
    vec2d_setX(&v, scalar);
    vec2d_setY(&v, scalar);
    return v;
}

vec2d vec2d_2(d64 x, d64 y) {
    vec2d v = vec2d_0();
    vec2d_setX(&v, x);
    vec2d_setY(&v, y);
    return v;
}

// ---------------------------------------------------------------- arithmetic

vec2f vec2f_add(vec2f a, vec2f b) {
    return vec2f_2(vec2f_x(a) + vec2f_x(b), vec2f_y(a) + vec2f_y(b));
}

vec2f vec2f_sub(vec2f a, vec2f b) {
    return vec2f_2(vec2f_x(a) - vec2f_x(b), vec2f_y(a) - vec2f_y(b));
}

vec2f vec2f_neg(vec2f a) {
    return vec2f_2(-vec2f_x(a), -vec2f_y(a));
}

vec2f vec2f_scale(vec2f a, f32 scalar) {
    return vec2f_2(vec2f_x(a) * scalar, vec2f_y(a) * scalar);
}

f32 vec2f_dot(vec2f a, vec2f b) {
    return vec2f_x(a) * vec2f_x(b) + vec2f_y(a) * vec2f_y(b);
}

f32 vec2f_length(vec2f self) { return sqrtf(vec2f_dot(self, self)); }

vec2f vec2f_normalize(vec2f self) {
    f32 len = vec2f_length(self);
    if (len == 0.0f) return vec2f_0();
    return vec2f_scale(self, 1.0f / len);
}

vec2d vec2d_add(vec2d a, vec2d b) {
    return vec2d_2(vec2d_x(a) + vec2d_x(b), vec2d_y(a) + vec2d_y(b));
}

vec2d vec2d_sub(vec2d a, vec2d b) {
    return vec2d_2(vec2d_x(a) - vec2d_x(b), vec2d_y(a) - vec2d_y(b));
}

vec2d vec2d_neg(vec2d a) {
    return vec2d_2(-vec2d_x(a), -vec2d_y(a));
}

vec2d vec2d_scale(vec2d a, d64 scalar) {
    return vec2d_2(vec2d_x(a) * scalar, vec2d_y(a) * scalar);
}

d64 vec2d_dot(vec2d a, vec2d b) {
    return vec2d_x(a) * vec2d_x(b) + vec2d_y(a) * vec2d_y(b);
}

d64 vec2d_length(vec2d self) { return sqrt(vec2d_dot(self, self)); }

vec2d vec2d_normalize(vec2d self) {
    d64 len = vec2d_length(self);
    if (len == 0.0) return vec2d_0();
    return vec2d_scale(self, 1.0 / len);
}

// ---------------------------------------------------------------- b16 bridge

vec2f vec2b_to_vec2f(vec2b a) {
    return vec2f_2(b16_to_f32(vec2b_x(a)), b16_to_f32(vec2b_y(a)));
}

vec2b vec2b_from_vec2f(vec2f a) {
    vec2b v = vec2b_0();
    vec2b_setX(&v, b16_from_f32(vec2f_x(a)));
    vec2b_setY(&v, b16_from_f32(vec2f_y(a)));
    return v;
}