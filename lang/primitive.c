/**
 * primitive.c, the bfloat16 bit-pattern bridge.
 *
 * primitive.h carries the scalar vocabulary and the struct; this translation
 * unit owns the conversions, because a header is an include surface and not a
 * place for function bodies. bfloat16 is a storage format: the arithmetic
 * surface widens to f32 first.
 */

#include "vex.h"
#include "lang/primitive.h"

;;DEFINITION
/**
 * ============================================================================
 * DEFINITION: b16
 * ============================================================================
 * Floor bfloat16 storage scalar (8-bit exponent, 7-bit mantissa, 2 bytes)
 * owned by lib/lang. Carries bit patterns without assuming compiler arithmetic
 * support and converts explicitly to/from f32 with round-to-nearest-even.
 * ============================================================================
 */

;;OVERVIEW
/**
 * ============================================================================
 * CLASS: b16 (lib/lang/primitive.c)
 * ============================================================================
 * 16-bit bfloat16 storage value wrapping a single u16 bit pattern.
 *
 * --FIELDS-------------------------------
 * public   u16          bits              raw IEEE bfloat16 bit pattern
 *
 * --FUNCTIONS----------------------------
 * public   b16_from_f32(...)              convert f32 to b16 (round-to-even)
 * public   b16_to_f32(...)                widen b16 bit pattern to f32
 *
 * --GETTERS------------------------------
 * public   b16_bits(...)                  read raw 16-bit pattern by value
 *
 * --SETTERS------------------------------
 * public   b16_setBits(...)               write raw 16-bit pattern
 * ============================================================================
 */

;;GETTER
u16 b16_bits(b16 self)
{
    return self.bits;
}

;;SETTER
fn b16_setBits(b16 *self, u16 value)
{
    (*self).bits = value;
}

b16 b16_from_f32(f32 value) {
    union { f32 as_f32; u32 as_u32; } pun = { .as_f32 = value };
    u32 raw = pun.as_u32;
    b16 out = { 0 };

    if ((raw & 0x7f800000u) == 0x7f800000u && (raw & 0x007fffffu) != 0) {
        b16_setBits(&out, (u16) ((raw >> 16) | 0x0040u));
        return out;
    }

    raw += 0x7fffu + ((raw >> 16) & 1u);
    b16_setBits(&out, (u16) (raw >> 16));
    return out;
}

f32 b16_to_f32(b16 value) {
    union { f32 as_f32; u32 as_u32; } pun = { .as_u32 = (u32) b16_bits(value) << 16 };
    return pun.as_f32;
}
