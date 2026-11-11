#ifndef VEXC_ANNOTATION_H
#define VEXC_ANNOTATION_H

// annotation.h — every source annotation marker in one file.
//
// Single include for the Two-Semicolon Annotation Style Law: each marker
// carries two semicolons on the left and expands to a _Static_assert, so a
// file documents its own architecture and the compiler checks the marker.
// Usage: `#include "vex.h"` then `;;OVERVIEW`, `;;DEFINITION`, etc.
//
// `;;VALIDATOR` marks a surface that exists to interrogate state — the C
// companion to a graphics validation layer. It is not `;;DEBUG`: that name is
// unusable because platform headers test `#if DEBUG`.
//
// Rust mirror: relational-engine/rust/annotations/lib.rs carries the fifteen
// shared markers with the same arity (`#[overview]` for `;;OVERVIEW`, and so
// on). C additionally owns CLASSNAME, DECONSTRUCT, FINAL and WIDGET_LAYERS,
// which have no Rust spelling. `;;TEST` equates to Rust's built-in `#[test]`,
// which cannot be shadowed by a proc macro.

#define CHECKER                         _Static_assert(1, "@Checker");
#define CLASSNAME(class_)               _Static_assert(1, "@ClassName(" class_ ")");
#define DECONSTRUCT                     _Static_assert(1, "@Deconstruct");
#define DEFINITION                      _Static_assert(1, "@Definition");
#define DRAFT                           _Static_assert(1, "@Draft");
#define FINAL                           _Static_assert(1, "@Final");
#define GETTER                          _Static_assert(1, "@Getter");
#define HOTCODE                         _Static_assert(1, "@HotCode");
#define INCOMPLETE                      _Static_assert(1, "@Incomplete");
#define INHERITS(class_)                _Static_assert(1, "@Inherits(" class_ ")");
#define INTENTION(comment)              _Static_assert(1, "@Intention(" comment ")");
#define OVERVIEW                        _Static_assert(1, "@Overview");
#define PLATFORM_EXCLUSIVE(platform)    _Static_assert(1, "@PlatformExclusive(" platform ")");
#define SETTER                          _Static_assert(1, "@Setter");
#define TEST                            _Static_assert(1, "@Test");
#define VALIDATOR                       _Static_assert(1, "@Validator");
#define WHAT(variable)                  _Static_assert(1, "@What(" variable ")");
#define WIDGET_LAYERS                   _Static_assert(1, "@WidgetLayers"); // darling exclusive btw... this is part of the overview where it tells us the rendering layers for each widget

#endif
