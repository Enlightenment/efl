#ifndef EINA_STRBUF_H
#define EINA_STRBUF_H

#include <stddef.h>
#include <stdarg.h>

#include "eina_types.h"

/**
 * @defgroup Eina_String_Buffer_Group String Buffer
 * @ingroup Eina_Data_Types_Group
 *
 * @brief This group discusses the functions that provide string buffers management.
 *
 * @remarks The String Buffer data type is designed to be a mutable string,
 *          allowing to append, prepend, or insert a string into a buffer.
 *
 * @{
 */

/**
 * @typedef Eina_Strbuf
 * @brief The structure type for a string buffer.
 */
typedef struct _Eina_Strbuf Eina_Strbuf;

/**
 * @brief Creates a new string buffer.
 *
 * @details This function creates a new string buffer. On error, @c NULL is
 *          returned and Eina error is set to #EINA_ERROR_OUT_OF_MEMORY. To
 *          free the resources, use eina_strbuf_free().
 *
 * @since_tizen 2.3
 *
 * @return A newly allocated string buffer instance
 *
 * @see eina_strbuf_free()
 * @see eina_strbuf_append()
 * @see eina_strbuf_string_get()
 */
EAPI Eina_Strbuf *eina_strbuf_new(void) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Creates a new string buffer using the passed string. The passed
 *        string is used directly as the buffer, it's somehow the opposite function of
 *        @ref eina_strbuf_string_steal. The passed string must be malloced.
 *
 * @details This function creates a new string buffer. On error, @c NULL is
 *          returned and Eina error is set to #EINA_ERROR_OUT_OF_MEMORY. To
 *          free the resources, use eina_strbuf_free().
 *
 * @since 1.1.0
 *
 * @since_tizen 2.3
 *
 * @param[in] str The string to manage
 * @return A newly allocated string buffer instance
 *
 * @see eina_strbuf_free()
 * @see eina_strbuf_append()
 * @see eina_strbuf_string_get()
 */
EAPI Eina_Strbuf *eina_strbuf_manage_new(char *str) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Creates a new string buffer using the passed string. The passed
 *        string is used directly as the buffer, it's somehow the opposite function of
 *        @ref eina_strbuf_string_steal. The passed string must be malloced.
 *
 * @details This function creates a new string buffer. On error, @c NULL is
 *          returned and Eina error is set to #EINA_ERROR_OUT_OF_MEMORY. To
 *          free the resources, use eina_strbuf_free().
 *
 * @since 1.2.0
 *
 * @since_tizen 2.3
 *
 * @param[in] str The string to manage
 * @param[in] length The length of the string
 * @return A newly allocated string buffer instance
 *
 * @see eina_strbuf_manage_new()
 */
EAPI Eina_Strbuf *eina_strbuf_manage_new_length(char *str, size_t length) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Frees a string buffer.
 *
 * @details This function frees the memory of @a buf. @a buf must have been
 *          created by eina_strbuf_new().
 *
 * @since_tizen 2.3
 *
 * @param[in] buf The string buffer to free
 */
EAPI void eina_strbuf_free(Eina_Strbuf *buf) EINA_ARG_NONNULL(1);

/**
 * @brief Resets a string buffer.
 *
 * @details This function resets @a buf: the buffer length is set to 0, and the
 *          string is set to '\\0'. No memory is freed.
 *
 * @since_tizen 2.3
 *
 * @param[in] buf The string buffer to reset
 */
EAPI void eina_strbuf_reset(Eina_Strbuf *buf) EINA_ARG_NONNULL(1);

/**
 * @brief Appends a string to a buffer, reallocating as necessary.
 *
 * @details This function appends @a str to @a buf. It computes the length of
 *          @a str, so is slightly slower than eina_strbuf_append_length(). If
 *          the length is known beforehand, consider using that variant. If
 *          @a buf can't append it, @c EINA_FALSE is returned, otherwise @c EINA_TRUE is
 *          returned.
 *
 * @since_tizen 2.3
 *
 * @param[in] buf The string buffer to append to
 * @param[in] str The string to append
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure
 *
 * @see eina_strbuf_append()
 * @see eina_strbuf_append_length()
 */
EAPI Eina_Bool eina_strbuf_append(Eina_Strbuf *buf, const char *str) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Appends an escaped string to a buffer, reallocating as necessary.
 *
 * @details This function escapes and then appends the string @a str to @a buf. If @a str
 *          cannot be appended, @c EINA_FALSE is returned, otherwise, @c EINA_TRUE is
 *          returned.
 *
 * @since_tizen 2.3
 *
 * @param[in] buf The string buffer to append to
 * @param[in] str The string to append
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure
 *
 */
EAPI Eina_Bool eina_strbuf_append_escaped(Eina_Strbuf *buf, const char *str) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Appends a string to a buffer, reallocating as necessary,
 *        limited by the given length.
 *
 * @details This function appends at most @a maxlen characters of @a str to
 *          @a buf. It can't append more than the length of @a str. It
 *          computes the length of @a str, so it is slightly slower than
 *          eina_strbuf_append_length(). If the length is known beforehand,
 *          consider using that variant (@a maxlen should then be checked so
 *          that it is greater than the size of @a str). If @a str cannot be
 *          appended, @c EINA_FALSE is returned, otherwise @c EINA_TRUE is
 *          returned.
 *
 * @since_tizen 2.3
 *
 * @param[in] buf The string buffer to append to
 * @param[in] str The string to append
 * @param[in] maxlen The maximum number of characters to append
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure
 *
 * @see eina_strbuf_append()
 * @see eina_strbuf_append_length()
 */
EAPI Eina_Bool eina_strbuf_append_n(Eina_Strbuf *buf, const char *str, size_t maxlen) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Appends a string of exact length to a buffer, reallocating as necessary.
 *
 * @details This function appends @a str to @a buf. @a str must be at most of size
 *          @a length. It is slightly faster than eina_strbuf_append() as
 *          it does not compute the size of @a str. It is useful when dealing
 *          with strings of known size, such as eina_stringshare. If @a buf
 *          can't append it, @c EINA_FALSE is returned, otherwise @c EINA_TRUE is
 *          returned.
 *
 * @since_tizen 2.3
 *
 * @param[in] buf The string buffer to append to
 * @param[in] str The string to append
 * @param[in] length The exact length to use
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure
 *
 *
 * @see eina_stringshare_length()
 * @see eina_strbuf_append()
 * @see eina_strbuf_append_n()
 */
EAPI Eina_Bool eina_strbuf_append_length(Eina_Strbuf *buf, const char *str, size_t length) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Appends a character to a string buffer, reallocating as
 *        necessary.
 *
 * @details This function inserts @a c to @a buf. If it cannot insert it, @c EINA_FALSE
 *          is returned, otherwise @c EINA_TRUE is returned.
 *
 * @since_tizen 2.3
 *
 * @param[in] buf The string buffer to append to
 * @param[in] c The character to append
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure
 *
 */
EAPI Eina_Bool eina_strbuf_append_char(Eina_Strbuf *buf, char c) EINA_ARG_NONNULL(1);

/**
 * @brief Appends a string to a buffer, reallocating as necessary.
 *
 * @details This function appends the string defined by the format @a fmt to @a buf. @a
 *          fmt must be of a valid format from the printf family of functions. If it can't
 *          insert it, @c EINA_FALSE is returned, otherwise @c EINA_TRUE is returned.
 *
 * @since_tizen 2.3
 *
 * @param[in] buf The string buffer to append to
 * @param[in] fmt The string to append
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure
 *
 * @see eina_strbuf_append()
 */
EAPI Eina_Bool eina_strbuf_append_printf(Eina_Strbuf *buf, const char *fmt, ...) EINA_ARG_NONNULL(1, 2) EINA_PRINTF(2, 3);

/**
 * @brief Appends a string to a buffer, reallocating as necessary.
 *
 * @since_tizen 2.3
 *
 * @param[in] buf The string buffer to append to
 * @param[in] fmt The string to append
 * @param[in] args The variable arguments
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure
 *
 * @see eina_strbuf_append_printf()
 */
EAPI Eina_Bool eina_strbuf_append_vprintf(Eina_Strbuf *buf, const char *fmt, va_list args) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Inserts a string into a buffer, reallocating as necessary.
 *
 * @details This function inserts @a str into @a buf at position @a pos. It
 *          computes the length of @a str, so is slightly slower than
 *          eina_strbuf_insert_length(). If  the length is known beforehand,
 *          consider using that variant. If @a buf can't insert it, @c EINA_FALSE
 *
 * @since_tizen 2.3
 *
 * @param[in] buf The string buffer to insert into
 * @param[in] str The string to insert
 * @param[in] pos The position at which to insert the string
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure
 *
 */
EAPI Eina_Bool eina_strbuf_insert(Eina_Strbuf *buf, const char *str, size_t pos) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Inserts an escaped string into a buffer, reallocating as
 *        necessary.
 *
 * @details This function escapes and inserts the string @a str into @a buf at
 *          position @a pos. If @a buf can't insert @a str, @c EINA_FALSE is
 *          returned, otherwise @c EINA_TRUE is returned.
 *
 * @since_tizen 2.3
 *
 * @param[in] buf The string buffer to insert into
 * @param[in] str The string to insert
 * @param[in] pos The position at which to insert the string
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure
 *
 */
EAPI Eina_Bool eina_strbuf_insert_escaped(Eina_Strbuf *buf, const char *str, size_t pos) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Inserts a string into a buffer, reallocating as necessary. Limited by maxlen.
 *
 * @details This function inserts @a str into @a buf at position @a pos, with at
 *          most @a maxlen bytes. The number of inserted characters cannot be
 *          greater than the length of @a str. It computes the length of
 *          @a str, so is slightly slower than eina_strbuf_insert_length(). If the
 *          length is known beforehand, consider using that variant (@a maxlen
 *          should then be checked so that it is greater than the size of
 *          @a str). If @a str cannot be inserted, @c EINA_FALSE is returned,
 *          otherwise @c EINA_TRUE is returned.
 *
 * @since_tizen 2.3
 *
 * @param[in] buf The string buffer to insert into
 * @param[in] str The string to insert
 * @param[in] maxlen The maximum number of chars to insert
 * @param[in] pos The position at which to insert the string
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure
 *
 */
EAPI Eina_Bool eina_strbuf_insert_n(Eina_Strbuf *buf, const char *str, size_t maxlen, size_t pos) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Inserts a string of exact length into a buffer, reallocating as necessary.
 *
 * @details This function inserts @a str into @a buf. @a str must be at most of size
 *          @a length. It is slightly faster than eina_strbuf_insert() as
 *          it does not compute the size of @a str. It is useful when dealing
 *          with strings of known size, such as eina_strngshare. If @a buf
 *          can't insert it, @c EINA_FALSE is returned, otherwise @c EINA_TRUE is
 *          returned.
 *
 * @since_tizen 2.3
 *
 * @param[in] buf The string buffer to insert into
 * @param[in] str The string to insert
 * @param[in] length The exact length to use
 * @param[in] pos The position at which to insert the string
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure
 *
 * @see eina_stringshare_length()
 * @see eina_strbuf_insert()
 * @see eina_strbuf_insert_n()
 */
EAPI Eina_Bool eina_strbuf_insert_length(Eina_Strbuf *buf, const char *str, size_t length, size_t pos) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Inserts a character into a string buffer, reallocating as
 *        necessary.
 *
 * @details This function inserts @a c into @a buf at position @a pos. If @a buf
 *          can't append it, @c EINA_FALSE is returned, otherwise @c EINA_TRUE is
 *          returned.
 *
 * @since_tizen 2.3
 *
 * @param[in] buf The string buffer to insert into
 * @param[in] c The character to insert
 * @param[in] pos The position at which to insert the char
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure
 *
 */
EAPI Eina_Bool eina_strbuf_insert_char(Eina_Strbuf *buf, char c, size_t pos) EINA_ARG_NONNULL(1);

/**
 * @brief Inserts a string into a buffer, reallocating as necessary.
 *
 * @details This function inserts a string as described by the format @a fmt into @a buf at
 *          the position @a pos. @a fmt must be of a valid format from the printf family of
 *          functions. If it can't insert it, @c EINA_FALSE is returned,
 *          otherwise @c EINA_TRUE is returned.
 *
 * @since_tizen 2.3
 *
 * @param[in] buf The string buffer to insert into
 * @param[in] fmt The string to insert
 * @param[in] pos The position at which to insert the string
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure
 *
 */
EAPI Eina_Bool eina_strbuf_insert_printf(Eina_Strbuf *buf, const char *fmt, size_t pos, ...) EINA_ARG_NONNULL(1, 2) EINA_PRINTF(2, 4);

/**
 * @brief Inserts a string into a buffer, reallocating as necessary.
 *
 * @since_tizen 2.3
 *
 * @param[in] buf The string buffer to insert into
 * @param[in] fmt The string to insert
 * @param[in] pos The position at which to insert the string
 * @param[in] args The variable arguments
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure
 *
 * @see eina_strbuf_insert_printf
 */
EAPI Eina_Bool eina_strbuf_insert_vprintf(Eina_Strbuf *buf, const char *fmt, size_t pos, va_list args) EINA_ARG_NONNULL(1, 2);

/**
 * @def eina_strbuf_prepend(buf, str)
 * @brief Prepends the given string to the given buffer.
 *
 * @details This macro calls eina_strbuf_insert() at position 0. If @a buf
 *          can't prepend it, @c EINA_FALSE is returned, otherwise @c EINA_TRUE is
 *          returned.
 *
 * @since_tizen 2.3
 *
 * @param buf The string buffer to prepend to
 * @param str The string to prepend
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure
 *
 */
#define eina_strbuf_prepend(buf, str)                eina_strbuf_insert(buf, str, 0)

/**
 * @def eina_strbuf_prepend_escaped(buf, str)
 * @brief Prepends the given escaped string to the given buffer.
 *
 * @details This macro calls eina_strbuf_insert_escaped() at position 0. If
 *          @a buf can't prepend it, @c EINA_FALSE is returned, otherwise @c EINA_TRUE is
 *          returned.
 *
 * @since_tizen 2.3
 *
 * @param buf The string buffer to prepend to
 * @param str The string to prepend
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure
 *
 */
#define eina_strbuf_prepend_escaped(buf, str)        eina_strbuf_insert_escaped(buf, str, 0)

/**
 * @def eina_strbuf_prepend_n(buf, str)
 * @brief Prepends the given escaped string to the given buffer.
 *
 * @details This macro calls eina_strbuf_insert_n() at position 0. If
 *          @a buf can't prepend it, @c EINA_FALSE is returned, otherwise @c EINA_TRUE is
 *          returned.
 *
 * @since_tizen 2.3
 *
 * @param buf The string buffer to prepend to
 * @param str The string to prepend
 * @param maxlen The maximum number of chars to prepend
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure
 *
 */
#define eina_strbuf_prepend_n(buf, str, maxlen)      eina_strbuf_insert_n(buf, str, maxlen, 0)

/**
 * @def eina_strbuf_prepend_length(buf, str)
 * @brief Prepends the given escaped string to the given buffer.
 *
 * @details This macro calls eina_strbuf_insert_length() at position 0. If
 *          @a buf can't prepend it, @c EINA_FALSE is returned, otherwise @c EINA_TRUE is
 *          returned.
 *
 * @since_tizen 2.3
 *
 * @param buf The string buffer to prepend to
 * @param str The string to prepend
 * @param length The exact length to use
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure
 *
 */
#define eina_strbuf_prepend_length(buf, str, length) eina_strbuf_insert_length(buf, str, length, 0)

/**
 * @def eina_strbuf_prepend_char(buf, str)
 * @brief Prepends the given character to the given buffer.
 *
 * @details This macro calls eina_strbuf_insert_char() at position 0. If
 *          @a buf can't prepend it, @c EINA_FALSE is returned, otherwise @c EINA_TRUE
 *          is returned.
 *
 * @since_tizen 2.3
 *
 * @param buf The string buffer to prepend to
 * @param c The character to prepend
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure.
 *
 */
#define eina_strbuf_prepend_char(buf, c)             eina_strbuf_insert_char(buf, c, 0)

/**
 * @def eina_strbuf_prepend_printf(buf, fmt, ...)
 * @brief Prepends the given string to the given buffer.
 *
 * @details This macro calls eina_strbuf_insert_printf() at position 0. If @a buf
 *          can't prepend it, @c EINA_FALSE is returned, otherwise @c EINA_TRUE is
 *          returned.
 *
 * @since_tizen 2.3
 *
 * @param buf The string buffer to prepend to
 * @param fmt The string to prepend
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure
 *
 */
#define eina_strbuf_prepend_printf(buf, fmt, ...)    eina_strbuf_insert_printf(buf, fmt, 0, ## __VA_ARGS__)

/**
 * @def eina_strbuf_prepend_vprintf(buf, fmt, args)
 * @brief Prepends the given string to the given buffer.
 *
 * @details This macro calls eina_strbuf_insert_vprintf() at position 0. If @a buf
 *          can't prepend it, @c EINA_FALSE is returned, otherwise @c EINA_TRUE is
 *          returned.
 *
 * @since_tizen 2.3
 *
 * @param buf The string buffer to prepend to
 * @param fmt The string to prepend
 * @param args The variable arguments
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure
 *
 */
#define eina_strbuf_prepend_vprintf(buf, fmt, args)  eina_strbuf_insert_vprintf(buf, fmt, 0, args)

/**
 * @brief Removes a slice of the given string buffer.
 *
 * @details This function removes a slice of @a buf, starting from @a start
 *          (inclusive) and ending at @a end (non-inclusive). Both the values are
 *          in bytes. It returns @c EINA_FALSE on failure, otherwise it returns @c EINA_TRUE.
 *
 * @since_tizen 2.3
 *
 * @param buf The string buffer to remove a slice of
 * @param start The initial (inclusive) slice position to start
 *              removing from, in bytes
 * @param end The final (non-inclusive) slice position to finish
 *            removing at, in bytes
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure
 *
 */

EAPI Eina_Bool eina_strbuf_remove(Eina_Strbuf *buf, size_t start, size_t end) EINA_ARG_NONNULL(1);

/**
 * @brief Gets a pointer to the contents of a string buffer.
 *
 * @details This function returns the string contained in @a buf. The returned
 *          value must not be modified as it is longer valid if @a buf is
 *          modified. In other words, any eina_strbuf_append() or similar
 *          makes that pointer invalid. The pointer returned by this function <b>must
 *          not</b> be freed.
 *
 * @since_tizen 2.3
 *
 * @param[in] buf The string buffer
 * @return The current string in the string buffer
 *
 * @see eina_strbuf_string_steal()
 */
EAPI const char *eina_strbuf_string_get(const Eina_Strbuf *buf) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Steals the contents of a string buffer.
 *
 * @details This function returns the string contained in @a buf. @a buf is
 *          then initialized and does not own the returned string anymore. The
 *          caller must release the memory of the returned string by calling
 *          free().
 *
 * @since_tizen 2.3
 *
 * @param[in] buf The string buffer to steal from
 * @return The current string in the string buffer
 *
 * @see eina_strbuf_string_get()
 */
EAPI char *eina_strbuf_string_steal(Eina_Strbuf *buf) EINA_MALLOC EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Frees the contents of a string buffer but not the buffer.
 *
 * @details This function frees the string contained in @a buf without freeing
 *          @a buf.
 *
 * @since_tizen 2.3
 *
 * @param[in] buf The string buffer to free the string from
 *
 */
EAPI void eina_strbuf_string_free(Eina_Strbuf *buf) EINA_ARG_NONNULL(1);

/**
 * @brief Gets the length of the string buffer's content.
 *
 * @details This function returns the length of @a buf.
 *
 * @since_tizen 2.3
 *
 * @param[in] buf The string buffer
 * @return The current length of the string, in bytes
 *
 */
EAPI size_t    eina_strbuf_length_get(const Eina_Strbuf *buf) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Replaces the nth string with another string.
 *
 * @details This function replaces the nth occurrence of @a str in @a buf with
 *          @a with. It returns @c EINA_FALSE on failure, otherwise @c EINA_TRUE.
 *
 * @since_tizen 2.3
 *
 * @param[in] buf The string buffer to work with
 * @param[in] str The string to replace
 * @param[in] with The string to replace with
 * @param[in] n The number of the fitting string
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure
 *
 */
EAPI Eina_Bool eina_strbuf_replace(Eina_Strbuf *buf, const char *str, const char *with, unsigned int n) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @def eina_strbuf_replace_first(buf, str, with)
 * @brief Prepends the given character to the given buffer.
 *
 * @details This macro calls eina_strbuf_replace() with the nth occurrence
 *          equal to @c 1. If @a buf can't replace it, @c EINA_FALSE is returned,
 *          otherwise @c EINA_TRUE is returned.
 *
 * @since_tizen 2.3
 *
 * @param buf The string buffer to work with
 * @param str The string to replace
 * @param with The string to replace with
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure
 *
 */
#define eina_strbuf_replace_first(buf, str, with) eina_strbuf_replace(buf, str, with, 1)


/**
 * @brief Replaces all strings with another string.
 *
 * @details This function replaces all the occurrences of @a str in @a buf with
 *          the string @a with. This function returns the number of times @a str
 *          has been replaced. On failure, it returns @c 0.
 *
 * @since_tizen 2.3
 *
 * @param[in] buf The string buffer to work with
 * @param[in] str The string to replace
 * @param[in] with The string to replace with
 * @return The frequency with which the string is replaced
 *
 */
EAPI int eina_strbuf_replace_all(Eina_Strbuf *buf, const char *str, const char *with) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Trims the string buffer.
 *
 * @details This function skips whitespaces at the beginning and the end of the buffer.
 *
 * @since_tizen 2.3
 *
 * @param[in] buf The string buffer to work with
 *
 */
EAPI void eina_strbuf_trim(Eina_Strbuf *buf) EINA_ARG_NONNULL(1);

/**
 * @brief Left trims the string buffer.
 *
 * @details This function skips whitespaces at the beginning of the buffer.
 *
 * @since_tizen 2.3
 *
 * @param[in] buf The string buffer to work with
 *
 */
EAPI void eina_strbuf_ltrim(Eina_Strbuf *buf) EINA_ARG_NONNULL(1);

/**
 * @brief Right trims the string buffer.
 *
 * @details This function skips whitespaces at the end of the buffer.
 *
 * @since_tizen 2.3
 *
 * @param[in] buf The string buffer to work with
 *
 */
EAPI void eina_strbuf_rtrim(Eina_Strbuf *buf) EINA_ARG_NONNULL(1);

/**
 * @}
 */

#endif /* EINA_STRBUF_H */
