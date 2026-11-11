// mat4.c — mat4f and mat4d.
//
// One shape, two widths, one translation unit. A mat4 column is a vec4, so the
// flat data stride is four lanes and the storage is the plain 4x4 case every
// other matrix matches.

#include "vex.h"
#include "matrix.h"

;;OVERVIEW
;;DEFINITION

// ---------------------------------------------------------------- accessors

;;GETTER
f32 mat4f_at(const mat4f *self, u32 row, u32 col) {
    vec4f column = (*self).col[col];
    return column.data[row];
}

;;SETTER
void mat4f_setAt(mat4f *self, u32 row, u32 col, f32 value) {
    vec4f column = (*self).col[col];
    column.data[row] = value;
    (*self).col[col] = column;
}

;;GETTER
vec4f mat4f_col(const mat4f *self, u32 col) { return (*self).col[col]; }

;;SETTER
void mat4f_setCol(mat4f *self, u32 col, vec4f value) { (*self).col[col] = value; }

;;GETTER
d64 mat4d_at(const mat4d *self, u32 row, u32 col) {
    vec4d column = (*self).col[col];
    return column.data[row];
}

;;SETTER
void mat4d_setAt(mat4d *self, u32 row, u32 col, d64 value) {
    vec4d column = (*self).col[col];
    column.data[row] = value;
    (*self).col[col] = column;
}

;;GETTER
vec4d mat4d_col(const mat4d *self, u32 col) { return (*self).col[col]; }

;;SETTER
void mat4d_setCol(mat4d *self, u32 col, vec4d value) { (*self).col[col] = value; }

// ---------------------------------------------------------------- constructors

mat4f mat4f_0() { return (mat4f) { 0 }; }

mat4f mat4f_1(f32 scalar) {
    mat4f m = mat4f_0();
    mat4f_setAt(&m, 0, 0, scalar);
    mat4f_setAt(&m, 1, 1, scalar);
    mat4f_setAt(&m, 2, 2, scalar);
    mat4f_setAt(&m, 3, 3, scalar);
    return m;
}

mat4f mat4f_4(vec4f c0, vec4f c1, vec4f c2, vec4f c3) {
    mat4f m = mat4f_0();
    mat4f_setCol(&m, 0, c0);
    mat4f_setCol(&m, 1, c1);
    mat4f_setCol(&m, 2, c2);
    mat4f_setCol(&m, 3, c3);
    return m;
}

mat4f mat4f_identity() { return mat4f_1(1.0f); }

mat4d mat4d_0() { return (mat4d) { 0 }; }

mat4d mat4d_1(d64 scalar) {
    mat4d m = mat4d_0();
    mat4d_setAt(&m, 0, 0, scalar);
    mat4d_setAt(&m, 1, 1, scalar);
    mat4d_setAt(&m, 2, 2, scalar);
    mat4d_setAt(&m, 3, 3, scalar);
    return m;
}

mat4d mat4d_4(vec4d c0, vec4d c1, vec4d c2, vec4d c3) {
    mat4d m = mat4d_0();
    mat4d_setCol(&m, 0, c0);
    mat4d_setCol(&m, 1, c1);
    mat4d_setCol(&m, 2, c2);
    mat4d_setCol(&m, 3, c3);
    return m;
}

mat4d mat4d_identity() { return mat4d_1(1.0); }

// ---------------------------------------------------------------- arithmetic

mat4f mat4f_add(mat4f a, mat4f b) {
    mat4f out = mat4f_0();
    for (u32 col = 0; col < 4; col++) {
        for (u32 row = 0; row < 4; row++) {
            mat4f_setAt(&out, row, col, mat4f_at(&a, row, col) + mat4f_at(&b, row, col));
        }
    }
    return out;
}

mat4f mat4f_sub(mat4f a, mat4f b) {
    mat4f out = mat4f_0();
    for (u32 col = 0; col < 4; col++) {
        for (u32 row = 0; row < 4; row++) {
            mat4f_setAt(&out, row, col, mat4f_at(&a, row, col) - mat4f_at(&b, row, col));
        }
    }
    return out;
}

mat4f mat4f_scale(mat4f a, f32 scalar) {
    mat4f out = mat4f_0();
    for (u32 col = 0; col < 4; col++) {
        for (u32 row = 0; row < 4; row++) {
            mat4f_setAt(&out, row, col, mat4f_at(&a, row, col) * scalar);
        }
    }
    return out;
}

mat4f mat4f_multiply(mat4f a, mat4f b) {
    mat4f out = mat4f_0();
    for (u32 col = 0; col < 4; col++) {
        for (u32 row = 0; row < 4; row++) {
            f32 sum = 0.0f;
            for (u32 k = 0; k < 4; k++) {
                sum += mat4f_at(&a, row, k) * mat4f_at(&b, k, col);
            }
            mat4f_setAt(&out, row, col, sum);
        }
    }
    return out;
}

vec4f mat4f_multiplyVec(mat4f a, vec4f v) {
    return vec4f_4(
        mat4f_at(&a, 0, 0) * vec4f_x(v) + mat4f_at(&a, 0, 1) * vec4f_y(v) + mat4f_at(&a, 0, 2) * vec4f_z(v) + mat4f_at(&a, 0, 3) * vec4f_w(v),
        mat4f_at(&a, 1, 0) * vec4f_x(v) + mat4f_at(&a, 1, 1) * vec4f_y(v) + mat4f_at(&a, 1, 2) * vec4f_z(v) + mat4f_at(&a, 1, 3) * vec4f_w(v),
        mat4f_at(&a, 2, 0) * vec4f_x(v) + mat4f_at(&a, 2, 1) * vec4f_y(v) + mat4f_at(&a, 2, 2) * vec4f_z(v) + mat4f_at(&a, 2, 3) * vec4f_w(v),
        mat4f_at(&a, 3, 0) * vec4f_x(v) + mat4f_at(&a, 3, 1) * vec4f_y(v) + mat4f_at(&a, 3, 2) * vec4f_z(v) + mat4f_at(&a, 3, 3) * vec4f_w(v));
}

mat4f mat4f_transpose(mat4f a) {
    mat4f out = mat4f_0();
    for (u32 col = 0; col < 4; col++) {
        for (u32 row = 0; row < 4; row++) {
            mat4f_setAt(&out, row, col, mat4f_at(&a, col, row));
        }
    }
    return out;
}

f32 mat4f_determinant(mat4f a) {
    f32 a00 = mat4f_at(&a, 0, 0), a01 = mat4f_at(&a, 0, 1), a02 = mat4f_at(&a, 0, 2), a03 = mat4f_at(&a, 0, 3);
    f32 a10 = mat4f_at(&a, 1, 0), a11 = mat4f_at(&a, 1, 1), a12 = mat4f_at(&a, 1, 2), a13 = mat4f_at(&a, 1, 3);
    f32 a20 = mat4f_at(&a, 2, 0), a21 = mat4f_at(&a, 2, 1), a22 = mat4f_at(&a, 2, 2), a23 = mat4f_at(&a, 2, 3);
    f32 a30 = mat4f_at(&a, 3, 0), a31 = mat4f_at(&a, 3, 1), a32 = mat4f_at(&a, 3, 2), a33 = mat4f_at(&a, 3, 3);

    f32 s0 = a00 * a11 - a10 * a01;
    f32 s1 = a00 * a12 - a10 * a02;
    f32 s2 = a00 * a13 - a10 * a03;
    f32 s3 = a01 * a12 - a11 * a02;
    f32 s4 = a01 * a13 - a11 * a03;
    f32 s5 = a02 * a13 - a12 * a03;

    f32 c5 = a22 * a33 - a32 * a23;
    f32 c4 = a21 * a33 - a31 * a23;
    f32 c3 = a21 * a32 - a31 * a22;
    f32 c2 = a20 * a33 - a30 * a23;
    f32 c1 = a20 * a32 - a30 * a22;
    f32 c0 = a20 * a31 - a30 * a21;

    return s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;
}

mat4d mat4d_add(mat4d a, mat4d b) {
    mat4d out = mat4d_0();
    for (u32 col = 0; col < 4; col++) {
        for (u32 row = 0; row < 4; row++) {
            mat4d_setAt(&out, row, col, mat4d_at(&a, row, col) + mat4d_at(&b, row, col));
        }
    }
    return out;
}

mat4d mat4d_sub(mat4d a, mat4d b) {
    mat4d out = mat4d_0();
    for (u32 col = 0; col < 4; col++) {
        for (u32 row = 0; row < 4; row++) {
            mat4d_setAt(&out, row, col, mat4d_at(&a, row, col) - mat4d_at(&b, row, col));
        }
    }
    return out;
}

mat4d mat4d_scale(mat4d a, d64 scalar) {
    mat4d out = mat4d_0();
    for (u32 col = 0; col < 4; col++) {
        for (u32 row = 0; row < 4; row++) {
            mat4d_setAt(&out, row, col, mat4d_at(&a, row, col) * scalar);
        }
    }
    return out;
}

mat4d mat4d_multiply(mat4d a, mat4d b) {
    mat4d out = mat4d_0();
    for (u32 col = 0; col < 4; col++) {
        for (u32 row = 0; row < 4; row++) {
            d64 sum = 0.0;
            for (u32 k = 0; k < 4; k++) {
                sum += mat4d_at(&a, row, k) * mat4d_at(&b, k, col);
            }
            mat4d_setAt(&out, row, col, sum);
        }
    }
    return out;
}

vec4d mat4d_multiplyVec(mat4d a, vec4d v) {
    return vec4d_4(
        mat4d_at(&a, 0, 0) * vec4d_x(v) + mat4d_at(&a, 0, 1) * vec4d_y(v) + mat4d_at(&a, 0, 2) * vec4d_z(v) + mat4d_at(&a, 0, 3) * vec4d_w(v),
        mat4d_at(&a, 1, 0) * vec4d_x(v) + mat4d_at(&a, 1, 1) * vec4d_y(v) + mat4d_at(&a, 1, 2) * vec4d_z(v) + mat4d_at(&a, 1, 3) * vec4d_w(v),
        mat4d_at(&a, 2, 0) * vec4d_x(v) + mat4d_at(&a, 2, 1) * vec4d_y(v) + mat4d_at(&a, 2, 2) * vec4d_z(v) + mat4d_at(&a, 2, 3) * vec4d_w(v),
        mat4d_at(&a, 3, 0) * vec4d_x(v) + mat4d_at(&a, 3, 1) * vec4d_y(v) + mat4d_at(&a, 3, 2) * vec4d_z(v) + mat4d_at(&a, 3, 3) * vec4d_w(v));
}

mat4d mat4d_transpose(mat4d a) {
    mat4d out = mat4d_0();
    for (u32 col = 0; col < 4; col++) {
        for (u32 row = 0; row < 4; row++) {
            mat4d_setAt(&out, row, col, mat4d_at(&a, col, row));
        }
    }
    return out;
}

d64 mat4d_determinant(mat4d a) {
    d64 a00 = mat4d_at(&a, 0, 0), a01 = mat4d_at(&a, 0, 1), a02 = mat4d_at(&a, 0, 2), a03 = mat4d_at(&a, 0, 3);
    d64 a10 = mat4d_at(&a, 1, 0), a11 = mat4d_at(&a, 1, 1), a12 = mat4d_at(&a, 1, 2), a13 = mat4d_at(&a, 1, 3);
    d64 a20 = mat4d_at(&a, 2, 0), a21 = mat4d_at(&a, 2, 1), a22 = mat4d_at(&a, 2, 2), a23 = mat4d_at(&a, 2, 3);
    d64 a30 = mat4d_at(&a, 3, 0), a31 = mat4d_at(&a, 3, 1), a32 = mat4d_at(&a, 3, 2), a33 = mat4d_at(&a, 3, 3);

    d64 s0 = a00 * a11 - a10 * a01;
    d64 s1 = a00 * a12 - a10 * a02;
    d64 s2 = a00 * a13 - a10 * a03;
    d64 s3 = a01 * a12 - a11 * a02;
    d64 s4 = a01 * a13 - a11 * a03;
    d64 s5 = a02 * a13 - a12 * a03;

    d64 c5 = a22 * a33 - a32 * a23;
    d64 c4 = a21 * a33 - a31 * a23;
    d64 c3 = a21 * a32 - a31 * a22;
    d64 c2 = a20 * a33 - a30 * a23;
    d64 c1 = a20 * a32 - a30 * a22;
    d64 c0 = a20 * a31 - a30 * a21;

    return s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;
}

