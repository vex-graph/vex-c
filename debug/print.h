#ifndef VEXC_PRINT_H
#define VEXC_PRINT_H

/**
 * print.h -- zero-format-string variadic console logger for vex-c.
 *
 * Concatenates 0-16 arguments of any floor type (scalars, 128-bit ints, b16,
 * if64, ld128, string, C strings, vec2/3/4, mat2/3/4, quat32/64, pointers,
 * nullptr) into a single stack utf8 buffer with zero heap allocation and writes
 * one line to stdout:
 *
 * print("fps=", 60, " pos=", vec3f(1.0f, 2.0f, 3.0f), " ok=", true);
 */

#include "lang/primitive.h"
#include "vector/vector.h"
#include "matrix/matrix.h"
#include "quaternion/quaternion.h"

#define PRINT_BUFFER_BYTES 4096u

typedef enum PrintArgKind {
    PRINT_ARG_NULL = 0,
    PRINT_ARG_BOOL,
    PRINT_ARG_CHAR,
    PRINT_ARG_SIGNED,
    PRINT_ARG_UNSIGNED,
#if defined(__SIZEOF_INT128__)
    PRINT_ARG_I128,
    PRINT_ARG_U128,
#endif
    PRINT_ARG_FLOAT,
    PRINT_ARG_CSTR,
    PRINT_ARG_VARCHAR,
    PRINT_ARG_PTR,
    PRINT_ARG_IF64,
    PRINT_ARG_LD128,
    PRINT_ARG_VEC2F,
    PRINT_ARG_VEC2D,
    PRINT_ARG_VEC3F,
    PRINT_ARG_VEC3D,
    PRINT_ARG_VEC4F,
    PRINT_ARG_VEC4D,
    PRINT_ARG_MAT2F,
    PRINT_ARG_MAT2D,
    PRINT_ARG_MAT3F,
    PRINT_ARG_MAT3D,
    PRINT_ARG_MAT4F,
    PRINT_ARG_MAT4D,
    PRINT_ARG_QUAT32,
    PRINT_ARG_QUAT64
} PrintArgKind;

typedef struct PrintArg {
    PrintArgKind kind;
    union {
        bool b;
        utf8 ch;
        i64 s;
        u64 u;
#if defined(__SIZEOF_INT128__)
        i128 s128;
        u128 u128;
#endif
        d64 d;
        string cstr;
        varchar vstr;
        const_any ptr;
        if64 pair64;
        ld128 pair128;
        vec2f v2f;
        vec2d v2d;
        vec3f v3f;
        vec3d v3d;
        vec4f v4f;
        vec4d v4d;
        mat2f m2f;
        mat2d m2d;
        mat3f m3f;
        mat3d m3d;
        mat4f m4f;
        mat4d m4d;
        quat32 q32;
        quat64 q64;
    } value;
} PrintArg;

PrintArg PrintArg_fromNull(const_any ignored);
PrintArg PrintArg_fromBool(bool value);
PrintArg PrintArg_fromChar(utf8 value);
PrintArg PrintArg_fromSigned(i64 value);
PrintArg PrintArg_fromUnsigned(u64 value);
#if defined(__SIZEOF_INT128__)
PrintArg PrintArg_fromI128(i128 value);
PrintArg PrintArg_fromU128(u128 value);
#endif
PrintArg PrintArg_fromFloat(d64 value);
PrintArg PrintArg_fromB16(b16 value);
PrintArg PrintArg_fromCStr(string value);
PrintArg PrintArg_fromVarchar(varchar value);
PrintArg PrintArg_fromPtr(const_any value);
PrintArg PrintArg_fromIf64(if64 value);
PrintArg PrintArg_fromLd128(ld128 value);
PrintArg PrintArg_fromVec2b(vec2b value);
PrintArg PrintArg_fromVec2f(vec2f value);
PrintArg PrintArg_fromVec2d(vec2d value);
PrintArg PrintArg_fromVec3b(vec3b value);
PrintArg PrintArg_fromVec3f(vec3f value);
PrintArg PrintArg_fromVec3d(vec3d value);
PrintArg PrintArg_fromVec4b(vec4b value);
PrintArg PrintArg_fromVec4f(vec4f value);
PrintArg PrintArg_fromVec4d(vec4d value);
PrintArg PrintArg_fromMat2f(mat2f value);
PrintArg PrintArg_fromMat2d(mat2d value);
PrintArg PrintArg_fromMat3f(mat3f value);
PrintArg PrintArg_fromMat3d(mat3d value);
PrintArg PrintArg_fromMat4f(mat4f value);
PrintArg PrintArg_fromMat4d(mat4d value);
PrintArg PrintArg_fromQuat32(quat32 value);
PrintArg PrintArg_fromQuat64(quat64 value);

bytes Print_concatArgs(utf8 *dest, bytes cap, const PrintArg *args, bytes count);
fn Print_writeStream(FILE *stream, const PrintArg *args, bytes count);

#if defined(__SIZEOF_INT128__)
#define PRINT_WRAP_128 \
    i128:               PrintArg_fromI128, \
    u128:               PrintArg_fromU128,
#else
#define PRINT_WRAP_128
#endif

#define PRINT_WRAP(x) _Generic((x), \
    typeof(nullptr):    PrintArg_fromNull, \
    bool:               PrintArg_fromBool, \
    utf8:               PrintArg_fromChar, \
    signed char:        PrintArg_fromSigned, \
    short:              PrintArg_fromSigned, \
    int:                PrintArg_fromSigned, \
    long:               PrintArg_fromSigned, \
    long long:          PrintArg_fromSigned, \
    unsigned char:      PrintArg_fromUnsigned, \
    unsigned short:     PrintArg_fromUnsigned, \
    unsigned int:       PrintArg_fromUnsigned, \
    unsigned long:      PrintArg_fromUnsigned, \
    unsigned long long: PrintArg_fromUnsigned, \
    PRINT_WRAP_128 \
    f32:                PrintArg_fromFloat, \
    d64:                PrintArg_fromFloat, \
    b16:                PrintArg_fromB16, \
    utf8*:              PrintArg_fromCStr, \
    string:             PrintArg_fromCStr, \
    varchar:            PrintArg_fromVarchar, \
    if64:               PrintArg_fromIf64, \
    ld128:              PrintArg_fromLd128, \
    vec2b:              PrintArg_fromVec2b, \
    vec2f:              PrintArg_fromVec2f, \
    vec2d:              PrintArg_fromVec2d, \
    vec3b:              PrintArg_fromVec3b, \
    vec3f:              PrintArg_fromVec3f, \
    vec3d:              PrintArg_fromVec3d, \
    vec4b:              PrintArg_fromVec4b, \
    vec4f:              PrintArg_fromVec4f, \
    vec4d:              PrintArg_fromVec4d, \
    mat2f:              PrintArg_fromMat2f, \
    mat2d:              PrintArg_fromMat2d, \
    mat3f:              PrintArg_fromMat3f, \
    mat3d:              PrintArg_fromMat3d, \
    mat4f:              PrintArg_fromMat4f, \
    mat4d:              PrintArg_fromMat4d, \
    quat32:             PrintArg_fromQuat32, \
    quat64:             PrintArg_fromQuat64, \
    default:            PrintArg_fromPtr \
)(x)

#define PRINT_MAP_0()
#define PRINT_MAP_1(a) PRINT_WRAP(a)
#define PRINT_MAP_2(a, ...)  PRINT_WRAP(a), PRINT_MAP_1(__VA_ARGS__)
#define PRINT_MAP_3(a, ...)  PRINT_WRAP(a), PRINT_MAP_2(__VA_ARGS__)
#define PRINT_MAP_4(a, ...)  PRINT_WRAP(a), PRINT_MAP_3(__VA_ARGS__)
#define PRINT_MAP_5(a, ...)  PRINT_WRAP(a), PRINT_MAP_4(__VA_ARGS__)
#define PRINT_MAP_6(a, ...)  PRINT_WRAP(a), PRINT_MAP_5(__VA_ARGS__)
#define PRINT_MAP_7(a, ...)  PRINT_WRAP(a), PRINT_MAP_6(__VA_ARGS__)
#define PRINT_MAP_8(a, ...)  PRINT_WRAP(a), PRINT_MAP_7(__VA_ARGS__)
#define PRINT_MAP_9(a, ...)  PRINT_WRAP(a), PRINT_MAP_8(__VA_ARGS__)
#define PRINT_MAP_10(a, ...) PRINT_WRAP(a), PRINT_MAP_9(__VA_ARGS__)
#define PRINT_MAP_11(a, ...) PRINT_WRAP(a), PRINT_MAP_10(__VA_ARGS__)
#define PRINT_MAP_12(a, ...) PRINT_WRAP(a), PRINT_MAP_11(__VA_ARGS__)
#define PRINT_MAP_13(a, ...) PRINT_WRAP(a), PRINT_MAP_12(__VA_ARGS__)
#define PRINT_MAP_14(a, ...) PRINT_WRAP(a), PRINT_MAP_13(__VA_ARGS__)
#define PRINT_MAP_15(a, ...) PRINT_WRAP(a), PRINT_MAP_14(__VA_ARGS__)
#define PRINT_MAP_16(a, ...) PRINT_WRAP(a), PRINT_MAP_15(__VA_ARGS__)

#define PRINT_COUNT_IMPL(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, N, ...) N
#define PRINT_COUNT(...) \
    PRINT_COUNT_IMPL(dummy __VA_OPT__(,) __VA_ARGS__, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define PRINT_CAT_INNER(a, b) a##b
#define PRINT_CAT(a, b) PRINT_CAT_INNER(a, b)
#define PRINT_MAP(...) PRINT_CAT(PRINT_MAP_, PRINT_COUNT(__VA_ARGS__))(__VA_ARGS__)

#define fprint(stream, ...) \
    Print_writeStream( \
        (stream), \
        (PRINT_COUNT(__VA_ARGS__) > 0 ? (const PrintArg[]){ __VA_OPT__(PRINT_MAP(__VA_ARGS__),) { .kind = PRINT_ARG_NULL } } : nullptr), \
        (bytes) PRINT_COUNT(__VA_ARGS__))

#define print(...) fprint(stdout __VA_OPT__(,) __VA_ARGS__)

#endif
