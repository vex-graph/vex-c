// types_test.c — the width vocabulary, proven at compile time.
//
// Every assertion here is a _Static_assert: the vocabulary is correct or the
// translation unit does not compile. The runtime body exists only so the test
// unit has an entry point and prints what it proved.

#include <stdio.h>

#include "types.h"

_Static_assert(sizeof(u8) == 1, "u8 must be 8 bits");
_Static_assert(sizeof(i8) == 1, "i8 must be 8 bits");
_Static_assert(sizeof(u16) == 2, "u16 must be 16 bits");
_Static_assert(sizeof(i16) == 2, "i16 must be 16 bits");
_Static_assert(sizeof(u32) == 4, "u32 must be 32 bits");
_Static_assert(sizeof(i32) == 4, "i32 must be 32 bits");
_Static_assert(sizeof(u64) == 8, "u64 must be 64 bits");
_Static_assert(sizeof(i64) == 8, "i64 must be 64 bits");

_Static_assert(sizeof(byte) == 1, "byte must be one byte");
_Static_assert((u8) -1 == 255, "u8 must be unsigned");
_Static_assert((i8) -1 == -1, "i8 must be signed");

_Static_assert(sizeof(f32) == 4, "f32 must be 32 bits");
_Static_assert(sizeof(f64) == 8, "f64 must be 64 bits");

#if defined(__FLT16_MAX__)
_Static_assert(sizeof(f16) == 2, "f16 must be 16 bits when the target has one");
#endif

#if defined(__SIZEOF_INT128__)
_Static_assert(sizeof(u128) == 16, "u128 must be 128 bits on GCC and clang");
_Static_assert(sizeof(i128) == 16, "i128 must be 128 bits on GCC and clang");
#endif

int main(void) {
    printf("types_test: u8=%zu u16=%zu u32=%zu u64=%zu f32=%zu f64=%zu\n",
           sizeof(u8), sizeof(u16), sizeof(u32), sizeof(u64),
           sizeof(f32), sizeof(f64));

#if defined(__FLT16_MAX__)
    printf("types_test: f16=%zu present\n", sizeof(f16));
#else
    printf("types_test: f16 absent on this target\n");
#endif

#if defined(__SIZEOF_INT128__)
    printf("types_test: u128=%zu present\n", sizeof(u128));
#else
    printf("types_test: u128 absent on this target\n");
#endif

    printf("types_test: all width assertions held\n");
    return 0;
}
