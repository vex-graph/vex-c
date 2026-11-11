#include "vex.h"
#include "primitive.h"

#include "debug/throw.h"
#include "pair_parse.h"

;;DEFINITION
;;OVERVIEW

;;GETTER
i64 ld128_integer(ld128 self) {
    return self.integer;
}

;;GETTER
d64 ld128_fraction(ld128 self) {
    return self.fraction;
}

;;SETTER
void ld128_setInteger(ld128 *self, i64 value) {
    (*self).integer = value;
}

;;SETTER
void ld128_setFraction(ld128 *self, d64 value) {
    (*self).fraction = value;
}

ld128 ld128_0() {
    return (ld128){ .integer = 0, .fraction = 0.0 };
}

ld128 ld128_1(const char *text) {
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

void ld128_toString(const ld128 *value, char *dest, bytes cap, bool *outTruncated) {
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

void ld128_toStringStruct(const ld128 *value, char *dest, bytes cap, bool *outTruncated) {
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
