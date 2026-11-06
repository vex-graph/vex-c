#ifndef UTIL_ANNOTATION_H
#define UTIL_ANNOTATION_H

// util/annotation.h — every source annotation marker in one file.
//
// Single include for the Two-Semicolon Annotation Style Law: each marker
// carries two semicolons on the left and expands to a _Static_assert, so a
// file documents its own architecture and the compiler checks the marker.
// Usage: `#include "util/annotation.h"` then `;;OVERVIEW`, `;;DEFINITION`, etc.
//
// `;;VALIDATOR` marks a surface that exists to interrogate state — the C
// companion to a graphics validation layer. It is not `;;VALIDATOR`: that name is
// unusable because platform headers test `#if DEBUG`.
//
// Rust mirror: relational-engine/rust/annotations/lib.rs. Same fourteen markers,
// same arity, `#[overview]` for `;;OVERVIEW` and so on. `;;TEST` equates to
// Rust's built-in `#[test]`, which cannot be shadowed by a proc macro.

#define CHECKER _Static_assert(1, "@Checker");
#define DECONSTRUCT _Static_assert(1, "@Deconstruct");
#define DEFINITION _Static_assert(1, "@Definition");
#define DRAFT _Static_assert(1, "@Draft");
#define FINAL _Static_assert(1, "@Final");
#define GETTER _Static_assert(1, "@Getter");
#define HOTCODE _Static_assert(1, "@HotCode");
#define INCOMPLETE _Static_assert(1, "@Incomplete");
#define INHERITS(text) _Static_assert(1, "@Inherits(" text ")");
#define INTENTION(text) _Static_assert(1, "@Intention(" text ")");
#define OVERVIEW _Static_assert(1, "@Overview");
#define PLATFORM_EXCLUSIVE(platform) _Static_assert(1, "@PlatformExclusive(" platform ")");
#define SETTER _Static_assert(1, "@Setter");
#define TEST _Static_assert(1, "@Test");
#define VALIDATOR _Static_assert(1, "@Validator");
#define WHAT(text) _Static_assert(1, "@What(" text ")");

#endif
