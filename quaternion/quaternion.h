#ifndef VEXC_QUATERNION_H
#define VEXC_QUATERNION_H

// quaternion.h — the quaternion rotation value types of the floor.
//
// The fourth math header: primitive.h (scalars), vector.h (vec2/vec3/vec4),
// matrix.h (mat2/mat3/mat4) and quaternion.h (quat32/quat64).
//
// Layout rule — a quaternion is four lanes (x, y, z, w) where (x, y, z) is the
// vector part and w is the scalar part. Identity is (0, 0, 0, 1):
//
//     quat32   4 f32 lanes   size = 16, align = 16
//     quat64   4 d64 lanes   size = 32, align = 32
//
// `data[]` is the deliberate SIMD seam and is the one member not reached
// through an accessor.
//
// Bodies live beside this header, one translation unit per width:
//
//     quaternion/quat32.c   quat32
//     quaternion/quat64.c   quat64

#include "matrix/matrix.h"

// ---------------------------------------------------------------- types

typedef struct quat32 {
    alignas(16) union {
        struct { f32 x, y, z, w; };
        f32 data[4];
    };
} quat32;

typedef struct quat64 {
    alignas(32) union {
        struct { d64 x, y, z, w; };
        d64 data[4];
    };
} quat64;

_Static_assert(sizeof(quat32) == 16 && _Alignof(quat32) == 16, "quat32: 4 f32 lanes");
_Static_assert(sizeof(quat64) == 32 && _Alignof(quat64) == 32, "quat64: 4 d64 lanes");

// ---------------------------------------------------------------- accessors

;;GETTER
f32 quat32_x(quat32 self);

;;SETTER
fn quat32_setX(quat32 *self, f32 value);

;;GETTER
f32 quat32_y(quat32 self);

;;SETTER
fn quat32_setY(quat32 *self, f32 value);

;;GETTER
f32 quat32_z(quat32 self);

;;SETTER
fn quat32_setZ(quat32 *self, f32 value);

;;GETTER
f32 quat32_w(quat32 self);

;;SETTER
fn quat32_setW(quat32 *self, f32 value);

;;GETTER
d64 quat64_x(quat64 self);

;;SETTER
fn quat64_setX(quat64 *self, d64 value);

;;GETTER
d64 quat64_y(quat64 self);

;;SETTER
fn quat64_setY(quat64 *self, d64 value);

;;GETTER
d64 quat64_z(quat64 self);

;;SETTER
fn quat64_setZ(quat64 *self, d64 value);

;;GETTER
d64 quat64_w(quat64 self);

;;SETTER
fn quat64_setW(quat64 *self, d64 value);

// ---------------------------------------------------------------- constructors

// 0-arg returns identity (0, 0, 0, 1).
// 2-arg takes (axis, radians).
// 4-arg takes (x, y, z, w).

quat32 quat32_0();
quat32 quat32_2(vec3f axis, f32 radians);
quat32 quat32_4(f32 x, f32 y, f32 z, f32 w);
quat32 quat32_identity();
quat32 quat32_fromAxisAngle(vec3f axis, f32 radians);

quat64 quat64_0();
quat64 quat64_2(vec3d axis, d64 radians);
quat64 quat64_4(d64 x, d64 y, d64 z, d64 w);
quat64 quat64_identity();
quat64 quat64_fromAxisAngle(vec3d axis, d64 radians);

// ---------------------------------------------------------------- operations

quat32 quat32_mul(quat32 a, quat32 b);
quat32 quat32_conjugate(quat32 self);
quat32 quat32_inverse(quat32 self);
f32    quat32_dot(quat32 a, quat32 b);
f32    quat32_length(quat32 self);
quat32 quat32_normalize(quat32 self);
vec3f  quat32_rotateVec3(quat32 self, vec3f v);
quat32 quat32_slerp(quat32 a, quat32 b, f32 t);
mat3f  quat32_toMat3(quat32 self);
mat4f  quat32_toMat4(quat32 self);

quat64 quat64_mul(quat64 a, quat64 b);
quat64 quat64_conjugate(quat64 self);
quat64 quat64_inverse(quat64 self);
d64    quat64_dot(quat64 a, quat64 b);
d64    quat64_length(quat64 self);
quat64 quat64_normalize(quat64 self);
vec3d  quat64_rotateVec3(quat64 self, vec3d v);
quat64 quat64_slerp(quat64 a, quat64 b, d64 t);
mat3d  quat64_toMat3(quat64 self);
mat4d  quat64_toMat4(quat64 self);

// ---------------------------------------------------------------- conversion

quat64 quat32_to_quat64(quat32 self);
quat32 quat64_to_quat32(quat64 self);

// ---------------------------------------------------------------- dispatch

#define quat32(...) CONSTRUCTOR_DISPATCH(quat32, __VA_ARGS__)
#define quat64(...) CONSTRUCTOR_DISPATCH(quat64, __VA_ARGS__)

#define quat_fromAxisAngle(axis, radians) _Generic((axis), \
    vec3f: quat32_fromAxisAngle, \
    vec3d: quat64_fromAxisAngle  \
)(axis, radians)

#define quat_mul(a, b) _Generic((a), \
    quat32: quat32_mul, \
    quat64: quat64_mul  \
)(a, b)

#define quat_conjugate(q) _Generic((q), \
    quat32: quat32_conjugate, \
    quat64: quat64_conjugate  \
)(q)

#define quat_inverse(q) _Generic((q), \
    quat32: quat32_inverse, \
    quat64: quat64_inverse  \
)(q)

#define quat_dot(a, b) _Generic((a), \
    quat32: quat32_dot, \
    quat64: quat64_dot  \
)(a, b)

#define quat_length(q) _Generic((q), \
    quat32: quat32_length, \
    quat64: quat64_length  \
)(q)

#define quat_normalize(q) _Generic((q), \
    quat32: quat32_normalize, \
    quat64: quat64_normalize  \
)(q)

#define quat_rotateVec3(q, v) _Generic((q), \
    quat32: quat32_rotateVec3, \
    quat64: quat64_rotateVec3  \
)(q, v)

#define quat_slerp(a, b, t) _Generic((a), \
    quat32: quat32_slerp, \
    quat64: quat64_slerp  \
)(a, b, t)

#define quat_toMat3(q) _Generic((q), \
    quat32: quat32_toMat3, \
    quat64: quat64_toMat3  \
)(q)

#define quat_toMat4(q) _Generic((q), \
    quat32: quat32_toMat4, \
    quat64: quat64_toMat4  \
)(q)

#endif
