# util — Repo-Local Living Preferences

## Constitution Link

The complete [workspace constitution](https://gist.github.com/vex-graph/4132a6c45cb6d3797c3e8eff2e94035a) governs this repository. The binding law list is `gist.md` (`../../metadata/gists/preferences/gist.md`); `../../../metadata/gists/preferences/PREFERENCES.md` is its full rationale and is never ingested whole. Read the law first, then this file, the proof lawbook `../../../tests/test-preferences.md` (slice one law with `./tools/prefs test "<title>"`), the evidence ledger `../../../metadata/gists/test-checklist.md`, and the actual implementation.

## Law Index (Binding Matrix)

| Law Title | Scope | Enforcement |
| :--- | :--- | :--- |
| Floor Law | dependency direction and ownership | no include leaves `util`; dependency-free build |
| Include Spelling Law | every consumer include | one spelling, rooted at the workspace include root |
| Width Vocabulary Law | `u8`..`i64`, `f32`/`f64`, `byte` | static asserts in the owning test |
| Constructor Dispatch Law | `Class(...)` arity spelling | the owner suites compile the dispatch |
| Annotation Marker Law | every `;;MARKER` | marker expands to `_Static_assert` |
| Loud Rejection Law | `THROW` | cold-path only, no unwind, no exit |
| Platform Spelling Law | compile-time target identity | six exact targets plus derived axes |
| Test Skip Law | `B_TEST_SKIP` | 77 is the only skip code |

### Floor Law

`util` is the bottom of the dependency graph. Nothing under `util` includes a
repository, a platform header above the C standard library, or another part of
the ecosystem. Every repository may include `util` downward; `util` includes
none of them. `b.assembly.json` declares `includes: ["util"]` and no
dependencies, and the build proves the claim by compiling the headers alone.

The floor exists because the vocabulary used to live in unowned workspace-root
files (`annotation/annotation.h`, `test_support.h`) and in higher repositories
(`vexspoke/exception/throw.h`, `vexspoke/c23/constructor.h`,
`lifetime/lifetime/platform.h`). A header that everything includes and nobody
owns cannot change without a cross-repository edit with no owner to review it.
The floor gives that set one owner.

### Include Spelling Law

The spelling is `#include "util/<header>.h"` from every repository. The
workspace root is on every compile's include path, so no manifest needs a
per-consumer root and no file needs a relative hop. A second spelling
(`vex/util/...`, `../util/...`, a forwarding header in the old location) is a
defect: it hides which floor a file actually consumes.

### Width Vocabulary Law

The width is the name. `u8`, `i8`, `u16`, `i16`, `u32`, `i32`, `u64`, `i64`,
`byte`, `f32`, `f64` are the ecosystem spellings. `int`, `long`, `long long`
and the `stdint.h` names do not appear in new ecosystem source; the aliases are
the only permitted spelling above this header.

`byte` is `u8`. `Byte` remains the boxed class in `vexspoke/primitive` until
that layer is retired; the two must not be confused in new code.

`f16` exists only when `__FLT16_MAX__` is defined, and `u128`/`i128` only when
`__SIZEOF_INT128__` is defined. A target without them gets a compile error at
the use site, never a silent wider substitute.

### Constructor Dispatch Law

`Class(...)` is a macro over `Class_0`..`Class_8`, dispatching by argument
count. It costs no code and allocates nothing. A class that exposes a
constructor names every accepted arity and adds the one-line shim; an arity gap
is intentional and a call into it fails to compile. Type-based overloading is
out of scope.

### Annotation Marker Law

`;;MARKER` carries two semicolons on the left only and expands to a
`_Static_assert(1, ...)`, so a file documents its own architecture and the
compiler checks that the marker is used where it claims. The marker set is
closed here; a new marker is added in one file and all consumers inherit it.

### Loud Rejection Law

`THROW(fmt, ...)` writes exactly one `[vex] file:line: message` line to stderr
and returns. It never unwinds, never exits, and is never called on a hot path,
in a signal handler, or during bounded teardown. The caller returns a safe
default. `grep -rn THROW` is the inventory of loud cold rejections.

### Platform Spelling Law

The six canonical targets are named once each in `util/platform.h`:
`VEX_PLATFORM_APPLE_SILICON`, `VEX_PLATFORM_APPLE_X86_64`,
`VEX_PLATFORM_LINUX_ARM64`, `VEX_PLATFORM_LINUX_X86_64`,
`VEX_PLATFORM_WINDOWS_ARM64`, `VEX_PLATFORM_WINDOWS_X86_64`. The coarse axes
are derived from the exact target, never the reverse. Raw `__APPLE__`, `_WIN32`
and `__linux__` are resolved in this header and tested nowhere else. A target
matching no branch is `#error`.

### Test Skip Law

`B_TEST_SKIP` is 77. An owner suite that cannot legitimately run (no loopback
port, no enrolled keychain item, no compositor surface) returns it so the
runner can distinguish skipped from passed and failed. A different skip code is
a defect.

## Current State

The floor is seven headers plus an umbrella, with one owner test
(`util/test/types`) proving the width vocabulary on macOS arm64. 904 include
sites were retargeted in the same cycle. No translation unit, no allocation, no
runtime state.

## Scope and Limitations

- The repository is local-only; its declared remote has no published history
  yet.
- `platform.h` is a compile-time answer only; runtime probing belongs to the
  capability owner.
- `test_support.h` carries no runner.
- Windows and Linux builds are declared, not yet proven.
