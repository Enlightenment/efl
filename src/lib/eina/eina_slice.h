/* EINA - EFL data type library
 * Copyright (C) 2016 ProFUSION embedded systems
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _EINA_SLICE_H
#define _EINA_SLICE_H

#include "eina_types.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>

/**
 * @addtogroup Eina_Slice_Group Memory Slices
 *
 * @brief These functions provide memory slices in read-only and
 * read-write forms.
 *
 * Memory slices define a contiguous linear memory starting at a given
 * pointer (@c mem) and spanning for a given length (@c len).
 *
 * They may be read-only (Eina_Slice) or read-write (Eina_Rw_Slice).
 *
 * @since 1.19
 */

/**
 * @addtogroup Eina_Data_Types_Group Data Types
 *
 * @{
 */

/**
 * @defgroup Eina_Slice_Group Memory Slices
 *
 * @{
 */

/**
 * @typedef Eina_Slice
 * Defines a read-only memory region.
 *
 * The slice is a memory starting at @c mem and accessible up to @c
 * len bytes.
 *
 * @see Eina_Rw_Slice for read-write memory regions.
 *
 * @since 1.19
 */
typedef struct _Eina_Slice Eina_Slice;

/**
 * @typedef Eina_Rw_Slice
 * Defines a read-and-write able memory region.
 *
 * The slice is a memory starting at @c mem and accessible up to @c
 * len bytes.
 *
 * @see Eina_Slice for read-only memory regions.
 *
 * @since 1.19
 */
typedef struct _Eina_Rw_Slice Eina_Rw_Slice;

/**
 * @struct _Eina_Slice
 * Defines a read-only memory region.
 *
 * The slice is a memory starting at @c mem and accessible up to @c
 * len bytes.
 *
 * @see Eina_Rw_Slice for read-write memory regions.
 *
 * @since 1.19
 */
struct _Eina_Slice
{
   size_t len; /**< size of memory pointed by @c mem */
   union {
      const void *mem; /**< memory pointed by this slice. Just read, never modify it. */
      const uint8_t *bytes; /**< memory as uint8_t pointer */
   };
};

/**
 * @struct _Eina_Rw_Slice
 * Defines a read-and-write able memory region.
 *
 * The slice is a memory starting at @c mem and accessible up to @c
 * len bytes.
 *
 * @see Eina_Slice for read-only memory regions.
 *
 * @since 1.19
 */
struct _Eina_Rw_Slice
{
   size_t len; /**< size of memory pointed by @c mem */
   union {
      void *mem; /**< memory pointed by this slice. It's write able. */
      uint8_t *bytes; /**< memory as uint8_t pointer */
   };
};

/**
 * @brief Convert the Read-write slice to read-only.
 *
 * @param[in] rw_slice the read-write slice to convert.
 * @return the red-only slice matching the slice.
 */
static inline Eina_Slice eina_rw_slice_slice_get(const Eina_Rw_Slice rw_slice);

/**
 * @brief Creates a duplicate of slice's memory.
 *
 * @param[in] slice the input to duplicate
 * @return a new read-write slice with new @c mem that matches @a slice
 *         contents. The new @c mem is allocated with malloc() and must
 *         be released with free().
 *
 * @see eina_rw_slice_copy()
 * @see eina_rw_slice_dup()
 *
 * @since 1.19
 */
static inline Eina_Rw_Slice eina_slice_dup(const Eina_Slice slice) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Creates a duplicate of slice's memory.
 *
 * @param[in] rw_slice the input to duplicate
 * @return a new read-write slice with new @c mem that matches @a slice
 *         contents. The new @c mem is allocated with malloc() and must
 *         be released with free().
 *
 * @see eina_rw_slice_copy()
 * @see eina_slice_dup()
 *
 * @since 1.19
 */
static inline Eina_Rw_Slice eina_rw_slice_dup(const Eina_Rw_Slice rw_slice) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Compare two slices, similar to memcmp()
 *
 * @param[in] a the first slice to compare.
 * @param[in] b the second slice to compare.
 * @return 0 if equal, < 0 if a < b, > 0 if a > b
 *
 * @since 1.19
 */
static inline int eina_slice_compare(const Eina_Slice a, const Eina_Slice b);

/**
 * @brief Compare two slices, similar to memcmp()
 *
 * @param[in] a the first slice to compare.
 * @param[in] b the second slice to compare.
 * @return 0 if equal, < 0 if a < b, > 0 if a > b
 *
 * @since 1.19
 */
static inline int eina_rw_slice_compare(const Eina_Rw_Slice a, const Eina_Rw_Slice b);

/**
 * @brief Copy a read-only slice to a read-write one, similar to memcpy().
 *
 * @param[in] dest where to write the memory.
 * @param[in] src where to load memory.
 *
 * @return a new slice with the resulting write. Note that the length
 *         (@c len) will be the smallest of @a dest and @a src.
 *
 * @see eina_rw_slice_dup()
 * @see eina_slice_dup()
 *
 * @since 1.19
 */
static inline Eina_Rw_Slice eina_rw_slice_copy(const Eina_Rw_Slice dest, const Eina_Slice src);

/**
 * @brief Seek within a slice, similar to fseek().
 *
 * @param[in] slice the containing slice to seek inside.
 * @param[in] offset how to get to the new position.
 * @param[in] whence SEEK_SET, SEEK_END as fseek().
 * @return a new slice contained inside, it will start at the given
 *         offset and have a length that goes until the end of the @a
 *         slice. If an invalid @a whence, a zero-sized slice starting
 *         at @a slice mem will be returned. The slice is guaranteed
 *         to be contained within @a slice, even if offset causes it
 *         to go out of bounds, then it will be clamped to 0 and
 *         slice.len.
 *
 * @since 1.19
 */
static inline Eina_Slice eina_slice_seek(const Eina_Slice slice, ssize_t offset, int whence);

/**
 * @brief Seek within a read-write slice, similar to fseek().
 *
 * @param[in] rw_slice the containing slice to seek inside.
 * @param[in] offset how to get to the new position.
 * @param[in] whence SEEK_SET, SEEK_END as fseek().
 * @return a new slice contained inside, it will start at the given
 *         offset and have a length that goes until the end of the @a
 *         rw_slice. If an invalid @a whence, a zero-sized slice
 *         starting at @a rw_slice mem will be returned. The slice is
 *         guaranteed to be contained within @a rw_slice, even if
 *         offset causes it to go out of bounds, then it will be
 *         clamped to 0 and slice.len.
 *
 * @since 1.19
 */
static inline Eina_Rw_Slice eina_rw_slice_seek(const Eina_Rw_Slice rw_slice, ssize_t offset, int whence);


/**
 * @brief Find a character inside the slice, similar to memchr().
 *
 * @param[in] slice the reference memory.
 * @param[in] c the byte (character) to find.
 * @return the memory within slice or @c NULL if not found.
 *
 * @since 1.19
 */
static inline const void *eina_slice_strchr(const Eina_Slice slice, int c);

/**
 * @brief Find a needle inside the slice, similar to memmem().
 *
 * @param[in] slice the reference memory.
 * @param[in] needle what to find.
 * @return the memory within slice or @c NULL if not found.
 *
 * @since 1.19
 */
static inline const void *eina_slice_find(const Eina_Slice slice, const Eina_Slice needle);

/**
 * @brief Checks if the slice starts with a prefix.
 *
 * @param[in] slice the reference memory.
 * @param[in] prefix the slice to check if @a slice starts with.
 * @return #EINA_TRUE if @a slice starts with @a prefix, #EINA_FALSE otherwise.
 *
 * @since 1.19
 */
static inline Eina_Bool eina_slice_startswith(const Eina_Slice slice, const Eina_Slice prefix);

/**
 * @brief Checks if the slice ends with a suffix.
 *
 * @param[in] slice the reference memory.
 * @param[in] suffix the slice to check if @a slice ends with.
 * @return #EINA_TRUE if @a slice ends with @a suffix, #EINA_FALSE otherwise.
 *
 * @since 1.19
 */
static inline Eina_Bool eina_slice_endswith(const Eina_Slice slice, const Eina_Slice suffix);

/**
 * @brief Find a character inside the slice, similar to memchr().
 *
 * @param[in] rw_slice the reference memory.
 * @param[in] c the byte (character) to find.
 * @return the memory within slice or @c NULL if not found.
 *
 * @since 1.19
 */
static inline void *eina_rw_slice_strchr(const Eina_Rw_Slice rw_slice, int c);

/**
 * @brief Find a needle inside the slice, similar to memmem().
 *
 * @param[in] rw_slice the reference memory.
 * @param[in] needle what to find.
 * @return the memory within slice or @c NULL if not found.
 *
 * @since 1.19
 */
static inline void *eina_rw_slice_find(const Eina_Rw_Slice rw_slice, const Eina_Slice needle);

/**
 * @brief Checks if the slice starts with a prefix.
 *
 * @param[in] slice the reference memory.
 * @param[in] prefix the slice to check if @a slice starts with.
 * @return #EINA_TRUE if @a slice starts with @a prefix, #EINA_FALSE otherwise.
 *
 * @since 1.19
 */
static inline Eina_Bool eina_rw_slice_startswith(const Eina_Rw_Slice slice, const Eina_Slice prefix);

/**
 * @brief Checks if the slice ends with a suffix.
 *
 * @param[in] slice the reference memory.
 * @param[in] suffix the slice to check if @a slice ends with.
 * @return #EINA_TRUE if @a slice ends with @a suffix, #EINA_FALSE otherwise.
 *
 * @since 1.19
 */
static inline Eina_Bool eina_rw_slice_endswith(const Eina_Rw_Slice slice, const Eina_Slice suffix);

/**
 * @brief The memory position where the slice ends.
 *
 * @note this is out-of the slice, the first byte after it ends and
 * must not be accessed.
 *
 * @param[in] slice the reference memory.
 * @return the first byte after the slice ends.
 *
 * @since 1.19
 */
static inline const void *eina_slice_end_get(const Eina_Slice slice);

/**
 * @brief The memory position where the slice ends.
 *
 * @note this is out-of the slice, the first byte after it ends and
 * must not be accessed.
 *
 * @param[in] rw_slice the reference memory.
 * @return the first byte after the slice ends.
 *
 * @since 1.19
 */
static inline void *eina_rw_slice_end_get(const Eina_Rw_Slice rw_slice);

/**
 * @brief A null-terminated string for this slice.
 *
 * @param[in] slice the reference memory.
 * @return newly allocated memory or @c NULL on error
 *
 * @since 1.19
 */
static inline char *eina_slice_strdup(const Eina_Slice slice);

/**
 * @brief A null-terminated string for this slice.
 *
 * @param[in] slice the reference memory.
 * @return newly allocated memory or @c NULL on error
 *
 * @since 1.19
 */
static inline char *eina_rw_slice_strdup(const Eina_Rw_Slice rw_slice);

/**
 * @def EINA_SLICE_ARRAY(buf)
 *
 * @brief Initializer for arrays of any kind.
 *
 * @param[in] buf The array to create the slice from.
 * @return The initialized slice object.
 *
 * It is often useful for globals.
 *
 * @note This macro is usable with both Eina_Slice or Eina_Rw_Slice.
 *
 * @code
 * static uint8_t buf[1024];
 * static Eina_Slice rw_slice = EINA_SLICE_ARRAY(buf);
 * @endcode
 *
 * @see EINA_SLICE_STR_LITERAL() for specific version that checks for string literals.
 *
 * @since 1.19
 */
#ifdef __cplusplus
#define EINA_SLICE_ARRAY(buf) {((sizeof(buf) / sizeof((buf)[0])) * sizeof((buf)[0])), (buf)}
#else
#define EINA_SLICE_ARRAY(buf) {.len = ((sizeof(buf) / sizeof((buf)[0])) * sizeof((buf)[0])), .mem = (buf)}
#endif


/**
 * @def EINA_RW_SLICE_DECLARE(name, length)
 *
 * Declare a local (stack) array for storage at given @a length and
 * initialize an Eina_Rw_Slice called @a name.
 *
 * @param[in] name the name of the variable to be the Eina_Rw_Slice.
 * @param[in] length the size in bytes of the storage.
 *
 * @since 1.19
 */
#define EINA_RW_SLICE_DECLARE(name, length) \
  uint8_t _eina_slice_storage_ ## name [(length)] = { 0 }; \
  Eina_Rw_Slice name = EINA_SLICE_ARRAY(_eina_slice_storage_ ## name)

/**
 * @def EINA_SLICE_STR_LITERAL(buf)
 *
 * Initializer for string literals (those declared as
 * double-quoted). The size will @b NOT include the trailing
 * null-terminator.
 *
 * It is often useful for globals.
 *
 * @param[in] buf The array to create the slice from.
 * @return The initialized slice object.
 *
 * @note This macro is usable with both Eina_Slice or Eina_Rw_Slice.
 *
 * @code
 * static const Eina_Slice ro_slice = EINA_SLICE_STR_LITERAL("hello world");
 * @endcode
 *
 * @see EINA_SLICE_STR() for more generic version.
 * @see EINA_SLICE_ARRAY() for version that uses a general array.
 *
 * @since 1.19
 */
#ifdef __cplusplus
#define EINA_SLICE_STR_LITERAL(buf) {(sizeof("" buf) - 1), (buf)}
#else
#define EINA_SLICE_STR_LITERAL(buf) {.len = (sizeof("" buf) - 1), .mem = (buf)}
#endif

/**
 * @def EINA_SLICE_STR(str)
 *
 * Initializer for strings (uses strlen()).
 *
 * @param[in] str The string to create the slice from.
 * @return The initialized slice object.
 *
 * @note This macro is usable with both Eina_Slice or Eina_Rw_Slice.
 *
 * @code
 * Eina_Slice ro_slice = EINA_SLICE_STR("hello world");
 * @endcode
 *
 * @see EINA_SLICE_STR() for specific version using literals.
 *
 * @since 1.19
 */
#ifdef __cplusplus
#define EINA_SLICE_STR(str) {strlen((str)), (str)}
#else
#define EINA_SLICE_STR(str) {.len = strlen((str)), .mem = (str)}
#endif

/**
 * @def EINA_SLICE_STR_FMT
 *
 * To be used in printf()-like statements, prints the slice as a
 * string, its @c len is to be used, then it doesn't need the null
 * terminator.
 *
 * Use with EINA_SLICE_STR_PRINT()
 *
 * @note This macro is usable with both Eina_Slice or Eina_Rw_Slice.
 *
 * @code
 * Eina_Slice s = EINA_SLICE_STR_LITERAL("hello");
 * printf("s=" EINA_SLICE_STR_FMT "\n", EINA_SLICE_STR_PRINT(s));
 * @endcode
 *
 * @since 1.19
 */
#define EINA_SLICE_STR_FMT "%.*s"

/**
 * @def EINA_SLICE_STR_PRINT(s)
 *
 * @brief To be used in printf()-like statements when EINA_SLICE_STR_FMT was
 * used, it will print the slice as a string up to @c len.
 *
 * @param[in] s The slice.
 * @return The arguments for an EINA_SLICE_STR_FMT formatted printf.
 *
 * Use with EINA_SLICE_STR_FMT.
 *
 * @note This macro is usable with both Eina_Slice or Eina_Rw_Slice.
 *
 * @code
 * Eina_Slice s = EINA_SLICE_STR_LITERAL("hello");
 * printf("s=" EINA_SLICE_STR_FMT "\n", EINA_SLICE_STR_PRINT(s));
 * @endcode
 *
 * @since 1.19
 */
#define EINA_SLICE_STR_PRINT(s) (int)(s).len, (const char *)(s).mem

/**
 * @def EINA_SLICE_FMT
 *
 * To be used in printf()-like statements, prints the slice as
 * @c 0x1234+12 (@c mem + @c len).
 *
 * Use with EINA_SLICE_PRINT()
 *
 * @note This macro is usable with both Eina_Slice or Eina_Rw_Slice.
 *
 * @code
 * Eina_Slice s = EINA_SLICE_STR_LITERAL("hello");
 * printf("s=" EINA_SLICE_FMT "\n", EINA_SLICE_PRINT(s));
 * @endcode
 *
 * @since 1.19
 */
#define EINA_SLICE_FMT "%p+%zu"

/**
 * @def EINA_SLICE_PRINT(s)
 *
 * To be used in printf()-like statements when EINA_SLICE_FMT was
 * used, it will print the slice @c mem and @c len.
 *
 * Use with EINA_SLICE_FMT.
 *
 * @note This macro is usable with both Eina_Slice or Eina_Rw_Slice.
 *
 * @param[in] s The slice.
 * @return The arguments for an EINA_SLICE_FMT formatted printf.
 *
 * @code
 * Eina_Slice s = EINA_SLICE_STR_LITERAL("hello");
 * printf("s=" EINA_SLICE_FMT "\n", EINA_SLICE_PRINT(s));
 * @endcode
 *
 * @since 1.19
 */
#define EINA_SLICE_PRINT(s) (s).mem, (s).len

/**
 * @def EINA_SLICE_FOREACH(s, itr)
 *
 * Iterate over the slice memory, using @c itr. Each increment will be
 * using the size of @c itr pointer (int32_t* will do in increments of
 * 4 bytes).
 *
 * @note This macro is usable with both Eina_Slice or Eina_Rw_Slice.
 *
 * @note Be aware of memory alignment! Accessing unaligned memory may
 *       not be supported in some architectures.
 *
 * @param[in] s The slice.
 * @param[in,out] itr the iterator to hold each byte. Use a proper type, not
 *        "void*" or "const void*" as it doesn't have an intrinsic
 *        size.
 *
 * @since 1.19
 */
#ifdef __cplusplus
#define EINA_SLICE_FOREACH(s, itr) \
  for ((itr) = static_cast<__typeof__(itr)>((s).mem); \
       (itr) < static_cast<__typeof__(itr)>(static_cast<void *>((s).bytes + (s).len)); \
       (itr)++)
#else
#define EINA_SLICE_FOREACH(s, itr) \
  for ((itr) = (s).mem; \
       (void *)(itr) < (void *)((s).bytes + (s).len); \
       (itr)++)
#endif

#include "eina_inline_slice.x"

/**
 * @}
 */

/**
 * @}
 */

#endif /* _EINA_SLICE_H */
