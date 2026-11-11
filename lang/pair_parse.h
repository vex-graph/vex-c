#ifndef VEXC_PAIR_PARSE_H
#define VEXC_PAIR_PARSE_H

// lang/pair_parse.h — strict decimal parsing for the composite numeric values.
//
// Internal seam: if64.c and ld128.c share this scanner; lang.h does not expose
// it. The whole token must be consumed. Named constants, inf and nan are
// accepted; no locale, no silent truncation.

#include "primitive.h"

typedef enum PairParseStatus {
    PAIR_PARSE_OK = 0,
    PAIR_PARSE_MALFORMED = 1,
    PAIR_PARSE_OVERFLOW = 2
} PairParseStatus;

typedef struct PairParseResult {
    i64 integer;
    d64 fraction;
    u8 nonFinite;
} PairParseResult;

PairParseStatus pair_parseText(const char *text, i64 maxPositive, PairParseResult *out);

#endif
