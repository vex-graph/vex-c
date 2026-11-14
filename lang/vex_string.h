#ifndef VEXC_STRING_H
#define VEXC_STRING_H

// string.h — the floor's stack text type (C-native, by value).
//
// INTENTIONAL(vex): `string` is deliberate floor vocabulary, named like `bytes`
// and lowercase on purpose. It is NOT a class and not CapitalCase: the floor owns
// small inline value types in lowercase (`bytes`, `b16`, `vec3f`, `mat4f`). It is
// mutable inline storage with explicit `len`, not a pool or an interned atom.
// `chr("hello")` copies into this struct on the stack; overflow is refused loudly
// (the Cold-Strict, Hot-Minimal Validation Law), never truncated.

#include "annotation.h"
#include "constructor.h"
#include "primitive.h"

// 23 bytes of inline payload (like a VariableSlot name) plus the explicit len is
// not the same tradeoff as the 40-byte layout: a 32-byte floor-native inline text
// type with 24 bytes of payload keeps every `string` 32 bytes.
#define STRING_DATA_BYTES 24u
#define STRING_DATA_MAX 23u

typedef struct string {
    u8 data[STRING_DATA_BYTES];
    bytes len;
} string;

_Static_assert(sizeof(string) == 32, "string is 24 inline bytes plus an 8-byte len");

// --- Constructors ---
string string_0();
string string_1(const u8 *bytesIn, bytes length);
string string_fromCString(const char *text);

#define string(...) CONSTRUCTOR_DISPATCH(string, __VA_ARGS__)

// The literal door: `string s = chr("hello");` — copies the literal, refuses
// overflow loudly (one [vex] line plus a zeroed string; check len/empty).
#define chr(literal) string_fromCString(literal)

// --- Core ---
bool string_equals(const string *self, const string *other);
bool string_append(string *self, const string *other);
fn string_clear(string *self);

// --- Getters ---
;;GETTER
bytes string_len(const string *self);

;;GETTER
bool string_isEmpty(const string *self);

// Borrow the inline bytes (not necessarily NUL-terminated).
const u8 *string_data(const string *self);

// --- String projections (the toString Law) ---
fn string_toString(const string *self, char *dest, bytes cap, bool *outTruncated);
fn string_toStringStruct(const string *self, char *dest, bytes cap, bool *outTruncated);

#endif
