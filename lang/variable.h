#ifndef VEXC_VARIABLE_H
#define VEXC_VARIABLE_H

/**
 * variable.h -- the 32-byte folded name and pointer floor primitive (the 23+1 rule).
 *
 * `lib` (`vex-c`) owns the zero-dependency 32-byte value layout (`variable`),
 * name folding and charset validation (`1..23` ASCII characters folded to
 * lowercase over `[a-z0-9_$-]` with `.` as the segment splitter, plus 1 NUL
 * terminator zero-padded to 24 bytes, plus an 8-byte `pointer` address),
 * inline initialization, getters, setters, and `toString` projections.
 *
 * `re` owns the arena-backed constructors (`variable(...)`, `variable_in`,
 * `variable_free`) and hash/pool tables over `variable`.
 */

#include "annotation.h"
#include "constructor.h"
#include "primitive.h"

#define VARIABLE_NAME_MAX 23u
#define VARIABLE_NAME_BYTES 24u
#define VARIABLE_SIZE 32u

#define VARIABLE_SLOT_NAME_MAX VARIABLE_NAME_MAX
#define VARIABLE_SLOT_NAME_BYTES VARIABLE_NAME_BYTES
#define VARIABLE_SLOT_SIZE VARIABLE_SIZE

;;INTENTION("variable is lowercase on purpose as a 32-byte floor primitive alongside string and varchar")
;;CLASSNAME("variable")
/**
 * 32-byte folded identifier and pointer slot (two slots pack into one 64-byte cache line).
 *
 * utf8 name[VARIABLE_NAME_BYTES] - folded lowercase name (1..23 chars plus NUL, zero-padded to 24 bytes)
 * pointer pointer                - bound 64-bit value-cell address
 */
typedef struct variable {
    utf8 name[VARIABLE_NAME_BYTES];
    pointer pointer;
} variable;

_Static_assert(sizeof(variable) == VARIABLE_SIZE, "variable must stay 32 bytes on the six-target floor");

/**
 * Folds an identifier to lowercase and validates the 23+1 segment grammar.
 * @param name input identifier (1..23 chars of [a-zA-Z0-9_$-] separated by '.')
 * @param out  destination buffer of at least VARIABLE_NAME_BYTES bytes
 * @return true when valid and written NUL-terminated to out
 */
bool variable_foldName(string name, utf8 *out);

/**
 * Maps a folded first character to its 0..38 radix bucket.
 * @param c folded ASCII character
 * @return bucket index in 0..38, or -1 if outside the variable charset
 */
i32 variable_bucketOf(utf8 c);

/**
 * Initializes a variable slot in place by validating and folding name and binding target.
 * @param self   destination variable slot
 * @param name   identifier to validate and fold
 * @param target value-cell address to bind
 * @return true when self is non-null and name is valid
 */
;;CONSTRUCTOR
bool variable_init(variable *self, string name, pointer target);

/**
 * Validates, folds, and stores a new name into self with zero-padding.
 * @param self target variable slot
 * @param name identifier to validate and fold
 * @return true when self is non-null and name is valid
 */
;;SETTER
bool variable_setName(variable *self, string name);

/**
 * Updates the bound pointer address on self.
 * @param self   target variable slot
 * @param target value-cell address to bind
 */
;;SETTER
fn variable_setPointer(variable *self, pointer target);

/**
 * Returns the NUL-terminated folded name stored in self.
 * @param self variable slot to inspect
 * @return borrowed NUL-terminated name pointer, or nullptr when self is nullptr
 */
;;GETTER
string variable_name(const variable *self);

/**
 * Returns the bound value-cell pointer address stored in self.
 * @param self variable slot to inspect
 * @return bound pointer address, or 0 when self is nullptr
 */
;;GETTER
pointer variable_pointer(const variable *self);

/**
 * Checks whether a variable slot is null or has an empty name.
 * @param self variable slot to inspect
 * @return true when self is nullptr or its name starts with NUL
 */
bool variable_isEmpty(const variable *self);

/**
 * Checks whether self's stored name equals the folded form of name.
 * @param self variable slot to inspect
 * @param name candidate identifier (folded before comparison)
 * @return true when name is valid and matches self's folded name
 */
bool variable_nameEquals(const variable *self, string name);

/**
 * Formats a value projection of self into dest.
 * @param self         variable slot to format
 * @param dest         destination character buffer
 * @param cap          byte capacity of dest
 * @param outTruncated optional flag set to true when output was truncated
 */
fn variable_toString(const variable *self, utf8 *dest, bytes cap, bool *outTruncated);

/**
 * Formats a structural projection of self into dest.
 * @param self         variable slot to format
 * @param dest         destination character buffer
 * @param cap          byte capacity of dest
 * @param outTruncated optional flag set to true when output was truncated
 */
fn variable_toStringStruct(const variable *self, utf8 *dest, bytes cap, bool *outTruncated);

#endif
