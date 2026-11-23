#include "vex.h"
#include "primitive.h"

#include "debug/throw.h"
#include "pair_parse.h"

;;DEFINITION
/**
 * ============================================================================
 * DEFINITION: if64
 * ============================================================================
 * 64-bit split fixed/floating composite number (32-bit signed integer plus
 * f32 remainder in [-1, 1)) owned by lib/lang. Preserves exact integer
 * magnitude alongside fractional precision for world coordinates.
 * ============================================================================
 */

;;OVERVIEW
/**
 * ============================================================================
 * CLASS: if64 (lib/lang/if64.c)
 * ============================================================================
 * 8-byte aligned integer+fraction scalar pair with exact comparison and
 * relative-to-origin f32 projection.
 *
 * --FIELDS-------------------------------
 * public   i32          integer           truncated signed integer half
 * public   f32          fraction          signed remainder in [-1, 1)
 *
 * --CONSTRUCTORS-------------------------
 * public   if64_0()                       zeroed if64 value
 * public   if64_1(...)                    parse decimal or constant text
 * public   if64_2(...)                    construct from integer and fraction
 *
 * --FUNCTIONS----------------------------
 * public   if64_toF32(...)                collapse pair to f32
 * public   if64_normalize(...)            carry overflow and normalize remainder
 * public   if64_fromF32(...)              build normalized pair from f32
 * public   if64_fromD64(...)              build normalized pair from d64
 * public   if64_add(...)                  add two pairs with normalization
 * public   if64_sub(...)                  subtract two pairs with normalization
 * public   if64_neg(...)                  negate both halves
 * public   if64_compare(...)              exact total ordering (-1, 0, 1)
 * public   if64_toF32Relative(...)        f32 delta relative to an origin
 * public   if64_toString(...)             value projection, cold path only
 * public   if64_toStringStruct(...)       structure projection, cold path only
 *
 * --GETTERS------------------------------
 * public   if64_integer(...)              read integer half by value
 * public   if64_fraction(...)             read fraction half by value
 *
 * --SETTERS------------------------------
 * public   if64_setInteger(...)           write integer half
 * public   if64_setFraction(...)          write fraction half
 * ============================================================================
 */

;;GETTER
i32 if64_integer(if64 self) {
    return self.integer;
}

;;GETTER
f32 if64_fraction(if64 self) {
    return self.fraction;
}

;;SETTER
fn if64_setInteger(if64 *self, i32 value) {
    (*self).integer = value;
}

;;SETTER
fn if64_setFraction(if64 *self, f32 value) {
    (*self).fraction = value;
}

if64 if64_0() {
    return (if64){ .integer = 0, .fraction = 0.0f };
}

if64 if64_1(string text) {
    PairParseResult parsed;
    PairParseStatus status = pair_parseText(text, INT32_MAX, &parsed);
    if (status != PAIR_PARSE_OK) {
        THROW("if64: %s literal rejected",
              status == PAIR_PARSE_OVERFLOW ? "overflowing" : "malformed");
        return if64_0();
    }
    if (parsed.nonFinite)
        return (if64){ .integer = 0, .fraction = (f32) NAN };

    f32 fraction = (f32) parsed.fraction;
    if (fraction >= 1.0f)
        fraction = nextafterf(1.0f, 0.0f);
    else if (fraction <= -1.0f)
        fraction = -nextafterf(1.0f, 0.0f);
    return (if64){ .integer = (i32) parsed.integer, .fraction = fraction };
}

if64 if64_2(i32 integer, f32 fraction) {
    if (isnan(fraction))
        return (if64){ .integer = 0, .fraction = (f32) NAN };
    if (fraction < -1.0f || fraction >= 1.0f) {
        THROW("if64: fraction outside [-1, 1)");
        return if64_0();
    }
    return (if64){ .integer = integer, .fraction = fraction };
}

f32 if64_toF32(if64 value) {
    return (f32) if64_integer(value) + if64_fraction(value);
}

if64 if64_normalize(i64 integer, d64 fraction) {
    if (!isfinite(fraction)) {
        return (if64){ .integer = 0, .fraction = (f32) NAN };
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
    f32 remF = (f32) rem;
    if (remF >= 1.0f) {
        total += 1;
        remF = 0.0f;
    } else if (remF <= -1.0f) {
        total -= 1;
        remF = 0.0f;
    }
    if (total > (i64) INT32_MAX || total < (i64) INT32_MIN) {
        THROW("if64: integer overflow on normalize");
        return if64_0();
    }
    return (if64){ .integer = (i32) total, .fraction = remF };
}

if64 if64_fromF32(f32 value) {
    return if64_normalize(0, (d64) value);
}

if64 if64_fromD64(d64 value) {
    return if64_normalize(0, value);
}

if64 if64_add(if64 a, if64 b) {
    i64 intSum = (i64) if64_integer(a) + (i64) if64_integer(b);
    d64 fracSum = (d64) if64_fraction(a) + (d64) if64_fraction(b);
    return if64_normalize(intSum, fracSum);
}

if64 if64_sub(if64 a, if64 b) {
    i64 intDiff = (i64) if64_integer(a) - (i64) if64_integer(b);
    d64 fracDiff = (d64) if64_fraction(a) - (d64) if64_fraction(b);
    return if64_normalize(intDiff, fracDiff);
}

if64 if64_neg(if64 self) {
    return (if64){ .integer = -if64_integer(self), .fraction = -if64_fraction(self) };
}

i32 if64_compare(if64 a, if64 b) {
    i32 ia = if64_integer(a), ib = if64_integer(b);
    if (ia != ib)
        return (ia < ib) ? -1 : 1;
    f32 fa = if64_fraction(a), fb = if64_fraction(b);
    if (fa < fb)
        return -1;
    if (fa > fb)
        return 1;
    return 0;
}

f32 if64_toF32Relative(if64 value, if64 origin) {
    i64 intDiff = (i64) if64_integer(value) - (i64) if64_integer(origin);
    f32 fracDiff = if64_fraction(value) - if64_fraction(origin);
    return (f32) intDiff + fracDiff;
}

fn if64_toString(const if64 *value, utf8 *dest, bytes cap, bool *outTruncated) {
    if (outTruncated)
        *outTruncated = false;
    if (!dest || cap == 0u)
        return;
    if (!value) {
        snprintf(dest, cap, "nullptr");
        return;
    }
    i32 written = snprintf(dest, cap, "if64(integer=%" PRId32 ", fraction=%.7g)",
                           if64_integer(*value), (d64) if64_fraction(*value));
    if (written < 0 || (bytes) written >= cap) {
        if (outTruncated)
            *outTruncated = true;
    }
}

fn if64_toStringStruct(const if64 *value, utf8 *dest, bytes cap, bool *outTruncated) {
    if (outTruncated)
        *outTruncated = false;
    if (!dest || cap == 0u)
        return;
    if (!value) {
        snprintf(dest, cap, "nullptr");
        return;
    }
    u32 bits = 0;
    f32 fraction = if64_fraction(*value);
    memcpy(&bits, &fraction, sizeof bits);
    i32 written = snprintf(dest, cap, "if64 { integer=%" PRId32 ", fraction=0x%08" PRIx32 " }",
                           if64_integer(*value), bits);
    if (written < 0 || (bytes) written >= cap) {
        if (outTruncated)
            *outTruncated = true;
    }
}
