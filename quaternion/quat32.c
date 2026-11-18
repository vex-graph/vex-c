// quat32.c — 32-bit float quaternion (4 x f32, 16 bytes).

#include "vex.h"
#include "quaternion/quaternion.h"

;;OVERVIEW
;;DEFINITION

// ---------------------------------------------------------------- accessors

;;GETTER
f32 quat32_x(quat32 self) { return self.x; }

;;SETTER
fn quat32_setX(quat32 *self, f32 value) { (*self).x = value; }

;;GETTER
f32 quat32_y(quat32 self) { return self.y; }

;;SETTER
fn quat32_setY(quat32 *self, f32 value) { (*self).y = value; }

;;GETTER
f32 quat32_z(quat32 self) { return self.z; }

;;SETTER
fn quat32_setZ(quat32 *self, f32 value) { (*self).z = value; }

;;GETTER
f32 quat32_w(quat32 self) { return self.w; }

;;SETTER
fn quat32_setW(quat32 *self, f32 value) { (*self).w = value; }

// ---------------------------------------------------------------- constructors

quat32 quat32_4(f32 x, f32 y, f32 z, f32 w) {
    quat32 q = { 0 };
    quat32_setX(&q, x);
    quat32_setY(&q, y);
    quat32_setZ(&q, z);
    quat32_setW(&q, w);
    return q;
}

quat32 quat32_0() {
    return quat32_4(0.0f, 0.0f, 0.0f, 1.0f);
}

quat32 quat32_identity() {
    return quat32_0();
}

quat32 quat32_fromAxisAngle(vec3f axis, f32 radians) {
    vec3f n = vec3f_normalize(axis);
    f32 half = radians * 0.5f;
    f32 s = sinf(half);
    f32 c = cosf(half);
    return quat32_4(vec3f_x(n) * s, vec3f_y(n) * s, vec3f_z(n) * s, c);
}

quat32 quat32_2(vec3f axis, f32 radians) {
    return quat32_fromAxisAngle(axis, radians);
}

// ---------------------------------------------------------------- operations

quat32 quat32_mul(quat32 a, quat32 b) {
    f32 ax = quat32_x(a), ay = quat32_y(a), az = quat32_z(a), aw = quat32_w(a);
    f32 bx = quat32_x(b), by = quat32_y(b), bz = quat32_z(b), bw = quat32_w(b);
    return quat32_4(
        aw * bx + ax * bw + ay * bz - az * by,
        aw * by - ax * bz + ay * bw + az * bx,
        aw * bz + ax * by - ay * bx + az * bw,
        aw * bw - ax * bx - ay * by - az * bz
    );
}

quat32 quat32_conjugate(quat32 self) {
    return quat32_4(-quat32_x(self), -quat32_y(self), -quat32_z(self), quat32_w(self));
}

f32 quat32_dot(quat32 a, quat32 b) {
    return quat32_x(a) * quat32_x(b)
         + quat32_y(a) * quat32_y(b)
         + quat32_z(a) * quat32_z(b)
         + quat32_w(a) * quat32_w(b);
}

f32 quat32_length(quat32 self) {
    return sqrtf(quat32_dot(self, self));
}

quat32 quat32_normalize(quat32 self) {
    f32 len = quat32_length(self);
    if (len == 0.0f) return quat32_0();
    f32 inv = 1.0f / len;
    return quat32_4(
        quat32_x(self) * inv,
        quat32_y(self) * inv,
        quat32_z(self) * inv,
        quat32_w(self) * inv
    );
}

quat32 quat32_inverse(quat32 self) {
    f32 d = quat32_dot(self, self);
    if (d == 0.0f) return quat32_0();
    f32 inv = 1.0f / d;
    quat32 c = quat32_conjugate(self);
    return quat32_4(
        quat32_x(c) * inv,
        quat32_y(c) * inv,
        quat32_z(c) * inv,
        quat32_w(c) * inv
    );
}

vec3f quat32_rotateVec3(quat32 self, vec3f v) {
    vec3f u = vec3f_3(quat32_x(self), quat32_y(self), quat32_z(self));
    f32 s = quat32_w(self);
    vec3f t = vec3f_scale(vec3f_cross(u, v), 2.0f);
    return vec3f_add(v, vec3f_add(vec3f_scale(t, s), vec3f_cross(u, t)));
}

quat32 quat32_slerp(quat32 a, quat32 b, f32 t) {
    f32 cosHalfTheta = quat32_dot(a, b);
    quat32 end = b;
    if (cosHalfTheta < 0.0f) {
        cosHalfTheta = -cosHalfTheta;
        end = quat32_4(-quat32_x(b), -quat32_y(b), -quat32_z(b), -quat32_w(b));
    }
    if (cosHalfTheta > 0.9995f) {
        quat32 lerped = quat32_4(
            quat32_x(a) + t * (quat32_x(end) - quat32_x(a)),
            quat32_y(a) + t * (quat32_y(end) - quat32_y(a)),
            quat32_z(a) + t * (quat32_z(end) - quat32_z(a)),
            quat32_w(a) + t * (quat32_w(end) - quat32_w(a))
        );
        return quat32_normalize(lerped);
    }
    f32 halfTheta = acosf(cosHalfTheta);
    f32 sinHalfTheta = sqrtf(1.0f - cosHalfTheta * cosHalfTheta);
    f32 wa = sinf((1.0f - t) * halfTheta) / sinHalfTheta;
    f32 wb = sinf(t * halfTheta) / sinHalfTheta;
    return quat32_4(
        quat32_x(a) * wa + quat32_x(end) * wb,
        quat32_y(a) * wa + quat32_y(end) * wb,
        quat32_z(a) * wa + quat32_z(end) * wb,
        quat32_w(a) * wa + quat32_w(end) * wb
    );
}

mat3f quat32_toMat3(quat32 self) {
    quat32 q = quat32_normalize(self);
    f32 x = quat32_x(q), y = quat32_y(q), z = quat32_z(q), w = quat32_w(q);
    f32 xx = x * x, yy = y * y, zz = z * z;
    f32 xy = x * y, xz = x * z, yz = y * z;
    f32 wx = w * x, wy = w * y, wz = w * z;

    vec3f c0 = vec3f_3(1.0f - 2.0f * (yy + zz), 2.0f * (xy + wz),        2.0f * (xz - wy));
    vec3f c1 = vec3f_3(2.0f * (xy - wz),        1.0f - 2.0f * (xx + zz), 2.0f * (yz + wx));
    vec3f c2 = vec3f_3(2.0f * (xz + wy),        2.0f * (yz - wx),        1.0f - 2.0f * (xx + yy));
    return mat3f_3(c0, c1, c2);
}

mat4f quat32_toMat4(quat32 self) {
    mat3f m3 = quat32_toMat3(self);
    vec3f r0 = mat3f_col(&m3, 0);
    vec3f r1 = mat3f_col(&m3, 1);
    vec3f r2 = mat3f_col(&m3, 2);
    return mat4f_4(
        vec4f_4(vec3f_x(r0), vec3f_y(r0), vec3f_z(r0), 0.0f),
        vec4f_4(vec3f_x(r1), vec3f_y(r1), vec3f_z(r1), 0.0f),
        vec4f_4(vec3f_x(r2), vec3f_y(r2), vec3f_z(r2), 0.0f),
        vec4f_4(0.0f,        0.0f,        0.0f,        1.0f)
    );
}

quat64 quat32_to_quat64(quat32 self) {
    return quat64_4(
        (d64) quat32_x(self),
        (d64) quat32_y(self),
        (d64) quat32_z(self),
        (d64) quat32_w(self)
    );
}
