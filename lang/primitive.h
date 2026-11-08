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
// It is `size_t`, not `u64`: on the six-target floor both are 8 bytes, but only
// `size_t` is what `sizeof`, `%zu`, `memcpy` and the C interfaces themselves
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

_Static_assert(_Generic((bytes) 0, size_t: 1, default: 0),          "bytes must be size_t, the type sizeof returns");
_Static_assert(sizeof(bytes) == 8,                                  "bytes must be 64-bit on the six-target floor");


// f32 and d64 are the floor: binary32 and binary64, 4 and 8 bytes on every
// target we ship.
typedef float  f32;
typedef double d64;


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


// 128-bit widths are not in the six-target floor — MSVC has no __int128, and
// Apple clang has no __float128 in the same spelling. GCC and clang have the
// integer form; code that uses it states that dependency rather than pretending
// it is portable.
#if defined(__SIZEOF_INT128__)
typedef unsigned __int128 u128;
typedef __int128          i128;
#endif

#endif
