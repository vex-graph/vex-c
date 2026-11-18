// quaternion_test.c — quat32 and quat64 layouts, operations and _Generic dispatch.

#include "vex.h"

_Static_assert(sizeof(quat32) == 16 && _Alignof(quat32) == 16, "quat32 is 16 bytes aligned to 16");
_Static_assert(sizeof(quat64) == 32 && _Alignof(quat64) == 32, "quat64 is 32 bytes aligned to 32");

static i32 failures;

static fn check(i32 ok, const char *what) {
    if (ok) return;
    fprintf(stderr, "quaternion_test: FAIL %s\n", what);
    failures++;
}

static i32 near_f32(f32 a, f32 b) { return fabsf(a - b) < 1e-5f; }
static i32 near_d64(d64 a, d64 b) { return fabs(a - b) < 1e-11; }

i32 main() {
    quat32 id32 = quat32();
    check(quat32_x(id32) == 0.0f && quat32_y(id32) == 0.0f &&
          quat32_z(id32) == 0.0f && quat32_w(id32) == 1.0f,
          "quat32() is identity (0,0,0,1)");

    quat64 id64 = quat64();
    check(quat64_x(id64) == 0.0 && quat64_y(id64) == 0.0 &&
          quat64_z(id64) == 0.0 && quat64_w(id64) == 1.0,
          "quat64() is identity (0,0,0,1)");

    // 90 deg rotation around Z axis rotates +X (1,0,0) to +Y (0,1,0)
    f32 halfPi32 = 1.57079632679f;
    quat32 qz32 = quat_fromAxisAngle(vec3f(0.0f, 0.0f, 1.0f), halfPi32);
    vec3f vx32 = vec3f(1.0f, 0.0f, 0.0f);
    vec3f vy32 = quat_rotateVec3(qz32, vx32);
    check(near_f32(vec3f_x(vy32), 0.0f) && near_f32(vec3f_y(vy32), 1.0f) && near_f32(vec3f_z(vy32), 0.0f),
          "quat32 90 deg around Z rotates +X to +Y");

    // Inverse rotates +Y back to +X
    quat32 inv32 = quat_inverse(qz32);
    vec3f back32 = quat_rotateVec3(inv32, vy32);
    check(near_f32(vec3f_x(back32), 1.0f) && near_f32(vec3f_y(back32), 0.0f) && near_f32(vec3f_z(back32), 0.0f),
          "quat_inverse reverses rotation");

    // Composition: two 90 deg rotations around Z = 180 deg (+X -> -X)
    quat32 q180 = quat_mul(qz32, qz32);
    vec3f vnx32 = quat_rotateVec3(q180, vx32);
    check(near_f32(vec3f_x(vnx32), -1.0f) && near_f32(vec3f_y(vnx32), 0.0f),
          "quat_mul composes two 90 deg Z rotations into 180 deg");

    // Slerp halfway between identity and 90 deg Z is 45 deg Z
    quat32 q45 = quat_slerp(id32, qz32, 0.5f);
    vec3f v45 = quat_rotateVec3(q45, vx32);
    check(near_f32(vec3f_x(v45), 0.70710678f) && near_f32(vec3f_y(v45), 0.70710678f),
          "quat_slerp halfway gives 45 deg rotation");

    // Matrix conversion matches rotateVec3
    mat3f m3 = quat_toMat3(qz32);
    vec3f vm3 = mat3f_multiplyVec(m3, vx32);
    check(near_f32(vec3f_x(vm3), vec3f_x(vy32)) && near_f32(vec3f_y(vm3), vec3f_y(vy32)),
          "quat_toMat3 matches quat_rotateVec3");

    // quat64 _Generic dispatch + precision
    d64 halfPi64 = 1.57079632679489661923;
    quat64 qz64 = quat_fromAxisAngle(vec3d(0.0, 0.0, 1.0), halfPi64);
    vec3d vy64 = quat_rotateVec3(qz64, vec3d(1.0, 0.0, 0.0));
    check(near_d64(vec3d_x(vy64), 0.0) && near_d64(vec3d_y(vy64), 1.0) && near_d64(vec3d_z(vy64), 0.0),
          "quat64 90 deg around Z rotates +X to +Y");

    // vec_* _Generic dispatch check
    vec3f sum3f = vec_add(vec3f(1.0f, 2.0f, 3.0f), vec3f(4.0f, 5.0f, 6.0f));
    check(vec3f_x(sum3f) == 5.0f && vec3f_y(sum3f) == 7.0f && vec3f_z(sum3f) == 9.0f,
          "vec_add _Generic routes vec3f");

    vec3d sum3d = vec_add(vec3d(1.0, 2.0, 3.0), vec3d(4.0, 5.0, 6.0));
    check(vec3d_x(sum3d) == 5.0 && vec3d_y(sum3d) == 7.0 && vec3d_z(sum3d) == 9.0,
          "vec_add _Generic routes vec3d");

    // if64 & ld128 arithmetic check
    if64 a64 = if64(10, 0.75f);
    if64 b64 = if64(5, 0.5f);
    if64 c64 = if64_add(a64, b64);
    check(if64_integer(c64) == 16 && near_f32(if64_fraction(c64), 0.25f),
          "if64_add carries across fraction >= 1.0");

    ld128 p1 = ld128(1000000000000LL, 0.25);
    ld128 p0 = ld128(1000000000000LL, 0.125);
    f32 rel = ld128_toF32Relative(p1, p0);
    check(near_f32(rel, 0.125f), "ld128_toF32Relative preserves sub-unit precision at 1e12");

    return failures ? 1 : 0;
}
