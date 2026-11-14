// mat2.c — mat2f and mat2d.
//
// One shape, two widths, one translation unit. Storage is column-major; every
// arithmetic function reaches it through the accessors declared in matrix.h.

#include "vex.h"
#include "matrix/matrix.h"

;;OVERVIEW
;;DEFINITION

// ---------------------------------------------------------------- accessors

;;GETTER
f32 mat2f_at(const mat2f *self, u32 row, u32 col) {
    vec2f column = (*self).col[col];
    return column.data[row];
}

;;SETTER
fn mat2f_setAt(mat2f *self, u32 row, u32 col, f32 value) {
    vec2f column = (*self).col[col];
    column.data[row] = value;
    (*self).col[col] = column;
}

;;GETTER
vec2f mat2f_col(const mat2f *self, u32 col) { return (*self).col[col]; }

;;SETTER
fn mat2f_setCol(mat2f *self, u32 col, vec2f value) { (*self).col[col] = value; }

;;GETTER
d64 mat2d_at(const mat2d *self, u32 row, u32 col) {
    vec2d column = (*self).col[col];
    return column.data[row];
}

;;SETTER
fn mat2d_setAt(mat2d *self, u32 row, u32 col, d64 value) {
    vec2d column = (*self).col[col];
    column.data[row] = value;
    (*self).col[col] = column;
}

;;GETTER
vec2d mat2d_col(const mat2d *self, u32 col) { return (*self).col[col]; }

;;SETTER
fn mat2d_setCol(mat2d *self, u32 col, vec2d value) { (*self).col[col] = value; }

// ---------------------------------------------------------------- constructors

mat2f mat2f_0() { return (mat2f) { 0 }; }

mat2f mat2f_1(f32 scalar) {
    mat2f m = mat2f_0();
    mat2f_setAt(&m, 0, 0, scalar);
    mat2f_setAt(&m, 1, 1, scalar);
    return m;
}

mat2f mat2f_2(vec2f c0, vec2f c1) {
    mat2f m = mat2f_0();
    mat2f_setCol(&m, 0, c0);
    mat2f_setCol(&m, 1, c1);
    return m;
}

mat2f mat2f_identity() { return mat2f_1(1.0f); }

mat2d mat2d_0() { return (mat2d) { 0 }; }

mat2d mat2d_1(d64 scalar) {
    mat2d m = mat2d_0();
    mat2d_setAt(&m, 0, 0, scalar);
    mat2d_setAt(&m, 1, 1, scalar);
    return m;
}

mat2d mat2d_2(vec2d c0, vec2d c1) {
    mat2d m = mat2d_0();
    mat2d_setCol(&m, 0, c0);
    mat2d_setCol(&m, 1, c1);
    return m;
}

mat2d mat2d_identity() { return mat2d_1(1.0); }

// ---------------------------------------------------------------- arithmetic

mat2f mat2f_add(mat2f a, mat2f b) {
    mat2f out = mat2f_0();
    for (u32 col = 0; col < 2; col++) {
        for (u32 row = 0; row < 2; row++) {
            mat2f_setAt(&out, row, col, mat2f_at(&a, row, col) + mat2f_at(&b, row, col));
        }
    }
    return out;
}

mat2f mat2f_sub(mat2f a, mat2f b) {
    mat2f out = mat2f_0();
    for (u32 col = 0; col < 2; col++) {
        for (u32 row = 0; row < 2; row++) {
            mat2f_setAt(&out, row, col, mat2f_at(&a, row, col) - mat2f_at(&b, row, col));
        }
    }
    return out;
}

mat2f mat2f_scale(mat2f a, f32 scalar) {
    mat2f out = mat2f_0();
    for (u32 col = 0; col < 2; col++) {
        for (u32 row = 0; row < 2; row++) {
            mat2f_setAt(&out, row, col, mat2f_at(&a, row, col) * scalar);
        }
    }
    return out;
}

mat2f mat2f_multiply(mat2f a, mat2f b) {
    mat2f out = mat2f_0();
    for (u32 col = 0; col < 2; col++) {
        for (u32 row = 0; row < 2; row++) {
            f32 sum = 0.0f;
            for (u32 k = 0; k < 2; k++) {
                sum += mat2f_at(&a, row, k) * mat2f_at(&b, k, col);
            }
            mat2f_setAt(&out, row, col, sum);
        }
    }
    return out;
}

vec2f mat2f_multiplyVec(mat2f a, vec2f v) {
    return vec2f_2(mat2f_at(&a, 0, 0) * vec2f_x(v) + mat2f_at(&a, 0, 1) * vec2f_y(v),
                   mat2f_at(&a, 1, 0) * vec2f_x(v) + mat2f_at(&a, 1, 1) * vec2f_y(v));
}

mat2f mat2f_transpose(mat2f a) {
    mat2f out = mat2f_0();
    for (u32 col = 0; col < 2; col++) {
        for (u32 row = 0; row < 2; row++) {
            mat2f_setAt(&out, row, col, mat2f_at(&a, col, row));
        }
    }
    return out;
}

f32 mat2f_determinant(mat2f a) {
    return mat2f_at(&a, 0, 0) * mat2f_at(&a, 1, 1)
         - mat2f_at(&a, 0, 1) * mat2f_at(&a, 1, 0);
}

mat2d mat2d_add(mat2d a, mat2d b) {
    mat2d out = mat2d_0();
    for (u32 col = 0; col < 2; col++) {
        for (u32 row = 0; row < 2; row++) {
            mat2d_setAt(&out, row, col, mat2d_at(&a, row, col) + mat2d_at(&b, row, col));
        }
    }
    return out;
}

mat2d mat2d_sub(mat2d a, mat2d b) {
    mat2d out = mat2d_0();
    for (u32 col = 0; col < 2; col++) {
        for (u32 row = 0; row < 2; row++) {
            mat2d_setAt(&out, row, col, mat2d_at(&a, row, col) - mat2d_at(&b, row, col));
        }
    }
    return out;
}

mat2d mat2d_scale(mat2d a, d64 scalar) {
    mat2d out = mat2d_0();
    for (u32 col = 0; col < 2; col++) {
        for (u32 row = 0; row < 2; row++) {
            mat2d_setAt(&out, row, col, mat2d_at(&a, row, col) * scalar);
        }
    }
    return out;
}

mat2d mat2d_multiply(mat2d a, mat2d b) {
    mat2d out = mat2d_0();
    for (u32 col = 0; col < 2; col++) {
        for (u32 row = 0; row < 2; row++) {
            d64 sum = 0.0;
            for (u32 k = 0; k < 2; k++) {
                sum += mat2d_at(&a, row, k) * mat2d_at(&b, k, col);
            }
            mat2d_setAt(&out, row, col, sum);
        }
    }
    return out;
}

vec2d mat2d_multiplyVec(mat2d a, vec2d v) {
    return vec2d_2(mat2d_at(&a, 0, 0) * vec2d_x(v) + mat2d_at(&a, 0, 1) * vec2d_y(v),
                   mat2d_at(&a, 1, 0) * vec2d_x(v) + mat2d_at(&a, 1, 1) * vec2d_y(v));
}

mat2d mat2d_transpose(mat2d a) {
    mat2d out = mat2d_0();
    for (u32 col = 0; col < 2; col++) {
        for (u32 row = 0; row < 2; row++) {
            mat2d_setAt(&out, row, col, mat2d_at(&a, col, row));
        }
    }
    return out;
}

d64 mat2d_determinant(mat2d a) {
    return mat2d_at(&a, 0, 0) * mat2d_at(&a, 1, 1)
         - mat2d_at(&a, 0, 1) * mat2d_at(&a, 1, 0);
}

