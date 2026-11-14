// string_test.c — the inline stack text type.
//
// Construction, accessors, equality, append, clearing and both projections are
// proven at run time. Overflow is a cold rejection: the constructor returns a
// zeroed string and the append returns false, each with one [vex] line on
// stderr. The test asserts the safe results; the diagnostic lines are emitted
// by the code under test and are not captured here.



#include "vex.h"

static int failures;

static fn check(int ok, const char *what) {
    if (ok) return;
    fprintf(stderr, "string_test: FAIL %s\n", what);
    failures++;
}

int main() {
    // Empty construction, including the zero-argument dispatch.
    string empty = string();
    check(string_len(&empty) == 0, "string() is empty");
    check(string_isEmpty(&empty), "string_isEmpty on empty");
    check(string_len(nullptr) == 0, "string_len(nullptr) is zero");
    check(string_isEmpty(nullptr), "string_isEmpty(nullptr) is true");
    check(string_data(nullptr) == nullptr, "string_data(nullptr) is nullptr");

    // Exact-capacity construction (23 bytes) and the literal door.
    const char *fits = "12345678901234567890123";
    string packed = chr(fits);
    check(string_len(&packed) == STRING_DATA_MAX, "chr copies the exact capacity");
    check(!string_isEmpty(&packed), "a packed string is not empty");
    check(string_data(&packed) != nullptr
          && memcmp(string_data(&packed), fits, STRING_DATA_MAX) == 0,
          "chr copies the literal bytes");

    string copied = string_1((const u8*) "hello", 5);
    string literal = chr("hello");
    check(string_equals(&copied, &literal), "string_1 and chr agree");
    check(string_len(&copied) == 5, "string_1 records the length");
    check(!string_equals(&copied, &packed), "different lengths are unequal");
    check(!string_equals(nullptr, &copied), "null equality is false");
    check(!string_equals(&copied, nullptr), "null equality is false both ways");
    check(string_equals(nullptr, nullptr) == false, "two nulls are not equal");

    string one = chr("a");
    string two = chr("b");
    check(!string_equals(&one, &two), "same length, different bytes are unequal");

    // Null bytes input is a zeroed string, not a crash.
    string nullBytes = string_1(nullptr, 5);
    string nullText = string_fromCString(nullptr);
    check(string_len(&nullBytes) == 0, "string_1(nullptr) is empty");
    check(string_len(&nullText) == 0, "string_fromCString(nullptr) is empty");

    // Append succeeds while the combined length fits.
    string greeting = chr("hello");
    string world = chr(" world");
    check(string_append(&greeting, &world), "append within capacity succeeds");
    check(string_len(&greeting) == 11, "append records the combined length");
    check(memcmp(string_data(&greeting), "hello world", 11) == 0,
          "append preserves both halves");
    check(string_append(&greeting, &empty), "append of the empty string succeeds");
    check(string_len(&greeting) == 11, "append of empty leaves the length alone");

    // Overflow is refused loudly through the safe result: the original keeps
    // its bytes and the constructor gives back an empty string.
    check(!string_append(&greeting, &packed), "append over capacity returns false");
    check(string_len(&greeting) == 11, "failed append leaves the original intact");
    string overflowed = chr("123456789012345678901234");
    check(string_len(&overflowed) == 0, "chr over capacity returns an empty string");
    string overflowedBytes = string_1((const u8*) "123456789012345678901234", 24);
    check(string_len(&overflowedBytes) == 0, "string_1 over capacity returns an empty string");

    // Clear releases the content but not the storage.
    string cleared = chr("temp");
    string_clear(&cleared);
    check(string_len(&cleared) == 0 && string_isEmpty(&cleared), "clear empties the string");
    string_clear(nullptr);
    check(string_len(&cleared) == 0, "clear(nullptr) is safe");

    // Value projection: bytes, NUL termination and an honest truncation flag.
    char text[16];
    bool truncated = true;
    string_toString(&literal, text, sizeof text, &truncated);
    check(strcmp(text, "hello") == 0, "toString writes the value");
    check(!truncated, "toString reports no truncation when it fits");

    char tight[5];
    truncated = false;
    string_toString(&literal, tight, sizeof tight, &truncated);
    check(strcmp(tight, "hell") == 0, "toString truncates at cap - 1");
    check(truncated, "toString reports truncation");

    truncated = false;
    string_toString(nullptr, tight, sizeof tight, &truncated);
    check(strcmp(tight, "null") == 0, "toString of nullptr truncates loudly");
    check(truncated, "toString of nullptr reports truncation");

    truncated = true;
    string_toString(&literal, nullptr, 0, &truncated);
    check(!truncated, "toString with no destination resets the flag");
    string_toString(&literal, text, 0, &truncated);
    check(!truncated, "toString with zero capacity resets the flag");

    // Structure projection: the shape and length are visible.
    char structure[64];
    truncated = false;
    string_toStringStruct(&literal, structure, sizeof structure, &truncated);
    check(strstr(structure, "data=\"hello\"") != nullptr,
          "toStringStruct includes the value");
    check(strstr(structure, "len=5") != nullptr,
          "toStringStruct includes the length");
    check(!truncated, "toStringStruct reports no truncation when it fits");

    char narrow[8];
    truncated = false;
    string_toStringStruct(&literal, narrow, sizeof narrow, &truncated);
    check(truncated, "toStringStruct reports truncation");

    truncated = false;
    char tiny[4];
    string_toStringStruct(nullptr, tiny, sizeof tiny, &truncated);
    check(truncated, "toStringStruct of nullptr reports truncation");

    if (failures != 0) {
        fprintf(stderr, "string_test: %d checks failed\n", failures);
        return 1;
    }

    printf("string_test: stack text construction, append and projections held\n");
    return 0;
}
