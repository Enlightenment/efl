#ifndef EINA_USTRBUF_H
#define EINA_USTRBUF_H

#include <stddef.h>

#include "eina_types.h"
#include "eina_unicode.h"

/**
 * @addtogroup Eina_Unicode_String_Buffer_Group Unicode String Buffer
 *
 * @brief These functions provide unicode string buffer management.
 *
 * The Unicode String Buffer data type is designed to be a mutable
 * string, allowing the appending, prepending or insertion of a string
 * to a buffer.
 */

/**
 * @addtogroup Eina_Data_Types_Group Data Types
 *
 * @{
 */

/**
 * @defgroup Eina_Unicode_String_Buffer_Group Unicode String Buffer
 *
 * @{
 */

/**
 * @typedef Eina_UStrbuf
 * Type for a unicode string buffer.
 */
typedef struct _Eina_Strbuf Eina_UStrbuf;

/**
 * @brief Creates a new unicode string buffer.
 *
 * @return Newly allocated string buffer instance, or @c NULL on error.
 *
 * This function creates a new string buffer. To free the resources, use
 * eina_ustrbuf_free().
 *
 * @see eina_ustrbuf_free()
 * @see eina_ustrbuf_append()
 * @see eina_ustrbuf_string_get()
 */
EAPI Eina_UStrbuf *eina_ustrbuf_new(void) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Creates a new string buffer using the passed string.
 *
 * @param[in] str The string to manage.
 * @return Newly allocated string buffer instance, or @c NULL on error.
 *
 * This function creates a new unicode string buffer. The passed string
 * is used directly as the buffer, it's effectively the inverse of
 * eina_ustrbuf_string_steal(). The passed string must be malloc'd.
 * To free the resources, use eina_ustrbuf_free().
 *
 * @see eina_ustrbuf_free()
 * @see eina_ustrbuf_append()
 * @see eina_ustrbuf_string_get()
 *
 * @since 1.1.0
 */
EAPI Eina_UStrbuf *eina_ustrbuf_manage_new(Eina_Unicode *str) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Creates a new string buffer using the passed string.
 *
 * @param[in] str The string to manage.
 * @param[in] length The length of the string.
 * @return Newly allocated string buffer instance, or @c NULL on error.
 *
 * This function creates a new string buffer. The passed string is used
 * directly as the buffer, it's effectively the inverse of
 * eina_ustrbuf_string_steal(). The passed string must be malloc'd.  To
 * free the resources, use eina_ustrbuf_free().
 *
 * @see eina_ustrbuf_manage_new()
 *
 * @since 1.2.0
 */
EAPI Eina_UStrbuf *eina_ustrbuf_manage_new_length(Eina_Unicode *str, size_t length) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Frees a string buffer.
 *
 * @param[in,out] buf The string buffer to free.
 *
 * This function frees the memory of @p buf. @p buf must have been
 * created by eina_ustrbuf_new().
 */
EAPI void eina_ustrbuf_free(Eina_UStrbuf *buf) EINA_ARG_NONNULL(1);

/**
 * @brief Resets a string buffer.
 *
 * @param[in,out] buf The string buffer.
 *
 * This function resets @p buf: the buffer len is set to 0, and the
 * string data is set to '\\0'. No memory is freed.
 */
EAPI void eina_ustrbuf_reset(Eina_UStrbuf *buf) EINA_ARG_NONNULL(1);

/**
 * @brief Appends a string to a buffer, reallocating as necessary.
 *
 * @param[in,out] buf The string buffer.
 * @param[in] str The string to append.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure such as if
 * @p str could not be appended.
 *
 * This function appends @p str to @p buf. It computes the length of
 * @p str, so is slightly slower than eina_ustrbuf_append_length(). If
 * the length is known beforehand, consider using that variant.
 *
 * @see eina_ustrbuf_append()
 * @see eina_ustrbuf_append_length()
 */
EAPI Eina_Bool eina_ustrbuf_append(Eina_UStrbuf *buf, const Eina_Unicode *str) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Appends an escaped string to a buffer, reallocating as necessary.
 *
 * @param[in,out] buf The string buffer.
 * @param[in] str The string to append.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p str could not be appended.
 *
 * This function appends the escaped string @p str to @p buf.
 */
EAPI Eina_Bool eina_ustrbuf_append_escaped(Eina_UStrbuf *buf, const Eina_Unicode *str) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Appends a string to a buffer, reallocating as necessary,
 * limited by the given length.
 *
 * @param[in,out] buf The string buffer.
 * @param[in] str The string to append.
 * @param[in] maxlen The maximum number of characters to append.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p str could not be appended.
 *
 * This function appends at most @p maxlen characters of @p str to
 * @p buf. It can't append more than the length of @p str. It
 * computes the length of @p str, so is slightly slower than
 * eina_ustrbuf_append_length(). If the length is known beforehand,
 * consider using that variant (@p maxlen should then be checked so
 * that it is greater than the size of @p str).
 *
 * @see eina_ustrbuf_append()
 * @see eina_ustrbuf_append_length()
 */
EAPI Eina_Bool eina_ustrbuf_append_n(Eina_UStrbuf *buf, const Eina_Unicode *str, size_t maxlen) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Appends a string of exact length to a buffer, reallocating as
 * necessary.
 *
 * @param[in,out] buf The string buffer.
 * @param[in] str The string to append.
 * @param[in] length The exact length to use.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p str could not be appended.
 *
 * This function appends @p str to @p buf. @p str must be of size at
 * most @p length. It is slightly faster than eina_ustrbuf_append() as
 * it does not compute the size of @p str. It is useful when dealing
 * with strings of known size, such as eina_stringshare.
 *
 * @see eina_stringshare_length()
 * @see eina_ustrbuf_append()
 * @see eina_ustrbuf_append_n()
 */
EAPI Eina_Bool eina_ustrbuf_append_length(Eina_UStrbuf *buf, const Eina_Unicode *str, size_t length) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Appends a slice to a buffer, reallocating as necessary.
 *
 * @param[in,out] buf The string buffer.
 * @param[in] slice The slice to append.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p slice could not be appended.
 *
 * This function appends @p slice to @p buf.
 *
 * @since 1.19.0
 */
EAPI Eina_Bool eina_ustrbuf_append_slice(Eina_UStrbuf *buf, const Eina_Slice slice) EINA_ARG_NONNULL(1);

/**
 * @brief Appends a character to a string buffer, reallocating as
 * necessary.
 *
 * @param[in,out] buf The string buffer.
 * @param[in] c The char to append.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p c could not be appended.
 *
 * This function appends @p c to @p buf.
 */
EAPI Eina_Bool eina_ustrbuf_append_char(Eina_UStrbuf *buf, Eina_Unicode c) EINA_ARG_NONNULL(1);

/**
 * @brief Inserts a string to a buffer, reallocating as necessary.
 *
 * @param[in,out] buf The string buffer.
 * @param[in] str The string to insert.
 * @param[in] pos The position to insert the string.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p str could not be inserted.
 *
 * This function inserts @p str to @p buf at position @p pos. It
 * computes the length of @p str, so is slightly slower than
 * eina_ustrbuf_insert_length(). If  the length is known beforehand,
 * consider using that variant.
 */
EAPI Eina_Bool eina_ustrbuf_insert(Eina_UStrbuf *buf, const Eina_Unicode *str, size_t pos) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Inserts an escaped string to a buffer, reallocating as
 * necessary.
 *
 * @param[in,out] buf The string buffer.
 * @param[in] str The string to insert.
 * @param[in] pos The position to insert the string.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p str could not be inserted.
 *
 * This function inserts the escaped string @p str to @p buf at
 * position @p pos.
 */
EAPI Eina_Bool eina_ustrbuf_insert_escaped(Eina_UStrbuf *buf, const Eina_Unicode *str, size_t pos) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Inserts a string to a buffer, reallocating as necessary. Limited by maxlen.
 *
 * @param[in,out] buf The string buffer.
 * @param[in] str The string to insert.
 * @param[in] maxlen The maximum number of chars to insert.
 * @param[in] pos The position to insert the string.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p str could not be inserted.
 *
 * This function inserts @p str into @p buf at position @p pos, with at
 * most @p maxlen bytes. The number of inserted characters cannot be
 * greater than the length of @p str. It computes the length of
 * @p str, so is slightly slower than eina_ustrbuf_insert_length(). If the
 * length is known beforehand, consider using that variant (@p maxlen
 * should then be checked so that it is greater than the size of
 * @p str).
 */
EAPI Eina_Bool eina_ustrbuf_insert_n(Eina_UStrbuf *buf, const Eina_Unicode *str, size_t maxlen, size_t pos) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Inserts a string of exact length to a buffer, reallocating as necessary.
 *
 * @param[in,out] buf The string buffer.
 * @param[in] str The string to insert.
 * @param[in] length The exact length to use.
 * @param[in] pos The position to insert the string.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p str could not be inserted.
 *
 * This function inserts @p str into @p buf. @p str must be no longer
 * than @p length. It is slightly faster than eina_ustrbuf_insert() as
 * it does not compute the size of @p str. It is useful when dealing
 * with strings of known size, such as eina_stringshare.
 *
 * @see eina_stringshare_length()
 * @see eina_ustrbuf_insert()
 * @see eina_ustrbuf_insert_n()
 */
EAPI Eina_Bool eina_ustrbuf_insert_length(Eina_UStrbuf *buf, const Eina_Unicode *str, size_t length, size_t pos) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Inserts a slice to a buffer, reallocating as necessary.
 *
 * @param[in,out] buf The string buffer.
 * @param[in] slice The slice to insert.
 * @param[in] pos The position to insert the string.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p slice could not be inserted.
 *
 * This function inserts @p slice to @p buf at position @p pos.
 *
 * @since 1.19.0
 */
EAPI Eina_Bool eina_ustrbuf_insert_slice(Eina_UStrbuf *buf, const Eina_Slice slice, size_t pos) EINA_ARG_NONNULL(1);

/**
 * @brief Inserts a character to a string buffer, reallocating as
 * necessary.
 *
 * @param[in,out] buf The string buffer.
 * @param[in] c The char to insert.
 * @param[in] pos The position to insert the char.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p c could not be inserted.
 *
 * This function inserts @p c to @p buf at position @p pos.
 */
EAPI Eina_Bool eina_ustrbuf_insert_char(Eina_UStrbuf *buf, Eina_Unicode c, size_t pos) EINA_ARG_NONNULL(1);

/**
 * @def eina_ustrbuf_prepend(buf, str)
 * @brief Prepends a string to the given buffer.
 *
 * @param[in,out] buf The string buffer.
 * @param[in] str The string to prepend.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p str could not be prepended.
 *
 * This macro simply calls eina_ustrbuf_insert() with position 0.
 */
#define eina_ustrbuf_prepend(buf, str) eina_ustrbuf_insert(buf, str, 0)

/**
 * @def eina_ustrbuf_prepend_escaped(buf, str)
 * @brief Prepends an escaped string to the given buffer.
 *
 * @param[in,out] buf The string buffer.
 * @param[in] str The string to prepend.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p str could not be prepended.
 *
 * This macro simply calls eina_ustrbuf_insert_escaped() with position 0.
 */
#define eina_ustrbuf_prepend_escaped(buf, str) eina_ustrbuf_insert_escaped(buf, str, 0)

/**
 * @def eina_ustrbuf_prepend_n(buf, str)
 * @brief Prepends an escaped string to the given buffer.
 *
 * @param[in,out] buf The string buffer.
 * @param[in] str The string to prepend.
 * @param[in] maxlen The maximum number of Eina_Unicode *s to prepend.
 * @return #EINA_TRUE on success, #EINA_FALSE if @str could not be prepended.
 *
 * This macro simply calls eina_ustrbuf_insert_n() with position 0.
 */
#define eina_ustrbuf_prepend_n(buf, str, maxlen) eina_ustrbuf_insert_n(buf, str, maxlen, 0)

/**
 * @def eina_ustrbuf_prepend_length(buf, str)
 * @brief Prepends an escaped string to the given buffer.
 *
 * @param[in,out] buf The string buffer.
 * @param[in] str The string to prepend.
 * @param[in] length The exact length to use.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p str could not be prepended.
 *
 * This macro simply calls eina_ustrbuf_insert_length() with position 0.
 */
#define eina_ustrbuf_prepend_length(buf, str, length) eina_ustrbuf_insert_length(buf, str, length, 0)

/**
 * @def eina_ustrbuf_prepend_char(buf, c)
 * @brief Prepends a unicode character to the given buffer.
 *
 * @param[in,out] buf The string buffer.
 * @param[in] c The Eina_Unicode character to prepend.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p c could not be prepended.
 *
 * This macro is calling eina_ustrbuf_insert_Eina_Unicode() at position 0.
 */
#define eina_ustrbuf_prepend_char(buf, c) eina_ustrbuf_insert_char(buf, c, 0)

/**
 * @brief Removes a section of the given string buffer.
 *
 * @param[in,out] buf The string buffer to remove a slice.
 * @param[in] start The initial (inclusive) slice position to start
 *        removing, in bytes.
 * @param[in] end The final (non-inclusive) slice position to finish
 *        removing, in bytes.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function removes a slice of @p buf, starting at @p start
 * (inclusive) and ending at @p end (non-inclusive). Both values are
 * in bytes.
 */
EAPI Eina_Bool
eina_ustrbuf_remove(Eina_UStrbuf *buf, size_t start, size_t end) EINA_ARG_NONNULL(1);

/**
 * @brief Retrieves a pointer to the contents of a string buffer.
 *
 * @param[in] buf The string buffer.
 * @return The current string in the string buffer.
 *
 * This function returns the string contained in @p buf. The returned
 * value must not be modified and will no longer be valid if @p buf is
 * modified. In other words, calling eina_ustrbuf_append() or similar
 * functions will make this pointer invalid.
 *
 * @see eina_ustrbuf_string_steal()
 */
EAPI const Eina_Unicode *
eina_ustrbuf_string_get(const Eina_UStrbuf *buf) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Steals the contents of a string buffer.
 *
 * @param[in] buf The string buffer.
 * @return The string that was contained in @p buf.
 *
 * This function returns the string contained in @p buf. @p buf is
 * then initialized and does not own the returned string anymore. The
 * caller must release the memory of the returned string by calling
 * free().
 *
 * @see eina_ustrbuf_string_get()
 */
EAPI Eina_Unicode *
eina_ustrbuf_string_steal(Eina_UStrbuf *buf) EINA_MALLOC EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Frees the contents of a string buffer but not the buffer.
 *
 * @param[in,out] buf The string buffer.
 *
 * This function frees the string contained in @p buf without freeing
 * @p buf.
 */
EAPI void
eina_ustrbuf_string_free(Eina_UStrbuf *buf) EINA_ARG_NONNULL(1);

/**
 * @brief Retrieves the length of the string buffer's content.
 *
 * @param[in] buf The string buffer.
 * @return The current length of the string, in bytes.
 *
 * This function returns the length of @p buf.
 */
EAPI size_t
eina_ustrbuf_length_get(const Eina_UStrbuf *buf) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Gets a read-only slice of the buffer contents.
 *
 * @param[in] buf The string buffer.
 * @return A read-only slice for the current contents. It may become
 * invalid as soon as @a buf is changed.
 *
 * @since 1.19
 */
EAPI Eina_Slice eina_ustrbuf_slice_get(const Eina_UStrbuf *buf) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Gets a read-write slice of the buffer contents.
 *
 * @param[in] buf The string buffer.
 * @return A read-write slice for the current contents. It may become
 * invalid as soon as the @p buf is changed, such as through calls like
 * eina_ustrbuf_append() or eina_ustrbuf_remove().
 *
 * @since 1.19.0
 */
EAPI Eina_Rw_Slice eina_ustrbuf_rw_slice_get(const Eina_UStrbuf *buf) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Frees the buffer, returning its old contents.
 *
 * @param[in,out] buf The string buffer.
 * @return The string contained by buf. The caller must release the
 * memory of the returned string by calling free().
 *
 * @since 1.19
 */
EAPI Eina_Unicode* eina_ustrbuf_release(Eina_UStrbuf *buf) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @}
 */

/**
 * @}
 */

#endif /* EINA_STRBUF_H */
