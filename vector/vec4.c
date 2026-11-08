// vec4.c — vec4b, vec4f and vec4d.
//
// One shape, three widths, one translation unit. A vec4 is the storage shape
// every other vector matches: vec3 differs only by pinning the fourth lane.

#include <math.h>

#include "vector.h"

;;OVERVIEW
;;DEFINITION

// ---------------------------------------------------------------- accessors

;;GETTER
b16 vec4b_x(vec4b self) { return self.x; }

;;SETTER
void vec4b_setX(vec4b *self, b16 value) { (*self).x = value; }

;;GETTER
b16 vec4b_y(vec4b self) { return self.y; }

;;SETTER
void vec4b_setY(vec4b *self, b16 value) { (*self).y = value; }

;;GETTER
b16 vec4b_z(vec4b self) { return self.z; }

;;SETTER
void vec4b_setZ(vec4b *self, b16 value) { (*self).z = value; }

;;GETTER
b16 vec4b_w(vec4b self) { return self.w; }

;;SETTER
void vec4b_setW(vec4b *self, b16 value) { (*self).w = value; }

;;GETTER
f32 vec4f_x(vec4f self) { return self.x; }

;;SETTER
void vec4f_setX(vec4f *self, f32 value) { (*self).x = value; }

;;GETTER
f32 vec4f_y(vec4f self) { return self.y; }

;;SETTER
void vec4f_setY(vec4f *self, f32 value) { (*self).y = value; }

;;GETTER
f32 vec4f_z(vec4f self) { return self.z; }

;;SETTER
void vec4f_setZ(vec4f *self, f32 value) { (*self).z = value; }

;;GETTER
f32 vec4f_w(vec4f self) { return self.w; }

;;SETTER
void vec4f_setW(vec4f *self, f32 value) { (*self).w = value; }

;;GETTER
d64 vec4d_x(vec4d self) { return self.x; }

;;SETTER
void vec4d_setX(vec4d *self, d64 value) { (*self).x = value; }

;;GETTER
d64 vec4d_y(vec4d self) { return self.y; }

;;SETTER
void vec4d_setY(vec4d *self, d64 value) { (*self).y = value; }

;;GETTER
d64 vec4d_z(vec4d self) { return self.z; }

;;SETTER
void vec4d_setZ(vec4d *self, d64 value) { (*self).z = value; }

;;GETTER
d64 vec4d_w(vec4d self) { return self.w; }

;;SETTER
void vec4d_setW(vec4d *self, d64 value) { (*self).w = value; }

// ---------------------------------------------------------------- constructors

vec4b vec4b_0() { return (vec4b) { 0 }; }

vec4b vec4b_1(b16 scalar) {
    vec4b v = vec4b_0();
    vec4b_setX(&v, scalar);
    vec4b_setY(&v, scalar);
    vec4b_setZ(&v, scalar);
    vec4b_setW(&v, scalar);
    return v;
}

vec4b vec4b_4(b16 x, b16 y, b16 z, b16 w) {
    vec4b v = vec4b_0();
    vec4b_setX(&v, x);
    vec4b_setY(&v, y);
    vec4b_setZ(&v, z);
    vec4b_setW(&v, w);
    return v;
}

vec4f vec4f_0() { return (vec4f) { 0 }; }

vec4f vec4f_1(f32 scalar) {
    vec4f v = vec4f_0();
    vec4f_setX(&v, scalar);
    vec4f_setY(&v, scalar);
    vec4f_setZ(&v, scalar);
    vec4f_setW(&v, scalar);
    return v;
}

vec4f vec4f_4(f32 x, f32 y, f32 z, f32 w) {
    vec4f v = vec4f_0();
    vec4f_setX(&v, x);
    vec4f_setY(&v, y);
    vec4f_setZ(&v, z);
    vec4f_setW(&v, w);
    return v;
}

vec4d vec4d_0() { return (vec4d) { 0 }; }

vec4d vec4d_1(d64 scalar) {
    vec4d v = vec4d_0();
    vec4d_setX(&v, scalar);
    vec4d_setY(&v, scalar);
    vec4d_setZ(&v, scalar);
    vec4d_setW(&v, scalar);
    return v;
}

vec4d vec4d_4(d64 x, d64 y, d64 z, d64 w) {
    vec4d v = vec4d_0();
    vec4d_setX(&v, x);
    vec4d_setY(&v, y);
    vec4d_setZ(&v, z);
    vec4d_setW(&v, w);
    return v;
}

// ---------------------------------------------------------------- arithmetic

vec4f vec4f_add(vec4f a, vec4f b) {
    return vec4f_4(vec4f_x(a) + vec4f_x(b), vec4f_y(a) + vec4f_y(b),
                   vec4f_z(a) + vec4f_z(b), vec4f_w(a) + vec4f_w(b));
}

vec4f vec4f_sub(vec4f a, vec4f b) {
    return vec4f_4(vec4f_x(a) - vec4f_x(b), vec4f_y(a) - vec4f_y(b),
                   vec4f_z(a) - vec4f_z(b), vec4f_w(a) - vec4f_w(b));
}

vec4f vec4f_neg(vec4f a) {
    return vec4f_4(-vec4f_x(a), -vec4f_y(a), -vec4f_z(a), -vec4f_w(a));
}

vec4f vec4f_scale(vec4f a, f32 scalar) {
    return vec4f_4(vec4f_x(a) * scalar, vec4f_y(a) * scalar,
                   vec4f_z(a) * scalar, vec4f_w(a) * scalar);
}

f32 vec4f_dot(vec4f a, vec4f b) {
    return vec4f_x(a) * vec4f_x(b) + vec4f_y(a) * vec4f_y(b)
         + vec4f_z(a) * vec4f_z(b) + vec4f_w(a) * vec4f_w(b);
}

f32 vec4f_length(vec4f self) { return sqrtf(vec4f_dot(self, self)); }

vec4f vec4f_normalize(vec4f self) {
    f32 len = vec4f_length(self);
    if (len == 0.0f) return vec4f_0();
    return vec4f_scale(self, 1.0f / len);
}

vec4d vec4d_add(vec4d a, vec4d b) {
    return vec4d_4(vec4d_x(a) + vec4d_x(b), vec4d_y(a) + vec4d_y(b),
                   vec4d_z(a) + vec4d_z(b), vec4d_w(a) + vec4d_w(b));
}

vec4d vec4d_sub(vec4d a, vec4d b) {
    return vec4d_4(vec4d_x(a) - vec4d_x(b), vec4d_y(a) - vec4d_y(b),
                   vec4d_z(a) - vec4d_z(b), vec4d_w(a) - vec4d_w(b));
}

vec4d vec4d_neg(vec4d a) {
    return vec4d_4(-vec4d_x(a), -vec4d_y(a), -vec4d_z(a), -vec4d_w(a));
}

vec4d vec4d_scale(vec4d a, d64 scalar) {
    return vec4d_4(vec4d_x(a) * scalar, vec4d_y(a) * scalar,
                   vec4d_z(a) * scalar, vec4d_w(a) * scalar);
}

d64 vec4d_dot(vec4d a, vec4d b) {
    return vec4d_x(a) * vec4d_x(b) + vec4d_y(a) * vec4d_y(b)
         + vec4d_z(a) * vec4d_z(b) + vec4d_w(a) * vec4d_w(b);
}

d64 vec4d_length(vec4d self) { return sqrt(vec4d_dot(self, self)); }

vec4d vec4d_normalize(vec4d self) {
    d64 len = vec4d_length(self);
    if (len == 0.0) return vec4d_0();
    return vec4d_scale(self, 1.0 / len);
}

// ---------------------------------------------------------------- b16 bridge

vec4f vec4b_to_vec4f(vec4b a) {
    return vec4f_4(b16_to_f32(vec4b_x(a)), b16_to_f32(vec4b_y(a)),
                   b16_to_f32(vec4b_z(a)), b16_to_f32(vec4b_w(a)));
}

vec4b vec4b_from_vec4f(vec4f a) {
    vec4b v = vec4b_0();
    vec4b_setX(&v, b16_from_f32(vec4f_x(a)));
    vec4b_setY(&v, b16_from_f32(vec4f_y(a)));
    vec4b_setZ(&v, b16_from_f32(vec4f_z(a)));
    vec4b_setW(&v, b16_from_f32(vec4f_w(a)));
    return v;
}

