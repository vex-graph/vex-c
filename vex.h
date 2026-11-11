#ifndef VEXC_VEX_H
#define VEXC_VEX_H

// vex.h — the one include of the vex-c floor.
// by yo vex, truly :D
//
//     #include "vex.h"
//
// No `lib/` prefix and no repository prefix: this directory is an include root,
// so the door is spelled exactly once. It carries the whole floor — source
// markers, loud failure, the six-target platform spelling, constructor
// dispatch, the scalar vocabulary, the vector and matrix value types, the
// stack text type (`string`, `chr("...")`), and the
// owner-suite skip code. Nothing here includes a repository above R0, allocates,
// boots, or computes.
//
// Every source file includes this door first and nothing system-level itself:
// the door batches the whole standard system surface the floor uses. A floor
// implementation file may add its own internal headers after the door.

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdalign.h>
#include <stdatomic.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "lang/lang.h"
#include "vector/vector.h"
#include "matrix/matrix.h"
#include "debug/test_support.h"
#include "debug/throw.h"

#endif
