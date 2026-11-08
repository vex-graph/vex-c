// mat3.c — mat3f and mat3d.
//
// One shape, two widths, one translation unit. A mat3 column is a vec3, so the
// column carries the zeroed fourth lane and the flat data stride is four lanes.
// Every arithmetic function reaches storage through the accessors declared in
// matrix.h.

#include "matrix.h"

;;OVERVIEW
;;DEFINITION

// ---------------------------------------------------------------- accessors

;;GETTER
f32 mat3f_at(const mat3f *self, u32 row, u32 col) {
    vec3f column = (*self).col[col];
    return column.data[row];
}

;;SETTER
void mat3f_setAt(mat3f *self, u32 row, u32 col, f32 value) {
    vec3f column = (*self).col[col];
    column.data[row] = value;
    (*self).col[col] = column;
}

;;GETTER
vec3f mat3f_col(const mat3f *self, u32 col) { return (*self).col[col]; }

;;SETTER
void mat3f_setCol(mat3f *self, u32 col, vec3f value) { (*self).col[col] = value; }

;;GETTER
d64 mat3d_at(const mat3d *self, u32 row, u32 col) {
    vec3d column = (*self).col[col];
    return column.data[row];
}

;;SETTER
void mat3d_setAt(mat3d *self, u32 row, u32 col, d64 value) {
    vec3d column = (*self).col[col];
    column.data[row] = value;
    (*self).col[col] = column;
}

;;GETTER
vec3d mat3d_col(const mat3d *self, u32 col) { return (*self).col[col]; }

;;SETTER
void mat3d_setCol(mat3d *self, u32 col, vec3d value) { (*self).col[col] = value; }

// ---------------------------------------------------------------- constructors

mat3f mat3f_0() { return (mat3f) { 0 }; }

mat3f mat3f_1(f32 scalar) {
    mat3f m = mat3f_0();
    mat3f_setAt(&m, 0, 0, scalar);
    mat3f_setAt(&m, 1, 1, scalar);
    mat3f_setAt(&m, 2, 2, scalar);
    return m;
}

mat3f mat3f_3(vec3f c0, vec3f c1, vec3f c2) {
    mat3f m = mat3f_0();
    mat3f_setCol(&m, 0, c0);
    mat3f_setCol(&m, 1, c1);
    mat3f_setCol(&m, 2, c2);
    return m;
}

mat3f mat3f_identity() { return mat3f_1(1.0f); }

mat3d mat3d_0() { return (mat3d) { 0 }; }

mat3d mat3d_1(d64 scalar) {
    mat3d m = mat3d_0();
    mat3d_setAt(&m, 0, 0, scalar);
    mat3d_setAt(&m, 1, 1, scalar);
    mat3d_setAt(&m, 2, 2, scalar);
    return m;
}

mat3d mat3d_3(vec3d c0, vec3d c1, vec3d c2) {
    mat3d m = mat3d_0();
    mat3d_setCol(&m, 0, c0);
    mat3d_setCol(&m, 1, c1);
    mat3d_setCol(&m, 2, c2);
    return m;
}

mat3d mat3d_identity() { return mat3d_1(1.0); }

// ---------------------------------------------------------------- arithmetic

mat3f mat3f_add(mat3f a, mat3f b) {
    mat3f out = mat3f_0();
    for (u32 col = 0; col < 3; col++) {
        for (u32 row = 0; row < 3; row++) {
            mat3f_setAt(&out, row, col, mat3f_at(&a, row, col) + mat3f_at(&b, row, col));
        }
    }
    return out;
}

mat3f mat3f_sub(mat3f a, mat3f b) {
    mat3f out = mat3f_0();
    for (u32 col = 0; col < 3; col++) {
        for (u32 row = 0; row < 3; row++) {
            mat3f_setAt(&out, row, col, mat3f_at(&a, row, col) - mat3f_at(&b, row, col));
        }
    }
    return out;
}

mat3f mat3f_scale(mat3f a, f32 scalar) {
    mat3f out = mat3f_0();
    for (u32 col = 0; col < 3; col++) {
        for (u32 row = 0; row < 3; row++) {
            mat3f_setAt(&out, row, col, mat3f_at(&a, row, col) * scalar);
        }
    }
    return out;
}

mat3f mat3f_multiply(mat3f a, mat3f b) {
    mat3f out = mat3f_0();
    for (u32 col = 0; col < 3; col++) {
        for (u32 row = 0; row < 3; row++) {
            f32 sum = 0.0f;
            for (u32 k = 0; k < 3; k++) {
                sum += mat3f_at(&a, row, k) * mat3f_at(&b, k, col);
            }
            mat3f_setAt(&out, row, col, sum);
        }
    }
    return out;
}

vec3f mat3f_multiplyVec(mat3f a, vec3f v) {
    return vec3f_3(
        mat3f_at(&a, 0, 0) * vec3f_x(v) + mat3f_at(&a, 0, 1) * vec3f_y(v) + mat3f_at(&a, 0, 2) * vec3f_z(v),
        mat3f_at(&a, 1, 0) * vec3f_x(v) + mat3f_at(&a, 1, 1) * vec3f_y(v) + mat3f_at(&a, 1, 2) * vec3f_z(v),
        mat3f_at(&a, 2, 0) * vec3f_x(v) + mat3f_at(&a, 2, 1) * vec3f_y(v) + mat3f_at(&a, 2, 2) * vec3f_z(v));
}

mat3f mat3f_transpose(mat3f a) {
    mat3f out = mat3f_0();
    for (u32 col = 0; col < 3; col++) {
        for (u32 row = 0; row < 3; row++) {
            mat3f_setAt(&out, row, col, mat3f_at(&a, col, row));
        }
    }
    return out;
}

f32 mat3f_determinant(mat3f a) {
    f32 a00 = mat3f_at(&a, 0, 0), a01 = mat3f_at(&a, 0, 1), a02 = mat3f_at(&a, 0, 2);
    f32 a10 = mat3f_at(&a, 1, 0), a11 = mat3f_at(&a, 1, 1), a12 = mat3f_at(&a, 1, 2);
    f32 a20 = mat3f_at(&a, 2, 0), a21 = mat3f_at(&a, 2, 1), a22 = mat3f_at(&a, 2, 2);
    return a00 * (a11 * a22 - a12 * a21)
         - a01 * (a10 * a22 - a12 * a20)
         + a02 * (a10 * a21 - a11 * a20);
}

mat3d mat3d_add(mat3d a, mat3d b) {
    mat3d out = mat3d_0();
    for (u32 col = 0; col < 3; col++) {
        for (u32 row = 0; row < 3; row++) {
            mat3d_setAt(&out, row, col, mat3d_at(&a, row, col) + mat3d_at(&b, row, col));
        }
    }
    return out;
}

mat3d mat3d_sub(mat3d a, mat3d b) {
    mat3d out = mat3d_0();
    for (u32 col = 0; col < 3; col++) {
        for (u32 row = 0; row < 3; row++) {
            mat3d_setAt(&out, row, col, mat3d_at(&a, row, col) - mat3d_at(&b, row, col));
        }
    }
    return out;
}

mat3d mat3d_scale(mat3d a, d64 scalar) {
    mat3d out = mat3d_0();
    for (u32 col = 0; col < 3; col++) {
        for (u32 row = 0; row < 3; row++) {
            mat3d_setAt(&out, row, col, mat3d_at(&a, row, col) * scalar);
        }
    }
    return out;
}

mat3d mat3d_multiply(mat3d a, mat3d b) {
    mat3d out = mat3d_0();
    for (u32 col = 0; col < 3; col++) {
        for (u32 row = 0; row < 3; row++) {
            d64 sum = 0.0;
            for (u32 k = 0; k < 3; k++) {
                sum += mat3d_at(&a, row, k) * mat3d_at(&b, k, col);
            }
            mat3d_setAt(&out, row, col, sum);
        }
    }
    return out;
}

vec3d mat3d_multiplyVec(mat3d a, vec3d v) {
    return vec3d_3(
        mat3d_at(&a, 0, 0) * vec3d_x(v) + mat3d_at(&a, 0, 1) * vec3d_y(v) + mat3d_at(&a, 0, 2) * vec3d_z(v),
        mat3d_at(&a, 1, 0) * vec3d_x(v) + mat3d_at(&a, 1, 1) * vec3d_y(v) + mat3d_at(&a, 1, 2) * vec3d_z(v),
        mat3d_at(&a, 2, 0) * vec3d_x(v) + mat3d_at(&a, 2, 1) * vec3d_y(v) + mat3d_at(&a, 2, 2) * vec3d_z(v));
}

mat3d mat3d_transpose(mat3d a) {
    mat3d out = mat3d_0();
    for (u32 col = 0; col < 3; col++) {
        for (u32 row = 0; row < 3; row++) {
            mat3d_setAt(&out, row, col, mat3d_at(&a, col, row));
        }
    }
    return out;
}

d64 mat3d_determinant(mat3d a) {
    d64 a00 = mat3d_at(&a, 0, 0), a01 = mat3d_at(&a, 0, 1), a02 = mat3d_at(&a, 0, 2);
    d64 a10 = mat3d_at(&a, 1, 0), a11 = mat3d_at(&a, 1, 1), a12 = mat3d_at(&a, 1, 2);
    d64 a20 = mat3d_at(&a, 2, 0), a21 = mat3d_at(&a, 2, 1), a22 = mat3d_at(&a, 2, 2);
    return a00 * (a11 * a22 - a12 * a21)
         - a01 * (a10 * a22 - a12 * a20)
         + a02 * (a10 * a21 - a11 * a20);
}

