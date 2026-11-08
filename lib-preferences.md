# lib (vex-c) — Repo-Local Living Preferences

## Constitution Link

The complete [workspace constitution](https://gist.github.com/vex-graph/4132a6c45cb6d3797c3e8eff2e94035a) governs this repository. The binding law list is `gist.md` (`../metadata/gists/preferences/gist.md`); `../metadata/gists/preferences/PREFERENCES.md` is its full rationale and is never ingested whole. Read the law first, then this file, the proof lawbook `../worktree/vex/tests/test-preferences.md` (slice one law with `./worktree/tools/prefs test "<title>"`), the evidence ledger `../metadata/gists/test-checklist.md`, and the actual implementation.

## Law Index (Binding Matrix)

| Law Title | Scope | Enforcement |
| :--- | :--- | :--- |
| Floor Law | dependency direction and ownership | no include leaves `lib`; dependency-free build |
| One Door Law | every consumer include | `#include "vex.h"`, no `lib/` prefix |
| Primitive Vocabulary Law | `lang/primitive.h`, `lang/string.h`, `bytes`, `f32`/`d64` | static asserts in the owning test |
| Byte Size Law | `bytes` is the type `sizeof` returns, never a field name | field names say what they count |
| Math Header Split Law (amends the One Header Per Directory Law) | math headers | exactly `lang/primitive.h`, `vector/vector.h`, `matrix/matrix.h`; one `.c` per shape in `vector/` / `matrix/` |
| Accessor Form Law | getters and setters | `;;GETTER` / `;;SETTER` inventory |
| Vector Layout Law | `vec2`/`vec3`/`vec4` geometry | `_Static_assert` on size and alignment |
| Formatting Law | `toString` / `toStringStruct` | deferred — see note below |
| Constructor Dispatch Law | `Class(...)` arity spelling | the owner suites compile the dispatch |
| Annotation Marker Law | every `;;MARKER` | marker expands to `_Static_assert` |
| Loud Rejection Law | `THROW` | cold-path only, no unwind, no exit |
| Platform Spelling Law | compile-time target identity | six exact targets plus derived axes |
| Test Skip Law | `B_TEST_SKIP` | 77 is the only skip code |

### Floor Law

`lib` (published as **vex-c**) is the bottom of the dependency graph. Nothing here includes a repository, a platform header above the C standard library, or another part of the ecosystem. Every repository may include `lib` downward; `lib` includes none of them. It is checked out at `lib/`, beside `vex/` in the repository tree, and the workspace adds that directory to the include path. `b.assembly.json` declares the repository root as its include root and no dependencies, and the build proves the claim by compiling the headers alone.

The floor exists because the vocabulary used to live in unowned workspace-root files (`annotation/annotation.h`, `test_support.h`) and in higher repositories (`vexspoke/exception/throw.h`, `vexspoke/c23/constructor.h`, `lifetime/lifetime/platform.h`). A header that everything includes and nobody owns cannot change without a cross-repository edit with no owner to review it. The floor gives that set one owner.

### One Door Law

The include is bare:

```c
#include "vex.h"
```

`lib/` is an include root, so the spelling never carries `lib/`, `vex/`, or a
relative hop. `vex.h` is the one door: it includes the floor's headers by their
bare names (`"lang/primitive.h"`, `"lang/string.h"`, `"lang/annotation.h"`), and
a consumer needs no second include for the floor. A file that genuinely needs
one piece may spell that header directly (`"lang/primitive.h"`,
`"lang/platform.h"`); two spellings of the same header are a defect because they
hide which floor a file consumes.

### Primitive Vocabulary Law

The width is the name. `u8`, `i8`, `u16`, `i16`, `u32`, `i32`, `u64`, `i64`,
`f32`, `d64` are the ecosystem spellings. `int`, `long`, `long long` and the
`stdint.h` names do not appear in new ecosystem source; the aliases are the only
permitted spelling above this header.

`b16` is bfloat16 — 2 bytes, 8-bit exponent, 7-bit mantissa — carried as a bit
pattern with `b16_from_f32`/`b16_to_f32`. It is not `_Float16`, which is IEEE
binary16 (5-bit exponent, 10-bit mantissa). No compiler on the six-target floor
ships bfloat16 as an arithmetic scalar, so the floor converts explicitly rather
than gating the type out. `u128`/`i128` exist only when `__SIZEOF_INT128__` is
defined; a target without them gets a compile error at the use site, never a
silent wider substitute.

`bytes` is `size_t`, the type `sizeof` returns. It is the size type, not a
fixed-width integer: `u64` is for 64-bit numbers, `bytes` is for object sizes.

`lang/primitive.h` owns the C built-in scalar vocabulary. Composite math value
types are the other two math headers: `vector/vector.h` owns `vec2`, `vec3`,
`vec4` and `matrix/matrix.h` owns `mat2`, `mat3`, `mat4`, in the same widths
(`f32`/`d64`; the vectors add `b16`). `relational-engine/primitive`'s `vec3f32`/`vec3i32`
family is the older duplicate: it is retired or adapted, never kept beside the
new owner.

### Byte Size Law

`bytes` is the type `sizeof` returns (`size_t`), never a field name. It is
`size_t` and not `u64`: both are 8 bytes on the six-target floor, but only
`size_t` is what `sizeof`, `%zu`, `memcpy` and the C interfaces speak.

The type carries the unit, so a byte count is simply `bytes length;`. A field
that holds some other size says what it counts:

| Field | Meaning |
| :--- | :--- |
| `count` | number of elements |
| `capacity` | allocated slots |
| `stride` | bytes between records |
| `offset` | byte position in a buffer |
| `padding` | bytes inserted for alignment |
| `alignment` | required alignment |
| `width` / `height` | a dimension, in native pixels |

`padding` is alignment filler, never a synonym for object size. A bare `size`
field is a defect because it does not say what is being counted.

### Math Header Split Law (amends the One Header Per Directory Law)

The math surface is exactly three headers, each one owning one kind of value:

- `lang/primitive.h` — the scalar vocabulary (`u8`..`i64`, `bytes`, `b16`, `f32`,
  `d64`, gated `u128`/`i128`).
- `vector/vector.h` — `vec2`/`vec3`/`vec4` at `b16`/`f32`/`d64`.
- `matrix/matrix.h` — `mat2`/`mat3`/`mat4` at `f32`/`d64`.
- `lang/string.h` — the inline stack text type `string`; `chr("...")` copies into it.

A matrix composes the vector columns; it does not redeclare their geometry. A
fourth math header is a defect: a new shape belongs in the header of the kind it
is, not in a fourth file.

- **One `.c` per shape.** `vector/vec2.c` owns `vec2b`/`vec2f`/`vec2d`,
  `vector/vec3.c` owns `vec3b`/`vec3f`/`vec3d`, `vector/vec4.c` owns
  `vec4b`/`vec4f`/`vec4d`; `matrix/mat2.c`, `matrix/mat3.c` and `matrix/mat4.c`
  own their shapes the same way, and `lang/primitive.c` owns the bfloat16 bridge.
  Headers are include surfaces — types, layout assertions and declarations —
  and no function body appears in a header. A per-width file (`vec3f.c`
  beside `vec3d.c`) is not required and is discouraged while the bodies are
  shared text.
- **Tag and typedef still share the name**, and helpers stay private in the
  `.c`.
- **Every function is written out.** No family macro and no token pasting
  (`##`) in the math surface. Accessors, constructor arities and arithmetic are
  external functions declared in the header and defined in the shape's `.c`;
  a reader sees exactly what ships. The one
  remaining macro is the `Class(...)` shim from `lang/constructor.h`.
- **Unused exports are dropped at link, not at source.** LTO (`-flto`) removes
  cross-translation-unit dead code; without LTO the build uses
  `-ffunction-sections` plus `-Wl,--gc-sections` (ELF) or `-Wl,-dead_strip`
  (Mach-O). A family file costs nothing for the widths a consumer does not use.

The reason is bloat: one header per class turned the math layer into twenty
files that all had to agree about `f32`. A kind header is the unit of
publication; a `.c` is the unit of implementation.

The widths follow the scalar widths: `b16` is bfloat16, `f32` is binary32,
`d64` is binary64, so the vector classes are `vec3b`, `vec3f`, `vec3d`.

### Accessor Form Law

A state-bearing field is read through a getter and written through a setter.
`(*ptr).field` appears only inside an accessor, never at a call site.

```c
;;GETTER
static inline f32 vec3f_x(vec3f self) { return self.x; }

;;SETTER
static inline void vec3f_setX(vec3f *self, f32 value) { (*self).x = value; }
```

- The getter takes the **value**: no `&`, no pointer layer, no `get` prefix.
- The setter takes the **pointer** and is the one place the field is assigned.
- Both are `static inline` and live in the kind header (`lang/primitive.h`,
  `vector/vector.h` or `matrix/matrix.h`), so the accessor is the public contract and the
  abstraction costs no code at `-O2`.
- `;;GETTER` and `;;SETTER` are the machine-readable inventory; `grep ';;SETTER'`
  lists every mutator.
- A large type (a matrix) takes a `const` pointer on both sides; a 12- or
  16-byte vector takes the value.
- Semantic accessors are not field accessors. `vec3f_left` is `-vec3f_x` or a
  frame mapping and belongs in the math layer, not in the field inventory.

### Vector Layout Law

A vector is its lanes and nothing else:

| Shape | Layout | `sizeof` | `_Alignof` |
| :--- | :--- | :--- | :--- |
| `vec2` | 2 lanes | `2 * width` | `sizeof` |
| `vec3` | 3 lanes + a zeroed `_pad` | `4 * width` | `sizeof` |
| `vec4` | 4 lanes | `4 * width` | `sizeof` |

- A `vec3` is a `vec4` whose fourth lane is pinned to zero, so `vec3f` and
  `vec4f` are both 16 bytes at 16-byte alignment and share one 128-bit load.
- The fourth lane is padding, never metadata. A coordinate frame belongs to a
  spatial type in a higher tier; a numeric primitive does not carry one.
- Alignment always equals size, so an array of vectors is loadable and no
  element straddles a boundary.
- `data[]` is the SIMD seam and the only member not reached through an
  accessor.
- The geometry is checked by `_Static_assert`, never described in prose only.

`vec3b`/`vec4b` are storage. bfloat16 is carried as a 2-byte bit pattern with
`b16_from_f32`/`b16_to_f32` because no compiler on the six-target floor ships
it as an arithmetic scalar; the arithmetic surface is `vecNb_to_vecNf` and
`vecNb_from_vecNf`.

### Matrix Layout Law

A matrix is its columns and nothing else, stored column-major:

| Shape | Columns | `sizeof` (f32 / d64) | `_Alignof` (f32 / d64) |
| :--- | :--- | :--- | :--- |
| `mat2` | 2 × `vec2` | 16 / 32 | 8 / 16 |
| `mat3` | 3 × `vec3` | 48 / 96 | 16 / 32 |
| `mat4` | 4 × `vec4` | 64 / 128 | 16 / 32 |

- A matrix composes the vector types instead of restating their geometry:
  `sizeof(matNf) == N * sizeof(vecNf)` and `_Alignof(matNf) == _Alignof(vecNf)`.
- A `mat3` column is a `vec3`, so it carries the zeroed fourth lane and is one
  128-bit load; the flat `data[]` stride is therefore four lanes, and entry
  `(row, col)` is `data[col * 4 + row]`.
- A matrix is large, so its accessors take a `const` pointer and its arithmetic
  takes and returns the value.
- The geometry is checked by `_Static_assert`, never described in prose only.

### Formatting Law (deferred)

The floor now owns the inline `string` type (`lang/string.h`) and `chr("...")`
for stack text. Projections still wait for the text-type owner; `toString` arrives
in the shape's `.c` of the day, governed by the universal `toString` Law.

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

The six canonical targets are named once each in `lang/platform.h`:
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

The floor is eleven public headers plus the `vex.h` door, checked out at
`lib/` beside `vex/` and published as `vex-c`. The math surface is three headers — `lang/primitive.h`,
`vector/vector.h`, `matrix/matrix.h` — and eight translation units
(`lang/primitive.c`, `lang/string.c`, `vector/vec2.c`, `vector/vec3.c`,
`vector/vec4.c`, `matrix/mat2.c`, `matrix/mat3.c`, `matrix/mat4.c`). Headers carry types, layout assertions and
declarations; every function body lives in the `.c` for its shape, with no
`##` in the math surface. Three owner tests prove it on macOS arm64:
`test/primitive` for the scalar widths and the bfloat16 conversion,
`test/vector` for the vector layouts, accessors and arithmetic, and
`test/matrix` for matrix composition and arithmetic. No allocation,
no runtime state. `lang/string.h` ships the inline text type and `chr("...")`;
projections still wait for the text-type owner.

`.clangd` states `-xc -std=gnu23 -I.` for the directory. Without it clangd's
no-database fallback parses headers as Objective-C++, where `alignas` on an
anonymous union member is ignored and the layout assertions report false
failures.

## Scope and Limitations

- The repository is local-only; its declared remote has no published history
  yet.
- `lang/platform.h` is a compile-time answer only; runtime probing belongs to the
  capability owner.
- `debug/test_support.h` carries no runner.
- Windows and Linux builds are declared, not yet proven.
- Matrices are `f32`/`d64` only. `b16` is a vector storage format; there is no
  matrix at bfloat16 width.
- `mat2`/`mat3`/`mat4` ship `add`, `sub`, `scale`, `multiply`, `multiplyVec`,
  `transpose` and `determinant`. Inverse and affine helpers are not here yet.
- The workspace tools (`tools/prefs`, `tools/shell/test.sh`, `tools/platforms.json`)
  still discover repositories under `worktree/vex`; `lib` beside `vex/` is
  wired into the include path but not yet into that discovery, so its three
  owner suites are run directly rather than through `tools/shell/test.sh`.
