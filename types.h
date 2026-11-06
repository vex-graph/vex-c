#ifndef UTIL_TYPES_H
#define UTIL_TYPES_H

// util/types.h — the one spelling of a width.
//
// The width is the name; the platform is never the name. `int`, `long`,
// `long long` and `uint64_t` do not appear in ecosystem source. A width that
// cannot be exactly 8, 16, 32 or 64 bits on all six canonical targets does not
// get a spelling here — it is a struct, and it lives where it is used.
//
// `byte` is `u8`. `Byte` is still the boxed class in vexspoke/primitive until
// that layer is retired; the two must not be confused in new code.

#include <stdint.h>

typedef uint8_t  u8;
typedef int8_t   i8;
typedef uint16_t u16;
typedef int16_t  i16;
typedef uint32_t u32;
typedef int32_t  i32;
typedef uint64_t u64;
typedef int64_t  i64;

typedef u8 byte;

// f32 and f64 are the floor: `float` and `double` are 4 and 8 bytes on every
// target we ship. f16 exists on clang and gcc; a target without it gets a
// compile error at the use site, never a silent 4-byte substitute.
typedef float  f32;
typedef double f64;

#if defined(__FLT16_MAX__)
typedef _Float16 f16;
#endif

// 128-bit widths are not in the six-target floor — MSVC has no __int128, and
// Apple clang has no float128. GCC and clang have the integer form; code that
// uses it states that dependency rather than pretending it is portable.
#if defined(__SIZEOF_INT128__)
typedef unsigned __int128 u128;
typedef __int128          i128;
#endif

#endif
