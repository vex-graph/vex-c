#ifndef UTIL_VEX_H
#define UTIL_VEX_H

// util/vex.h — the mandatory vocabulary, in one include.
//
// This is the floor: every tier includes it downward, and nothing here includes
// a repo above it. The set is deliberately small — source markers, loud
// failure, the six-target platform spelling, the constructor dispatcher, the
// width vocabulary, and the owner-suite skip code. No allocation, no boot, no
// computation.
//
// A production file normally includes the one header it uses; a test that needs
// the whole vocabulary includes this.

#include "util/annotation.h"
#include "util/constructor.h"
#include "util/platform.h"
#include "util/test_support.h"
#include "util/throw.h"
#include "util/types.h"

#endif
