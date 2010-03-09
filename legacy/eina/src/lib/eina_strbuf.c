/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "eina_private.h"
#include "eina_str.h"
#include "eina_magic.h"
#include "eina_error.h"
#include "eina_safety_checks.h"
#include "eina_strbuf.h"

#include <stdlib.h>
#include <string.h>

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

static const char EINA_MAGIC_STRBUF_STR[] = "Eina Strbuf";

#define EINA_MAGIC_CHECK_STRBUF(d, ...)				\
   do {								\
	if (!EINA_MAGIC_CHECK((d), EINA_MAGIC_STRBUF))		\
	  {							\
	     EINA_MAGIC_FAIL((d), EINA_MAGIC_STRBUF);		\
	     return __VA_ARGS__;				\
	  }							\
   } while (0);

#define EINA_STRBUF_INIT_SIZE 32
#define EINA_STRBUF_INIT_STEP 32
#define EINA_STRBUF_MAX_STEP 4096

struct _Eina_Strbuf
{
   char *buf;
   size_t len;
   size_t size;
   size_t step;

   EINA_MAGIC
};

/**
 * @internal
 *
 * init the buffer
 * @param buf the buffer to init
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 */
static Eina_Bool
_eina_strbuf_init(Eina_Strbuf *buf)
{
   buf->len = 0;
   buf->size = EINA_STRBUF_INIT_SIZE;
   buf->step = EINA_STRBUF_INIT_STEP;

   eina_error_set(0);
   buf->buf = malloc(buf->size);
   if (EINA_UNLIKELY(!buf->buf))
     {
	eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
	return EINA_FALSE;
     }
   buf->buf[0] = '\0';
   return EINA_TRUE;
}

/**
 * @internal
 *
 * resize the buffer
 * @param buf the buffer to resize
 * @param size the minimum size of the buffer
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 */
static inline Eina_Bool
_eina_strbuf_resize(Eina_Strbuf *buf, size_t size)
{
   size_t new_size, new_step, delta;
   char *buffer;

   size += 1; // Add extra space for '\0'

   if (size == buf->size)
     /* nothing to do */
     return EINA_TRUE;
   else if (size > buf->size)
     delta = size - buf->size;
   else
     delta = buf->size - size;

   /* check if should keep the same step (just used while growing) */
   if ((delta <= buf->step) && (size > buf->size))
     new_step = buf->step;
   else
     {
	new_step = (((delta / EINA_STRBUF_INIT_STEP) + 1)
		    * EINA_STRBUF_INIT_STEP);

	if (new_step > EINA_STRBUF_MAX_STEP)
	  new_step = EINA_STRBUF_MAX_STEP;
     }

   new_size = (((size / new_step) + 1) * new_step);

   /* reallocate the buffer to the new size */
   buffer = realloc(buf->buf, new_size);
   if (EINA_UNLIKELY(!buffer))
     {
	eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
	return EINA_FALSE;
     }

   buf->buf = buffer;
   buf->size = new_size;
   buf->step = new_step;
   eina_error_set(0);
   return EINA_TRUE;
}

/**
 * @internal
 *
 * If required, enlarge the buffer to fit the new size.
 *
 * @param buf the buffer to resize
 * @param size the minimum size of the buffer
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 */
static inline Eina_Bool
_eina_strbuf_grow(Eina_Strbuf *buf, size_t size)
{
   if ((size + 1) < buf->size)
     return EINA_TRUE;
   return _eina_strbuf_resize(buf, size);
}

/**
 * @internal
 *
 * insert string of known length at random within existing strbuf limits.
 *
 * @param buf the buffer to resize, must be valid.
 * @param str the string to copy, must be valid (!NULL and smaller than @a len)
 * @param len the amount of bytes in @a str to copy, must be valid.
 * @param pos the position inside buffer to insert, must be valid (smaller
 *        than eina_strbuf_length_get())
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 */
static inline Eina_Bool
_eina_strbuf_insert_length(Eina_Strbuf *buf, const char *str, size_t len, size_t pos)
{
   if (EINA_UNLIKELY(!_eina_strbuf_grow(buf, buf->len + len)))
     return EINA_FALSE;

   /* move the existing text */
   memmove(buf->buf + len + pos, buf->buf + pos, buf->len - pos);

   /* and now insert the given string */
   memcpy(buf->buf + pos, str, len);

   buf->len += len;
   buf->buf[buf->len] = '\0';
   return EINA_TRUE;
}

/**
 * @endcond
 */

/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

/**
 * @internal
 * @brief Initialize the strbuf module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function sets up the strbuf module of Eina. It is called by
 * eina_init().
 *
 * @see eina_init()
 */
Eina_Bool
eina_strbuf_init(void)
{
   eina_magic_string_static_set(EINA_MAGIC_STRBUF, EINA_MAGIC_STRBUF_STR);
   return EINA_TRUE;
}

/**
 * @internal
 * @brief Shut down the strbuf module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function shuts down the strbuf module set up by
 * eina_strbuf_init(). It is called by eina_shutdown().
 *
 * @see eina_shutdown()
 */
Eina_Bool
eina_strbuf_shutdown(void)
{
   return EINA_TRUE;
}

/**
 * @endcond
 */

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

/**
 * @addtogroup Eina_String_Buffer_Group String Buffer
 *
 * @brief These functions provide string buffers management.
 *
 * The String Buffer data type is designed to be a mutable string,
 * allowing to append, prepend or insert a string to a buffer. 
 *
 * @{
 */

/**
 * @brief Create a new string buffer.
 *
 * @return Newly allocated string buffer instance.
 *
 * This function creates a new string buffer. On error, @c NULL is
 * returned and Eina error is set to #EINA_ERROR_OUT_OF_MEMORY. To
 * free the resources, use eina_strbuf_free().
 *
 * @see eina_strbuf_free()
 * @see eina_strbuf_append()
 * @see eina_strbuf_string_get()
 */
EAPI Eina_Strbuf *
eina_strbuf_new(void)
{
   Eina_Strbuf *buf;

   eina_error_set(0);
   buf = malloc(sizeof(Eina_Strbuf));
   if (EINA_UNLIKELY(!buf))
     {
	eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
	return NULL;
     }
   EINA_MAGIC_SET(buf, EINA_MAGIC_STRBUF);

   if (EINA_UNLIKELY(!_eina_strbuf_init(buf)))
     {
	eina_strbuf_free(buf);
	return NULL;
     }

   return buf;
}

/**
 * @brief Free a string buffer.
 *
 * @param buf The string buffer to free.
 *
 * This function frees the memory of @p buf. @p buf must have been
 * created by eina_strbuf_new().
 */
EAPI void
eina_strbuf_free(Eina_Strbuf *buf)
{
   EINA_MAGIC_CHECK_STRBUF(buf);
   EINA_MAGIC_SET(buf, EINA_MAGIC_NONE);
   free(buf->buf);
   free(buf);
}

/**
 * @brief Reset a string buffer.
 *
 * @param buf The string buffer to reset.
 *
 * This function reset @p buf: the buffer len is set to 0, and the
 * string is set to '\\0'. No memory is free'd.
 */
EAPI void
eina_strbuf_reset(Eina_Strbuf *buf)
{
   EINA_MAGIC_CHECK_STRBUF(buf);
   buf->len = 0;
   buf->step = EINA_STRBUF_INIT_STEP;

   buf->buf[0] = '\0';
}

/**
 * @brief Append a string to a buffer, reallocating as necessary.
 *
 * @param buf The string buffer to append to.
 * @param str The string to append.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function appends @p str to @p buf. It computes the length of
 * @p str, so is slightly slower than eina_strbuf_append_length(). If
 * the length is known beforehand, consider using that variant. If
 * @p buf can't append it, #EINA_FALSE is returned, otherwise
 * #EINA_TRUE is returned.
 *
 * @see eina_strbuf_append()
 * @see eina_strbuf_append_length()
 */
EAPI Eina_Bool
eina_strbuf_append(Eina_Strbuf *buf, const char *str)
{
   size_t len;

   EINA_SAFETY_ON_NULL_RETURN_VAL(str, EINA_FALSE);
   EINA_MAGIC_CHECK_STRBUF(buf, EINA_FALSE);

   len = strlen(str);
   if (EINA_UNLIKELY(!_eina_strbuf_grow(buf, buf->len + len)))
     return EINA_FALSE;
   memcpy(buf->buf + buf->len, str, len + 1);
   buf->len += len;
   return EINA_TRUE;
}

/**
 * @brief Append an escaped string to a buffer, reallocating as necessary.
 *
 * @param buf The string buffer to append to.
 * @param str The string to append.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function appends the escaped string @p str to @p buf. If @p
 * str can not be appended, #EINA_FALSE is returned, otherwise,
 * #EINA_TRUE is returned.
 */
EAPI Eina_Bool
eina_strbuf_append_escaped(Eina_Strbuf *buf, const char *str)
{
   size_t len;
   char *esc;

   EINA_SAFETY_ON_NULL_RETURN_VAL(str, EINA_FALSE);
   EINA_MAGIC_CHECK_STRBUF(buf, EINA_FALSE);

   esc = eina_str_escape(str);
   if (EINA_UNLIKELY(!esc))
     return EINA_FALSE;
   len = strlen(esc);
   if (EINA_UNLIKELY(!_eina_strbuf_grow(buf, buf->len + len)))
     {
	free(esc);
	return EINA_FALSE;
     }
   memcpy(buf->buf + buf->len, esc, len + 1);
   buf->len += len;
   free(esc);
   return EINA_TRUE;
}

/**
 * @brief Append a string to a buffer, reallocating as necessary,
 * limited by the given length.
 *
 * @param buf The string buffer to append to.
 * @param str The string to append.
 * @param maxlen The maximum number of characters to append.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function appends at most @p maxlen characters of @p str to
 * @p buf. It can't appends more than the length of @p str. It
 * computes the length of @p str, so is slightly slower than
 * eina_strbuf_append_length(). If the length is known beforehand,
 * consider using that variant (@p maxlen should then be checked so
 * that it is greater than the size of @p str). If @p str can not be
 * appended, #EINA_FALSE is returned, otherwise, #EINA_TRUE is
 * returned.
 *
 * @see eina_strbuf_append()
 * @see eina_strbuf_append_length()
 */
EAPI Eina_Bool
eina_strbuf_append_n(Eina_Strbuf *buf, const char *str, size_t maxlen)
{
   size_t len;

   EINA_SAFETY_ON_NULL_RETURN_VAL(str, EINA_FALSE);
   EINA_MAGIC_CHECK_STRBUF(buf, EINA_FALSE);

   len = strlen(str);
   if (len > maxlen) len = maxlen;
   if (EINA_UNLIKELY(!_eina_strbuf_grow(buf, buf->len + len)))
     return EINA_FALSE;

   memcpy(buf->buf + buf->len, str, len);
   buf->len += len;
   buf->buf[buf->len] = '\0';
   return EINA_TRUE;
}

/**
 * @brief Append a string of exact length to a buffer, reallocating as necessary.
 *
 * @param buf The string buffer to append to.
 * @param str The string to append.
 * @param length The exact length to use.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function appends @p str to @p buf. @p str must be of size at
 * most @p length. It is slightly faster than eina_strbuf_append() as
 * it does not compute the size of @p str. It is useful when dealing
 * with strings of known size, such as eina_strngshare. If @p buf
 * can't append it, #EINA_FALSE is returned, otherwise #EINA_TRUE is
 * returned.
 *
 * @see eina_stringshare_length()
 * @see eina_strbuf_append()
 * @see eina_strbuf_append_n()
 */
EAPI Eina_Bool
eina_strbuf_append_length(Eina_Strbuf *buf, const char *str, size_t length)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(str, EINA_FALSE);
   EINA_MAGIC_CHECK_STRBUF(buf, EINA_FALSE);

   if (EINA_UNLIKELY(!_eina_strbuf_grow(buf, buf->len + length)))
     return EINA_FALSE;

   memcpy(buf->buf + buf->len, str, length);
   buf->len += length;
   buf->buf[buf->len] = '\0';
   return EINA_TRUE;
}

/**
 * @brief Insert a string to a buffer, reallocating as necessary.
 *
 * @param buf The string buffer to insert.
 * @param str The string to insert.
 * @param pos The position to insert the string.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function inserts @p str to @p buf at position @p pos. It
 * computes the length of @p str, so is slightly slower than
 * eina_strbuf_insert_length(). If  the length is known beforehand,
 * consider using that variant. If @p buf can't insert it, #EINA_FALSE
 * is returned, otherwise #EINA_TRUE is returned.
 */
EAPI Eina_Bool
eina_strbuf_insert(Eina_Strbuf *buf, const char *str, size_t pos)
{
   size_t len;

   EINA_SAFETY_ON_NULL_RETURN_VAL(str, EINA_FALSE);
   EINA_MAGIC_CHECK_STRBUF(buf, EINA_FALSE);

   if (pos >= buf->len)
     return eina_strbuf_append(buf, str);

   len = strlen(str);
   return _eina_strbuf_insert_length(buf, str, len, pos);
}

/**
 * @brief Insert an escaped string to a buffer, reallocating as
 * necessary.
 *
 * @param buf The string buffer to insert to.
 * @param str The string to insert.
 * @param pos The position to insert the string.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function inserts the escaped string @p str to @p buf at
 * position @p pos. If @p buf can't insert @p str, #EINA_FALSE is
 * returned, otherwise #EINA_TRUE is returned.
 */
EAPI Eina_Bool
eina_strbuf_insert_escaped(Eina_Strbuf *buf, const char *str, size_t pos)
{
   Eina_Bool ret;
   size_t len;
   char *esc;

   EINA_SAFETY_ON_NULL_RETURN_VAL(str, EINA_FALSE);
   EINA_MAGIC_CHECK_STRBUF(buf, EINA_FALSE);

   esc = eina_str_escape(str);
   if (EINA_UNLIKELY(!esc))
     return EINA_FALSE;
   len = strlen(esc);
   ret = _eina_strbuf_insert_length(buf, esc, len, pos);
   free(esc);
   return ret;
}

/**
 * @brief Insert a string to a buffer, reallocating as necessary. Limited by maxlen.
 *
 * @param buf The string buffer to insert to.
 * @param str The string to insert.
 * @param maxlen The maximum number of chars to insert.
 * @param pos The position to insert the string.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function inserts @p str ot @p buf at position @p pos, with at
 * most @p maxlen bytes. The number of inserted characters can not be
 * greater than the length of @p str. It computes the length of
 * @p str, so is slightly slower than eina_strbuf_insert_length(). If the
 * length is known beforehand, consider using that variant (@p maxlen
 * should then be checked so that it is greater than the size of
 * @p str). If @p str can not be inserted, #EINA_FALSE is returned,
 * otherwise, #EINA_TRUE is returned.
 */
EAPI Eina_Bool
eina_strbuf_insert_n(Eina_Strbuf *buf, const char *str, size_t maxlen, size_t pos)
{
   size_t len;

   EINA_SAFETY_ON_NULL_RETURN_VAL(str, EINA_FALSE);
   EINA_MAGIC_CHECK_STRBUF(buf, EINA_FALSE);

   if (pos >= buf->len)
     return eina_strbuf_append_n(buf, str, maxlen);

   len = strlen(str);
   if (len > maxlen) len = maxlen;
   return _eina_strbuf_insert_length(buf, str, len, pos);
}

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
 * most @p length. It is slightly faster than eina_strbuf_insert() as
 * it does not compute the size of @p str. It is useful when dealing
 * with strings of known size, such as eina_strngshare. If @p buf
 * can't insert it, #EINA_FALSE is returned, otherwise #EINA_TRUE is
 * returned.
 *
 * @see eina_stringshare_length()
 * @see eina_strbuf_insert()
 * @see eina_strbuf_insert_n()
 */
EAPI Eina_Bool
eina_strbuf_insert_length(Eina_Strbuf *buf, const char *str, size_t length, size_t pos)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(str, EINA_FALSE);
   EINA_MAGIC_CHECK_STRBUF(buf, EINA_FALSE);

   if (pos >= buf->len)
     return eina_strbuf_append_length(buf, str, length);

   return _eina_strbuf_insert_length(buf, str, length, pos);
}

/**
 * @brief Append a character to a string buffer, reallocating as
 * necessary.
 *
 * @param buf The string buffer to append to.
 * @param c The char to append.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function inserts @p c to @p buf. If it can not insert it,
 * #EINA_FALSE is returned, otherwise #EINA_TRUE is returned.
 */
EAPI Eina_Bool
eina_strbuf_append_char(Eina_Strbuf *buf, char c)
{
   EINA_MAGIC_CHECK_STRBUF(buf, EINA_FALSE);

   if (EINA_UNLIKELY(!_eina_strbuf_grow(buf, buf->len + 1)))
     return EINA_FALSE;
   buf->buf[(buf->len)++] = c;
   buf->buf[buf->len] = '\0';
   return EINA_TRUE;
}

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
EAPI Eina_Bool
eina_strbuf_insert_char(Eina_Strbuf *buf, char c, size_t pos)
{
   EINA_MAGIC_CHECK_STRBUF(buf, EINA_FALSE);

   if (pos >= buf->len)
     return eina_strbuf_append_char(buf, c);

   return _eina_strbuf_insert_length(buf, &c, 1, pos);
}

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
EAPI Eina_Bool
eina_strbuf_remove(Eina_Strbuf *buf, size_t start, size_t end)
{
   size_t remove_len, tail_len;

   EINA_MAGIC_CHECK_STRBUF(buf, EINA_FALSE);

   if (end >= buf->len)
     end = buf->len;

   if (end <= start)
     return EINA_TRUE;

   remove_len = end - start;
   if (remove_len == buf->len)
     {
	free(buf->buf);
	return _eina_strbuf_init(buf);
     }

   tail_len = buf->len - end + 1; /* includes '\0' */
   memmove(buf->buf + start, buf->buf + end, tail_len);
   buf->len -= remove_len;
   return _eina_strbuf_resize(buf, buf->len);
}

/**
 * @brief Retrieve a pointer to the contents of a string buffer
 *
 * @param buf The string buffer.
 * @return The current string in the string buffer.
 *
 * This function returns the string contained in @p buf. The returned
 * value must not be modified and will no longer be valid if @p buf is
 * modified. In other words, any eina_strbuf_append() or similar will
 * make that pointer invalid.
 *
 * @see eina_strbuf_string_steal()
 */
EAPI const char *
eina_strbuf_string_get(const Eina_Strbuf *buf)
{
   EINA_MAGIC_CHECK_STRBUF(buf, NULL);

   return buf->buf;
}

/**
 * @brief Steal the contents of a string buffer.
 *
 * @param buf The string buffer to steal.
 * @return The current string in the string buffer.
 *
 * This function returns the string contained in @p buf. @p buf is
 * then initialized and does not own anymore the returned string. The
 * caller must release the memory of the returned string by calling
 * free(). 
 *
 * @see eina_strbuf_string_get()
 */
EAPI char *
eina_strbuf_string_steal(Eina_Strbuf *buf)
{
   char *ret;
   EINA_MAGIC_CHECK_STRBUF(buf, NULL);

   ret = buf->buf;
   // TODO: Check return value and do something clever
   _eina_strbuf_init(buf);
   return ret;
}

/**
 * @brief Retrieve the length of the string buffer content.
 *
 * @param buf The string buffer.
 * @return The current length of the string, in bytes.
 *
 * This function returns the length of @p buf.
 */
EAPI size_t
eina_strbuf_length_get(const Eina_Strbuf *buf)
{
   EINA_MAGIC_CHECK_STRBUF(buf, 0);

   return buf->len;
}

/**
 * @ brief Replace the n-th string with an other string.
 *
 * @param buf The string buffer to work with.
 * @param str The string to replace.
 * @param with The replaceing string.
 * @param n The number of the fitting string.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function replaces the n-th occurence of @p str in @p buf with
 * @p with. It returns #EINA_FALSE on failure, #EINA_TRUE otherwise.
 */
EAPI Eina_Bool
eina_strbuf_replace(Eina_Strbuf *buf, const char *str, const char *with, unsigned int n)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(str, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(with, EINA_FALSE);
   EINA_MAGIC_CHECK_STRBUF(buf, 0);

   size_t len1, len2;
   char *spos;
   size_t pos;

   if (n == 0)
     return EINA_FALSE;

   spos = buf->buf;
   while (n--)
     {
	spos = strstr(spos, str);
	if (!spos || *spos == '\0')
	  return EINA_FALSE;
	if (n) spos++;
     }

   pos = spos - buf->buf;
   len1 = strlen(str);
   len2 = strlen(with);
   if (len1 != len2)
     {
	/* resize the buffer if necessary */
	if (EINA_UNLIKELY(!_eina_strbuf_grow(buf, buf->len - len1 + len2)))
	  return EINA_FALSE;
	/* move the existing text */
	memmove(buf->buf + pos + len2, buf->buf + pos + len1,
	      buf->len - pos - len1);
     }
   /* and now insert the given string */
   memcpy(buf->buf + pos, with, len2);
   buf->len += len2 - len1;
   buf->buf[buf->len] = 0;

   return EINA_TRUE;
}

/**
 * @brief Replace all strings with an other string.

 * @param buf the string buffer to work with.
 * @param str The string to replace.
 * @param with The replaceing string.
 * @return How often the string was replaced.
 *
 * This function replaces all the occurences of @p str in @ buf with
 * the string @p with. This function returns the number of times @p str
 * has been replaced. On failure, it returns 0.
 */
EAPI int
eina_strbuf_replace_all(Eina_Strbuf *buf, const char *str, const char *with)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(str, 0);
   EINA_SAFETY_ON_NULL_RETURN_VAL(with, 0);
   EINA_MAGIC_CHECK_STRBUF(buf, 0);

   size_t len1, len2, len;
   char *tmp_buf = NULL;
   char *spos;
   size_t pos, start;
   size_t pos_tmp, start_tmp;
   int n = 0;

   spos = strstr(buf->buf, str);
   if (!spos || *spos == '\0')
     return 0;

   len1 = strlen(str);
   len2 = strlen(with);

   /* if the size of the two string is equal, it is fairly easy to replace them
    * we don't need to resize the buffer or doing other calculations */
   if (len1 == len2)
     {
	while (spos)
	  {
	     memcpy(spos, with, len2);
	     spos = strstr(spos + len2, str);
	     n++;
	  }
	return n;
     }

   pos = pos_tmp = spos - buf->buf;
   tmp_buf = buf->buf;
   buf->buf = malloc(buf->size);
   if (EINA_UNLIKELY(!buf->buf))
     {
	buf->buf = tmp_buf;
	return 0;
     }
   start = start_tmp = 0;
   len = buf->len;

   while (spos)
     {
	n++;
	len = (len + len2) - len1;
	/* resize the buffer if necessary */
	if (EINA_UNLIKELY(!_eina_strbuf_grow(buf, len)))
	  {
	     /* we have to stop replacing here, because we haven't enough
	      * memory to go on */
	     len = (len + len1) - len2;
	     break;
	  }

	/* copy the untouched text */
	memcpy(buf->buf + start, tmp_buf + start_tmp, pos - start);
	/* copy the new string */
	memcpy(buf->buf + pos, with, len2);

	/* calculate the next positions */
	start_tmp = pos_tmp + len1;
	start = pos + len2;
	spos = strstr(tmp_buf + start_tmp, str);
	/* this calculations don't make sense if spos == NULL, but the
	 * calculated values won't be used, because the loop will stop
	 * then */
	pos_tmp = spos - tmp_buf;
	pos = start + pos_tmp - start_tmp;
     }
   /* and now copy the rest of the text */
   memcpy(buf->buf + start, tmp_buf + start_tmp, len - start);
   buf->len = len;
   buf->buf[buf->len] = 0;

   free(tmp_buf);

   return n;
}

/**
 * @}
 */
