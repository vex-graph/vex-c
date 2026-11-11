#include "vex.h"
#include "primitive.h"

#include "debug/throw.h"
#include "pair_parse.h"

;;DEFINITION
;;OVERVIEW

;;GETTER
i32 if64_integer(if64 self) {
    return self.integer;
}

;;GETTER
f32 if64_fraction(if64 self) {
    return self.fraction;
}

;;SETTER
void if64_setInteger(if64 *self, i32 value) {
    (*self).integer = value;
}

;;SETTER
void if64_setFraction(if64 *self, f32 value) {
    (*self).fraction = value;
}

if64 if64_0() {
    return (if64){ .integer = 0, .fraction = 0.0f };
}

if64 if64_1(const char *text) {
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

void if64_toString(const if64 *value, char *dest, bytes cap, bool *outTruncated) {
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

void if64_toStringStruct(const if64 *value, char *dest, bytes cap, bool *outTruncated) {
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
