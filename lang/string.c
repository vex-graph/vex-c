// string.c — the floor's stack text type.
//
// Shallow copy in on construction and append; zeroed on overflow so the caller
// sees len==0 instead of a silently truncated literal. Cold seams validate; hot
// reads trust len.

#include <stdio.h>
#include <string.h>

#include "string.h"

;;OVERVIEW
;;DEFINITION

static void zeroString(string *self) {
    memset(self, 0, sizeof(*self));
}

string string_0() {
    return (string) { 0 };
}

string string_1(const u8 *bytesIn, bytes length) {
    string out = { 0 };

    if (bytesIn == nullptr || length > STRING_DATA_MAX)
        return out;

    memcpy(out.data, bytesIn, (size_t) length);
    out.len = length;
    return out;
}

string string_fromCString(const char *text) {
    string out = { 0 };

    if (text == nullptr)
        return out;

    bytes length = strlen(text);

    if (length > STRING_DATA_MAX)
        return out;

    memcpy(out.data, text, (size_t) length);
    out.len = length;
    return out;
}

bool string_equals(const string *self, const string *other) {
    if (self == nullptr || other == nullptr)
        return false;

    if ((*self).len != (*other).len)
        return false;

    if ((*self).len == 0u)
        return true;

    return memcmp((*self).data, (*other).data, (size_t) (*self).len) == 0;
}

bool string_append(string *self, const string *other) {
    if (self == nullptr || other == nullptr)
        return false;

    bytes combined = (*self).len + (*other).len;

    if (combined > STRING_DATA_MAX)
        return false;

    memcpy((*self).data + (*self).len, (*other).data, (size_t) (*other).len);
    (*self).len = combined;
    return true;
}

void string_clear(string *self) {
    if (self == nullptr)
        return;

    zeroString(self);
}

;;GETTER
bytes string_len(const string *self) {
    if (self == nullptr)
        return 0u;

    return (*self).len;
}

;;GETTER
bool string_isEmpty(const string *self) {
    return string_len(self) == 0u;
}

const u8 *string_data(const string *self) {
    if (self == nullptr)
        return nullptr;

    return (*self).data;
}

void string_toString(const string *self, char *dest, size_t cap, bool *outTruncated) {
    if (outTruncated != nullptr)
        *outTruncated = false;

    if (dest == nullptr || cap == 0u)
        return;

    if (self == nullptr) {
        snprintf(dest, cap, "nullptr");
        return;
    }

    size_t n = (size_t) (*self).len;
    size_t take = n < cap - 1u ? n : cap - 1u;
    memcpy(dest, (*self).data, take);
    dest[take] = '\0';

    if (take < n && outTruncated != nullptr)
        *outTruncated = true;
}

void string_toStringStruct(const string *self, char *dest, size_t cap, bool *outTruncated) {
    if (outTruncated != nullptr)
        *outTruncated = false;

    if (dest == nullptr || cap == 0u)
        return;

    if (self == nullptr) {
        snprintf(dest, cap, "nullptr");
        return;
    }

    char body[STRING_DATA_BYTES + 1u];
    size_t n = (size_t) (*self).len;

    memcpy(body, (*self).data, n);
    body[n] = '\0';

    int written = snprintf(dest, cap, "string { data=\"%s\", len=%zu }", body, n);

    if (written < 0 || (size_t) written >= cap) {
        if (outTruncated != nullptr)
            *outTruncated = true;
    }
}
