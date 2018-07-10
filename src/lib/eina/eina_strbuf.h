#ifndef EINA_STRBUF_H
#define EINA_STRBUF_H

#include <stddef.h>
#include <stdarg.h>

#include "eina_types.h"
#include "eina_slice.h"


/**
 * @page tutorial_strbuf Eina_Strbuf example
 * @dontinclude eina_strbuf_01.c
 *
 * First thing always is including Eina:
 * @skipline #include
 * @until #include
 *
 * Next we initialize eina and create a string buffer to play with:
 * @until strbuf_new
 *
 * Here you can see two different ways of creating a buffer with the same
 * contents. We could create them in simpler ways, but this gives us an
 * opportunity to demonstrate several functions in action:
 * @until strbuf_reset
 * @until strbuf_reset
 *
 * Next we use the printf family of functions to create a formated string,
 * add, remove and replace some content:
 * @until strbuf_string_get
 * @until strbuf_string_get
 * @until strbuf_string_get
 *
 * Once done we free our string buffer, shut down Eina and end the application:
 * @until }
 *
 * @example eina_strbuf_01.c
 */

/**
 * @addtogroup Eina_String_Buffer_Group String Buffer
 *
 * @brief These functions provide string buffer management.
 *
 * The String Buffer data type is designed to be a mutable string,
 * allowing the appending, prepending or insertion of a string to a
 * buffer.
 *
 * For more information see @ref tutorial_strbuf "this example".
 */

/**
 * @addtogroup Eina_Data_Types_Group Data Types
 *
 * @{
 */

/**
 * @defgroup Eina_String_Buffer_Group String Buffer
 *
 * @{
 */

/**
 * @typedef Eina_Strbuf
 * Type for a string buffer.
 */
typedef struct _Eina_Strbuf Eina_Strbuf;

/**
 * @brief Creates a new string buffer.
 *
 * @return Newly allocated string buffer instance, or @c NULL on error.
 *
 * This function creates a new string buffer. To free the resources, use
 * eina_strbuf_free().
 *
 * @see eina_strbuf_free()
 * @see eina_strbuf_append()
 * @see eina_strbuf_string_get()
 */
EAPI Eina_Strbuf *eina_strbuf_new(void) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Creates a new string buffer using the passed string.
 *
 * @param str The string to manage.
 * @return Newly allocated string buffer instance, or @c NULL on error.
 *
 * This function creates a new string buffer.  The passed string is used
 * directly as the buffer, it's effectively the inverse of
 * eina_strbuf_string_steal().  The passed string must be malloc'd.
 * To free the resources, use eina_strbuf_free().
 *
 * @see eina_strbuf_free()
 * @see eina_strbuf_append()
 * @see eina_strbuf_string_get()
 *
 * @since 1.1.0
 */
EAPI Eina_Strbuf *eina_strbuf_manage_new(char *str) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Creates a new string buffer using the passed string.
 *
 * @param str The string to manage.
 * @param length The length of the string.
 * @return Newly allocated string buffer instance, or @c NULL on error.
 *
 * This function creates a new string buffer. The passed string is used
 * directly as the buffer, it's effectively the inverse of
 * eina_strbuf_string_steal(). The passed string must be malloc'd.  To
 * free the resources, use eina_strbuf_free().
 *
 * @see eina_strbuf_manage_new()
 *
 * @since 1.2.0
 */
EAPI Eina_Strbuf *eina_strbuf_manage_new_length(char *str, size_t length) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Creates a new string buffer using the passed string.
 *
 * @param str The string to manage.
 * @param length The length of the string.
 * @return Newly allocated string buffer instance, or @c NULL on error.
 *
 * This function creates a new string buffer. The passed string is used
 * directly as the buffer, it's effectively the inverse of
 * eina_strbuf_string_steal(). The passed string is expected to be static
 * or otherwise on the stack rather than the malloc'd string required by
 * eina_strbuf_manage_new_length().  To free the resources, use
 * eina_strbuf_free().
 *
 * @see eina_strbuf_manage_new()
 * @see eina_strbuf_manage_new_length()
 *
 * @since 1.9.0
 */
EAPI Eina_Strbuf *eina_strbuf_manage_read_only_new_length(const char *str, size_t length) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Frees a string buffer.
 *
 * @param buf The string buffer to free.
 *
 * This function frees the memory of @p buf. @p buf must have been
 * created by eina_strbuf_new().
 */
EAPI void eina_strbuf_free(Eina_Strbuf *buf) EINA_ARG_NONNULL(1);

/**
 * @brief Resets a string buffer.
 *
 * @param buf The string buffer.
 *
 * This function resets @p buf: the buffer len is set to 0, and the
 * string data is set to '\\0'. No memory is freed.
 */
EAPI void eina_strbuf_reset(Eina_Strbuf *buf) EINA_ARG_NONNULL(1);

/**
 * @brief Appends a string to a buffer, reallocating as necessary.
 *
 * @param buf The string buffer.
 * @param str The string to append.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure such as if
 * @p str could not be appended.
 *
 * This function appends @p str to @p buf. It computes the length of
 * @p str, so is slightly slower than eina_strbuf_append_length(). If
 * the length is known beforehand, consider using that variant.
 *
 * @see eina_strbuf_append()
 * @see eina_strbuf_append_length()
 */
EAPI Eina_Bool eina_strbuf_append(Eina_Strbuf *buf, const char *str) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Appends an escaped string to a buffer, reallocating as necessary.
 *
 * @param buf The string buffer.
 * @param str The string to append.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p str could not be appended.
 *
 * This function escapes and then appends the string @p str to @p buf.
 */
EAPI Eina_Bool eina_strbuf_append_escaped(Eina_Strbuf *buf, const char *str) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Appends a string to a buffer, reallocating as necessary,
 * limited by the given length.
 *
 * @param buf The string buffer.
 * @param str The string to append.
 * @param maxlen The maximum number of characters to append.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p str could not be appended.
 *
 * This function appends at most @p maxlen characters of @p str to
 * @p buf. It can't append more than the length of @p str. It
 * computes the length of @p str, so it is slightly slower than
 * eina_strbuf_append_length(). If the length is known beforehand,
 * consider using that variant (@p maxlen should then be checked so
 * that it is greater than the size of @p str).
 *
 * @see eina_strbuf_append()
 * @see eina_strbuf_append_length()
 */
EAPI Eina_Bool eina_strbuf_append_n(Eina_Strbuf *buf, const char *str, size_t maxlen) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Appends a string of exact length to a buffer, reallocating as
 * necessary.
 *
 * @param buf The string buffer.
 * @param str The string to append.
 * @param length The exact length to use.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p str could not be appended.
 *
 * This function appends @p str to @p buf. @p str must be of size at
 * most @p length. It is slightly faster than eina_strbuf_append() as
 * it does not compute the size of @p str. It is useful when dealing
 * with strings of known size, such as eina_stringshare.
 *
 * @see eina_stringshare_length()
 * @see eina_strbuf_append()
 * @see eina_strbuf_append_n()
 */
EAPI Eina_Bool eina_strbuf_append_length(Eina_Strbuf *buf, const char *str, size_t length) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Append a slice to a buffer, reallocating as necessary.
 *
 * @param buf The string buffer.
 * @param slice The slice to append.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p slice could not be appended.
 *
 * This function appends @p slice to @p buf.
 *
 * @since 1.19.0
 */
EAPI Eina_Bool eina_strbuf_append_slice(Eina_Strbuf *buf, const Eina_Slice slice) EINA_ARG_NONNULL(1);

/**
 * @brief Appends an Eina_Strbuf to a buffer, reallocating as necessary.
 *
 * @param buf The string buffer.
 * @param data The string buffer to append.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p data could not be appended.
 *
 * This function appends @p data to @p buf. @p data must be allocated and
 * different from @c NULL. It is slightly faster than eina_strbuf_append() as
 * it does not compute the size of @p str.
 *
 * @see eina_strbuf_append()
 * @see eina_strbuf_append_n()
 * @see eina_strbuf_append_length()
 *
 * @since 1.9.0
 */
EAPI Eina_Bool eina_strbuf_append_buffer(Eina_Strbuf *buf, const Eina_Strbuf *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Appends a character to a string buffer, reallocating as
 * necessary.
 *
 * @param buf The string buffer.
 * @param c The character to append.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p c could not be appended.
 *
 * This function appends @p c to @p buf.
 */
EAPI Eina_Bool eina_strbuf_append_char(Eina_Strbuf *buf, char c) EINA_ARG_NONNULL(1);

/**
 * @brief Appends data elements to a buffer using printf-style formatting.
 *
 * @param buf The string buffer.
 * @param fmt A formatting string.
 * @param ... Variable list of data parameters to be appended.
 * @return #EINA_TRUE on success, #EINA_FALSE if formatted string could
 * not be appended.
 *
 * This function appends the string defined by the format @p fmt to @p
 * buf, reallocating as necessary. @p fmt must be of a valid format for
 * the printf family of functions.
 *
 * @see eina_strbuf_append()
 */
EAPI Eina_Bool eina_strbuf_append_printf(Eina_Strbuf *buf, const char *fmt, ...) EINA_ARG_NONNULL(1, 2) EINA_PRINTF(2, 3);

/**
 * @brief Appends data elements to a string buffer using a va_list.
 *
 * @param buf The string buffer.
 * @param fmt A formatting string.
 * @param args The variable arguments.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @see eina_strbuf_append_printf()
 */
EAPI Eina_Bool eina_strbuf_append_vprintf(Eina_Strbuf *buf, const char *fmt, va_list args) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Inserts a string into a buffer, reallocating as necessary.
 *
 * @param buf The string buffer.
 * @param str The string to insert.
 * @param pos The position to insert the string.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p str could not be inserted.
 *
 * This function inserts @p str into @p buf at position @p pos. It
 * computes the length of @p str, so is slightly slower than
 * eina_strbuf_insert_length(). If the length is known beforehand,
 * consider using that variant.
 */
EAPI Eina_Bool eina_strbuf_insert(Eina_Strbuf *buf, const char *str, size_t pos) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Inserts an escaped string into a buffer, reallocating as
 * necessary.
 *
 * @param buf The string buffer.
 * @param str The string to insert.
 * @param pos The position to insert the string.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p str could not be inserted.
 *
 * This function escapes and inserts the string @p str into @p buf at
 * position @p pos.
 */
EAPI Eina_Bool eina_strbuf_insert_escaped(Eina_Strbuf *buf, const char *str, size_t pos) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Inserts a string of a maximum length into a buffer, reallocating as necessary.
 *
 * @param buf The string buffer.
 * @param str The string to insert.
 * @param maxlen The maximum number of chars to insert.
 * @param pos The position to insert the string.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p str could not be inserted.
 *
 * This function inserts @p str into @p buf at position @p pos, with at
 * most @p maxlen bytes. The number of inserted characters cannot be
 * greater than the length of @p str. It computes the length of
 * @p str, so is slightly slower than eina_strbuf_insert_length(). If the
 * length is known beforehand, consider using that variant (@p maxlen
 * should then be checked so that it is greater than the size of
 * @p str).
 */
EAPI Eina_Bool eina_strbuf_insert_n(Eina_Strbuf *buf, const char *str, size_t maxlen, size_t pos) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Inserts a string of an exact length into a buffer, reallocating as necessary.
 *
 * @param buf The string buffer.
 * @param str The string to insert.
 * @param length The exact length to use.
 * @param pos The position to insert the string.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p string could not be inserted.
 *
 * This function inserts @p str into @p buf. @p str must be no longer
 * than @p length. It is slightly faster than eina_strbuf_insert() as
 * it does not compute the size of @p str. It is useful when dealing
 * with strings of known size, such as eina_stringshare.
 *
 * @see eina_stringshare_length()
 * @see eina_strbuf_insert()
 * @see eina_strbuf_insert_n()
 */
EAPI Eina_Bool eina_strbuf_insert_length(Eina_Strbuf *buf, const char *str, size_t length, size_t pos) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Insert a slice into a buffer, reallocating as necessary.
 *
 * @param buf The string buffer.
 * @param slice The slice to insert.
 * @param pos The position to insert the string.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p slice could not be inserted.
 *
 * This function inserts @p slice into @p buf at position @p pos.
 *
 * @since 1.19.0
 */
EAPI Eina_Bool eina_strbuf_insert_slice(Eina_Strbuf *buf, const Eina_Slice slice, size_t pos) EINA_ARG_NONNULL(1);

/**
 * @brief Inserts a character into a string buffer, reallocating as
 * necessary.
 *
 * @param buf The string buffer.
 * @param c The character to insert.
 * @param pos The position to insert the character.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p c could not be inserted.
 *
 * This function inserts @p c into @p buf at position @p pos.
 */
EAPI Eina_Bool eina_strbuf_insert_char(Eina_Strbuf *buf, char c, size_t pos) EINA_ARG_NONNULL(1);

/**
 * @brief Inserts data elements into a buffer using printf-style formatting.
 *
 * @param buf The string buffer.
 * @param fmt The string to insert.
 * @param pos The position to insert the string.
 * @param ... Variable list of data parameters to be inserted.
 * @return #EINA_TRUE on success, #EINA_FALSE if the string could not be inserted.
 *
 * This function inserts a string as described by the format @p fmt into @p buf at
 * the position @p pos. @p fmt must be of a valid format for printf family of
 * functions.
 */
EAPI Eina_Bool eina_strbuf_insert_printf(Eina_Strbuf *buf, const char *fmt, size_t pos, ...) EINA_ARG_NONNULL(1, 2) EINA_PRINTF(2, 4);

/**
 * @brief Inserts data elements into a buffer using a va_list.
 *
 * @param buf The string buffer.
 * @param fmt The string to insert.
 * @param pos The position to insert the string.
 * @param args The variable arguments.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @see eina_strbuf_insert_printf
 */
EAPI Eina_Bool eina_strbuf_insert_vprintf(Eina_Strbuf *buf, const char *fmt, size_t pos, va_list args) EINA_ARG_NONNULL(1, 2);

/**
 * @def eina_strbuf_prepend(buf, str)
 * @brief Prepends a string to the given buffer.
 *
 * @param buf The string buffer.
 * @param str The string to prepend.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p str could not be prepended.
 *
 * This macro simply calls eina_strbuf_insert() with position 0.
 */
#define eina_strbuf_prepend(buf, str) eina_strbuf_insert(buf, str, 0)

/**
 * @def eina_strbuf_prepend_escaped(buf, str)
 * @brief Prepends an escaped string to the given buffer.
 *
 * @param buf The string buffer.
 * @param str The string to prepend.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p str could not be prepended.
 *
 * This macro simply calls eina_strbuf_insert_escaped() with position 0.
 */
#define eina_strbuf_prepend_escaped(buf, str) eina_strbuf_insert_escaped(buf, str, 0)

/**
 * @def eina_strbuf_prepend_n(buf, str)
 * @brief Prepends an escaped string to the given buffer.
 *
 * @param buf The string buffer.
 * @param str The string to prepend.
 * @param maxlen The maximum number of chars to prepend.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p str could not be prepended.
 *
 * This macro simply calls eina_strbuf_insert_n() with position 0.
 */
#define eina_strbuf_prepend_n(buf, str, maxlen) eina_strbuf_insert_n(buf, str, maxlen, 0)

/**
 * @def eina_strbuf_prepend_length(buf, str)
 * @brief Prepends an escaped string to the given buffer.
 *
 * @param buf The string buffer.
 * @param str The string to prepend.
 * @param length The exact length to use.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p str could not be prepended.
 *
 * This macro simply calls eina_strbuf_insert_length() with position 0.
 */
#define eina_strbuf_prepend_length(buf, str, length) eina_strbuf_insert_length(buf, str, length, 0)

/**
 * @def eina_strbuf_prepend_char(buf, str)
 * @brief Prepends a character to the given buffer.
 *
 * @param buf The string buffer.
 * @param c The character to prepend.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This macro simply calls eina_strbuf_insert_char() with position 0.
 */
#define eina_strbuf_prepend_char(buf, c) eina_strbuf_insert_char(buf, c, 0)

/**
 * @def eina_strbuf_prepend_printf(buf, fmt, ...)
 * @brief Prepends data elements to the given buffer using printf-style formatting.
 *
 * @param buf The string buffer.
 * @param fmt The string to prepend.
 * @param ... Variable list of data elements to prepend.
 * @return #EINA_TRUE on success, #EINA_FALSE if data could not be prepended.
 *
 * This macro simply calls eina_strbuf_insert_printf() with position 0.
 */
#define eina_strbuf_prepend_printf(buf, fmt, ...) eina_strbuf_insert_printf(buf, fmt, 0, ## __VA_ARGS__)

/**
 * @def eina_strbuf_prepend_vprintf(buf, fmt, args)
 * @brief Prepends the given string to the given buffer.
 *
 * @param buf The string buffer.
 * @param fmt The string to prepend.
 * @param args The variable arguments.
 * @return #EINA_TRUE on success, #EINA_FALSE if data could not be prepended.
 *
 * This macro simply calls eina_strbuf_insert_vprintf() with position 0.
 */
#define eina_strbuf_prepend_vprintf(buf, fmt, args)  eina_strbuf_insert_vprintf(buf, fmt, 0, args)

/**
 * @brief Removes a section from the given string buffer.
 *
 * @param buf The string buffer.
 * @param start The initial (inclusive) byte position to start removal.
 * @param end The final (non-inclusive) byte position to end removal.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function removes a slice of @p buf, starting at @p start
 * (inclusive) and ending at @p end (non-inclusive). Both values are
 * in bytes.
 */

EAPI Eina_Bool eina_strbuf_remove(Eina_Strbuf *buf, size_t start, size_t end) EINA_ARG_NONNULL(1);

/**
 * @brief Retrieves a pointer to the contents of a string buffer.
 *
 * @param buf The string buffer.
 * @return The current string in the string buffer.
 *
 * This function returns the string contained in @p buf. The returned
 * value must not be modified and will no longer be valid if @p buf is
 * modified. In other words, calling eina_strbuf_append() or similar
 * functions will make this pointer invalid. The pointer returned by
 * this function <b>must not</b> be freed.
 *
 * @see eina_strbuf_string_steal()
 */
EAPI const char *eina_strbuf_string_get(const Eina_Strbuf *buf) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Steals the contents of a string buffer.
 *
 * @param buf The string buffer.
 * @return The string that was contained in @p buf.
 *
 * This function returns the string contained in @p buf. @p buf is
 * then initialized and does not own the returned string anymore. The
 * caller must release the memory of the returned string by calling
 * free().
 *
 * @see eina_strbuf_string_get()
 */
EAPI char *eina_strbuf_string_steal(Eina_Strbuf *buf) EINA_MALLOC EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Frees the contents of a string buffer but not the buffer.
 *
 * @param buf The string buffer.
 *
 * This function frees the string contained in @p buf without freeing
 * @p buf.
 */
EAPI void eina_strbuf_string_free(Eina_Strbuf *buf) EINA_ARG_NONNULL(1);

/**
 * @brief Retrieves the length of the string buffer's content.
 *
 * @param buf The string buffer.
 * @return The current length of the string, in bytes.
 *
 * This function returns the length of @p buf.
 */
EAPI size_t eina_strbuf_length_get(const Eina_Strbuf *buf) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Replaces a substring in the buffer with another string.
 *
 * @param buf The string buffer.
 * @param str The text to match.
 * @param with The replacement string.
 * @param n The indexed occurrence to be replaced.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function replaces the n-th occurrence of @p str in @p buf with
 * @p with.
 */
EAPI Eina_Bool eina_strbuf_replace(Eina_Strbuf *buf, const char *str, const char *with, unsigned int n) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @def eina_strbuf_replace_first(buf, str, with)
 * @brief Replaces the first occurance of a substring with another string.
 *
 * @param buf The string buffer.
 * @param str The text to match.
 * @param with The replacement string.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p str could not be
 * replaced in @p buf.
 *
 * This macro simply calls eina_strbuf_replace() with @p n
 * equal to @c 1.
 */
#define eina_strbuf_replace_first(buf, str, with) eina_strbuf_replace(buf, str, with, 1)

/**
 * @brief Replaces all matching substrings with another string.
 *
 * @param buf The string buffer.
 * @param str The text to match.
 * @param with The replacement string.
 * @return Number of replacements made, or @c 0 on failure.
 *
 * This function replaces all the occurrences of @p str in @p buf with
 * the string @p with.
 */
EAPI int eina_strbuf_replace_all(Eina_Strbuf *buf, const char *str, const char *with) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Trims the string buffer.
 *
 * @param buf The string buffer.
 *
 * This function removes whitespace at the beginning and end of the
 * buffer's string.
 */
EAPI void eina_strbuf_trim(Eina_Strbuf *buf) EINA_ARG_NONNULL(1);

/**
 * @brief Left trims the string buffer.
 *
 * @param buf The string buffer.
 *
 * This function removes whitespace at the beginning of the buffer's
 * string.
 */
EAPI void eina_strbuf_ltrim(Eina_Strbuf *buf) EINA_ARG_NONNULL(1);

/**
 * @brief Right trims the string buffer.
 *
 * @param buf The string buffer.
 *
 * This function removes whitespace at the end of the buffer's string.
 */
EAPI void eina_strbuf_rtrim(Eina_Strbuf *buf) EINA_ARG_NONNULL(1);

/**
 * @brief Converts the string to lower case.
 *
 * @param buf The string buffer.
 *
 * This function converts all the characters in the buffer's string to
 * lower case.
 *
 * @since 1.17
 */
EAPI void eina_strbuf_tolower(Eina_Strbuf *buf) EINA_ARG_NONNULL(1);

/**
 * @brief Obtains substring from a source string buffer.
 *
 * @param buf The string buffer.
 * @param pos The (0-based) position where the substring starts.
 * @param len The length from @p pos that should be copied.
 * @return A string buffer containing the substring, or @c NULL if
 * invalid parameters were specified.
 *
 * This function creates a Eina_Strbuf containing a copy of a substring
 * of buf starting at @p pos with @p len length.  If len is 0, then an
 * empty strbuf will be returned.
 *
 * @since 1.17
 */
EAPI Eina_Strbuf * eina_strbuf_substr_get(Eina_Strbuf *buf, size_t pos, size_t len) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Gets a read-only slice of the buffer contents.
 *
 * @param buf The string buffer.
 * @return A read-only slice of the current contents. It may become
 * invalid as soon as @p buf is changed.
 *
 * @since 1.19
 */
EAPI Eina_Slice eina_strbuf_slice_get(const Eina_Strbuf *buf) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Gets a read-write slice of the buffer contents.
 *
 * @param buf The string buffer.
 * @return A read-write slice for the current contents. It may become
 * invalid as soon as the @p buf is changed, such as through calls like
 * eina_strbuf_append() or eina_strbuf_remove().
 *
 * @since 1.19.0
 */
EAPI Eina_Rw_Slice eina_strbuf_rw_slice_get(const Eina_Strbuf *buf) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Frees the buffer, returning its old contents.
 *
 * @param buf The string buffer.*
 * @return The string contained by buf. The caller must release the
 * memory of the returned string by calling free().
 *
 * @since 1.19
 */
EAPI char* eina_strbuf_release(Eina_Strbuf *buf) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Appends a strftime-style timestamp to the buffer.
 *
 * @param buf The string buffer.
 * @param fmt The formatting string.
 * @param tm The time value.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function appends a timestamp to the buffer, formatted using
 * strftime.  A string of the exact size required by strftime's output
 * is added to the end.
 *
 * Example usage:
 * @code
 * time_t curr_time;
 * struct tm *info;
 * Eina_Strbuf *buf = eina_strbuf_new();
 * curr_time = time(NULL);
 * info = localtime(&curr_time);
 * eina_strbuf_append_strftime(buf, "%I:%M%p", info);
 * //after use
 * eina_strbuf_free(buf);
 * @endcode
 *
 * @since 1.21
 */
EAPI Eina_Bool eina_strbuf_append_strftime(Eina_Strbuf *buf, const char *fmt, const struct tm *tm);

/**
 * @brief Inserts a strftime-style timestamp into the buffer.
 *
 * @param buf The string buffer.
 * @param fmt The formatting string.
 * @param tm The time value.
 * @param pos The position in the buffer to insert the timestamp.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function inserts a timestamp into the buffer, formatted using
 * strftime.  The buffer's length will be increased by the exact amount
 * required by strftime's output.
 *
 * Example usage:
 * @code
 * time_t curr_time;
 * struct tm *info;
 * Eina_Strbuf *buf = eina_strbuf_new();
 * curr_time = time(NULL);
 * info = localtime(&curr_time);
 * eina_strbuf_insert_strftime(buf, "%I:%M%p", info, 2);
 * //after use
 * eina_strbuf_free(buf);
 * @endcode
 *
 * @since 1.21
 */
EAPI Eina_Bool eina_strbuf_insert_strftime(Eina_Strbuf *buf, const char *fmt, const struct tm *tm, size_t pos);

/**
 * @def eina_strbuf_prepend_strftime(buf, fmt, tm)
 * @brief Prepends a strftime-style timestamp to the buffer.
 *
 * @param buf The string buffer.
 * @param fmt The formatting string.
 * @param tm The time value.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p tm could not be prepended.
 *
 * This macro simply calls eina_strbuf_insert_strftime() with position 0.
 *
 * @since 1.21
 */
#define eina_strbuf_prepend_strftime(buf, fmt, tm) eina_strbuf_insert_strftime(buf, fmt, tm, 0)

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

#endif /* EINA_STRBUF_H */
