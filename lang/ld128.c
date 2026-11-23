#include "vex.h"
#include "primitive.h"

#include "debug/throw.h"
#include "pair_parse.h"

;;DEFINITION
/**
 * ============================================================================
 * DEFINITION: ld128
 * ============================================================================
 * 128-bit split fixed/floating composite number (64-bit signed integer plus
 * d64 remainder in [-1, 1)) owned by lib/lang. Preserves exact 64-bit integer
 * magnitude alongside double-precision fractional remainder.
 * ============================================================================
 */

;;OVERVIEW
/**
 * ============================================================================
 * CLASS: ld128 (lib/lang/ld128.c)
 * ============================================================================
 * 16-byte integer+fraction scalar pair with exact comparison and
 * relative-to-origin f32/d64 projection.
 *
 * --FIELDS-------------------------------
 * public   i64          integer           truncated signed 64-bit integer half
 * public   d64          fraction          signed remainder in [-1, 1)
 *
 * --CONSTRUCTORS-------------------------
 * public   ld128_0()                      zeroed ld128 value
 * public   ld128_1(...)                   parse decimal or constant text
 * public   ld128_2(...)                   construct from integer and fraction
 *
 * --FUNCTIONS----------------------------
 * public   ld128_toD64(...)               collapse pair to d64
 * public   ld128_normalize(...)           carry overflow and normalize remainder
 * public   ld128_fromD64(...)             build normalized pair from d64
 * public   ld128_add(...)                 add two pairs with normalization
 * public   ld128_sub(...)                 subtract two pairs with normalization
 * public   ld128_neg(...)                 negate both halves
 * public   ld128_compare(...)             exact total ordering (-1, 0, 1)
 * public   ld128_toD64Relative(...)       d64 delta relative to an origin
 * public   ld128_toF32Relative(...)       f32 delta relative to an origin
 * public   ld128_toString(...)            value projection, cold path only
 * public   ld128_toStringStruct(...)      structure projection, cold path only
 *
 * --GETTERS------------------------------
 * public   ld128_integer(...)             read integer half by value
 * public   ld128_fraction(...)            read fraction half by value
 *
 * --SETTERS------------------------------
 * public   ld128_setInteger(...)          write integer half
 * public   ld128_setFraction(...)         write fraction half
 * ============================================================================
 */

;;GETTER
i64 ld128_integer(ld128 self) {
    return self.integer;
}

;;GETTER
d64 ld128_fraction(ld128 self) {
    return self.fraction;
}

;;SETTER
fn ld128_setInteger(ld128 *self, i64 value) {
    (*self).integer = value;
}

;;SETTER
fn ld128_setFraction(ld128 *self, d64 value) {
    (*self).fraction = value;
}

ld128 ld128_0() {
    return (ld128){ .integer = 0, .fraction = 0.0 };
}

ld128 ld128_1(string text) {
    PairParseResult parsed;
    PairParseStatus status = pair_parseText(text, INT64_MAX, &parsed);
    if (status != PAIR_PARSE_OK) {
        THROW("ld128: %s literal rejected",
              status == PAIR_PARSE_OVERFLOW ? "overflowing" : "malformed");
        return ld128_0();
    }
    if (parsed.nonFinite)
        return (ld128){ .integer = 0, .fraction = (d64) NAN };

    d64 fraction = parsed.fraction;
    if (fraction >= 1.0)
        fraction = nextafter(1.0, 0.0);
    else if (fraction <= -1.0)
        fraction = -nextafter(1.0, 0.0);
    return (ld128){ .integer = parsed.integer, .fraction = fraction };
}

ld128 ld128_2(i64 integer, d64 fraction) {
    if (isnan(fraction))
        return (ld128){ .integer = 0, .fraction = (d64) NAN };
    if (fraction < -1.0 || fraction >= 1.0) {
        THROW("ld128: fraction outside [-1, 1)");
        return ld128_0();
    }
    return (ld128){ .integer = integer, .fraction = fraction };
}

d64 ld128_toD64(ld128 value) {
    return (d64) ld128_integer(value) + ld128_fraction(value);
}

ld128 ld128_normalize(i64 integer, d64 fraction) {
    if (!isfinite(fraction)) {
        return (ld128){ .integer = 0, .fraction = (d64) NAN };
    }
    d64 carry = trunc(fraction);
    i64 total = integer + (i64) carry;
    d64 rem = fraction - carry;
    if (total > 0 && rem < 0.0) {
        total -= 1;
        rem += 1.0;
    } else if (total < 0 && rem > 0.0) {
        total += 1;
        rem -= 1.0;
    }
    if (rem >= 1.0) {
        total += 1;
        rem = 0.0;
    } else if (rem <= -1.0) {
        total -= 1;
        rem = 0.0;
    }
    return (ld128){ .integer = total, .fraction = rem };
}

ld128 ld128_fromD64(d64 value) {
    return ld128_normalize(0, value);
}

ld128 ld128_add(ld128 a, ld128 b) {
    i64 intSum = ld128_integer(a) + ld128_integer(b);
    d64 fracSum = ld128_fraction(a) + ld128_fraction(b);
    return ld128_normalize(intSum, fracSum);
}

ld128 ld128_sub(ld128 a, ld128 b) {
    i64 intDiff = ld128_integer(a) - ld128_integer(b);
    d64 fracDiff = ld128_fraction(a) - ld128_fraction(b);
    return ld128_normalize(intDiff, fracDiff);
}

ld128 ld128_neg(ld128 self) {
    return (ld128){ .integer = -ld128_integer(self), .fraction = -ld128_fraction(self) };
}

i32 ld128_compare(ld128 a, ld128 b) {
    i64 ia = ld128_integer(a), ib = ld128_integer(b);
    if (ia != ib)
        return (ia < ib) ? -1 : 1;
    d64 fa = ld128_fraction(a), fb = ld128_fraction(b);
    if (fa < fb)
        return -1;
    if (fa > fb)
        return 1;
    return 0;
}

d64 ld128_toD64Relative(ld128 value, ld128 origin) {
    i64 intDiff = ld128_integer(value) - ld128_integer(origin);
    d64 fracDiff = ld128_fraction(value) - ld128_fraction(origin);
    return (d64) intDiff + fracDiff;
}

f32 ld128_toF32Relative(ld128 value, ld128 origin) {
    return (f32) ld128_toD64Relative(value, origin);
}

fn ld128_toString(const ld128 *value, utf8 *dest, bytes cap, bool *outTruncated) {
    if (outTruncated)
        *outTruncated = false;
    if (!dest || cap == 0u)
        return;
    if (!value) {
        snprintf(dest, cap, "nullptr");
        return;
    }
    i32 written = snprintf(dest, cap, "ld128(integer=%" PRId64 ", fraction=%.15g)",
                           ld128_integer(*value), ld128_fraction(*value));
    if (written < 0 || (bytes) written >= cap) {
        if (outTruncated)
            *outTruncated = true;
    }
}

fn ld128_toStringStruct(const ld128 *value, utf8 *dest, bytes cap, bool *outTruncated) {
    if (outTruncated)
        *outTruncated = false;
    if (!dest || cap == 0u)
        return;
    if (!value) {
        snprintf(dest, cap, "nullptr");
        return;
    }
    u64 bits = 0;
    d64 fraction = ld128_fraction(*value);
    memcpy(&bits, &fraction, sizeof bits);
    i32 written = snprintf(dest, cap, "ld128 { integer=%" PRId64 ", fraction=0x%016" PRIx64 " }",
                           ld128_integer(*value), bits);
    if (written < 0 || (bytes) written >= cap) {
        if (outTruncated)
            *outTruncated = true;
    }
}
