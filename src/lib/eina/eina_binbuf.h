#ifndef EINA_BINBUF_H
#define EINA_BINBUF_H

#include <stddef.h>
#include <stdarg.h>

#include "eina_types.h"
#include "eina_slice.h"

/**
 * @addtogroup Eina_Binary_Buffer_Group Binary Buffer
 *
 * @brief These functions provide string buffer management.
 *
 * The Binary Buffer data type is designed to be a mutable string,
 * allowing the appending, prepending or insertion of a string to a
 * buffer.
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
 * Type for a binary string buffer.
 */
typedef struct _Eina_Strbuf Eina_Binbuf;

/**
 * @brief Creates a new binary string buffer.
 *
 * @return Newly allocated string buffer instance, or @c NULL on error.
 *
 * This function creates a new string buffer. To free the resources, use
 * eina_binbuf_free().
 *
 * @see eina_binbuf_free()
 * @see eina_binbuf_append()
 * @see eina_binbuf_string_get()
 */
EAPI Eina_Binbuf *eina_binbuf_new(void) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Creates a new string buffer using the passed string.
 *
 * @param[in] str The string to manage.
 * @param[in] length The length of the string.
 * @param[in] ro The passed string will not be touched if set to #EINA_TRUE.
 * @return Newly allocated string buffer instance, or @c NULL on error.
 *
 * This function creates a new binary string buffer. The passed string
 * is used directly as the buffer, it's effectively the inverse of
 * eina_binbuf_string_steal().
 *
 * If @p ro is #EINA_TRUE, then @p str will be treated as read-only.
 * Other routines using this buffer that would normally modify the
 * string, will instead create a separate copy to modify and return.
 *
 * To free the resources, use eina_binbuf_free().
 *
 * @see eina_binbuf_manage_new()
 * @see eina_binbuf_manage_new_length()
 * @see eina_binbuf_manage_read_only_new_length()
 *
 * @since 1.14.0
 */
EAPI Eina_Binbuf *eina_binbuf_manage_new(const unsigned char *str, size_t length, Eina_Bool ro) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Creates a new string buffer using the passed string.
 *
 * @param[in] str The string to manage.
 * @param[in] length The length of the string.
 * @return Newly allocated string buffer instance, or @c NULL on error.
 *
 * This function creates a new string buffer. To free the resources, use
 * eina_binbuf_free().
 *
 * @see eina_binbuf_manage_new()
 *
 * @since 1.2.0
 */
EAPI Eina_Binbuf *eina_binbuf_manage_new_length(unsigned char *str, size_t length) EINA_MALLOC EINA_WARN_UNUSED_RESULT EINA_DEPRECATED;

/**
 * @brief Creates a new string buffer using the passed string.
 *
 * @param[in] str The string to start from.
 * @param[in] length The length of the string.
 * @return Newly allocated string buffer instance, or @c NULL on error.
 *
 * This function creates a new read-only string buffer.  The passed
 * string is used directly as the buffer, it's effectively the inverse
 * of eina_binbuf_string_steal().  Other routines using this buffer that
 * would normally modify the string, will instead create a separate copy
 * to modify and return.
 *
 * To free the resources, use eina_binbuf_free().
 *
 * @see eina_binbuf_manage_new()
 *
 * @since 1.9.0
 */
EAPI Eina_Binbuf *eina_binbuf_manage_read_only_new_length(const unsigned char *str, size_t length) EINA_MALLOC EINA_WARN_UNUSED_RESULT EINA_DEPRECATED;

/**
 * @brief Frees a string buffer.
 *
 * @param[in,out] buf The string buffer.
 *
 * This function frees the memory of @p buf. @p buf must have been
 * created by eina_binbuf_new().
 */
EAPI void eina_binbuf_free(Eina_Binbuf *buf) EINA_ARG_NONNULL(1);

/**
 * @brief Resets a string buffer.
 *
 * @param[in,out] buf The string buffer.
 *
 * This function resets @p buf: the buffer len is set to 0, and the
 * string is set to '\\0'. No memory is freed.
 */
EAPI void eina_binbuf_reset(Eina_Binbuf *buf) EINA_ARG_NONNULL(1);

/**
 * @brief Expands a buffer, making room for at least @p minimum_unused_space.
 *
 * @param[in,out] buf The Buffer to expand.
 * @param[in] minimum_unused_space The minimum unused allocated space, in
 *        bytes, at the end of the buffer. Zero can be used to query
 *        the available slice of unused bytes.
 * @return The slice of unused bytes. The slice length may be zero if
 * @p minimum_unused_space couldn't be allocated, otherwise it will
 * be at least @p minimum_unused_space. After bytes are used, mark
 * them as such using eina_binbuf_use().
 *
 * One of the properties of the buffer is that it may overallocate
 * space, thus it may have more than eina_binbuf_length_get() bytes
 * allocated. How much depends on the buffer growth logic, but this
 * function allows requesting a minimum amount of bytes to be allocated
 * at the end of the buffer.
 *
 * This is particularly useful to write directly to buffer's memory
 * (i.e.: a call to read(2)). After the bytes are used call
 * eina_binbuf_use() to mark them as such, so eina_binbuf_length_get()
 * will account for the new bytes.
 *
 * @see eina_binbuf_rw_slice_get()
 * @see eina_binbuf_use()
 *
 * @since 1.19
 */
EAPI Eina_Rw_Slice eina_binbuf_expand(Eina_Binbuf *buf, size_t minimum_unused_space) EINA_ARG_NONNULL(1);

/**
 * @brief Marks more bytes as used.
 *
 * @param[in,out] buf The string buffer.
 * @param[in] extra_bytes the number of bytes to be considered used, must
 *        be between zero and the length of the slice returned by
 *        eina_binbuf_expand().
 * @return #EINA_TRUE on success, #EINA_FALSE on failure, such as
 * @p extra_bytes is too big or @a buf is NULL.
 *
 * This function should be used after eina_binbuf_expand(), marking
 * the extra bytes returned there as used, then they will be
 * taken into account by other functions, such as eina_binbuf_length_get().
 *
 * @see eina_binbuf_expand()
 *
 * @since 1.19
 */
EAPI Eina_Bool eina_binbuf_use(Eina_Binbuf *buf, size_t extra_bytes) EINA_ARG_NONNULL(1);

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
 * most @p length. It is slightly faster than eina_binbuf_append() as
 * it does not compute the size of @p str. It is useful when dealing
 * with strings of known size, such as eina_stringshare.
 *
 * @see eina_stringshare_length()
 * @see eina_binbuf_append()
 * @see eina_binbuf_append_n()
 */
EAPI Eina_Bool eina_binbuf_append_length(Eina_Binbuf *buf, const unsigned char *str, size_t length) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Appends a slice to a buffer, reallocating as necessary.
 *
 * @param[in,out] buf The string buffer.
 * @param[in] slice The slice to append.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p slice could not be appended.
 *
 * This function appends @p slice to @p buf.
 *
 * @since 1.19
 */
EAPI Eina_Bool eina_binbuf_append_slice(Eina_Binbuf *buf, const Eina_Slice slice) EINA_ARG_NONNULL(1);

/**
 * @brief Appends an Eina_Binbuf to a buffer, reallocating as necessary.
 *
 * @param[in,out] buf The string buffer.
 * @param[in] data The string buffer to append.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p data could not be appended.
 *
 * This function appends @p data to @p buf. @p data must be allocated and
 * different from @c NULL. It is slightly faster than eina_binbuf_append() as
 * it does not compute the size of @p data.
 *
 * @see eina_binbuf_append()
 * @see eina_binbuf_append_n()
 * @see eina_binbuf_append_length()
 *
 * @since 1.9.0
 */
EAPI Eina_Bool eina_binbuf_append_buffer(Eina_Binbuf *buf, const Eina_Binbuf *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Appends a character to a string buffer, reallocating as
 * necessary.
 *
 * @param[in,out] buf The string buffer.
 * @param[in] c The char to append.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p c could not be inserted.
 *
 * This function appends @p c to @p buf.
 */
EAPI Eina_Bool eina_binbuf_append_char(Eina_Binbuf *buf, unsigned char c) EINA_ARG_NONNULL(1);

/**
 * @brief Inserts a string of exact length into a buffer, reallocating as necessary.
 *
 * @param[in,out] buf The string buffer.
 * @param[in] str The string to insert.
 * @param[in] length The exact length to use.
 * @param[in] pos The position to insert the string.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p str could not be inserted.
 *
 * This function inserts @p str into @p buf. @p str must be no longer
 * than @p length. It is slightly faster than eina_binbuf_insert() as
 * it does not compute the size of @p str. It is useful when dealing
 * with strings of known size, such as eina_stringshare.
 *
 * @see eina_stringshare_length()
 * @see eina_binbuf_insert()
 * @see eina_binbuf_insert_n()
 */
EAPI Eina_Bool eina_binbuf_insert_length(Eina_Binbuf *buf, const unsigned char *str, size_t length, size_t pos) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Inserts a slice into a buffer, reallocating as necessary.
 *
 * @param[in,out] buf The string buffer.
 * @param[in] slice The slice to insert.
 * @param[in] pos The position to insert the string.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p slice could not be inserted.
 *
 * This function inserts @p slice into @p buf at position @p pos.
 *
 * @since 1.19.0
 */
EAPI Eina_Bool eina_binbuf_insert_slice(Eina_Binbuf *buf, const Eina_Slice slice, size_t pos) EINA_ARG_NONNULL(1);

/**
 * @brief Inserts a character into a string buffer, reallocating as
 * necessary.
 *
 * @param[in,out] buf The string buffer.
 * @param[in] c The char to insert.
 * @param[in] pos The position to insert the char.
 * @return #EINA_TRUE on success, #EINA_FALSE if @p c could not be inserted.
 *
 * This function inserts @p c to @p buf at position @p pos.
 */
EAPI Eina_Bool eina_binbuf_insert_char(Eina_Binbuf *buf, unsigned char c, size_t pos) EINA_ARG_NONNULL(1);

/**
 * @brief Removes a slice of the given string buffer.
 *
 * @param[in,out] buf The string buffer.
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
EAPI Eina_Bool eina_binbuf_remove(Eina_Binbuf *buf, size_t start, size_t end) EINA_ARG_NONNULL(1);

/**
 * @brief Retrieves a pointer to the contents of a string buffer.
 *
 * @param[in] buf The string buffer.
 * @return The string that is contained in @p buf.
 *
 * This function returns the string contained in @p buf. The returned
 * value must not be modified and will no longer be valid if @p buf is
 * modified. In other words, calling eina_binbuf_append() or similar
 * functions will make the pointer invalid.
 *
 * @see eina_binbuf_string_steal()
 */
EAPI const unsigned char *eina_binbuf_string_get(const Eina_Binbuf *buf) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Steals the contents of a string buffer.
 *
 * @param[in,out] buf The string buffer.
 * @return The string that was contained in @p buf.
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
 * @brief Frees the contents of a string buffer but not the buffer.
 *
 * @param[in,out] buf The string buffer.
 *
 * This function frees the string contained in @p buf without freeing
 * @p buf.
 */
EAPI void eina_binbuf_string_free(Eina_Binbuf *buf) EINA_ARG_NONNULL(1);

/**
 * @brief Retrieves the length of the string buffer's content.
 *
 * @param[in] buf The string buffer.
 * @return The current length of the string, in bytes.
 *
 * This function returns the length of @p buf.
 */
EAPI size_t eina_binbuf_length_get(const Eina_Binbuf *buf) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Gets a read-only slice of the buffer contents.
 *
 * @param[in] buf The string buffer.
 * @return A read-only slice of the current contents. It may become
 * invalid as soon as @p buf is changed.
 *
 * @since 1.19
 */
EAPI Eina_Slice eina_binbuf_slice_get(const Eina_Binbuf *buf) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Gets a read-write slice of the buffer contents.
 *
 * @param[in,out] buf The string buffer.
 * @return A read-write slice for the current contents. It may become
 * invalid as soon as @p buf is changed, such as through calls like
 * eina_binbuf_append() or eina_binbuf_remove().
 *
 * @see eina_binbuf_expand()
 *
 * @since 1.19
 */
EAPI Eina_Rw_Slice eina_binbuf_rw_slice_get(const Eina_Binbuf *buf) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Frees the buffer, returning its old contents.
 *
 * @param[in,out] buf The string buffer.
 * @return The content contained by buf. The caller must release the
 * memory of the returned string by calling free().
 *
 * @since 1.19
 */
EAPI unsigned char* eina_binbuf_release(Eina_Binbuf *buf) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @}
 */

/**
 * @}
 */

#endif /* EINA_STRBUF_H */
