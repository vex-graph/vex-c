// primitives_test.c — the width vocabulary, proven at compile time.
//
// Every assertion here is a _Static_assert: the vocabulary is correct or the
// translation unit does not compile. The runtime body exists only so the test
// unit has an entry point and prints what it proved.

#include <math.h>
#include <stdatomic.h>
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

_Static_assert(_Generic((bytes) 0, bytes: 1, default: 0),
               "bytes must be bytes, the type sizeof returns");
_Static_assert(sizeof(bytes) == sizeof(sizeof(0)), "bytes must agree with sizeof");

_Static_assert(sizeof(f32) == 4, "f32 must be 32 bits");
_Static_assert(sizeof(d64) == 8, "d64 must be 64 bits");

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

_Static_assert(sizeof(b16) == 2, "b16 must be 2 bytes: bfloat16 is a 16-bit format");
_Static_assert(sizeof(if64) == 8, "if64 must be one 64-bit word");
_Static_assert(_Alignof(if64) == 8, "if64 must align to 8 for the atomic form");
_Static_assert(offsetof(if64, fraction) == 4, "if64.fraction must sit at offset 4");
_Static_assert(sizeof(ld128) == 16, "ld128 must be 128 bits");
_Static_assert(_Alignof(ld128) == 8, "ld128 must align to 8");
_Static_assert(offsetof(ld128, fraction) == 8, "ld128.fraction must sit at offset 8");
_Static_assert(sizeof(aif64) == 8, "aif64 must be one 64-bit word");
_Static_assert(_Alignof(aif64) == 8, "aif64 must be naturally aligned");
_Static_assert(sizeof(ald128) == 16, "ald128 must be 128 bits");
_Static_assert(_Alignof(ald128) == 8, "ald128 must align to 8");
_Static_assert(B_TEST_SKIP == 77, "the shared skip code stays 77");
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

    {
        int failures = 0;

        au32 counter = 0;
        atomic_fetch_add(&counter, 1);
        if (atomic_load(&counter) != 1) failures++;

        af32 f = 9;
        if (atomic_load(&f) != 9.0f) failures++;

        ai64 signedCounter = -1;
        atomic_fetch_add(&signedCounter, 2);
        if (atomic_load(&signedCounter) != 1) failures++;

        aif64 atomicPair;
        atomic_init(&atomicPair, if64_2(1, 0.5f));
        if (if64_toF32(atomic_load(&atomicPair)) != 1.5f) failures++;
        atomic_store(&atomicPair, if64_2(2, 0.25f));
        if (if64_toF32(atomic_load(&atomicPair)) != 2.25f) failures++;

        ald128 halves = { .integer = 0, .fraction = 0.0 };
        atomic_store(&halves.integer, 7);
        atomic_store(&halves.fraction, 0.25);
        if (atomic_load(&halves.integer) != 7) failures++;
        if (atomic_load(&halves.fraction) != 0.25) failures++;

        if (failures != 0) {
            fprintf(stderr, "primitives_test: %d atomic checks failed\n", failures);
            return 1;
        }
    }

    {
        int failures = 0;

        if64 a = if64("1.375");
        if (if64_integer(a) != 1 || if64_fraction(a) != 0.375f) failures++;
        if (if64_toF32(a) != 1.375f) failures++;

        if64 negative = if64("-1.5");
        if (if64_integer(negative) != -1 || if64_fraction(negative) != -0.5f) failures++;

        if64 pair = if64_2(7, 0.25f);
        if (if64_toF32(pair) != 7.25f) failures++;

        if64 nonFinite = if64("nan");
        if (!isnan(if64_fraction(nonFinite))) failures++;

        ld128 b = ld128("1.375");
        if (ld128_integer(b) != 1 || fabs(ld128_fraction(b) - 0.375) > 1e-12) failures++;
        if (ld128_toD64(b) != 1.375) failures++;

        ld128 big = ld128("9223372036854775807.25");
        if (ld128_integer(big) != INT64_MAX || ld128_fraction(big) != 0.25) failures++;

        ld128 pair128 = ld128_2(-9, -0.5);
        if (ld128_toD64(pair128) != -9.5) failures++;

        vec3f coordinate = vec3f(if64_toF32(if64_2(1, 0.5f)), 2.0f, 3.0f);
        if (coordinate.data[0] != 1.5f) failures++;

        if (failures != 0) {
            fprintf(stderr, "primitives_test: %d if64/ld128 checks failed\n", failures);
            return 1;
        }
    }

    printf("primitives_test: if64/ld128 checks passed\n");

    printf("primitives_test: all width assertions held\n");
    return 0;
}
