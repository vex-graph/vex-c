// vec3.c — vec3b, vec3f and vec3d.
//
// One shape, three widths, one translation unit. A vec3 carries a zeroed fourth
// lane; the constructors start from the zeroed value, so the lane stays zero.

#include "vex.h"
#include "vector/vector.h"

;;OVERVIEW
;;DEFINITION

// ---------------------------------------------------------------- accessors

;;GETTER
b16 vec3b_x(vec3b self) { return self.x; }

;;SETTER
fn vec3b_setX(vec3b *self, b16 value) { (*self).x = value; }

;;GETTER
b16 vec3b_y(vec3b self) { return self.y; }

;;SETTER
fn vec3b_setY(vec3b *self, b16 value) { (*self).y = value; }

;;GETTER
b16 vec3b_z(vec3b self) { return self.z; }

;;SETTER
fn vec3b_setZ(vec3b *self, b16 value) { (*self).z = value; }

;;GETTER
f32 vec3f_x(vec3f self) { return self.x; }

;;SETTER
fn vec3f_setX(vec3f *self, f32 value) { (*self).x = value; }

;;GETTER
f32 vec3f_y(vec3f self) { return self.y; }

;;SETTER
fn vec3f_setY(vec3f *self, f32 value) { (*self).y = value; }

;;GETTER
f32 vec3f_z(vec3f self) { return self.z; }

;;SETTER
fn vec3f_setZ(vec3f *self, f32 value) { (*self).z = value; }

;;GETTER
d64 vec3d_x(vec3d self) { return self.x; }

;;SETTER
fn vec3d_setX(vec3d *self, d64 value) { (*self).x = value; }

;;GETTER
d64 vec3d_y(vec3d self) { return self.y; }

;;SETTER
fn vec3d_setY(vec3d *self, d64 value) { (*self).y = value; }

;;GETTER
d64 vec3d_z(vec3d self) { return self.z; }

;;SETTER
fn vec3d_setZ(vec3d *self, d64 value) { (*self).z = value; }

// ---------------------------------------------------------------- constructors

vec3b vec3b_0() { return (vec3b) { 0 }; }

vec3b vec3b_1(b16 scalar) {
    vec3b v = vec3b_0();
    vec3b_setX(&v, scalar);
    vec3b_setY(&v, scalar);
    vec3b_setZ(&v, scalar);
    return v;
}

vec3b vec3b_3(b16 x, b16 y, b16 z) {
    vec3b v = vec3b_0();
    vec3b_setX(&v, x);
    vec3b_setY(&v, y);
    vec3b_setZ(&v, z);
    return v;
}

vec3f vec3f_0() { return (vec3f) { 0 }; }

vec3f vec3f_1(f32 scalar) {
    vec3f v = vec3f_0();
    vec3f_setX(&v, scalar);
    vec3f_setY(&v, scalar);
    vec3f_setZ(&v, scalar);
    return v;
}

vec3f vec3f_3(f32 x, f32 y, f32 z) {
    vec3f v = vec3f_0();
    vec3f_setX(&v, x);
    vec3f_setY(&v, y);
    vec3f_setZ(&v, z);
    return v;
}

vec3d vec3d_0() { return (vec3d) { 0 }; }

vec3d vec3d_1(d64 scalar) {
    vec3d v = vec3d_0();
    vec3d_setX(&v, scalar);
    vec3d_setY(&v, scalar);
    vec3d_setZ(&v, scalar);
    return v;
}

vec3d vec3d_3(d64 x, d64 y, d64 z) {
    vec3d v = vec3d_0();
    vec3d_setX(&v, x);
    vec3d_setY(&v, y);
    vec3d_setZ(&v, z);
    return v;
}

// ---------------------------------------------------------------- arithmetic

vec3f vec3f_add(vec3f a, vec3f b) {
    return vec3f_3(vec3f_x(a) + vec3f_x(b), vec3f_y(a) + vec3f_y(b),
                   vec3f_z(a) + vec3f_z(b));
}

vec3f vec3f_sub(vec3f a, vec3f b) {
    return vec3f_3(vec3f_x(a) - vec3f_x(b), vec3f_y(a) - vec3f_y(b),
                   vec3f_z(a) - vec3f_z(b));
}

vec3f vec3f_neg(vec3f a) {
    return vec3f_3(-vec3f_x(a), -vec3f_y(a), -vec3f_z(a));
}

vec3f vec3f_scale(vec3f a, f32 scalar) {
    return vec3f_3(vec3f_x(a) * scalar, vec3f_y(a) * scalar,
                   vec3f_z(a) * scalar);
}

f32 vec3f_dot(vec3f a, vec3f b) {
    return vec3f_x(a) * vec3f_x(b) + vec3f_y(a) * vec3f_y(b)
         + vec3f_z(a) * vec3f_z(b);
}

vec3f vec3f_cross(vec3f a, vec3f b) {
    return vec3f_3(vec3f_y(a) * vec3f_z(b) - vec3f_z(a) * vec3f_y(b),
                   vec3f_z(a) * vec3f_x(b) - vec3f_x(a) * vec3f_z(b),
                   vec3f_x(a) * vec3f_y(b) - vec3f_y(a) * vec3f_x(b));
}

f32 vec3f_length(vec3f self) { return sqrtf(vec3f_dot(self, self)); }

vec3f vec3f_normalize(vec3f self) {
    f32 len = vec3f_length(self);
    if (len == 0.0f) return vec3f_0();
    return vec3f_scale(self, 1.0f / len);
}

vec3d vec3d_add(vec3d a, vec3d b) {
    return vec3d_3(vec3d_x(a) + vec3d_x(b), vec3d_y(a) + vec3d_y(b),
                   vec3d_z(a) + vec3d_z(b));
}

vec3d vec3d_sub(vec3d a, vec3d b) {
    return vec3d_3(vec3d_x(a) - vec3d_x(b), vec3d_y(a) - vec3d_y(b),
                   vec3d_z(a) - vec3d_z(b));
}

vec3d vec3d_neg(vec3d a) {
    return vec3d_3(-vec3d_x(a), -vec3d_y(a), -vec3d_z(a));
}

vec3d vec3d_scale(vec3d a, d64 scalar) {
    return vec3d_3(vec3d_x(a) * scalar, vec3d_y(a) * scalar,
                   vec3d_z(a) * scalar);
}

d64 vec3d_dot(vec3d a, vec3d b) {
    return vec3d_x(a) * vec3d_x(b) + vec3d_y(a) * vec3d_y(b)
         + vec3d_z(a) * vec3d_z(b);
}

vec3d vec3d_cross(vec3d a, vec3d b) {
    return vec3d_3(vec3d_y(a) * vec3d_z(b) - vec3d_z(a) * vec3d_y(b),
                   vec3d_z(a) * vec3d_x(b) - vec3d_x(a) * vec3d_z(b),
                   vec3d_x(a) * vec3d_y(b) - vec3d_y(a) * vec3d_x(b));
}

d64 vec3d_length(vec3d self) { return sqrt(vec3d_dot(self, self)); }

vec3d vec3d_normalize(vec3d self) {
    d64 len = vec3d_length(self);
    if (len == 0.0) return vec3d_0();
    return vec3d_scale(self, 1.0 / len);
}

// ---------------------------------------------------------------- b16 bridge

vec3f vec3b_to_vec3f(vec3b a) {
    return vec3f_3(b16_to_f32(vec3b_x(a)), b16_to_f32(vec3b_y(a)),
                   b16_to_f32(vec3b_z(a)));
}

vec3b vec3b_from_vec3f(vec3f a) {
    vec3b v = vec3b_0();
    vec3b_setX(&v, b16_from_f32(vec3f_x(a)));
    vec3b_setY(&v, b16_from_f32(vec3f_y(a)));
    vec3b_setZ(&v, b16_from_f32(vec3f_z(a)));
    return v;
}

