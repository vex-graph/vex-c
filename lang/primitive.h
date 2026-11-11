#ifndef VEXC_PRIMITIVE_H
#define VEXC_PRIMITIVE_H

// primitive.h — the one spelling of a width.
//
// The width is the name; the platform is never the name. `int`, `long`,
// `long long` and `uint64_t` do not appear in ecosystem source. A width that
// cannot be exactly 8, 16, 32 or 64 bits on all six canonical targets does not
// get a spelling here — it is a struct, and it lives where it is used.
//
// vex-c owns the C built-in scalar vocabulary. Composite math value types are
// the other two math headers: vector.h owns vec2/vec3/vec4 and matrix.h owns
// mat2/mat3/mat4, in the same widths.

#include <stddef.h>
#include <stdint.h>

#include "annotation.h"
#include "constructor.h"

typedef uint8_t       u8;
typedef int8_t        i8;
typedef uint16_t     u16;
typedef int16_t      i16;
typedef uint32_t     u32;
typedef int32_t      i32;
typedef uint64_t     u64;
typedef int64_t      i64;

// `bytes` is the type `sizeof` yields — the one spelling for an object size:
//
//     bytes n = sizeof(*value);
//
// It is `bytes`, not `u64`: on the six-target floor both are 8 bytes, but only
// `bytes` is what `sizeof`, `%zu`, `memcpy` and the C interfaces themselves
// speak. A width-spelled size would be a cast at every seam.
//
// `bytes` is a TYPE, never a field name. A field says what it counts:
//
//     bytes count;      // number of elements
//     bytes capacity;   // allocated slots
//     bytes stride;     // bytes between records
//     bytes offset;     // byte position in a buffer
//     bytes padding;    // bytes inserted for alignment
//     bytes alignment;  // required alignment
//     bytes width;      // a dimension, in native pixels
//     bytes height;
//     bytes length;     // a byte count; the type carries the unit
//
// `padding` is alignment filler, never a synonym for object size.
typedef size_t bytes;

_Static_assert(_Generic((bytes) 0, bytes: 1, default: 0),          "bytes must be bytes, the type sizeof returns");
_Static_assert(sizeof(bytes) == 8,                                  "bytes must be 64-bit on the six-target floor");


// f32 and d64 are the floor: binary32 and binary64, 4 and 8 bytes on every
// target we ship.
typedef float  f32;
typedef double d64;


// Atomic spellings: a = atomic, then the scalar spelling. The atomic form
// carries the same width as its scalar on the six targets; the type is the
// vocabulary, not a promise of lock-free arithmetic. Use atomic_load,
// atomic_store, atomic_exchange and CAS for every one of them.
typedef _Atomic i8  ai8;
typedef _Atomic i16 ai16;
typedef _Atomic i32 ai32;
typedef _Atomic i64 ai64;

typedef _Atomic u8  au8;
typedef _Atomic u16 au16;
typedef _Atomic u32 au32;
typedef _Atomic u64 au64;

typedef _Atomic f32 af32;
typedef _Atomic d64 ad64;

_Static_assert(sizeof(ai8) == 1, "ai8 must be one 8-bit word");
_Static_assert(sizeof(ai16) == 2, "ai16 must be one 16-bit word");
_Static_assert(sizeof(ai32) == 4, "ai32 must be one 32-bit word");
_Static_assert(sizeof(ai64) == 8, "ai64 must be one 64-bit word");
_Static_assert(sizeof(au8) == 1, "au8 must be one 8-bit word");
_Static_assert(sizeof(au16) == 2, "au16 must be one 16-bit word");
_Static_assert(sizeof(au32) == 4, "au32 must be one 32-bit word");
_Static_assert(sizeof(au64) == 8, "au64 must be one 64-bit word");
_Static_assert(sizeof(af32) == 4, "af32 must be one 32-bit word");
_Static_assert(sizeof(ad64) == 8, "ad64 must be one 64-bit word");


// `b16` is bfloat16: 8-bit exponent, 7-bit mantissa, 2 bytes. It is NOT
// `_Float16` — that is IEEE binary16, 5-bit exponent, 10-bit mantissa. Same
// width, different type, and the difference is the whole point: bfloat16 keeps
// f32's exponent range for AI quantisation, at the cost of mantissa bits.
//
// No compiler on the six-target floor ships bfloat16 as an arithmetic scalar:
// Apple clang has `__bf16` but no feature macro to gate it (`__BFLT16_MAX__`
// is undefined and `__has_extension(c_bfloat16)` is false), Windows/MSVC has
// neither, and `_BFloat16` is not universal. So the floor carries the bit
// pattern and converts explicitly:
//
//     b16 half = b16_from_f32(1.5f);
//     f32 back = b16_to_f32(half);
//
// Rounding is round-to-nearest-even; NaN keeps its quiet bit and stays NaN.
// Arithmetic is done in f32: b16 is a storage format, not a compute type.
typedef struct b16 { u16 bits; } b16;

;;GETTER
u16 b16_bits(b16 self);

;;SETTER
void b16_setBits(b16 *self, u16 value);

b16 b16_from_f32(f32 value);
f32 b16_to_f32(b16 value);

_Static_assert(sizeof(b16) == 2, "b16 must be 2 bytes: bfloat16 is a 16-bit format");


// The two composite numeric values: 64 bits and 128 bits, nothing between.
// The integer half is the truncation and the fraction is the signed remainder
// in [-1, 1), so the pair is exact about magnitude and approximate about the
// fractional tail. No 96-bit form exists: 32+64 and 64+32 only pad to 16 bytes.
//
// Rendering coordinates come from if64_toF32 / ld128_toD64; the floor's
// vec/mat constructors consume those f32/d64 values.
typedef struct if64 {
    _Alignas(8) i32 integer;
    f32 fraction;
} if64;

_Static_assert(sizeof(if64) == 8,                   "if64 must be one 64-bit word");
_Static_assert(_Alignof(if64) == 8,                 "if64 must align to 8 for a lock-free atomic form");
_Static_assert(offsetof(if64, fraction) == 4,       "if64.fraction must sit at offset 4");

//
// same thing as the document below
//
typedef struct ld128 {
    i64 integer;
    d64 fraction;
} ld128;

_Static_assert(sizeof(ld128) == 16,                 "ld128 must be 128 bits");
_Static_assert(_Alignof(ld128) == 8,                "ld128 must align to 8");
_Static_assert(offsetof(ld128, fraction) == 8,      "ld128.fraction must sit at offset 8");

// aif64 is one whole-word atomic: load/store/exchange/CAS move the pair as a
// single value. ald128 is deliberately two independently atomic halves, not a
// single 128-bit atomic: a reader may observe the halves from different
// updates. Use aif64 when the pair must be indivisible, or publish an ld128
// through an atomic pointer when a consistent snapshot is required.
typedef _Atomic if64 aif64;

typedef struct ald128 {
    ai64 integer;
    ad64 fraction;
} ald128;

_Static_assert(sizeof(aif64) == 8,                  "aif64 must be one 64-bit word");
_Static_assert(_Alignof(aif64) == 8,                "aif64 must be naturally aligned for lock-free access");
_Static_assert(sizeof(ald128) == 16,                "ald128 must be 128 bits");
_Static_assert(_Alignof(ald128) == 8,               "ald128 must align to 8");

if64 if64_0();
if64 if64_1(const char *text);
if64 if64_2(i32 integer, f32 fraction);
#define if64(...) CONSTRUCTOR_DISPATCH(if64, __VA_ARGS__)


//
// getters for this whole thing
//
;;GETTER
i32 if64_integer(if64 self);

;;GETTER
f32 if64_fraction(if64 self);

;;SETTER
void if64_setInteger(if64 *self, i32 value);

;;SETTER
void if64_setFraction(if64 *self, f32 value);

f32 if64_toF32(if64 value);
void if64_toString(const if64 *value, char *dest, bytes cap, bool *outTruncated);
void if64_toStringStruct(const if64 *value, char *dest, bytes cap, bool *outTruncated);

ld128 ld128_0();
ld128 ld128_1(const char *text);
ld128 ld128_2(i64 integer, d64 fraction);
#define ld128(...) CONSTRUCTOR_DISPATCH(ld128, __VA_ARGS__)

;;GETTER
i64 ld128_integer(ld128 self);

;;GETTER
d64 ld128_fraction(ld128 self);

;;SETTER
void ld128_setInteger(ld128 *self, i64 value);

;;SETTER
void ld128_setFraction(ld128 *self, d64 value);

d64 ld128_toD64(ld128 value);
void ld128_toString(const ld128 *value, char *dest, bytes cap, bool *outTruncated);
void ld128_toStringStruct(const ld128 *value, char *dest, bytes cap, bool *outTruncated);


// 128-bit widths are not in the six-target floor — MSVC has no __int128, and
// Apple clang has no __float128 in the same spelling. GCC and clang have the
// integer form; code that uses it states that dependency rather than pretending
// it is portable.
#if defined(__SIZEOF_INT128__)
typedef unsigned __int128 u128;
typedef __int128          i128;
#endif

// constants that some might matter
#define ZERO 0
#define ONE 1
#define ONE_THOUSAND 1'000
#define ONE_MILLION 1'000'000
#define ONE_BILLION 1'000'000'000

#endif
