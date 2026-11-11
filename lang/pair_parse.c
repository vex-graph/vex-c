#include "pair_parse.h"

#include <math.h>
#include <string.h>

;;DEFINITION
;;OVERVIEW

static u8 pair_isSpace(const char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f';
}

static char pair_fold(const char c) {
    return c >= 'A' && c <= 'Z' ? (char) (c + ('a' - 'A')) : c;
}

static u8 pair_tokenEquals(const char *begin, const char *end, const char *word) {
    const bytes length =  end - begin;
    if (strlen(word) != length)
        return 0;
    for (bytes index = 0; index < length; index++)
        if (pair_fold(begin[index]) != word[index])
            return 0;
    return 1;
}

static PairParseStatus pair_parseDecimal(const char *begin, const char *end,
                                         u8 negative, i64 maxPositive,
                                         PairParseResult *out) {
    char digits[64];
    bytes count = 0;
    i64 integerDigits = 0;
    u8 sawDigit = 0;
    const char *cursor = begin;

    while (cursor < end && *cursor >= '0' && *cursor <= '9') {
        sawDigit = 1;
        if (!(count == 0 && *cursor == '0')) {
            if (count >= sizeof(digits))
                return PAIR_PARSE_OVERFLOW;
            digits[count++] = *cursor;
            integerDigits++;
        }
        cursor++;
    }

    if (cursor < end && *cursor == '.') {
        cursor++;
        while (cursor < end && *cursor >= '0' && *cursor <= '9') {
            sawDigit = 1;
            if (count < sizeof(digits))
                digits[count++] = *cursor;
            cursor++;
        }
    }

    if (!sawDigit)
        return PAIR_PARSE_MALFORMED;

    i64 exponent = 0;
    if (cursor < end && (*cursor == 'e' || *cursor == 'E')) {
        cursor++;
        u8 exponentNegative = 0;
        if (cursor < end && (*cursor == '+' || *cursor == '-')) {
            exponentNegative = (u8) (*cursor == '-');
            cursor++;
        }
        if (cursor >= end || *cursor < '0' || *cursor > '9')
            return PAIR_PARSE_MALFORMED;
        i64 magnitude = 0;
        while (cursor < end && *cursor >= '0' && *cursor <= '9') {
            if (magnitude < 1000000)
                magnitude = magnitude * 10 + (*cursor - '0');
            cursor++;
        }
        exponent = exponentNegative ? -magnitude : magnitude;
    }

    if (cursor != end)
        return PAIR_PARSE_MALFORMED;

    (*out).integer = 0;
    (*out).fraction = 0.0;
    (*out).nonFinite = 0;

    if (count == 0)
        return PAIR_PARSE_OK;

    const i64 point = integerDigits + exponent;
    const u64 limit = (u64) maxPositive + (negative ? 1u : 0u);
    u64 magnitude = 0;
    d64 fraction = 0.0;

    if (point >= (i64) count) {
        for (bytes index = 0; index < count; index++) {
            if (magnitude > limit / 10u)
                return PAIR_PARSE_OVERFLOW;
            magnitude = magnitude * 10u + (u64) (digits[index] - '0');
            if (magnitude > limit)
                return PAIR_PARSE_OVERFLOW;
        }
        for (i64 index = (i64) count; index < point; index++) {
            if (magnitude > limit / 10u)
                return PAIR_PARSE_OVERFLOW;
            magnitude *= 10u;
        }
    }
    else if (point <= 0) {
        d64 scale = pow(10.0, (d64) (point - 1));
        for (bytes index = 0; index < count; index++) {
            fraction += (d64) (digits[index] - '0') * scale;
            scale *= 0.1;
        }
    }
    else {
        for (i64 index = 0; index < point; index++) {
            if (magnitude > limit / 10u)
                return PAIR_PARSE_OVERFLOW;
            magnitude = magnitude * 10u + (u64) (digits[index] - '0');
            if (magnitude > limit)
                return PAIR_PARSE_OVERFLOW;
        }
        d64 scale = 0.1;
        for (bytes index = point; index < count; index++) {
            fraction += (d64) (digits[index] - '0') * scale;
            scale *= 0.1;
        }
    }

    if (negative)
        fraction = -fraction;

    if (!negative) {
        (*out).integer = (i64) magnitude;
    }
    else if (magnitude == (u64) maxPositive + 1u) {
        if (maxPositive == INT64_MAX)
            (*out).integer = INT64_MIN;
        else
            (*out).integer = -maxPositive - 1;
    }
    else {
        (*out).integer = -(i64) magnitude;
    }

    (*out).fraction = fraction;
    return PAIR_PARSE_OK;
}

static PairParseStatus pair_parseConstant(const char *literal, u8 negative,
                                          i64 maxPositive, PairParseResult *out) {
    return pair_parseDecimal(literal, literal + strlen(literal), negative, maxPositive, out);
}

PairParseStatus pair_parseText(const char *text, i64 maxPositive,
                               PairParseResult *out) {
    (*out).integer = 0;
    (*out).fraction = 0.0;
    (*out).nonFinite = 0;
    if (text == nullptr)
        return PAIR_PARSE_MALFORMED;

    const char *cursor = text;
    while (pair_isSpace(*cursor))
        cursor++;

    u8 negative = 0;
    if (*cursor == '+' || *cursor == '-') {
        negative = (u8) (*cursor == '-');
        cursor++;
    }

    const char *begin = cursor;
    const char *end = cursor;
    while (*end != '\0' && !pair_isSpace(*end))
        end++;
    const char *tail = end;
    while (pair_isSpace(*tail))
        tail++;
    if (*tail != '\0' || begin == end)
        return PAIR_PARSE_MALFORMED;

    if (pair_tokenEquals(begin, end, "pi"))
        return pair_parseConstant("3.14159265358979323846", negative, maxPositive, out);
    if (pair_tokenEquals(begin, end, "tau"))
        return pair_parseConstant("6.28318530717958647692", negative, maxPositive, out);
    if (pair_tokenEquals(begin, end, "e"))
        return pair_parseConstant("2.71828182845904523536", negative, maxPositive, out);
    if (pair_tokenEquals(begin, end, "phi"))
        return pair_parseConstant("1.61803398874989484820", negative, maxPositive, out);

    if (pair_tokenEquals(begin, end, "inf") || pair_tokenEquals(begin, end, "infinity") ||
        pair_tokenEquals(begin, end, "nan")) {
        (*out).integer = 0;
        (*out).fraction = (d64) NAN;
        (*out).nonFinite = 1;
        return PAIR_PARSE_OK;
    }

    return pair_parseDecimal(begin, end, negative, maxPositive, out);
}
