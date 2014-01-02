#ifndef EINA_BINBUF_H
#define EINA_BINBUF_H

#include <stddef.h>
#include <stdarg.h>

#include "eina_types.h"

/**
 * @addtogroup Eina_Binary_Buffer_Group Binary Buffer
 *
 * @brief These functions provide string buffers management.
 *
 * The Binary Buffer data type is designed to be a mutable string,
 * allowing to append, prepend or insert a string to a buffer.
 */

/**
 * @addtogroup Eina_Data_Types_Group Data Types
 *
 * @{
 */

/**
 * @defgroup Eina_Binary_Buffer_Group Binary Buffer
 *
 * @{
 */

/**
 * @typedef Eina_Binbuf
 * Type for a string buffer.
 */
typedef struct _Eina_Strbuf Eina_Binbuf;

/**
 * @brief Create a new string buffer.
 *
 * @return Newly allocated string buffer instance.
 *
 * This function creates a new string buffer. On error, @c NULL is
 * returned. To free the resources, use eina_binbuf_free().
 *
 * @see eina_binbuf_free()
 * @see eina_binbuf_append()
 * @see eina_binbuf_string_get()
 */
EAPI Eina_Binbuf *eina_binbuf_new(void) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Create a new string buffer using the passed string. The passed
 * string is used directly as the buffer, it's somehow the opposite function of
 * @ref eina_binbuf_string_steal . The passed string must be malloced.
 *
 * @param str the string to manage
 * @param length the length of the string.
 * @return Newly allocated string buffer instance.
 *
 * This function creates a new string buffer. On error, @c NULL is
 * returned. To free the resources, use eina_binbuf_free().
 *
 * @see eina_binbuf_manage_new()
 * @since 1.2.0
 */
EAPI Eina_Binbuf *eina_binbuf_manage_new_length(unsigned char *str, size_t length) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Create a new string buffer using the passed string. The passed
 * string is used directly as the buffer, it's somehow the opposite function of
 * @ref eina_binbuf_string_steal . The passed string will not be touched.
 *
 * @param str the string to start from
 * @param length the length of the string.
 * @return Newly allocated string buffer instance.
 *
 * This function creates a new string buffer. On error, @c NULL is
 * returned. To free the resources, use eina_binbuf_free(). It will
 * not touch the internal buffer. Any changing operation will
 * create a fresh new memory, copy old data there and starting modifying
 * that one.
 *
 * @see eina_binbuf_manage_new()
 * @since 1.9.0
 */
EAPI Eina_Binbuf *eina_binbuf_manage_read_only_new_length(const unsigned char *str, size_t length) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Free a string buffer.
 *
 * @param buf The string buffer to free.
 *
 * This function frees the memory of @p buf. @p buf must have been
 * created by eina_binbuf_new().
 */
EAPI void eina_binbuf_free(Eina_Binbuf *buf) EINA_ARG_NONNULL(1);

/**
 * @brief Reset a string buffer.
 *
 * @param buf The string buffer to reset.
 *
 * This function reset @p buf: the buffer len is set to 0, and the
 * string is set to '\\0'. No memory is free'd.
 */
EAPI void eina_binbuf_reset(Eina_Binbuf *buf) EINA_ARG_NONNULL(1);

/**
 * @brief Append a string of exact length to a buffer, reallocating as necessary.
 *
 * @param buf The string buffer to append to.
 * @param str The string to append.
 * @param length The exact length to use.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function appends @p str to @p buf. @p str must be of size at
 * most @p length. It is slightly faster than eina_binbuf_append() as
 * it does not compute the size of @p str. It is useful when dealing
 * with strings of known size, such as eina_strngshare. If @p buf
 * can't append it, #EINA_FALSE is returned, otherwise #EINA_TRUE is
 * returned.
 *
 * @see eina_stringshare_length()
 * @see eina_binbuf_append()
 * @see eina_binbuf_append_n()
 */
EAPI Eina_Bool eina_binbuf_append_length(Eina_Binbuf *buf, const unsigned char *str, size_t length) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Append an Eina_Binbuf to a buffer, reallocating as necessary.
 *
 * @param buf The string buffer to append to.
 * @param data The string buffer to append.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function appends @p data to @p buf. @p data must be allocated and
 * different from @NULL. It is slightly faster than eina_binbuf_append() as
 * it does not compute the size of @p str. If @p buf can't append it,
 * #EINA_FALSE is returned, otherwise #EINA_TRUE is returned.
 *
 * @see eina_binbuf_append()
 * @see eina_binbuf_append_n()
 * @see eina_binbuf_append_length()
 * @since 1.9.0
 */
EAPI Eina_Bool eina_binbuf_append_buffer(Eina_Binbuf *buf, const Eina_Binbuf *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Append a character to a string buffer, reallocating as
 * necessary.
 *
 * @param buf The string buffer to append to.
 * @param c The char to append.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function inserts @p c to @p buf. If it can not insert it, #EINA_FALSE
 * is returned, otherwise #EINA_TRUE is returned.
 */
EAPI Eina_Bool eina_binbuf_append_char(Eina_Binbuf *buf, unsigned char c) EINA_ARG_NONNULL(1);

/**
 * @brief Insert a string of exact length to a buffer, reallocating as necessary.
 *
 * @param buf The string buffer to insert to.
 * @param str The string to insert.
 * @param length The exact length to use.
 * @param pos The position to insert the string.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function inserts @p str to @p buf. @p str must be of size at
 * most @p length. It is slightly faster than eina_binbuf_insert() as
 * it does not compute the size of @p str. It is useful when dealing
 * with strings of known size, such as eina_strngshare. If @p buf
 * can't insert it, #EINA_FALSE is returned, otherwise #EINA_TRUE is
 * returned.
 *
 * @see eina_stringshare_length()
 * @see eina_binbuf_insert()
 * @see eina_binbuf_insert_n()
 */
EAPI Eina_Bool eina_binbuf_insert_length(Eina_Binbuf *buf, const unsigned char *str, size_t length, size_t pos) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Insert a character to a string buffer, reallocating as
 * necessary.
 *
 * @param buf The string buffer to insert to.
 * @param c The char to insert.
 * @param pos The position to insert the char.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function inserts @p c to @p buf at position @p pos. If @p buf
 * can't append it, #EINA_FALSE is returned, otherwise #EINA_TRUE is
 * returned.
 */
EAPI Eina_Bool eina_binbuf_insert_char(Eina_Binbuf *buf, unsigned char c, size_t pos) EINA_ARG_NONNULL(1);

/**
 * @brief Remove a slice of the given string buffer.
 *
 * @param buf The string buffer to remove a slice.
 * @param start The initial (inclusive) slice position to start
 *        removing, in bytes.
 * @param end The final (non-inclusive) slice position to finish
 *        removing, in bytes.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function removes a slice of @p buf, starting at @p start
 * (inclusive) and ending at @p end (non-inclusive). Both values are
 * in bytes. It returns #EINA_FALSE on failure, #EINA_TRUE otherwise.
 */

EAPI Eina_Bool eina_binbuf_remove(Eina_Binbuf *buf, size_t start, size_t end) EINA_ARG_NONNULL(1);

/**
 * @brief Retrieve a pointer to the contents of a string buffer
 *
 * @param buf The string buffer.
 * @return The current string in the string buffer.
 *
 * This function returns the string contained in @p buf. The returned
 * value must not be modified and will no longer be valid if @p buf is
 * modified. In other words, any eina_binbuf_append() or similar will
 * make that pointer invalid.
 *
 * @see eina_binbuf_string_steal()
 */
EAPI const unsigned char *eina_binbuf_string_get(const Eina_Binbuf *buf) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Steal the contents of a string buffer.
 *
 * @param buf The string buffer to steal.
 * @return The current string in the string buffer.
 *
 * This function returns the string contained in @p buf. @p buf is
 * then initialized and does not own the returned string anymore. The
 * caller must release the memory of the returned string by calling
 * free().
 *
 * @see eina_binbuf_string_get()
 */
EAPI unsigned char *eina_binbuf_string_steal(Eina_Binbuf *buf) EINA_MALLOC EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Free the contents of a string buffer but not the buffer.
 *
 * @param buf The string buffer to free the string of.
 *
 * This function frees the string contained in @p buf without freeing
 * @p buf.
 */
EAPI void eina_binbuf_string_free(Eina_Binbuf *buf) EINA_ARG_NONNULL(1);

/**
 * @brief Retrieve the length of the string buffer content.
 *
 * @param buf The string buffer.
 * @return The current length of the string, in bytes.
 *
 * This function returns the length of @p buf.
 */
EAPI size_t    eina_binbuf_length_get(const Eina_Binbuf *buf) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @}
 */

/**
 * @}
 */

#endif /* EINA_STRBUF_H */
