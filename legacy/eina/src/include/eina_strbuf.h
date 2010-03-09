#ifndef EINA_STRBUF_H
#define EINA_STRBUF_H

#include <stddef.h>

#include "eina_types.h"

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

EAPI Eina_Strbuf *eina_strbuf_new(void) EINA_MALLOC EINA_WARN_UNUSED_RESULT;
EAPI void eina_strbuf_free(Eina_Strbuf *buf) EINA_ARG_NONNULL(1);
EAPI void eina_strbuf_reset(Eina_Strbuf *buf) EINA_ARG_NONNULL(1);

EAPI Eina_Bool eina_strbuf_append(Eina_Strbuf *buf, const char *str) EINA_ARG_NONNULL(1, 2);
EAPI Eina_Bool eina_strbuf_append_escaped(Eina_Strbuf *buf, const char *str) EINA_ARG_NONNULL(1, 2);
EAPI Eina_Bool eina_strbuf_append_n(Eina_Strbuf *buf, const char *str, size_t maxlen) EINA_ARG_NONNULL(1, 2);
EAPI Eina_Bool eina_strbuf_append_length(Eina_Strbuf *buf, const char *str, size_t length) EINA_ARG_NONNULL(1, 2);
EAPI Eina_Bool eina_strbuf_append_char(Eina_Strbuf *buf, char c) EINA_ARG_NONNULL(1);

EAPI Eina_Bool eina_strbuf_insert(Eina_Strbuf *buf, const char *str, size_t pos) EINA_ARG_NONNULL(1, 2);
EAPI Eina_Bool eina_strbuf_insert_escaped(Eina_Strbuf *buf, const char *str, size_t pos) EINA_ARG_NONNULL(1, 2);
EAPI Eina_Bool eina_strbuf_insert_n(Eina_Strbuf *buf, const char *str, size_t maxlen, size_t pos) EINA_ARG_NONNULL(1, 2);
EAPI Eina_Bool eina_strbuf_insert_length(Eina_Strbuf *buf, const char *str, size_t length, size_t pos) EINA_ARG_NONNULL(1, 2);
EAPI Eina_Bool eina_strbuf_insert_char(Eina_Strbuf *buf, char c, size_t pos) EINA_ARG_NONNULL(1);

/**
 * @def eina_strbuf_prepend(buf, str)
 * @brief Prepend the given string to the given buffer
 *
 * @param buf The string buffer to prepend to.
 * @param str The string to prepend.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This macro is calling eina_strbuf_insert() at position 0.If @p buf
 * can't prepend it, #EINA_FALSE is returned, otherwise #EINA_TRUE is
 * returned.
 */
#define eina_strbuf_prepend(buf, str) eina_strbuf_insert(buf, str, 0)

/**
 * @def eina_strbuf_prepend_escaped(buf, str)
 * @brief Prepend the given escaped string to the given buffer
 *
 * @param buf The string buffer to prepend to.
 * @param str The string to prepend.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This macro is calling eina_strbuf_insert_escaped() at position 0. If
 * @p buf can't prepend it, #EINA_FALSE is returned, otherwise
 * #EINA_TRUE is returned.
 */
#define eina_strbuf_prepend_escaped(buf, str) eina_strbuf_insert_escaped(buf, str, 0)

/**
 * @def eina_strbuf_prepend_n(buf, str)
 * @brief Prepend the given escaped string to the given buffer
 *
 * @param buf The string buffer to prepend to.
 * @param str The string to prepend.
 * @param maxlen The maximum number of chars to prepend.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This macro is calling eina_strbuf_insert_n() at position 0. If
 * @p buf can't prepend it, #EINA_FALSE is returned, otherwise
 * #EINA_TRUE is returned.
 */
#define eina_strbuf_prepend_n(buf, str, maxlen) eina_strbuf_insert_n(buf, str, maxlen, 0)

/**
 * @def eina_strbuf_prepend_length(buf, str)
 * @brief Prepend the given escaped string to the given buffer
 *
 * @param buf The string buffer to prepend to.
 * @param str The string to prepend.
 * @param length The exact length to use.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This macro is calling eina_strbuf_insert_length() at position 0. If
 * @p buf can't prepend it, #EINA_FALSE is returned, otherwise
 * #EINA_TRUE is returned.
 */
#define eina_strbuf_prepend_length(buf, str, length) eina_strbuf_insert_length(buf, str, length, 0)

/**
 * @def eina_strbuf_prepend_char(buf, str)
 * @brief Prepend the given character to the given buffer
 *
 * @param buf The string buffer to prepend to.
 * @param c The character to prepend.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This macro is calling eina_strbuf_insert_char() at position 0. If
 * @p buf can't prepend it, #EINA_FALSE is returned, otherwise
 * #EINA_TRUE is returned.
 */
#define eina_strbuf_prepend_char(buf, c) eina_strbuf_insert_char(buf, c, 0)


EAPI Eina_Bool eina_strbuf_remove(Eina_Strbuf *buf, size_t start, size_t end) EINA_ARG_NONNULL(1);
EAPI const char *eina_strbuf_string_get(const Eina_Strbuf *buf) EINA_ARG_NONNULL(1);
EAPI char *eina_strbuf_string_steal(Eina_Strbuf *buf) EINA_MALLOC EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
EAPI size_t eina_strbuf_length_get(const Eina_Strbuf *buf) EINA_ARG_NONNULL(1);
EAPI Eina_Bool eina_strbuf_replace(Eina_Strbuf *buf, const char *str, const char *with, unsigned int n) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @def eina_strbuf_replace_first(buf, str, with)
 * @brief Prepend the given character to the given buffer
 *
 * @param buf The string buffer to work with.
 * @param str The string to replace.
 * @param with The replaceing string.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This macro is calling eina_strbuf_replace() with the n-th occurence
 * equal to @c 1. If @p buf can't replace it, #EINA_FALSE is returned,
 * otherwise #EINA_TRUE is returned.
 */
#define eina_strbuf_replace_first(buf, str, with) eina_strbuf_replace(buf, str, with, 1)

EAPI int eina_strbuf_replace_all(Eina_Strbuf *buf, const char *str, const char *with) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @}
 */

/**
 * @}
 */

#endif /* EINA_STRBUF_H */
