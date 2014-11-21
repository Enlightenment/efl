#ifndef EINA_BINBUF_H
#define EINA_BINBUF_H

#include <stddef.h>
#include <stdarg.h>

#include "eina_types.h"

/**
 * @defgroup Eina_Binary_Buffer_Group Binary Buffer
 * @ingroup Eina_Data_Types_Group
 *
 * @brief This group discusses the functions that provide string buffers management.
 *
 * The Binary Buffer data type is designed to be a mutable string,
 * allowing to append, prepend, or insert a string into a buffer.
 *
 * @{
 */

/**
 * @typedef Eina_Binbuf
 * @brief The structure type for a string buffer.
 */
typedef struct _Eina_Strbuf Eina_Binbuf;

/**
 * @brief Creates a new string buffer.
 *
 * @details This function creates a new string buffer. On error, @c NULL is
 *          returned and Eina error is set to #EINA_ERROR_OUT_OF_MEMORY. To
 *          free the resources, use eina_binbuf_free().
 *
 * @since_tizen 2.3
 *
 * @return Newly allocated string buffer instance.
 *
 * @see eina_binbuf_free()
 * @see eina_binbuf_append()
 * @see eina_binbuf_string_get()
 */
EAPI Eina_Binbuf *eina_binbuf_new(void) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Creates a new string buffer using the passed string. The passed
 *        string is used directly as the buffer, it's somehow the opposite function of
 *        @ref eina_binbuf_string_steal . The passed string must be malloced.
 *
 * @details This function creates a new string buffer. On error, @c NULL is
 *          returned and Eina error is set to #EINA_ERROR_OUT_OF_MEMORY. To
 *          free the resources, use eina_binbuf_free().
 *
 * @since 1.2.0
 *
 * @since_tizen 2.3
 *
 * @param[in] str The string to manage
 * @param[in] length The length of the string
 * @return A newly allocated string buffer instance
 *
 * @see eina_binbuf_manage_new()
 */
EAPI Eina_Binbuf *eina_binbuf_manage_new_length(unsigned char *str, size_t length) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Frees a string buffer.
 *
 * @details This function frees the memory of @a buf. @a buf must have been
 *          created by eina_binbuf_new().
 *
 * @since_tizen 2.3
 *
 * @param[in] buf The string buffer to free
 */
EAPI void eina_binbuf_free(Eina_Binbuf *buf) EINA_ARG_NONNULL(1);

/**
 * @brief Resets a string buffer.
 *
 * @details This function resets @a buf, the buffer length is set to @c 0 and the
 *          string is set to '\\0'. No memory is freed.
 *
 * @since_tizen 2.3
 *
 * @param[in] buf The string buffer to reset
 *
 */
EAPI void eina_binbuf_reset(Eina_Binbuf *buf) EINA_ARG_NONNULL(1);

/**
 * @brief Appends a string of exact length to a buffer, reallocating as necessary.
 *
 * @details This function appends @a str to @a buf. @a str must be at most of size
 *          @a length. It is slightly faster than eina_binbuf_append() as
 *          it does not compute the size of @a str. It is useful when dealing
 *          with strings of known size, such as eina_strngshare. If @a buf
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
 * @see eina_stringshare_length()
 * @see eina_binbuf_append()
 * @see eina_binbuf_append_n()
 */
EAPI Eina_Bool eina_binbuf_append_length(Eina_Binbuf *buf, const unsigned char *str, size_t length) EINA_ARG_NONNULL(1, 2);

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
EAPI Eina_Bool eina_binbuf_append_char(Eina_Binbuf *buf, unsigned char c) EINA_ARG_NONNULL(1);

/**
 * @brief Inserts a string of exact length into a buffer, reallocating as necessary.
 *
 * @details This function inserts @a str into @a buf. @a str must be at most of size
 *          @a length. It is slightly faster than eina_binbuf_insert() as
 *          it does not compute the size of @a str. It is useful when dealing
 *          with strings of known size, such as eina_strngshare. If @a buf
 *          can't insert it, @c EINA_FALSE is returned, otherwise @c EINA_TRUE is
 *          returned.
 *
 * @since_tizen 2.3
 *
 * @param[in] buf The string buffer to insert to
 * @param[in] str The string to insert
 * @param[in] length The exact length to use
 * @param[in] pos The position at which to insert the string
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure
 *
 * @see eina_stringshare_length()
 * @see eina_binbuf_insert()
 * @see eina_binbuf_insert_n()
 */
EAPI Eina_Bool eina_binbuf_insert_length(Eina_Binbuf *buf, const unsigned char *str, size_t length, size_t pos) EINA_ARG_NONNULL(1, 2);

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
 * @param[in] pos The position at which to insert the character
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure
 *
 */
EAPI Eina_Bool eina_binbuf_insert_char(Eina_Binbuf *buf, unsigned char c, size_t pos) EINA_ARG_NONNULL(1);

/**
 * @brief Removes a slice of the given string buffer.
 *
 * @details This function removes a slice of @a buf, starting from @a start
 *          (inclusive) and ending at @a end (non-inclusive). Both values are
 *          in bytes. It returns @c EINA_FALSE on failure, otherwise @c EINA_TRUE.
 *
 * @since_tizen 2.3
 *
 * @param[in] buf The string buffer to remove a slice of
 * @param[in] start The initial (inclusive) slice position to start
 *              removing from, in bytes
 * @param[in] end The final (non-inclusive) slice position to finish
 *            removing at, in bytes
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure
 */

EAPI Eina_Bool eina_binbuf_remove(Eina_Binbuf *buf, size_t start, size_t end) EINA_ARG_NONNULL(1);

/**
 * @brief Gets a pointer to the contents of a string buffer.
 *
 * @details This function returns the string contained in @a buf. The returned
 *          value must not be modified and is no longer valid if @a buf is
 *          modified. In other words, any eina_binbuf_append() or similar
 *          makes that pointer invalid.
 *
 * @since_tizen 2.3
 *
 * @param[in] buf The string buffer
 * @return The current string in the string buffer
 *
 * @see eina_binbuf_string_steal()
 */
EAPI const unsigned char *eina_binbuf_string_get(const Eina_Binbuf *buf) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

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
 * @see eina_binbuf_string_get()
 */
EAPI unsigned char *eina_binbuf_string_steal(Eina_Binbuf *buf) EINA_MALLOC EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Frees the contents of a string buffer but not the buffer.
 *
 * @details This function frees the string contained in @a buf without freeing
 *          @a buf.
 *
 * @since_tizen 2.3
 *
 * @param[in] buf The string buffer to free the string of
 *
 */
EAPI void eina_binbuf_string_free(Eina_Binbuf *buf) EINA_ARG_NONNULL(1);

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
EAPI size_t    eina_binbuf_length_get(const Eina_Binbuf *buf) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @}
 */

#endif /* EINA_STRBUF_H */
