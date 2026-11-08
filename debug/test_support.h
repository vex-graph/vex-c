#ifndef VEXC_TEST_SUPPORT_H
#define VEXC_TEST_SUPPORT_H

// test_support.h — the shared vocabulary of the owner suites.
//
// Every test that can legitimately decline to run (no loopback port, no
// enrolled keychain item, no compositor surface) returns B_TEST_SKIP so the
// runner can tell "skipped" apart from "passed" and "failed". 77 is the
// long-standing skip code; do not renumber it.
//
// This header used to live in the archived tests/ repository. The suites that
// include it live in their own repositories, so the vocabulary lives with the
// workspace they are run from.

#define B_TEST_SKIP 77

#endif
