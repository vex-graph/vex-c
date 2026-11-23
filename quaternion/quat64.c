/** quat64.c, 64-bit double quaternion (4 x d64, 32 bytes). */

#include "vex.h"
#include "quaternion/quaternion.h"

;;DEFINITION
/**
 * ============================================================================
 * DEFINITION: quat64
 * ============================================================================
 * 64-bit double quaternion (4 x d64, 32 bytes) owned by lib/quaternion for
 * high-precision 3D rotations, slerp interpolation, and mat3d/mat4d conversion.
 * ============================================================================
 */

;;OVERVIEW
/**
 * ============================================================================
 * CLASS: quat64 (lib/quaternion/quat64.c)
 * ============================================================================
 * 32-byte unit-rotation quaternion (x, y, z, w) in d64 precision.
 *
 * --FIELDS-------------------------------
 * public   d64          x, y, z, w        imaginary vector and real scalar
 *
 * --CONSTRUCTORS-------------------------
 * public   quat64_0 / quat64_2 / quat64_4 identity, axis-angle, explicit xyzw
 * public   quat64_identity / fromAxisAngle named identity and axis-angle builders
 *
 * --FUNCTIONS----------------------------
 * public   quat64_mul / conjugate / dot   Hamilton product, conjugate, dot product
 * public   quat64_length / normalize      norm and unit normalization
 * public   quat64_inverse / rotateVec3    multiplicative inverse and 3D rotation
 * public   quat64_slerp                   spherical linear interpolation
 * public   quat64_toMat3 / toMat4         rotation matrix projections
 * public   quat64_to_quat32               narrow to f32 quaternion
 *
 * --GETTERS------------------------------
 * public   quat64_x / y / z / w           component readers
 *
 * --SETTERS------------------------------
 * public   quat64_setX / setY / setZ / setW component writers
 * ============================================================================
 */

/** ---------------------------------------------------------------- accessors */

;;GETTER
d64 quat64_x(quat64 self)
{
    return self.x;
}

;;SETTER
fn quat64_setX(quat64 *self, d64 value)
{
    (*self).x = value;
}

;;GETTER
d64 quat64_y(quat64 self)
{
    return self.y;
}

;;SETTER
fn quat64_setY(quat64 *self, d64 value)
{
    (*self).y = value;
}

;;GETTER
d64 quat64_z(quat64 self)
{
    return self.z;
}

;;SETTER
fn quat64_setZ(quat64 *self, d64 value)
{
    (*self).z = value;
}

;;GETTER
d64 quat64_w(quat64 self)
{
    return self.w;
}

;;SETTER
fn quat64_setW(quat64 *self, d64 value)
{
    (*self).w = value;
}

/** ---------------------------------------------------------------- constructors */

quat64 quat64_4(d64 x, d64 y, d64 z, d64 w) {
    quat64 q = { 0 };
    quat64_setX(&q, x);
    quat64_setY(&q, y);
    quat64_setZ(&q, z);
    quat64_setW(&q, w);
    return q;
}

quat64 quat64_0() {
    return quat64_4(0.0, 0.0, 0.0, 1.0);
}

quat64 quat64_identity() {
    return quat64_0();
}

quat64 quat64_fromAxisAngle(vec3d axis, d64 radians) {
    vec3d n = vec3d_normalize(axis);
    d64 half = radians * 0.5;
    d64 s = sin(half);
    d64 c = cos(half);
    return quat64_4(vec3d_x(n) * s, vec3d_y(n) * s, vec3d_z(n) * s, c);
}

quat64 quat64_2(vec3d axis, d64 radians) {
    return quat64_fromAxisAngle(axis, radians);
}

/** ---------------------------------------------------------------- operations */

quat64 quat64_mul(quat64 a, quat64 b) {
    d64 ax = quat64_x(a), ay = quat64_y(a), az = quat64_z(a), aw = quat64_w(a);
    d64 bx = quat64_x(b), by = quat64_y(b), bz = quat64_z(b), bw = quat64_w(b);
    return quat64_4(
        aw * bx + ax * bw + ay * bz - az * by,
        aw * by - ax * bz + ay * bw + az * bx,
        aw * bz + ax * by - ay * bx + az * bw,
        aw * bw - ax * bx - ay * by - az * bz
    );
}

quat64 quat64_conjugate(quat64 self) {
    return quat64_4(-quat64_x(self), -quat64_y(self), -quat64_z(self), quat64_w(self));
}

d64 quat64_dot(quat64 a, quat64 b) {
    return quat64_x(a) * quat64_x(b)
         + quat64_y(a) * quat64_y(b)
         + quat64_z(a) * quat64_z(b)
         + quat64_w(a) * quat64_w(b);
}

d64 quat64_length(quat64 self) {
    return sqrt(quat64_dot(self, self));
}

quat64 quat64_normalize(quat64 self) {
    d64 len = quat64_length(self);
    if (len == 0.0)
        return quat64_0();
    d64 inv = 1.0 / len;
    return quat64_4(
        quat64_x(self) * inv,
        quat64_y(self) * inv,
        quat64_z(self) * inv,
        quat64_w(self) * inv
    );
}

quat64 quat64_inverse(quat64 self) {
    d64 d = quat64_dot(self, self);
    if (d == 0.0)
        return quat64_0();
    d64 inv = 1.0 / d;
    quat64 c = quat64_conjugate(self);
    return quat64_4(
        quat64_x(c) * inv,
        quat64_y(c) * inv,
        quat64_z(c) * inv,
        quat64_w(c) * inv
    );
}

vec3d quat64_rotateVec3(quat64 self, vec3d v) {
    vec3d u = vec3d_3(quat64_x(self), quat64_y(self), quat64_z(self));
    d64 s = quat64_w(self);
    vec3d t = vec3d_scale(vec3d_cross(u, v), 2.0);
    return vec3d_add(v, vec3d_add(vec3d_scale(t, s), vec3d_cross(u, t)));
}

quat64 quat64_slerp(quat64 a, quat64 b, d64 t) {
    d64 cosHalfTheta = quat64_dot(a, b);
    quat64 end = b;
    if (cosHalfTheta < 0.0) {
        cosHalfTheta = -cosHalfTheta;
        end = quat64_4(-quat64_x(b), -quat64_y(b), -quat64_z(b), -quat64_w(b));
    }
    if (cosHalfTheta > 0.999995) {
        quat64 lerped = quat64_4(
            quat64_x(a) + t * (quat64_x(end) - quat64_x(a)),
            quat64_y(a) + t * (quat64_y(end) - quat64_y(a)),
            quat64_z(a) + t * (quat64_z(end) - quat64_z(a)),
            quat64_w(a) + t * (quat64_w(end) - quat64_w(a))
        );
        return quat64_normalize(lerped);
    }
    d64 halfTheta = acos(cosHalfTheta);
    d64 sinHalfTheta = sqrt(1.0 - cosHalfTheta * cosHalfTheta);
    d64 wa = sin((1.0 - t) * halfTheta) / sinHalfTheta;
    d64 wb = sin(t * halfTheta) / sinHalfTheta;
    return quat64_4(
        quat64_x(a) * wa + quat64_x(end) * wb,
        quat64_y(a) * wa + quat64_y(end) * wb,
        quat64_z(a) * wa + quat64_z(end) * wb,
        quat64_w(a) * wa + quat64_w(end) * wb
    );
}

mat3d quat64_toMat3(quat64 self) {
    quat64 q = quat64_normalize(self);
    d64 x = quat64_x(q), y = quat64_y(q), z = quat64_z(q), w = quat64_w(q);
    d64 xx = x * x, yy = y * y, zz = z * z;
    d64 xy = x * y, xz = x * z, yz = y * z;
    d64 wx = w * x, wy = w * y, wz = w * z;

    vec3d c0 = vec3d_3(1.0 - 2.0 * (yy + zz), 2.0 * (xy + wz),       2.0 * (xz - wy));
    vec3d c1 = vec3d_3(2.0 * (xy - wz),       1.0 - 2.0 * (xx + zz), 2.0 * (yz + wx));
    vec3d c2 = vec3d_3(2.0 * (xz + wy),       2.0 * (yz - wx),       1.0 - 2.0 * (xx + yy));
    return mat3d_3(c0, c1, c2);
}

mat4d quat64_toMat4(quat64 self) {
    mat3d m3 = quat64_toMat3(self);
    vec3d r0 = mat3d_col(&m3, 0);
    vec3d r1 = mat3d_col(&m3, 1);
    vec3d r2 = mat3d_col(&m3, 2);
    return mat4d_4(
        vec4d_4(vec3d_x(r0), vec3d_y(r0), vec3d_z(r0), 0.0),
        vec4d_4(vec3d_x(r1), vec3d_y(r1), vec3d_z(r1), 0.0),
        vec4d_4(vec3d_x(r2), vec3d_y(r2), vec3d_z(r2), 0.0),
        vec4d_4(0.0,         0.0,         0.0,         1.0)
    );
}

quat32 quat64_to_quat32(quat64 self) {
    return quat32_4(
        (f32) quat64_x(self),
        (f32) quat64_y(self),
        (f32) quat64_z(self),
        (f32) quat64_w(self)
    );
}
