// primitives_test.c — the width vocabulary, proven at compile time.
//
// Every assertion here is a _Static_assert: the vocabulary is correct or the
// translation unit does not compile. The runtime body exists only so the test
// unit has an entry point and prints what it proved.

#include <stdio.h>

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

_Static_assert(_Generic((bytes) 0, size_t: 1, default: 0),
               "bytes must be size_t, the type sizeof returns");
_Static_assert(sizeof(bytes) == sizeof(sizeof(0)), "bytes must agree with sizeof");

_Static_assert(sizeof(f32) == 4, "f32 must be 32 bits");
_Static_assert(sizeof(d64) == 8, "d64 must be 64 bits");

_Static_assert(sizeof(b16) == 2, "b16 must be 2 bytes: bfloat16 is a 16-bit format");
_Static_assert(_Generic((b16_bits((b16) { 0 })) , u16: 1, default: 0),
               "b16_bits must yield u16");

#if defined(__SIZEOF_INT128__)
_Static_assert(sizeof(u128) == 16, "u128 must be 128 bits on GCC and clang");
_Static_assert(sizeof(i128) == 16, "i128 must be 128 bits on GCC and clang");
#endif

int main() {
    printf("primitives_test: u8=%zu u16=%zu u32=%zu u64=%zu f32=%zu d64=%zu\n",
           sizeof(u8), sizeof(u16), sizeof(u32), sizeof(u64),
           sizeof(f32), sizeof(d64));

    printf("primitives_test: b16=%zu present\n", sizeof(b16));

    {
        union { f32 as_f32; u32 as_u32; } pun;
        int failures = 0;

        if (b16_bits(b16_from_f32(1.0f)) != 0x3f80u) failures++;
        if (b16_bits(b16_from_f32(-0.5f)) != 0xbf00u) failures++;
        if (b16_bits(b16_from_f32(1.5f)) != 0x3fc0u) failures++;
        if (b16_bits(b16_from_f32(0.1f)) != 0x3dcdu) failures++;
        if (b16_to_f32(b16_from_f32(1.5f)) != 1.5f) failures++;

        pun.as_u32 = 0x3f808000u;
        if (b16_bits(b16_from_f32(pun.as_f32)) != 0x3f80u) failures++;

        pun.as_u32 = 0x3f818000u;
        if (b16_bits(b16_from_f32(pun.as_f32)) != 0x3f82u) failures++;

        pun.as_u32 = 0x7f800001u;
        {
            f32 nan = b16_to_f32(b16_from_f32(pun.as_f32));
            if (nan == nan) failures++;   /* a NaN never equals itself */
        }

        if (failures != 0) {
            fprintf(stderr, "primitives_test: %d b16 checks failed\n", failures);
            return 1;
        }
    }

#if defined(__SIZEOF_INT128__)
    printf("primitives_test: u128=%zu present\n", sizeof(u128));
#else
    printf("primitives_test: u128 absent on this target\n");
#endif

    printf("primitives_test: all width assertions held\n");
    return 0;
}
