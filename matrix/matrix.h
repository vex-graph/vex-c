#ifndef VEXC_MATRIX_H
#define VEXC_MATRIX_H

// matrix.h — the matrix value types of the floor.
//
// The third and last math header: primitive.h (scalars), vector.h
// (vec2/vec3/vec4) and matrix.h (mat2/mat3/mat4). A matrix composes the
// vectors; it does not re-own their geometry.
//
// Storage is column-major and flat. A matrix is its columns and nothing else:
//
//     mat2   vec2 columns, 2 columns   size = 2 * sizeof(vec2)
//     mat3   vec3 columns, 3 columns   size = 3 * sizeof(vec3)
//     mat4   vec4 columns, 4 columns   size = 4 * sizeof(vec4)
//
// A vec3 column carries the zeroed fourth lane the Vector Layout Law already
// pins, so a mat3f column is 16 bytes at 16-byte alignment and every column is
// one 128-bit load. `data[]` is the flat seam: mat3 data has a column stride of
// four lanes, so entry (row, col) lives at data[col * 4 + row].
//
// This is the include surface: types, layout assertions and declarations. The
// bodies live beside it, one shape per translation unit:
//
//     matrix/mat2.c   mat2f, mat2d
//     matrix/mat3.c   mat3f, mat3d
//     matrix/mat4.c   mat4f, mat4d
//
//     mat3f m = mat3f_1(1.0f);          // diagonal
//     f32   a = mat3f_at(&m, 0, 1);
//     mat3f_setAt(&m, 2, 2, 7.0f);

#include "../lang/annotation.h"
#include "../lang/constructor.h"
#include "../lang/primitive.h"
#include "../vector/vector.h"

// ---------------------------------------------------------------- mat2

typedef struct mat2f {
    union {
        vec2f col[2];
        f32 data[4];
    };
} mat2f;

typedef struct mat2d {
    union {
        vec2d col[2];
        d64 data[4];
    };
} mat2d;

// ---------------------------------------------------------------- mat3

typedef struct mat3f {
    union {
        vec3f col[3];
        f32 data[12];
    };
} mat3f;

typedef struct mat3d {
    union {
        vec3d col[3];
        d64 data[12];
    };
} mat3d;

// ---------------------------------------------------------------- mat4

typedef struct mat4f {
    union {
        vec4f col[4];
        f32 data[16];
    };
} mat4f;

typedef struct mat4d {
    union {
        vec4d col[4];
        d64 data[16];
    };
} mat4d;

// The layout rule, checked rather than described.

_Static_assert(sizeof(mat2f) == 2 * sizeof(vec2f), "mat2f: two vec2f columns");
_Static_assert(sizeof(mat2d) == 2 * sizeof(vec2d), "mat2d: two vec2d columns");
_Static_assert(sizeof(mat3f) == 3 * sizeof(vec3f), "mat3f: three vec3f columns");
_Static_assert(sizeof(mat3d) == 3 * sizeof(vec3d), "mat3d: three vec3d columns");
_Static_assert(sizeof(mat4f) == 4 * sizeof(vec4f), "mat4f: four vec4f columns");
_Static_assert(sizeof(mat4d) == 4 * sizeof(vec4d), "mat4d: four vec4d columns");

_Static_assert(_Alignof(mat2f) == _Alignof(vec2f), "mat2f inherits vec2f alignment");
_Static_assert(_Alignof(mat2d) == _Alignof(vec2d), "mat2d inherits vec2d alignment");
_Static_assert(_Alignof(mat3f) == _Alignof(vec3f), "mat3f inherits vec3f alignment");
_Static_assert(_Alignof(mat3d) == _Alignof(vec3d), "mat3d inherits vec3d alignment");
_Static_assert(_Alignof(mat4f) == _Alignof(vec4f), "mat4f inherits vec4f alignment");
_Static_assert(_Alignof(mat4d) == _Alignof(vec4d), "mat4d inherits vec4d alignment");

_Static_assert(sizeof(mat3f) / sizeof(f32) == 12, "mat3f data seam is 12 lanes");
_Static_assert(sizeof(mat3d) / sizeof(d64) == 12, "mat3d data seam is 12 lanes");

// ---------------------------------------------------------------- mat2 accessors

;;GETTER
f32 mat2f_at(const mat2f *self, u32 row, u32 col);

;;SETTER
void mat2f_setAt(mat2f *self, u32 row, u32 col, f32 value);

;;GETTER
vec2f mat2f_col(const mat2f *self, u32 col);

;;SETTER
void mat2f_setCol(mat2f *self, u32 col, vec2f value);

;;GETTER
d64 mat2d_at(const mat2d *self, u32 row, u32 col);

;;SETTER
void mat2d_setAt(mat2d *self, u32 row, u32 col, d64 value);

;;GETTER
vec2d mat2d_col(const mat2d *self, u32 col);

;;SETTER
void mat2d_setCol(mat2d *self, u32 col, vec2d value);

// ---------------------------------------------------------------- mat3 accessors

;;GETTER
f32 mat3f_at(const mat3f *self, u32 row, u32 col);

;;SETTER
void mat3f_setAt(mat3f *self, u32 row, u32 col, f32 value);

;;GETTER
vec3f mat3f_col(const mat3f *self, u32 col);

;;SETTER
void mat3f_setCol(mat3f *self, u32 col, vec3f value);

;;GETTER
d64 mat3d_at(const mat3d *self, u32 row, u32 col);

;;SETTER
void mat3d_setAt(mat3d *self, u32 row, u32 col, d64 value);

;;GETTER
vec3d mat3d_col(const mat3d *self, u32 col);

;;SETTER
void mat3d_setCol(mat3d *self, u32 col, vec3d value);

// ---------------------------------------------------------------- mat4 accessors

;;GETTER
f32 mat4f_at(const mat4f *self, u32 row, u32 col);

;;SETTER
void mat4f_setAt(mat4f *self, u32 row, u32 col, f32 value);

;;GETTER
vec4f mat4f_col(const mat4f *self, u32 col);

;;SETTER
void mat4f_setCol(mat4f *self, u32 col, vec4f value);

;;GETTER
d64 mat4d_at(const mat4d *self, u32 row, u32 col);

;;SETTER
void mat4d_setAt(mat4d *self, u32 row, u32 col, d64 value);

;;GETTER
vec4d mat4d_col(const mat4d *self, u32 col);

;;SETTER
void mat4d_setCol(mat4d *self, u32 col, vec4d value);

// ---------------------------------------------------------------- mat2 constructors

mat2f mat2f_0();
mat2f mat2f_1(f32 scalar);
mat2f mat2f_2(vec2f c0, vec2f c1);
mat2f mat2f_identity();

mat2d mat2d_0();
mat2d mat2d_1(d64 scalar);
mat2d mat2d_2(vec2d c0, vec2d c1);
mat2d mat2d_identity();

// ---------------------------------------------------------------- mat3 constructors

mat3f mat3f_0();
mat3f mat3f_1(f32 scalar);
mat3f mat3f_3(vec3f c0, vec3f c1, vec3f c2);
mat3f mat3f_identity();

mat3d mat3d_0();
mat3d mat3d_1(d64 scalar);
mat3d mat3d_3(vec3d c0, vec3d c1, vec3d c2);
mat3d mat3d_identity();

// ---------------------------------------------------------------- mat4 constructors

mat4f mat4f_0();
mat4f mat4f_1(f32 scalar);
mat4f mat4f_4(vec4f c0, vec4f c1, vec4f c2, vec4f c3);
mat4f mat4f_identity();

mat4d mat4d_0();
mat4d mat4d_1(d64 scalar);
mat4d mat4d_4(vec4d c0, vec4d c1, vec4d c2, vec4d c3);
mat4d mat4d_identity();

// ---------------------------------------------------------------- arithmetic

mat2f mat2f_add(mat2f a, mat2f b);
mat2f mat2f_sub(mat2f a, mat2f b);
mat2f mat2f_scale(mat2f a, f32 scalar);
mat2f mat2f_multiply(mat2f a, mat2f b);
vec2f mat2f_multiplyVec(mat2f a, vec2f v);
mat2f mat2f_transpose(mat2f a);
f32   mat2f_determinant(mat2f a);

mat2d mat2d_add(mat2d a, mat2d b);
mat2d mat2d_sub(mat2d a, mat2d b);
mat2d mat2d_scale(mat2d a, d64 scalar);
mat2d mat2d_multiply(mat2d a, mat2d b);
vec2d mat2d_multiplyVec(mat2d a, vec2d v);
mat2d mat2d_transpose(mat2d a);
d64   mat2d_determinant(mat2d a);

mat3f mat3f_add(mat3f a, mat3f b);
mat3f mat3f_sub(mat3f a, mat3f b);
mat3f mat3f_scale(mat3f a, f32 scalar);
mat3f mat3f_multiply(mat3f a, mat3f b);
vec3f mat3f_multiplyVec(mat3f a, vec3f v);
mat3f mat3f_transpose(mat3f a);
f32   mat3f_determinant(mat3f a);

mat3d mat3d_add(mat3d a, mat3d b);
mat3d mat3d_sub(mat3d a, mat3d b);
mat3d mat3d_scale(mat3d a, d64 scalar);
mat3d mat3d_multiply(mat3d a, mat3d b);
vec3d mat3d_multiplyVec(mat3d a, vec3d v);
mat3d mat3d_transpose(mat3d a);
d64   mat3d_determinant(mat3d a);

mat4f mat4f_add(mat4f a, mat4f b);
mat4f mat4f_sub(mat4f a, mat4f b);
mat4f mat4f_scale(mat4f a, f32 scalar);
mat4f mat4f_multiply(mat4f a, mat4f b);
vec4f mat4f_multiplyVec(mat4f a, vec4f v);
mat4f mat4f_transpose(mat4f a);
f32   mat4f_determinant(mat4f a);

mat4d mat4d_add(mat4d a, mat4d b);
mat4d mat4d_sub(mat4d a, mat4d b);
mat4d mat4d_scale(mat4d a, d64 scalar);
mat4d mat4d_multiply(mat4d a, mat4d b);
vec4d mat4d_multiplyVec(mat4d a, vec4d v);
mat4d mat4d_transpose(mat4d a);
d64   mat4d_determinant(mat4d a);

// ---------------------------------------------------------------- dispatch

#define mat2f(...) CONSTRUCTOR_DISPATCH(mat2f, __VA_ARGS__)
#define mat2d(...) CONSTRUCTOR_DISPATCH(mat2d, __VA_ARGS__)

#define mat3f(...) CONSTRUCTOR_DISPATCH(mat3f, __VA_ARGS__)
#define mat3d(...) CONSTRUCTOR_DISPATCH(mat3d, __VA_ARGS__)

#define mat4f(...) CONSTRUCTOR_DISPATCH(mat4f, __VA_ARGS__)
#define mat4d(...) CONSTRUCTOR_DISPATCH(mat4d, __VA_ARGS__)

#endif
