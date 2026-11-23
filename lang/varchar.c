/**
 * varchar.c -- immutable character-limited encoded text floor primitive.
 *
 * Pure zero-allocation encoding, validation, measurement, placement
 * initialization, comparison, and projection logic owned by lib/lang.
 * Arena-backed constructors live in re (`re/primitive/varchar.c`).
 */

#include "vex.h"
#include "debug/throw.h"
#include "lang/varchar.h"

;;DEFINITION
/**
 * ============================================================================
 * DEFINITION: varchar
 * ============================================================================
 * Immutable character-limited encoded text floor primitive owned by lib/lang.
 * Carries a fixed 16-byte header (`limit`, `length`, `byteLength`, `format`)
 * followed by inline encoded bytes and a 4-byte zero NUL pad so UTF-8, UTF-16,
 * and UTF-32 payloads are all NUL-safe. Allocates zero bytes itself; `re`
 * owns the arena-backed constructors (`varchar(...)`, `varchar_clone`,
 * `varchar_convert`, `varchar_free`).
 * ============================================================================
 */

;;OVERVIEW
/**
 * ============================================================================
 * CLASS: varchar (lib/lang/varchar.c)
 * ============================================================================
 * Immutable handle (`typedef const varchar_data *varchar`) over a character-
 * bounded Unicode/ASCII/Latin-1 payload suitable for direct database mapping.
 *
 * --CONSTANTS----------------------------
 * public   const bytes VARCHAR_NUL_PAD_BYTES   4-byte trailing NUL pad (UTF-8/16/32 safe)
 * private  const u32   UNICODE_MAX             U+10FFFF maximum Unicode scalar value
 * private  const u32   SURROGATE_MIN           U+D800 start of UTF-16 surrogate range
 * private  const u32   SURROGATE_MAX           U+DFFF end of UTF-16 surrogate range
 * private  const u32   SURROGATE_HIGH_MAX      U+DBFF end of high surrogate range
 * private  const u32   SURROGATE_LOW_MIN       U+DC00 start of low surrogate range
 *
 * --FIELDS-------------------------------
 * public   u32         limit                   attribute, max Unicode characters (SQL VARCHAR(N))
 * public   u32         length                  attribute, active Unicode character count (0..limit)
 * public   u32         byteLength              attribute, active encoded byte length
 * public   TextFormat  format                  attribute, active encoding (default FORMAT_UTF8)
 * public   u8          value[]                 owns inline encoded bytes + 4-byte zero pad
 *
 * --FUNCTIONS----------------------------
 * public   TextFormat_isValid(...)             validates TextFormat enum range
 * public   TextFormat_name(...)                canonical IANA-style format name
 * public   TextFormat_unitBytes(...)           minimum code-unit width in bytes (1, 2, or 4)
 * public   varchar_requiredBytes(...)          header + payload + NUL pad total bytes
 * public   varchar_measureBytes(...)           validates encoded bytes and counts characters
 * public   varchar_measureUtf8As(...)          validates UTF-8 and measures target format size
 * public   varchar_measureConvert(...)         measures size to convert varchar to target format
 * public   varchar_initInto(...)               placement-init from pre-encoded bytes (THROW on fail)
 * public   varchar_initUtf8Into(...)           placement-init from UTF-8 transcoded to targetFormat
 * public   varchar_convertInto(...)            placement-convert between any two TextFormats
 * public   varchar_decodeUtf8(...)             bounded code-point-safe decode to UTF-8
 * public   varchar_equals(...)                 exact same-format byte equality
 * public   varchar_equalsCodePoints(...)       format-independent Unicode equality
 * public   varchar_equalsString(...)           format-independent equality vs UTF-8 string
 * public   varchar_toString(...)               value projection as UTF-8, cold path only
 * public   varchar_toStringStruct(...)         structure projection with metadata, cold path only
 * private  readU16(...)                        endian-agnostic 16-bit code-unit decode
 * private  writeU16(...)                       endian-agnostic 16-bit code-unit encode
 * private  readU32(...)                        endian-agnostic 32-bit code-unit decode
 * private  writeU32(...)                       endian-agnostic 32-bit code-unit encode
 * private  isScalarValue(...)                  true when code point is a valid Unicode scalar
 * private  decodeNextUtf8(...)                 decodes one scalar value from strict UTF-8
 * private  decodeNextCesu8(...)                decodes one scalar value from UTR #26 CESU-8
 * private  decodeNextUtf16(...)                decodes one scalar value from UTF-16
 * private  decodeNextCodePoint(...)            dispatches one scalar decode for any TextFormat
 * private  encodedCodePointBytes(...)          encoded byte length of one scalar in targetFormat
 * private  encodeCodePoint(...)                encodes one scalar into targetFormat
 * private  writeHeaderAndPad(...)              writes varchar_data header fields and 4-byte zero pad
 *
 * --GETTERS------------------------------
 * public   varchar_limit(...)                  reads character limit, null self is 0
 * public   varchar_length(...)                 reads active character count, null self is 0
 * public   varchar_byteLength(...)             reads encoded byte length, null self is 0
 * public   varchar_format(...)                 reads TextFormat, null self is FORMAT_UTF8
 * public   varchar_isEmpty(...)                true when null or length == 0
 * public   varchar_value(...)                  borrows raw encoded payload bytes
 * public   varchar_string(...)                 borrows UTF-8/ASCII payload as string, else nullptr
 * private  varchar_valueMut(...)               writable payload pointer during placement init
 *
 * --SETTERS------------------------------
 * private  varchar_setLimit(...)               sets limit during placement init
 * private  varchar_setLength(...)              sets length during placement init
 * private  varchar_setByteLength(...)          sets byteLength during placement init
 * private  varchar_setFormat(...)              sets format during placement init
 * ============================================================================
 */

#define UNICODE_MAX        0x10FFFFu
#define SURROGATE_MIN      0xD800u
#define SURROGATE_MAX      0xDFFFu
#define SURROGATE_HIGH_MAX 0xDBFFu
#define SURROGATE_LOW_MIN  0xDC00u
#define MAX_U32_LIMIT      0xFFFFFFFFu

/**
 * Borrows writable inline payload pointer during placement initialization.
 * @param self mutable varchar_data storage
 * @return writable byte pointer to value[], or nullptr when self is nullptr
 */
;;GETTER
static u8 *varchar_valueMut(varchar_data *self)
{
    if (!self)
        return nullptr;

    return (*self).value;
}

/**
 * Sets limit on freshly allocated varchar_data storage.
 * @param self  mutable varchar_data storage
 * @param limit declared maximum character limit
 */
;;SETTER
static fn varchar_setLimit(varchar_data *self, u32 limit)
{
    if (!self)
        return;

    (*self).limit = limit;
}

/**
 * Sets active character length on freshly allocated varchar_data storage.
 * @param self   mutable varchar_data storage
 * @param length active character count
 */
;;SETTER
static fn varchar_setLength(varchar_data *self, u32 length)
{
    if (!self)
        return;

    (*self).length = length;
}

/**
 * Sets encoded byte length on freshly allocated varchar_data storage.
 * @param self       mutable varchar_data storage
 * @param byteLength encoded byte count excluding NUL pad
 */
;;SETTER
static fn varchar_setByteLength(varchar_data *self, u32 byteLength)
{
    if (!self)
        return;

    (*self).byteLength = byteLength;
}

/**
 * Sets TextFormat on freshly allocated varchar_data storage.
 * @param self   mutable varchar_data storage
 * @param format encoding format
 */
;;SETTER
static fn varchar_setFormat(varchar_data *self, TextFormat format)
{
    if (!self)
        return;

    (*self).format = format;
}

/**
 * Reads a 16-bit code unit via canonical endian-agnostic byte shifts.
 * @param src two-byte source pointer
 * @return decoded 16-bit unsigned integer
 */
static u16 readU16(const u8 *src)
{
    return (u16) ((u16) src[0] | ((u16) src[1] << 8u));
}

/**
 * Writes a 16-bit code unit via canonical endian-agnostic byte shifts.
 * @param dst   two-byte destination pointer
 * @param value 16-bit code unit to write
 */
static fn writeU16(u8 *dst, u16 value)
{
    dst[0] = (u8) (value & 0xFFu);
    dst[1] = (u8) ((value >> 8u) & 0xFFu);
}

/**
 * Reads a 32-bit code unit via canonical endian-agnostic byte shifts.
 * @param src four-byte source pointer
 * @return decoded 32-bit unsigned integer
 */
static u32 readU32(const u8 *src)
{
    return (u32) src[0]
        | ((u32) src[1] << 8u)
        | ((u32) src[2] << 16u)
        | ((u32) src[3] << 24u);
}

/**
 * Writes a 32-bit code unit via canonical endian-agnostic byte shifts.
 * @param dst   four-byte destination pointer
 * @param value 32-bit code unit to write
 */
static fn writeU32(u8 *dst, u32 value)
{
    dst[0] = (u8) (value & 0xFFu);
    dst[1] = (u8) ((value >> 8u) & 0xFFu);
    dst[2] = (u8) ((value >> 16u) & 0xFFu);
    dst[3] = (u8) ((value >> 24u) & 0xFFu);
}

/**
 * Checks whether a 32-bit code point is a valid Unicode scalar value.
 * @param cp candidate Unicode code point
 * @return true when cp <= U+10FFFF and not in surrogate range U+D800..U+DFFF
 */
static bool isScalarValue(u32 cp)
{
    if (cp > UNICODE_MAX)
        return false;

    if (cp >= SURROGATE_MIN && cp <= SURROGATE_MAX)
        return false;

    return true;
}

/**
 * Decodes one Unicode scalar value from strict RFC 3629 UTF-8.
 * @param data   source byte buffer
 * @param len    total byte length of data
 * @param pos    in-out byte cursor advanced past the decoded sequence
 * @param outCp  output receiving decoded Unicode scalar value
 * @return true when a valid UTF-8 scalar sequence was decoded
 */
static bool decodeNextUtf8(const u8 *data, bytes len, bytes *pos, u32 *outCp)
{
    bytes i = *pos;

    if (i >= len)
        return false;

    u8 b0 = data[i];

    if (b0 < 0x80u) {
        *outCp = (u32) b0;
        *pos = i + 1u;
        return true;
    }

    if ((b0 & 0xE0u) == 0xC0u) {
        if (i + 1u >= len)
            return false;

        u8 b1 = data[i + 1u];

        if ((b1 & 0xC0u) != 0x80u)
            return false;

        u32 cp = (((u32) (b0 & 0x1Fu)) << 6u) | (u32) (b1 & 0x3Fu);

        if (cp < 0x80u)
            return false;

        *outCp = cp;
        *pos = i + 2u;
        return true;
    }

    if ((b0 & 0xF0u) == 0xE0u) {
        if (i + 2u >= len)
            return false;

        u8 b1 = data[i + 1u];
        u8 b2 = data[i + 2u];

        if ((b1 & 0xC0u) != 0x80u || (b2 & 0xC0u) != 0x80u)
            return false;

        u32 cp = (((u32) (b0 & 0x0Fu)) << 12u)
            | (((u32) (b1 & 0x3Fu)) << 6u)
            | (u32) (b2 & 0x3Fu);

        if (cp < 0x800u || !isScalarValue(cp))
            return false;

        *outCp = cp;
        *pos = i + 3u;
        return true;
    }

    if ((b0 & 0xF8u) == 0xF0u) {
        if (i + 3u >= len)
            return false;

        u8 b1 = data[i + 1u];
        u8 b2 = data[i + 2u];
        u8 b3 = data[i + 3u];

        if ((b1 & 0xC0u) != 0x80u || (b2 & 0xC0u) != 0x80u || (b3 & 0xC0u) != 0x80u)
            return false;

        u32 cp = (((u32) (b0 & 0x07u)) << 18u)
            | (((u32) (b1 & 0x3Fu)) << 12u)
            | (((u32) (b2 & 0x3Fu)) << 6u)
            | (u32) (b3 & 0x3Fu);

        if (cp < 0x10000u || cp > UNICODE_MAX)
            return false;

        *outCp = cp;
        *pos = i + 4u;
        return true;
    }

    return false;
}

/**
 * Decodes one Unicode scalar value from UTR #26 CESU-8 (BMP 1..3 bytes, supplementary 6-byte surrogate pair).
 * @param data   source byte buffer
 * @param len    total byte length of data
 * @param pos    in-out byte cursor advanced past the decoded sequence
 * @param outCp  output receiving decoded Unicode scalar value
 * @return true when a valid CESU-8 sequence was decoded
 */
static bool decodeNextCesu8(const u8 *data, bytes len, bytes *pos, u32 *outCp)
{
    bytes i = *pos;

    if (i >= len)
        return false;

    u8 b0 = data[i];

    if (b0 < 0x80u) {
        *outCp = (u32) b0;
        *pos = i + 1u;
        return true;
    }

    if ((b0 & 0xE0u) == 0xC0u) {
        if (i + 1u >= len)
            return false;

        u8 b1 = data[i + 1u];

        if ((b1 & 0xC0u) != 0x80u)
            return false;

        u32 cp = (((u32) (b0 & 0x1Fu)) << 6u) | (u32) (b1 & 0x3Fu);

        if (cp < 0x80u)
            return false;

        *outCp = cp;
        *pos = i + 2u;
        return true;
    }

    if ((b0 & 0xF0u) == 0xE0u) {
        if (i + 2u >= len)
            return false;

        u8 b1 = data[i + 1u];
        u8 b2 = data[i + 2u];

        if ((b1 & 0xC0u) != 0x80u || (b2 & 0xC0u) != 0x80u)
            return false;

        u32 w1 = (((u32) (b0 & 0x0Fu)) << 12u)
            | (((u32) (b1 & 0x3Fu)) << 6u)
            | (u32) (b2 & 0x3Fu);

        if (w1 < 0x800u)
            return false;

        if (w1 < SURROGATE_MIN || w1 > SURROGATE_MAX) {
            *outCp = w1;
            *pos = i + 3u;
            return true;
        }

        if (w1 > SURROGATE_HIGH_MAX)
            return false;

        if (i + 5u >= len)
            return false;

        u8 b3 = data[i + 3u];
        u8 b4 = data[i + 4u];
        u8 b5 = data[i + 5u];

        if ((b3 & 0xF0u) != 0xE0u || (b4 & 0xC0u) != 0x80u || (b5 & 0xC0u) != 0x80u)
            return false;

        u32 w2 = (((u32) (b3 & 0x0Fu)) << 12u)
            | (((u32) (b4 & 0x3Fu)) << 6u)
            | (u32) (b5 & 0x3Fu);

        if (w2 < SURROGATE_LOW_MIN || w2 > SURROGATE_MAX)
            return false;

        *outCp = 0x10000u + (((w1 - SURROGATE_MIN) << 10u) | (w2 - SURROGATE_LOW_MIN));
        *pos = i + 6u;
        return true;
    }

    return false;
}

/**
 * Decodes one Unicode scalar value from endian-agnostic UTF-16.
 * @param data  source byte buffer
 * @param len   total byte length of data
 * @param pos   in-out byte cursor advanced past the decoded code unit(s)
 * @param outCp output receiving decoded Unicode scalar value
 * @return true when a valid UTF-16 scalar was decoded
 */
static bool decodeNextUtf16(const u8 *data, bytes len, bytes *pos, u32 *outCp)
{
    bytes i = *pos;

    if (i + 2u > len)
        return false;

    u32 w1 = (u32) readU16(data + i);

    if (w1 < SURROGATE_MIN || w1 > SURROGATE_MAX) {
        *outCp = w1;
        *pos = i + 2u;
        return true;
    }

    if (w1 > SURROGATE_HIGH_MAX)
        return false;

    if (i + 4u > len)
        return false;

    u32 w2 = (u32) readU16(data + i + 2u);

    if (w2 < SURROGATE_LOW_MIN || w2 > SURROGATE_MAX)
        return false;

    *outCp = 0x10000u + (((w1 - SURROGATE_MIN) << 10u) | (w2 - SURROGATE_LOW_MIN));
    *pos = i + 4u;
    return true;
}

/**
 * Decodes one Unicode scalar value from any supported TextFormat.
 * @param data   source byte buffer
 * @param len    total byte length of data
 * @param format encoding of data
 * @param pos    in-out byte cursor advanced past the decoded character
 * @param outCp  output receiving decoded Unicode scalar value
 * @return true when a valid character was decoded
 */
static bool decodeNextCodePoint(const u8 *data, bytes len, TextFormat format, bytes *pos, u32 *outCp)
{
    bytes i = *pos;

    if (i >= len)
        return false;

    switch (format) {
        case FORMAT_UTF8:
            return decodeNextUtf8(data, len, pos, outCp);

        case FORMAT_ASCII: {
            u8 b = data[i];

            if (b >= 0x80u)
                return false;

            *outCp = (u32) b;
            *pos = i + 1u;
            return true;
        }

        case FORMAT_LATIN1:
            *outCp = (u32) data[i];
            *pos = i + 1u;
            return true;

        case FORMAT_UTF16:
            return decodeNextUtf16(data, len, pos, outCp);

        case FORMAT_UTF32: {
            if (i + 4u > len)
                return false;

            u32 cp = readU32(data + i);

            if (!isScalarValue(cp))
                return false;

            *outCp = cp;
            *pos = i + 4u;
            return true;
        }

        case FORMAT_CESU8:
            return decodeNextCesu8(data, len, pos, outCp);
    }

    return false;
}

/**
 * Computes the encoded byte length of a single Unicode scalar value in targetFormat.
 * @param cp           valid Unicode scalar value
 * @param targetFormat target TextFormat
 * @return 1..6 encoded bytes, or 0 if cp cannot be represented in targetFormat
 */
static u32 encodedCodePointBytes(u32 cp, TextFormat targetFormat)
{
    if (!isScalarValue(cp))
        return 0u;

    switch (targetFormat) {
        case FORMAT_UTF8:
            if (cp < 0x80u)
                return 1u;

            if (cp < 0x800u)
                return 2u;

            if (cp < 0x10000u)
                return 3u;

            return 4u;

        case FORMAT_ASCII:
            if (cp < 0x80u)
                return 1u;

            return 0u;

        case FORMAT_LATIN1:
            if (cp <= 0xFFu)
                return 1u;

            return 0u;

        case FORMAT_UTF16:
            if (cp < 0x10000u)
                return 2u;

            return 4u;

        case FORMAT_UTF32:
            return 4u;

        case FORMAT_CESU8:
            if (cp < 0x80u)
                return 1u;

            if (cp < 0x800u)
                return 2u;

            if (cp < 0x10000u)
                return 3u;

            return 6u;
    }

    return 0u;
}

/**
 * Encodes a single Unicode scalar value into dst in targetFormat.
 * @param cp           valid Unicode scalar value
 * @param targetFormat target TextFormat
 * @param dst          destination buffer with at least encodedCodePointBytes(cp, targetFormat) bytes
 * @return number of bytes written, or 0 if cp cannot be encoded in targetFormat
 */
static u32 encodeCodePoint(u32 cp, TextFormat targetFormat, u8 *dst)
{
    if (!isScalarValue(cp) || !dst)
        return 0u;

    switch (targetFormat) {
        case FORMAT_UTF8:
            if (cp < 0x80u) {
                dst[0] = (u8) cp;
                return 1u;
            }

            if (cp < 0x800u) {
                dst[0] = (u8) (0xC0u | (cp >> 6u));
                dst[1] = (u8) (0x80u | (cp & 0x3Fu));
                return 2u;
            }

            if (cp < 0x10000u) {
                dst[0] = (u8) (0xE0u | (cp >> 12u));
                dst[1] = (u8) (0x80u | ((cp >> 6u) & 0x3Fu));
                dst[2] = (u8) (0x80u | (cp & 0x3Fu));
                return 3u;
            }

            dst[0] = (u8) (0xF0u | (cp >> 18u));
            dst[1] = (u8) (0x80u | ((cp >> 12u) & 0x3Fu));
            dst[2] = (u8) (0x80u | ((cp >> 6u) & 0x3Fu));
            dst[3] = (u8) (0x80u | (cp & 0x3Fu));
            return 4u;

        case FORMAT_ASCII:
            if (cp >= 0x80u)
                return 0u;

            dst[0] = (u8) cp;
            return 1u;

        case FORMAT_LATIN1:
            if (cp > 0xFFu)
                return 0u;

            dst[0] = (u8) cp;
            return 1u;

        case FORMAT_UTF16: {
            if (cp < 0x10000u) {
                writeU16(dst, (u16) cp);
                return 2u;
            }

            u32 adj = cp - 0x10000u;
            u16 hi = (u16) (SURROGATE_MIN + (adj >> 10u));
            u16 lo = (u16) (SURROGATE_LOW_MIN + (adj & 0x3FFu));
            writeU16(dst, hi);
            writeU16(dst + 2u, lo);
            return 4u;
        }

        case FORMAT_UTF32:
            writeU32(dst, cp);
            return 4u;

        case FORMAT_CESU8:
            if (cp < 0x10000u)
                return encodeCodePoint(cp, FORMAT_UTF8, dst);

            u32 adj = cp - 0x10000u;
            u32 hi = SURROGATE_MIN + (adj >> 10u);
            u32 lo = SURROGATE_LOW_MIN + (adj & 0x3FFu);
            dst[0] = (u8) (0xE0u | (hi >> 12u));
            dst[1] = (u8) (0x80u | ((hi >> 6u) & 0x3Fu));
            dst[2] = (u8) (0x80u | (hi & 0x3Fu));
            dst[3] = (u8) (0xE0u | (lo >> 12u));
            dst[4] = (u8) (0x80u | ((lo >> 6u) & 0x3Fu));
            dst[5] = (u8) (0x80u | (lo & 0x3Fu));
            return 6u;
    }

    return 0u;
}

/**
 * Writes the varchar_data header fields and trailing 4-byte zero NUL pad.
 * @param slot       mutable varchar_data block
 * @param limit      maximum character limit
 * @param length     active character count
 * @param byteLength active encoded byte length
 * @param format     encoding format
 */
static fn writeHeaderAndPad(varchar_data *slot, u32 limit, u32 length, u32 byteLength, TextFormat format)
{
    varchar_setLimit(slot, limit);
    varchar_setLength(slot, length);
    varchar_setByteLength(slot, byteLength);
    varchar_setFormat(slot, format);
    memset(varchar_valueMut(slot) + byteLength, 0, VARCHAR_NUL_PAD_BYTES);
}

/**
 * Checks whether a TextFormat enum value is a supported encoding.
 * @param format encoding value to test
 * @return true when format is one of FORMAT_UTF8..FORMAT_CESU8
 */
bool TextFormat_isValid(TextFormat format)
{
    switch (format) {
        case FORMAT_UTF8:
        case FORMAT_ASCII:
        case FORMAT_LATIN1:
        case FORMAT_UTF16:
        case FORMAT_UTF32:
        case FORMAT_CESU8:
            return true;
    }

    return false;
}

/**
 * Returns the canonical IANA-style name for a TextFormat.
 * @param format encoding value
 * @return immutable ASCII name, or "UNKNOWN" when invalid
 */
string TextFormat_name(TextFormat format)
{
    switch (format) {
        case FORMAT_UTF8:
            return "UTF-8";

        case FORMAT_ASCII:
            return "ASCII";

        case FORMAT_LATIN1:
            return "ISO-8859-1";

        case FORMAT_UTF16:
            return "UTF-16";

        case FORMAT_UTF32:
            return "UTF-32";

        case FORMAT_CESU8:
            return "CESU-8";
    }

    return "UNKNOWN";
}

/**
 * Returns the minimum code-unit byte width for a TextFormat.
 * @param format encoding value
 * @return 1, 2, or 4 for valid formats; 0 when invalid
 */
u32 TextFormat_unitBytes(TextFormat format)
{
    switch (format) {
        case FORMAT_UTF8:
        case FORMAT_ASCII:
        case FORMAT_LATIN1:
        case FORMAT_CESU8:
            return 1u;

        case FORMAT_UTF16:
            return 2u;

        case FORMAT_UTF32:
            return 4u;
    }

    return 0u;
}

/**
 * Computes the total allocation size in bytes for a varchar payload.
 * @param byteLength encoded payload byte length excluding trailing NUL pad
 * @return total bytes required (header + byteLength + 4-byte NUL pad), or 0 on overflow
 */
bytes varchar_requiredBytes(bytes byteLength)
{
    bytes overhead = sizeof(varchar_data) + VARCHAR_NUL_PAD_BYTES;

    if (byteLength > (bytes) MAX_U32_LIMIT)
        return 0u;

    if (byteLength > ((bytes) -1) - overhead)
        return 0u;

    return overhead + byteLength;
}

/**
 * Validates raw encoded bytes in a given TextFormat and counts Unicode characters.
 * @param rawBytes     encoded input bytes (may be nullptr only when byteLength == 0)
 * @param byteLength   byte count of rawBytes
 * @param format       encoding of rawBytes
 * @param outCharCount optional output receiving Unicode scalar character count
 * @return true when all bytes are valid in format
 */
bool varchar_measureBytes(const u8 *rawBytes, bytes byteLength, TextFormat format, u32 *outCharCount)
{
    if (outCharCount)
        *outCharCount = 0u;

    if (!TextFormat_isValid(format))
        return false;

    if (!rawBytes && byteLength > 0u)
        return false;

    if (byteLength > (bytes) MAX_U32_LIMIT)
        return false;

    u32 unit = TextFormat_unitBytes(format);

    if (unit > 1u && (byteLength % (bytes) unit) != 0u)
        return false;

    bytes pos = 0u;
    u32 chars = 0u;

    while (pos < byteLength) {
        u32 cp = 0u;

        if (!decodeNextCodePoint(rawBytes, byteLength, format, &pos, &cp))
            return false;

        if (chars == MAX_U32_LIMIT)
            return false;

        chars++;
    }

    if (outCharCount)
        *outCharCount = chars;

    return true;
}

/**
 * Validates UTF-8 input and measures its character count and byte size when encoded into targetFormat.
 * @param utf8Text        UTF-8 source bytes (may be nullptr only when utf8ByteLength == 0)
 * @param utf8ByteLength  byte length of utf8Text
 * @param targetFormat    target encoding format
 * @param outCharCount    optional output receiving Unicode scalar character count
 * @param outEncodedBytes optional output receiving byte length in targetFormat
 * @return true when utf8Text is valid UTF-8 and every code point is representable in targetFormat
 */
bool varchar_measureUtf8As(string utf8Text, bytes utf8ByteLength, TextFormat targetFormat, u32 *outCharCount, bytes *outEncodedBytes)
{
    if (outCharCount)
        *outCharCount = 0u;

    if (outEncodedBytes)
        *outEncodedBytes = 0u;

    if (!TextFormat_isValid(targetFormat))
        return false;

    if (!utf8Text && utf8ByteLength > 0u)
        return false;

    const u8 *raw = (const u8*) utf8Text;
    bytes pos = 0u;
    u32 chars = 0u;
    bytes totalBytes = 0u;

    while (pos < utf8ByteLength) {
        u32 cp = 0u;

        if (!decodeNextUtf8(raw, utf8ByteLength, &pos, &cp))
            return false;

        u32 needed = encodedCodePointBytes(cp, targetFormat);

        if (needed == 0u)
            return false;

        if (totalBytes > (bytes) MAX_U32_LIMIT - (bytes) needed)
            return false;

        if (chars == MAX_U32_LIMIT)
            return false;

        totalBytes += (bytes) needed;
        chars++;
    }

    if (outCharCount)
        *outCharCount = chars;

    if (outEncodedBytes)
        *outEncodedBytes = totalBytes;

    return true;
}

/**
 * Measures the character count and byte size required to convert an existing varchar to targetFormat.
 * @param src             source varchar
 * @param targetFormat    target encoding format
 * @param outCharCount    optional output receiving Unicode scalar character count
 * @param outEncodedBytes optional output receiving byte length in targetFormat
 * @return true when src can be losslessly represented in targetFormat
 */
bool varchar_measureConvert(varchar src, TextFormat targetFormat, u32 *outCharCount, bytes *outEncodedBytes)
{
    if (outCharCount)
        *outCharCount = 0u;

    if (outEncodedBytes)
        *outEncodedBytes = 0u;

    if (!src || !TextFormat_isValid(targetFormat))
        return false;

    TextFormat srcFormat = varchar_format(src);

    if (!TextFormat_isValid(srcFormat))
        return false;

    const u8 *raw = varchar_value(src);
    bytes rawLen = (bytes) varchar_byteLength(src);
    bytes pos = 0u;
    u32 chars = 0u;
    bytes totalBytes = 0u;

    while (pos < rawLen) {
        u32 cp = 0u;

        if (!decodeNextCodePoint(raw, rawLen, srcFormat, &pos, &cp))
            return false;

        u32 needed = encodedCodePointBytes(cp, targetFormat);

        if (needed == 0u)
            return false;

        if (totalBytes > (bytes) MAX_U32_LIMIT - (bytes) needed)
            return false;

        if (chars == MAX_U32_LIMIT)
            return false;

        totalBytes += (bytes) needed;
        chars++;
    }

    if (outCharCount)
        *outCharCount = chars;

    if (outEncodedBytes)
        *outEncodedBytes = totalBytes;

    return true;
}

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
varchar varchar_initInto(any storage, bytes storageCapacity, u32 limit, const u8 *rawBytes, bytes byteLength, TextFormat format)
{
    if (!storage) {
        THROW("varchar: storage is nullptr");
        return nullptr;
    }

    if (!TextFormat_isValid(format)) {
        THROW("varchar: invalid TextFormat %u", (u32) format);
        return nullptr;
    }

    u32 charCount = 0u;

    if (!varchar_measureBytes(rawBytes, byteLength, format, &charCount)) {
        THROW("varchar: invalid %s byte sequence (%zu bytes)", TextFormat_name(format), byteLength);
        return nullptr;
    }

    if (charCount > limit) {
        THROW("varchar: %u characters exceed limit %u", charCount, limit);
        return nullptr;
    }

    bytes required = varchar_requiredBytes(byteLength);

    if (required == 0u || storageCapacity < required) {
        THROW("varchar: storage %zu bytes smaller than required %zu bytes", storageCapacity, required);
        return nullptr;
    }

    varchar_data *slot = (varchar_data*) storage;

    if (byteLength > 0u)
        memcpy(varchar_valueMut(slot), rawBytes, byteLength);

    writeHeaderAndPad(slot, limit, charCount, (u32) byteLength, format);
    return (varchar) slot;
}

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
varchar varchar_initUtf8Into(any storage, bytes storageCapacity, u32 limit, string utf8Text, bytes utf8ByteLength, TextFormat targetFormat)
{
    if (!storage) {
        THROW("varchar: storage is nullptr");
        return nullptr;
    }

    if (!TextFormat_isValid(targetFormat)) {
        THROW("varchar: invalid TextFormat %u", (u32) targetFormat);
        return nullptr;
    }

    u32 charCount = 0u;
    bytes encodedBytes = 0u;

    if (!varchar_measureUtf8As(utf8Text, utf8ByteLength, targetFormat, &charCount, &encodedBytes)) {
        THROW("varchar: UTF-8 input cannot be encoded as %s", TextFormat_name(targetFormat));
        return nullptr;
    }

    if (charCount > limit) {
        THROW("varchar: %u characters exceed limit %u", charCount, limit);
        return nullptr;
    }

    bytes required = varchar_requiredBytes(encodedBytes);

    if (required == 0u || storageCapacity < required) {
        THROW("varchar: storage %zu bytes smaller than required %zu bytes", storageCapacity, required);
        return nullptr;
    }

    varchar_data *slot = (varchar_data*) storage;
    u8 *dst = varchar_valueMut(slot);

    if (targetFormat == FORMAT_UTF8 || targetFormat == FORMAT_ASCII) {
        if (encodedBytes > 0u)
            memcpy(dst, utf8Text, encodedBytes);

        writeHeaderAndPad(slot, limit, charCount, (u32) encodedBytes, targetFormat);
        return (varchar) slot;
    }

    const u8 *raw = (const u8*) utf8Text;
    bytes pos = 0u;
    bytes outPos = 0u;

    while (pos < utf8ByteLength) {
        u32 cp = 0u;

        if (!decodeNextUtf8(raw, utf8ByteLength, &pos, &cp))
            return nullptr;

        u32 written = encodeCodePoint(cp, targetFormat, dst + outPos);

        if (written == 0u)
            return nullptr;

        outPos += (bytes) written;
    }

    writeHeaderAndPad(slot, limit, charCount, (u32) outPos, targetFormat);
    return (varchar) slot;
}

/**
 * Placement-converts an existing varchar into targetFormat inside caller/arena storage, preserving limit.
 * @param src             source varchar
 * @param targetFormat    target encoding format
 * @param storage         writable aligned backing memory owned by caller or arena
 * @param storageCapacity capacity in bytes of storage
 * @return immutable varchar handle pointing at storage, or nullptr on failure (THROW)
 */
varchar varchar_convertInto(varchar src, TextFormat targetFormat, any storage, bytes storageCapacity)
{
    if (!src || !storage) {
        THROW("varchar: convertInto received nullptr argument");
        return nullptr;
    }

    if (!TextFormat_isValid(targetFormat)) {
        THROW("varchar: invalid target TextFormat %u", (u32) targetFormat);
        return nullptr;
    }

    u32 charCount = 0u;
    bytes encodedBytes = 0u;

    if (!varchar_measureConvert(src, targetFormat, &charCount, &encodedBytes)) {
        THROW("varchar: cannot convert %s payload into %s",
              TextFormat_name(varchar_format(src)), TextFormat_name(targetFormat));
        return nullptr;
    }

    u32 limit = varchar_limit(src);

    if (charCount > limit) {
        THROW("varchar: %u characters exceed limit %u", charCount, limit);
        return nullptr;
    }

    bytes required = varchar_requiredBytes(encodedBytes);

    if (required == 0u || storageCapacity < required) {
        THROW("varchar: storage %zu bytes smaller than required %zu bytes", storageCapacity, required);
        return nullptr;
    }

    varchar_data *slot = (varchar_data*) storage;
    u8 *dst = varchar_valueMut(slot);
    TextFormat srcFormat = varchar_format(src);
    const u8 *raw = varchar_value(src);
    bytes rawLen = (bytes) varchar_byteLength(src);

    if (srcFormat == targetFormat) {
        if (rawLen > 0u)
            memcpy(dst, raw, rawLen);

        writeHeaderAndPad(slot, limit, charCount, (u32) rawLen, targetFormat);
        return (varchar) slot;
    }

    bytes pos = 0u;
    bytes outPos = 0u;

    while (pos < rawLen) {
        u32 cp = 0u;

        if (!decodeNextCodePoint(raw, rawLen, srcFormat, &pos, &cp))
            return nullptr;

        u32 written = encodeCodePoint(cp, targetFormat, dst + outPos);

        if (written == 0u)
            return nullptr;

        outPos += (bytes) written;
    }

    writeHeaderAndPad(slot, limit, charCount, (u32) outPos, targetFormat);
    return (varchar) slot;
}

/**
 * Decodes a varchar from any TextFormat into a NUL-terminated UTF-8 buffer without splitting code points.
 * @param self         source varchar
 * @param dest         destination UTF-8 buffer
 * @param cap          destination buffer capacity in bytes (including trailing NUL)
 * @param outTruncated optional output set true if not all characters fit in dest
 * @return UTF-8 bytes written excluding the trailing NUL
 */
bytes varchar_decodeUtf8(varchar self, utf8 *dest, bytes cap, bool *outTruncated)
{
    if (outTruncated)
        *outTruncated = false;

    if (!dest || cap == 0u)
        return 0u;

    if (!self) {
        dest[0] = '\0';
        return 0u;
    }

    TextFormat fmt = varchar_format(self);
    const u8 *raw = varchar_value(self);
    bytes rawLen = (bytes) varchar_byteLength(self);
    bytes pos = 0u;
    bytes outPos = 0u;
    bytes maxWrite = cap - 1u;

    while (pos < rawLen) {
        u32 cp = 0u;

        if (!decodeNextCodePoint(raw, rawLen, fmt, &pos, &cp))
            break;

        u8 encoded[4];
        u32 needed = encodeCodePoint(cp, FORMAT_UTF8, encoded);

        if (needed == 0u)
            break;

        if (outPos + (bytes) needed > maxWrite) {
            if (outTruncated)
                *outTruncated = true;

            break;
        }

        memcpy(dest + outPos, encoded, (bytes) needed);
        outPos += (bytes) needed;
    }

    dest[outPos] = '\0';
    return outPos;
}

/**
 * Exact same-format byte equality between two varchars (ignores limit).
 * @param self  first varchar
 * @param other second varchar
 * @return true when both are non-null, share the same format, and have identical bytes
 */
bool varchar_equals(varchar self, varchar other)
{
    if (!self || !other)
        return false;

    if (varchar_format(self) != varchar_format(other))
        return false;

    u32 selfBytes = varchar_byteLength(self);
    u32 otherBytes = varchar_byteLength(other);

    if (selfBytes != otherBytes)
        return false;

    if (selfBytes == 0u)
        return true;

    return memcmp(varchar_value(self), varchar_value(other), (bytes) selfBytes) == 0;
}

/**
 * Format-independent Unicode code-point equality between two varchars (ignores limit and encoding).
 * @param self  first varchar
 * @param other second varchar
 * @return true when both are non-null and decode to the exact same Unicode scalar sequence
 */
bool varchar_equalsCodePoints(varchar self, varchar other)
{
    if (!self || !other)
        return false;

    if (varchar_length(self) != varchar_length(other))
        return false;

    TextFormat fmtA = varchar_format(self);
    TextFormat fmtB = varchar_format(other);

    if (fmtA == fmtB)
        return varchar_equals(self, other);

    const u8 *rawA = varchar_value(self);
    const u8 *rawB = varchar_value(other);
    bytes lenA = (bytes) varchar_byteLength(self);
    bytes lenB = (bytes) varchar_byteLength(other);
    bytes posA = 0u;
    bytes posB = 0u;

    while (posA < lenA && posB < lenB) {
        u32 cpA = 0u;
        u32 cpB = 0u;

        if (!decodeNextCodePoint(rawA, lenA, fmtA, &posA, &cpA))
            return false;

        if (!decodeNextCodePoint(rawB, lenB, fmtB, &posB, &cpB))
            return false;

        if (cpA != cpB)
            return false;
    }

    return posA == lenA && posB == lenB;
}

/**
 * Format-independent equality comparing a varchar against a NUL-terminated UTF-8 string.
 * @param self     varchar to compare
 * @param utf8Text NUL-terminated UTF-8 string
 * @return true when both are non-null and represent the exact same Unicode scalar sequence
 */
bool varchar_equalsString(varchar self, string utf8Text)
{
    if (!self || !utf8Text)
        return false;

    bytes textLen = strlen(utf8Text);
    TextFormat fmt = varchar_format(self);

    if (fmt == FORMAT_UTF8 || fmt == FORMAT_ASCII) {
        bytes selfLen = (bytes) varchar_byteLength(self);

        if (selfLen != textLen)
            return false;

        if (selfLen == 0u)
            return true;

        return memcmp(varchar_value(self), utf8Text, selfLen) == 0;
    }

    const u8 *rawA = varchar_value(self);
    bytes lenA = (bytes) varchar_byteLength(self);
    const u8 *rawB = (const u8*) utf8Text;
    bytes posA = 0u;
    bytes posB = 0u;

    while (posA < lenA && posB < textLen) {
        u32 cpA = 0u;
        u32 cpB = 0u;

        if (!decodeNextCodePoint(rawA, lenA, fmt, &posA, &cpA))
            return false;

        if (!decodeNextUtf8(rawB, textLen, &posB, &cpB))
            return false;

        if (cpA != cpB)
            return false;
    }

    return posA == lenA && posB == textLen;
}

/**
 * Reads the maximum character limit of a varchar.
 * @param self varchar handle
 * @return declared character limit, or 0 when self is nullptr
 */
;;GETTER
u32 varchar_limit(varchar self)
{
    if (!self)
        return 0u;

    return (*self).limit;
}

/**
 * Reads the active character (Unicode code point) length of a varchar.
 * @param self varchar handle
 * @return active character count, or 0 when self is nullptr
 */
;;GETTER
u32 varchar_length(varchar self)
{
    if (!self)
        return 0u;

    return (*self).length;
}

/**
 * Reads the active encoded byte length of a varchar (excluding NUL pad).
 * @param self varchar handle
 * @return active byte count, or 0 when self is nullptr
 */
;;GETTER
u32 varchar_byteLength(varchar self)
{
    if (!self)
        return 0u;

    return (*self).byteLength;
}

/**
 * Reads the TextFormat encoding of a varchar.
 * @param self varchar handle
 * @return TextFormat enum value, or FORMAT_UTF8 when self is nullptr
 */
;;GETTER
TextFormat varchar_format(varchar self)
{
    if (!self)
        return FORMAT_UTF8;

    return (*self).format;
}

/**
 * Checks whether a varchar has zero characters.
 * @param self varchar handle
 * @return true when self is nullptr or length == 0
 */
;;GETTER
bool varchar_isEmpty(varchar self)
{
    return varchar_length(self) == 0u;
}

/**
 * Borrows the raw encoded byte payload (followed by 4 zero NUL-pad bytes).
 * @param self varchar handle
 * @return read-only pointer to value[], or nullptr when self is nullptr
 */
;;GETTER
const u8 *varchar_value(varchar self)
{
    if (!self)
        return nullptr;

    return (*self).value;
}

/**
 * Borrows the payload as an immutable UTF-8 string when format is UTF-8 or ASCII.
 * @param self varchar handle
 * @return NUL-terminated string pointer for FORMAT_UTF8 / FORMAT_ASCII, or nullptr otherwise
 */
;;GETTER
string varchar_string(varchar self)
{
    if (!self)
        return nullptr;

    TextFormat fmt = (*self).format;

    if (fmt != FORMAT_UTF8 && fmt != FORMAT_ASCII)
        return nullptr;

    return (string) (*self).value;
}

/**
 * Value projection: decodes varchar text into UTF-8 (cold path).
 * @param self         varchar handle
 * @param dest         destination UTF-8 buffer
 * @param cap          destination capacity in bytes
 * @param outTruncated optional output set true on truncation
 */
fn varchar_toString(varchar self, utf8 *dest, bytes cap, bool *outTruncated)
{
    if (outTruncated)
        *outTruncated = false;

    if (!dest || cap == 0u)
        return;

    if (!self) {
        i32 written = snprintf(dest, cap, "nullptr");

        if (written < 0 || (bytes) written >= cap) {
            if (outTruncated)
                *outTruncated = true;
        }

        return;
    }

    varchar_decodeUtf8(self, dest, cap, outTruncated);
}

/**
 * Structure projection: writes varchar metadata and decoded text (cold path).
 * @param self         varchar handle
 * @param dest         destination UTF-8 buffer
 * @param cap          destination capacity in bytes
 * @param outTruncated optional output set true on truncation
 */
fn varchar_toStringStruct(varchar self, utf8 *dest, bytes cap, bool *outTruncated)
{
    if (outTruncated)
        *outTruncated = false;

    if (!dest || cap == 0u)
        return;

    if (!self) {
        i32 written = snprintf(dest, cap, "nullptr");

        if (written < 0 || (bytes) written >= cap) {
            if (outTruncated)
                *outTruncated = true;
        }

        return;
    }

    utf8 body[256];
    bool bodyTruncated = false;
    varchar_decodeUtf8(self, body, sizeof(body), &bodyTruncated);

    i32 written = snprintf(
        dest,
        cap,
        "varchar { format=%s, limit=%u, length=%u, byteLength=%u, value=\"%s%s\" }",
        TextFormat_name(varchar_format(self)),
        varchar_limit(self),
        varchar_length(self),
        varchar_byteLength(self),
        body,
        bodyTruncated ? "..." : "");

    if (written < 0 || (bytes) written >= cap || bodyTruncated) {
        if (outTruncated)
            *outTruncated = true;
    }
}
