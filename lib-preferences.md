# lib (vex-c) — Repo-Local Living Preferences

## Constitution Link

gist.md (`../../metadata/gists/preferences/gist.md`) is the compressed law list; all universal laws bind this repository. Read one on demand: `./tools/prefs law "<title>"`. Universal laws are inherited, never restated.

Tier: lifetime (floor). Owns: `vex.h` door, `lang/` scalar + string vocabulary, `vector/` and `matrix/` math layouts, `annotation.h`, platform spelling. Includes nothing above the C standard library.

## Exclusive Laws

| Law Title | Scope | Enforcement |
| **Floor Law** | dependency direction and ownership | no include leaves `lib`; dependency-free build |
| **One Door Law** | every consumer include | `#include "vex.h"`, no `lib/` or `vex/` prefix |
| **Primitive Vocabulary Law** | `lang/primitive.h`, `lang/vex_string.h`, `bytes`, `f32`/`d64` | static asserts in the owning test |
| **Byte Size Law** | `bytes` is `sizeof` type, never a field name | field names say what they count |
| **Math Header Split Law** | math headers | exactly `lang/primitive.h`, `vector/vector.h`, `matrix/matrix.h`; one `.c` per shape |
| **Accessor Form Law** | getters and setters | `;;GETTER` / `;;SETTER` inventory |
| **Vector Layout Law** | `vec2`/`vec3`/`vec4` geometry | `_Static_assert` on size and alignment |
| **Matrix Layout Law** | `mat2`/`mat3`/`mat4` geometry | `_Static_assert` on size and alignment |
| **Constructor Dispatch Law** | `Class(...)` arity spelling | owner suites compile the dispatch |
| **Annotation Marker Law** | every `;;MARKER` | marker expands to `_Static_assert` |
| **Loud Rejection Law** | `THROW` | cold-path only, no unwind, no exit |
| **Platform Spelling Law** | compile-time target identity | six exact targets plus derived axes |
| **Test Skip Law** | `B_TEST_SKIP` | 77 is the only skip code |

### Floor Law

* `lib` is the bottom of the dependency graph.
* Nothing here includes a repository, a platform header above the C standard library, or another part of the ecosystem.
* Every repository may include `lib` downward; `lib` includes none of them.
* Checked out at `lib/` beside `vex/`; the workspace adds that directory to the include path.
* `b.assembly.json` declares the repository root as its include root and no dependencies; the build compiles the headers alone to prove the claim.

### One Door Law

* The include is bare: `#include "vex.h"`.
* `lib/` is an include root, so the spelling never carries `lib/`, `vex/`, or a relative hop.
* `vex.h` is the one door: it includes the floor's headers by bare names (`"lang/primitive.h"`, `"lang/vex_string.h"`, `"annotation.h"`).
* A consumer needs no second include for the floor.
* No file includes a system header itself; `vex.h` batches them all, and a floor implementation file may add only its own internal headers after the door.
* Two spellings of the same header are a defect.

### Primitive Vocabulary Law

* The width is the name: `u8`, `i8`, `u16`, `i16`, `u32`, `i32`, `u64`, `i64`, `f32`, `d64` are the ecosystem spellings.
* `u32` is the default integer: a count, index, mask, flag or any value that can be unsigned is spelled `u32`; `i32` is used only when the value is signed. `u64` is the default when a wider integer is needed, `i64` the signed exception.
* Atomic spellings prefix the scalar with `a`: `ai8`..`ai64`, `au8`..`au64`, `af32`, `ad64`; the atomic form keeps the scalar's width and promises no particular lock-freedom.
* `aif64` is one whole-word atomic pair; `ald128` is two independently atomic halves (`ai64` + `ad64`), never a single 128-bit atomic.
* `int`, `long`, `long long` and the `stdint.h` names do not appear in new ecosystem source; the aliases are the only permitted spelling above this header.
* `b16` is bfloat16 — 2 bytes, 8-bit exponent, 7-bit mantissa — carried as a bit pattern with `b16_from_f32`/`b16_to_f32`.
* `b16` is not `_Float16` (IEEE binary16).
* `u128`/`i128` exist only when `__SIZEOF_INT128__` is defined; a target without them gets a compile error at the use site, never a silent wider substitute.
* `bytes` is `bytes`, the type `sizeof` returns; it is the size type, not a fixed-width integer.
* Rust projects the same vocabulary: `u8`..`u64`, `i8`..`i64`, `f32`, `bool` are already the name and get no alias; `d64`, `bytes`, `any`, `const_any`, `fnptr` are projected once by the Rust vocabulary module, never redefined per crate.
* `b16` and the float atomics (`af32`, `ad64`, `ald128`) are owned Rust types, not aliases: Rust core has no bfloat16 and no float atomics.
* `fn` is `void` in return position: a function returning no value is spelled `fn name(params)`.
* `discard` is `void` in discard-cast position: `(discard) value;` states an intentional discard. `(fn) value;` is legacy debt, not the spelling for new source.
* `fn` is `()` in Rust; `discard` is `let _ =`/`drop`; `pointer` is `*mut T`/`*const T` (or `usize` for an address), never a third alias.

### Byte Size Law

* `bytes` is `bytes`, never a field name.
* A field holds what it counts: `count` (number of elements), `capacity` (allocated slots), `stride` (bytes between records), `offset` (byte position in a buffer), `padding` (alignment filler), `alignment` (required alignment), `width`/`height` (dimension in native pixels).
* `padding` is alignment filler, never a synonym for object size.
* A bare `size` field is a defect.

### Math Header Split Law

* The math surface is exactly three headers: `lang/primitive.h` (scalar vocabulary), `vector/vector.h` (`vec2`/`vec3`/`vec4` at `b16`/`f32`/`d64`), `matrix/matrix.h` (`mat2`/`mat3`/`mat4` at `f32`/`d64`).
* One `.c` per shape; headers are include surfaces only (types, layout assertions, declarations), no function body in a header.
* Tag and typedef share the name; helpers stay private in the `.c`.
* Every function is written out; no family macro and no token pasting (`##`) in the math surface.
* The one remaining macro is the `Class(...)` shim from `lang/constructor.h`.
* Unused exports are dropped at link (`-flto`, or `-ffunction-sections` plus `-Wl,--gc-sections` / `-Wl,-dead_strip`), not at source.

### Accessor Form Law

* A state-bearing field is read through a getter and written through a setter.
* `(*ptr).field` appears only inside a getter or setter accessor, never at a call site.
* The getter takes the value (no `&`, no pointer layer, no `get` prefix).
* The setter takes the pointer and is the one place the field is assigned.
* Both are `static inline` and live in the kind header.
* `;;GETTER` and `;;SETTER` are the machine-readable inventory.

### Vector Layout Law

* A vector is its lanes and nothing else: `vec2` = 2 lanes; `vec3` = 3 lanes + zeroed `_pad`; `vec4` = 4 lanes.
* `sizeof` is `4 * width` for `vec3`/`vec4`, `2 * width` for `vec2`; `_Alignof` equals `sizeof`.
* A `vec3` is a `vec4` whose fourth lane is pinned to zero; `vec3f` and `vec4f` are both 16 bytes at 16-byte alignment and share one 128-bit load.
* Alignment always equals size; an array of vectors is loadable and no element straddles a boundary.
* `data[]` is the SIMD seam and the only member not reached through an accessor.
* The geometry is checked by `_Static_assert`, never described in prose only.
* `vec3b`/`vec4b` are storage; bfloat16 is carried as a 2-byte bit pattern; arithmetic is `vecNb_to_vecNf`/`vecNb_from_vecNf`.

### Matrix Layout Law

* A matrix is its columns and nothing else, stored column-major.
* `matN` composes the vector types: `sizeof(matNf) == N * sizeof(vecNf)` and `_Alignof(matNf) == _Alignof(vecNf)`.
* A `mat3` column is a `vec3` (zeroed fourth lane, one 128-bit load); flat `data[]` stride is four lanes; entry `(row, col)` is `data[col * 4 + row]`.
* A matrix is large: accessors take a `const` pointer; arithmetic takes and returns the value.
* The geometry is checked by `_Static_assert`, never described in prose only.

### Constructor Dispatch Law

* `Class(...)` is a macro over `Class_0`..`Class_8`, dispatching by argument count; it costs no code and allocates nothing.
* A class that exposes a constructor names every accepted arity and adds the one-line shim.
* An arity gap is intentional; a call into it fails to compile.
* Type-based overloading is out of scope.

### Annotation Marker Law

* `;;MARKER` carries two semicolons on the left only.
* `;;MARKER` expands to `_Static_assert(1, ...)`.
* The marker set is closed here; a new marker is added in one file and all consumers inherit it.

### Loud Rejection Law

* `THROW(fmt, ...)` writes exactly one `[vex] file:line: message` line to stderr and returns.
* `THROW` never unwinds, never exits.
* `THROW` is never called on a hot path, in a signal handler, or during bounded teardown.
* The caller returns a safe default.
* `grep -rn THROW` is the inventory of loud cold rejections.

### Platform Spelling Law

* The six canonical targets are named once each in `lang/platform.h`: `VEX_PLATFORM_APPLE_SILICON`, `VEX_PLATFORM_APPLE_X86_64`, `VEX_PLATFORM_LINUX_ARM64`, `VEX_PLATFORM_LINUX_X86_64`, `VEX_PLATFORM_WINDOWS_ARM64`, `VEX_PLATFORM_WINDOWS_X86_64`.
* The coarse axes are derived from the exact target, never the reverse.
* Raw `__APPLE__`, `_WIN32` and `__linux__` are resolved in this header and tested nowhere else.
* A target matching no branch is `#error`.

### Test Skip Law

* `B_TEST_SKIP` is 77; 77 is the only skip code.

## Repo-Local Extensions

* None — no managed exceptions to universal laws.

## Readiness Cross-Reference

* Feature readiness: lib readiness gist.
* Open blockers: ecosystem blockers gist.
* Proof lawbook: `tests/test-preferences.md` — `./tools/prefs test "<title>"`.
* Evidence ledger: `metadata/gists/test-checklist.md` — `python3 tools/test_checklist.py check`.
