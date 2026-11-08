#ifndef VEXC_VECTOR_H
#define VEXC_VECTOR_H

// vector.h — the vector value types of the floor.
//
// This is one of the three math headers: primitive.h (scalars), vector.h
// (vec2/vec3/vec4) and matrix.h (mat2/mat3/mat4). Nothing else in the floor
// carries a numeric shape.
//
// Layout rule — a vector is its lanes and nothing else:
//
//     vec2   2 lanes                     size = 2 * width, align = size
//     vec3   3 lanes + one zeroed _pad   size = 4 * width, align = size
//     vec4   4 lanes                     size = 4 * width, align = size
//
// A vec3 is a vec4 whose fourth lane is pinned to zero, so vec3f and vec4f
// have the same size, the same alignment and the same 128-bit load. The fourth
// lane is padding, never metadata: a coordinate frame belongs to a spatial type
// in a higher tier, not to a numeric primitive.
//
// Widths follow the scalar vocabulary: b16 (bfloat16), f32 (binary32), d64
// (binary64). b16 vectors are storage — bfloat16 has a 7-bit mantissa, so the
// arithmetic surface converts to f32 (vec3b_to_vec3f) and computes there.
//
// This is the include surface: types, layout assertions and declarations.
// The bodies live beside it, one shape per translation unit:
//
//     vector/vec2.c   vec2b, vec2f, vec2d
//     vector/vec3.c   vec3b, vec3f, vec3d
//     vector/vec4.c   vec4b, vec4f, vec4d
//
// Every function is declared by hand. There is no family macro and no token
// pasting: a reader sees the exact signature that ships.
//
//     vec3f v = vec3f(1.0f, 2.0f, 3.0f);
//     f32   x = vec3f_x(v);
//     vec3f_setX(&v, 9.0f);
//
// `data[]` is the deliberate SIMD seam and is the one member not reached
// through an accessor.

#include "../lang/annotation.h"
#include "../lang/constructor.h"
#include "../lang/primitive.h"

// ---------------------------------------------------------------- vec2

typedef struct vec2b {
    alignas(4) union {
        struct { b16 x, y; };
        b16 data[2];
    };
} vec2b;

typedef struct vec2f {
    alignas(8) union {
        struct { f32 x, y; };
        f32 data[2];
    };
} vec2f;

typedef struct vec2d {
    alignas(16) union {
        struct { d64 x, y; };
        d64 data[2];
    };
} vec2d;

// ---------------------------------------------------------------- vec3

typedef struct vec3b {
    alignas(8) union {
        struct { b16 x, y, z, _pad; };
        b16 data[4];
    };
} vec3b;

typedef struct vec3f {
    alignas(16) union {
        struct { f32 x, y, z, _pad; };
        f32 data[4];
    };
} vec3f;

typedef struct vec3d {
    alignas(32) union {
        struct { d64 x, y, z, _pad; };
        d64 data[4];
    };
} vec3d;

// ---------------------------------------------------------------- vec4

typedef struct vec4b {
    alignas(8) union {
        struct { b16 x, y, z, w; };
        b16 data[4];
    };
} vec4b;

typedef struct vec4f {
    alignas(16) union {
        struct { f32 x, y, z, w; };
        f32 data[4];
    };
} vec4f;

typedef struct vec4d {
    alignas(32) union {
        struct { d64 x, y, z, w; };
        d64 data[4];
    };
} vec4d;

// The layout rule, checked rather than described.

_Static_assert(sizeof(vec2b) == 4  && _Alignof(vec2b) == 4,  "vec2b: 2 b16 lanes");
_Static_assert(sizeof(vec2f) == 8  && _Alignof(vec2f) == 8,  "vec2f: 2 f32 lanes");
_Static_assert(sizeof(vec2d) == 16 && _Alignof(vec2d) == 16, "vec2d: 2 d64 lanes");

_Static_assert(sizeof(vec3b) == 8  && _Alignof(vec3b) == 8,  "vec3b: 4 b16 lanes, last zeroed");
_Static_assert(sizeof(vec3f) == 16 && _Alignof(vec3f) == 16, "vec3f: 4 f32 lanes, last zeroed");
_Static_assert(sizeof(vec3d) == 32 && _Alignof(vec3d) == 32, "vec3d: 4 d64 lanes, last zeroed");

_Static_assert(sizeof(vec4b) == 8  && _Alignof(vec4b) == 8,  "vec4b: 4 b16 lanes");
_Static_assert(sizeof(vec4f) == 16 && _Alignof(vec4f) == 16, "vec4f: 4 f32 lanes");
_Static_assert(sizeof(vec4d) == 32 && _Alignof(vec4d) == 32, "vec4d: 4 d64 lanes");

_Static_assert(sizeof(vec3f) == sizeof(vec4f), "a vec3f is a vec4f with lane 4 zeroed");
_Static_assert(sizeof(vec3d) == sizeof(vec4d), "a vec3d is a vec4d with lane 4 zeroed");

// ---------------------------------------------------------------- vec2 accessors

// One getter/setter pair per lane. The getter takes the value, so a read is
// `vec3f_x(v)` with no `&` and no pointer layer. The setter takes the pointer
// and is the only place the field is assigned.

;;GETTER
b16 vec2b_x(vec2b self);

;;SETTER
void vec2b_setX(vec2b *self, b16 value);

;;GETTER
b16 vec2b_y(vec2b self);

;;SETTER
void vec2b_setY(vec2b *self, b16 value);

;;GETTER
f32 vec2f_x(vec2f self);

;;SETTER
void vec2f_setX(vec2f *self, f32 value);

;;GETTER
f32 vec2f_y(vec2f self);

;;SETTER
void vec2f_setY(vec2f *self, f32 value);

;;GETTER
d64 vec2d_x(vec2d self);

;;SETTER
void vec2d_setX(vec2d *self, d64 value);

;;GETTER
d64 vec2d_y(vec2d self);

;;SETTER
void vec2d_setY(vec2d *self, d64 value);

// ---------------------------------------------------------------- vec3 accessors

;;GETTER
b16 vec3b_x(vec3b self);

;;SETTER
void vec3b_setX(vec3b *self, b16 value);

;;GETTER
b16 vec3b_y(vec3b self);

;;SETTER
void vec3b_setY(vec3b *self, b16 value);

;;GETTER
b16 vec3b_z(vec3b self);

;;SETTER
void vec3b_setZ(vec3b *self, b16 value);

;;GETTER
f32 vec3f_x(vec3f self);

;;SETTER
void vec3f_setX(vec3f *self, f32 value);

;;GETTER
f32 vec3f_y(vec3f self);

;;SETTER
void vec3f_setY(vec3f *self, f32 value);

;;GETTER
f32 vec3f_z(vec3f self);

;;SETTER
void vec3f_setZ(vec3f *self, f32 value);

;;GETTER
d64 vec3d_x(vec3d self);

;;SETTER
void vec3d_setX(vec3d *self, d64 value);

;;GETTER
d64 vec3d_y(vec3d self);

;;SETTER
void vec3d_setY(vec3d *self, d64 value);

;;GETTER
d64 vec3d_z(vec3d self);

;;SETTER
void vec3d_setZ(vec3d *self, d64 value);

// ---------------------------------------------------------------- vec4 accessors

;;GETTER
b16 vec4b_x(vec4b self);

;;SETTER
void vec4b_setX(vec4b *self, b16 value);

;;GETTER
b16 vec4b_y(vec4b self);

;;SETTER
void vec4b_setY(vec4b *self, b16 value);

;;GETTER
b16 vec4b_z(vec4b self);

;;SETTER
void vec4b_setZ(vec4b *self, b16 value);

;;GETTER
b16 vec4b_w(vec4b self);

;;SETTER
void vec4b_setW(vec4b *self, b16 value);

;;GETTER
f32 vec4f_x(vec4f self);

;;SETTER
void vec4f_setX(vec4f *self, f32 value);

;;GETTER
f32 vec4f_y(vec4f self);

;;SETTER
void vec4f_setY(vec4f *self, f32 value);

;;GETTER
f32 vec4f_z(vec4f self);

;;SETTER
void vec4f_setZ(vec4f *self, f32 value);

;;GETTER
f32 vec4f_w(vec4f self);

;;SETTER
void vec4f_setW(vec4f *self, f32 value);

;;GETTER
d64 vec4d_x(vec4d self);

;;SETTER
void vec4d_setX(vec4d *self, d64 value);

;;GETTER
d64 vec4d_y(vec4d self);

;;SETTER
void vec4d_setY(vec4d *self, d64 value);

;;GETTER
d64 vec4d_z(vec4d self);

;;SETTER
void vec4d_setZ(vec4d *self, d64 value);

;;GETTER
d64 vec4d_w(vec4d self);

;;SETTER
void vec4d_setW(vec4d *self, d64 value);

// ---------------------------------------------------------------- vec2 constructors

// Arity names follow the Constructor Dispatch Law: every accepted arity is a
// function, and the class-name macro after each family picks one.

vec2b vec2b_0();
vec2b vec2b_1(b16 scalar);
vec2b vec2b_2(b16 x, b16 y);

vec2f vec2f_0();
vec2f vec2f_1(f32 scalar);
vec2f vec2f_2(f32 x, f32 y);

vec2d vec2d_0();
vec2d vec2d_1(d64 scalar);
vec2d vec2d_2(d64 x, d64 y);

// ---------------------------------------------------------------- vec3 constructors

vec3b vec3b_0();
vec3b vec3b_1(b16 scalar);
vec3b vec3b_3(b16 x, b16 y, b16 z);

vec3f vec3f_0();
vec3f vec3f_1(f32 scalar);
vec3f vec3f_3(f32 x, f32 y, f32 z);

vec3d vec3d_0();
vec3d vec3d_1(d64 scalar);
vec3d vec3d_3(d64 x, d64 y, d64 z);

// ---------------------------------------------------------------- vec4 constructors

vec4b vec4b_0();
vec4b vec4b_1(b16 scalar);
vec4b vec4b_4(b16 x, b16 y, b16 z, b16 w);

vec4f vec4f_0();
vec4f vec4f_1(f32 scalar);
vec4f vec4f_4(f32 x, f32 y, f32 z, f32 w);

vec4d vec4d_0();
vec4d vec4d_1(d64 scalar);
vec4d vec4d_4(d64 x, d64 y, d64 z, d64 w);

// ---------------------------------------------------------------- arithmetic

// f32 and d64 only. bfloat16 is storage; its arithmetic surface is the
// conversion to vecNf below.

vec2f vec2f_add(vec2f a, vec2f b);
vec2f vec2f_sub(vec2f a, vec2f b);
vec2f vec2f_neg(vec2f a);
vec2f vec2f_scale(vec2f a, f32 scalar);
f32   vec2f_dot(vec2f a, vec2f b);
f32   vec2f_length(vec2f self);
vec2f vec2f_normalize(vec2f self);

vec2d vec2d_add(vec2d a, vec2d b);
vec2d vec2d_sub(vec2d a, vec2d b);
vec2d vec2d_neg(vec2d a);
vec2d vec2d_scale(vec2d a, d64 scalar);
d64   vec2d_dot(vec2d a, vec2d b);
d64   vec2d_length(vec2d self);
vec2d vec2d_normalize(vec2d self);

vec3f vec3f_add(vec3f a, vec3f b);
vec3f vec3f_sub(vec3f a, vec3f b);
vec3f vec3f_neg(vec3f a);
vec3f vec3f_scale(vec3f a, f32 scalar);
f32   vec3f_dot(vec3f a, vec3f b);
vec3f vec3f_cross(vec3f a, vec3f b);
f32   vec3f_length(vec3f self);
vec3f vec3f_normalize(vec3f self);

vec3d vec3d_add(vec3d a, vec3d b);
vec3d vec3d_sub(vec3d a, vec3d b);
vec3d vec3d_neg(vec3d a);
vec3d vec3d_scale(vec3d a, d64 scalar);
d64   vec3d_dot(vec3d a, vec3d b);
vec3d vec3d_cross(vec3d a, vec3d b);
d64   vec3d_length(vec3d self);
vec3d vec3d_normalize(vec3d self);

vec4f vec4f_add(vec4f a, vec4f b);
vec4f vec4f_sub(vec4f a, vec4f b);
vec4f vec4f_neg(vec4f a);
vec4f vec4f_scale(vec4f a, f32 scalar);
f32   vec4f_dot(vec4f a, vec4f b);
f32   vec4f_length(vec4f self);
vec4f vec4f_normalize(vec4f self);

vec4d vec4d_add(vec4d a, vec4d b);
vec4d vec4d_sub(vec4d a, vec4d b);
vec4d vec4d_neg(vec4d a);
vec4d vec4d_scale(vec4d a, d64 scalar);
d64   vec4d_dot(vec4d a, vec4d b);
d64   vec4d_length(vec4d self);
vec4d vec4d_normalize(vec4d self);

// ---------------------------------------------------------------- b16 bridge

vec2f vec2b_to_vec2f(vec2b a);
vec2b vec2b_from_vec2f(vec2f a);

vec3f vec3b_to_vec3f(vec3b a);
vec3b vec3b_from_vec3f(vec3f a);

vec4f vec4b_to_vec4f(vec4b a);
vec4b vec4b_from_vec4f(vec4f a);

// ---------------------------------------------------------------- dispatch

// The class name is the constructor. A function-like macro only expands when
// followed by `(`, so a declaration such as `vec3f v;` is untouched.

#define vec2b(...) CONSTRUCTOR_DISPATCH(vec2b, __VA_ARGS__)
#define vec2f(...) CONSTRUCTOR_DISPATCH(vec2f, __VA_ARGS__)
#define vec2d(...) CONSTRUCTOR_DISPATCH(vec2d, __VA_ARGS__)

#define vec3b(...) CONSTRUCTOR_DISPATCH(vec3b, __VA_ARGS__)
#define vec3f(...) CONSTRUCTOR_DISPATCH(vec3f, __VA_ARGS__)
#define vec3d(...) CONSTRUCTOR_DISPATCH(vec3d, __VA_ARGS__)

#define vec4b(...) CONSTRUCTOR_DISPATCH(vec4b, __VA_ARGS__)
#define vec4f(...) CONSTRUCTOR_DISPATCH(vec4f, __VA_ARGS__)
#define vec4d(...) CONSTRUCTOR_DISPATCH(vec4d, __VA_ARGS__)

#endif
