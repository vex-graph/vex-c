#ifndef VEXC_VARCHAR_H
#define VEXC_VARCHAR_H

/**
 * varchar.h -- immutable character-limited encoded text floor primitive.
 *
 * `lib` (`vex-c`) owns the zero-dependency primitive body (`TextFormat`,
 * `varchar_data`, `varchar`, encoding/decoding, validation, comparisons,
 * getters, and `toString` projections) without allocating.
 *
 * `re` owns the arena-backed constructors (`varchar(...)`, `varchar_fromBytes`,
 * `varchar_clone`, `varchar_convert`, `varchar_free`) built on top of
 * `varchar_requiredBytes`, `varchar_initInto`, `varchar_initUtf8Into`, and
 * `varchar_convertInto`.
 */

#include "annotation.h"
#include "constructor.h"
#include "primitive.h"

#define VARCHAR_NUL_PAD_BYTES 4u

;;CLASSNAME("TextFormat")
/**
 * Native endian-agnostic character encodings supported by `varchar`.
 * Multi-byte formats (`FORMAT_UTF16`, `FORMAT_UTF32`) serialize code units
 * through explicit byte shifts so the in-memory layout is architecture-neutral.
 *
 * FORMAT_UTF8   - UTF-8 (default = 0); 1..4 bytes per Unicode scalar value
 * FORMAT_ASCII  - 7-bit US-ASCII (U+0000..U+007F); 1 byte per character
 * FORMAT_LATIN1 - ISO-8859-1 (U+0000..U+00FF); 1 byte per character
 * FORMAT_UTF16  - UTF-16; 2 or 4 bytes per Unicode scalar value
 * FORMAT_UTF32  - UTF-32; 4 bytes per Unicode scalar value
 * FORMAT_CESU8  - Compatibility Encoding Scheme for UTF-16: 8-Bit (UTR #26)
 */
typedef enum TextFormat {
    FORMAT_UTF8 = 0,
    FORMAT_ASCII = 1,
    FORMAT_LATIN1 = 2,
    FORMAT_UTF16 = 3,
    FORMAT_UTF32 = 4,
    FORMAT_CESU8 = 5
} TextFormat;

_Static_assert(sizeof(TextFormat) == 4, "TextFormat must stay 32-bit on the six-target floor");

;;CLASSNAME("varchar_data")
/**
 * Immutable bounded character-sequence header and inline encoded payload.
 *
 * u32 limit         - maximum allowed characters (Unicode code points, SQL VARCHAR(N))
 * u32 length        - active character (code point) count (0..limit)
 * u32 byteLength    - active encoded byte count excluding trailing NUL pad
 * TextFormat format - character encoding of value[] (default FORMAT_UTF8)
 * u8 value[]        - inline encoded bytes followed by 4 zero NUL-pad bytes
 */
typedef struct varchar_data {
    u32 limit;
    u32 length;
    u32 byteLength;
    TextFormat format;
    u8 value[];
} varchar_data;

/** Immutable handle to a bounded encoded character sequence. */
typedef const varchar_data *varchar;

_Static_assert(sizeof(varchar_data) == 16, "varchar_data header must be 16 bytes");
_Static_assert(sizeof(varchar) * 8 == VEX_ARCH_BITS, "varchar must be one 64-bit word on the six-target floor");

/**
 * Checks whether a TextFormat enum value is a supported encoding.
 * @param format encoding value to test
 * @return true when format is one of FORMAT_UTF8..FORMAT_CESU8
 */
bool TextFormat_isValid(TextFormat format);

/**
 * Returns the canonical IANA-style name for a TextFormat.
 * @param format encoding value
 * @return immutable ASCII name, or "UNKNOWN" when invalid
 */
string TextFormat_name(TextFormat format);

/**
 * Returns the minimum code-unit byte width for a TextFormat.
 * @param format encoding value
 * @return 1, 2, or 4 for valid formats; 0 when invalid
 */
u32 TextFormat_unitBytes(TextFormat format);

/**
 * Computes the total allocation size in bytes for a varchar payload.
 * @param byteLength encoded payload byte length excluding trailing NUL pad
 * @return total bytes required (header + byteLength + 4-byte NUL pad), or 0 on overflow
 */
bytes varchar_requiredBytes(bytes byteLength);

/**
 * Validates raw encoded bytes in a given TextFormat and counts Unicode characters.
 * @param rawBytes     encoded input bytes (may be nullptr only when byteLength == 0)
 * @param byteLength   byte count of rawBytes
 * @param format       encoding of rawBytes
 * @param outCharCount optional output receiving Unicode scalar character count
 * @return true when all bytes are valid in format
 */
bool varchar_measureBytes(const u8 *rawBytes, bytes byteLength, TextFormat format, u32 *outCharCount);

/**
 * Validates UTF-8 input and measures its character count and byte size when encoded into targetFormat.
 * @param utf8Text        UTF-8 source bytes (may be nullptr only when utf8ByteLength == 0)
 * @param utf8ByteLength  byte length of utf8Text
 * @param targetFormat    target encoding format
 * @param outCharCount    optional output receiving Unicode scalar character count
 * @param outEncodedBytes optional output receiving byte length in targetFormat
 * @return true when utf8Text is valid UTF-8 and every code point is representable in targetFormat
 */
bool varchar_measureUtf8As(string utf8Text, bytes utf8ByteLength, TextFormat targetFormat, u32 *outCharCount, bytes *outEncodedBytes);

/**
 * Measures the character count and byte size required to convert an existing varchar to targetFormat.
 * @param src             source varchar
 * @param targetFormat    target encoding format
 * @param outCharCount    optional output receiving Unicode scalar character count
 * @param outEncodedBytes optional output receiving byte length in targetFormat
 * @return true when src can be losslessly represented in targetFormat
 */
bool varchar_measureConvert(varchar src, TextFormat targetFormat, u32 *outCharCount, bytes *outEncodedBytes);

/**
 * Placement-initializes an immutable varchar inside caller/arena storage from pre-encoded bytes.
 * @param storage         writable aligned backing memory owned by caller or arena
 * @param storageCapacity capacity in bytes of storage
 * @param limit           maximum allowed characters (SQL VARCHAR(N))
 * @param rawBytes        pre-encoded bytes in format (may be nullptr when byteLength == 0)
 * @param byteLength      byte count of rawBytes
 * @param format          encoding of rawBytes
 * @return immutable varchar handle pointing at storage, or nullptr on validation failure (THROW)
 */
varchar varchar_initInto(any storage, bytes storageCapacity, u32 limit, const u8 *rawBytes, bytes byteLength, TextFormat format);

/**
 * Placement-initializes an immutable varchar inside caller/arena storage by encoding UTF-8 input into targetFormat.
 * @param storage         writable aligned backing memory owned by caller or arena
 * @param storageCapacity capacity in bytes of storage
 * @param limit           maximum allowed characters (SQL VARCHAR(N))
 * @param utf8Text        UTF-8 source bytes (may be nullptr when utf8ByteLength == 0)
 * @param utf8ByteLength  byte count of utf8Text
 * @param targetFormat    target encoding format
 * @return immutable varchar handle pointing at storage, or nullptr on validation/overflow failure (THROW)
 */
varchar varchar_initUtf8Into(any storage, bytes storageCapacity, u32 limit, string utf8Text, bytes utf8ByteLength, TextFormat targetFormat);

/**
 * Placement-converts an existing varchar into targetFormat inside caller/arena storage, preserving limit.
 * @param src             source varchar
 * @param targetFormat    target encoding format
 * @param storage         writable aligned backing memory owned by caller or arena
 * @param storageCapacity capacity in bytes of storage
 * @return immutable varchar handle pointing at storage, or nullptr on failure (THROW)
 */
varchar varchar_convertInto(varchar src, TextFormat targetFormat, any storage, bytes storageCapacity);

/**
 * Decodes a varchar from any TextFormat into a NUL-terminated UTF-8 buffer without splitting code points.
 * @param self         source varchar
 * @param dest         destination UTF-8 buffer
 * @param cap          destination buffer capacity in bytes (including trailing NUL)
 * @param outTruncated optional output set true if not all characters fit in dest
 * @return UTF-8 bytes written excluding the trailing NUL
 */
bytes varchar_decodeUtf8(varchar self, utf8 *dest, bytes cap, bool *outTruncated);

/**
 * Exact same-format byte equality between two varchars (ignores limit).
 * @param self  first varchar
 * @param other second varchar
 * @return true when both are non-null, share the same format, and have identical bytes
 */
bool varchar_equals(varchar self, varchar other);

/**
 * Format-independent Unicode code-point equality between two varchars (ignores limit and encoding).
 * @param self  first varchar
 * @param other second varchar
 * @return true when both are non-null and decode to the exact same Unicode scalar sequence
 */
bool varchar_equalsCodePoints(varchar self, varchar other);

/**
 * Format-independent equality comparing a varchar against a NUL-terminated UTF-8 string.
 * @param self     varchar to compare
 * @param utf8Text NUL-terminated UTF-8 string
 * @return true when both are non-null and represent the exact same Unicode scalar sequence
 */
bool varchar_equalsString(varchar self, string utf8Text);

/**
 * Reads the maximum character limit of a varchar.
 * @param self varchar handle
 * @return declared character limit, or 0 when self is nullptr
 */
;;GETTER
u32 varchar_limit(varchar self);

/**
 * Reads the active character (Unicode code point) length of a varchar.
 * @param self varchar handle
 * @return active character count, or 0 when self is nullptr
 */
;;GETTER
u32 varchar_length(varchar self);

/**
 * Reads the active encoded byte length of a varchar (excluding NUL pad).
 * @param self varchar handle
 * @return active byte count, or 0 when self is nullptr
 */
;;GETTER
u32 varchar_byteLength(varchar self);

/**
 * Reads the TextFormat encoding of a varchar.
 * @param self varchar handle
 * @return TextFormat enum value, or FORMAT_UTF8 when self is nullptr
 */
;;GETTER
TextFormat varchar_format(varchar self);

/**
 * Checks whether a varchar has zero characters.
 * @param self varchar handle
 * @return true when self is nullptr or length == 0
 */
;;GETTER
bool varchar_isEmpty(varchar self);

/**
 * Borrows the raw encoded byte payload (followed by 4 zero NUL-pad bytes).
 * @param self varchar handle
 * @return read-only pointer to value[], or nullptr when self is nullptr
 */
;;GETTER
const u8 *varchar_value(varchar self);

/**
 * Borrows the payload as an immutable UTF-8 string when format is UTF-8 or ASCII.
 * @param self varchar handle
 * @return NUL-terminated string pointer for FORMAT_UTF8 / FORMAT_ASCII, or nullptr otherwise
 */
;;GETTER
string varchar_string(varchar self);

/**
 * Value projection: decodes varchar text into UTF-8 (cold path).
 * @param self         varchar handle
 * @param dest         destination UTF-8 buffer
 * @param cap          destination capacity in bytes
 * @param outTruncated optional output set true on truncation
 */
fn varchar_toString(varchar self, utf8 *dest, bytes cap, bool *outTruncated);

/**
 * Structure projection: writes varchar metadata and decoded text (cold path).
 * @param self         varchar handle
 * @param dest         destination UTF-8 buffer
 * @param cap          destination capacity in bytes
 * @param outTruncated optional output set true on truncation
 */
fn varchar_toStringStruct(varchar self, utf8 *dest, bytes cap, bool *outTruncated);

#endif
