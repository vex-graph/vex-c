/**
 * lib_test.c, single-file automated owner suite for lib (primitives, varchar, vector, matrix, quaternion, print).
 * Run all: ./lib_test
 * Run one: ./lib_test primitive | varchar | vector | matrix | quaternion | print
 */

#include "vex.h"

_Static_assert(sizeof(u8) == 1, "u8 must be 8 bits");
_Static_assert(sizeof(i8) == 1, "i8 must be 8 bits");
_Static_assert(sizeof(u16) == 2, "u16 must be 16 bits");
_Static_assert(sizeof(i16) == 2, "i16 must be 16 bits");
_Static_assert(sizeof(u32) == 4, "u32 must be 32 bits");
_Static_assert(sizeof(i32) == 4, "i32 must be 32 bits");
_Static_assert(sizeof(u64) == 8, "u64 must be 64 bits");
_Static_assert(sizeof(i64) == 8, "i64 must be 64 bits");
_Static_assert((u8) -1 == 255, "u8 must be unsigned");
_Static_assert((i8) -1 == -1, "i8 must be signed");
_Static_assert(_Generic((bytes) 0, bytes: 1, default: 0), "bytes must be sizeof type");
_Static_assert(sizeof(bytes) == sizeof(sizeof(0)), "bytes must agree with sizeof");
_Static_assert(sizeof(f32) == 4, "f32 must be 32 bits");
_Static_assert(sizeof(d64) == 8, "d64 must be 64 bits");
_Static_assert(sizeof(ai8) == 1 && sizeof(au8) == 1, "8-bit atomics");
_Static_assert(sizeof(ai16) == 2 && sizeof(au16) == 2, "16-bit atomics");
_Static_assert(sizeof(ai32) == 4 && sizeof(au32) == 4, "32-bit atomics");
_Static_assert(sizeof(ai64) == 8 && sizeof(au64) == 8, "64-bit atomics");
_Static_assert(sizeof(af32) == 4 && sizeof(ad64) == 8, "float atomics");
_Static_assert(sizeof(b16) == 2, "b16 must be 2 bytes");
_Static_assert(sizeof(if64) == 8 && _Alignof(if64) == 8, "if64 layout");
_Static_assert(offsetof(if64, fraction) == 4, "if64.fraction offset");
_Static_assert(sizeof(ld128) == 16 && _Alignof(ld128) == 8, "ld128 layout");
_Static_assert(offsetof(ld128, fraction) == 8, "ld128.fraction offset");
_Static_assert(sizeof(aif64) == 8 && _Alignof(aif64) == 8, "aif64 layout");
_Static_assert(sizeof(ald128) == 16 && _Alignof(ald128) == 8, "ald128 layout");
_Static_assert(B_TEST_SKIP == 77, "shared skip code stays 77");
_Static_assert(_Generic((b16_bits((b16) { 0 })), u16: 1, default: 0), "b16_bits yields u16");
#if defined(__SIZEOF_INT128__)
_Static_assert(sizeof(u128) == 16 && sizeof(i128) == 16, "128-bit integers");
#endif

_Static_assert(sizeof(vec3b) == 4 * sizeof(b16), "vec3b lanes");
_Static_assert(sizeof(vec3f) == 4 * sizeof(f32), "vec3f lanes");
_Static_assert(sizeof(vec3d) == 4 * sizeof(d64), "vec3d lanes");
_Static_assert(sizeof(vec2f) == 2 * sizeof(f32), "vec2f lanes");
_Static_assert(sizeof(vec4d) == 4 * sizeof(d64), "vec4d lanes");
_Static_assert(_Alignof(vec3d) == 32 && _Alignof(vec4f) == 16, "vector alignments");

_Static_assert(sizeof(mat2f) == 2 * sizeof(vec2f), "mat2f columns");
_Static_assert(sizeof(mat3f) == 3 * sizeof(vec3f), "mat3f columns");
_Static_assert(sizeof(mat4f) == 4 * sizeof(vec4f), "mat4f columns");
_Static_assert(sizeof(mat2d) == 2 * sizeof(vec2d), "mat2d columns");
_Static_assert(sizeof(mat3d) == 3 * sizeof(vec3d), "mat3d columns");
_Static_assert(sizeof(mat4d) == 4 * sizeof(vec4d), "mat4d columns");
_Static_assert(_Alignof(mat3f) == 16 && _Alignof(mat4d) == 32, "matrix alignments");

_Static_assert(sizeof(quat32) == 16 && _Alignof(quat32) == 16, "quat32 layout");
_Static_assert(sizeof(quat64) == 32 && _Alignof(quat64) == 32, "quat64 layout");

static i32 failures;

/** Records a failed assertion without aborting the remaining suites. */
static fn check(i32 ok, string suite, string what) {
    if (ok)
        return;
    fprint(stderr, "lib_test [", suite, "]: FAIL ", what);
    failures++;
}

static i32 near_f32(f32 a, f32 b)
{
    return fabsf(a - b) < 1e-5f;
}
static i32 near_d64(d64 a, d64 b)
{
    return fabs(a - b) < 1e-11;
}

/** Proves b16 rounding/NaN, atomic scalar ops, and if64/ld128 parsing and math. */
static fn test_primitive() {
    union { f32 as_f32; u32 as_u32; } pun;

    check(b16_bits(b16_from_f32(1.0f)) == 0x3f80u, "primitive", "b16 1.0");
    check(b16_bits(b16_from_f32(-0.5f)) == 0xbf00u, "primitive", "b16 -0.5");
    check(b16_bits(b16_from_f32(1.5f)) == 0x3fc0u, "primitive", "b16 1.5");
    check(b16_bits(b16_from_f32(0.1f)) == 0x3dcdu, "primitive", "b16 0.1");
    check(b16_to_f32(b16_from_f32(1.5f)) == 1.5f, "primitive", "b16 roundtrip");

    pun.as_u32 = 0x3f808000u;
    check(b16_bits(b16_from_f32(pun.as_f32)) == 0x3f80u, "primitive", "b16 tie-to-even down");
    pun.as_u32 = 0x3f818000u;
    check(b16_bits(b16_from_f32(pun.as_f32)) == 0x3f82u, "primitive", "b16 tie-to-even up");
    pun.as_u32 = 0x7f800001u;
    f32 nan = b16_to_f32(b16_from_f32(pun.as_f32));
    check(nan != nan, "primitive", "b16 preserves NaN");

    au32 counter = 0;
    atomic_fetch_add(&counter, 1);
    check(atomic_load(&counter) == 1, "primitive", "au32 fetch_add");

    af32 f = 9;
    check(atomic_load(&f) == 9.0f, "primitive", "af32 load");

    ai64 signedCounter = -1;
    atomic_fetch_add(&signedCounter, 2);
    check(atomic_load(&signedCounter) == 1, "primitive", "ai64 fetch_add");

    aif64 atomicPair;
    atomic_init(&atomicPair, if64_2(1, 0.5f));
    check(if64_toF32(atomic_load(&atomicPair)) == 1.5f, "primitive", "aif64 init");
    atomic_store(&atomicPair, if64_2(2, 0.25f));
    check(if64_toF32(atomic_load(&atomicPair)) == 2.25f, "primitive", "aif64 store");

    ald128 halves = { .integer = 0, .fraction = 0.0 };
    atomic_store(&halves.integer, 7);
    atomic_store(&halves.fraction, 0.25);
    check(atomic_load(&halves.integer) == 7 && atomic_load(&halves.fraction) == 0.25, "primitive", "ald128 store");

    if64 a = if64("1.375");
    check(if64_integer(a) == 1 && if64_fraction(a) == 0.375f, "primitive", "if64 parse");
    check(if64_toF32(a) == 1.375f, "primitive", "if64 toF32");
    if64 negative = if64("-1.5");
    check(if64_integer(negative) == -1 && if64_fraction(negative) == -0.5f, "primitive", "if64 negative");
    check(if64_toF32(if64_2(7, 0.25f)) == 7.25f, "primitive", "if64_2");
    check(isnan(if64_fraction(if64("nan"))), "primitive", "if64 nan");

    ld128 b = ld128("1.375");
    check(ld128_integer(b) == 1 && fabs(ld128_fraction(b) - 0.375) <= 1e-12, "primitive", "ld128 parse");
    check(ld128_toD64(b) == 1.375, "primitive", "ld128 toD64");
    ld128 big = ld128("9223372036854775807.25");
    check(ld128_integer(big) == INT64_MAX && ld128_fraction(big) == 0.25, "primitive", "ld128 INT64_MAX");
    check(ld128_toD64(ld128_2(-9, -0.5)) == -9.5, "primitive", "ld128_2");

    vec3f coordinate = vec3f(if64_toF32(if64_2(1, 0.5f)), 2.0f, 3.0f);
    check(coordinate.data[0] == 1.5f, "primitive", "if64 into vec3f");

    if64 sum64 = if64_add(if64(10, 0.75f), if64(5, 0.5f));
    check(if64_integer(sum64) == 16 && near_f32(if64_fraction(sum64), 0.25f), "primitive", "if64_add carry");

    ld128 p1 = ld128(1000000000000LL, 0.25);
    ld128 p0 = ld128(1000000000000LL, 0.125);
    check(near_f32(ld128_toF32Relative(p1, p0), 0.125f), "primitive", "ld128_toF32Relative");
}

/**
 * Compares two NUL-terminated UTF-8 strings for exact byte equality.
 * @param a first string
 * @param b second string
 * @return true when both are non-null and have identical bytes
 */
static bool strEq(string a, string b)
{
    if (!a || !b)
        return false;

    bytes aLen = strlen(a);
    bytes bLen = strlen(b);

    if (aLen != bLen)
        return false;

    return memcmp(a, b, aLen) == 0;
}

/**
 * Checks whether a starts with the first n bytes of b.
 * @param a haystack string
 * @param b prefix string
 * @param n byte count to compare
 * @return true when a has at least n bytes matching b
 */
static bool strPrefixN(string a, string b, bytes n)
{
    if (!a || !b)
        return false;

    if (strlen(a) < n)
        return false;

    return memcmp(a, b, n) == 0;
}

/**
 * Checks whether haystack contains needle.
 * @param haystack string to search
 * @param needle   substring to find
 * @return true when needle occurs within haystack
 */
static bool strContains(string haystack, string needle)
{
    if (!haystack || !needle)
        return false;

    bytes hLen = strlen(haystack);
    bytes nLen = strlen(needle);

    if (nLen == 0u)
        return true;

    if (nLen > hLen)
        return false;

    for (bytes i = 0u; i <= hLen - nLen; i++)
    {
        if (memcmp(haystack + i, needle, nLen) == 0)
            return true;
    }

    return false;
}

/** Proves immutable string pointer and floor varchar encoding, limit, conversion, equality, and projections. */
static fn test_varchar()
{
    string hello = "hello world";
    check(hello != nullptr && strlen(hello) == 11u, "varchar", "immutable string typedef");

    check(TextFormat_isValid(FORMAT_UTF8) && strEq(TextFormat_name(FORMAT_UTF8), "UTF-8"), "varchar", "format utf8");
    check(TextFormat_isValid(FORMAT_ASCII) && TextFormat_unitBytes(FORMAT_ASCII) == 1u, "varchar", "format ascii");
    check(TextFormat_isValid(FORMAT_LATIN1) && strEq(TextFormat_name(FORMAT_LATIN1), "ISO-8859-1"), "varchar", "format latin1");
    check(TextFormat_isValid(FORMAT_UTF16) && TextFormat_unitBytes(FORMAT_UTF16) == 2u, "varchar", "format utf16");
    check(TextFormat_isValid(FORMAT_UTF32) && TextFormat_unitBytes(FORMAT_UTF32) == 4u, "varchar", "format utf32");
    check(TextFormat_isValid(FORMAT_CESU8) && strEq(TextFormat_name(FORMAT_CESU8), "CESU-8"), "varchar", "format cesu8");
    check(!TextFormat_isValid((TextFormat) 99) && TextFormat_unitBytes((TextFormat) 99) == 0u, "varchar", "format invalid");

    check(varchar_limit(nullptr) == 0u && varchar_length(nullptr) == 0u && varchar_byteLength(nullptr) == 0u, "varchar", "null getters");
    check(varchar_format(nullptr) == FORMAT_UTF8 && varchar_isEmpty(nullptr) && varchar_value(nullptr) == nullptr && varchar_string(nullptr) == nullptr, "varchar", "null getters extra");

    alignas(8) u8 bufEmpty[64];
    varchar empty = varchar_initUtf8Into(bufEmpty, sizeof(bufEmpty), 100u, nullptr, 0u, FORMAT_UTF8);
    check(empty != nullptr && varchar_limit(empty) == 100u && varchar_length(empty) == 0u && varchar_byteLength(empty) == 0u && varchar_isEmpty(empty), "varchar", "empty init");

    alignas(8) u8 bufHello[64];
    varchar vHello = varchar_initUtf8Into(bufHello, sizeof(bufHello), 100u, hello, 11u, FORMAT_UTF8);
    check(vHello != nullptr && varchar_limit(vHello) == 100u && varchar_length(vHello) == 11u && varchar_byteLength(vHello) == 11u, "varchar", "utf8 init");
    check(varchar_string(vHello) != nullptr && strEq(varchar_string(vHello), "hello world"), "varchar", "utf8 string borrow");
    check(varchar_equalsString(vHello, "hello world") && !varchar_equalsString(vHello, "hello"), "varchar", "equalsString");

    string multi = "A\xC3\xA9\xE6\x97\xA5\xF0\x9F\x94\xA5";
    bytes multiBytes = strlen(multi);
    alignas(8) u8 bufUtf8[128];
    varchar vUtf8 = varchar_initUtf8Into(bufUtf8, sizeof(bufUtf8), 4u, multi, multiBytes, FORMAT_UTF8);
    check(vUtf8 != nullptr && varchar_limit(vUtf8) == 4u && varchar_length(vUtf8) == 4u && varchar_byteLength(vUtf8) == 10u, "varchar", "unicode char limit vs bytes");

    alignas(8) u8 bufOverflow[128];
    varchar vOver = varchar_initUtf8Into(bufOverflow, sizeof(bufOverflow), 3u, multi, multiBytes, FORMAT_UTF8);
    check(vOver == nullptr, "varchar", "character limit overflow refusal");

    alignas(8) u8 bufUtf16[128];
    varchar vUtf16 = varchar_convertInto(vUtf8, FORMAT_UTF16, bufUtf16, sizeof(bufUtf16));
    check(vUtf16 != nullptr && varchar_format(vUtf16) == FORMAT_UTF16 && varchar_limit(vUtf16) == 4u && varchar_length(vUtf16) == 4u && varchar_byteLength(vUtf16) == 10u, "varchar", "convert utf8 to utf16");
    check(varchar_string(vUtf16) == nullptr, "varchar", "utf16 not borrowable as utf8 string");
    check(!varchar_equals(vUtf8, vUtf16) && varchar_equalsCodePoints(vUtf8, vUtf16) && varchar_equalsString(vUtf16, multi), "varchar", "cross-format equality utf16");

    alignas(8) u8 bufUtf32[128];
    varchar vUtf32 = varchar_convertInto(vUtf16, FORMAT_UTF32, bufUtf32, sizeof(bufUtf32));
    check(vUtf32 != nullptr && varchar_format(vUtf32) == FORMAT_UTF32 && varchar_length(vUtf32) == 4u && varchar_byteLength(vUtf32) == 16u, "varchar", "convert utf16 to utf32");
    check(varchar_equalsCodePoints(vUtf8, vUtf32) && varchar_equalsString(vUtf32, multi), "varchar", "cross-format equality utf32");

    alignas(8) u8 bufCesu[128];
    varchar vCesu = varchar_convertInto(vUtf32, FORMAT_CESU8, bufCesu, sizeof(bufCesu));
    check(vCesu != nullptr && varchar_format(vCesu) == FORMAT_CESU8 && varchar_length(vCesu) == 4u && varchar_byteLength(vCesu) == 12u, "varchar", "convert utf32 to cesu8");
    check(varchar_equalsCodePoints(vUtf8, vCesu) && varchar_equalsString(vCesu, multi), "varchar", "cross-format equality cesu8");

    alignas(8) u8 bufRoundTrip[128];
    varchar vBack = varchar_convertInto(vCesu, FORMAT_UTF8, bufRoundTrip, sizeof(bufRoundTrip));
    check(vBack != nullptr && varchar_equals(vUtf8, vBack), "varchar", "cesu8 to utf8 roundtrip");

    string cafe = "caf\xC3\xA9";
    alignas(8) u8 bufLatin[64];
    varchar vLatin = varchar_initUtf8Into(bufLatin, sizeof(bufLatin), 10u, cafe, strlen(cafe), FORMAT_LATIN1);
    check(vLatin != nullptr && varchar_length(vLatin) == 4u && varchar_byteLength(vLatin) == 4u && varchar_value(vLatin)[3] == 0xE9u, "varchar", "latin1 encode");
    check(varchar_equalsString(vLatin, cafe), "varchar", "latin1 equalsString");

    alignas(8) u8 bufBadAscii[64];
    check(varchar_initUtf8Into(bufBadAscii, sizeof(bufBadAscii), 10u, cafe, strlen(cafe), FORMAT_ASCII) == nullptr, "varchar", "ascii rejects non-ascii");
    check(varchar_initUtf8Into(bufBadAscii, sizeof(bufBadAscii), 10u, multi, multiBytes, FORMAT_LATIN1) == nullptr, "varchar", "latin1 rejects >U+00FF");

    const u8 overlongUtf8[2] = { 0xC0u, 0xAFu };
    check(varchar_initInto(bufBadAscii, sizeof(bufBadAscii), 10u, overlongUtf8, 2u, FORMAT_UTF8) == nullptr, "varchar", "utf8 rejects overlong");

    const u8 loneSurrogateUtf16[2] = { 0x00u, 0xD8u };
    check(varchar_initInto(bufBadAscii, sizeof(bufBadAscii), 10u, loneSurrogateUtf16, 2u, FORMAT_UTF16) == nullptr, "varchar", "utf16 rejects lone surrogate");

    utf8 text[32];
    bool truncated = true;
    varchar_toString(vUtf16, text, sizeof(text), &truncated);
    check(strEq(text, multi) && !truncated, "varchar", "toString utf16");

    utf8 tight[5];
    truncated = false;
    varchar_toString(vUtf8, tight, sizeof(tight), &truncated);
    check(strEq(tight, "A\xC3\xA9") && truncated, "varchar", "toString codepoint-safe truncation");

    truncated = false;
    varchar_toString(nullptr, tight, sizeof(tight), &truncated);
    check(strEq(tight, "null") && truncated, "varchar", "toString nullptr");

    utf8 structure[160];
    truncated = false;
    varchar_toStringStruct(vHello, structure, sizeof(structure), &truncated);
    check(strContains(structure, "format=UTF-8") && strContains(structure, "limit=100") && strContains(structure, "length=11") && strContains(structure, "value=\"hello world\"") && !truncated, "varchar", "toStringStruct");

    variable slot = {0};
    check(sizeof(slot) == 32u && variable_isEmpty(&slot), "variable", "32-byte empty layout");
    check(variable_init(&slot, "Hero.Pos_1", (pointer) 0x1234u), "variable", "fold and bind");
    check(!variable_isEmpty(&slot) && strEq(variable_name(&slot), "hero.pos_1") && variable_pointer(&slot) == 0x1234u, "variable", "folded name and pointer");
    check(variable_nameEquals(&slot, "HERO.POS_1") && !variable_nameEquals(&slot, "hero.pos_2"), "variable", "nameEquals case-folding");
    check(!variable_setName(&slot, "123456789012345678901234"), "variable", "reject >23 chars");
}

/** Proves vec2/vec3/vec4 operations across b16, f32, d64, and _Generic dispatch. */
static fn test_vector() {
    vec3f v = vec3f(1.0f, 2.0f, 3.0f);
    check(vec3f_x(v) == 1.0f && vec3f_y(v) == 2.0f && vec3f_z(v) == 3.0f && v.data[3] == 0.0f, "vector", "vec3f lanes");

    vec3f_setX(&v, 9.0f);
    check(vec3f_x(v) == 9.0f, "vector", "vec3f_setX");

    vec3f x_axis = vec3f(1.0f, 0.0f, 0.0f);
    vec3f y_axis = vec3f(0.0f, 1.0f, 0.0f);
    check(vec3f_dot(x_axis, y_axis) == 0.0f && vec3f_dot(x_axis, x_axis) == 1.0f, "vector", "vec3f_dot");

    vec3f z_axis = vec3f_cross(x_axis, y_axis);
    check(vec3f_x(z_axis) == 0.0f && vec3f_y(z_axis) == 0.0f && vec3f_z(z_axis) == 1.0f, "vector", "vec3f_cross");

    vec3f sum = vec3f_add(vec3f(1.0f, 2.0f, 3.0f), vec3f(4.0f, 5.0f, 6.0f));
    check(vec3f_x(sum) == 5.0f && vec3f_y(sum) == 7.0f && vec3f_z(sum) == 9.0f, "vector", "vec3f_add");

    vec3f diff = vec3f_sub(vec3f(4.0f, 5.0f, 6.0f), vec3f(1.0f, 2.0f, 3.0f));
    check(vec3f_x(diff) == 3.0f && vec3f_y(diff) == 3.0f && vec3f_z(diff) == 3.0f, "vector", "vec3f_sub");

    check(vec3f_x(vec3f_scale(x_axis, 4.0f)) == 4.0f, "vector", "vec3f_scale");
    check(vec3f_length(vec3f(3.0f, 4.0f, 0.0f)) == 5.0f, "vector", "vec3f_length");
    check(near_f32(vec3f_z(vec3f_normalize(vec3f(0.0f, 0.0f, 7.0f))), 1.0f), "vector", "vec3f_normalize");

    vec3f zero = vec3f_normalize(vec3f_0());
    check(vec3f_x(zero) == 0.0f && vec3f_y(zero) == 0.0f && vec3f_z(zero) == 0.0f, "vector", "normalize zero");
    check(vec3f_x(vec3f_1(5.0f)) == 5.0f, "vector", "vec3f splat");

    vec3b packed = vec3b_from_vec3f(vec3f(1.0f, -0.5f, 1.5f));
    vec3f widened = vec3b_to_vec3f(packed);
    check(vec3f_x(widened) == 1.0f && vec3f_y(widened) == -0.5f && vec3f_z(widened) == 1.5f, "vector", "vec3b roundtrip");

    vec2d point = vec2d(1.5, -2.5);
    check(vec2d_x(point) == 1.5 && vec2d_y(point) == -2.5, "vector", "vec2d lanes");
    check(near_d64(vec2d_length(vec2d(3.0, 4.0)), 5.0), "vector", "vec2d_length");

    vec4f rgba = vec4f(1.0f, 2.0f, 3.0f, 4.0f);
    check(vec4f_w(rgba) == 4.0f && vec4f_dot(rgba, rgba) == 30.0f, "vector", "vec4f dot");

    vec3f g3f = vec_add(vec3f(1.0f, 2.0f, 3.0f), vec3f(4.0f, 5.0f, 6.0f));
    check(vec3f_x(g3f) == 5.0f && vec3f_y(g3f) == 7.0f && vec3f_z(g3f) == 9.0f, "vector", "vec_add generic f32");

    vec3d g3d = vec_add(vec3d(1.0, 2.0, 3.0), vec3d(4.0, 5.0, 6.0));
    check(vec3d_x(g3d) == 5.0 && vec3d_y(g3d) == 7.0 && vec3d_z(g3d) == 9.0, "vector", "vec_add generic d64");
}

static mat2f mat2f_from(f32 a, f32 b, f32 c, f32 d) {
    mat2f m = mat2f_0();
    mat2f_setAt(&m, 0, 0, a);
    mat2f_setAt(&m, 0, 1, b);
    mat2f_setAt(&m, 1, 0, c);
    mat2f_setAt(&m, 1, 1, d);
    return m;
}

static mat3f mat3f_from_rows(f32 a, f32 b, f32 c, f32 d, f32 e, f32 f, f32 g, f32 h, f32 i) {
    mat3f m = mat3f_0();
    mat3f_setAt(&m, 0, 0, a); mat3f_setAt(&m, 0, 1, b); mat3f_setAt(&m, 0, 2, c);
    mat3f_setAt(&m, 1, 0, d); mat3f_setAt(&m, 1, 1, e); mat3f_setAt(&m, 1, 2, f);
    mat3f_setAt(&m, 2, 0, g); mat3f_setAt(&m, 2, 1, h); mat3f_setAt(&m, 2, 2, i);
    return m;
}

/** Proves mat2/mat3/mat4 accessors, constructors, multiplication, transpose, and determinant. */
static fn test_matrix() {
    mat3f id = mat3f_identity();
    check(mat3f_at(&id, 0, 0) == 1.0f && mat3f_at(&id, 1, 1) == 1.0f && mat3f_at(&id, 2, 2) == 1.0f, "matrix", "identity diag");
    check(mat3f_at(&id, 0, 1) == 0.0f && mat3f_at(&id, 2, 0) == 0.0f && id.col[0].data[3] == 0.0f, "matrix", "identity zero");

    mat3f_setAt(&id, 0, 1, 4.0f);
    check(mat3f_at(&id, 0, 1) == 4.0f, "matrix", "setAt");
    mat3f_setAt(&id, 0, 1, 0.0f);
    check(vec3f_y(mat3f_col(&id, 1)) == 1.0f, "matrix", "col getter");

    mat3f z = mat3f_0();
    check(mat3f_at(&z, 0, 0) == 0.0f && mat3f_at(&z, 2, 2) == 0.0f, "matrix", "mat3f_0");

    mat3f diag = mat3f_1(3.0f);
    check(mat3f_at(&diag, 0, 0) == 3.0f && mat3f_at(&diag, 1, 1) == 3.0f && mat3f_at(&diag, 0, 1) == 0.0f, "matrix", "mat3f_1");

    mat3f built = mat3f(vec3f(1.0f, 2.0f, 3.0f), vec3f(4.0f, 5.0f, 6.0f), vec3f(7.0f, 8.0f, 9.0f));
    check(mat3f_at(&built, 1, 0) == 2.0f && mat3f_at(&built, 2, 1) == 6.0f, "matrix", "column-major");

    mat3f sum = mat3f_add(id, diag);
    mat3f diff = mat3f_sub(diag, id);
    mat3f scaled = mat3f_scale(id, 2.0f);
    check(mat3f_at(&sum, 1, 1) == 4.0f && mat3f_at(&diff, 0, 0) == 2.0f && mat3f_at(&scaled, 2, 2) == 2.0f, "matrix", "arithmetic");

    mat2f a2 = mat2f_from(1.0f, 2.0f, 3.0f, 4.0f);
    mat2f b2 = mat2f_from(2.0f, 0.0f, 0.0f, 2.0f);
    mat2f p2 = mat2f_multiply(a2, b2);
    check(mat2f_at(&p2, 0, 0) == 2.0f && mat2f_at(&p2, 0, 1) == 4.0f && mat2f_at(&p2, 1, 0) == 6.0f && mat2f_at(&p2, 1, 1) == 8.0f, "matrix", "mat2f_multiply");

    mat2f t2 = mat2f_transpose(a2);
    check(mat2f_at(&t2, 0, 1) == 3.0f && mat2f_at(&t2, 1, 0) == 2.0f, "matrix", "mat2f_transpose");
    check(mat2f_determinant(a2) == -2.0f, "matrix", "mat2f_determinant");

    vec2f v2 = mat2f_multiplyVec(a2, vec2f(5.0f, 6.0f));
    check(vec2f_x(v2) == 17.0f && vec2f_y(v2) == 39.0f, "matrix", "mat2f_multiplyVec");

    mat3f a3 = mat3f_from_rows(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 10.0f);
    check(near_f32(mat3f_determinant(a3), -3.0f), "matrix", "mat3f_determinant");

    mat3f p3 = mat3f_multiply(a3, mat3f_identity());
    check(mat3f_at(&p3, 0, 2) == 3.0f && mat3f_at(&p3, 2, 0) == 7.0f, "matrix", "mat3f identity multiply");

    mat2d d2 = mat2d(2.0);
    check(mat2d_at(&d2, 0, 0) == 2.0 && mat2d_at(&d2, 1, 1) == 2.0 && mat2d_determinant(d2) == 4.0, "matrix", "mat2d");

    mat4d d4 = mat4d_1(2.0);
    check(near_d64(mat4d_determinant(d4), 16.0), "matrix", "mat4d_determinant");

    mat4f m4 = mat4f_identity();
    mat4f p4 = mat4f_multiply(m4, m4);
    check(mat4f_at(&p4, 3, 3) == 1.0f && mat4f_at(&p4, 0, 3) == 0.0f && near_f32(mat4f_determinant(m4), 1.0f), "matrix", "mat4f identity");
}

/** Proves quat32/quat64 identity, axis-angle rotation, inverse, composition, slerp, and mat3 conversion. */
static fn test_quaternion() {
    quat32 id32 = quat32();
    check(quat32_x(id32) == 0.0f && quat32_y(id32) == 0.0f && quat32_z(id32) == 0.0f && quat32_w(id32) == 1.0f, "quaternion", "quat32 identity");

    quat64 id64 = quat64();
    check(quat64_x(id64) == 0.0 && quat64_y(id64) == 0.0 && quat64_z(id64) == 0.0 && quat64_w(id64) == 1.0, "quaternion", "quat64 identity");

    f32 halfPi32 = 1.57079632679f;
    quat32 qz32 = quat_fromAxisAngle(vec3f(0.0f, 0.0f, 1.0f), halfPi32);
    vec3f vx32 = vec3f(1.0f, 0.0f, 0.0f);
    vec3f vy32 = quat_rotateVec3(qz32, vx32);
    check(near_f32(vec3f_x(vy32), 0.0f) && near_f32(vec3f_y(vy32), 1.0f) && near_f32(vec3f_z(vy32), 0.0f), "quaternion", "quat32 90 deg Z");

    vec3f back32 = quat_rotateVec3(quat_inverse(qz32), vy32);
    check(near_f32(vec3f_x(back32), 1.0f) && near_f32(vec3f_y(back32), 0.0f) && near_f32(vec3f_z(back32), 0.0f), "quaternion", "quat_inverse");

    vec3f vnx32 = quat_rotateVec3(quat_mul(qz32, qz32), vx32);
    check(near_f32(vec3f_x(vnx32), -1.0f) && near_f32(vec3f_y(vnx32), 0.0f), "quaternion", "quat_mul 180 deg");

    vec3f v45 = quat_rotateVec3(quat_slerp(id32, qz32, 0.5f), vx32);
    check(near_f32(vec3f_x(v45), 0.70710678f) && near_f32(vec3f_y(v45), 0.70710678f), "quaternion", "quat_slerp 45 deg");

    vec3f vm3 = mat3f_multiplyVec(quat_toMat3(qz32), vx32);
    check(near_f32(vec3f_x(vm3), vec3f_x(vy32)) && near_f32(vec3f_y(vm3), vec3f_y(vy32)), "quaternion", "quat_toMat3");

    d64 halfPi64 = 1.57079632679489661923;
    quat64 qz64 = quat_fromAxisAngle(vec3d(0.0, 0.0, 1.0), halfPi64);
    vec3d vy64 = quat_rotateVec3(qz64, vec3d(1.0, 0.0, 0.0));
    check(near_d64(vec3d_x(vy64), 0.0) && near_d64(vec3d_y(vy64), 1.0) && near_d64(vec3d_z(vy64), 0.0), "quaternion", "quat64 90 deg Z");
}

/** Proves variadic print() across all vex-c floor primitives, math types, and truncation bounds. */
static fn test_print() {
    utf8 buf[256];
    const PrintArg scalars[] = {
        PRINT_WRAP("fps="), PRINT_WRAP((i32) 60),
        PRINT_WRAP(" b="), PRINT_WRAP((bytes) 4096u),
        PRINT_WRAP(" f="), PRINT_WRAP(b16_from_f32(1.5f)),
        PRINT_WRAP(" ok="), PRINT_WRAP(true),
        PRINT_WRAP(" no="), PRINT_WRAP(false)
    };
    bytes n = Print_concatArgs(buf, sizeof buf, scalars, sizeof scalars / sizeof scalars[0]);
    check(n == strlen(buf) && strEq(buf, "fps=60 b=4096 f=1.5 ok=true no=false"),
          "print", "scalars bytes b16 bools");

    string tag = "vex";
    alignas(8) u8 vbuf[64];
    varchar vtag = varchar_initUtf8Into(vbuf, sizeof(vbuf), 16u, "ok", 2u, FORMAT_UTF16);
    const PrintArg composites[] = {
        PRINT_WRAP("tag="), PRINT_WRAP(tag),
        PRINT_WRAP(" v="), PRINT_WRAP(vtag),
        PRINT_WRAP(" vb="), PRINT_WRAP(vec3b_from_vec3f(vec3f(1.0f, 2.0f, 3.0f))),
        PRINT_WRAP(" m="), PRINT_WRAP(mat2f_identity()),
        PRINT_WRAP(" q="), PRINT_WRAP(quat32()),
        PRINT_WRAP(" p="), PRINT_WRAP(if64("1.5")),
        PRINT_WRAP(" n="), PRINT_WRAP(nullptr)
    };
    n = Print_concatArgs(buf, sizeof buf, composites, sizeof composites / sizeof composites[0]);
    check(n == strlen(buf) &&
          strEq(buf, "tag=vex v=ok vb=(1, 2, 3) m=[[1, 0], [0, 1]] q=(0, 0, 0, 1) p=if64(integer=1, fraction=0.5) n=null"),
          "print", "string varchar vec3b mat2f quat32 if64 nullptr");

#if defined(__SIZEOF_INT128__)
    const PrintArg wide[] = {
        PRINT_WRAP("i128="), PRINT_WRAP((i128) -1234567890123456789LL),
        PRINT_WRAP(" u128="), PRINT_WRAP((u128) 9876543210987654321ULL)
    };
    n = Print_concatArgs(buf, sizeof buf, wide, sizeof wide / sizeof wide[0]);
    check(n == strlen(buf) &&
          strEq(buf, "i128=-1234567890123456789 u128=9876543210987654321"),
          "print", "i128 u128");
#endif

    i32 local = 7;
    const PrintArg ptrArg[] = { PRINT_WRAP("ptr="), PRINT_WRAP(&local) };
    n = Print_concatArgs(buf, sizeof buf, ptrArg, 2u);
    check(n > 4u && strPrefixN(buf, "ptr=0x", 6u), "print", "generic pointer");

    n = Print_concatArgs(buf, sizeof buf, nullptr, 0u);
    check(n == 0u && buf[0] == '\0', "print", "zero args");

    utf8 tiny[8];
    const PrintArg truncArgs[] = { PRINT_WRAP("hello"), PRINT_WRAP(" "), PRINT_WRAP("world") };
    n = Print_concatArgs(tiny, sizeof tiny, truncArgs, 3u);
    check(n == 7u && strEq(tiny, "hello w"), "print", "truncation safety");

    print("lib_test print smoke: ", vtag, " ", (bytes) 64u, " ", vec2b_from_vec2f(vec2f(1.0f, 2.0f)), " ", mat2f_identity(), " ", true);
    print();
}

/** Runs all lib suites, or only the named suite when argv[1] is passed. */
i32 main(i32 argc, utf8 **argv) {
    string filter = argc > 1 ? argv[1] : nullptr;

    if (!filter || strEq(filter, "primitive"))
        test_primitive();

    if (!filter || strEq(filter, "varchar") || strEq(filter, "string") || strEq(filter, "variable"))
        test_varchar();

    if (!filter || strEq(filter, "vector"))
        test_vector();

    if (!filter || strEq(filter, "matrix"))
        test_matrix();

    if (!filter || strEq(filter, "quaternion"))
        test_quaternion();

    if (!filter || strEq(filter, "print"))
        test_print();

    if (failures != 0) {
        fprint(stderr, "lib_test: ", failures, " checks failed");
        return 1;
    }

    print("lib_test: PASS", filter ? " (" : "", filter ? filter : "", filter ? ")" : "");
    return 0;
}
