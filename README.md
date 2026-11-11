# lib — the vex-c floor

`lib` is **vex-c**: the bottom of the stack, checked out at `lib/` beside `vex/`
in the repository tree. It owns the vocabulary every other repository speaks and
nothing else: source annotations, loud cold failure, the six-target platform
spelling, the constructor dispatcher, the primitive width vocabulary, the vector
and matrix value types, and the owner-suite skip code.

It has no dependencies. It is not a library in the "compose me" sense; it is the
shared floor that makes a repository able to state what it means without
inventing a second spelling of it.

## The one include

`lib/` is an include root, so the floor is entered bare:

```c
#include "vex.h"
```

No `lib/vex.h`, no `vex/lib/vex.h`, no relative hop. `vex.h` carries the whole
floor by including its headers under their own bare names.

Every file includes the door and nothing system-level itself; the door batches
the whole standard system surface the floor uses.

## Headers

| Header | Owns |
| :--- | :--- |
| `annotation.h` | every `;;MARKER` the Two-Semicolon Annotation Style Law names |
| `lang/constructor.h` | arity-based `Class(...)` dispatch |
| `lang/platform.h` | the six canonical platform targets and their coarse axes |
| `lang/primitive.h` | `u8`..`i64`, `bytes` (`bytes`), `b16`, `f32`/`d64`, gated `u128` |
| `lang/vex_string.h` | the 24-byte inline text type (`string`) and `chr("...")` |
| `vector/vector.h` | `vec2`/`vec3`/`vec4` at `b16`/`f32`/`d64`, accessors, constructors, arithmetic |
| `matrix/matrix.h` | `mat2`/`mat3`/`mat4` at `f32`/`d64`, accessors, constructors, arithmetic |
| `debug/test_support.h` | `B_TEST_SKIP` (77) for owner suites |
| `debug/throw.h` | `THROW(fmt, ...)`, one `[vex]` line on stderr, no unwind |
| `vex.h` | the whole floor in one include |

The math surface is exactly three headers: **lang/primitive.h** (scalars),
**vector/vector.h** (vectors), **matrix/matrix.h** (matrices). A matrix composes
the vector columns; it does not re-own their geometry.

```text
lib/
  vex.h                     # the door
  lang/
    annotation.h            # every ;;MARKER
    constructor.h           # arity-based Class(...) dispatch
    platform.h              # six-target platform spelling
    primitive.h             # scalar width vocabulary
    primitive.c             # the bfloat16 bit-pattern bridge
    string.h                # the 24-byte inline text type (`string`)
    string.c                # the stack-text bridge
  vector/
    vector.h                # vec2/vec3/vec4, explicit per function
    vec2.c                  # vec2b, vec2f, vec2d
    vec3.c                  # vec3b, vec3f, vec3d
    vec4.c                  # vec4b, vec4f, vec4d
  matrix/
    matrix.h                # mat2/mat3/mat4, explicit per function
    mat2.c                  # mat2f, mat2d
    mat3.c                  # mat3f, mat3d
    mat4.c                  # mat4f, mat4d
  debug/
    test_support.h          # B_TEST_SKIP (77)
    throw.h                 # THROW(fmt, ...)
```

Headers are include surfaces: types, layout assertions and declarations only —
function bodies live in the `.c` beside them, one shape per translation unit.

Every accessor, constructor arity and arithmetic function is written out by
hand. There is no family macro and no token pasting (`##`): the header shows the
exact signature and body that ships. The only macro left in the math surface is
the one-line `Class(...)` constructor shim from `lang/constructor.h`.

## Vectors

A vector is its lanes and nothing else:

| Shape | Layout | `sizeof` | `_Alignof` |
| :--- | :--- | :--- | :--- |
| `vec2` | 2 lanes | `2 * width` | `sizeof` |
| `vec3` | 3 lanes + a zeroed `_pad` | `4 * width` | `sizeof` |
| `vec4` | 4 lanes | `4 * width` | `sizeof` |

`vec3f` and `vec4f` are both 16 bytes at 16-byte alignment: a `vec3` is a
`vec4` whose fourth lane is pinned to zero. That lane is padding, never
metadata; a coordinate frame belongs to a spatial type in a higher tier.

```c
vec3f v = vec3f(1.0f, 2.0f, 3.0f);
f32   x = vec3f_x(v);            // by value: no &, no pointer layer
vec3f_setX(&v, 9.0f);            // (*self).x lives only in the setter
```

`b16` is bfloat16 carried as a 2-byte bit pattern with `b16_from_f32` /
`b16_to_f32`, because no compiler on the six-target floor ships it as an
arithmetic scalar. `vec3b` is therefore storage: widen to `vec3f` to compute.

## Matrices

A matrix is its columns and nothing else, stored column-major:

| Shape | Columns | `sizeof` (f32 / d64) | `_Alignof` (f32 / d64) |
| :--- | :--- | :--- | :--- |
| `mat2` | 2 × `vec2` | 16 / 32 | 8 / 16 |
| `mat3` | 3 × `vec3` | 48 / 96 | 16 / 32 |
| `mat4` | 4 × `vec4` | 64 / 128 | 16 / 32 |

A `mat3` column is a `vec3`, so it carries the same zeroed fourth lane and the
same 128-bit load. `data[]` is the flat seam; entry `(row, col)` is at
`data[col * 4 + row]` (the stride is four lanes because every column is a
`vec4`-shaped vector).

```c
mat3f m = mat3f_1(1.0f);          // diagonal
f32   a = mat3f_at(&m, 0, 1);     // entry getter: const pointer
mat3f_setAt(&m, 2, 2, 7.0f);
mat3f p = mat3f_multiply(m, mat3f_identity());
```

## Identity

The directory is `lib`; the repository and published identity are `vex-c`. The
two names are deliberate: `lib` is where the workspace finds it, `vex-c` is what
it is. `vexspoke` is R2 CPU behavior and is a different owner.

## Current State

- Nine public headers plus the `vex.h` door, and eight translation units
  (`lang/primitive.c`, `lang/string.c`, `vector/vec2.c`, `vector/vec3.c`,
  `vector/vec4.c`, `matrix/mat2.c`, `matrix/mat3.c`, `matrix/mat4.c`).
  Headers carry types, layout assertions and declarations; every function body
  lives in the `.c` for its shape.
- `test/primitive` proves the scalar widths and the bfloat16 conversion;
  `test/vector` proves the vector layouts, accessors and arithmetic;
  `test/matrix` proves the matrix composition and arithmetic; `test/string`
  proves construction, equality, append, clear, both projections and the loud
  overflow refusal. All four pass with
  `cc -std=gnu23 -Wall -Wextra -Werror` on macOS arm64.
- `.clangd` states that this tree is C23: without it, clangd falls back to
  Objective-C++, where `alignas` on an anonymous union member is ignored and
  the layout assertions report false failures.
- `lang/vex_string.h` is the inline text vocabulary (`string`, `chr("...")`);
  it ships both `toString` and `toStringStruct`. The vector and matrix
  projections remain deferred.

## Scope and Limitations

- This repository does not own memory. `transient`, `Memory_alloc` and
  `MemoryArena_alloc` belong to `re`.
- `lang/platform.h` answers a compile-time question. Runtime capability probing is a
  separate owner.
- `lang/constructor.h` dispatches by argument count only. Type-based overloading is
  deliberately out of scope.
- `debug/test_support.h` is a vocabulary header, not a test runner.
- `b16` vectors have no arithmetic surface. bfloat16 is storage; the bridge is
  `vecNb_to_vecNf` and `vecNb_from_vecNf`. Matrices are `f32`/`d64` only.
- `mat2`/`mat3`/`mat4` ship `add`, `sub`, `scale`, `multiply`, `multiplyVec`,
  `transpose` and `determinant`; inverse and the affine helpers are not here
  yet.
- `relational-engine/primitive`'s `vec3f32`/`vec3i32` family is the older
  duplicate and is retired once its consumers move to `vec3f`/`vec3d`.
