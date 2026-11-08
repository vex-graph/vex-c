#ifndef VEXC_CONSTRUCTOR_H
#define VEXC_CONSTRUCTOR_H

// constructor.h — Java-style constructor overloading for C23.
//
// C has no overloading; this header fakes the arity half of it with pure
// preprocessor dispatch. A class exports one function per accepted arity,
// named Name_0, Name_1, ... Name_8, plus a one-line shim:
//
//     Window *Window_0();
//     Window *Window_1(const char *title);
//     Window *Window_3(const char *title, int width, int height);
//     #define Window(...) CONSTRUCTOR_DISPATCH(Window, __VA_ARGS__)
//
// Call sites read like Java:
//
//     Window *a = Window();                        // => Window_0()
//     Window *b.sh = Window("_main");              // => Window_1("_main")
//     Window *c = Window("_main", 800, 600);       // => Window_3(...)
//
// Dispatch is by ARG COUNT only. Type-based overloading (add(x, y) over
// int/Vec3/Mat4) is a different problem and deliberately _out of scope here.
// Arity gaps are legal: declaring Window_0/_1/_3 without Window_2 means a
// 2-arg call fails to compile, exactly like Java missing that overload.
//
// Pure preprocessor: nothing here compiles to code, allocates, or costs a
// cycle at runtime. Extending past 8 args: grow CONSTRUCTOR_PICK and the
// NAME##_k ladder in step.
//
// Uses standard ISO C23 `__VA_OPT__(,) __VA_ARGS__` comma-elision, portable
// across all conforming C23 compilers without GCC/Clang extensions.

#define CONSTRUCTOR_COUNT_IMPL(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N

#define CONSTRUCTOR_COUNT(...) \
    CONSTRUCTOR_COUNT_IMPL(dummy __VA_OPT__(,) __VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define CONSTRUCTOR_CAT_INNER(a, b) a##b

#define CONSTRUCTOR_CAT(a, b) CONSTRUCTOR_CAT_INNER(a, b)

#define CONSTRUCTOR_DISPATCH(NAME, ...)                                    \
    CONSTRUCTOR_CAT(CONSTRUCTOR_CAT(NAME, _), CONSTRUCTOR_COUNT(__VA_ARGS__))(__VA_ARGS__)

#endif
