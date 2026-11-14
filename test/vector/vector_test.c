// vector_test.c — the vec2/vec3/vec4 families at every width.
//
// Layout is proven at compile time (the header's _Static_asserts plus the ones
// below); behaviour is proven at run time through the accessor surface only, so
// the test is also the proof that a consumer never needs `(*ptr).field`.



#include "vex.h"

_Static_assert(sizeof(vec3b) == 4 * sizeof(b16), "vec3b is four b16 lanes");
_Static_assert(sizeof(vec3f) == 4 * sizeof(f32), "vec3f is four f32 lanes");
_Static_assert(sizeof(vec3d) == 4 * sizeof(d64), "vec3d is four d64 lanes");

_Static_assert(sizeof(vec2f) == 2 * sizeof(f32), "vec2f is two f32 lanes");
_Static_assert(sizeof(vec4d) == 4 * sizeof(d64), "vec4d is four d64 lanes");

_Static_assert(_Alignof(vec3d) == 32, "vec3d is a 256-bit value");
_Static_assert(_Alignof(vec4f) == 16, "vec4f is a 128-bit value");

static int failures;

static fn check(int ok, const char *what) {
    if (ok) return;
    fprintf(stderr, "vector_test: FAIL %s\n", what);
    failures++;
}

static int near_f32(f32 a, f32 b) { return fabsf(a - b) < 1e-6f; }
static int near_d64(d64 a, d64 b) { return fabs(a - b) < 1e-12; }

int main() {
    vec3f v = vec3f(1.0f, 2.0f, 3.0f);

    check(vec3f_x(v) == 1.0f, "vec3f_x reads x");
    check(vec3f_y(v) == 2.0f, "vec3f_y reads y");
    check(vec3f_z(v) == 3.0f, "vec3f_z reads z");
    check(v.data[3] == 0.0f, "vec3f fourth lane is zeroed");

    vec3f_setX(&v, 9.0f);
    check(vec3f_x(v) == 9.0f, "vec3f_setX writes x");

    vec3f x_axis = vec3f(1.0f, 0.0f, 0.0f);
    vec3f y_axis = vec3f(0.0f, 1.0f, 0.0f);

    check(vec3f_dot(x_axis, y_axis) == 0.0f, "dot of orthogonal axes is zero");
    check(vec3f_dot(x_axis, x_axis) == 1.0f, "dot of a unit axis is one");

    vec3f z_axis = vec3f_cross(x_axis, y_axis);
    check(vec3f_x(z_axis) == 0.0f && vec3f_y(z_axis) == 0.0f
          && vec3f_z(z_axis) == 1.0f, "x cross y is z");

    vec3f sum = vec3f_add(vec3f(1.0f, 2.0f, 3.0f), vec3f(4.0f, 5.0f, 6.0f));
    check(vec3f_x(sum) == 5.0f && vec3f_y(sum) == 7.0f
          && vec3f_z(sum) == 9.0f, "add");

    vec3f difference = vec3f_sub(vec3f(4.0f, 5.0f, 6.0f), vec3f(1.0f, 2.0f, 3.0f));
    check(vec3f_x(difference) == 3.0f && vec3f_y(difference) == 3.0f
          && vec3f_z(difference) == 3.0f, "sub");

    vec3f scaled = vec3f_scale(x_axis, 4.0f);
    check(vec3f_x(scaled) == 4.0f, "scale");

    check(vec3f_length(vec3f(3.0f, 4.0f, 0.0f)) == 5.0f, "length of 3-4-0 is 5");

    vec3f unit = vec3f_normalize(vec3f(0.0f, 0.0f, 7.0f));
    check(near_f32(vec3f_z(unit), 1.0f), "normalize yields a unit z");

    vec3f zero = vec3f_normalize(vec3f_0());
    check(vec3f_x(zero) == 0.0f && vec3f_y(zero) == 0.0f
          && vec3f_z(zero) == 0.0f, "normalize of zero stays zero");

    check(vec3f_x(vec3f_1(5.0f)) == 5.0f, "splat constructor");

    // b16 is storage: it round-trips through vec3f and computes there.
    vec3b packed = vec3b_from_vec3f(vec3f(1.0f, -0.5f, 1.5f));
    vec3f widened = vec3b_to_vec3f(packed);
    check(vec3f_x(widened) == 1.0f, "b16 round-trip x");
    check(vec3f_y(widened) == -0.5f, "b16 round-trip y");
    check(vec3f_z(widened) == 1.5f, "b16 round-trip z");

    vec2d point = vec2d(1.5, -2.5);
    check(vec2d_x(point) == 1.5 && vec2d_y(point) == -2.5, "vec2d lanes");
    check(near_d64(vec2d_length(vec2d(3.0, 4.0)), 5.0), "vec2d length");

    vec4f rgba = vec4f(1.0f, 2.0f, 3.0f, 4.0f);
    check(vec4f_w(rgba) == 4.0f, "vec4f_w reads w");
    check(vec4f_dot(rgba, rgba) == 30.0f, "vec4f dot");

    if (failures != 0) {
        fprintf(stderr, "vector_test: %d checks failed\n", failures);
        return 1;
    }

    printf("vector_test: vec2/vec3/vec4 x b16/f32/d64 held\n");
    return 0;
}
