#include "vex.h"

;;DEFINITION
/**
 * ============================================================================
 * DEFINITION: variable
 * ============================================================================
 * The 32-byte identifier and pointer floor primitive (the 23+1 name box).
 * Holds a folded lowercase name (`1..23` ASCII characters over `[a-z0-9_$-]`
 * with `.` as the path segment splitter, NUL-terminated and zero-padded to
 * 24 bytes) plus an 8-byte `pointer` value-cell address. Two `variable` slots
 * pack into one 64-byte cache line with zero padding waste.
 *
 * Owned by the `lib` (`lifetime` floor) tier with zero dynamic allocation so
 * every higher tier can embed or pass `variable` by value or pointer; `re`
 * layers arena allocation and hash/pool tables on top.
 * ============================================================================
 */

;;OVERVIEW
/**
 * ============================================================================
 * CLASS: variable (lib/lang/variable.c)
 * ============================================================================
 * 32-byte folded name (`utf8 name[24]`) and 8-byte address (`pointer pointer`).
 *
 * --CONSTANTS----------------------------
 * public   const u32 VARIABLE_NAME_MAX       maximum identifier length (23)
 * public   const u32 VARIABLE_NAME_BYTES     padded name buffer size (24)
 * public   const u32 VARIABLE_SIZE           total struct size in bytes (32)
 *
 * --FIELDS-------------------------------
 * public   utf8    name[24]                  attribute, folded NUL-padded name
 * public   pointer pointer                   attribute, bound 64-bit address
 *
 * --CONSTRUCTORS-------------------------
 * public   variable_init(...)                in-place fold, validate, and bind
 *
 * --FUNCTIONS----------------------------
 * public   variable_foldName(...)            folds to lowercase and validates grammar
 * public   variable_bucketOf(...)            maps leading char to 0..38 bucket
 * public   variable_isEmpty(...)             true when null or name is empty
 * public   variable_nameEquals(...)          compares against candidate name
 * public   variable_toString(...)            value projection, cold path only
 * public   variable_toStringStruct(...)      structure projection, cold path only
 * private  storeFoldedName(...)              zero-pads and copies folded name
 *
 * --GETTERS------------------------------
 * public   variable_name(...)                returns borrowed folded name
 * public   variable_pointer(...)             returns bound pointer value
 * private  variable_nameMut(...)             mutable name buffer for init/setter
 *
 * --SETTERS------------------------------
 * public   variable_setName(...)             validates, folds, and updates name
 * public   variable_setPointer(...)          updates bound pointer value
 * ============================================================================
 */

;;GETTER
/**
 * Returns mutable access to the 24-byte inline name buffer.
 * @param self target variable slot
 * @return mutable pointer to self's name buffer
 */
static inline utf8 *variable_nameMut(variable *self)
{
    return (*self).name;
}

;;GETTER
/**
 * Returns the NUL-terminated folded name stored in self.
 * @param self variable slot to inspect
 * @return borrowed NUL-terminated name pointer, or nullptr when self is nullptr
 */
string variable_name(const variable *self)
{
    if (!self)
        return nullptr;

    return (*self).name;
}

;;GETTER
/**
 * Returns the bound value-cell pointer address stored in self.
 * @param self variable slot to inspect
 * @return bound pointer address, or 0 when self is nullptr
 */
pointer variable_pointer(const variable *self)
{
    if (!self)
        return 0u;

    return (*self).pointer;
}

/**
 * Copies a validated folded name into self, zero-padding all 24 bytes.
 * @param self   target variable slot
 * @param folded validated NUL-terminated folded name
 */
static fn storeFoldedName(variable *self, string folded)
{
    utf8 *dest = variable_nameMut(self);
    memset(dest, 0, VARIABLE_NAME_BYTES);
    memcpy(dest, folded, strlen(folded) + 1u);
}

/**
 * Folds an identifier to lowercase and validates the 23+1 segment grammar.
 * @param name input identifier (1..23 chars of [a-zA-Z0-9_$-] separated by '.')
 * @param out  destination buffer of at least VARIABLE_NAME_BYTES bytes
 * @return true when valid and written NUL-terminated to out
 */
bool variable_foldName(string name, utf8 *out)
{
    if (!name || !out || name[0] == '\0')
        return false;

    bytes len = 0u;
    bytes segLen = 0u;

    for (string p = name; ; p++)
    {
        utf8 c = *p;

        if (c == '\0' || c == '.')
        {
            if (segLen == 0u)
                return false;

            if (c == '\0')
            {
                out[len] = '\0';
                return true;
            }

            if (len >= VARIABLE_NAME_MAX)
                return false;

            out[len++] = '.';
            segLen = 0u;
            continue;
        }

        u8 u = (u8) c;
        bool ok = (u >= 'a' && u <= 'z') || (u >= 'A' && u <= 'Z') ||
                  (u >= '0' && u <= '9') || u == '_' || u == '$' || u == '-';

        if (!ok)
            return false;

        if (len >= VARIABLE_NAME_MAX)
            return false;

        out[len++] = (utf8) ((u >= 'A' && u <= 'Z') ? (u + 32u) : u);
        segLen++;
    }
}

/**
 * Maps a folded first character to its 0..38 radix bucket.
 * @param c folded ASCII character
 * @return bucket index in 0..38, or -1 if outside the variable charset
 */
i32 variable_bucketOf(utf8 c)
{
    if (c >= 'a' && c <= 'z')
        return (i32) (c - 'a');

    if (c >= '0' && c <= '9')
        return 26 + (i32) (c - '0');

    if (c == '_')
        return 36;

    if (c == '$')
        return 37;

    if (c == '-')
        return 38;

    return -1;
}

;;CONSTRUCTOR
/**
 * Initializes a variable slot in place by validating and folding name and binding target.
 * @param self   destination variable slot
 * @param name   identifier to validate and fold
 * @param target value-cell address to bind
 * @return true when self is non-null and name is valid
 */
bool variable_init(variable *self, string name, pointer target)
{
    if (!self)
        return false;

    utf8 folded[VARIABLE_NAME_BYTES];

    if (!variable_foldName(name, folded))
        return false;

    storeFoldedName(self, folded);
    variable_setPointer(self, target);
    return true;
}

;;SETTER
/**
 * Validates, folds, and stores a new name into self with zero-padding.
 * @param self target variable slot
 * @param name identifier to validate and fold
 * @return true when self is non-null and name is valid
 */
bool variable_setName(variable *self, string name)
{
    if (!self)
        return false;

    utf8 folded[VARIABLE_NAME_BYTES];

    if (!variable_foldName(name, folded))
        return false;

    storeFoldedName(self, folded);
    return true;
}

;;SETTER
/**
 * Updates the bound pointer address on self.
 * @param self   target variable slot
 * @param target value-cell address to bind
 */
fn variable_setPointer(variable *self, pointer target)
{
    if (!self)
        return;

    (*self).pointer = target;
}

/**
 * Checks whether a variable slot is null or has an empty name.
 * @param self variable slot to inspect
 * @return true when self is nullptr or its name starts with NUL
 */
bool variable_isEmpty(const variable *self)
{
    if (!self)
        return true;

    return variable_name(self)[0] == '\0';
}

/**
 * Checks whether self's stored name equals the folded form of name.
 * @param self variable slot to inspect
 * @param name candidate identifier (folded before comparison)
 * @return true when name is valid and matches self's folded name
 */
bool variable_nameEquals(const variable *self, string name)
{
    if (!self)
        return false;

    utf8 folded[VARIABLE_NAME_BYTES];

    if (!variable_foldName(name, folded))
        return false;

    string sname = variable_name(self);

    for (bytes i = 0u; i < VARIABLE_NAME_BYTES; i++)
    {
        if (sname[i] != folded[i])
            return false;

        if (sname[i] == '\0')
            return true;
    }

    return true;
}

/**
 * Formats a value projection of self into dest.
 * @param self         variable slot to format
 * @param dest         destination character buffer
 * @param cap          byte capacity of dest
 * @param outTruncated optional flag set to true when output was truncated
 */
fn variable_toString(const variable *self, utf8 *dest, bytes cap, bool *outTruncated)
{
    if (outTruncated)
        *outTruncated = false;

    if (!dest || cap == 0u)
        return;

    if (!self)
    {
        i32 n = snprintf(dest, cap, "nullptr");

        if ((n < 0 || (bytes) n >= cap) && outTruncated)
            *outTruncated = true;

        return;
    }

    string name = variable_name(self);

    if (name[0] == '\0')
        name = "?";

    i32 written = snprintf(dest, cap, "variable(%s => 0x%" PRIx64 ")", name,
                           (u64) variable_pointer(self));

    if ((written < 0 || (bytes) written >= cap) && outTruncated)
        *outTruncated = true;
}

/**
 * Formats a structural projection of self into dest.
 * @param self         variable slot to format
 * @param dest         destination character buffer
 * @param cap          byte capacity of dest
 * @param outTruncated optional flag set to true when output was truncated
 */
fn variable_toStringStruct(const variable *self, utf8 *dest, bytes cap, bool *outTruncated)
{
    if (outTruncated)
        *outTruncated = false;

    if (!dest || cap == 0u)
        return;

    if (!self)
    {
        i32 n = snprintf(dest, cap, "nullptr");

        if ((n < 0 || (bytes) n >= cap) && outTruncated)
            *outTruncated = true;

        return;
    }

    i32 written = snprintf(dest, cap, "variable { name=\"%s\", pointer=0x%" PRIx64 " }",
                           variable_name(self),
                           (u64) variable_pointer(self));

    if ((written < 0 || (bytes) written >= cap) && outTruncated)
        *outTruncated = true;
}
