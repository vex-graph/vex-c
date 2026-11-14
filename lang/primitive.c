// primitive.c — the bfloat16 bit-pattern bridge.
//
// primitive.h carries the scalar vocabulary and the struct; this translation
// unit owns the conversions, because a header is an include surface and not a
// place for function bodies. bfloat16 is a storage format: the arithmetic
// surface widens to f32 first.

#include "vex.h"
#include "lang/primitive.h"

;;OVERVIEW
;;DEFINITION

;;GETTER
u16 b16_bits(b16 self) { return self.bits; }

;;SETTER
fn b16_setBits(b16 *self, u16 value) { (*self).bits = value; }

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
