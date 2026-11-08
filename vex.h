#ifndef VEXC_VEX_H
#define VEXC_VEX_H

// vex.h — the one include of the vex-c floor.
//
//     #include "vex.h"
//
// No `lib/` prefix and no repository prefix: this directory is an include root,
// so the door is spelled exactly once. It carries the floor's source markers,
// loud failure, the six-target platform spelling, constructor dispatch, the scalar
// vocabulary, and the owner-suite skip code. Nothing here includes a repository
// above R0, allocates, boots, or computes.

#include "lang/annotation.h"
#include "lang/constructor.h"
#include "lang/platform.h"
#include "lang/primitive.h"
#include "debug/test_support.h"
#include "debug/throw.h"

#endif
