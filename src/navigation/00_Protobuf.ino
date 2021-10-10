/* Common parts of the nanopb library. Most of these are quite low-level
 * stuff. For the high-level interface, see pb_encode.h and pb_decode.h.
 */

#ifndef PB_H_INCLUDED
#define PB_H_INCLUDED

/*****************************************************************
 * Nanopb compilation time options. You can change these here by *
 * uncommenting the lines, or on the compiler command line.      *
 *****************************************************************/

/* Enable support for dynamically allocated fields */
/* #define PB_ENABLE_MALLOC 1 */

/* Define this if your CPU / compiler combination does not support
 * unaligned memory access to packed structures. */
/* #define PB_NO_PACKED_STRUCTS 1 */

/* Increase the number of required fields that are tracked.
 * A compiler warning will tell if you need this. */
/* #define PB_MAX_REQUIRED_FIELDS 256 */

/* Add support for tag numbers > 65536 and fields larger than 65536 bytes. */
/* #define PB_FIELD_32BIT 1 */

/* Disable support for error messages in order to save some code space. */
/* #define PB_NO_ERRMSG 1 */

/* Disable support for custom streams (support only memory buffers). */
/* #define PB_BUFFER_ONLY 1 */

/* Disable support for 64-bit datatypes, for compilers without int64_t
   or to save some code space. */
/* #define PB_WITHOUT_64BIT 1 */

/* Don't encode scalar arrays as packed. This is only to be used when
 * the decoder on the receiving side cannot process packed scalar arrays.
 * Such example is older protobuf.js. */
/* #define PB_ENCODE_ARRAYS_UNPACKED 1 */

/* Enable conversion of doubles to floats for platforms that do not
 * support 64-bit doubles. Most commonly AVR. */
/* #define PB_CONVERT_DOUBLE_FLOAT 1 */

/* Check whether incoming strings are valid UTF-8 sequences. Slows down
 * the string processing slightly and slightly increases code size. */
/* #define PB_VALIDATE_UTF8 1 */

/******************************************************************
 * You usually don't need to change anything below this line.     *
 * Feel free to look around and use the defined macros, though.   *
 ******************************************************************/


/* Version of the nanopb library. Just in case you want to check it in
 * your own program. */
#define NANOPB_VERSION nanopb-0.4.6-dev

/* Include all the system headers needed by nanopb. You will need the
 * definitions of the following:
 * - strlen, memcpy, memset functions
 * - [u]int_least8_t, uint_fast8_t, [u]int_least16_t, [u]int32_t, [u]int64_t
 * - size_t
 * - bool
 *
 * If you don't have the standard header files, you can instead provide
 * a custom header that defines or includes all this. In that case,
 * define PB_SYSTEM_HEADER to the path of this file.
 */
#ifdef PB_SYSTEM_HEADER
#include PB_SYSTEM_HEADER
#else
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

#ifdef PB_ENABLE_MALLOC
#include <stdlib.h>
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Macro for defining packed structures (compiler dependent).
 * This just reduces memory requirements, but is not required.
 */
#if defined(PB_NO_PACKED_STRUCTS)
    /* Disable struct packing */
#   define PB_PACKED_STRUCT_START
#   define PB_PACKED_STRUCT_END
#   define pb_packed
#elif defined(__GNUC__) || defined(__clang__)
    /* For GCC and clang */
#   define PB_PACKED_STRUCT_START
#   define PB_PACKED_STRUCT_END
#   define pb_packed __attribute__((packed))
#elif defined(__ICCARM__) || defined(__CC_ARM)
    /* For IAR ARM and Keil MDK-ARM compilers */
#   define PB_PACKED_STRUCT_START _Pragma("pack(push, 1)")
#   define PB_PACKED_STRUCT_END _Pragma("pack(pop)")
#   define pb_packed
#elif defined(_MSC_VER) && (_MSC_VER >= 1500)
    /* For Microsoft Visual C++ */
#   define PB_PACKED_STRUCT_START __pragma(pack(push, 1))
#   define PB_PACKED_STRUCT_END __pragma(pack(pop))
#   define pb_packed
#else
    /* Unknown compiler */
#   define PB_PACKED_STRUCT_START
#   define PB_PACKED_STRUCT_END
#   define pb_packed
#endif

/* Handly macro for suppressing unreferenced-parameter compiler warnings. */
#ifndef PB_UNUSED
#define PB_UNUSED(x) (void)(x)
#endif

/* Harvard-architecture processors may need special attributes for storing
 * field information in program memory. */
#ifndef PB_PROGMEM
#ifdef __AVR__
#include <avr/pgmspace.h>
#define PB_PROGMEM             PROGMEM
#define PB_PROGMEM_READU32(x)  pgm_read_dword(&x)
#else
#define PB_PROGMEM
#define PB_PROGMEM_READU32(x)  (x)
#endif
#endif

/* Compile-time assertion, used for checking compatible compilation options.
 * If this does not work properly on your compiler, use
 * #define PB_NO_STATIC_ASSERT to disable it.
 *
 * But before doing that, check carefully the error message / place where it
 * comes from to see if the error has a real cause. Unfortunately the error
 * message is not always very clear to read, but you can see the reason better
 * in the place where the PB_STATIC_ASSERT macro was called.
 */
#ifndef PB_NO_STATIC_ASSERT
#  ifndef PB_STATIC_ASSERT
#    if defined(__ICCARM__)
       /* IAR has static_assert keyword but no _Static_assert */
#      define PB_STATIC_ASSERT(COND,MSG) static_assert(COND,#MSG);
#    elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
       /* C11 standard _Static_assert mechanism */
#      define PB_STATIC_ASSERT(COND,MSG) _Static_assert(COND,#MSG);
#    else
       /* Classic negative-size-array static assert mechanism */
#      define PB_STATIC_ASSERT(COND,MSG) typedef char PB_STATIC_ASSERT_MSG(MSG, __LINE__, __COUNTER__)[(COND)?1:-1];
#      define PB_STATIC_ASSERT_MSG(MSG, LINE, COUNTER) PB_STATIC_ASSERT_MSG_(MSG, LINE, COUNTER)
#      define PB_STATIC_ASSERT_MSG_(MSG, LINE, COUNTER) pb_static_assertion_##MSG##_##LINE##_##COUNTER
#    endif
#  endif
#else
   /* Static asserts disabled by PB_NO_STATIC_ASSERT */
#  define PB_STATIC_ASSERT(COND,MSG)
#endif

/* Number of required fields to keep track of. */
#ifndef PB_MAX_REQUIRED_FIELDS
#define PB_MAX_REQUIRED_FIELDS 64
#endif

#if PB_MAX_REQUIRED_FIELDS < 64
#error You should not lower PB_MAX_REQUIRED_FIELDS from the default value (64).
#endif

#ifdef PB_WITHOUT_64BIT
#ifdef PB_CONVERT_DOUBLE_FLOAT
/* Cannot use doubles without 64-bit types */
#undef PB_CONVERT_DOUBLE_FLOAT
#endif
#endif

/* List of possible field types. These are used in the autogenerated code.
 * Least-significant 4 bits tell the scalar type
 * Most-significant 4 bits specify repeated/required/packed etc.
 */

typedef uint_least8_t pb_type_t;

/**** Field data types ****/

/* Numeric types */
#define PB_LTYPE_BOOL    0x00U /* bool */
#define PB_LTYPE_VARINT  0x01U /* int32, int64, enum, bool */
#define PB_LTYPE_UVARINT 0x02U /* uint32, uint64 */
#define PB_LTYPE_SVARINT 0x03U /* sint32, sint64 */
#define PB_LTYPE_FIXED32 0x04U /* fixed32, sfixed32, float */
#define PB_LTYPE_FIXED64 0x05U /* fixed64, sfixed64, double */

/* Marker for last packable field type. */
#define PB_LTYPE_LAST_PACKABLE 0x05U

/* Byte array with pre-allocated buffer.
 * data_size is the length of the allocated PB_BYTES_ARRAY structure. */
#define PB_LTYPE_BYTES 0x06U

/* String with pre-allocated buffer.
 * data_size is the maximum length. */
#define PB_LTYPE_STRING 0x07U

/* Submessage
 * submsg_fields is pointer to field descriptions */
#define PB_LTYPE_SUBMESSAGE 0x08U

/* Submessage with pre-decoding callback
 * The pre-decoding callback is stored as pb_callback_t right before pSize.
 * submsg_fields is pointer to field descriptions */
#define PB_LTYPE_SUBMSG_W_CB 0x09U

/* Extension pseudo-field
 * The field contains a pointer to pb_extension_t */
#define PB_LTYPE_EXTENSION 0x0AU

/* Byte array with inline, pre-allocated byffer.
 * data_size is the length of the inline, allocated buffer.
 * This differs from PB_LTYPE_BYTES by defining the element as
 * pb_byte_t[data_size] rather than pb_bytes_array_t. */
#define PB_LTYPE_FIXED_LENGTH_BYTES 0x0BU

/* Number of declared LTYPES */
#define PB_LTYPES_COUNT 0x0CU
#define PB_LTYPE_MASK 0x0FU

/**** Field repetition rules ****/

#define PB_HTYPE_REQUIRED 0x00U
#define PB_HTYPE_OPTIONAL 0x10U
#define PB_HTYPE_SINGULAR 0x10U
#define PB_HTYPE_REPEATED 0x20U
#define PB_HTYPE_FIXARRAY 0x20U
#define PB_HTYPE_ONEOF    0x30U
#define PB_HTYPE_MASK     0x30U

/**** Field allocation types ****/
 
#define PB_ATYPE_STATIC   0x00U
#define PB_ATYPE_POINTER  0x80U
#define PB_ATYPE_CALLBACK 0x40U
#define PB_ATYPE_MASK     0xC0U

#define PB_ATYPE(x) ((x) & PB_ATYPE_MASK)
#define PB_HTYPE(x) ((x) & PB_HTYPE_MASK)
#define PB_LTYPE(x) ((x) & PB_LTYPE_MASK)
#define PB_LTYPE_IS_SUBMSG(x) (PB_LTYPE(x) == PB_LTYPE_SUBMESSAGE || \
                               PB_LTYPE(x) == PB_LTYPE_SUBMSG_W_CB)

/* Data type used for storing sizes of struct fields
 * and array counts.
 */
#if defined(PB_FIELD_32BIT)
    typedef uint32_t pb_size_t;
    typedef int32_t pb_ssize_t;
#else
    typedef uint_least16_t pb_size_t;
    typedef int_least16_t pb_ssize_t;
#endif
#define PB_SIZE_MAX ((pb_size_t)-1)

/* Data type for storing encoded data and other byte streams.
 * This typedef exists to support platforms where uint8_t does not exist.
 * You can regard it as equivalent on uint8_t on other platforms.
 */
typedef uint_least8_t pb_byte_t;

/* Forward declaration of struct types */
typedef struct pb_istream_s pb_istream_t;
typedef struct pb_ostream_s pb_ostream_t;
typedef struct pb_field_iter_s pb_field_iter_t;

/* This structure is used in auto-generated constants
 * to specify struct fields.
 */
typedef struct pb_msgdesc_s pb_msgdesc_t;
struct pb_msgdesc_s {
    const uint32_t *field_info;
    const pb_msgdesc_t * const * submsg_info;
    const pb_byte_t *default_value;

    bool (*field_callback)(pb_istream_t *istream, pb_ostream_t *ostream, const pb_field_iter_t *field);

    pb_size_t field_count;
    pb_size_t required_field_count;
    pb_size_t largest_tag;
};

/* Iterator for message descriptor */
struct pb_field_iter_s {
    const pb_msgdesc_t *descriptor;  /* Pointer to message descriptor constant */
    void *message;                   /* Pointer to start of the structure */

    pb_size_t index;                 /* Index of the field */
    pb_size_t field_info_index;      /* Index to descriptor->field_info array */
    pb_size_t required_field_index;  /* Index that counts only the required fields */
    pb_size_t submessage_index;      /* Index that counts only submessages */

    pb_size_t tag;                   /* Tag of current field */
    pb_size_t data_size;             /* sizeof() of a single item */
    pb_size_t array_size;            /* Number of array entries */
    pb_type_t type;                  /* Type of current field */

    void *pField;                    /* Pointer to current field in struct */
    void *pData;                     /* Pointer to current data contents. Different than pField for arrays and pointers. */
    void *pSize;                     /* Pointer to count/has field */

    const pb_msgdesc_t *submsg_desc; /* For submessage fields, pointer to field descriptor for the submessage. */
};

/* For compatibility with legacy code */
typedef pb_field_iter_t pb_field_t;

/* Make sure that the standard integer types are of the expected sizes.
 * Otherwise fixed32/fixed64 fields can break.
 *
 * If you get errors here, it probably means that your stdint.h is not
 * correct for your platform.
 */
#ifndef PB_WITHOUT_64BIT
PB_STATIC_ASSERT(sizeof(int64_t) == 2 * sizeof(int32_t), INT64_T_WRONG_SIZE)
PB_STATIC_ASSERT(sizeof(uint64_t) == 2 * sizeof(uint32_t), UINT64_T_WRONG_SIZE)
#endif

/* This structure is used for 'bytes' arrays.
 * It has the number of bytes in the beginning, and after that an array.
 * Note that actual structs used will have a different length of bytes array.
 */
#define PB_BYTES_ARRAY_T(n) struct { pb_size_t size; pb_byte_t bytes[n]; }
#define PB_BYTES_ARRAY_T_ALLOCSIZE(n) ((size_t)n + offsetof(pb_bytes_array_t, bytes))

struct pb_bytes_array_s {
    pb_size_t size;
    pb_byte_t bytes[1];
};
typedef struct pb_bytes_array_s pb_bytes_array_t;

/* This structure is used for giving the callback function.
 * It is stored in the message structure and filled in by the method that
 * calls pb_decode.
 *
 * The decoding callback will be given a limited-length stream
 * If the wire type was string, the length is the length of the string.
 * If the wire type was a varint/fixed32/fixed64, the length is the length
 * of the actual value.
 * The function may be called multiple times (especially for repeated types,
 * but also otherwise if the message happens to contain the field multiple
 * times.)
 *
 * The encoding callback will receive the actual output stream.
 * It should write all the data in one call, including the field tag and
 * wire type. It can write multiple fields.
 *
 * The callback can be null if you want to skip a field.
 */
typedef struct pb_callback_s pb_callback_t;
struct pb_callback_s {
    /* Callback functions receive a pointer to the arg field.
     * You can access the value of the field as *arg, and modify it if needed.
     */
    union {
        bool (*decode)(pb_istream_t *stream, const pb_field_t *field, void **arg);
        bool (*encode)(pb_ostream_t *stream, const pb_field_t *field, void * const *arg);
    } funcs;
    
    /* Free arg for use by callback */
    void *arg;
};

extern bool pb_default_field_callback(pb_istream_t *istream, pb_ostream_t *ostream, const pb_field_t *field);

/* Wire types. Library user needs these only in encoder callbacks. */
typedef enum {
    PB_WT_VARINT = 0,
    PB_WT_64BIT  = 1,
    PB_WT_STRING = 2,
    PB_WT_32BIT  = 5,
    PB_WT_PACKED = 255 /* PB_WT_PACKED is internal marker for packed arrays. */
} pb_wire_type_t;

/* Structure for defining the handling of unknown/extension fields.
 * Usually the pb_extension_type_t structure is automatically generated,
 * while the pb_extension_t structure is created by the user. However,
 * if you want to catch all unknown fields, you can also create a custom
 * pb_extension_type_t with your own callback.
 */
typedef struct pb_extension_type_s pb_extension_type_t;
typedef struct pb_extension_s pb_extension_t;
struct pb_extension_type_s {
    /* Called for each unknown field in the message.
     * If you handle the field, read off all of its data and return true.
     * If you do not handle the field, do not read anything and return true.
     * If you run into an error, return false.
     * Set to NULL for default handler.
     */
    bool (*decode)(pb_istream_t *stream, pb_extension_t *extension,
                   uint32_t tag, pb_wire_type_t wire_type);
    
    /* Called once after all regular fields have been encoded.
     * If you have something to write, do so and return true.
     * If you do not have anything to write, just return true.
     * If you run into an error, return false.
     * Set to NULL for default handler.
     */
    bool (*encode)(pb_ostream_t *stream, const pb_extension_t *extension);
    
    /* Free field for use by the callback. */
    const void *arg;
};

struct pb_extension_s {
    /* Type describing the extension field. Usually you'll initialize
     * this to a pointer to the automatically generated structure. */
    const pb_extension_type_t *type;
    
    /* Destination for the decoded data. This must match the datatype
     * of the extension field. */
    void *dest;
    
    /* Pointer to the next extension handler, or NULL.
     * If this extension does not match a field, the next handler is
     * automatically called. */
    pb_extension_t *next;

    /* The decoder sets this to true if the extension was found.
     * Ignored for encoding. */
    bool found;
};

#define pb_extension_init_zero {NULL,NULL,NULL,false}

/* Memory allocation functions to use. You can define pb_realloc and
 * pb_free to custom functions if you want. */
#ifdef PB_ENABLE_MALLOC
#   ifndef pb_realloc
#       define pb_realloc(ptr, size) realloc(ptr, size)
#   endif
#   ifndef pb_free
#       define pb_free(ptr) free(ptr)
#   endif
#endif

/* This is used to inform about need to regenerate .pb.h/.pb.c files. */
#define PB_PROTO_HEADER_VERSION 40

/* These macros are used to declare pb_field_t's in the constant array. */
/* Size of a structure member, in bytes. */
#define pb_membersize(st, m) (sizeof ((st*)0)->m)
/* Number of entries in an array. */
#define pb_arraysize(st, m) (pb_membersize(st, m) / pb_membersize(st, m[0]))
/* Delta from start of one member to the start of another member. */
#define pb_delta(st, m1, m2) ((int)offsetof(st, m1) - (int)offsetof(st, m2))

/* Force expansion of macro value */
#define PB_EXPAND(x) x

/* Binding of a message field set into a specific structure */
#define PB_BIND(msgname, structname, width) \
    const uint32_t structname ## _field_info[] PB_PROGMEM = \
    { \
        msgname ## _FIELDLIST(PB_GEN_FIELD_INFO_ ## width, structname) \
        0 \
    }; \
    const pb_msgdesc_t* const structname ## _submsg_info[] = \
    { \
        msgname ## _FIELDLIST(PB_GEN_SUBMSG_INFO, structname) \
        NULL \
    }; \
    const pb_msgdesc_t structname ## _msg = \
    { \
       structname ## _field_info, \
       structname ## _submsg_info, \
       msgname ## _DEFAULT, \
       msgname ## _CALLBACK, \
       0 msgname ## _FIELDLIST(PB_GEN_FIELD_COUNT, structname), \
       0 msgname ## _FIELDLIST(PB_GEN_REQ_FIELD_COUNT, structname), \
       0 msgname ## _FIELDLIST(PB_GEN_LARGEST_TAG, structname), \
    }; \
    msgname ## _FIELDLIST(PB_GEN_FIELD_INFO_ASSERT_ ## width, structname)

#define PB_GEN_FIELD_COUNT(structname, atype, htype, ltype, fieldname, tag) +1
#define PB_GEN_REQ_FIELD_COUNT(structname, atype, htype, ltype, fieldname, tag) \
    + (PB_HTYPE_ ## htype == PB_HTYPE_REQUIRED)
#define PB_GEN_LARGEST_TAG(structname, atype, htype, ltype, fieldname, tag) \
    * 0 + tag

/* X-macro for generating the entries in struct_field_info[] array. */
#define PB_GEN_FIELD_INFO_1(structname, atype, htype, ltype, fieldname, tag) \
    PB_FIELDINFO_1(tag, PB_ATYPE_ ## atype | PB_HTYPE_ ## htype | PB_LTYPE_MAP_ ## ltype, \
                   PB_DATA_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_DATA_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_SIZE_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_ARRAY_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname))

#define PB_GEN_FIELD_INFO_2(structname, atype, htype, ltype, fieldname, tag) \
    PB_FIELDINFO_2(tag, PB_ATYPE_ ## atype | PB_HTYPE_ ## htype | PB_LTYPE_MAP_ ## ltype, \
                   PB_DATA_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_DATA_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_SIZE_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_ARRAY_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname))

#define PB_GEN_FIELD_INFO_4(structname, atype, htype, ltype, fieldname, tag) \
    PB_FIELDINFO_4(tag, PB_ATYPE_ ## atype | PB_HTYPE_ ## htype | PB_LTYPE_MAP_ ## ltype, \
                   PB_DATA_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_DATA_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_SIZE_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_ARRAY_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname))

#define PB_GEN_FIELD_INFO_8(structname, atype, htype, ltype, fieldname, tag) \
    PB_FIELDINFO_8(tag, PB_ATYPE_ ## atype | PB_HTYPE_ ## htype | PB_LTYPE_MAP_ ## ltype, \
                   PB_DATA_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_DATA_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_SIZE_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_ARRAY_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname))

#define PB_GEN_FIELD_INFO_AUTO(structname, atype, htype, ltype, fieldname, tag) \
    PB_FIELDINFO_AUTO2(PB_FIELDINFO_WIDTH_AUTO(_PB_ATYPE_ ## atype, _PB_HTYPE_ ## htype, _PB_LTYPE_ ## ltype), \
                   tag, PB_ATYPE_ ## atype | PB_HTYPE_ ## htype | PB_LTYPE_MAP_ ## ltype, \
                   PB_DATA_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_DATA_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_SIZE_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_ARRAY_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname))

#define PB_FIELDINFO_AUTO2(width, tag, type, data_offset, data_size, size_offset, array_size) \
    PB_FIELDINFO_AUTO3(width, tag, type, data_offset, data_size, size_offset, array_size)

#define PB_FIELDINFO_AUTO3(width, tag, type, data_offset, data_size, size_offset, array_size) \
    PB_FIELDINFO_ ## width(tag, type, data_offset, data_size, size_offset, array_size)

/* X-macro for generating asserts that entries fit in struct_field_info[] array.
 * The structure of macros here must match the structure above in PB_GEN_FIELD_INFO_x(),
 * but it is not easily reused because of how macro substitutions work. */
#define PB_GEN_FIELD_INFO_ASSERT_1(structname, atype, htype, ltype, fieldname, tag) \
    PB_FIELDINFO_ASSERT_1(tag, PB_ATYPE_ ## atype | PB_HTYPE_ ## htype | PB_LTYPE_MAP_ ## ltype, \
                   PB_DATA_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_DATA_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_SIZE_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_ARRAY_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname))

#define PB_GEN_FIELD_INFO_ASSERT_2(structname, atype, htype, ltype, fieldname, tag) \
    PB_FIELDINFO_ASSERT_2(tag, PB_ATYPE_ ## atype | PB_HTYPE_ ## htype | PB_LTYPE_MAP_ ## ltype, \
                   PB_DATA_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_DATA_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_SIZE_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_ARRAY_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname))

#define PB_GEN_FIELD_INFO_ASSERT_4(structname, atype, htype, ltype, fieldname, tag) \
    PB_FIELDINFO_ASSERT_4(tag, PB_ATYPE_ ## atype | PB_HTYPE_ ## htype | PB_LTYPE_MAP_ ## ltype, \
                   PB_DATA_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_DATA_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_SIZE_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_ARRAY_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname))

#define PB_GEN_FIELD_INFO_ASSERT_8(structname, atype, htype, ltype, fieldname, tag) \
    PB_FIELDINFO_ASSERT_8(tag, PB_ATYPE_ ## atype | PB_HTYPE_ ## htype | PB_LTYPE_MAP_ ## ltype, \
                   PB_DATA_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_DATA_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_SIZE_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_ARRAY_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname))

#define PB_GEN_FIELD_INFO_ASSERT_AUTO(structname, atype, htype, ltype, fieldname, tag) \
    PB_FIELDINFO_ASSERT_AUTO2(PB_FIELDINFO_WIDTH_AUTO(_PB_ATYPE_ ## atype, _PB_HTYPE_ ## htype, _PB_LTYPE_ ## ltype), \
                   tag, PB_ATYPE_ ## atype | PB_HTYPE_ ## htype | PB_LTYPE_MAP_ ## ltype, \
                   PB_DATA_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_DATA_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_SIZE_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_ARRAY_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname))

#define PB_FIELDINFO_ASSERT_AUTO2(width, tag, type, data_offset, data_size, size_offset, array_size) \
    PB_FIELDINFO_ASSERT_AUTO3(width, tag, type, data_offset, data_size, size_offset, array_size)

#define PB_FIELDINFO_ASSERT_AUTO3(width, tag, type, data_offset, data_size, size_offset, array_size) \
    PB_FIELDINFO_ASSERT_ ## width(tag, type, data_offset, data_size, size_offset, array_size)

#define PB_DATA_OFFSET_STATIC(htype, structname, fieldname) PB_DO ## htype(structname, fieldname)
#define PB_DATA_OFFSET_POINTER(htype, structname, fieldname) PB_DO ## htype(structname, fieldname)
#define PB_DATA_OFFSET_CALLBACK(htype, structname, fieldname) PB_DO ## htype(structname, fieldname)
#define PB_DO_PB_HTYPE_REQUIRED(structname, fieldname) offsetof(structname, fieldname)
#define PB_DO_PB_HTYPE_SINGULAR(structname, fieldname) offsetof(structname, fieldname)
#define PB_DO_PB_HTYPE_ONEOF(structname, fieldname) offsetof(structname, PB_ONEOF_NAME(FULL, fieldname))
#define PB_DO_PB_HTYPE_OPTIONAL(structname, fieldname) offsetof(structname, fieldname)
#define PB_DO_PB_HTYPE_REPEATED(structname, fieldname) offsetof(structname, fieldname)
#define PB_DO_PB_HTYPE_FIXARRAY(structname, fieldname) offsetof(structname, fieldname)

#define PB_SIZE_OFFSET_STATIC(htype, structname, fieldname) PB_SO ## htype(structname, fieldname)
#define PB_SIZE_OFFSET_POINTER(htype, structname, fieldname) PB_SO_PTR ## htype(structname, fieldname)
#define PB_SIZE_OFFSET_CALLBACK(htype, structname, fieldname) PB_SO_CB ## htype(structname, fieldname)
#define PB_SO_PB_HTYPE_REQUIRED(structname, fieldname) 0
#define PB_SO_PB_HTYPE_SINGULAR(structname, fieldname) 0
#define PB_SO_PB_HTYPE_ONEOF(structname, fieldname) PB_SO_PB_HTYPE_ONEOF2(structname, PB_ONEOF_NAME(FULL, fieldname), PB_ONEOF_NAME(UNION, fieldname))
#define PB_SO_PB_HTYPE_ONEOF2(structname, fullname, unionname) PB_SO_PB_HTYPE_ONEOF3(structname, fullname, unionname)
#define PB_SO_PB_HTYPE_ONEOF3(structname, fullname, unionname) pb_delta(structname, fullname, which_ ## unionname)
#define PB_SO_PB_HTYPE_OPTIONAL(structname, fieldname) pb_delta(structname, fieldname, has_ ## fieldname)
#define PB_SO_PB_HTYPE_REPEATED(structname, fieldname) pb_delta(structname, fieldname, fieldname ## _count)
#define PB_SO_PB_HTYPE_FIXARRAY(structname, fieldname) 0
#define PB_SO_PTR_PB_HTYPE_REQUIRED(structname, fieldname) 0
#define PB_SO_PTR_PB_HTYPE_SINGULAR(structname, fieldname) 0
#define PB_SO_PTR_PB_HTYPE_ONEOF(structname, fieldname) PB_SO_PB_HTYPE_ONEOF(structname, fieldname)
#define PB_SO_PTR_PB_HTYPE_OPTIONAL(structname, fieldname) 0
#define PB_SO_PTR_PB_HTYPE_REPEATED(structname, fieldname) PB_SO_PB_HTYPE_REPEATED(structname, fieldname)
#define PB_SO_PTR_PB_HTYPE_FIXARRAY(structname, fieldname) 0
#define PB_SO_CB_PB_HTYPE_REQUIRED(structname, fieldname) 0
#define PB_SO_CB_PB_HTYPE_SINGULAR(structname, fieldname) 0
#define PB_SO_CB_PB_HTYPE_ONEOF(structname, fieldname) PB_SO_PB_HTYPE_ONEOF(structname, fieldname)
#define PB_SO_CB_PB_HTYPE_OPTIONAL(structname, fieldname) 0
#define PB_SO_CB_PB_HTYPE_REPEATED(structname, fieldname) 0
#define PB_SO_CB_PB_HTYPE_FIXARRAY(structname, fieldname) 0

#define PB_ARRAY_SIZE_STATIC(htype, structname, fieldname) PB_AS ## htype(structname, fieldname)
#define PB_ARRAY_SIZE_POINTER(htype, structname, fieldname) PB_AS_PTR ## htype(structname, fieldname)
#define PB_ARRAY_SIZE_CALLBACK(htype, structname, fieldname) 1
#define PB_AS_PB_HTYPE_REQUIRED(structname, fieldname) 1
#define PB_AS_PB_HTYPE_SINGULAR(structname, fieldname) 1
#define PB_AS_PB_HTYPE_OPTIONAL(structname, fieldname) 1
#define PB_AS_PB_HTYPE_ONEOF(structname, fieldname) 1
#define PB_AS_PB_HTYPE_REPEATED(structname, fieldname) pb_arraysize(structname, fieldname)
#define PB_AS_PB_HTYPE_FIXARRAY(structname, fieldname) pb_arraysize(structname, fieldname)
#define PB_AS_PTR_PB_HTYPE_REQUIRED(structname, fieldname) 1
#define PB_AS_PTR_PB_HTYPE_SINGULAR(structname, fieldname) 1
#define PB_AS_PTR_PB_HTYPE_OPTIONAL(structname, fieldname) 1
#define PB_AS_PTR_PB_HTYPE_ONEOF(structname, fieldname) 1
#define PB_AS_PTR_PB_HTYPE_REPEATED(structname, fieldname) 1
#define PB_AS_PTR_PB_HTYPE_FIXARRAY(structname, fieldname) pb_arraysize(structname, fieldname[0])

#define PB_DATA_SIZE_STATIC(htype, structname, fieldname) PB_DS ## htype(structname, fieldname)
#define PB_DATA_SIZE_POINTER(htype, structname, fieldname) PB_DS_PTR ## htype(structname, fieldname)
#define PB_DATA_SIZE_CALLBACK(htype, structname, fieldname) PB_DS_CB ## htype(structname, fieldname)
#define PB_DS_PB_HTYPE_REQUIRED(structname, fieldname) pb_membersize(structname, fieldname)
#define PB_DS_PB_HTYPE_SINGULAR(structname, fieldname) pb_membersize(structname, fieldname)
#define PB_DS_PB_HTYPE_OPTIONAL(structname, fieldname) pb_membersize(structname, fieldname)
#define PB_DS_PB_HTYPE_ONEOF(structname, fieldname) pb_membersize(structname, PB_ONEOF_NAME(FULL, fieldname))
#define PB_DS_PB_HTYPE_REPEATED(structname, fieldname) pb_membersize(structname, fieldname[0])
#define PB_DS_PB_HTYPE_FIXARRAY(structname, fieldname) pb_membersize(structname, fieldname[0])
#define PB_DS_PTR_PB_HTYPE_REQUIRED(structname, fieldname) pb_membersize(structname, fieldname[0])
#define PB_DS_PTR_PB_HTYPE_SINGULAR(structname, fieldname) pb_membersize(structname, fieldname[0])
#define PB_DS_PTR_PB_HTYPE_OPTIONAL(structname, fieldname) pb_membersize(structname, fieldname[0])
#define PB_DS_PTR_PB_HTYPE_ONEOF(structname, fieldname) pb_membersize(structname, PB_ONEOF_NAME(FULL, fieldname)[0])
#define PB_DS_PTR_PB_HTYPE_REPEATED(structname, fieldname) pb_membersize(structname, fieldname[0])
#define PB_DS_PTR_PB_HTYPE_FIXARRAY(structname, fieldname) pb_membersize(structname, fieldname[0][0])
#define PB_DS_CB_PB_HTYPE_REQUIRED(structname, fieldname) pb_membersize(structname, fieldname)
#define PB_DS_CB_PB_HTYPE_SINGULAR(structname, fieldname) pb_membersize(structname, fieldname)
#define PB_DS_CB_PB_HTYPE_OPTIONAL(structname, fieldname) pb_membersize(structname, fieldname)
#define PB_DS_CB_PB_HTYPE_ONEOF(structname, fieldname) pb_membersize(structname, PB_ONEOF_NAME(FULL, fieldname))
#define PB_DS_CB_PB_HTYPE_REPEATED(structname, fieldname) pb_membersize(structname, fieldname)
#define PB_DS_CB_PB_HTYPE_FIXARRAY(structname, fieldname) pb_membersize(structname, fieldname)

#define PB_ONEOF_NAME(type, tuple) PB_EXPAND(PB_ONEOF_NAME_ ## type tuple)
#define PB_ONEOF_NAME_UNION(unionname,membername,fullname) unionname
#define PB_ONEOF_NAME_MEMBER(unionname,membername,fullname) membername
#define PB_ONEOF_NAME_FULL(unionname,membername,fullname) fullname

#define PB_GEN_SUBMSG_INFO(structname, atype, htype, ltype, fieldname, tag) \
    PB_SUBMSG_INFO_ ## htype(_PB_LTYPE_ ## ltype, structname, fieldname)

#define PB_SUBMSG_INFO_REQUIRED(ltype, structname, fieldname) PB_SI ## ltype(structname ## _ ## fieldname ## _MSGTYPE)
#define PB_SUBMSG_INFO_SINGULAR(ltype, structname, fieldname) PB_SI ## ltype(structname ## _ ## fieldname ## _MSGTYPE)
#define PB_SUBMSG_INFO_OPTIONAL(ltype, structname, fieldname) PB_SI ## ltype(structname ## _ ## fieldname ## _MSGTYPE)
#define PB_SUBMSG_INFO_ONEOF(ltype, structname, fieldname) PB_SUBMSG_INFO_ONEOF2(ltype, structname, PB_ONEOF_NAME(UNION, fieldname), PB_ONEOF_NAME(MEMBER, fieldname))
#define PB_SUBMSG_INFO_ONEOF2(ltype, structname, unionname, membername) PB_SUBMSG_INFO_ONEOF3(ltype, structname, unionname, membername)
#define PB_SUBMSG_INFO_ONEOF3(ltype, structname, unionname, membername) PB_SI ## ltype(structname ## _ ## unionname ## _ ## membername ## _MSGTYPE)
#define PB_SUBMSG_INFO_REPEATED(ltype, structname, fieldname) PB_SI ## ltype(structname ## _ ## fieldname ## _MSGTYPE)
#define PB_SUBMSG_INFO_FIXARRAY(ltype, structname, fieldname) PB_SI ## ltype(structname ## _ ## fieldname ## _MSGTYPE)
#define PB_SI_PB_LTYPE_BOOL(t)
#define PB_SI_PB_LTYPE_BYTES(t)
#define PB_SI_PB_LTYPE_DOUBLE(t)
#define PB_SI_PB_LTYPE_ENUM(t)
#define PB_SI_PB_LTYPE_UENUM(t)
#define PB_SI_PB_LTYPE_FIXED32(t)
#define PB_SI_PB_LTYPE_FIXED64(t)
#define PB_SI_PB_LTYPE_FLOAT(t)
#define PB_SI_PB_LTYPE_INT32(t)
#define PB_SI_PB_LTYPE_INT64(t)
#define PB_SI_PB_LTYPE_MESSAGE(t)  PB_SUBMSG_DESCRIPTOR(t)
#define PB_SI_PB_LTYPE_MSG_W_CB(t) PB_SUBMSG_DESCRIPTOR(t)
#define PB_SI_PB_LTYPE_SFIXED32(t)
#define PB_SI_PB_LTYPE_SFIXED64(t)
#define PB_SI_PB_LTYPE_SINT32(t)
#define PB_SI_PB_LTYPE_SINT64(t)
#define PB_SI_PB_LTYPE_STRING(t)
#define PB_SI_PB_LTYPE_UINT32(t)
#define PB_SI_PB_LTYPE_UINT64(t)
#define PB_SI_PB_LTYPE_EXTENSION(t)
#define PB_SI_PB_LTYPE_FIXED_LENGTH_BYTES(t)
#define PB_SUBMSG_DESCRIPTOR(t)    &(t ## _msg),

/* The field descriptors use a variable width format, with width of either
 * 1, 2, 4 or 8 of 32-bit words. The two lowest bytes of the first byte always
 * encode the descriptor size, 6 lowest bits of field tag number, and 8 bits
 * of the field type.
 *
 * Descriptor size is encoded as 0 = 1 word, 1 = 2 words, 2 = 4 words, 3 = 8 words.
 *
 * Formats, listed starting with the least significant bit of the first word.
 * 1 word:  [2-bit len] [6-bit tag] [8-bit type] [8-bit data_offset] [4-bit size_offset] [4-bit data_size]
 *
 * 2 words: [2-bit len] [6-bit tag] [8-bit type] [12-bit array_size] [4-bit size_offset]
 *          [16-bit data_offset] [12-bit data_size] [4-bit tag>>6]
 *
 * 4 words: [2-bit len] [6-bit tag] [8-bit type] [16-bit array_size]
 *          [8-bit size_offset] [24-bit tag>>6]
 *          [32-bit data_offset]
 *          [32-bit data_size]
 *
 * 8 words: [2-bit len] [6-bit tag] [8-bit type] [16-bit reserved]
 *          [8-bit size_offset] [24-bit tag>>6]
 *          [32-bit data_offset]
 *          [32-bit data_size]
 *          [32-bit array_size]
 *          [32-bit reserved]
 *          [32-bit reserved]
 *          [32-bit reserved]
 */

#define PB_FIELDINFO_1(tag, type, data_offset, data_size, size_offset, array_size) \
    (0 | (((tag) << 2) & 0xFF) | ((type) << 8) | (((uint32_t)(data_offset) & 0xFF) << 16) | \
     (((uint32_t)(size_offset) & 0x0F) << 24) | (((uint32_t)(data_size) & 0x0F) << 28)),

#define PB_FIELDINFO_2(tag, type, data_offset, data_size, size_offset, array_size) \
    (1 | (((tag) << 2) & 0xFF) | ((type) << 8) | (((uint32_t)(array_size) & 0xFFF) << 16) | (((uint32_t)(size_offset) & 0x0F) << 28)), \
    (((uint32_t)(data_offset) & 0xFFFF) | (((uint32_t)(data_size) & 0xFFF) << 16) | (((uint32_t)(tag) & 0x3c0) << 22)),

#define PB_FIELDINFO_4(tag, type, data_offset, data_size, size_offset, array_size) \
    (2 | (((tag) << 2) & 0xFF) | ((type) << 8) | (((uint32_t)(array_size) & 0xFFFF) << 16)), \
    ((uint32_t)(int_least8_t)(size_offset) | (((uint32_t)(tag) << 2) & 0xFFFFFF00)), \
    (data_offset), (data_size),

#define PB_FIELDINFO_8(tag, type, data_offset, data_size, size_offset, array_size) \
    (3 | (((tag) << 2) & 0xFF) | ((type) << 8)), \
    ((uint32_t)(int_least8_t)(size_offset) | (((uint32_t)(tag) << 2) & 0xFFFFFF00)), \
    (data_offset), (data_size), (array_size), 0, 0, 0,

/* These assertions verify that the field information fits in the allocated space.
 * The generator tries to automatically determine the correct width that can fit all
 * data associated with a message. These asserts will fail only if there has been a
 * problem in the automatic logic - this may be worth reporting as a bug. As a workaround,
 * you can increase the descriptor width by defining PB_FIELDINFO_WIDTH or by setting
 * descriptorsize option in .options file.
 */
#define PB_FITS(value,bits) ((uint32_t)(value) < ((uint32_t)1<<bits))
#define PB_FIELDINFO_ASSERT_1(tag, type, data_offset, data_size, size_offset, array_size) \
    PB_STATIC_ASSERT(PB_FITS(tag,6) && PB_FITS(data_offset,8) && PB_FITS(size_offset,4) && PB_FITS(data_size,4) && PB_FITS(array_size,1), FIELDINFO_DOES_NOT_FIT_width1_field ## tag)

#define PB_FIELDINFO_ASSERT_2(tag, type, data_offset, data_size, size_offset, array_size) \
    PB_STATIC_ASSERT(PB_FITS(tag,10) && PB_FITS(data_offset,16) && PB_FITS(size_offset,4) && PB_FITS(data_size,12) && PB_FITS(array_size,12), FIELDINFO_DOES_NOT_FIT_width2_field ## tag)

#ifndef PB_FIELD_32BIT
/* Maximum field sizes are still 16-bit if pb_size_t is 16-bit */
#define PB_FIELDINFO_ASSERT_4(tag, type, data_offset, data_size, size_offset, array_size) \
    PB_STATIC_ASSERT(PB_FITS(tag,16) && PB_FITS(data_offset,16) && PB_FITS((int_least8_t)size_offset,8) && PB_FITS(data_size,16) && PB_FITS(array_size,16), FIELDINFO_DOES_NOT_FIT_width4_field ## tag)

#define PB_FIELDINFO_ASSERT_8(tag, type, data_offset, data_size, size_offset, array_size) \
    PB_STATIC_ASSERT(PB_FITS(tag,16) && PB_FITS(data_offset,16) && PB_FITS((int_least8_t)size_offset,8) && PB_FITS(data_size,16) && PB_FITS(array_size,16), FIELDINFO_DOES_NOT_FIT_width8_field ## tag)
#else
/* Up to 32-bit fields supported.
 * Note that the checks are against 31 bits to avoid compiler warnings about shift wider than type in the test.
 * I expect that there is no reasonable use for >2GB messages with nanopb anyway.
 */
#define PB_FIELDINFO_ASSERT_4(tag, type, data_offset, data_size, size_offset, array_size) \
    PB_STATIC_ASSERT(PB_FITS(tag,30) && PB_FITS(data_offset,31) && PB_FITS(size_offset,8) && PB_FITS(data_size,31) && PB_FITS(array_size,16), FIELDINFO_DOES_NOT_FIT_width4_field ## tag)

#define PB_FIELDINFO_ASSERT_8(tag, type, data_offset, data_size, size_offset, array_size) \
    PB_STATIC_ASSERT(PB_FITS(tag,30) && PB_FITS(data_offset,31) && PB_FITS(size_offset,8) && PB_FITS(data_size,31) && PB_FITS(array_size,31), FIELDINFO_DOES_NOT_FIT_width8_field ## tag)
#endif


/* Automatic picking of FIELDINFO width:
 * Uses width 1 when possible, otherwise resorts to width 2.
 * This is used when PB_BIND() is called with "AUTO" as the argument.
 * The generator will give explicit size argument when it knows that a message
 * structure grows beyond 1-word format limits.
 */
#define PB_FIELDINFO_WIDTH_AUTO(atype, htype, ltype) PB_FI_WIDTH ## atype(htype, ltype)
#define PB_FI_WIDTH_PB_ATYPE_STATIC(htype, ltype) PB_FI_WIDTH ## htype(ltype)
#define PB_FI_WIDTH_PB_ATYPE_POINTER(htype, ltype) PB_FI_WIDTH ## htype(ltype)
#define PB_FI_WIDTH_PB_ATYPE_CALLBACK(htype, ltype) 2
#define PB_FI_WIDTH_PB_HTYPE_REQUIRED(ltype) PB_FI_WIDTH ## ltype
#define PB_FI_WIDTH_PB_HTYPE_SINGULAR(ltype) PB_FI_WIDTH ## ltype
#define PB_FI_WIDTH_PB_HTYPE_OPTIONAL(ltype) PB_FI_WIDTH ## ltype
#define PB_FI_WIDTH_PB_HTYPE_ONEOF(ltype) PB_FI_WIDTH ## ltype
#define PB_FI_WIDTH_PB_HTYPE_REPEATED(ltype) 2
#define PB_FI_WIDTH_PB_HTYPE_FIXARRAY(ltype) 2
#define PB_FI_WIDTH_PB_LTYPE_BOOL      1
#define PB_FI_WIDTH_PB_LTYPE_BYTES     2
#define PB_FI_WIDTH_PB_LTYPE_DOUBLE    1
#define PB_FI_WIDTH_PB_LTYPE_ENUM      1
#define PB_FI_WIDTH_PB_LTYPE_UENUM     1
#define PB_FI_WIDTH_PB_LTYPE_FIXED32   1
#define PB_FI_WIDTH_PB_LTYPE_FIXED64   1
#define PB_FI_WIDTH_PB_LTYPE_FLOAT     1
#define PB_FI_WIDTH_PB_LTYPE_INT32     1
#define PB_FI_WIDTH_PB_LTYPE_INT64     1
#define PB_FI_WIDTH_PB_LTYPE_MESSAGE   2
#define PB_FI_WIDTH_PB_LTYPE_MSG_W_CB  2
#define PB_FI_WIDTH_PB_LTYPE_SFIXED32  1
#define PB_FI_WIDTH_PB_LTYPE_SFIXED64  1
#define PB_FI_WIDTH_PB_LTYPE_SINT32    1
#define PB_FI_WIDTH_PB_LTYPE_SINT64    1
#define PB_FI_WIDTH_PB_LTYPE_STRING    2
#define PB_FI_WIDTH_PB_LTYPE_UINT32    1
#define PB_FI_WIDTH_PB_LTYPE_UINT64    1
#define PB_FI_WIDTH_PB_LTYPE_EXTENSION 1
#define PB_FI_WIDTH_PB_LTYPE_FIXED_LENGTH_BYTES 2

/* The mapping from protobuf types to LTYPEs is done using these macros. */
#define PB_LTYPE_MAP_BOOL               PB_LTYPE_BOOL
#define PB_LTYPE_MAP_BYTES              PB_LTYPE_BYTES
#define PB_LTYPE_MAP_DOUBLE             PB_LTYPE_FIXED64
#define PB_LTYPE_MAP_ENUM               PB_LTYPE_VARINT
#define PB_LTYPE_MAP_UENUM              PB_LTYPE_UVARINT
#define PB_LTYPE_MAP_FIXED32            PB_LTYPE_FIXED32
#define PB_LTYPE_MAP_FIXED64            PB_LTYPE_FIXED64
#define PB_LTYPE_MAP_FLOAT              PB_LTYPE_FIXED32
#define PB_LTYPE_MAP_INT32              PB_LTYPE_VARINT
#define PB_LTYPE_MAP_INT64              PB_LTYPE_VARINT
#define PB_LTYPE_MAP_MESSAGE            PB_LTYPE_SUBMESSAGE
#define PB_LTYPE_MAP_MSG_W_CB           PB_LTYPE_SUBMSG_W_CB
#define PB_LTYPE_MAP_SFIXED32           PB_LTYPE_FIXED32
#define PB_LTYPE_MAP_SFIXED64           PB_LTYPE_FIXED64
#define PB_LTYPE_MAP_SINT32             PB_LTYPE_SVARINT
#define PB_LTYPE_MAP_SINT64             PB_LTYPE_SVARINT
#define PB_LTYPE_MAP_STRING             PB_LTYPE_STRING
#define PB_LTYPE_MAP_UINT32             PB_LTYPE_UVARINT
#define PB_LTYPE_MAP_UINT64             PB_LTYPE_UVARINT
#define PB_LTYPE_MAP_EXTENSION          PB_LTYPE_EXTENSION
#define PB_LTYPE_MAP_FIXED_LENGTH_BYTES PB_LTYPE_FIXED_LENGTH_BYTES

/* These macros are used for giving out error messages.
 * They are mostly a debugging aid; the main error information
 * is the true/false return value from functions.
 * Some code space can be saved by disabling the error
 * messages if not used.
 *
 * PB_SET_ERROR() sets the error message if none has been set yet.
 *                msg must be a constant string literal.
 * PB_GET_ERROR() always returns a pointer to a string.
 * PB_RETURN_ERROR() sets the error and returns false from current
 *                   function.
 */
#ifdef PB_NO_ERRMSG
#define PB_SET_ERROR(stream, msg) PB_UNUSED(stream)
#define PB_GET_ERROR(stream) "(errmsg disabled)"
#else
#define PB_SET_ERROR(stream, msg) (stream->errmsg = (stream)->errmsg ? (stream)->errmsg : (msg))
#define PB_GET_ERROR(stream) ((stream)->errmsg ? (stream)->errmsg : "(none)")
#endif

#define PB_RETURN_ERROR(stream, msg) return PB_SET_ERROR(stream, msg), false

#ifdef __cplusplus
} /* extern "C" */
#endif

#ifdef __cplusplus
#if __cplusplus >= 201103L
#define PB_CONSTEXPR constexpr
#else  // __cplusplus >= 201103L
#define PB_CONSTEXPR
#endif  // __cplusplus >= 201103L

#if __cplusplus >= 201703L
#define PB_INLINE_CONSTEXPR inline constexpr
#else  // __cplusplus >= 201703L
#define PB_INLINE_CONSTEXPR PB_CONSTEXPR
#endif  // __cplusplus >= 201703L

namespace nanopb {
// Each type will be partially specialized by the generator.
template <typename GenMessageT> struct MessageDescriptor;
}  // namespace nanopb
#endif  /* __cplusplus */

#endif

/* pb_common.h: Common support functions for pb_encode.c and pb_decode.c.
 * These functions are rarely needed by applications directly.
 */

#ifndef PB_COMMON_H_INCLUDED
#define PB_COMMON_H_INCLUDED


#ifdef __cplusplus
extern "C" {
#endif

/* Initialize the field iterator structure to beginning.
 * Returns false if the message type is empty. */
bool pb_field_iter_begin(pb_field_iter_t *iter, const pb_msgdesc_t *desc, void *message);

/* Get a field iterator for extension field. */
bool pb_field_iter_begin_extension(pb_field_iter_t *iter, pb_extension_t *extension);

/* Same as pb_field_iter_begin(), but for const message pointer.
 * Note that the pointers in pb_field_iter_t will be non-const but shouldn't
 * be written to when using these functions. */
bool pb_field_iter_begin_const(pb_field_iter_t *iter, const pb_msgdesc_t *desc, const void *message);
bool pb_field_iter_begin_extension_const(pb_field_iter_t *iter, const pb_extension_t *extension);

/* Advance the iterator to the next field.
 * Returns false when the iterator wraps back to the first field. */
bool pb_field_iter_next(pb_field_iter_t *iter);

/* Advance the iterator until it points at a field with the given tag.
 * Returns false if no such field exists. */
bool pb_field_iter_find(pb_field_iter_t *iter, uint32_t tag);

/* Find a field with type PB_LTYPE_EXTENSION, or return false if not found.
 * There can be only one extension range field per message. */
bool pb_field_iter_find_extension(pb_field_iter_t *iter);

#ifdef PB_VALIDATE_UTF8
/* Validate UTF-8 text string */
bool pb_validate_utf8(const char *s);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

/* pb_encode.h: Functions to encode protocol buffers. Depends on pb_encode.c.
 * The main function is pb_encode. You also need an output stream, and the
 * field descriptions created by nanopb_generator.py.
 */

#ifndef PB_ENCODE_H_INCLUDED
#define PB_ENCODE_H_INCLUDED


#ifdef __cplusplus
extern "C" {
#endif

/* Structure for defining custom output streams. You will need to provide
 * a callback function to write the bytes to your storage, which can be
 * for example a file or a network socket.
 *
 * The callback must conform to these rules:
 *
 * 1) Return false on IO errors. This will cause encoding to abort.
 * 2) You can use state to store your own data (e.g. buffer pointer).
 * 3) pb_write will update bytes_written after your callback runs.
 * 4) Substreams will modify max_size and bytes_written. Don't use them
 *    to calculate any pointers.
 */
struct pb_ostream_s
{
#ifdef PB_BUFFER_ONLY
    /* Callback pointer is not used in buffer-only configuration.
     * Having an int pointer here allows binary compatibility but
     * gives an error if someone tries to assign callback function.
     * Also, NULL pointer marks a 'sizing stream' that does not
     * write anything.
     */
    int *callback;
#else
    bool (*callback)(pb_ostream_t *stream, const pb_byte_t *buf, size_t count);
#endif
    void *state;          /* Free field for use by callback implementation. */
    size_t max_size;      /* Limit number of output bytes written (or use SIZE_MAX). */
    size_t bytes_written; /* Number of bytes written so far. */
    
#ifndef PB_NO_ERRMSG
    const char *errmsg;
#endif
};

/***************************
 * Main encoding functions *
 ***************************/

/* Encode a single protocol buffers message from C structure into a stream.
 * Returns true on success, false on any failure.
 * The actual struct pointed to by src_struct must match the description in fields.
 * All required fields in the struct are assumed to have been filled in.
 *
 * Example usage:
 *    MyMessage msg = {};
 *    uint8_t buffer[64];
 *    pb_ostream_t stream;
 *
 *    msg.field1 = 42;
 *    stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
 *    pb_encode(&stream, MyMessage_fields, &msg);
 */
bool pb_encode(pb_ostream_t *stream, const pb_msgdesc_t *fields, const void *src_struct);

/* Extended version of pb_encode, with several options to control the
 * encoding process:
 *
 * PB_ENCODE_DELIMITED:      Prepend the length of message as a varint.
 *                           Corresponds to writeDelimitedTo() in Google's
 *                           protobuf API.
 *
 * PB_ENCODE_NULLTERMINATED: Append a null byte to the message for termination.
 *                           NOTE: This behaviour is not supported in most other
 *                           protobuf implementations, so PB_ENCODE_DELIMITED
 *                           is a better option for compatibility.
 */
#define PB_ENCODE_DELIMITED       0x02U
#define PB_ENCODE_NULLTERMINATED  0x04U
bool pb_encode_ex(pb_ostream_t *stream, const pb_msgdesc_t *fields, const void *src_struct, unsigned int flags);

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define pb_encode_delimited(s,f,d) pb_encode_ex(s,f,d, PB_ENCODE_DELIMITED)
#define pb_encode_nullterminated(s,f,d) pb_encode_ex(s,f,d, PB_ENCODE_NULLTERMINATED)

/* Encode the message to get the size of the encoded data, but do not store
 * the data. */
bool pb_get_encoded_size(size_t *size, const pb_msgdesc_t *fields, const void *src_struct);

/**************************************
 * Functions for manipulating streams *
 **************************************/

/* Create an output stream for writing into a memory buffer.
 * The number of bytes written can be found in stream.bytes_written after
 * encoding the message.
 *
 * Alternatively, you can use a custom stream that writes directly to e.g.
 * a file or a network socket.
 */
pb_ostream_t pb_ostream_from_buffer(pb_byte_t *buf, size_t bufsize);

/* Pseudo-stream for measuring the size of a message without actually storing
 * the encoded data.
 * 
 * Example usage:
 *    MyMessage msg = {};
 *    pb_ostream_t stream = PB_OSTREAM_SIZING;
 *    pb_encode(&stream, MyMessage_fields, &msg);
 *    printf("Message size is %d\n", stream.bytes_written);
 */
#ifndef PB_NO_ERRMSG
#define PB_OSTREAM_SIZING {0,0,0,0,0}
#else
#define PB_OSTREAM_SIZING {0,0,0,0}
#endif

/* Function to write into a pb_ostream_t stream. You can use this if you need
 * to append or prepend some custom headers to the message.
 */
bool pb_write(pb_ostream_t *stream, const pb_byte_t *buf, size_t count);


/************************************************
 * Helper functions for writing field callbacks *
 ************************************************/

/* Encode field header based on type and field number defined in the field
 * structure. Call this from the callback before writing out field contents. */
bool pb_encode_tag_for_field(pb_ostream_t *stream, const pb_field_iter_t *field);

/* Encode field header by manually specifying wire type. You need to use this
 * if you want to write out packed arrays from a callback field. */
bool pb_encode_tag(pb_ostream_t *stream, pb_wire_type_t wiretype, uint32_t field_number);

/* Encode an integer in the varint format.
 * This works for bool, enum, int32, int64, uint32 and uint64 field types. */
#ifndef PB_WITHOUT_64BIT
bool pb_encode_varint(pb_ostream_t *stream, uint64_t value);
#else
bool pb_encode_varint(pb_ostream_t *stream, uint32_t value);
#endif

/* Encode an integer in the zig-zagged svarint format.
 * This works for sint32 and sint64. */
#ifndef PB_WITHOUT_64BIT
bool pb_encode_svarint(pb_ostream_t *stream, int64_t value);
#else
bool pb_encode_svarint(pb_ostream_t *stream, int32_t value);
#endif

/* Encode a string or bytes type field. For strings, pass strlen(s) as size. */
bool pb_encode_string(pb_ostream_t *stream, const pb_byte_t *buffer, size_t size);

/* Encode a fixed32, sfixed32 or float value.
 * You need to pass a pointer to a 4-byte wide C variable. */
bool pb_encode_fixed32(pb_ostream_t *stream, const void *value);

#ifndef PB_WITHOUT_64BIT
/* Encode a fixed64, sfixed64 or double value.
 * You need to pass a pointer to a 8-byte wide C variable. */
bool pb_encode_fixed64(pb_ostream_t *stream, const void *value);
#endif

#ifdef PB_CONVERT_DOUBLE_FLOAT
/* Encode a float value so that it appears like a double in the encoded
 * message. */
bool pb_encode_float_as_double(pb_ostream_t *stream, float value);
#endif

/* Encode a submessage field.
 * You need to pass the pb_field_t array and pointer to struct, just like
 * with pb_encode(). This internally encodes the submessage twice, first to
 * calculate message size and then to actually write it out.
 */
bool pb_encode_submessage(pb_ostream_t *stream, const pb_msgdesc_t *fields, const void *src_struct);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
/* pb_decode.h: Functions to decode protocol buffers. Depends on pb_decode.c.
 * The main function is pb_decode. You also need an input stream, and the
 * field descriptions created by nanopb_generator.py.
 */

#ifndef PB_DECODE_H_INCLUDED
#define PB_DECODE_H_INCLUDED


#ifdef __cplusplus
extern "C" {
#endif

/* Structure for defining custom input streams. You will need to provide
 * a callback function to read the bytes from your storage, which can be
 * for example a file or a network socket.
 * 
 * The callback must conform to these rules:
 *
 * 1) Return false on IO errors. This will cause decoding to abort.
 * 2) You can use state to store your own data (e.g. buffer pointer),
 *    and rely on pb_read to verify that no-body reads past bytes_left.
 * 3) Your callback may be used with substreams, in which case bytes_left
 *    is different than from the main stream. Don't use bytes_left to compute
 *    any pointers.
 */
struct pb_istream_s
{
#ifdef PB_BUFFER_ONLY
    /* Callback pointer is not used in buffer-only configuration.
     * Having an int pointer here allows binary compatibility but
     * gives an error if someone tries to assign callback function.
     */
    int *callback;
#else
    bool (*callback)(pb_istream_t *stream, pb_byte_t *buf, size_t count);
#endif

    void *state; /* Free field for use by callback implementation */
    size_t bytes_left;
    
#ifndef PB_NO_ERRMSG
    const char *errmsg;
#endif
};

#ifndef PB_NO_ERRMSG
#define PB_ISTREAM_EMPTY {0,0,0,0}
#else
#define PB_ISTREAM_EMPTY {0,0,0}
#endif

/***************************
 * Main decoding functions *
 ***************************/
 
/* Decode a single protocol buffers message from input stream into a C structure.
 * Returns true on success, false on any failure.
 * The actual struct pointed to by dest must match the description in fields.
 * Callback fields of the destination structure must be initialized by caller.
 * All other fields will be initialized by this function.
 *
 * Example usage:
 *    MyMessage msg = {};
 *    uint8_t buffer[64];
 *    pb_istream_t stream;
 *    
 *    // ... read some data into buffer ...
 *
 *    stream = pb_istream_from_buffer(buffer, count);
 *    pb_decode(&stream, MyMessage_fields, &msg);
 */
bool pb_decode(pb_istream_t *stream, const pb_msgdesc_t *fields, void *dest_struct);

/* Extended version of pb_decode, with several options to control
 * the decoding process:
 *
 * PB_DECODE_NOINIT:         Do not initialize the fields to default values.
 *                           This is slightly faster if you do not need the default
 *                           values and instead initialize the structure to 0 using
 *                           e.g. memset(). This can also be used for merging two
 *                           messages, i.e. combine already existing data with new
 *                           values.
 *
 * PB_DECODE_DELIMITED:      Input message starts with the message size as varint.
 *                           Corresponds to parseDelimitedFrom() in Google's
 *                           protobuf API.
 *
 * PB_DECODE_NULLTERMINATED: Stop reading when field tag is read as 0. This allows
 *                           reading null terminated messages.
 *                           NOTE: Until nanopb-0.4.0, pb_decode() also allows
 *                           null-termination. This behaviour is not supported in
 *                           most other protobuf implementations, so PB_DECODE_DELIMITED
 *                           is a better option for compatibility.
 *
 * Multiple flags can be combined with bitwise or (| operator)
 */
#define PB_DECODE_NOINIT          0x01U
#define PB_DECODE_DELIMITED       0x02U
#define PB_DECODE_NULLTERMINATED  0x04U
bool pb_decode_ex(pb_istream_t *stream, const pb_msgdesc_t *fields, void *dest_struct, unsigned int flags);

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define pb_decode_noinit(s,f,d) pb_decode_ex(s,f,d, PB_DECODE_NOINIT)
#define pb_decode_delimited(s,f,d) pb_decode_ex(s,f,d, PB_DECODE_DELIMITED)
#define pb_decode_delimited_noinit(s,f,d) pb_decode_ex(s,f,d, PB_DECODE_DELIMITED | PB_DECODE_NOINIT)
#define pb_decode_nullterminated(s,f,d) pb_decode_ex(s,f,d, PB_DECODE_NULLTERMINATED)

#ifdef PB_ENABLE_MALLOC
/* Release any allocated pointer fields. If you use dynamic allocation, you should
 * call this for any successfully decoded message when you are done with it. If
 * pb_decode() returns with an error, the message is already released.
 */
void pb_release(const pb_msgdesc_t *fields, void *dest_struct);
#else
/* Allocation is not supported, so release is no-op */
#define pb_release(fields, dest_struct) PB_UNUSED(fields); PB_UNUSED(dest_struct);
#endif


/**************************************
 * Functions for manipulating streams *
 **************************************/

/* Create an input stream for reading from a memory buffer.
 *
 * msglen should be the actual length of the message, not the full size of
 * allocated buffer.
 *
 * Alternatively, you can use a custom stream that reads directly from e.g.
 * a file or a network socket.
 */
pb_istream_t pb_istream_from_buffer(const pb_byte_t *buf, size_t msglen);

/* Function to read from a pb_istream_t. You can use this if you need to
 * read some custom header data, or to read data in field callbacks.
 */
bool pb_read(pb_istream_t *stream, pb_byte_t *buf, size_t count);


/************************************************
 * Helper functions for writing field callbacks *
 ************************************************/

/* Decode the tag for the next field in the stream. Gives the wire type and
 * field tag. At end of the message, returns false and sets eof to true. */
bool pb_decode_tag(pb_istream_t *stream, pb_wire_type_t *wire_type, uint32_t *tag, bool *eof);

/* Skip the field payload data, given the wire type. */
bool pb_skip_field(pb_istream_t *stream, pb_wire_type_t wire_type);

/* Decode an integer in the varint format. This works for enum, int32,
 * int64, uint32 and uint64 field types. */
#ifndef PB_WITHOUT_64BIT
bool pb_decode_varint(pb_istream_t *stream, uint64_t *dest);
#else
#define pb_decode_varint pb_decode_varint32
#endif

/* Decode an integer in the varint format. This works for enum, int32,
 * and uint32 field types. */
bool pb_decode_varint32(pb_istream_t *stream, uint32_t *dest);

/* Decode a bool value in varint format. */
bool pb_decode_bool(pb_istream_t *stream, bool *dest);

/* Decode an integer in the zig-zagged svarint format. This works for sint32
 * and sint64. */
#ifndef PB_WITHOUT_64BIT
bool pb_decode_svarint(pb_istream_t *stream, int64_t *dest);
#else
bool pb_decode_svarint(pb_istream_t *stream, int32_t *dest);
#endif

/* Decode a fixed32, sfixed32 or float value. You need to pass a pointer to
 * a 4-byte wide C variable. */
bool pb_decode_fixed32(pb_istream_t *stream, void *dest);

#ifndef PB_WITHOUT_64BIT
/* Decode a fixed64, sfixed64 or double value. You need to pass a pointer to
 * a 8-byte wide C variable. */
bool pb_decode_fixed64(pb_istream_t *stream, void *dest);
#endif

#ifdef PB_CONVERT_DOUBLE_FLOAT
/* Decode a double value into float variable. */
bool pb_decode_double_as_float(pb_istream_t *stream, float *dest);
#endif

/* Make a limited-length substream for reading a PB_WT_STRING field. */
bool pb_make_string_substream(pb_istream_t *stream, pb_istream_t *substream);
bool pb_close_string_substream(pb_istream_t *stream, pb_istream_t *substream);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
/* pb_common.c: Common support functions for pb_encode.c and pb_decode.c.
 *
 * 2014 Petteri Aimonen <jpa@kapsi.fi>
 */


static bool load_descriptor_values(pb_field_iter_t *iter)
{
    uint32_t word0;
    uint32_t data_offset;
    int_least8_t size_offset;

    if (iter->index >= iter->descriptor->field_count)
        return false;

    word0 = PB_PROGMEM_READU32(iter->descriptor->field_info[iter->field_info_index]);
    iter->type = (pb_type_t)((word0 >> 8) & 0xFF);

    switch(word0 & 3)
    {
        case 0: {
            /* 1-word format */
            iter->array_size = 1;
            iter->tag = (pb_size_t)((word0 >> 2) & 0x3F);
            size_offset = (int_least8_t)((word0 >> 24) & 0x0F);
            data_offset = (word0 >> 16) & 0xFF;
            iter->data_size = (pb_size_t)((word0 >> 28) & 0x0F);
            break;
        }

        case 1: {
            /* 2-word format */
            uint32_t word1 = PB_PROGMEM_READU32(iter->descriptor->field_info[iter->field_info_index + 1]);

            iter->array_size = (pb_size_t)((word0 >> 16) & 0x0FFF);
            iter->tag = (pb_size_t)(((word0 >> 2) & 0x3F) | ((word1 >> 28) << 6));
            size_offset = (int_least8_t)((word0 >> 28) & 0x0F);
            data_offset = word1 & 0xFFFF;
            iter->data_size = (pb_size_t)((word1 >> 16) & 0x0FFF);
            break;
        }

        case 2: {
            /* 4-word format */
            uint32_t word1 = PB_PROGMEM_READU32(iter->descriptor->field_info[iter->field_info_index + 1]);
            uint32_t word2 = PB_PROGMEM_READU32(iter->descriptor->field_info[iter->field_info_index + 2]);
            uint32_t word3 = PB_PROGMEM_READU32(iter->descriptor->field_info[iter->field_info_index + 3]);

            iter->array_size = (pb_size_t)(word0 >> 16);
            iter->tag = (pb_size_t)(((word0 >> 2) & 0x3F) | ((word1 >> 8) << 6));
            size_offset = (int_least8_t)(word1 & 0xFF);
            data_offset = word2;
            iter->data_size = (pb_size_t)word3;
            break;
        }

        default: {
            /* 8-word format */
            uint32_t word1 = PB_PROGMEM_READU32(iter->descriptor->field_info[iter->field_info_index + 1]);
            uint32_t word2 = PB_PROGMEM_READU32(iter->descriptor->field_info[iter->field_info_index + 2]);
            uint32_t word3 = PB_PROGMEM_READU32(iter->descriptor->field_info[iter->field_info_index + 3]);
            uint32_t word4 = PB_PROGMEM_READU32(iter->descriptor->field_info[iter->field_info_index + 4]);

            iter->array_size = (pb_size_t)word4;
            iter->tag = (pb_size_t)(((word0 >> 2) & 0x3F) | ((word1 >> 8) << 6));
            size_offset = (int_least8_t)(word1 & 0xFF);
            data_offset = word2;
            iter->data_size = (pb_size_t)word3;
            break;
        }
    }

    if (!iter->message)
    {
        /* Avoid doing arithmetic on null pointers, it is undefined */
        iter->pField = NULL;
        iter->pSize = NULL;
    }
    else
    {
        iter->pField = (char*)iter->message + data_offset;

        if (size_offset)
        {
            iter->pSize = (char*)iter->pField - size_offset;
        }
        else if (PB_HTYPE(iter->type) == PB_HTYPE_REPEATED &&
                 (PB_ATYPE(iter->type) == PB_ATYPE_STATIC ||
                  PB_ATYPE(iter->type) == PB_ATYPE_POINTER))
        {
            /* Fixed count array */
            iter->pSize = &iter->array_size;
        }
        else
        {
            iter->pSize = NULL;
        }

        if (PB_ATYPE(iter->type) == PB_ATYPE_POINTER && iter->pField != NULL)
        {
            iter->pData = *(void**)iter->pField;
        }
        else
        {
            iter->pData = iter->pField;
        }
    }

    if (PB_LTYPE_IS_SUBMSG(iter->type))
    {
        iter->submsg_desc = iter->descriptor->submsg_info[iter->submessage_index];
    }
    else
    {
        iter->submsg_desc = NULL;
    }

    return true;
}

static void advance_iterator(pb_field_iter_t *iter)
{
    iter->index++;

    if (iter->index >= iter->descriptor->field_count)
    {
        /* Restart */
        iter->index = 0;
        iter->field_info_index = 0;
        iter->submessage_index = 0;
        iter->required_field_index = 0;
    }
    else
    {
        /* Increment indexes based on previous field type.
         * All field info formats have the following fields:
         * - lowest 2 bits tell the amount of words in the descriptor (2^n words)
         * - bits 2..7 give the lowest bits of tag number.
         * - bits 8..15 give the field type.
         */
        uint32_t prev_descriptor = PB_PROGMEM_READU32(iter->descriptor->field_info[iter->field_info_index]);
        pb_type_t prev_type = (prev_descriptor >> 8) & 0xFF;
        pb_size_t descriptor_len = (pb_size_t)(1 << (prev_descriptor & 3));

        /* Add to fields.
         * The cast to pb_size_t is needed to avoid -Wconversion warning.
         * Because the data is is constants from generator, there is no danger of overflow.
         */
        iter->field_info_index = (pb_size_t)(iter->field_info_index + descriptor_len);
        iter->required_field_index = (pb_size_t)(iter->required_field_index + (PB_HTYPE(prev_type) == PB_HTYPE_REQUIRED));
        iter->submessage_index = (pb_size_t)(iter->submessage_index + PB_LTYPE_IS_SUBMSG(prev_type));
    }
}

bool pb_field_iter_begin(pb_field_iter_t *iter, const pb_msgdesc_t *desc, void *message)
{
    memset(iter, 0, sizeof(*iter));

    iter->descriptor = desc;
    iter->message = message;

    return load_descriptor_values(iter);
}

bool pb_field_iter_begin_extension(pb_field_iter_t *iter, pb_extension_t *extension)
{
    const pb_msgdesc_t *msg = (const pb_msgdesc_t*)extension->type->arg;
    bool status;

    uint32_t word0 = PB_PROGMEM_READU32(msg->field_info[0]);
    if (PB_ATYPE(word0 >> 8) == PB_ATYPE_POINTER)
    {
        /* For pointer extensions, the pointer is stored directly
         * in the extension structure. This avoids having an extra
         * indirection. */
        status = pb_field_iter_begin(iter, msg, &extension->dest);
    }
    else
    {
        status = pb_field_iter_begin(iter, msg, extension->dest);
    }

    iter->pSize = &extension->found;
    return status;
}

bool pb_field_iter_next(pb_field_iter_t *iter)
{
    advance_iterator(iter);
    (void)load_descriptor_values(iter);
    return iter->index != 0;
}

bool pb_field_iter_find(pb_field_iter_t *iter, uint32_t tag)
{
    if (iter->tag == tag)
    {
        return true; /* Nothing to do, correct field already. */
    }
    else if (tag > iter->descriptor->largest_tag)
    {
        return false;
    }
    else
    {
        pb_size_t start = iter->index;
        uint32_t fieldinfo;

        if (tag < iter->tag)
        {
            /* Fields are in tag number order, so we know that tag is between
             * 0 and our start position. Setting index to end forces
             * advance_iterator() call below to restart from beginning. */
            iter->index = iter->descriptor->field_count;
        }

        do
        {
            /* Advance iterator but don't load values yet */
            advance_iterator(iter);

            /* Do fast check for tag number match */
            fieldinfo = PB_PROGMEM_READU32(iter->descriptor->field_info[iter->field_info_index]);

            if (((fieldinfo >> 2) & 0x3F) == (tag & 0x3F))
            {
                /* Good candidate, check further */
                (void)load_descriptor_values(iter);

                if (iter->tag == tag &&
                    PB_LTYPE(iter->type) != PB_LTYPE_EXTENSION)
                {
                    /* Found it */
                    return true;
                }
            }
        } while (iter->index != start);

        /* Searched all the way back to start, and found nothing. */
        (void)load_descriptor_values(iter);
        return false;
    }
}

bool pb_field_iter_find_extension(pb_field_iter_t *iter)
{
    if (PB_LTYPE(iter->type) == PB_LTYPE_EXTENSION)
    {
        return true;
    }
    else
    {
        pb_size_t start = iter->index;
        uint32_t fieldinfo;

        do
        {
            /* Advance iterator but don't load values yet */
            advance_iterator(iter);

            /* Do fast check for field type */
            fieldinfo = PB_PROGMEM_READU32(iter->descriptor->field_info[iter->field_info_index]);

            if (PB_LTYPE((fieldinfo >> 8) & 0xFF) == PB_LTYPE_EXTENSION)
            {
                return load_descriptor_values(iter);
            }
        } while (iter->index != start);

        /* Searched all the way back to start, and found nothing. */
        (void)load_descriptor_values(iter);
        return false;
    }
}

static void *pb_const_cast(const void *p)
{
    /* Note: this casts away const, in order to use the common field iterator
     * logic for both encoding and decoding. The cast is done using union
     * to avoid spurious compiler warnings. */
    union {
        void *p1;
        const void *p2;
    } t;
    t.p2 = p;
    return t.p1;
}

bool pb_field_iter_begin_const(pb_field_iter_t *iter, const pb_msgdesc_t *desc, const void *message)
{
    return pb_field_iter_begin(iter, desc, pb_const_cast(message));
}

bool pb_field_iter_begin_extension_const(pb_field_iter_t *iter, const pb_extension_t *extension)
{
    return pb_field_iter_begin_extension(iter, (pb_extension_t*)pb_const_cast(extension));
}

bool pb_default_field_callback(pb_istream_t *istream, pb_ostream_t *ostream, const pb_field_t *field)
{
    if (field->data_size == sizeof(pb_callback_t))
    {
        pb_callback_t *pCallback = (pb_callback_t*)field->pData;

        if (pCallback != NULL)
        {
            if (istream != NULL && pCallback->funcs.decode != NULL)
            {
                return pCallback->funcs.decode(istream, field, &pCallback->arg);
            }

            if (ostream != NULL && pCallback->funcs.encode != NULL)
            {
                return pCallback->funcs.encode(ostream, field, &pCallback->arg);
            }
        }
    }

    return true; /* Success, but didn't do anything */

}

#ifdef PB_VALIDATE_UTF8

/* This function checks whether a string is valid UTF-8 text.
 *
 * Algorithm is adapted from https://www.cl.cam.ac.uk/~mgk25/ucs/utf8_check.c
 * Original copyright: Markus Kuhn <http://www.cl.cam.ac.uk/~mgk25/> 2005-03-30
 * Licensed under "Short code license", which allows use under MIT license or
 * any compatible with it.
 */

bool pb_validate_utf8(const char *str)
{
    const pb_byte_t *s = (const pb_byte_t*)str;
    while (*s)
    {
        if (*s < 0x80)
        {
            /* 0xxxxxxx */
            s++;
        }
        else if ((s[0] & 0xe0) == 0xc0)
        {
            /* 110XXXXx 10xxxxxx */
            if ((s[1] & 0xc0) != 0x80 ||
                (s[0] & 0xfe) == 0xc0)                        /* overlong? */
                return false;
            else
                s += 2;
        }
        else if ((s[0] & 0xf0) == 0xe0)
        {
            /* 1110XXXX 10Xxxxxx 10xxxxxx */
            if ((s[1] & 0xc0) != 0x80 ||
                (s[2] & 0xc0) != 0x80 ||
                (s[0] == 0xe0 && (s[1] & 0xe0) == 0x80) ||    /* overlong? */
                (s[0] == 0xed && (s[1] & 0xe0) == 0xa0) ||    /* surrogate? */
                (s[0] == 0xef && s[1] == 0xbf &&
                (s[2] & 0xfe) == 0xbe))                 /* U+FFFE or U+FFFF? */
                return false;
            else
                s += 3;
        }
        else if ((s[0] & 0xf8) == 0xf0)
        {
            /* 11110XXX 10XXxxxx 10xxxxxx 10xxxxxx */
            if ((s[1] & 0xc0) != 0x80 ||
                (s[2] & 0xc0) != 0x80 ||
                (s[3] & 0xc0) != 0x80 ||
                (s[0] == 0xf0 && (s[1] & 0xf0) == 0x80) ||    /* overlong? */
                (s[0] == 0xf4 && s[1] > 0x8f) || s[0] > 0xf4) /* > U+10FFFF? */
                return false;
            else
                s += 4;
        }
        else
        {
            return false;
        }
    }

    return true;
}

#endif

/* pb_encode.c -- encode a protobuf using minimal resources
 *
 * 2011 Petteri Aimonen <jpa@kapsi.fi>
 */


/* Use the GCC warn_unused_result attribute to check that all return values
 * are propagated correctly. On other compilers and gcc before 3.4.0 just
 * ignore the annotation.
 */
#if !defined(__GNUC__) || ( __GNUC__ < 3) || (__GNUC__ == 3 && __GNUC_MINOR__ < 4)
    #define checkreturn
#else
    #define checkreturn __attribute__((warn_unused_result))
#endif

/**************************************
 * Declarations internal to this file *
 **************************************/
static bool checkreturn buf_write(pb_ostream_t *stream, const pb_byte_t *buf, size_t count);
static bool checkreturn encode_array(pb_ostream_t *stream, pb_field_iter_t *field);
static bool checkreturn pb_check_proto3_default_value(const pb_field_iter_t *field);
static bool checkreturn encode_basic_field(pb_ostream_t *stream, const pb_field_iter_t *field);
static bool checkreturn encode_callback_field(pb_ostream_t *stream, const pb_field_iter_t *field);
static bool checkreturn encode_field(pb_ostream_t *stream, pb_field_iter_t *field);
static bool checkreturn encode_extension_field(pb_ostream_t *stream, const pb_field_iter_t *field);
static bool checkreturn default_extension_encoder(pb_ostream_t *stream, const pb_extension_t *extension);
static bool checkreturn pb_encode_varint_32(pb_ostream_t *stream, uint32_t low, uint32_t high);
static bool checkreturn pb_enc_bool(pb_ostream_t *stream, const pb_field_iter_t *field);
static bool checkreturn pb_enc_varint(pb_ostream_t *stream, const pb_field_iter_t *field);
static bool checkreturn pb_enc_fixed(pb_ostream_t *stream, const pb_field_iter_t *field);
static bool checkreturn pb_enc_bytes(pb_ostream_t *stream, const pb_field_iter_t *field);
static bool checkreturn pb_enc_string(pb_ostream_t *stream, const pb_field_iter_t *field);
static bool checkreturn pb_enc_submessage(pb_ostream_t *stream, const pb_field_iter_t *field);
static bool checkreturn pb_enc_fixed_length_bytes(pb_ostream_t *stream, const pb_field_iter_t *field);

#ifdef PB_WITHOUT_64BIT
#define pb_int64_t int32_t
#define pb_uint64_t uint32_t
#else
#define pb_int64_t int64_t
#define pb_uint64_t uint64_t
#endif

/*******************************
 * pb_ostream_t implementation *
 *******************************/

static bool checkreturn buf_write(pb_ostream_t *stream, const pb_byte_t *buf, size_t count)
{
    size_t i;
    pb_byte_t *dest = (pb_byte_t*)stream->state;
    stream->state = dest + count;
    
    for (i = 0; i < count; i++)
        dest[i] = buf[i];
    
    return true;
}

pb_ostream_t pb_ostream_from_buffer(pb_byte_t *buf, size_t bufsize)
{
    pb_ostream_t stream;
#ifdef PB_BUFFER_ONLY
    stream.callback = (void*)1; /* Just a marker value */
#else
    stream.callback = &buf_write;
#endif
    stream.state = buf;
    stream.max_size = bufsize;
    stream.bytes_written = 0;
#ifndef PB_NO_ERRMSG
    stream.errmsg = NULL;
#endif
    return stream;
}

bool checkreturn pb_write(pb_ostream_t *stream, const pb_byte_t *buf, size_t count)
{
    if (count > 0 && stream->callback != NULL)
    {
        if (stream->bytes_written + count < stream->bytes_written ||
            stream->bytes_written + count > stream->max_size)
        {
            PB_RETURN_ERROR(stream, "stream full");
        }

#ifdef PB_BUFFER_ONLY
        if (!buf_write(stream, buf, count))
            PB_RETURN_ERROR(stream, "io error");
#else        
        if (!stream->callback(stream, buf, count))
            PB_RETURN_ERROR(stream, "io error");
#endif
    }
    
    stream->bytes_written += count;
    return true;
}

/*************************
 * Encode a single field *
 *************************/

/* Read a bool value without causing undefined behavior even if the value
 * is invalid. See issue #434 and
 * https://stackoverflow.com/questions/27661768/weird-results-for-conditional
 */
static bool safe_read_bool(const void *pSize)
{
    const char *p = (const char *)pSize;
    size_t i;
    for (i = 0; i < sizeof(bool); i++)
    {
        if (p[i] != 0)
            return true;
    }
    return false;
}

/* Encode a static array. Handles the size calculations and possible packing. */
static bool checkreturn encode_array(pb_ostream_t *stream, pb_field_iter_t *field)
{
    pb_size_t i;
    pb_size_t count;
#ifndef PB_ENCODE_ARRAYS_UNPACKED
    size_t size;
#endif

    count = *(pb_size_t*)field->pSize;

    if (count == 0)
        return true;

    if (PB_ATYPE(field->type) != PB_ATYPE_POINTER && count > field->array_size)
        PB_RETURN_ERROR(stream, "array max size exceeded");
    
#ifndef PB_ENCODE_ARRAYS_UNPACKED
    /* We always pack arrays if the datatype allows it. */
    if (PB_LTYPE(field->type) <= PB_LTYPE_LAST_PACKABLE)
    {
        if (!pb_encode_tag(stream, PB_WT_STRING, field->tag))
            return false;
        
        /* Determine the total size of packed array. */
        if (PB_LTYPE(field->type) == PB_LTYPE_FIXED32)
        {
            size = 4 * (size_t)count;
        }
        else if (PB_LTYPE(field->type) == PB_LTYPE_FIXED64)
        {
            size = 8 * (size_t)count;
        }
        else
        { 
            pb_ostream_t sizestream = PB_OSTREAM_SIZING;
            void *pData_orig = field->pData;
            for (i = 0; i < count; i++)
            {
                if (!pb_enc_varint(&sizestream, field))
                    PB_RETURN_ERROR(stream, PB_GET_ERROR(&sizestream));
                field->pData = (char*)field->pData + field->data_size;
            }
            field->pData = pData_orig;
            size = sizestream.bytes_written;
        }
        
        if (!pb_encode_varint(stream, (pb_uint64_t)size))
            return false;
        
        if (stream->callback == NULL)
            return pb_write(stream, NULL, size); /* Just sizing.. */
        
        /* Write the data */
        for (i = 0; i < count; i++)
        {
            if (PB_LTYPE(field->type) == PB_LTYPE_FIXED32 || PB_LTYPE(field->type) == PB_LTYPE_FIXED64)
            {
                if (!pb_enc_fixed(stream, field))
                    return false;
            }
            else
            {
                if (!pb_enc_varint(stream, field))
                    return false;
            }

            field->pData = (char*)field->pData + field->data_size;
        }
    }
    else /* Unpacked fields */
#endif
    {
        for (i = 0; i < count; i++)
        {
            /* Normally the data is stored directly in the array entries, but
             * for pointer-type string and bytes fields, the array entries are
             * actually pointers themselves also. So we have to dereference once
             * more to get to the actual data. */
            if (PB_ATYPE(field->type) == PB_ATYPE_POINTER &&
                (PB_LTYPE(field->type) == PB_LTYPE_STRING ||
                 PB_LTYPE(field->type) == PB_LTYPE_BYTES))
            {
                bool status;
                void *pData_orig = field->pData;
                field->pData = *(void* const*)field->pData;

                if (!field->pData)
                {
                    /* Null pointer in array is treated as empty string / bytes */
                    status = pb_encode_tag_for_field(stream, field) &&
                             pb_encode_varint(stream, 0);
                }
                else
                {
                    status = encode_basic_field(stream, field);
                }

                field->pData = pData_orig;

                if (!status)
                    return false;
            }
            else
            {
                if (!encode_basic_field(stream, field))
                    return false;
            }
            field->pData = (char*)field->pData + field->data_size;
        }
    }
    
    return true;
}

/* In proto3, all fields are optional and are only encoded if their value is "non-zero".
 * This function implements the check for the zero value. */
static bool checkreturn pb_check_proto3_default_value(const pb_field_iter_t *field)
{
    pb_type_t type = field->type;

    if (PB_ATYPE(type) == PB_ATYPE_STATIC)
    {
        if (PB_HTYPE(type) == PB_HTYPE_REQUIRED)
        {
            /* Required proto2 fields inside proto3 submessage, pretty rare case */
            return false;
        }
        else if (PB_HTYPE(type) == PB_HTYPE_REPEATED)
        {
            /* Repeated fields inside proto3 submessage: present if count != 0 */
            return *(const pb_size_t*)field->pSize == 0;
        }
        else if (PB_HTYPE(type) == PB_HTYPE_ONEOF)
        {
            /* Oneof fields */
            return *(const pb_size_t*)field->pSize == 0;
        }
        else if (PB_HTYPE(type) == PB_HTYPE_OPTIONAL && field->pSize != NULL)
        {
            /* Proto2 optional fields inside proto3 message, or proto3
             * submessage fields. */
            return safe_read_bool(field->pSize) == false;
        }
        else if (field->descriptor->default_value)
        {
            /* Proto3 messages do not have default values, but proto2 messages
             * can contain optional fields without has_fields (generator option 'proto3').
             * In this case they must always be encoded, to make sure that the
             * non-zero default value is overwritten.
             */
            return false;
        }

        /* Rest is proto3 singular fields */
        if (PB_LTYPE(type) <= PB_LTYPE_LAST_PACKABLE)
        {
            /* Simple integer / float fields */
            pb_size_t i;
            const char *p = (const char*)field->pData;
            for (i = 0; i < field->data_size; i++)
            {
                if (p[i] != 0)
                {
                    return false;
                }
            }

            return true;
        }
        else if (PB_LTYPE(type) == PB_LTYPE_BYTES)
        {
            const pb_bytes_array_t *bytes = (const pb_bytes_array_t*)field->pData;
            return bytes->size == 0;
        }
        else if (PB_LTYPE(type) == PB_LTYPE_STRING)
        {
            return *(const char*)field->pData == '\0';
        }
        else if (PB_LTYPE(type) == PB_LTYPE_FIXED_LENGTH_BYTES)
        {
            /* Fixed length bytes is only empty if its length is fixed
             * as 0. Which would be pretty strange, but we can check
             * it anyway. */
            return field->data_size == 0;
        }
        else if (PB_LTYPE_IS_SUBMSG(type))
        {
            /* Check all fields in the submessage to find if any of them
             * are non-zero. The comparison cannot be done byte-per-byte
             * because the C struct may contain padding bytes that must
             * be skipped. Note that usually proto3 submessages have
             * a separate has_field that is checked earlier in this if.
             */
            pb_field_iter_t iter;
            if (pb_field_iter_begin(&iter, field->submsg_desc, field->pData))
            {
                do
                {
                    if (!pb_check_proto3_default_value(&iter))
                    {
                        return false;
                    }
                } while (pb_field_iter_next(&iter));
            }
            return true;
        }
    }
    else if (PB_ATYPE(type) == PB_ATYPE_POINTER)
    {
        return field->pData == NULL;
    }
    else if (PB_ATYPE(type) == PB_ATYPE_CALLBACK)
    {
        if (PB_LTYPE(type) == PB_LTYPE_EXTENSION)
        {
            const pb_extension_t *extension = *(const pb_extension_t* const *)field->pData;
            return extension == NULL;
        }
        else if (field->descriptor->field_callback == pb_default_field_callback)
        {
            pb_callback_t *pCallback = (pb_callback_t*)field->pData;
            return pCallback->funcs.encode == NULL;
        }
        else
        {
            return field->descriptor->field_callback == NULL;
        }
    }

    return false; /* Not typically reached, safe default for weird special cases. */
}

/* Encode a field with static or pointer allocation, i.e. one whose data
 * is available to the encoder directly. */
static bool checkreturn encode_basic_field(pb_ostream_t *stream, const pb_field_iter_t *field)
{
    if (!field->pData)
    {
        /* Missing pointer field */
        return true;
    }

    if (!pb_encode_tag_for_field(stream, field))
        return false;

    switch (PB_LTYPE(field->type))
    {
        case PB_LTYPE_BOOL:
            return pb_enc_bool(stream, field);

        case PB_LTYPE_VARINT:
        case PB_LTYPE_UVARINT:
        case PB_LTYPE_SVARINT:
            return pb_enc_varint(stream, field);

        case PB_LTYPE_FIXED32:
        case PB_LTYPE_FIXED64:
            return pb_enc_fixed(stream, field);

        case PB_LTYPE_BYTES:
            return pb_enc_bytes(stream, field);

        case PB_LTYPE_STRING:
            return pb_enc_string(stream, field);

        case PB_LTYPE_SUBMESSAGE:
        case PB_LTYPE_SUBMSG_W_CB:
            return pb_enc_submessage(stream, field);

        case PB_LTYPE_FIXED_LENGTH_BYTES:
            return pb_enc_fixed_length_bytes(stream, field);

        default:
            PB_RETURN_ERROR(stream, "invalid field type");
    }
}

/* Encode a field with callback semantics. This means that a user function is
 * called to provide and encode the actual data. */
static bool checkreturn encode_callback_field(pb_ostream_t *stream, const pb_field_iter_t *field)
{
    if (field->descriptor->field_callback != NULL)
    {
        if (!field->descriptor->field_callback(NULL, stream, field))
            PB_RETURN_ERROR(stream, "callback error");
    }
    return true;
}

/* Encode a single field of any callback, pointer or static type. */
static bool checkreturn encode_field(pb_ostream_t *stream, pb_field_iter_t *field)
{
    /* Check field presence */
    if (PB_HTYPE(field->type) == PB_HTYPE_ONEOF)
    {
        if (*(const pb_size_t*)field->pSize != field->tag)
        {
            /* Different type oneof field */
            return true;
        }
    }
    else if (PB_HTYPE(field->type) == PB_HTYPE_OPTIONAL)
    {
        if (field->pSize)
        {
            if (safe_read_bool(field->pSize) == false)
            {
                /* Missing optional field */
                return true;
            }
        }
        else if (PB_ATYPE(field->type) == PB_ATYPE_STATIC)
        {
            /* Proto3 singular field */
            if (pb_check_proto3_default_value(field))
                return true;
        }
    }

    if (!field->pData)
    {
        if (PB_HTYPE(field->type) == PB_HTYPE_REQUIRED)
            PB_RETURN_ERROR(stream, "missing required field");

        /* Pointer field set to NULL */
        return true;
    }

    /* Then encode field contents */
    if (PB_ATYPE(field->type) == PB_ATYPE_CALLBACK)
    {
        return encode_callback_field(stream, field);
    }
    else if (PB_HTYPE(field->type) == PB_HTYPE_REPEATED)
    {
        return encode_array(stream, field);
    }
    else
    {
        return encode_basic_field(stream, field);
    }
}

/* Default handler for extension fields. Expects to have a pb_msgdesc_t
 * pointer in the extension->type->arg field, pointing to a message with
 * only one field in it.  */
static bool checkreturn default_extension_encoder(pb_ostream_t *stream, const pb_extension_t *extension)
{
    pb_field_iter_t iter;

    if (!pb_field_iter_begin_extension_const(&iter, extension))
        PB_RETURN_ERROR(stream, "invalid extension");

    return encode_field(stream, &iter);
}


/* Walk through all the registered extensions and give them a chance
 * to encode themselves. */
static bool checkreturn encode_extension_field(pb_ostream_t *stream, const pb_field_iter_t *field)
{
    const pb_extension_t *extension = *(const pb_extension_t* const *)field->pData;

    while (extension)
    {
        bool status;
        if (extension->type->encode)
            status = extension->type->encode(stream, extension);
        else
            status = default_extension_encoder(stream, extension);

        if (!status)
            return false;
        
        extension = extension->next;
    }
    
    return true;
}

/*********************
 * Encode all fields *
 *********************/

bool checkreturn pb_encode(pb_ostream_t *stream, const pb_msgdesc_t *fields, const void *src_struct)
{
    pb_field_iter_t iter;
    if (!pb_field_iter_begin_const(&iter, fields, src_struct))
        return true; /* Empty message type */
    
    do {
        if (PB_LTYPE(iter.type) == PB_LTYPE_EXTENSION)
        {
            /* Special case for the extension field placeholder */
            if (!encode_extension_field(stream, &iter))
                return false;
        }
        else
        {
            /* Regular field */
            if (!encode_field(stream, &iter))
                return false;
        }
    } while (pb_field_iter_next(&iter));
    
    return true;
}

bool checkreturn pb_encode_ex(pb_ostream_t *stream, const pb_msgdesc_t *fields, const void *src_struct, unsigned int flags)
{
  if ((flags & PB_ENCODE_DELIMITED) != 0)
  {
    return pb_encode_submessage(stream, fields, src_struct);
  }
  else if ((flags & PB_ENCODE_NULLTERMINATED) != 0)
  {
    const pb_byte_t zero = 0;

    if (!pb_encode(stream, fields, src_struct))
        return false;

    return pb_write(stream, &zero, 1);
  }
  else
  {
    return pb_encode(stream, fields, src_struct);
  }
}

bool pb_get_encoded_size(size_t *size, const pb_msgdesc_t *fields, const void *src_struct)
{
    pb_ostream_t stream = PB_OSTREAM_SIZING;
    
    if (!pb_encode(&stream, fields, src_struct))
        return false;
    
    *size = stream.bytes_written;
    return true;
}

/********************
 * Helper functions *
 ********************/

/* This function avoids 64-bit shifts as they are quite slow on many platforms. */
static bool checkreturn pb_encode_varint_32(pb_ostream_t *stream, uint32_t low, uint32_t high)
{
    size_t i = 0;
    pb_byte_t buffer[10];
    pb_byte_t byte = (pb_byte_t)(low & 0x7F);
    low >>= 7;

    while (i < 4 && (low != 0 || high != 0))
    {
        byte |= 0x80;
        buffer[i++] = byte;
        byte = (pb_byte_t)(low & 0x7F);
        low >>= 7;
    }

    if (high)
    {
        byte = (pb_byte_t)(byte | ((high & 0x07) << 4));
        high >>= 3;

        while (high)
        {
            byte |= 0x80;
            buffer[i++] = byte;
            byte = (pb_byte_t)(high & 0x7F);
            high >>= 7;
        }
    }

    buffer[i++] = byte;

    return pb_write(stream, buffer, i);
}

bool checkreturn pb_encode_varint(pb_ostream_t *stream, pb_uint64_t value)
{
    if (value <= 0x7F)
    {
        /* Fast path: single byte */
        pb_byte_t byte = (pb_byte_t)value;
        return pb_write(stream, &byte, 1);
    }
    else
    {
#ifdef PB_WITHOUT_64BIT
        return pb_encode_varint_32(stream, value, 0);
#else
        return pb_encode_varint_32(stream, (uint32_t)value, (uint32_t)(value >> 32));
#endif
    }
}

bool checkreturn pb_encode_svarint(pb_ostream_t *stream, pb_int64_t value)
{
    pb_uint64_t zigzagged;
    if (value < 0)
        zigzagged = ~((pb_uint64_t)value << 1);
    else
        zigzagged = (pb_uint64_t)value << 1;
    
    return pb_encode_varint(stream, zigzagged);
}

bool checkreturn pb_encode_fixed32(pb_ostream_t *stream, const void *value)
{
    uint32_t val = *(const uint32_t*)value;
    pb_byte_t bytes[4];
    bytes[0] = (pb_byte_t)(val & 0xFF);
    bytes[1] = (pb_byte_t)((val >> 8) & 0xFF);
    bytes[2] = (pb_byte_t)((val >> 16) & 0xFF);
    bytes[3] = (pb_byte_t)((val >> 24) & 0xFF);
    return pb_write(stream, bytes, 4);
}

#ifndef PB_WITHOUT_64BIT
bool checkreturn pb_encode_fixed64(pb_ostream_t *stream, const void *value)
{
    uint64_t val = *(const uint64_t*)value;
    pb_byte_t bytes[8];
    bytes[0] = (pb_byte_t)(val & 0xFF);
    bytes[1] = (pb_byte_t)((val >> 8) & 0xFF);
    bytes[2] = (pb_byte_t)((val >> 16) & 0xFF);
    bytes[3] = (pb_byte_t)((val >> 24) & 0xFF);
    bytes[4] = (pb_byte_t)((val >> 32) & 0xFF);
    bytes[5] = (pb_byte_t)((val >> 40) & 0xFF);
    bytes[6] = (pb_byte_t)((val >> 48) & 0xFF);
    bytes[7] = (pb_byte_t)((val >> 56) & 0xFF);
    return pb_write(stream, bytes, 8);
}
#endif

bool checkreturn pb_encode_tag(pb_ostream_t *stream, pb_wire_type_t wiretype, uint32_t field_number)
{
    pb_uint64_t tag = ((pb_uint64_t)field_number << 3) | wiretype;
    return pb_encode_varint(stream, tag);
}

bool pb_encode_tag_for_field ( pb_ostream_t* stream, const pb_field_iter_t* field )
{
    pb_wire_type_t wiretype;
    switch (PB_LTYPE(field->type))
    {
        case PB_LTYPE_BOOL:
        case PB_LTYPE_VARINT:
        case PB_LTYPE_UVARINT:
        case PB_LTYPE_SVARINT:
            wiretype = PB_WT_VARINT;
            break;
        
        case PB_LTYPE_FIXED32:
            wiretype = PB_WT_32BIT;
            break;
        
        case PB_LTYPE_FIXED64:
            wiretype = PB_WT_64BIT;
            break;
        
        case PB_LTYPE_BYTES:
        case PB_LTYPE_STRING:
        case PB_LTYPE_SUBMESSAGE:
        case PB_LTYPE_SUBMSG_W_CB:
        case PB_LTYPE_FIXED_LENGTH_BYTES:
            wiretype = PB_WT_STRING;
            break;
        
        default:
            PB_RETURN_ERROR(stream, "invalid field type");
    }
    
    return pb_encode_tag(stream, wiretype, field->tag);
}

bool checkreturn pb_encode_string(pb_ostream_t *stream, const pb_byte_t *buffer, size_t size)
{
    if (!pb_encode_varint(stream, (pb_uint64_t)size))
        return false;
    
    return pb_write(stream, buffer, size);
}

bool checkreturn pb_encode_submessage(pb_ostream_t *stream, const pb_msgdesc_t *fields, const void *src_struct)
{
    /* First calculate the message size using a non-writing substream. */
    pb_ostream_t substream = PB_OSTREAM_SIZING;
    size_t size;
    bool status;
    
    if (!pb_encode(&substream, fields, src_struct))
    {
#ifndef PB_NO_ERRMSG
        stream->errmsg = substream.errmsg;
#endif
        return false;
    }
    
    size = substream.bytes_written;
    
    if (!pb_encode_varint(stream, (pb_uint64_t)size))
        return false;
    
    if (stream->callback == NULL)
        return pb_write(stream, NULL, size); /* Just sizing */
    
    if (stream->bytes_written + size > stream->max_size)
        PB_RETURN_ERROR(stream, "stream full");
        
    /* Use a substream to verify that a callback doesn't write more than
     * what it did the first time. */
    substream.callback = stream->callback;
    substream.state = stream->state;
    substream.max_size = size;
    substream.bytes_written = 0;
#ifndef PB_NO_ERRMSG
    substream.errmsg = NULL;
#endif
    
    status = pb_encode(&substream, fields, src_struct);
    
    stream->bytes_written += substream.bytes_written;
    stream->state = substream.state;
#ifndef PB_NO_ERRMSG
    stream->errmsg = substream.errmsg;
#endif
    
    if (substream.bytes_written != size)
        PB_RETURN_ERROR(stream, "submsg size changed");
    
    return status;
}

/* Field encoders */

static bool checkreturn pb_enc_bool(pb_ostream_t *stream, const pb_field_iter_t *field)
{
    uint32_t value = safe_read_bool(field->pData) ? 1 : 0;
    PB_UNUSED(field);
    return pb_encode_varint(stream, value);
}

static bool checkreturn pb_enc_varint(pb_ostream_t *stream, const pb_field_iter_t *field)
{
    if (PB_LTYPE(field->type) == PB_LTYPE_UVARINT)
    {
        /* Perform unsigned integer extension */
        pb_uint64_t value = 0;

        if (field->data_size == sizeof(uint_least8_t))
            value = *(const uint_least8_t*)field->pData;
        else if (field->data_size == sizeof(uint_least16_t))
            value = *(const uint_least16_t*)field->pData;
        else if (field->data_size == sizeof(uint32_t))
            value = *(const uint32_t*)field->pData;
        else if (field->data_size == sizeof(pb_uint64_t))
            value = *(const pb_uint64_t*)field->pData;
        else
            PB_RETURN_ERROR(stream, "invalid data_size");

        return pb_encode_varint(stream, value);
    }
    else
    {
        /* Perform signed integer extension */
        pb_int64_t value = 0;

        if (field->data_size == sizeof(int_least8_t))
            value = *(const int_least8_t*)field->pData;
        else if (field->data_size == sizeof(int_least16_t))
            value = *(const int_least16_t*)field->pData;
        else if (field->data_size == sizeof(int32_t))
            value = *(const int32_t*)field->pData;
        else if (field->data_size == sizeof(pb_int64_t))
            value = *(const pb_int64_t*)field->pData;
        else
            PB_RETURN_ERROR(stream, "invalid data_size");

        if (PB_LTYPE(field->type) == PB_LTYPE_SVARINT)
            return pb_encode_svarint(stream, value);
#ifdef PB_WITHOUT_64BIT
        else if (value < 0)
            return pb_encode_varint_32(stream, (uint32_t)value, (uint32_t)-1);
#endif
        else
            return pb_encode_varint(stream, (pb_uint64_t)value);

    }
}

static bool checkreturn pb_enc_fixed(pb_ostream_t *stream, const pb_field_iter_t *field)
{
#ifdef PB_CONVERT_DOUBLE_FLOAT
    if (field->data_size == sizeof(float) && PB_LTYPE(field->type) == PB_LTYPE_FIXED64)
    {
        return pb_encode_float_as_double(stream, *(float*)field->pData);
    }
#endif

    if (field->data_size == sizeof(uint32_t))
    {
        return pb_encode_fixed32(stream, field->pData);
    }
#ifndef PB_WITHOUT_64BIT
    else if (field->data_size == sizeof(uint64_t))
    {
        return pb_encode_fixed64(stream, field->pData);
    }
#endif
    else
    {
        PB_RETURN_ERROR(stream, "invalid data_size");
    }
}

static bool checkreturn pb_enc_bytes(pb_ostream_t *stream, const pb_field_iter_t *field)
{
    const pb_bytes_array_t *bytes = NULL;

    bytes = (const pb_bytes_array_t*)field->pData;
    
    if (bytes == NULL)
    {
        /* Treat null pointer as an empty bytes field */
        return pb_encode_string(stream, NULL, 0);
    }
    
    if (PB_ATYPE(field->type) == PB_ATYPE_STATIC &&
        bytes->size > field->data_size - offsetof(pb_bytes_array_t, bytes))
    {
        PB_RETURN_ERROR(stream, "bytes size exceeded");
    }
    
    return pb_encode_string(stream, bytes->bytes, (size_t)bytes->size);
}

static bool checkreturn pb_enc_string(pb_ostream_t *stream, const pb_field_iter_t *field)
{
    size_t size = 0;
    size_t max_size = (size_t)field->data_size;
    const char *str = (const char*)field->pData;
    
    if (PB_ATYPE(field->type) == PB_ATYPE_POINTER)
    {
        max_size = (size_t)-1;
    }
    else
    {
        /* pb_dec_string() assumes string fields end with a null
         * terminator when the type isn't PB_ATYPE_POINTER, so we
         * shouldn't allow more than max-1 bytes to be written to
         * allow space for the null terminator.
         */
        if (max_size == 0)
            PB_RETURN_ERROR(stream, "zero-length string");

        max_size -= 1;
    }


    if (str == NULL)
    {
        size = 0; /* Treat null pointer as an empty string */
    }
    else
    {
        const char *p = str;

        /* strnlen() is not always available, so just use a loop */
        while (size < max_size && *p != '\0')
        {
            size++;
            p++;
        }

        if (*p != '\0')
        {
            PB_RETURN_ERROR(stream, "unterminated string");
        }
    }

#ifdef PB_VALIDATE_UTF8
    if (!pb_validate_utf8(str))
        PB_RETURN_ERROR(stream, "invalid utf8");
#endif

    return pb_encode_string(stream, (const pb_byte_t*)str, size);
}

static bool checkreturn pb_enc_submessage(pb_ostream_t *stream, const pb_field_iter_t *field)
{
    if (field->submsg_desc == NULL)
        PB_RETURN_ERROR(stream, "invalid field descriptor");

    if (PB_LTYPE(field->type) == PB_LTYPE_SUBMSG_W_CB && field->pSize != NULL)
    {
        /* Message callback is stored right before pSize. */
        pb_callback_t *callback = (pb_callback_t*)field->pSize - 1;
        if (callback->funcs.encode)
        {
            if (!callback->funcs.encode(stream, field, &callback->arg))
                return false;
        }
    }
    
    return pb_encode_submessage(stream, field->submsg_desc, field->pData);
}

static bool checkreturn pb_enc_fixed_length_bytes(pb_ostream_t *stream, const pb_field_iter_t *field)
{
    return pb_encode_string(stream, (const pb_byte_t*)field->pData, (size_t)field->data_size);
}

#ifdef PB_CONVERT_DOUBLE_FLOAT
bool pb_encode_float_as_double(pb_ostream_t *stream, float value)
{
    union { float f; uint32_t i; } in;
    uint_least8_t sign;
    int exponent;
    uint64_t mantissa;

    in.f = value;

    /* Decompose input value */
    sign = (uint_least8_t)((in.i >> 31) & 1);
    exponent = (int)((in.i >> 23) & 0xFF) - 127;
    mantissa = in.i & 0x7FFFFF;

    if (exponent == 128)
    {
        /* Special value (NaN etc.) */
        exponent = 1024;
    }
    else if (exponent == -127)
    {
        if (!mantissa)
        {
            /* Zero */
            exponent = -1023;
        }
        else
        {
            /* Denormalized */
            mantissa <<= 1;
            while (!(mantissa & 0x800000))
            {
                mantissa <<= 1;
                exponent--;
            }
            mantissa &= 0x7FFFFF;
        }
    }

    /* Combine fields */
    mantissa <<= 29;
    mantissa |= (uint64_t)(exponent + 1023) << 52;
    mantissa |= (uint64_t)sign << 63;

    return pb_encode_fixed64(stream, &mantissa);
}
#endif
/* pb_decode.c -- decode a protobuf using minimal resources
 *
 * 2011 Petteri Aimonen <jpa@kapsi.fi>
 */

/* Use the GCC warn_unused_result attribute to check that all return values
 * are propagated correctly. On other compilers and gcc before 3.4.0 just
 * ignore the annotation.
 */
#if !defined(__GNUC__) || ( __GNUC__ < 3) || (__GNUC__ == 3 && __GNUC_MINOR__ < 4)
    #define checkreturn
#else
    #define checkreturn __attribute__((warn_unused_result))
#endif


/**************************************
 * Declarations internal to this file *
 **************************************/

static bool checkreturn buf_read(pb_istream_t *stream, pb_byte_t *buf, size_t count);
static bool checkreturn pb_decode_varint32_eof(pb_istream_t *stream, uint32_t *dest, bool *eof);
static bool checkreturn read_raw_value(pb_istream_t *stream, pb_wire_type_t wire_type, pb_byte_t *buf, size_t *size);
static bool checkreturn decode_basic_field(pb_istream_t *stream, pb_wire_type_t wire_type, pb_field_iter_t *field);
static bool checkreturn decode_static_field(pb_istream_t *stream, pb_wire_type_t wire_type, pb_field_iter_t *field);
static bool checkreturn decode_pointer_field(pb_istream_t *stream, pb_wire_type_t wire_type, pb_field_iter_t *field);
static bool checkreturn decode_callback_field(pb_istream_t *stream, pb_wire_type_t wire_type, pb_field_iter_t *field);
static bool checkreturn decode_field(pb_istream_t *stream, pb_wire_type_t wire_type, pb_field_iter_t *field);
static bool checkreturn default_extension_decoder(pb_istream_t *stream, pb_extension_t *extension, uint32_t tag, pb_wire_type_t wire_type);
static bool checkreturn decode_extension(pb_istream_t *stream, uint32_t tag, pb_wire_type_t wire_type, pb_extension_t *extension);
static bool pb_field_set_to_default(pb_field_iter_t *field);
static bool pb_message_set_to_defaults(pb_field_iter_t *iter);
static bool checkreturn pb_dec_bool(pb_istream_t *stream, const pb_field_iter_t *field);
static bool checkreturn pb_dec_varint(pb_istream_t *stream, const pb_field_iter_t *field);
static bool checkreturn pb_dec_bytes(pb_istream_t *stream, const pb_field_iter_t *field);
static bool checkreturn pb_dec_string(pb_istream_t *stream, const pb_field_iter_t *field);
static bool checkreturn pb_dec_submessage(pb_istream_t *stream, const pb_field_iter_t *field);
static bool checkreturn pb_dec_fixed_length_bytes(pb_istream_t *stream, const pb_field_iter_t *field);
static bool checkreturn pb_skip_varint(pb_istream_t *stream);
static bool checkreturn pb_skip_string(pb_istream_t *stream);

#ifdef PB_ENABLE_MALLOC
static bool checkreturn allocate_field(pb_istream_t *stream, void *pData, size_t data_size, size_t array_size);
static void initialize_pointer_field(void *pItem, pb_field_iter_t *field);
static bool checkreturn pb_release_union_field(pb_istream_t *stream, pb_field_iter_t *field);
static void pb_release_single_field(pb_field_iter_t *field);
#endif

#ifdef PB_WITHOUT_64BIT
#define pb_int64_t int32_t
#define pb_uint64_t uint32_t
#else
#define pb_int64_t int64_t
#define pb_uint64_t uint64_t
#endif

typedef struct {
    uint32_t bitfield[(PB_MAX_REQUIRED_FIELDS + 31) / 32];
} pb_fields_seen_t;

/*******************************
 * pb_istream_t implementation *
 *******************************/

static bool checkreturn buf_read(pb_istream_t *stream, pb_byte_t *buf, size_t count)
{
    size_t i;
    const pb_byte_t *source = (const pb_byte_t*)stream->state;
    stream->state = (pb_byte_t*)stream->state + count;
    
    if (buf != NULL)
    {
        for (i = 0; i < count; i++)
            buf[i] = source[i];
    }
    
    return true;
}

bool checkreturn pb_read(pb_istream_t *stream, pb_byte_t *buf, size_t count)
{
    if (count == 0)
        return true;

#ifndef PB_BUFFER_ONLY
	if (buf == NULL && stream->callback != buf_read)
	{
		/* Skip input bytes */
		pb_byte_t tmp[16];
		while (count > 16)
		{
			if (!pb_read(stream, tmp, 16))
				return false;
			
			count -= 16;
		}
		
		return pb_read(stream, tmp, count);
	}
#endif

    if (stream->bytes_left < count)
        PB_RETURN_ERROR(stream, "end-of-stream");
    
#ifndef PB_BUFFER_ONLY
    if (!stream->callback(stream, buf, count))
        PB_RETURN_ERROR(stream, "io error");
#else
    if (!buf_read(stream, buf, count))
        return false;
#endif
    
    stream->bytes_left -= count;
    return true;
}

/* Read a single byte from input stream. buf may not be NULL.
 * This is an optimization for the varint decoding. */
static bool checkreturn pb_readbyte(pb_istream_t *stream, pb_byte_t *buf)
{
    if (stream->bytes_left == 0)
        PB_RETURN_ERROR(stream, "end-of-stream");

#ifndef PB_BUFFER_ONLY
    if (!stream->callback(stream, buf, 1))
        PB_RETURN_ERROR(stream, "io error");
#else
    *buf = *(const pb_byte_t*)stream->state;
    stream->state = (pb_byte_t*)stream->state + 1;
#endif

    stream->bytes_left--;
    
    return true;    
}

pb_istream_t pb_istream_from_buffer(const pb_byte_t *buf, size_t msglen)
{
    pb_istream_t stream;
    /* Cast away the const from buf without a compiler error.  We are
     * careful to use it only in a const manner in the callbacks.
     */
    union {
        void *state;
        const void *c_state;
    } state;
#ifdef PB_BUFFER_ONLY
    stream.callback = NULL;
#else
    stream.callback = &buf_read;
#endif
    state.c_state = buf;
    stream.state = state.state;
    stream.bytes_left = msglen;
#ifndef PB_NO_ERRMSG
    stream.errmsg = NULL;
#endif
    return stream;
}

/********************
 * Helper functions *
 ********************/

static bool checkreturn pb_decode_varint32_eof(pb_istream_t *stream, uint32_t *dest, bool *eof)
{
    pb_byte_t byte;
    uint32_t result;
    
    if (!pb_readbyte(stream, &byte))
    {
        if (stream->bytes_left == 0)
        {
            if (eof)
            {
                *eof = true;
            }
        }

        return false;
    }
    
    if ((byte & 0x80) == 0)
    {
        /* Quick case, 1 byte value */
        result = byte;
    }
    else
    {
        /* Multibyte case */
        uint_fast8_t bitpos = 7;
        result = byte & 0x7F;
        
        do
        {
            if (!pb_readbyte(stream, &byte))
                return false;
            
            if (bitpos >= 32)
            {
                /* Note: The varint could have trailing 0x80 bytes, or 0xFF for negative. */
                pb_byte_t sign_extension = (bitpos < 63) ? 0xFF : 0x01;
                bool valid_extension = ((byte & 0x7F) == 0x00 ||
                         ((result >> 31) != 0 && byte == sign_extension));

                if (bitpos >= 64 || !valid_extension)
                {
                    PB_RETURN_ERROR(stream, "varint overflow");
                }
            }
            else
            {
                result |= (uint32_t)(byte & 0x7F) << bitpos;
            }
            bitpos = (uint_fast8_t)(bitpos + 7);
        } while (byte & 0x80);
        
        if (bitpos == 35 && (byte & 0x70) != 0)
        {
            /* The last byte was at bitpos=28, so only bottom 4 bits fit. */
            PB_RETURN_ERROR(stream, "varint overflow");
        }
   }
   
   *dest = result;
   return true;
}

bool checkreturn pb_decode_varint32(pb_istream_t *stream, uint32_t *dest)
{
    return pb_decode_varint32_eof(stream, dest, NULL);
}

#ifndef PB_WITHOUT_64BIT
bool checkreturn pb_decode_varint(pb_istream_t *stream, uint64_t *dest)
{
    pb_byte_t byte;
    uint_fast8_t bitpos = 0;
    uint64_t result = 0;
    
    do
    {
        if (bitpos >= 64)
            PB_RETURN_ERROR(stream, "varint overflow");
        
        if (!pb_readbyte(stream, &byte))
            return false;

        result |= (uint64_t)(byte & 0x7F) << bitpos;
        bitpos = (uint_fast8_t)(bitpos + 7);
    } while (byte & 0x80);
    
    *dest = result;
    return true;
}
#endif

bool checkreturn pb_skip_varint(pb_istream_t *stream)
{
    pb_byte_t byte;
    do
    {
        if (!pb_read(stream, &byte, 1))
            return false;
    } while (byte & 0x80);
    return true;
}

bool checkreturn pb_skip_string(pb_istream_t *stream)
{
    uint32_t length;
    if (!pb_decode_varint32(stream, &length))
        return false;
    
    if ((size_t)length != length)
    {
        PB_RETURN_ERROR(stream, "size too large");
    }

    return pb_read(stream, NULL, (size_t)length);
}

bool checkreturn pb_decode_tag(pb_istream_t *stream, pb_wire_type_t *wire_type, uint32_t *tag, bool *eof)
{
    uint32_t temp;
    *eof = false;
    *wire_type = (pb_wire_type_t) 0;
    *tag = 0;
    
    if (!pb_decode_varint32_eof(stream, &temp, eof))
    {
        return false;
    }
    
    *tag = temp >> 3;
    *wire_type = (pb_wire_type_t)(temp & 7);
    return true;
}

bool checkreturn pb_skip_field(pb_istream_t *stream, pb_wire_type_t wire_type)
{
    switch (wire_type)
    {
        case PB_WT_VARINT: return pb_skip_varint(stream);
        case PB_WT_64BIT: return pb_read(stream, NULL, 8);
        case PB_WT_STRING: return pb_skip_string(stream);
        case PB_WT_32BIT: return pb_read(stream, NULL, 4);
        default: PB_RETURN_ERROR(stream, "invalid wire_type");
    }
}

/* Read a raw value to buffer, for the purpose of passing it to callback as
 * a substream. Size is maximum size on call, and actual size on return.
 */
static bool checkreturn read_raw_value(pb_istream_t *stream, pb_wire_type_t wire_type, pb_byte_t *buf, size_t *size)
{
    size_t max_size = *size;
    switch (wire_type)
    {
        case PB_WT_VARINT:
            *size = 0;
            do
            {
                (*size)++;
                if (*size > max_size)
                    PB_RETURN_ERROR(stream, "varint overflow");

                if (!pb_read(stream, buf, 1))
                    return false;
            } while (*buf++ & 0x80);
            return true;
            
        case PB_WT_64BIT:
            *size = 8;
            return pb_read(stream, buf, 8);
        
        case PB_WT_32BIT:
            *size = 4;
            return pb_read(stream, buf, 4);
        
        case PB_WT_STRING:
            /* Calling read_raw_value with a PB_WT_STRING is an error.
             * Explicitly handle this case and fallthrough to default to avoid
             * compiler warnings.
             */

        default: PB_RETURN_ERROR(stream, "invalid wire_type");
    }
}

/* Decode string length from stream and return a substream with limited length.
 * Remember to close the substream using pb_close_string_substream().
 */
bool checkreturn pb_make_string_substream(pb_istream_t *stream, pb_istream_t *substream)
{
    uint32_t size;
    if (!pb_decode_varint32(stream, &size))
        return false;
    
    *substream = *stream;
    if (substream->bytes_left < size)
        PB_RETURN_ERROR(stream, "parent stream too short");
    
    substream->bytes_left = (size_t)size;
    stream->bytes_left -= (size_t)size;
    return true;
}

bool checkreturn pb_close_string_substream(pb_istream_t *stream, pb_istream_t *substream)
{
    if (substream->bytes_left) {
        if (!pb_read(substream, NULL, substream->bytes_left))
            return false;
    }

    stream->state = substream->state;

#ifndef PB_NO_ERRMSG
    stream->errmsg = substream->errmsg;
#endif
    return true;
}

/*************************
 * Decode a single field *
 *************************/

static bool checkreturn decode_basic_field(pb_istream_t *stream, pb_wire_type_t wire_type, pb_field_iter_t *field)
{
    switch (PB_LTYPE(field->type))
    {
        case PB_LTYPE_BOOL:
            if (wire_type != PB_WT_VARINT && wire_type != PB_WT_PACKED)
                PB_RETURN_ERROR(stream, "wrong wire type");

            return pb_dec_bool(stream, field);

        case PB_LTYPE_VARINT:
        case PB_LTYPE_UVARINT:
        case PB_LTYPE_SVARINT:
            if (wire_type != PB_WT_VARINT && wire_type != PB_WT_PACKED)
                PB_RETURN_ERROR(stream, "wrong wire type");

            return pb_dec_varint(stream, field);

        case PB_LTYPE_FIXED32:
            if (wire_type != PB_WT_32BIT && wire_type != PB_WT_PACKED)
                PB_RETURN_ERROR(stream, "wrong wire type");

            return pb_decode_fixed32(stream, field->pData);

        case PB_LTYPE_FIXED64:
            if (wire_type != PB_WT_64BIT && wire_type != PB_WT_PACKED)
                PB_RETURN_ERROR(stream, "wrong wire type");

#ifdef PB_CONVERT_DOUBLE_FLOAT
            if (field->data_size == sizeof(float))
            {
                return pb_decode_double_as_float(stream, (float*)field->pData);
            }
#endif

#ifdef PB_WITHOUT_64BIT
            PB_RETURN_ERROR(stream, "invalid data_size");
#else
            return pb_decode_fixed64(stream, field->pData);
#endif

        case PB_LTYPE_BYTES:
            if (wire_type != PB_WT_STRING)
                PB_RETURN_ERROR(stream, "wrong wire type");

            return pb_dec_bytes(stream, field);

        case PB_LTYPE_STRING:
            if (wire_type != PB_WT_STRING)
                PB_RETURN_ERROR(stream, "wrong wire type");

            return pb_dec_string(stream, field);

        case PB_LTYPE_SUBMESSAGE:
        case PB_LTYPE_SUBMSG_W_CB:
            if (wire_type != PB_WT_STRING)
                PB_RETURN_ERROR(stream, "wrong wire type");

            return pb_dec_submessage(stream, field);

        case PB_LTYPE_FIXED_LENGTH_BYTES:
            if (wire_type != PB_WT_STRING)
                PB_RETURN_ERROR(stream, "wrong wire type");

            return pb_dec_fixed_length_bytes(stream, field);

        default:
            PB_RETURN_ERROR(stream, "invalid field type");
    }
}

static bool checkreturn decode_static_field(pb_istream_t *stream, pb_wire_type_t wire_type, pb_field_iter_t *field)
{
    switch (PB_HTYPE(field->type))
    {
        case PB_HTYPE_REQUIRED:
            return decode_basic_field(stream, wire_type, field);
            
        case PB_HTYPE_OPTIONAL:
            if (field->pSize != NULL)
                *(bool*)field->pSize = true;
            return decode_basic_field(stream, wire_type, field);
    
        case PB_HTYPE_REPEATED:
            if (wire_type == PB_WT_STRING
                && PB_LTYPE(field->type) <= PB_LTYPE_LAST_PACKABLE)
            {
                /* Packed array */
                bool status = true;
                pb_istream_t substream;
                pb_size_t *size = (pb_size_t*)field->pSize;
                field->pData = (char*)field->pField + field->data_size * (*size);

                if (!pb_make_string_substream(stream, &substream))
                    return false;

                while (substream.bytes_left > 0 && *size < field->array_size)
                {
                    if (!decode_basic_field(&substream, PB_WT_PACKED, field))
                    {
                        status = false;
                        break;
                    }
                    (*size)++;
                    field->pData = (char*)field->pData + field->data_size;
                }

                if (substream.bytes_left != 0)
                    PB_RETURN_ERROR(stream, "array overflow");
                if (!pb_close_string_substream(stream, &substream))
                    return false;

                return status;
            }
            else
            {
                /* Repeated field */
                pb_size_t *size = (pb_size_t*)field->pSize;
                field->pData = (char*)field->pField + field->data_size * (*size);

                if ((*size)++ >= field->array_size)
                    PB_RETURN_ERROR(stream, "array overflow");

                return decode_basic_field(stream, wire_type, field);
            }

        case PB_HTYPE_ONEOF:
            if (PB_LTYPE_IS_SUBMSG(field->type) &&
                *(pb_size_t*)field->pSize != field->tag)
            {
                /* We memset to zero so that any callbacks are set to NULL.
                 * This is because the callbacks might otherwise have values
                 * from some other union field.
                 * If callbacks are needed inside oneof field, use .proto
                 * option submsg_callback to have a separate callback function
                 * that can set the fields before submessage is decoded.
                 * pb_dec_submessage() will set any default values. */
                memset(field->pData, 0, (size_t)field->data_size);

                /* Set default values for the submessage fields. */
                if (field->submsg_desc->default_value != NULL ||
                    field->submsg_desc->field_callback != NULL ||
                    field->submsg_desc->submsg_info[0] != NULL)
                {
                    pb_field_iter_t submsg_iter;
                    if (pb_field_iter_begin(&submsg_iter, field->submsg_desc, field->pData))
                    {
                        if (!pb_message_set_to_defaults(&submsg_iter))
                            PB_RETURN_ERROR(stream, "failed to set defaults");
                    }
                }
            }
            *(pb_size_t*)field->pSize = field->tag;

            return decode_basic_field(stream, wire_type, field);

        default:
            PB_RETURN_ERROR(stream, "invalid field type");
    }
}

#ifdef PB_ENABLE_MALLOC
/* Allocate storage for the field and store the pointer at iter->pData.
 * array_size is the number of entries to reserve in an array.
 * Zero size is not allowed, use pb_free() for releasing.
 */
static bool checkreturn allocate_field(pb_istream_t *stream, void *pData, size_t data_size, size_t array_size)
{    
    void *ptr = *(void**)pData;
    
    if (data_size == 0 || array_size == 0)
        PB_RETURN_ERROR(stream, "invalid size");
    
#ifdef __AVR__
    /* Workaround for AVR libc bug 53284: http://savannah.nongnu.org/bugs/?53284
     * Realloc to size of 1 byte can cause corruption of the malloc structures.
     */
    if (data_size == 1 && array_size == 1)
    {
        data_size = 2;
    }
#endif

    /* Check for multiplication overflows.
     * This code avoids the costly division if the sizes are small enough.
     * Multiplication is safe as long as only half of bits are set
     * in either multiplicand.
     */
    {
        const size_t check_limit = (size_t)1 << (sizeof(size_t) * 4);
        if (data_size >= check_limit || array_size >= check_limit)
        {
            const size_t size_max = (size_t)-1;
            if (size_max / array_size < data_size)
            {
                PB_RETURN_ERROR(stream, "size too large");
            }
        }
    }
    
    /* Allocate new or expand previous allocation */
    /* Note: on failure the old pointer will remain in the structure,
     * the message must be freed by caller also on error return. */
    ptr = pb_realloc(ptr, array_size * data_size);
    if (ptr == NULL)
        PB_RETURN_ERROR(stream, "realloc failed");
    
    *(void**)pData = ptr;
    return true;
}

/* Clear a newly allocated item in case it contains a pointer, or is a submessage. */
static void initialize_pointer_field(void *pItem, pb_field_iter_t *field)
{
    if (PB_LTYPE(field->type) == PB_LTYPE_STRING ||
        PB_LTYPE(field->type) == PB_LTYPE_BYTES)
    {
        *(void**)pItem = NULL;
    }
    else if (PB_LTYPE_IS_SUBMSG(field->type))
    {
        /* We memset to zero so that any callbacks are set to NULL.
         * Default values will be set by pb_dec_submessage(). */
        memset(pItem, 0, field->data_size);
    }
}
#endif

static bool checkreturn decode_pointer_field(pb_istream_t *stream, pb_wire_type_t wire_type, pb_field_iter_t *field)
{
#ifndef PB_ENABLE_MALLOC
    PB_UNUSED(wire_type);
    PB_UNUSED(field);
    PB_RETURN_ERROR(stream, "no malloc support");
#else
    switch (PB_HTYPE(field->type))
    {
        case PB_HTYPE_REQUIRED:
        case PB_HTYPE_OPTIONAL:
        case PB_HTYPE_ONEOF:
            if (PB_LTYPE_IS_SUBMSG(field->type) && *(void**)field->pField != NULL)
            {
                /* Duplicate field, have to release the old allocation first. */
                /* FIXME: Does this work correctly for oneofs? */
                pb_release_single_field(field);
            }
        
            if (PB_HTYPE(field->type) == PB_HTYPE_ONEOF)
            {
                *(pb_size_t*)field->pSize = field->tag;
            }

            if (PB_LTYPE(field->type) == PB_LTYPE_STRING ||
                PB_LTYPE(field->type) == PB_LTYPE_BYTES)
            {
                /* pb_dec_string and pb_dec_bytes handle allocation themselves */
                field->pData = field->pField;
                return decode_basic_field(stream, wire_type, field);
            }
            else
            {
                if (!allocate_field(stream, field->pField, field->data_size, 1))
                    return false;
                
                field->pData = *(void**)field->pField;
                initialize_pointer_field(field->pData, field);
                return decode_basic_field(stream, wire_type, field);
            }
    
        case PB_HTYPE_REPEATED:
            if (wire_type == PB_WT_STRING
                && PB_LTYPE(field->type) <= PB_LTYPE_LAST_PACKABLE)
            {
                /* Packed array, multiple items come in at once. */
                bool status = true;
                pb_size_t *size = (pb_size_t*)field->pSize;
                size_t allocated_size = *size;
                pb_istream_t substream;
                
                if (!pb_make_string_substream(stream, &substream))
                    return false;
                
                while (substream.bytes_left)
                {
                    if (*size == PB_SIZE_MAX)
                    {
#ifndef PB_NO_ERRMSG
                        stream->errmsg = "too many array entries";
#endif
                        status = false;
                        break;
                    }

                    if ((size_t)*size + 1 > allocated_size)
                    {
                        /* Allocate more storage. This tries to guess the
                         * number of remaining entries. Round the division
                         * upwards. */
                        size_t remain = (substream.bytes_left - 1) / field->data_size + 1;
                        if (remain < PB_SIZE_MAX - allocated_size)
                            allocated_size += remain;
                        else
                            allocated_size += 1;
                        
                        if (!allocate_field(&substream, field->pField, field->data_size, allocated_size))
                        {
                            status = false;
                            break;
                        }
                    }

                    /* Decode the array entry */
                    field->pData = *(char**)field->pField + field->data_size * (*size);
                    if (field->pData == NULL)
                    {
                        /* Shouldn't happen, but satisfies static analyzers */
                        status = false;
                        break;
                    }
                    initialize_pointer_field(field->pData, field);
                    if (!decode_basic_field(&substream, PB_WT_PACKED, field))
                    {
                        status = false;
                        break;
                    }
                    
                    (*size)++;
                }
                if (!pb_close_string_substream(stream, &substream))
                    return false;
                
                return status;
            }
            else
            {
                /* Normal repeated field, i.e. only one item at a time. */
                pb_size_t *size = (pb_size_t*)field->pSize;

                if (*size == PB_SIZE_MAX)
                    PB_RETURN_ERROR(stream, "too many array entries");
                
                if (!allocate_field(stream, field->pField, field->data_size, (size_t)(*size + 1)))
                    return false;
            
                field->pData = *(char**)field->pField + field->data_size * (*size);
                (*size)++;
                initialize_pointer_field(field->pData, field);
                return decode_basic_field(stream, wire_type, field);
            }

        default:
            PB_RETURN_ERROR(stream, "invalid field type");
    }
#endif
}

static bool checkreturn decode_callback_field(pb_istream_t *stream, pb_wire_type_t wire_type, pb_field_iter_t *field)
{
    if (!field->descriptor->field_callback)
        return pb_skip_field(stream, wire_type);

    if (wire_type == PB_WT_STRING)
    {
        pb_istream_t substream;
        size_t prev_bytes_left;
        
        if (!pb_make_string_substream(stream, &substream))
            return false;
        
        do
        {
            prev_bytes_left = substream.bytes_left;
            if (!field->descriptor->field_callback(&substream, NULL, field))
                PB_RETURN_ERROR(stream, "callback failed");
        } while (substream.bytes_left > 0 && substream.bytes_left < prev_bytes_left);
        
        if (!pb_close_string_substream(stream, &substream))
            return false;

        return true;
    }
    else
    {
        /* Copy the single scalar value to stack.
         * This is required so that we can limit the stream length,
         * which in turn allows to use same callback for packed and
         * not-packed fields. */
        pb_istream_t substream;
        pb_byte_t buffer[10];
        size_t size = sizeof(buffer);
        
        if (!read_raw_value(stream, wire_type, buffer, &size))
            return false;
        substream = pb_istream_from_buffer(buffer, size);
        
        return field->descriptor->field_callback(&substream, NULL, field);
    }
}

static bool checkreturn decode_field(pb_istream_t *stream, pb_wire_type_t wire_type, pb_field_iter_t *field)
{
#ifdef PB_ENABLE_MALLOC
    /* When decoding an oneof field, check if there is old data that must be
     * released first. */
    if (PB_HTYPE(field->type) == PB_HTYPE_ONEOF)
    {
        if (!pb_release_union_field(stream, field))
            return false;
    }
#endif

    switch (PB_ATYPE(field->type))
    {
        case PB_ATYPE_STATIC:
            return decode_static_field(stream, wire_type, field);
        
        case PB_ATYPE_POINTER:
            return decode_pointer_field(stream, wire_type, field);
        
        case PB_ATYPE_CALLBACK:
            return decode_callback_field(stream, wire_type, field);
        
        default:
            PB_RETURN_ERROR(stream, "invalid field type");
    }
}

/* Default handler for extension fields. Expects to have a pb_msgdesc_t
 * pointer in the extension->type->arg field, pointing to a message with
 * only one field in it.  */
static bool checkreturn default_extension_decoder(pb_istream_t *stream,
    pb_extension_t *extension, uint32_t tag, pb_wire_type_t wire_type)
{
    pb_field_iter_t iter;

    if (!pb_field_iter_begin_extension(&iter, extension))
        PB_RETURN_ERROR(stream, "invalid extension");

    if (iter.tag != tag || !iter.message)
        return true;

    extension->found = true;
    return decode_field(stream, wire_type, &iter);
}

/* Try to decode an unknown field as an extension field. Tries each extension
 * decoder in turn, until one of them handles the field or loop ends. */
static bool checkreturn decode_extension(pb_istream_t *stream,
    uint32_t tag, pb_wire_type_t wire_type, pb_extension_t *extension)
{
    size_t pos = stream->bytes_left;
    
    while (extension != NULL && pos == stream->bytes_left)
    {
        bool status;
        if (extension->type->decode)
            status = extension->type->decode(stream, extension, tag, wire_type);
        else
            status = default_extension_decoder(stream, extension, tag, wire_type);

        if (!status)
            return false;
        
        extension = extension->next;
    }
    
    return true;
}

/* Initialize message fields to default values, recursively */
static bool pb_field_set_to_default(pb_field_iter_t *field)
{
    pb_type_t type;
    type = field->type;

    if (PB_LTYPE(type) == PB_LTYPE_EXTENSION)
    {
        pb_extension_t *ext = *(pb_extension_t* const *)field->pData;
        while (ext != NULL)
        {
            pb_field_iter_t ext_iter;
            if (pb_field_iter_begin_extension(&ext_iter, ext))
            {
                ext->found = false;
                if (!pb_message_set_to_defaults(&ext_iter))
                    return false;
            }
            ext = ext->next;
        }
    }
    else if (PB_ATYPE(type) == PB_ATYPE_STATIC)
    {
        bool init_data = true;
        if (PB_HTYPE(type) == PB_HTYPE_OPTIONAL && field->pSize != NULL)
        {
            /* Set has_field to false. Still initialize the optional field
             * itself also. */
            *(bool*)field->pSize = false;
        }
        else if (PB_HTYPE(type) == PB_HTYPE_REPEATED ||
                 PB_HTYPE(type) == PB_HTYPE_ONEOF)
        {
            /* REPEATED: Set array count to 0, no need to initialize contents.
               ONEOF: Set which_field to 0. */
            *(pb_size_t*)field->pSize = 0;
            init_data = false;
        }

        if (init_data)
        {
            if (PB_LTYPE_IS_SUBMSG(field->type) &&
                (field->submsg_desc->default_value != NULL ||
                 field->submsg_desc->field_callback != NULL ||
                 field->submsg_desc->submsg_info[0] != NULL))
            {
                /* Initialize submessage to defaults.
                 * Only needed if it has default values
                 * or callback/submessage fields. */
                pb_field_iter_t submsg_iter;
                if (pb_field_iter_begin(&submsg_iter, field->submsg_desc, field->pData))
                {
                    if (!pb_message_set_to_defaults(&submsg_iter))
                        return false;
                }
            }
            else
            {
                /* Initialize to zeros */
                memset(field->pData, 0, (size_t)field->data_size);
            }
        }
    }
    else if (PB_ATYPE(type) == PB_ATYPE_POINTER)
    {
        /* Initialize the pointer to NULL. */
        *(void**)field->pField = NULL;

        /* Initialize array count to 0. */
        if (PB_HTYPE(type) == PB_HTYPE_REPEATED ||
            PB_HTYPE(type) == PB_HTYPE_ONEOF)
        {
            *(pb_size_t*)field->pSize = 0;
        }
    }
    else if (PB_ATYPE(type) == PB_ATYPE_CALLBACK)
    {
        /* Don't overwrite callback */
    }

    return true;
}

static bool pb_message_set_to_defaults(pb_field_iter_t *iter)
{
    pb_istream_t defstream = PB_ISTREAM_EMPTY;
    uint32_t tag = 0;
    pb_wire_type_t wire_type = PB_WT_VARINT;
    bool eof;

    if (iter->descriptor->default_value)
    {
        defstream = pb_istream_from_buffer(iter->descriptor->default_value, (size_t)-1);
        if (!pb_decode_tag(&defstream, &wire_type, &tag, &eof))
            return false;
    }

    do
    {
        if (!pb_field_set_to_default(iter))
            return false;

        if (tag != 0 && iter->tag == tag)
        {
            /* We have a default value for this field in the defstream */
            if (!decode_field(&defstream, wire_type, iter))
                return false;
            if (!pb_decode_tag(&defstream, &wire_type, &tag, &eof))
                return false;

            if (iter->pSize)
                *(bool*)iter->pSize = false;
        }
    } while (pb_field_iter_next(iter));

    return true;
}

/*********************
 * Decode all fields *
 *********************/

static bool checkreturn pb_decode_inner(pb_istream_t *stream, const pb_msgdesc_t *fields, void *dest_struct, unsigned int flags)
{
    uint32_t extension_range_start = 0;
    pb_extension_t *extensions = NULL;

    /* 'fixed_count_field' and 'fixed_count_size' track position of a repeated fixed
     * count field. This can only handle _one_ repeated fixed count field that
     * is unpacked and unordered among other (non repeated fixed count) fields.
     */
    pb_size_t fixed_count_field = PB_SIZE_MAX;
    pb_size_t fixed_count_size = 0;
    pb_size_t fixed_count_total_size = 0;

    pb_fields_seen_t fields_seen = {{0, 0}};
    const uint32_t allbits = ~(uint32_t)0;
    pb_field_iter_t iter;

    if (pb_field_iter_begin(&iter, fields, dest_struct))
    {
        if ((flags & PB_DECODE_NOINIT) == 0)
        {
            if (!pb_message_set_to_defaults(&iter))
                PB_RETURN_ERROR(stream, "failed to set defaults");
        }
    }

    while (stream->bytes_left)
    {
        uint32_t tag;
        pb_wire_type_t wire_type;
        bool eof;

        if (!pb_decode_tag(stream, &wire_type, &tag, &eof))
        {
            if (eof)
                break;
            else
                return false;
        }

        if (tag == 0)
        {
          if (flags & PB_DECODE_NULLTERMINATED)
          {
            break;
          }
          else
          {
            PB_RETURN_ERROR(stream, "zero tag");
          }
        }

        if (!pb_field_iter_find(&iter, tag) || PB_LTYPE(iter.type) == PB_LTYPE_EXTENSION)
        {
            /* No match found, check if it matches an extension. */
            if (extension_range_start == 0)
            {
                if (pb_field_iter_find_extension(&iter))
                {
                    extensions = *(pb_extension_t* const *)iter.pData;
                    extension_range_start = iter.tag;
                }

                if (!extensions)
                {
                    extension_range_start = (uint32_t)-1;
                }
            }

            if (tag >= extension_range_start)
            {
                size_t pos = stream->bytes_left;

                if (!decode_extension(stream, tag, wire_type, extensions))
                    return false;

                if (pos != stream->bytes_left)
                {
                    /* The field was handled */
                    continue;
                }
            }

            /* No match found, skip data */
            if (!pb_skip_field(stream, wire_type))
                return false;
            continue;
        }

        /* If a repeated fixed count field was found, get size from
         * 'fixed_count_field' as there is no counter contained in the struct.
         */
        if (PB_HTYPE(iter.type) == PB_HTYPE_REPEATED && iter.pSize == &iter.array_size)
        {
            if (fixed_count_field != iter.index) {
                /* If the new fixed count field does not match the previous one,
                 * check that the previous one is NULL or that it finished
                 * receiving all the expected data.
                 */
                if (fixed_count_field != PB_SIZE_MAX &&
                    fixed_count_size != fixed_count_total_size)
                {
                    PB_RETURN_ERROR(stream, "wrong size for fixed count field");
                }

                fixed_count_field = iter.index;
                fixed_count_size = 0;
                fixed_count_total_size = iter.array_size;
            }

            iter.pSize = &fixed_count_size;
        }

        if (PB_HTYPE(iter.type) == PB_HTYPE_REQUIRED
            && iter.required_field_index < PB_MAX_REQUIRED_FIELDS)
        {
            uint32_t tmp = ((uint32_t)1 << (iter.required_field_index & 31));
            fields_seen.bitfield[iter.required_field_index >> 5] |= tmp;
        }

        if (!decode_field(stream, wire_type, &iter))
            return false;
    }

    /* Check that all elements of the last decoded fixed count field were present. */
    if (fixed_count_field != PB_SIZE_MAX &&
        fixed_count_size != fixed_count_total_size)
    {
        PB_RETURN_ERROR(stream, "wrong size for fixed count field");
    }

    /* Check that all required fields were present. */
    {
        pb_size_t req_field_count = iter.descriptor->required_field_count;

        if (req_field_count > 0)
        {
            pb_size_t i;

            if (req_field_count > PB_MAX_REQUIRED_FIELDS)
                req_field_count = PB_MAX_REQUIRED_FIELDS;

            /* Check the whole words */
            for (i = 0; i < (req_field_count >> 5); i++)
            {
                if (fields_seen.bitfield[i] != allbits)
                    PB_RETURN_ERROR(stream, "missing required field");
            }

            /* Check the remaining bits (if any) */
            if ((req_field_count & 31) != 0)
            {
                if (fields_seen.bitfield[req_field_count >> 5] !=
                    (allbits >> (uint_least8_t)(32 - (req_field_count & 31))))
                {
                    PB_RETURN_ERROR(stream, "missing required field");
                }
            }
        }
    }

    return true;
}

bool checkreturn pb_decode_ex(pb_istream_t *stream, const pb_msgdesc_t *fields, void *dest_struct, unsigned int flags)
{
    bool status;

    if ((flags & PB_DECODE_DELIMITED) == 0)
    {
      status = pb_decode_inner(stream, fields, dest_struct, flags);
    }
    else
    {
      pb_istream_t substream;
      if (!pb_make_string_substream(stream, &substream))
        return false;

      status = pb_decode_inner(&substream, fields, dest_struct, flags);

      if (!pb_close_string_substream(stream, &substream))
        return false;
    }
    
#ifdef PB_ENABLE_MALLOC
    if (!status)
        pb_release(fields, dest_struct);
#endif
    
    return status;
}

bool checkreturn pb_decode(pb_istream_t *stream, const pb_msgdesc_t *fields, void *dest_struct)
{
    bool status;

    status = pb_decode_inner(stream, fields, dest_struct, 0);

#ifdef PB_ENABLE_MALLOC
    if (!status)
        pb_release(fields, dest_struct);
#endif

    return status;
}

#ifdef PB_ENABLE_MALLOC
/* Given an oneof field, if there has already been a field inside this oneof,
 * release it before overwriting with a different one. */
static bool pb_release_union_field(pb_istream_t *stream, pb_field_iter_t *field)
{
    pb_field_iter_t old_field = *field;
    pb_size_t old_tag = *(pb_size_t*)field->pSize; /* Previous which_ value */
    pb_size_t new_tag = field->tag; /* New which_ value */

    if (old_tag == 0)
        return true; /* Ok, no old data in union */

    if (old_tag == new_tag)
        return true; /* Ok, old data is of same type => merge */

    /* Release old data. The find can fail if the message struct contains
     * invalid data. */
    if (!pb_field_iter_find(&old_field, old_tag))
        PB_RETURN_ERROR(stream, "invalid union tag");

    pb_release_single_field(&old_field);

    if (PB_ATYPE(field->type) == PB_ATYPE_POINTER)
    {
        /* Initialize the pointer to NULL to make sure it is valid
         * even in case of error return. */
        *(void**)field->pField = NULL;
        field->pData = NULL;
    }

    return true;
}

static void pb_release_single_field(pb_field_iter_t *field)
{
    pb_type_t type;
    type = field->type;

    if (PB_HTYPE(type) == PB_HTYPE_ONEOF)
    {
        if (*(pb_size_t*)field->pSize != field->tag)
            return; /* This is not the current field in the union */
    }

    /* Release anything contained inside an extension or submsg.
     * This has to be done even if the submsg itself is statically
     * allocated. */
    if (PB_LTYPE(type) == PB_LTYPE_EXTENSION)
    {
        /* Release fields from all extensions in the linked list */
        pb_extension_t *ext = *(pb_extension_t**)field->pData;
        while (ext != NULL)
        {
            pb_field_iter_t ext_iter;
            if (pb_field_iter_begin_extension(&ext_iter, ext))
            {
                pb_release_single_field(&ext_iter);
            }
            ext = ext->next;
        }
    }
    else if (PB_LTYPE_IS_SUBMSG(type) && PB_ATYPE(type) != PB_ATYPE_CALLBACK)
    {
        /* Release fields in submessage or submsg array */
        pb_size_t count = 1;
        
        if (PB_ATYPE(type) == PB_ATYPE_POINTER)
        {
            field->pData = *(void**)field->pField;
        }
        else
        {
            field->pData = field->pField;
        }
        
        if (PB_HTYPE(type) == PB_HTYPE_REPEATED)
        {
            count = *(pb_size_t*)field->pSize;

            if (PB_ATYPE(type) == PB_ATYPE_STATIC && count > field->array_size)
            {
                /* Protect against corrupted _count fields */
                count = field->array_size;
            }
        }
        
        if (field->pData)
        {
            for (; count > 0; count--)
            {
                pb_release(field->submsg_desc, field->pData);
                field->pData = (char*)field->pData + field->data_size;
            }
        }
    }
    
    if (PB_ATYPE(type) == PB_ATYPE_POINTER)
    {
        if (PB_HTYPE(type) == PB_HTYPE_REPEATED &&
            (PB_LTYPE(type) == PB_LTYPE_STRING ||
             PB_LTYPE(type) == PB_LTYPE_BYTES))
        {
            /* Release entries in repeated string or bytes array */
            void **pItem = *(void***)field->pField;
            pb_size_t count = *(pb_size_t*)field->pSize;
            for (; count > 0; count--)
            {
                pb_free(*pItem);
                *pItem++ = NULL;
            }
        }
        
        if (PB_HTYPE(type) == PB_HTYPE_REPEATED)
        {
            /* We are going to release the array, so set the size to 0 */
            *(pb_size_t*)field->pSize = 0;
        }
        
        /* Release main pointer */
        pb_free(*(void**)field->pField);
        *(void**)field->pField = NULL;
    }
}

void pb_release(const pb_msgdesc_t *fields, void *dest_struct)
{
    pb_field_iter_t iter;
    
    if (!dest_struct)
        return; /* Ignore NULL pointers, similar to free() */

    if (!pb_field_iter_begin(&iter, fields, dest_struct))
        return; /* Empty message type */
    
    do
    {
        pb_release_single_field(&iter);
    } while (pb_field_iter_next(&iter));
}
#endif

/* Field decoders */

bool pb_decode_bool(pb_istream_t *stream, bool *dest)
{
    uint32_t value;
    if (!pb_decode_varint32(stream, &value))
        return false;

    *(bool*)dest = (value != 0);
    return true;
}

bool pb_decode_svarint(pb_istream_t *stream, pb_int64_t *dest)
{
    pb_uint64_t value;
    if (!pb_decode_varint(stream, &value))
        return false;
    
    if (value & 1)
        *dest = (pb_int64_t)(~(value >> 1));
    else
        *dest = (pb_int64_t)(value >> 1);
    
    return true;
}

bool pb_decode_fixed32(pb_istream_t *stream, void *dest)
{
    union {
        uint32_t fixed32;
        pb_byte_t bytes[4];
    } u;

    if (!pb_read(stream, u.bytes, 4))
        return false;

#if defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN && CHAR_BIT == 8
    /* fast path - if we know that we're on little endian, assign directly */
    *(uint32_t*)dest = u.fixed32;
#else
    *(uint32_t*)dest = ((uint32_t)u.bytes[0] << 0) |
                       ((uint32_t)u.bytes[1] << 8) |
                       ((uint32_t)u.bytes[2] << 16) |
                       ((uint32_t)u.bytes[3] << 24);
#endif
    return true;
}

#ifndef PB_WITHOUT_64BIT
bool pb_decode_fixed64(pb_istream_t *stream, void *dest)
{
    union {
        uint64_t fixed64;
        pb_byte_t bytes[8];
    } u;

    if (!pb_read(stream, u.bytes, 8))
        return false;

#if defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN && CHAR_BIT == 8
    /* fast path - if we know that we're on little endian, assign directly */
    *(uint64_t*)dest = u.fixed64;
#else
    *(uint64_t*)dest = ((uint64_t)u.bytes[0] << 0) |
                       ((uint64_t)u.bytes[1] << 8) |
                       ((uint64_t)u.bytes[2] << 16) |
                       ((uint64_t)u.bytes[3] << 24) |
                       ((uint64_t)u.bytes[4] << 32) |
                       ((uint64_t)u.bytes[5] << 40) |
                       ((uint64_t)u.bytes[6] << 48) |
                       ((uint64_t)u.bytes[7] << 56);
#endif
    return true;
}
#endif

static bool checkreturn pb_dec_bool(pb_istream_t *stream, const pb_field_iter_t *field)
{
    return pb_decode_bool(stream, (bool*)field->pData);
}

static bool checkreturn pb_dec_varint(pb_istream_t *stream, const pb_field_iter_t *field)
{
    if (PB_LTYPE(field->type) == PB_LTYPE_UVARINT)
    {
        pb_uint64_t value, clamped;
        if (!pb_decode_varint(stream, &value))
            return false;

        /* Cast to the proper field size, while checking for overflows */
        if (field->data_size == sizeof(pb_uint64_t))
            clamped = *(pb_uint64_t*)field->pData = value;
        else if (field->data_size == sizeof(uint32_t))
            clamped = *(uint32_t*)field->pData = (uint32_t)value;
        else if (field->data_size == sizeof(uint_least16_t))
            clamped = *(uint_least16_t*)field->pData = (uint_least16_t)value;
        else if (field->data_size == sizeof(uint_least8_t))
            clamped = *(uint_least8_t*)field->pData = (uint_least8_t)value;
        else
            PB_RETURN_ERROR(stream, "invalid data_size");

        if (clamped != value)
            PB_RETURN_ERROR(stream, "integer too large");

        return true;
    }
    else
    {
        pb_uint64_t value;
        pb_int64_t svalue;
        pb_int64_t clamped;

        if (PB_LTYPE(field->type) == PB_LTYPE_SVARINT)
        {
            if (!pb_decode_svarint(stream, &svalue))
                return false;
        }
        else
        {
            if (!pb_decode_varint(stream, &value))
                return false;

            /* See issue 97: Google's C++ protobuf allows negative varint values to
            * be cast as int32_t, instead of the int64_t that should be used when
            * encoding. Nanopb versions before 0.2.5 had a bug in encoding. In order to
            * not break decoding of such messages, we cast <=32 bit fields to
            * int32_t first to get the sign correct.
            */
            if (field->data_size == sizeof(pb_int64_t))
                svalue = (pb_int64_t)value;
            else
                svalue = (int32_t)value;
        }

        /* Cast to the proper field size, while checking for overflows */
        if (field->data_size == sizeof(pb_int64_t))
            clamped = *(pb_int64_t*)field->pData = svalue;
        else if (field->data_size == sizeof(int32_t))
            clamped = *(int32_t*)field->pData = (int32_t)svalue;
        else if (field->data_size == sizeof(int_least16_t))
            clamped = *(int_least16_t*)field->pData = (int_least16_t)svalue;
        else if (field->data_size == sizeof(int_least8_t))
            clamped = *(int_least8_t*)field->pData = (int_least8_t)svalue;
        else
            PB_RETURN_ERROR(stream, "invalid data_size");

        if (clamped != svalue)
            PB_RETURN_ERROR(stream, "integer too large");

        return true;
    }
}

static bool checkreturn pb_dec_bytes(pb_istream_t *stream, const pb_field_iter_t *field)
{
    uint32_t size;
    size_t alloc_size;
    pb_bytes_array_t *dest;
    
    if (!pb_decode_varint32(stream, &size))
        return false;
    
    if (size > PB_SIZE_MAX)
        PB_RETURN_ERROR(stream, "bytes overflow");
    
    alloc_size = PB_BYTES_ARRAY_T_ALLOCSIZE(size);
    if (size > alloc_size)
        PB_RETURN_ERROR(stream, "size too large");
    
    if (PB_ATYPE(field->type) == PB_ATYPE_POINTER)
    {
#ifndef PB_ENABLE_MALLOC
        PB_RETURN_ERROR(stream, "no malloc support");
#else
        if (stream->bytes_left < size)
            PB_RETURN_ERROR(stream, "end-of-stream");

        if (!allocate_field(stream, field->pData, alloc_size, 1))
            return false;
        dest = *(pb_bytes_array_t**)field->pData;
#endif
    }
    else
    {
        if (alloc_size > field->data_size)
            PB_RETURN_ERROR(stream, "bytes overflow");
        dest = (pb_bytes_array_t*)field->pData;
    }

    dest->size = (pb_size_t)size;
    return pb_read(stream, dest->bytes, (size_t)size);
}

static bool checkreturn pb_dec_string(pb_istream_t *stream, const pb_field_iter_t *field)
{
    uint32_t size;
    size_t alloc_size;
    pb_byte_t *dest = (pb_byte_t*)field->pData;

    if (!pb_decode_varint32(stream, &size))
        return false;

    if (size == (uint32_t)-1)
        PB_RETURN_ERROR(stream, "size too large");

    /* Space for null terminator */
    alloc_size = (size_t)(size + 1);

    if (alloc_size < size)
        PB_RETURN_ERROR(stream, "size too large");

    if (PB_ATYPE(field->type) == PB_ATYPE_POINTER)
    {
#ifndef PB_ENABLE_MALLOC
        PB_RETURN_ERROR(stream, "no malloc support");
#else
        if (stream->bytes_left < size)
            PB_RETURN_ERROR(stream, "end-of-stream");

        if (!allocate_field(stream, field->pData, alloc_size, 1))
            return false;
        dest = *(pb_byte_t**)field->pData;
#endif
    }
    else
    {
        if (alloc_size > field->data_size)
            PB_RETURN_ERROR(stream, "string overflow");
    }
    
    dest[size] = 0;

    if (!pb_read(stream, dest, (size_t)size))
        return false;

#ifdef PB_VALIDATE_UTF8
    if (!pb_validate_utf8((const char*)dest))
        PB_RETURN_ERROR(stream, "invalid utf8");
#endif

    return true;
}

static bool checkreturn pb_dec_submessage(pb_istream_t *stream, const pb_field_iter_t *field)
{
    bool status = true;
    bool submsg_consumed = false;
    pb_istream_t substream;

    if (!pb_make_string_substream(stream, &substream))
        return false;
    
    if (field->submsg_desc == NULL)
        PB_RETURN_ERROR(stream, "invalid field descriptor");
    
    /* Submessages can have a separate message-level callback that is called
     * before decoding the message. Typically it is used to set callback fields
     * inside oneofs. */
    if (PB_LTYPE(field->type) == PB_LTYPE_SUBMSG_W_CB && field->pSize != NULL)
    {
        /* Message callback is stored right before pSize. */
        pb_callback_t *callback = (pb_callback_t*)field->pSize - 1;
        if (callback->funcs.decode)
        {
            status = callback->funcs.decode(&substream, field, &callback->arg);

            if (substream.bytes_left == 0)
            {
                submsg_consumed = true;
            }
        }
    }

    /* Now decode the submessage contents */
    if (status && !submsg_consumed)
    {
        unsigned int flags = 0;

        /* Static required/optional fields are already initialized by top-level
         * pb_decode(), no need to initialize them again. */
        if (PB_ATYPE(field->type) == PB_ATYPE_STATIC &&
            PB_HTYPE(field->type) != PB_HTYPE_REPEATED)
        {
            flags = PB_DECODE_NOINIT;
        }

        status = pb_decode_inner(&substream, field->submsg_desc, field->pData, flags);
    }
    
    if (!pb_close_string_substream(stream, &substream))
        return false;

    return status;
}

static bool checkreturn pb_dec_fixed_length_bytes(pb_istream_t *stream, const pb_field_iter_t *field)
{
    uint32_t size;

    if (!pb_decode_varint32(stream, &size))
        return false;

    if (size > PB_SIZE_MAX)
        PB_RETURN_ERROR(stream, "bytes overflow");

    if (size == 0)
    {
        /* As a special case, treat empty bytes string as all zeros for fixed_length_bytes. */
        memset(field->pData, 0, (size_t)field->data_size);
        return true;
    }

    if (size != field->data_size)
        PB_RETURN_ERROR(stream, "incorrect fixed length bytes size");

    return pb_read(stream, (pb_byte_t*)field->pData, (size_t)field->data_size);
}

#ifdef PB_CONVERT_DOUBLE_FLOAT
bool pb_decode_double_as_float(pb_istream_t *stream, float *dest)
{
    uint_least8_t sign;
    int exponent;
    uint32_t mantissa;
    uint64_t value;
    union { float f; uint32_t i; } out;

    if (!pb_decode_fixed64(stream, &value))
        return false;

    /* Decompose input value */
    sign = (uint_least8_t)((value >> 63) & 1);
    exponent = (int)((value >> 52) & 0x7FF) - 1023;
    mantissa = (value >> 28) & 0xFFFFFF; /* Highest 24 bits */

    /* Figure if value is in range representable by floats. */
    if (exponent == 1024)
    {
        /* Special value */
        exponent = 128;
        mantissa >>= 1;
    }
    else
    {
        if (exponent > 127)
        {
            /* Too large, convert to infinity */
            exponent = 128;
            mantissa = 0;
        }
        else if (exponent < -150)
        {
            /* Too small, convert to zero */
            exponent = -127;
            mantissa = 0;
        }
        else if (exponent < -126)
        {
            /* Denormalized */
            mantissa |= 0x1000000;
            mantissa >>= (-126 - exponent);
            exponent = -127;
        }

        /* Round off mantissa */
        mantissa = (mantissa + 1) >> 1;

        /* Check if mantissa went over 2.0 */
        if (mantissa & 0x800000)
        {
            exponent += 1;
            mantissa &= 0x7FFFFF;
            mantissa >>= 1;
        }
    }

    /* Combine fields */
    out.i = mantissa;
    out.i |= (uint32_t)(exponent + 127) << 23;
    out.i |= (uint32_t)sign << 31;

    *dest = out.f;
    return true;
}
#endif
/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.6-dev */

#ifndef PB_UTILS_PB_H_INCLUDED
#define PB_UTILS_PB_H_INCLUDED

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Struct definitions */
typedef struct _GeoCoordinate { 
    double latitude; 
    double longitude; 
} GeoCoordinate;


#ifdef __cplusplus
extern "C" {
#endif

/* Initializer values for message structs */
#define GeoCoordinate_init_default               {0, 0}
#define GeoCoordinate_init_zero                  {0, 0}

/* Field tags (for use in manual encoding/decoding) */
#define GeoCoordinate_latitude_tag               1
#define GeoCoordinate_longitude_tag              2

/* Struct field encoding specification for nanopb */
#define GeoCoordinate_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, DOUBLE,   latitude,          1) \
X(a, STATIC,   REQUIRED, DOUBLE,   longitude,         2)
#define GeoCoordinate_CALLBACK NULL
#define GeoCoordinate_DEFAULT NULL

extern const pb_msgdesc_t GeoCoordinate_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define GeoCoordinate_fields &GeoCoordinate_msg

/* Maximum encoded size of messages (where known) */
#define GeoCoordinate_size                       18

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.6-dev */

#ifndef PB_MANAGERS_PB_H_INCLUDED
#define PB_MANAGERS_PB_H_INCLUDED

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Enum definitions */
typedef enum _DataMissionManager_MissionElement_ElementType { 
    DataMissionManager_MissionElement_ElementType_WPT = 0, 
    DataMissionManager_MissionElement_ElementType_AWA = 1 
} DataMissionManager_MissionElement_ElementType;

typedef enum _DataCaptainManager_Behaviour { 
    DataCaptainManager_Behaviour_SLEEP = 0, 
    DataCaptainManager_Behaviour_ACQUISITION = 1, 
    DataCaptainManager_Behaviour_DECIDE = 2, 
    DataCaptainManager_Behaviour_PROCESS = 3, 
    DataCaptainManager_Behaviour_RADIO_CONTROLLED = 4 
} DataCaptainManager_Behaviour;

/* Struct definitions */
typedef struct _DataCommManager { 
    char dummy_field;
} DataCommManager;

typedef struct _DataConfManager { 
    char dummy_field;
} DataConfManager;

typedef struct _DataLightManager { 
    char dummy_field;
} DataLightManager;

typedef struct _ConfBaseManager { 
    int32_t interval; 
} ConfBaseManager;

typedef struct _ConfSensorManager_ConfGps { 
    double max_valid_hdop; 
} ConfSensorManager_ConfGps;

typedef struct _DataCaptainManager { /* string msg_received = 1; */
    bool just_wake_up; 
    DataCaptainManager_Behaviour behaviour; 
    float helm_order; 
    float ask_add_awa_angle; 
} DataCaptainManager;

typedef struct _DataHelmManager { 
    float angle; 
    bool cmd_applied; 
} DataHelmManager;

typedef struct _DataLogManager { 
    bool sd_ready; 
} DataLogManager;

typedef struct _DataMissionManager_MissionElement { 
    DataMissionManager_MissionElement_ElementType type; 
    GeoCoordinate coord; 
    double corridor_width; 
    double valid_dist; 
    double angle; 
    double duration; 
    bool ephemeral; 
} DataMissionManager_MissionElement;

typedef struct _DataSensorManager_DataBattery { 
    float voltage; 
} DataSensorManager_DataBattery;

typedef struct _DataSensorManager_DataGps { 
    GeoCoordinate coord; 
    float altitude; 
    bool fix; 
    int32_t fix_quality; 
    int32_t satellites; 
    uint32_t fix_age; 
    uint32_t time; 
    uint32_t date; 
    float speed; 
    float course; 
    double hdop; 
    bool recent_data; 
    float average_course; 
    bool average_course_full; 
    bool ready; 
} DataSensorManager_DataGps;

typedef struct _DataSensorManager_DataRadioControl { 
    bool radio_controlled; /* [ms] wakeup interval */
} DataSensorManager_DataRadioControl;

typedef struct _DataWingManager { 
    float angle; 
} DataWingManager;

typedef struct _ConfCaptainManager { 
    ConfBaseManager base; 
    float max_upwind; 
    float max_downwind; 
    float sleeping_duration; 
    float too_far_for_tack; 
} ConfCaptainManager;

typedef struct _ConfCommManager { 
    ConfBaseManager base; 
} ConfCommManager;

typedef struct _ConfConfManager { 
    ConfBaseManager base; 
} ConfConfManager;

typedef struct _ConfHelmManager { 
    ConfBaseManager base; 
    float ratio; 
    float offset; 
    float normal_speed; 
    float tack_speed; 
} ConfHelmManager;

typedef struct _ConfLightManager { 
    ConfBaseManager base; 
} ConfLightManager;

typedef struct _ConfLogManager { 
    ConfBaseManager base; 
} ConfLogManager;

typedef struct _ConfMissionManager { 
    ConfBaseManager base; 
    float default_corridor_width; 
    float default_validation_distance; 
    float start_auto_angle; 
    float start_auto_duration; 
    float awa_tack_duration; 
} ConfMissionManager;

typedef struct _ConfSensorManager { 
    ConfBaseManager base; 
    ConfSensorManager_ConfGps gps; 
} ConfSensorManager;

typedef struct _ConfWingManager { 
    ConfBaseManager base; 
    float step; 
    float calib_offset; 
} ConfWingManager;

typedef struct _DataMissionManager { 
    float wpt_dist; 
    float wpt_angle; 
    int32_t element_index; 
    float dist_to_axis; 
    bool in_corridor; 
    float corridor_angle; 
    DataMissionManager_MissionElement prev_element; 
    DataMissionManager_MissionElement next_element; 
    bool setpoint_update_asked; 
} DataMissionManager;

typedef struct _DataSensorManager { 
    DataSensorManager_DataGps gps; 
    DataSensorManager_DataBattery battery; 
    DataSensorManager_DataRadioControl radio; 
} DataSensorManager;

typedef struct _Conf { 
    ConfMissionManager mission_manager; 
    ConfLogManager log_manager; 
    ConfHelmManager helm_manager; 
    ConfWingManager wing_manager; 
    ConfCaptainManager captain_manager; 
    ConfCommManager comm_manager; 
    ConfSensorManager sensor_manager; 
    ConfLightManager light_manager; 
    ConfConfManager config_manager; 
} Conf;

typedef struct _Data { 
    DataMissionManager mission_manager; 
    DataLogManager log_manager; 
    DataHelmManager helm_manager; 
    DataWingManager wing_manager; 
    DataCaptainManager captain_manager; 
    DataCommManager comm_manager; 
    DataSensorManager sensor_manager; 
    DataLightManager light_manager; 
    DataConfManager config_manager; 
} Data;


/* Helper constants for enums */
#define _DataMissionManager_MissionElement_ElementType_MIN DataMissionManager_MissionElement_ElementType_WPT
#define _DataMissionManager_MissionElement_ElementType_MAX DataMissionManager_MissionElement_ElementType_AWA
#define _DataMissionManager_MissionElement_ElementType_ARRAYSIZE ((DataMissionManager_MissionElement_ElementType)(DataMissionManager_MissionElement_ElementType_AWA+1))

#define _DataCaptainManager_Behaviour_MIN DataCaptainManager_Behaviour_SLEEP
#define _DataCaptainManager_Behaviour_MAX DataCaptainManager_Behaviour_RADIO_CONTROLLED
#define _DataCaptainManager_Behaviour_ARRAYSIZE ((DataCaptainManager_Behaviour)(DataCaptainManager_Behaviour_RADIO_CONTROLLED+1))


#ifdef __cplusplus
extern "C" {
#endif

/* Initializer values for message structs */
#define Data_init_default                        {DataMissionManager_init_default, DataLogManager_init_default, DataHelmManager_init_default, DataWingManager_init_default, DataCaptainManager_init_default, DataCommManager_init_default, DataSensorManager_init_default, DataLightManager_init_default, DataConfManager_init_default}
#define DataMissionManager_init_default          {0, 0, 0, 0, 0, 0, DataMissionManager_MissionElement_init_default, DataMissionManager_MissionElement_init_default, 0}
#define DataMissionManager_MissionElement_init_default {_DataMissionManager_MissionElement_ElementType_MIN, GeoCoordinate_init_default, 0, 0, 0, 0, 0}
#define DataLogManager_init_default              {0}
#define DataHelmManager_init_default             {0, 0}
#define DataWingManager_init_default             {0}
#define DataCaptainManager_init_default          {0, _DataCaptainManager_Behaviour_MIN, 0, 0}
#define DataCommManager_init_default             {0}
#define DataSensorManager_init_default           {DataSensorManager_DataGps_init_default, DataSensorManager_DataBattery_init_default, DataSensorManager_DataRadioControl_init_default}
#define DataSensorManager_DataGps_init_default   {GeoCoordinate_init_default, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
#define DataSensorManager_DataBattery_init_default {0}
#define DataSensorManager_DataRadioControl_init_default {0}
#define DataLightManager_init_default            {0}
#define DataConfManager_init_default             {0}
#define Conf_init_default                        {ConfMissionManager_init_default, ConfLogManager_init_default, ConfHelmManager_init_default, ConfWingManager_init_default, ConfCaptainManager_init_default, ConfCommManager_init_default, ConfSensorManager_init_default, ConfLightManager_init_default, ConfConfManager_init_default}
#define ConfBaseManager_init_default             {0}
#define ConfMissionManager_init_default          {ConfBaseManager_init_default, 0, 0, 0, 0, 0}
#define ConfLogManager_init_default              {ConfBaseManager_init_default}
#define ConfHelmManager_init_default             {ConfBaseManager_init_default, 0, 0, 0, 0}
#define ConfWingManager_init_default             {ConfBaseManager_init_default, 0, 0}
#define ConfCaptainManager_init_default          {ConfBaseManager_init_default, 0, 0, 0, 0}
#define ConfCommManager_init_default             {ConfBaseManager_init_default}
#define ConfSensorManager_init_default           {ConfBaseManager_init_default, ConfSensorManager_ConfGps_init_default}
#define ConfSensorManager_ConfGps_init_default   {0}
#define ConfLightManager_init_default            {ConfBaseManager_init_default}
#define ConfConfManager_init_default             {ConfBaseManager_init_default}
#define Data_init_zero                           {DataMissionManager_init_zero, DataLogManager_init_zero, DataHelmManager_init_zero, DataWingManager_init_zero, DataCaptainManager_init_zero, DataCommManager_init_zero, DataSensorManager_init_zero, DataLightManager_init_zero, DataConfManager_init_zero}
#define DataMissionManager_init_zero             {0, 0, 0, 0, 0, 0, DataMissionManager_MissionElement_init_zero, DataMissionManager_MissionElement_init_zero, 0}
#define DataMissionManager_MissionElement_init_zero {_DataMissionManager_MissionElement_ElementType_MIN, GeoCoordinate_init_zero, 0, 0, 0, 0, 0}
#define DataLogManager_init_zero                 {0}
#define DataHelmManager_init_zero                {0, 0}
#define DataWingManager_init_zero                {0}
#define DataCaptainManager_init_zero             {0, _DataCaptainManager_Behaviour_MIN, 0, 0}
#define DataCommManager_init_zero                {0}
#define DataSensorManager_init_zero              {DataSensorManager_DataGps_init_zero, DataSensorManager_DataBattery_init_zero, DataSensorManager_DataRadioControl_init_zero}
#define DataSensorManager_DataGps_init_zero      {GeoCoordinate_init_zero, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
#define DataSensorManager_DataBattery_init_zero  {0}
#define DataSensorManager_DataRadioControl_init_zero {0}
#define DataLightManager_init_zero               {0}
#define DataConfManager_init_zero                {0}
#define Conf_init_zero                           {ConfMissionManager_init_zero, ConfLogManager_init_zero, ConfHelmManager_init_zero, ConfWingManager_init_zero, ConfCaptainManager_init_zero, ConfCommManager_init_zero, ConfSensorManager_init_zero, ConfLightManager_init_zero, ConfConfManager_init_zero}
#define ConfBaseManager_init_zero                {0}
#define ConfMissionManager_init_zero             {ConfBaseManager_init_zero, 0, 0, 0, 0, 0}
#define ConfLogManager_init_zero                 {ConfBaseManager_init_zero}
#define ConfHelmManager_init_zero                {ConfBaseManager_init_zero, 0, 0, 0, 0}
#define ConfWingManager_init_zero                {ConfBaseManager_init_zero, 0, 0}
#define ConfCaptainManager_init_zero             {ConfBaseManager_init_zero, 0, 0, 0, 0}
#define ConfCommManager_init_zero                {ConfBaseManager_init_zero}
#define ConfSensorManager_init_zero              {ConfBaseManager_init_zero, ConfSensorManager_ConfGps_init_zero}
#define ConfSensorManager_ConfGps_init_zero      {0}
#define ConfLightManager_init_zero               {ConfBaseManager_init_zero}
#define ConfConfManager_init_zero                {ConfBaseManager_init_zero}

/* Field tags (for use in manual encoding/decoding) */
#define ConfBaseManager_interval_tag             1
#define ConfSensorManager_ConfGps_max_valid_hdop_tag 1
#define DataCaptainManager_just_wake_up_tag      1
#define DataCaptainManager_behaviour_tag         2
#define DataCaptainManager_helm_order_tag        3
#define DataCaptainManager_ask_add_awa_angle_tag 4
#define DataHelmManager_angle_tag                1
#define DataHelmManager_cmd_applied_tag          2
#define DataLogManager_sd_ready_tag              1
#define DataMissionManager_MissionElement_type_tag 1
#define DataMissionManager_MissionElement_coord_tag 2
#define DataMissionManager_MissionElement_corridor_width_tag 3
#define DataMissionManager_MissionElement_valid_dist_tag 4
#define DataMissionManager_MissionElement_angle_tag 5
#define DataMissionManager_MissionElement_duration_tag 6
#define DataMissionManager_MissionElement_ephemeral_tag 7
#define DataSensorManager_DataBattery_voltage_tag 1
#define DataSensorManager_DataGps_coord_tag      1
#define DataSensorManager_DataGps_altitude_tag   2
#define DataSensorManager_DataGps_fix_tag        3
#define DataSensorManager_DataGps_fix_quality_tag 4
#define DataSensorManager_DataGps_satellites_tag 5
#define DataSensorManager_DataGps_fix_age_tag    6
#define DataSensorManager_DataGps_time_tag       7
#define DataSensorManager_DataGps_date_tag       8
#define DataSensorManager_DataGps_speed_tag      9
#define DataSensorManager_DataGps_course_tag     10
#define DataSensorManager_DataGps_hdop_tag       11
#define DataSensorManager_DataGps_recent_data_tag 50
#define DataSensorManager_DataGps_average_course_tag 51
#define DataSensorManager_DataGps_average_course_full_tag 52
#define DataSensorManager_DataGps_ready_tag      53
#define DataSensorManager_DataRadioControl_radio_controlled_tag 1
#define DataWingManager_angle_tag                1
#define ConfCaptainManager_base_tag              1
#define ConfCaptainManager_max_upwind_tag        2
#define ConfCaptainManager_max_downwind_tag      3
#define ConfCaptainManager_sleeping_duration_tag 4
#define ConfCaptainManager_too_far_for_tack_tag  5
#define ConfCommManager_base_tag                 1
#define ConfConfManager_base_tag                 1
#define ConfHelmManager_base_tag                 1
#define ConfHelmManager_ratio_tag                2
#define ConfHelmManager_offset_tag               3
#define ConfHelmManager_normal_speed_tag         4
#define ConfHelmManager_tack_speed_tag           5
#define ConfLightManager_base_tag                1
#define ConfLogManager_base_tag                  1
#define ConfMissionManager_base_tag              1
#define ConfMissionManager_default_corridor_width_tag 2
#define ConfMissionManager_default_validation_distance_tag 3
#define ConfMissionManager_start_auto_angle_tag  4
#define ConfMissionManager_start_auto_duration_tag 5
#define ConfMissionManager_awa_tack_duration_tag 6
#define ConfSensorManager_base_tag               1
#define ConfSensorManager_gps_tag                2
#define ConfWingManager_base_tag                 1
#define ConfWingManager_step_tag                 2
#define ConfWingManager_calib_offset_tag         3
#define DataMissionManager_wpt_dist_tag          1
#define DataMissionManager_wpt_angle_tag         2
#define DataMissionManager_element_index_tag     3
#define DataMissionManager_dist_to_axis_tag      4
#define DataMissionManager_in_corridor_tag       5
#define DataMissionManager_corridor_angle_tag    6
#define DataMissionManager_prev_element_tag      7
#define DataMissionManager_next_element_tag      8
#define DataMissionManager_setpoint_update_asked_tag 9
#define DataSensorManager_gps_tag                1
#define DataSensorManager_battery_tag            2
#define DataSensorManager_radio_tag              3
#define Conf_mission_manager_tag                 1
#define Conf_log_manager_tag                     2
#define Conf_helm_manager_tag                    3
#define Conf_wing_manager_tag                    4
#define Conf_captain_manager_tag                 5
#define Conf_comm_manager_tag                    6
#define Conf_sensor_manager_tag                  7
#define Conf_light_manager_tag                   8
#define Conf_config_manager_tag                  9
#define Data_mission_manager_tag                 1
#define Data_log_manager_tag                     2
#define Data_helm_manager_tag                    3
#define Data_wing_manager_tag                    4
#define Data_captain_manager_tag                 5
#define Data_comm_manager_tag                    6
#define Data_sensor_manager_tag                  7
#define Data_light_manager_tag                   8
#define Data_config_manager_tag                  9

/* Struct field encoding specification for nanopb */
#define Data_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, MESSAGE,  mission_manager,   1) \
X(a, STATIC,   REQUIRED, MESSAGE,  log_manager,       2) \
X(a, STATIC,   REQUIRED, MESSAGE,  helm_manager,      3) \
X(a, STATIC,   REQUIRED, MESSAGE,  wing_manager,      4) \
X(a, STATIC,   REQUIRED, MESSAGE,  captain_manager,   5) \
X(a, STATIC,   REQUIRED, MESSAGE,  comm_manager,      6) \
X(a, STATIC,   REQUIRED, MESSAGE,  sensor_manager,    7) \
X(a, STATIC,   REQUIRED, MESSAGE,  light_manager,     8) \
X(a, STATIC,   REQUIRED, MESSAGE,  config_manager,    9)
#define Data_CALLBACK NULL
#define Data_DEFAULT NULL
#define Data_mission_manager_MSGTYPE DataMissionManager
#define Data_log_manager_MSGTYPE DataLogManager
#define Data_helm_manager_MSGTYPE DataHelmManager
#define Data_wing_manager_MSGTYPE DataWingManager
#define Data_captain_manager_MSGTYPE DataCaptainManager
#define Data_comm_manager_MSGTYPE DataCommManager
#define Data_sensor_manager_MSGTYPE DataSensorManager
#define Data_light_manager_MSGTYPE DataLightManager
#define Data_config_manager_MSGTYPE DataConfManager

#define DataMissionManager_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, FLOAT,    wpt_dist,          1) \
X(a, STATIC,   REQUIRED, FLOAT,    wpt_angle,         2) \
X(a, STATIC,   REQUIRED, INT32,    element_index,     3) \
X(a, STATIC,   REQUIRED, FLOAT,    dist_to_axis,      4) \
X(a, STATIC,   REQUIRED, BOOL,     in_corridor,       5) \
X(a, STATIC,   REQUIRED, FLOAT,    corridor_angle,    6) \
X(a, STATIC,   REQUIRED, MESSAGE,  prev_element,      7) \
X(a, STATIC,   REQUIRED, MESSAGE,  next_element,      8) \
X(a, STATIC,   REQUIRED, BOOL,     setpoint_update_asked,   9)
#define DataMissionManager_CALLBACK NULL
#define DataMissionManager_DEFAULT NULL
#define DataMissionManager_prev_element_MSGTYPE DataMissionManager_MissionElement
#define DataMissionManager_next_element_MSGTYPE DataMissionManager_MissionElement

#define DataMissionManager_MissionElement_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, UENUM,    type,              1) \
X(a, STATIC,   REQUIRED, MESSAGE,  coord,             2) \
X(a, STATIC,   REQUIRED, DOUBLE,   corridor_width,    3) \
X(a, STATIC,   REQUIRED, DOUBLE,   valid_dist,        4) \
X(a, STATIC,   REQUIRED, DOUBLE,   angle,             5) \
X(a, STATIC,   REQUIRED, DOUBLE,   duration,          6) \
X(a, STATIC,   REQUIRED, BOOL,     ephemeral,         7)
#define DataMissionManager_MissionElement_CALLBACK NULL
#define DataMissionManager_MissionElement_DEFAULT NULL
#define DataMissionManager_MissionElement_coord_MSGTYPE GeoCoordinate

#define DataLogManager_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, BOOL,     sd_ready,          1)
#define DataLogManager_CALLBACK NULL
#define DataLogManager_DEFAULT NULL

#define DataHelmManager_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, FLOAT,    angle,             1) \
X(a, STATIC,   REQUIRED, BOOL,     cmd_applied,       2)
#define DataHelmManager_CALLBACK NULL
#define DataHelmManager_DEFAULT NULL

#define DataWingManager_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, FLOAT,    angle,             1)
#define DataWingManager_CALLBACK NULL
#define DataWingManager_DEFAULT NULL

#define DataCaptainManager_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, BOOL,     just_wake_up,      1) \
X(a, STATIC,   REQUIRED, UENUM,    behaviour,         2) \
X(a, STATIC,   REQUIRED, FLOAT,    helm_order,        3) \
X(a, STATIC,   REQUIRED, FLOAT,    ask_add_awa_angle,   4)
#define DataCaptainManager_CALLBACK NULL
#define DataCaptainManager_DEFAULT NULL

#define DataCommManager_FIELDLIST(X, a) \

#define DataCommManager_CALLBACK NULL
#define DataCommManager_DEFAULT NULL

#define DataSensorManager_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, MESSAGE,  gps,               1) \
X(a, STATIC,   REQUIRED, MESSAGE,  battery,           2) \
X(a, STATIC,   REQUIRED, MESSAGE,  radio,             3)
#define DataSensorManager_CALLBACK NULL
#define DataSensorManager_DEFAULT NULL
#define DataSensorManager_gps_MSGTYPE DataSensorManager_DataGps
#define DataSensorManager_battery_MSGTYPE DataSensorManager_DataBattery
#define DataSensorManager_radio_MSGTYPE DataSensorManager_DataRadioControl

#define DataSensorManager_DataGps_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, MESSAGE,  coord,             1) \
X(a, STATIC,   REQUIRED, FLOAT,    altitude,          2) \
X(a, STATIC,   REQUIRED, BOOL,     fix,               3) \
X(a, STATIC,   REQUIRED, INT32,    fix_quality,       4) \
X(a, STATIC,   REQUIRED, INT32,    satellites,        5) \
X(a, STATIC,   REQUIRED, UINT32,   fix_age,           6) \
X(a, STATIC,   REQUIRED, UINT32,   time,              7) \
X(a, STATIC,   REQUIRED, UINT32,   date,              8) \
X(a, STATIC,   REQUIRED, FLOAT,    speed,             9) \
X(a, STATIC,   REQUIRED, FLOAT,    course,           10) \
X(a, STATIC,   REQUIRED, DOUBLE,   hdop,             11) \
X(a, STATIC,   REQUIRED, BOOL,     recent_data,      50) \
X(a, STATIC,   REQUIRED, FLOAT,    average_course,   51) \
X(a, STATIC,   REQUIRED, BOOL,     average_course_full,  52) \
X(a, STATIC,   REQUIRED, BOOL,     ready,            53)
#define DataSensorManager_DataGps_CALLBACK NULL
#define DataSensorManager_DataGps_DEFAULT NULL
#define DataSensorManager_DataGps_coord_MSGTYPE GeoCoordinate

#define DataSensorManager_DataBattery_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, FLOAT,    voltage,           1)
#define DataSensorManager_DataBattery_CALLBACK NULL
#define DataSensorManager_DataBattery_DEFAULT NULL

#define DataSensorManager_DataRadioControl_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, BOOL,     radio_controlled,   1)
#define DataSensorManager_DataRadioControl_CALLBACK NULL
#define DataSensorManager_DataRadioControl_DEFAULT NULL

#define DataLightManager_FIELDLIST(X, a) \

#define DataLightManager_CALLBACK NULL
#define DataLightManager_DEFAULT NULL

#define DataConfManager_FIELDLIST(X, a) \

#define DataConfManager_CALLBACK NULL
#define DataConfManager_DEFAULT NULL

#define Conf_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, MESSAGE,  mission_manager,   1) \
X(a, STATIC,   REQUIRED, MESSAGE,  log_manager,       2) \
X(a, STATIC,   REQUIRED, MESSAGE,  helm_manager,      3) \
X(a, STATIC,   REQUIRED, MESSAGE,  wing_manager,      4) \
X(a, STATIC,   REQUIRED, MESSAGE,  captain_manager,   5) \
X(a, STATIC,   REQUIRED, MESSAGE,  comm_manager,      6) \
X(a, STATIC,   REQUIRED, MESSAGE,  sensor_manager,    7) \
X(a, STATIC,   REQUIRED, MESSAGE,  light_manager,     8) \
X(a, STATIC,   REQUIRED, MESSAGE,  config_manager,    9)
#define Conf_CALLBACK NULL
#define Conf_DEFAULT NULL
#define Conf_mission_manager_MSGTYPE ConfMissionManager
#define Conf_log_manager_MSGTYPE ConfLogManager
#define Conf_helm_manager_MSGTYPE ConfHelmManager
#define Conf_wing_manager_MSGTYPE ConfWingManager
#define Conf_captain_manager_MSGTYPE ConfCaptainManager
#define Conf_comm_manager_MSGTYPE ConfCommManager
#define Conf_sensor_manager_MSGTYPE ConfSensorManager
#define Conf_light_manager_MSGTYPE ConfLightManager
#define Conf_config_manager_MSGTYPE ConfConfManager

#define ConfBaseManager_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, INT32,    interval,          1)
#define ConfBaseManager_CALLBACK NULL
#define ConfBaseManager_DEFAULT NULL

#define ConfMissionManager_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, MESSAGE,  base,              1) \
X(a, STATIC,   REQUIRED, FLOAT,    default_corridor_width,   2) \
X(a, STATIC,   REQUIRED, FLOAT,    default_validation_distance,   3) \
X(a, STATIC,   REQUIRED, FLOAT,    start_auto_angle,   4) \
X(a, STATIC,   REQUIRED, FLOAT,    start_auto_duration,   5) \
X(a, STATIC,   REQUIRED, FLOAT,    awa_tack_duration,   6)
#define ConfMissionManager_CALLBACK NULL
#define ConfMissionManager_DEFAULT NULL
#define ConfMissionManager_base_MSGTYPE ConfBaseManager

#define ConfLogManager_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, MESSAGE,  base,              1)
#define ConfLogManager_CALLBACK NULL
#define ConfLogManager_DEFAULT NULL
#define ConfLogManager_base_MSGTYPE ConfBaseManager

#define ConfHelmManager_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, MESSAGE,  base,              1) \
X(a, STATIC,   REQUIRED, FLOAT,    ratio,             2) \
X(a, STATIC,   REQUIRED, FLOAT,    offset,            3) \
X(a, STATIC,   REQUIRED, FLOAT,    normal_speed,      4) \
X(a, STATIC,   REQUIRED, FLOAT,    tack_speed,        5)
#define ConfHelmManager_CALLBACK NULL
#define ConfHelmManager_DEFAULT NULL
#define ConfHelmManager_base_MSGTYPE ConfBaseManager

#define ConfWingManager_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, MESSAGE,  base,              1) \
X(a, STATIC,   REQUIRED, FLOAT,    step,              2) \
X(a, STATIC,   REQUIRED, FLOAT,    calib_offset,      3)
#define ConfWingManager_CALLBACK NULL
#define ConfWingManager_DEFAULT NULL
#define ConfWingManager_base_MSGTYPE ConfBaseManager

#define ConfCaptainManager_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, MESSAGE,  base,              1) \
X(a, STATIC,   REQUIRED, FLOAT,    max_upwind,        2) \
X(a, STATIC,   REQUIRED, FLOAT,    max_downwind,      3) \
X(a, STATIC,   REQUIRED, FLOAT,    sleeping_duration,   4) \
X(a, STATIC,   REQUIRED, FLOAT,    too_far_for_tack,   5)
#define ConfCaptainManager_CALLBACK NULL
#define ConfCaptainManager_DEFAULT NULL
#define ConfCaptainManager_base_MSGTYPE ConfBaseManager

#define ConfCommManager_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, MESSAGE,  base,              1)
#define ConfCommManager_CALLBACK NULL
#define ConfCommManager_DEFAULT NULL
#define ConfCommManager_base_MSGTYPE ConfBaseManager

#define ConfSensorManager_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, MESSAGE,  base,              1) \
X(a, STATIC,   REQUIRED, MESSAGE,  gps,               2)
#define ConfSensorManager_CALLBACK NULL
#define ConfSensorManager_DEFAULT NULL
#define ConfSensorManager_base_MSGTYPE ConfBaseManager
#define ConfSensorManager_gps_MSGTYPE ConfSensorManager_ConfGps

#define ConfSensorManager_ConfGps_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, DOUBLE,   max_valid_hdop,    1)
#define ConfSensorManager_ConfGps_CALLBACK NULL
#define ConfSensorManager_ConfGps_DEFAULT NULL

#define ConfLightManager_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, MESSAGE,  base,              1)
#define ConfLightManager_CALLBACK NULL
#define ConfLightManager_DEFAULT NULL
#define ConfLightManager_base_MSGTYPE ConfBaseManager

#define ConfConfManager_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, MESSAGE,  base,              1)
#define ConfConfManager_CALLBACK NULL
#define ConfConfManager_DEFAULT NULL
#define ConfConfManager_base_MSGTYPE ConfBaseManager

extern const pb_msgdesc_t Data_msg;
extern const pb_msgdesc_t DataMissionManager_msg;
extern const pb_msgdesc_t DataMissionManager_MissionElement_msg;
extern const pb_msgdesc_t DataLogManager_msg;
extern const pb_msgdesc_t DataHelmManager_msg;
extern const pb_msgdesc_t DataWingManager_msg;
extern const pb_msgdesc_t DataCaptainManager_msg;
extern const pb_msgdesc_t DataCommManager_msg;
extern const pb_msgdesc_t DataSensorManager_msg;
extern const pb_msgdesc_t DataSensorManager_DataGps_msg;
extern const pb_msgdesc_t DataSensorManager_DataBattery_msg;
extern const pb_msgdesc_t DataSensorManager_DataRadioControl_msg;
extern const pb_msgdesc_t DataLightManager_msg;
extern const pb_msgdesc_t DataConfManager_msg;
extern const pb_msgdesc_t Conf_msg;
extern const pb_msgdesc_t ConfBaseManager_msg;
extern const pb_msgdesc_t ConfMissionManager_msg;
extern const pb_msgdesc_t ConfLogManager_msg;
extern const pb_msgdesc_t ConfHelmManager_msg;
extern const pb_msgdesc_t ConfWingManager_msg;
extern const pb_msgdesc_t ConfCaptainManager_msg;
extern const pb_msgdesc_t ConfCommManager_msg;
extern const pb_msgdesc_t ConfSensorManager_msg;
extern const pb_msgdesc_t ConfSensorManager_ConfGps_msg;
extern const pb_msgdesc_t ConfLightManager_msg;
extern const pb_msgdesc_t ConfConfManager_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define Data_fields &Data_msg
#define DataMissionManager_fields &DataMissionManager_msg
#define DataMissionManager_MissionElement_fields &DataMissionManager_MissionElement_msg
#define DataLogManager_fields &DataLogManager_msg
#define DataHelmManager_fields &DataHelmManager_msg
#define DataWingManager_fields &DataWingManager_msg
#define DataCaptainManager_fields &DataCaptainManager_msg
#define DataCommManager_fields &DataCommManager_msg
#define DataSensorManager_fields &DataSensorManager_msg
#define DataSensorManager_DataGps_fields &DataSensorManager_DataGps_msg
#define DataSensorManager_DataBattery_fields &DataSensorManager_DataBattery_msg
#define DataSensorManager_DataRadioControl_fields &DataSensorManager_DataRadioControl_msg
#define DataLightManager_fields &DataLightManager_msg
#define DataConfManager_fields &DataConfManager_msg
#define Conf_fields &Conf_msg
#define ConfBaseManager_fields &ConfBaseManager_msg
#define ConfMissionManager_fields &ConfMissionManager_msg
#define ConfLogManager_fields &ConfLogManager_msg
#define ConfHelmManager_fields &ConfHelmManager_msg
#define ConfWingManager_fields &ConfWingManager_msg
#define ConfCaptainManager_fields &ConfCaptainManager_msg
#define ConfCommManager_fields &ConfCommManager_msg
#define ConfSensorManager_fields &ConfSensorManager_msg
#define ConfSensorManager_ConfGps_fields &ConfSensorManager_ConfGps_msg
#define ConfLightManager_fields &ConfLightManager_msg
#define ConfConfManager_fields &ConfConfManager_msg

/* Maximum encoded size of messages (where known) */
#define ConfBaseManager_size                     11
#define ConfCaptainManager_size                  33
#define ConfCommManager_size                     13
#define ConfConfManager_size                     13
#define ConfHelmManager_size                     33
#define ConfLightManager_size                    13
#define ConfLogManager_size                      13
#define ConfMissionManager_size                  38
#define ConfSensorManager_ConfGps_size           9
#define ConfSensorManager_size                   24
#define ConfWingManager_size                     23
#define Conf_size                                221
#define DataCaptainManager_size                  14
#define DataCommManager_size                     0
#define DataConfManager_size                     0
#define DataHelmManager_size                     7
#define DataLightManager_size                    0
#define DataLogManager_size                      2
#define DataMissionManager_MissionElement_size   60
#define DataMissionManager_size                  159
#define DataSensorManager_DataBattery_size       5
#define DataSensorManager_DataGps_size           101
#define DataSensorManager_DataRadioControl_size  2
#define DataSensorManager_size                   114
#define DataWingManager_size                     5
#define Data_size                                320

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
/* Automatically generated nanopb constant definitions */
/* Generated by nanopb-0.4.6-dev */

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

PB_BIND(GeoCoordinate, GeoCoordinate, AUTO)



#ifndef PB_CONVERT_DOUBLE_FLOAT
/* On some platforms (such as AVR), double is really float.
 * To be able to encode/decode double on these platforms, you need.
 * to define PB_CONVERT_DOUBLE_FLOAT in pb.h or compiler command line.
 */
PB_STATIC_ASSERT(sizeof(double) == 8, DOUBLE_MUST_BE_8_BYTES)
#endif

/* Automatically generated nanopb constant definitions */
/* Generated by nanopb-0.4.6-dev */

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

PB_BIND(Data, Data, 2)


PB_BIND(DataMissionManager, DataMissionManager, AUTO)


PB_BIND(DataMissionManager_MissionElement, DataMissionManager_MissionElement, AUTO)


PB_BIND(DataLogManager, DataLogManager, AUTO)


PB_BIND(DataHelmManager, DataHelmManager, AUTO)


PB_BIND(DataWingManager, DataWingManager, AUTO)


PB_BIND(DataCaptainManager, DataCaptainManager, AUTO)


PB_BIND(DataCommManager, DataCommManager, AUTO)


PB_BIND(DataSensorManager, DataSensorManager, AUTO)


PB_BIND(DataSensorManager_DataGps, DataSensorManager_DataGps, AUTO)


PB_BIND(DataSensorManager_DataBattery, DataSensorManager_DataBattery, AUTO)


PB_BIND(DataSensorManager_DataRadioControl, DataSensorManager_DataRadioControl, AUTO)


PB_BIND(DataLightManager, DataLightManager, AUTO)


PB_BIND(DataConfManager, DataConfManager, AUTO)


PB_BIND(Conf, Conf, AUTO)


PB_BIND(ConfBaseManager, ConfBaseManager, AUTO)


PB_BIND(ConfMissionManager, ConfMissionManager, AUTO)


PB_BIND(ConfLogManager, ConfLogManager, AUTO)


PB_BIND(ConfHelmManager, ConfHelmManager, AUTO)


PB_BIND(ConfWingManager, ConfWingManager, AUTO)


PB_BIND(ConfCaptainManager, ConfCaptainManager, AUTO)


PB_BIND(ConfCommManager, ConfCommManager, AUTO)


PB_BIND(ConfSensorManager, ConfSensorManager, AUTO)


PB_BIND(ConfSensorManager_ConfGps, ConfSensorManager_ConfGps, AUTO)


PB_BIND(ConfLightManager, ConfLightManager, AUTO)


PB_BIND(ConfConfManager, ConfConfManager, AUTO)





#ifndef PB_CONVERT_DOUBLE_FLOAT
/* On some platforms (such as AVR), double is really float.
 * To be able to encode/decode double on these platforms, you need.
 * to define PB_CONVERT_DOUBLE_FLOAT in pb.h or compiler command line.
 */
PB_STATIC_ASSERT(sizeof(double) == 8, DOUBLE_MUST_BE_8_BYTES)
#endif

