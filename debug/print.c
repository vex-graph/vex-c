/** debug/print.c -- zero-allocation variadic console logger for vex-c. */

#include "vex.h"

;;DEFINITION
/**
 * ============================================================================
 * DEFINITION: Print
 * ============================================================================
 * Zero-allocation variadic console logger owned by lib/debug. Formats up to 16
 * typed PrintArg values (scalars, 128-bit ints, b16, if64, ld128, string, varchar,
 * vectors, matrices, quaternions, pointers) into a 4096-byte stack buffer and
 * writes one newline-terminated line to a FILE stream.
 * ============================================================================
 */

;;OVERVIEW
/**
 * ============================================================================
 * CLASS: PrintArg (lib/debug/print.c)
 * ============================================================================
 * Tagged union carrying one typed argument for Print_concatArgs / Print_writeStream.
 *
 * --CONSTANTS----------------------------
 * public   const bytes PRINT_BUFFER_BYTES  4096-byte stack output buffer
 *
 * --FIELDS-------------------------------
 * public   PrintArgKind kind              active union arm discriminator
 * public   union        value             typed payload for the active kind
 *
 * --CONSTRUCTORS-------------------------
 * public   PrintArg_from*(...)            typed argument wrappers for _Generic
 *
 * --FUNCTIONS----------------------------
 * public   Print_concatArgs(...)          concatenate PrintArg array into utf8 buffer
 * public   Print_writeStream(...)         format args on stack and write line to stream
 * private  printAppendBytes(...)          bounded raw byte append
 * private  printAppendCStr(...)           bounded NUL-terminated utf8* append
 * private  printAppendDouble(...)         bounded %g float append
 * private  printAppendU128(...)           bounded decimal u128 append
 * private  printAppendI128(...)           bounded decimal i128 append
 * private  printAppendArg(...)            dispatch one PrintArg by kind
 * ============================================================================
 */

/** ---------------------------------------------------------------- PrintArg constructors */

PrintArg PrintArg_fromNull(const_any ignored) {
    (discard) ignored;
    return (PrintArg){ .kind = PRINT_ARG_NULL };
}

PrintArg PrintArg_fromBool(bool value) {
    return (PrintArg){ .kind = PRINT_ARG_BOOL, .value.b = value };
}

PrintArg PrintArg_fromChar(utf8 value) {
    return (PrintArg){ .kind = PRINT_ARG_CHAR, .value.ch = value };
}

PrintArg PrintArg_fromSigned(i64 value) {
    return (PrintArg){ .kind = PRINT_ARG_SIGNED, .value.s = value };
}

PrintArg PrintArg_fromUnsigned(u64 value) {
    return (PrintArg){ .kind = PRINT_ARG_UNSIGNED, .value.u = value };
}

#if defined(__SIZEOF_INT128__)
PrintArg PrintArg_fromI128(i128 value) {
    return (PrintArg){ .kind = PRINT_ARG_I128, .value.s128 = value };
}

PrintArg PrintArg_fromU128(u128 value) {
    return (PrintArg){ .kind = PRINT_ARG_U128, .value.u128 = value };
}
#endif

PrintArg PrintArg_fromFloat(d64 value) {
    return (PrintArg){ .kind = PRINT_ARG_FLOAT, .value.d = value };
}

PrintArg PrintArg_fromB16(b16 value) {
    return (PrintArg){ .kind = PRINT_ARG_FLOAT, .value.d = (d64) b16_to_f32(value) };
}

PrintArg PrintArg_fromCStr(string value) {
    return (PrintArg){ .kind = PRINT_ARG_CSTR, .value.cstr = value };
}

PrintArg PrintArg_fromVarchar(varchar value)
{
    PrintArg arg;
    memset(&arg, 0, sizeof(arg));
    arg.kind = PRINT_ARG_VARCHAR;
    arg.value.vstr = value;
    return arg;
}

PrintArg PrintArg_fromPtr(const_any value) {
    return (PrintArg){ .kind = PRINT_ARG_PTR, .value.ptr = value };
}

PrintArg PrintArg_fromIf64(if64 value) {
    return (PrintArg){ .kind = PRINT_ARG_IF64, .value.pair64 = value };
}

PrintArg PrintArg_fromLd128(ld128 value) {
    return (PrintArg){ .kind = PRINT_ARG_LD128, .value.pair128 = value };
}

PrintArg PrintArg_fromVec2b(vec2b value) {
    return (PrintArg){ .kind = PRINT_ARG_VEC2F, .value.v2f = vec2b_to_vec2f(value) };
}

PrintArg PrintArg_fromVec2f(vec2f value) {
    return (PrintArg){ .kind = PRINT_ARG_VEC2F, .value.v2f = value };
}

PrintArg PrintArg_fromVec2d(vec2d value) {
    return (PrintArg){ .kind = PRINT_ARG_VEC2D, .value.v2d = value };
}

PrintArg PrintArg_fromVec3b(vec3b value) {
    return (PrintArg){ .kind = PRINT_ARG_VEC3F, .value.v3f = vec3b_to_vec3f(value) };
}

PrintArg PrintArg_fromVec3f(vec3f value) {
    return (PrintArg){ .kind = PRINT_ARG_VEC3F, .value.v3f = value };
}

PrintArg PrintArg_fromVec3d(vec3d value) {
    return (PrintArg){ .kind = PRINT_ARG_VEC3D, .value.v3d = value };
}

PrintArg PrintArg_fromVec4b(vec4b value) {
    return (PrintArg){ .kind = PRINT_ARG_VEC4F, .value.v4f = vec4b_to_vec4f(value) };
}

PrintArg PrintArg_fromVec4f(vec4f value) {
    return (PrintArg){ .kind = PRINT_ARG_VEC4F, .value.v4f = value };
}

PrintArg PrintArg_fromVec4d(vec4d value) {
    return (PrintArg){ .kind = PRINT_ARG_VEC4D, .value.v4d = value };
}

PrintArg PrintArg_fromMat2f(mat2f value) {
    return (PrintArg){ .kind = PRINT_ARG_MAT2F, .value.m2f = value };
}

PrintArg PrintArg_fromMat2d(mat2d value) {
    return (PrintArg){ .kind = PRINT_ARG_MAT2D, .value.m2d = value };
}

PrintArg PrintArg_fromMat3f(mat3f value) {
    return (PrintArg){ .kind = PRINT_ARG_MAT3F, .value.m3f = value };
}

PrintArg PrintArg_fromMat3d(mat3d value) {
    return (PrintArg){ .kind = PRINT_ARG_MAT3D, .value.m3d = value };
}

PrintArg PrintArg_fromMat4f(mat4f value) {
    return (PrintArg){ .kind = PRINT_ARG_MAT4F, .value.m4f = value };
}

PrintArg PrintArg_fromMat4d(mat4d value) {
    return (PrintArg){ .kind = PRINT_ARG_MAT4D, .value.m4d = value };
}

PrintArg PrintArg_fromQuat32(quat32 value) {
    return (PrintArg){ .kind = PRINT_ARG_QUAT32, .value.q32 = value };
}

PrintArg PrintArg_fromQuat64(quat64 value) {
    return (PrintArg){ .kind = PRINT_ARG_QUAT64, .value.q64 = value };
}

/** ---------------------------------------------------------------- Concatenation */

static bytes printAppendBytes(utf8 *dest, bytes cap, bytes pos, string src, bytes len) {
    if (pos + 1u >= cap || !src || len == 0u)
        return pos;
    bytes avail = cap - pos - 1u;
    bytes n = len < avail ? len : avail;
    memcpy(dest + pos, src, n);
    return pos + n;
}

static bytes printAppendCStr(utf8 *dest, bytes cap, bytes pos, string src) {
    if (!src)
        return printAppendBytes(dest, cap, pos, "null", 4u);
    return printAppendBytes(dest, cap, pos, src, strlen(src));
}

static bytes printAppendDouble(utf8 *dest, bytes cap, bytes pos, d64 value) {
    if (pos + 1u >= cap)
        return pos;
    bytes avail = cap - pos;
    i32 written = snprintf(dest + pos, avail, "%g", value);
    if (written <= 0)
        return pos;
    bytes advance = (bytes) written;
    return advance >= avail ? cap - 1u : pos + advance;
}

#if defined(__SIZEOF_INT128__)
static bytes printAppendU128(utf8 *dest, bytes cap, bytes pos, u128 value) {
    utf8 rev[48];
    bytes n = 0u;
    if (value == 0u)
        return printAppendBytes(dest, cap, pos, "0", 1u);
    while (value > 0u) {
        rev[n++] = (utf8) ('0' + (u32) (value % 10u));
        value /= 10u;
    }
    utf8 out[48];
    for (bytes i = 0u; i < n; i++)
        out[i] = rev[n - 1u - i];
    return printAppendBytes(dest, cap, pos, out, n);
}

static bytes printAppendI128(utf8 *dest, bytes cap, bytes pos, i128 value) {
    if (value < 0) {
        pos = printAppendBytes(dest, cap, pos, "-", 1u);
        u128 mag = (u128) (-(value + 1)) + 1u;
        return printAppendU128(dest, cap, pos, mag);
    }
    return printAppendU128(dest, cap, pos, (u128) value);
}
#endif

static bytes printAppendArg(utf8 *dest, bytes cap, bytes pos, PrintArg arg) {
    if (pos + 1u >= cap)
        return pos;
    bytes avail = cap - pos;
    i32 written = 0;

    switch (arg.kind) {
        case PRINT_ARG_NULL:
            return printAppendBytes(dest, cap, pos, "null", 4u);
        case PRINT_ARG_BOOL:
            return arg.value.b
                ? printAppendBytes(dest, cap, pos, "true", 4u)
                : printAppendBytes(dest, cap, pos, "false", 5u);
        case PRINT_ARG_CHAR:
            return printAppendBytes(dest, cap, pos, &arg.value.ch, 1u);
        case PRINT_ARG_SIGNED:
            written = snprintf(dest + pos, avail, "%" PRId64, arg.value.s);
            break;
        case PRINT_ARG_UNSIGNED:
            written = snprintf(dest + pos, avail, "%" PRIu64, arg.value.u);
            break;
#if defined(__SIZEOF_INT128__)
        case PRINT_ARG_I128:
            return printAppendI128(dest, cap, pos, arg.value.s128);
        case PRINT_ARG_U128:
            return printAppendU128(dest, cap, pos, arg.value.u128);
#endif
        case PRINT_ARG_FLOAT:
            return printAppendDouble(dest, cap, pos, arg.value.d);
        case PRINT_ARG_CSTR:
            return printAppendCStr(dest, cap, pos, arg.value.cstr);
        case PRINT_ARG_VARCHAR: {
            if (!arg.value.vstr)
                return printAppendBytes(dest, cap, pos, "null", 4u);

            bytes decoded = varchar_decodeUtf8(arg.value.vstr, dest + pos, avail, nullptr);
            return pos + decoded;
        }
        case PRINT_ARG_PTR:
            if (!arg.value.ptr)
                return printAppendBytes(dest, cap, pos, "null", 4u);
            written = snprintf(dest + pos, avail, "%p", arg.value.ptr);
            break;
        case PRINT_ARG_IF64: {
            utf8 tmp[64];
            if64_toString(&arg.value.pair64, tmp, sizeof tmp, nullptr);
            return printAppendCStr(dest, cap, pos, tmp);
        }
        case PRINT_ARG_LD128: {
            utf8 tmp[96];
            ld128_toString(&arg.value.pair128, tmp, sizeof tmp, nullptr);
            return printAppendCStr(dest, cap, pos, tmp);
        }
        case PRINT_ARG_VEC2F:
            written = snprintf(dest + pos, avail, "(%g, %g)",
                               (d64) vec2f_x(arg.value.v2f), (d64) vec2f_y(arg.value.v2f));
            break;
        case PRINT_ARG_VEC2D:
            written = snprintf(dest + pos, avail, "(%g, %g)",
                               vec2d_x(arg.value.v2d), vec2d_y(arg.value.v2d));
            break;
        case PRINT_ARG_VEC3F:
            written = snprintf(dest + pos, avail, "(%g, %g, %g)",
                               (d64) vec3f_x(arg.value.v3f),
                               (d64) vec3f_y(arg.value.v3f),
                               (d64) vec3f_z(arg.value.v3f));
            break;
        case PRINT_ARG_VEC3D:
            written = snprintf(dest + pos, avail, "(%g, %g, %g)",
                               vec3d_x(arg.value.v3d),
                               vec3d_y(arg.value.v3d),
                               vec3d_z(arg.value.v3d));
            break;
        case PRINT_ARG_VEC4F:
            written = snprintf(dest + pos, avail, "(%g, %g, %g, %g)",
                               (d64) vec4f_x(arg.value.v4f),
                               (d64) vec4f_y(arg.value.v4f),
                               (d64) vec4f_z(arg.value.v4f),
                               (d64) vec4f_w(arg.value.v4f));
            break;
        case PRINT_ARG_VEC4D:
            written = snprintf(dest + pos, avail, "(%g, %g, %g, %g)",
                               vec4d_x(arg.value.v4d),
                               vec4d_y(arg.value.v4d),
                               vec4d_z(arg.value.v4d),
                               vec4d_w(arg.value.v4d));
            break;
        case PRINT_ARG_MAT2F:
            pos = printAppendBytes(dest, cap, pos, "[", 1u);
            for (u32 r = 0u; r < 2u; r++) {
                if (r > 0u)
                    pos = printAppendBytes(dest, cap, pos, ", ", 2u);
                pos = printAppendBytes(dest, cap, pos, "[", 1u);
                for (u32 c = 0u; c < 2u; c++) {
                    if (c > 0u)
                        pos = printAppendBytes(dest, cap, pos, ", ", 2u);
                    pos = printAppendDouble(dest, cap, pos, (d64) mat2f_at(&arg.value.m2f, r, c));
                }
                pos = printAppendBytes(dest, cap, pos, "]", 1u);
            }
            return printAppendBytes(dest, cap, pos, "]", 1u);
        case PRINT_ARG_MAT2D:
            pos = printAppendBytes(dest, cap, pos, "[", 1u);
            for (u32 r = 0u; r < 2u; r++) {
                if (r > 0u)
                    pos = printAppendBytes(dest, cap, pos, ", ", 2u);
                pos = printAppendBytes(dest, cap, pos, "[", 1u);
                for (u32 c = 0u; c < 2u; c++) {
                    if (c > 0u)
                        pos = printAppendBytes(dest, cap, pos, ", ", 2u);
                    pos = printAppendDouble(dest, cap, pos, mat2d_at(&arg.value.m2d, r, c));
                }
                pos = printAppendBytes(dest, cap, pos, "]", 1u);
            }
            return printAppendBytes(dest, cap, pos, "]", 1u);
        case PRINT_ARG_MAT3F:
            pos = printAppendBytes(dest, cap, pos, "[", 1u);
            for (u32 r = 0u; r < 3u; r++) {
                if (r > 0u)
                    pos = printAppendBytes(dest, cap, pos, ", ", 2u);
                pos = printAppendBytes(dest, cap, pos, "[", 1u);
                for (u32 c = 0u; c < 3u; c++) {
                    if (c > 0u)
                        pos = printAppendBytes(dest, cap, pos, ", ", 2u);
                    pos = printAppendDouble(dest, cap, pos, (d64) mat3f_at(&arg.value.m3f, r, c));
                }
                pos = printAppendBytes(dest, cap, pos, "]", 1u);
            }
            return printAppendBytes(dest, cap, pos, "]", 1u);
        case PRINT_ARG_MAT3D:
            pos = printAppendBytes(dest, cap, pos, "[", 1u);
            for (u32 r = 0u; r < 3u; r++) {
                if (r > 0u)
                    pos = printAppendBytes(dest, cap, pos, ", ", 2u);
                pos = printAppendBytes(dest, cap, pos, "[", 1u);
                for (u32 c = 0u; c < 3u; c++) {
                    if (c > 0u)
                        pos = printAppendBytes(dest, cap, pos, ", ", 2u);
                    pos = printAppendDouble(dest, cap, pos, mat3d_at(&arg.value.m3d, r, c));
                }
                pos = printAppendBytes(dest, cap, pos, "]", 1u);
            }
            return printAppendBytes(dest, cap, pos, "]", 1u);
        case PRINT_ARG_MAT4F:
            pos = printAppendBytes(dest, cap, pos, "[", 1u);
            for (u32 r = 0u; r < 4u; r++) {
                if (r > 0u)
                    pos = printAppendBytes(dest, cap, pos, ", ", 2u);
                pos = printAppendBytes(dest, cap, pos, "[", 1u);
                for (u32 c = 0u; c < 4u; c++) {
                    if (c > 0u)
                        pos = printAppendBytes(dest, cap, pos, ", ", 2u);
                    pos = printAppendDouble(dest, cap, pos, (d64) mat4f_at(&arg.value.m4f, r, c));
                }
                pos = printAppendBytes(dest, cap, pos, "]", 1u);
            }
            return printAppendBytes(dest, cap, pos, "]", 1u);
        case PRINT_ARG_MAT4D:
            pos = printAppendBytes(dest, cap, pos, "[", 1u);
            for (u32 r = 0u; r < 4u; r++) {
                if (r > 0u)
                    pos = printAppendBytes(dest, cap, pos, ", ", 2u);
                pos = printAppendBytes(dest, cap, pos, "[", 1u);
                for (u32 c = 0u; c < 4u; c++) {
                    if (c > 0u)
                        pos = printAppendBytes(dest, cap, pos, ", ", 2u);
                    pos = printAppendDouble(dest, cap, pos, mat4d_at(&arg.value.m4d, r, c));
                }
                pos = printAppendBytes(dest, cap, pos, "]", 1u);
            }
            return printAppendBytes(dest, cap, pos, "]", 1u);
        case PRINT_ARG_QUAT32:
            written = snprintf(dest + pos, avail, "(%g, %g, %g, %g)",
                               (d64) quat32_x(arg.value.q32),
                               (d64) quat32_y(arg.value.q32),
                               (d64) quat32_z(arg.value.q32),
                               (d64) quat32_w(arg.value.q32));
            break;
        case PRINT_ARG_QUAT64:
            written = snprintf(dest + pos, avail, "(%g, %g, %g, %g)",
                               quat64_x(arg.value.q64),
                               quat64_y(arg.value.q64),
                               quat64_z(arg.value.q64),
                               quat64_w(arg.value.q64));
            break;
    }

    if (written <= 0)
        return pos;
    bytes advance = (bytes) written;
    if (advance >= avail)
        return cap - 1u;
    return pos + advance;
}

bytes Print_concatArgs(utf8 *dest, bytes cap, const PrintArg *args, bytes count) {
    if (!dest || cap == 0u)
        return 0u;
    bytes pos = 0u;
    if (args) {
        for (bytes i = 0u; i < count; i++)
            pos = printAppendArg(dest, cap, pos, args[i]);
    }
    dest[pos] = '\0';
    return pos;
}

fn Print_writeStream(FILE *stream, const PrintArg *args, bytes count) {
    if (!stream)
        return;
    utf8 buf[PRINT_BUFFER_BYTES];
    bytes len = Print_concatArgs(buf, sizeof buf, args, count);
    if (len > 0u)
        (discard) fwrite(buf, 1u, len, stream);
    (discard) fputc('\n', stream);
}
