// matrix_test.c — the mat2/mat3/mat4 families at f32/d64.
//
// Layout is proven at compile time (the header's _Static_asserts plus the ones
// below); behaviour is proven at run time through the entry accessors only, so
// the test is also the proof that a consumer never needs `(*ptr).col`.
//
// A matrix composes the vectors: each column is the vecN already proven by
// vector_test, so the size and alignment assertions below are the composition
// law, not a second geometry rule.

#include <math.h>
#include <stdio.h>

#include "vex.h"

_Static_assert(sizeof(mat2f) == 2 * sizeof(vec2f), "mat2f is two vec2f columns");
_Static_assert(sizeof(mat3f) == 3 * sizeof(vec3f), "mat3f is three vec3f columns");
_Static_assert(sizeof(mat4f) == 4 * sizeof(vec4f), "mat4f is four vec4f columns");

_Static_assert(sizeof(mat2d) == 2 * sizeof(vec2d), "mat2d is two vec2d columns");
_Static_assert(sizeof(mat3d) == 3 * sizeof(vec3d), "mat3d is three vec3d columns");
_Static_assert(sizeof(mat4d) == 4 * sizeof(vec4d), "mat4d is four vec4d columns");

_Static_assert(_Alignof(mat3f) == 16, "mat3f columns are 128-bit, 16-aligned");
_Static_assert(_Alignof(mat4d) == 32, "mat4d columns are 256-bit, 32-aligned");

static int failures;

static void check(int ok, const char *what) {
    if (ok) return;
    fprintf(stderr, "matrix_test: FAIL %s\n", what);
    failures++;
}

static int near_f32(f32 a, f32 b) { return fabsf(a - b) < 1e-6f; }
static int near_d64(d64 a, d64 b) { return fabs(a - b) < 1e-12; }

static mat2f mat2f_from(f32 a, f32 b, f32 c, f32 d) {
    mat2f m = mat2f_0();
    mat2f_setAt(&m, 0, 0, a);
    mat2f_setAt(&m, 0, 1, b);
    mat2f_setAt(&m, 1, 0, c);
    mat2f_setAt(&m, 1, 1, d);
    return m;
}

static mat3f mat3f_from_rows(f32 a, f32 b, f32 c,
                             f32 d, f32 e, f32 f,
                             f32 g, f32 h, f32 i) {
    mat3f m = mat3f_0();
    mat3f_setAt(&m, 0, 0, a); mat3f_setAt(&m, 0, 1, b); mat3f_setAt(&m, 0, 2, c);
    mat3f_setAt(&m, 1, 0, d); mat3f_setAt(&m, 1, 1, e); mat3f_setAt(&m, 1, 2, f);
    mat3f_setAt(&m, 2, 0, g); mat3f_setAt(&m, 2, 1, h); mat3f_setAt(&m, 2, 2, i);
    return m;
}

int main() {
    // Entry accessors and the zeroed column padding.
    mat3f id = mat3f_identity();
    check(mat3f_at(&id, 0, 0) == 1.0f && mat3f_at(&id, 1, 1) == 1.0f
          && mat3f_at(&id, 2, 2) == 1.0f, "identity diagonal is one");
    check(mat3f_at(&id, 0, 1) == 0.0f && mat3f_at(&id, 2, 0) == 0.0f,
          "identity off-diagonal is zero");
    check(id.col[0].data[3] == 0.0f, "mat3f column fourth lane is zeroed");

    mat3f_setAt(&id, 0, 1, 4.0f);
    check(mat3f_at(&id, 0, 1) == 4.0f, "setAt writes an entry");
    mat3f_setAt(&id, 0, 1, 0.0f);

    vec3f c = mat3f_col(&id, 1);
    check(vec3f_y(c) == 1.0f, "column getter returns the vec3 column");

    // Constructors: zero, diagonal splat, columns.
    mat3f z = mat3f_0();
    check(mat3f_at(&z, 0, 0) == 0.0f && mat3f_at(&z, 2, 2) == 0.0f, "mat3f_0 is zero");

    mat3f diag = mat3f_1(3.0f);
    check(mat3f_at(&diag, 0, 0) == 3.0f && mat3f_at(&diag, 1, 1) == 3.0f
          && mat3f_at(&diag, 0, 1) == 0.0f, "mat3f_1 is a diagonal splat");

    mat3f built = mat3f(vec3f(1.0f, 2.0f, 3.0f), vec3f(4.0f, 5.0f, 6.0f),
                        vec3f(7.0f, 8.0f, 9.0f));
    check(mat3f_at(&built, 1, 0) == 2.0f && mat3f_at(&built, 2, 1) == 6.0f,
          "mat3f(...) takes columns, storage is column-major");

    // Arithmetic.
    mat3f sum = mat3f_add(id, diag);
    check(mat3f_at(&sum, 0, 1) == 0.0f && mat3f_at(&sum, 1, 1) == 4.0f, "mat3f_add");

    mat3f difference = mat3f_sub(diag, id);
    check(mat3f_at(&difference, 0, 0) == 2.0f && mat3f_at(&difference, 1, 1) == 2.0f,
          "mat3f_sub");

    mat3f scaled = mat3f_scale(id, 2.0f);
    check(mat3f_at(&scaled, 2, 2) == 2.0f, "mat3f_scale");

    mat2f a2 = mat2f_from(1.0f, 2.0f, 3.0f, 4.0f);
    mat2f b2 = mat2f_from(2.0f, 0.0f, 0.0f, 2.0f);
    mat2f p2 = mat2f_multiply(a2, b2);
    check(mat2f_at(&p2, 0, 0) == 2.0f && mat2f_at(&p2, 0, 1) == 4.0f
          && mat2f_at(&p2, 1, 0) == 6.0f && mat2f_at(&p2, 1, 1) == 8.0f,
          "mat2f_multiply");

    mat2f t2 = mat2f_transpose(a2);
    check(mat2f_at(&t2, 0, 1) == 3.0f && mat2f_at(&t2, 1, 0) == 2.0f, "mat2f_transpose");

    check(mat2f_determinant(a2) == -2.0f, "mat2f_determinant");

    vec2f v2 = mat2f_multiplyVec(a2, vec2f(5.0f, 6.0f));
    check(vec2f_x(v2) == 17.0f && vec2f_y(v2) == 39.0f, "mat2f_multiplyVec");

    mat3f a3 = mat3f_from_rows(1.0f, 2.0f, 3.0f,
                               4.0f, 5.0f, 6.0f,
                               7.0f, 8.0f, 10.0f);
    check(near_f32(mat3f_determinant(a3), -3.0f), "mat3f_determinant");

    mat3f p3 = mat3f_multiply(a3, mat3f_identity());
    check(mat3f_at(&p3, 0, 2) == 3.0f && mat3f_at(&p3, 2, 0) == 7.0f,
          "identity is the multiply unit");

    // d64 side: the same law at the wider lane.
    mat2d d2 = mat2d(2.0);
    check(mat2d_at(&d2, 0, 0) == 2.0 && mat2d_at(&d2, 1, 1) == 2.0, "mat2d identity splat");
    check(mat2d_determinant(d2) == 4.0, "mat2d_determinant");

    mat4d d4 = mat4d_1(2.0);
    check(near_d64(mat4d_determinant(d4), 16.0), "mat4d_determinant of a diagonal");

    mat4f m4 = mat4f_identity();
    mat4f p4 = mat4f_multiply(m4, m4);
    check(mat4f_at(&p4, 3, 3) == 1.0f && mat4f_at(&p4, 0, 3) == 0.0f,
          "mat4f identity multiply");
    check(near_f32(mat4f_determinant(m4), 1.0f), "mat4f_determinant of identity");

    if (failures != 0) {
        fprintf(stderr, "matrix_test: %d checks failed\n", failures);
        return 1;
    }

    printf("matrix_test: mat2/mat3/mat4 x f32/d64 held\n");
    return 0;
}
