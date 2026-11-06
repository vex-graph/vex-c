# util — the zero-dependency floor

`util` is the bottom of the stack. It owns the vocabulary every other repository
speaks and nothing else: source annotations, loud cold failure, the six-target
platform spelling, the constructor dispatcher, the width vocabulary, and the
owner-suite skip code.

It has no dependencies. It is not a library in the "compose me" sense; it is the
shared floor that makes a repository able to state what it means without
inventing a second spelling of it.

## Headers

| Header | Owns |
| :--- | :--- |
| `util/annotation.h` | every `;;MARKER` the Two-Semicolon Annotation Style Law names |
| `util/constructor.h` | arity-based `Class(...)` dispatch |
| `util/platform.h` | the six canonical platform targets and their coarse axes |
| `util/test_support.h` | `B_TEST_SKIP` (77) for owner suites |
| `util/throw.h` | `THROW(fmt, ...)`, one `[vex]` line on stderr, no unwind |
| `util/types.h` | `u8`..`i64`, `byte`, `f32`/`f64`, gated `f16`/`u128` |
| `util/vex.h` | the mandatory set in one include, for tests and entry points |

A production file includes the one header it uses. A test that needs the whole
vocabulary includes `util/vex.h`.

## Include spelling

The workspace root is on every build's include path, so the spelling is always

```c
#include "util/throw.h"
```

never `vex/util/throw.h` and never a relative hop.

## Current State

- Seven headers, no translation units, no allocation, no boot.
- `util/test/types` proves the width vocabulary on macOS arm64.
- 904 include sites across the tree were retargeted from the old unowned
  workspace-root headers to `util/...` in the same cycle that created this
  repository.

## Scope and Limitations

- This repository does not own memory. `transient`, `Memory_alloc` and
  `MemoryArena_alloc` belong to `re`.
- `platform.h` answers a compile-time question. Runtime capability probing is a
  separate owner.
- `constructor.h` dispatches by argument count only. Type-based overloading is
  deliberately out of scope.
- `test_support.h` is a vocabulary header, not a test runner.
